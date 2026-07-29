#include "zone/CharacterStateLoader.hpp"
#include "StatKeyMapping.hpp"
#include "core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

namespace {

int64_t toInt(const std::string& s, int64_t fallback = 0) {
  if (s.empty()) return fallback;
  try {
    return std::stoll(s);
  } catch (...) {
    return fallback;
  }
}

int64_t jsonInt(const nlohmann::json& j, const char* key) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) return 0;
  try {
    if (it->is_number()) return static_cast<int64_t>(it->get<double>());
    if (it->is_string()) return toInt(it->get<std::string>());
  } catch (...) {
  }
  return 0;
}

// Acumula stats de equipamento (stats_json) e refinamento no mapa de totais, espelhando
// character_info_helper.php. Mapeia critical_resistance/resistance -> "resistance".
void accumulateItemStats(const nlohmann::json& stats, std::unordered_map<std::string, int64_t>& totals) {
  if (!stats.is_object()) return;
  static const char* kDirectKeys[] = {
      "strength", "dexterity", "intelligence", "vitality", "luck",
      "health_bonus", "mana_bonus", "defense", "magic_defense", "attack",
      "magic_attack", "accuracy", "dodge", "critical", "movement",
      "double_attack_resistance", "double_attack_rate"};
  for (const char* key : kDirectKeys) {
    const int64_t v = jsonInt(stats, key);
    if (v != 0) totals[key] += v;
  }
  if (stats.contains("critical_resistance")) {
    totals["resistance"] += jsonInt(stats, "critical_resistance");
  } else if (stats.contains("resistance")) {
    totals["resistance"] += jsonInt(stats, "resistance");
  }
}

}  // namespace

CharacterStateLoader::CharacterStateLoader(std::shared_ptr<Database::MySQLConnector> db)
    : db_(std::move(db)) {
  warmRunning_ = true;
  warmThread_ = std::thread([this]() { warmLoop(); });
}

CharacterStateLoader::~CharacterStateLoader() {
  warmRunning_ = false;
  warmCv_.notify_all();
  if (warmThread_.joinable()) {
    warmThread_.join();
  }
}

void CharacterStateLoader::requestWarm(uint32_t playerId) {
  if (playerId == 0) return;
  {
    std::lock_guard<std::mutex> lock(warmMu_);
    if (!warmPending_.insert(playerId).second) return;  // já enfileirado
    warmQueue_.push_back(playerId);
  }
  warmCv_.notify_one();
}

void CharacterStateLoader::warmLoop() {
  while (warmRunning_) {
    uint32_t playerId = 0;
    {
      std::unique_lock<std::mutex> lock(warmMu_);
      warmCv_.wait(lock, [this]() { return !warmRunning_ || !warmQueue_.empty(); });
      if (!warmRunning_) break;
      playerId = warmQueue_.front();
      warmQueue_.pop_front();
      warmPending_.erase(playerId);
    }
    // Carrega fora de qualquer lock; loadPlayerState popula o cache se ainda vazio.
    Combat::CharacterState tmp;
    loadPlayerState(playerId, tmp);
  }
}

bool CharacterStateLoader::getCachedOrWarm(uint32_t playerId, Combat::CharacterState& out) {
  if (tryGetCachedState(playerId, out)) return true;
  requestWarm(playerId);
  return false;
}

void CharacterStateLoader::invalidate(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(cacheMu_);
  cache_.erase(playerId);
}

bool CharacterStateLoader::reloadStatsPreservingVitals(uint32_t playerId) {
  if (playerId == 0) return false;

  int32_t preservedHp = -1;
  int32_t preservedMp = -1;
  {
    std::lock_guard<std::mutex> lock(cacheMu_);
    auto it = cache_.find(playerId);
    if (it != cache_.end()) {
      preservedHp = it->second.state.buffedStats.currentHealth;
      preservedMp = it->second.state.buffedStats.currentMana;
      cache_.erase(it);
    }
  }

  Combat::CharacterState fresh;
  if (!loadPlayerStateFromDb(playerId, fresh)) {
    return false;
  }

  const int32_t newMaxHp = std::max(1, fresh.buffedStats.maxHealth);
  const int32_t newMaxMp = std::max(1, fresh.buffedStats.maxMana);
  if (preservedHp >= 0) {
    fresh.buffedStats.currentHealth = std::min(preservedHp, newMaxHp);
    fresh.baseStats.currentHealth = fresh.buffedStats.currentHealth;
  } else {
    fresh.buffedStats.currentHealth = std::min(fresh.buffedStats.currentHealth, newMaxHp);
    fresh.baseStats.currentHealth = fresh.buffedStats.currentHealth;
  }
  if (preservedMp >= 0) {
    fresh.buffedStats.currentMana = std::min(preservedMp, newMaxMp);
    fresh.baseStats.currentMana = fresh.buffedStats.currentMana;
  } else {
    fresh.buffedStats.currentMana = std::min(fresh.buffedStats.currentMana, newMaxMp);
    fresh.baseStats.currentMana = fresh.buffedStats.currentMana;
  }
  fresh.isAlive = fresh.buffedStats.currentHealth > 0;

  {
    std::lock_guard<std::mutex> lock(cacheMu_);
    CachedState entry;
    entry.state = fresh;
    entry.expiresAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(kCacheTtlMs);
    cache_[playerId] = std::move(entry);
  }

  Core::Logger::getInstance().info(
      "[CharacterStateLoader] reloadStatsPreservingVitals player={} HP={}/{} MP={}/{}",
      playerId, fresh.buffedStats.currentHealth, newMaxHp, fresh.buffedStats.currentMana, newMaxMp);
  return true;
}

void CharacterStateLoader::patchCachedMana(uint32_t playerId, int32_t newMana) {
  std::lock_guard<std::mutex> lock(cacheMu_);
  auto it = cache_.find(playerId);
  if (it == cache_.end()) return;
  it->second.state.baseStats.currentMana = newMana;
  it->second.state.buffedStats.currentMana = newMana;
  // Renova TTL: patch de vital prova que o estado em memória ainda é autoritativo.
  it->second.expiresAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(kCacheTtlMs);
}

void CharacterStateLoader::patchCachedHealth(uint32_t playerId, int32_t newHealth) {
  std::lock_guard<std::mutex> lock(cacheMu_);
  auto it = cache_.find(playerId);
  if (it == cache_.end()) return;
  it->second.state.baseStats.currentHealth = newHealth;
  it->second.state.buffedStats.currentHealth = newHealth;
  // Sem isso: morte deixa isAlive=false no load e respawn/patch de HP não reabilitava skills
  // (CANNOT_CAST "Personagem não pode usar skills").
  it->second.state.isAlive = (newHealth > 0);
  it->second.expiresAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(kCacheTtlMs);
}

void CharacterStateLoader::patchCachedMaxVitals(uint32_t playerId, int32_t maxHealth, int32_t maxMana) {
  std::lock_guard<std::mutex> lock(cacheMu_);
  auto it = cache_.find(playerId);
  if (it == cache_.end()) return;
  if (maxHealth > 0) {
    it->second.state.baseStats.maxHealth = maxHealth;
    it->second.state.buffedStats.maxHealth = maxHealth;
    it->second.state.buffedStats.currentHealth =
        std::min(it->second.state.buffedStats.currentHealth, maxHealth);
    it->second.state.baseStats.currentHealth = it->second.state.buffedStats.currentHealth;
  }
  if (maxMana > 0) {
    it->second.state.baseStats.maxMana = maxMana;
    it->second.state.buffedStats.maxMana = maxMana;
    it->second.state.buffedStats.currentMana =
        std::min(it->second.state.buffedStats.currentMana, maxMana);
    it->second.state.baseStats.currentMana = it->second.state.buffedStats.currentMana;
  }
  it->second.expiresAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(kCacheTtlMs);
}

bool CharacterStateLoader::tryGetCachedState(uint32_t playerId, Combat::CharacterState& out) const {
  std::lock_guard<std::mutex> lock(cacheMu_);
  auto it = cache_.find(playerId);
  if (it == cache_.end()) return false;
  out = it->second.state;
  return true;
}

bool CharacterStateLoader::loadPlayerState(uint32_t playerId, Combat::CharacterState& out) {
  // Hot path de combate: se já há entrada em cache, NUNCA ir ao MySQL (mesmo pós-TTL).
  // TTL forçava JOIN remoto a cada 10s e media 4s no Proxmox → fila de skills em cascata.
  // HP/mana são patchados no cache; buffs de skill agora são in-memory no CombatCoreEngine.
  {
    std::lock_guard<std::mutex> lock(cacheMu_);
    auto it = cache_.find(playerId);
    if (it != cache_.end()) {
      out = it->second.state;
      return true;
    }
  }

  Combat::CharacterState state;
  if (!loadPlayerStateFromDb(playerId, state)) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(cacheMu_);
    cache_[playerId] = CachedState{state, std::chrono::steady_clock::now() +
                                              std::chrono::milliseconds(kCacheTtlMs)};
  }
  out = std::move(state);
  return true;
}

bool CharacterStateLoader::loadPlayerStateFromDb(uint32_t playerId, Combat::CharacterState& out) {
  if (!db_ || !db_->isConnected()) return false;

  const std::string pid = std::to_string(playerId);

  const char* kPlayerStateSql =
      "SELECT p.level, p.health, p.mana, p.class_id, p.pvp, "
      "COALESCE(c.base_strength,10), COALESCE(c.base_dexterity,10), COALESCE(c.base_intelligence,10), "
      "COALESCE(c.base_vitality,10), COALESCE(c.base_luck,10), "
      "COALESCE(c.base_health, p.max_health), COALESCE(c.base_mana, p.max_mana), "
      "COALESCE(c.base_physical_attack,0), COALESCE(c.base_magic_attack,0), "
      "COALESCE(c.base_physical_defense,0), COALESCE(c.base_magic_defense,0), "
      "COALESCE(c.base_accuracy,0), COALESCE(c.base_dodge,0), COALESCE(c.base_critical,0), "
      "COALESCE(c.base_movement,0), COALESCE(c.base_critical_resistance,0), "
      "COALESCE(c.base_double_attack_rate,0), "
      "COALESCE(sp.strength_points,0), COALESCE(sp.dexterity_points,0), COALESCE(sp.intelligence_points,0), "
      "COALESCE(sp.vitality_points,0), COALESCE(sp.luck_points,0), "
      "COALESCE(p.is_dead,0) "
      "FROM players p "
      "LEFT JOIN classes c ON p.class_id = c.class_id "
      "LEFT JOIN player_stat_points sp ON sp.player_id = p.id "
      "WHERE p.id = ? LIMIT 1";

  auto rows = db_->executePreparedQuery(kPlayerStateSql, {pid});
  // Rede de segurança: se o prepared vier vazio (edge do C API em MySQL 8), cair para
  // texto com pid numérico (injection-safe), espelhando o fallback de active_buffs abaixo.
  if (rows.empty() || rows[0].size() < 28) {
    const std::string textSql =
        "SELECT p.level, p.health, p.mana, p.class_id, p.pvp, "
        "COALESCE(c.base_strength,10), COALESCE(c.base_dexterity,10), COALESCE(c.base_intelligence,10), "
        "COALESCE(c.base_vitality,10), COALESCE(c.base_luck,10), "
        "COALESCE(c.base_health, p.max_health), COALESCE(c.base_mana, p.max_mana), "
        "COALESCE(c.base_physical_attack,0), COALESCE(c.base_magic_attack,0), "
        "COALESCE(c.base_physical_defense,0), COALESCE(c.base_magic_defense,0), "
        "COALESCE(c.base_accuracy,0), COALESCE(c.base_dodge,0), COALESCE(c.base_critical,0), "
        "COALESCE(c.base_movement,0), COALESCE(c.base_critical_resistance,0), "
        "COALESCE(c.base_double_attack_rate,0), "
        "COALESCE(sp.strength_points,0), COALESCE(sp.dexterity_points,0), COALESCE(sp.intelligence_points,0), "
        "COALESCE(sp.vitality_points,0), COALESCE(sp.luck_points,0), "
        "COALESCE(p.is_dead,0) "
        "FROM players p "
        "LEFT JOIN classes c ON p.class_id = c.class_id "
        "LEFT JOIN player_stat_points sp ON sp.player_id = p.id "
        "WHERE p.id = " + pid + " LIMIT 1";
    rows = db_->executeQuery(textSql);
  }

  if (rows.empty() || rows[0].size() < 28) return false;
  const auto& r = rows[0];

  const int64_t level = toInt(r[0], 1);
  const int64_t health = toInt(r[1]);
  const int64_t mana = toInt(r[2]);
  const int64_t classId = toInt(r[3]);
  const int64_t pvpFlag = toInt(r[4]);

  const int64_t baseStrength = toInt(r[5], 10);
  const int64_t baseDexterity = toInt(r[6], 10);
  const int64_t baseIntelligence = toInt(r[7], 10);
  const int64_t baseVitality = toInt(r[8], 10);
  const int64_t baseLuck = toInt(r[9], 10);
  const int64_t baseHealth = toInt(r[10], 100);
  const int64_t baseMana = toInt(r[11], 50);
  const int64_t basePhysAtk = toInt(r[12]);
  const int64_t baseMagAtk = toInt(r[13]);
  const int64_t basePhysDef = toInt(r[14]);
  const int64_t baseMagDef = toInt(r[15]);
  const int64_t baseAccuracy = toInt(r[16]);
  const int64_t baseDodge = toInt(r[17]);
  const int64_t baseCritical = toInt(r[18]);
  const int64_t baseMovement = toInt(r[19]);
  const int64_t baseCritRes = toInt(r[20]);
  const int64_t baseDoubleAtk = toInt(r[21]);

  const int64_t strPoints = toInt(r[22]);
  const int64_t dexPoints = toInt(r[23]);
  const int64_t intPoints = toInt(r[24]);
  const int64_t vitPoints = toInt(r[25]);
  const int64_t lckPoints = toInt(r[26]);
  const int64_t isDeadFlag = toInt(r[27]);

  const int64_t levelHp = level * 20;
  const int64_t levelMp = level * 20;
  const int64_t levelPhysAtk = level * 5;
  const int64_t levelMagAtk = level * 5;
  const int64_t levelPhysDef = level * 3;
  const int64_t levelMagDef = level * 3;

  std::unordered_map<std::string, int64_t> t;
  t["strength"] = baseStrength + strPoints;
  t["dexterity"] = baseDexterity + dexPoints;
  t["intelligence"] = baseIntelligence + intPoints;
  t["vitality"] = baseVitality + vitPoints;
  t["luck"] = baseLuck + lckPoints;
  t["health_bonus"] = 0;
  t["mana_bonus"] = 0;
  t["defense"] = basePhysDef + levelPhysDef;
  t["magic_defense"] = baseMagDef + levelMagDef;
  t["attack"] = basePhysAtk + levelPhysAtk;
  t["magic_attack"] = baseMagAtk + levelMagAtk;
  t["accuracy"] = baseAccuracy;
  t["dodge"] = baseDodge;
  t["critical"] = baseCritical;
  t["movement"] = baseMovement;
  t["resistance"] = baseCritRes;
  t["double_attack_resistance"] = 0;
  t["double_attack_rate"] = baseDoubleAtk;

  // Equipamento equipado: somar stats_json + refinement_bonus_stats.
  // Texto com pid numérico (injection-safe): prepared com '?' retorna vazio neste ambiente
  // (MySQL remoto), mesmo motivo do fallback de active_buffs. Sem texto, equipamento não soma.
  auto equipRows = db_->executeQuery(
      "SELECT COALESCE(it.stats_json,''), COALESCE(pi.refinement_bonus_stats,'') "
      "FROM player_inventory pi "
      "INNER JOIN item_templates it ON pi.item_template_id = it.item_id "
      "WHERE pi.player_id = " + pid + " AND pi.is_equipped = TRUE");

  for (const auto& er : equipRows) {
    if (er.empty()) continue;
    nlohmann::json stats = nlohmann::json::object();
    if (!er[0].empty() && er[0] != "null") {
      nlohmann::json parsed = nlohmann::json::parse(er[0], nullptr, false);
      if (parsed.is_object()) stats = parsed;
    }
    if (er.size() > 1 && !er[1].empty() && er[1] != "null") {
      nlohmann::json bonus = nlohmann::json::parse(er[1], nullptr, false);
      if (bonus.is_object()) {
        for (auto it = bonus.begin(); it != bonus.end(); ++it) {
          const int64_t v = jsonInt(bonus, it.key().c_str());
          if (!stats.contains(it.key())) stats[it.key()] = 0;
          stats[it.key()] = jsonInt(stats, it.key().c_str()) + v;
        }
      }
    }
    accumulateItemStats(stats, t);
  }

  // Buffs temporários de poção (player_item_buffs) — somados antes do scaling de atributo.
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  auto buffRows = db_->executeQuery(
      "SELECT buff_key, bonus_value FROM player_item_buffs "
      "WHERE player_id = " + pid + " AND expires_at_ms > " + std::to_string(nowMs));
  for (const auto& br : buffRows) {
    if (br.size() < 2) continue;
    const std::string& key = br[0];
    if (key.size() < 6 || key.compare(key.size() - 5, 5, "_buff") != 0) continue;
    const std::string statBase = key.substr(0, key.size() - 5);
    const int64_t bonus = toInt(br[1]);
    if (bonus <= 0) continue;
    auto it = t.find(statBase);
    if (it != t.end()) it->second += bonus;
  }

  // Buffs/debuffs de skills (active_buffs) — flat antes dos derivados; CC/shield registrados.
  bool ccStunned = false;
  bool ccSilenced = false;
  bool ccRooted = false;
  int32_t totalShield = 0;
  struct SkillPercentMod {
    std::string key;
    int32_t percent;
    bool applyToTotals = false;
  };
  std::vector<SkillPercentMod> skillPercentMods;

  auto applyBuffSnapshot = [&](const nlohmann::json& snap, int stacks) {
    if (!snap.is_object()) return;
    if (snap.value("reaction_armed", false)) return;

    const std::string effectType = snap.value("effect_type", std::string{});
    if (effectType == "STUN") ccStunned = true;
    if (effectType == "SILENCE") ccSilenced = true;
    if (effectType == "ROOT") ccRooted = true;

    const std::string statKey =
        Combat::StatKeyMapping::mapTargetStatToCanonical(snap.value("target_stat", std::string{}));
    const int64_t flat = jsonInt(snap, "value_flat") * stacks;
    const int32_t pct = static_cast<int32_t>(jsonInt(snap, "value_percent")) * stacks;
    if (!statKey.empty() && flat != 0) {
      Combat::StatKeyMapping::applyFlatToTotals(statKey, flat, t);
    }
    if (!statKey.empty() && pct != 0) {
      skillPercentMods.push_back(
          {statKey, pct, Combat::StatKeyMapping::isTotalsPercentKey(statKey)});
    }
  };

  auto skillBuffRows = db_->executePreparedQuery(
      "SELECT buff_type, current_stacks, value_snapshot, COALESCE(snapshot_json,'') "
      "FROM active_buffs WHERE target_player_id = ? AND (expires_at > NOW(3) OR is_permanent = 1)",
      {pid});
  // Proxmox/MySQL 8: prepared pode vir vazio por falha de bind; fallback text (playerId numérico).
  if (skillBuffRows.empty()) {
    skillBuffRows = db_->executeQuery(
        "SELECT buff_type, current_stacks, value_snapshot, COALESCE(snapshot_json,'') "
        "FROM active_buffs WHERE target_player_id = " +
        pid + " AND (expires_at > NOW(3) OR is_permanent = 1)");
  }
  int skillBuffsApplied = 0;
  for (const auto& br : skillBuffRows) {
    if (br.size() < 4) continue;
    int stacks = 1;
    try {
      stacks = std::max(1, std::stoi(br[1]));
    } catch (...) {
    }
    const std::string& buffTypeDb = br[0];
    nlohmann::json snap = nlohmann::json::parse(br[3], nullptr, false);

    if (buffTypeDb == "SHIELD") {
      int32_t shieldVal = 0;
      try {
        shieldVal = std::stoi(br[2]);
      } catch (...) {
      }
      if (shieldVal <= 0 && snap.is_object()) {
        shieldVal = static_cast<int32_t>(jsonInt(snap, "value_flat"));
      }
      totalShield += shieldVal * stacks;
      ++skillBuffsApplied;
      continue;
    }

    applyBuffSnapshot(snap, stacks);
    ++skillBuffsApplied;
  }

  // Passivas aprendidas com condição health_below_percent (sem linha em active_buffs).
  const int64_t healthPct = (health > 0 && baseHealth + levelHp > 0)
                                ? (health * 100 / std::max<int64_t>(1, baseHealth + levelHp))
                                : 100;
  auto passiveRows = db_->executeQuery(
      "SELECT s.effects_json FROM player_skills ps "
      "INNER JOIN skills s ON ps.skill_id = s.skill_id "
      "WHERE ps.player_id = " + pid + " AND s.type_id = 2");
  for (const auto& pr : passiveRows) {
    if (pr.empty() || pr[0].empty() || pr[0] == "null") continue;
    nlohmann::json effects = nlohmann::json::parse(pr[0], nullptr, false);
    if (!effects.is_array()) continue;
    for (const auto& item : effects) {
      if (!item.is_object()) continue;
      nlohmann::json cond = item.value("conditions_json", nlohmann::json::object());
      if (!cond.is_object()) continue;
      const int64_t threshold = jsonInt(cond, "health_below_percent");
      if (threshold <= 0 || healthPct >= threshold) continue;

      nlohmann::json virtualSnap;
      virtualSnap["target_stat"] = item.value("target_stat", std::string{});
      virtualSnap["value_flat"] = item.contains("value_flat") ? item["value_flat"].get<int>() : item.value("value", 0);
      virtualSnap["value_percent"] = item.value("value_percent", 0);
      virtualSnap["effect_type"] = item.value("type", item.value("effect_type", std::string{}));
      applyBuffSnapshot(virtualSnap, 1);
    }
  }

  // Bônus derivados de atributos (mesma ordem/fórmula do helper PHP).
  const int64_t str = t["strength"];
  const int64_t dex = t["dexterity"];
  const int64_t intel = t["intelligence"];
  const int64_t vit = t["vitality"];

  t["attack"] += (str / 5) * 2 + (dex / 10);
  t["magic_attack"] += (intel / 5) * 2;
  t["accuracy"] += (dex / 5);
  t["dodge"] += (dex / 10);
  t["critical"] += (str / 10) + (intel / 10);
  t["resistance"] += (vit / 5);
  t["double_attack_resistance"] += (vit / 10);
  t["double_attack_rate"] += (str / 10);
  t["health_bonus"] += (vit / 10) * 30;
  t["mana_bonus"] += (intel / 10) * 30;

  for (const auto& pm : skillPercentMods) {
    if (pm.applyToTotals) {
      Combat::StatKeyMapping::applyPercentToTotals(pm.key, pm.percent, t);
    }
  }

  const int64_t finalMaxHealth = baseHealth + levelHp + t["health_bonus"];
  const int64_t finalMaxMana = baseMana + levelMp + t["mana_bonus"];

  Combat::CharacterStats stats;
  stats.strength = static_cast<int32_t>(t["strength"]);
  stats.dexterity = static_cast<int32_t>(t["dexterity"]);
  stats.intelligence = static_cast<int32_t>(t["intelligence"]);
  stats.vitality = static_cast<int32_t>(t["vitality"]);
  stats.luck = static_cast<int32_t>(t["luck"]);
  stats.maxHealth = static_cast<int32_t>(std::max<int64_t>(1, finalMaxHealth));
  stats.currentHealth = static_cast<int32_t>(health);
  stats.maxMana = static_cast<int32_t>(std::max<int64_t>(1, finalMaxMana));
  stats.currentMana = static_cast<int32_t>(mana);
  stats.physicalAttack = static_cast<int32_t>(t["attack"]);
  stats.magicAttack = static_cast<int32_t>(t["magic_attack"]);
  stats.physicalDefense = static_cast<int32_t>(t["defense"]);
  stats.magicDefense = static_cast<int32_t>(t["magic_defense"]);
  stats.accuracy = static_cast<int32_t>(t["accuracy"]);
  stats.dodge = static_cast<int32_t>(t["dodge"]);
  stats.criticalChance = static_cast<int32_t>(t["critical"]);
  stats.criticalResistance = static_cast<int32_t>(t["resistance"]);
  stats.doubleAttackRate = static_cast<int32_t>(t["double_attack_rate"]);
  stats.doubleAttackResistance = static_cast<int32_t>(t["double_attack_resistance"]);
  stats.movementSpeed = static_cast<int32_t>(std::max<int64_t>(50, 100 + t["movement"]));

  for (const auto& pm : skillPercentMods) {
    if (!pm.applyToTotals) {
      Combat::StatKeyMapping::applyPercentToCharacterStats(pm.key, pm.percent, stats);
    }
  }

  out = Combat::CharacterState{};
  out.playerId = playerId;
  out.classId = static_cast<uint64_t>(classId);
  out.level = static_cast<int32_t>(level);
  out.baseStats = stats;
  out.buffedStats = stats;
  out.isAlive = (health > 0) && (isDeadFlag == 0);
  out.isPvPEnabled = (pvpFlag != 0);
  out.isStunned = ccStunned;
  out.isSilenced = ccSilenced;
  out.isRooted = ccRooted;
  out.currentShield = totalShield;
  out.maxShield = totalShield;

  // Ranks de todas as skills aprendidas: loadSkillRank passa a ler daqui (cache),
  // eliminando o SELECT current_rank por skill no worker de combate.
  auto rankRows = db_->executeQuery(
      "SELECT skill_id, current_rank FROM player_skills WHERE player_id = " + pid);
  for (const auto& rr : rankRows) {
    if (rr.size() < 2) continue;
    const int64_t sid = toInt(rr[0]);
    if (sid <= 0) continue;
    out.skillRanks[static_cast<uint32_t>(sid)] =
        static_cast<uint8_t>(std::max<int64_t>(1, toInt(rr[1], 1)));
  }

  Core::Logger::getInstance().debug(
      "[CharacterStateLoader] loaded player={} lvl={} class={} str={} physAtk={} magAtk={} "
      "equipRows={} skillBuffs={}",
      playerId, level, classId, stats.strength, stats.physicalAttack, stats.magicAttack,
      equipRows.size(), skillBuffsApplied);
  return true;
}

Combat::CharacterState CharacterStateLoader::makeNpcDefenderState(const NpcRuntimeInstance& inst) {
  Combat::CharacterState s;
  s.playerId = inst.npcInstanceId;
  s.level = static_cast<int32_t>(inst.level);
  s.isAlive = !inst.isDead && inst.currentHealth > 0;

  Combat::CharacterStats stats;
  stats.maxHealth = std::max(1, inst.maxHealth);
  stats.currentHealth = inst.currentHealth;
  stats.maxMana = std::max(1, inst.maxMana);
  stats.currentMana = inst.currentMana;
  stats.physicalDefense = inst.physicalDefense;
  stats.magicDefense = (inst.magicDefense > 0) ? inst.magicDefense : inst.physicalDefense;
  stats.criticalResistance = inst.criticalResistance;
  stats.dodge = inst.dodge;
  stats.doubleAttackResistance = inst.doubleAttackResistance;
  s.baseStats = stats;
  s.buffedStats = stats;
  return s;
}

Combat::CharacterState CharacterStateLoader::makeNpcAttackerState(const NpcRuntimeInstance& inst) {
  Combat::CharacterState s = makeNpcDefenderState(inst);
  s.baseStats.physicalAttack = inst.physicalAttack;
  s.baseStats.magicAttack = inst.magicAttack;
  s.baseStats.accuracy = inst.accuracy;
  s.baseStats.criticalChance = inst.critical;
  s.baseStats.doubleAttackRate = inst.doubleAttackRate;
  s.buffedStats = s.baseStats;
  return s;
}

}  // namespace Zone
}  // namespace Umbra
