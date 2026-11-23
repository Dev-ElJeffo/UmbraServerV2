#include "zone/ConsumableService.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementServer.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>

namespace Umbra {
namespace Zone {

namespace {

constexpr int64_t kElementalScanStepMs = 100;
/** Item da regra não está na bolsa: espera antes de tentar de novo (e avisa uma vez). */
constexpr int64_t kElementalMissingItemBackoffMs = 30000;

long long toLL(const std::string& s, long long def = 0) {
  try {
    return s.empty() ? def : std::stoll(s);
  } catch (...) {
    return def;
  }
}

std::string triggerToString(uint8_t trigger) {
  switch (static_cast<ElementalTrigger>(trigger)) {
    case ElementalTrigger::HpBelow: return "hp_below";
    case ElementalTrigger::MpBelow: return "mp_below";
    case ElementalTrigger::BuffMissing: return "buff_missing";
  }
  return "";
}

uint8_t triggerFromString(const std::string& s) {
  if (s == "hp_below") return static_cast<uint8_t>(ElementalTrigger::HpBelow);
  if (s == "mp_below") return static_cast<uint8_t>(ElementalTrigger::MpBelow);
  if (s == "buff_missing") return static_cast<uint8_t>(ElementalTrigger::BuffMissing);
  return 0;
}

}  // namespace

bool parseConsumableStats(const std::string& statsJson, ConsumableDef& def) {
  def.healthRestore = 0;
  def.manaRestore = 0;
  def.buffKey.clear();
  def.buffValue = 0;
  def.buffDurationMs = 0;
  if (statsJson.empty()) return false;
  auto j = nlohmann::json::parse(statsJson, nullptr, false);
  if (j.is_discarded() || !j.is_object()) return false;
  auto num = [&](const char* key) -> long long {
    auto it = j.find(key);
    if (it == j.end()) return 0;
    if (it->is_number()) return it->get<long long>();
    if (it->is_string()) return toLL(it->get<std::string>());
    return 0;
  };
  def.healthRestore = static_cast<int32_t>(std::max(0LL, num("health_restore")));
  def.manaRestore = static_cast<int32_t>(std::max(0LL, num("mana_restore")));
  const long long durationSec = num("duration");
  // Primeira chave *_buff numérica (mesma regra do use_item.php).
  for (auto it = j.begin(); it != j.end(); ++it) {
    const std::string& key = it.key();
    if (key.size() > 5 && key.compare(key.size() - 5, 5, "_buff") == 0 && it->is_number()) {
      if (durationSec > 0 && it->get<long long>() > 0) {
        def.buffKey = key;
        def.buffValue = static_cast<int32_t>(it->get<long long>());
        def.buffDurationMs = static_cast<uint32_t>(std::min<long long>(durationSec, 86400) * 1000);
      }
      break;
    }
  }
  return def.healthRestore > 0 || def.manaRestore > 0 || !def.buffKey.empty();
}

bool parseElementalRulesJson(const std::string& json, std::vector<ElementalRuleWire>& out) {
  out.clear();
  if (json.empty()) return true;
  auto j = nlohmann::json::parse(json, nullptr, false);
  if (j.is_discarded() || !j.is_array()) return false;
  for (const auto& r : j) {
    if (!r.is_object() || out.size() >= kElementalMaxRulesCap) continue;
    ElementalRuleWire w;
    w.trigger = triggerFromString(r.value("trigger", std::string()));
    w.threshold = static_cast<uint8_t>(std::clamp(r.value("threshold", 0), 0, 99));
    w.itemTemplateId = static_cast<uint32_t>(std::max(0, r.value("item_template_id", 0)));
    if (w.trigger == 0 || w.itemTemplateId == 0) continue;
    out.push_back(w);
  }
  return true;
}

std::string elementalRulesToJson(const std::vector<ElementalRuleWire>& rules) {
  nlohmann::json arr = nlohmann::json::array();
  for (const auto& r : rules) {
    arr.push_back({{"trigger", triggerToString(r.trigger)},
                   {"threshold", static_cast<int>(r.threshold)},
                   {"item_template_id", r.itemTemplateId}});
  }
  return arr.dump();
}

bool elementalRuleMatchesItem(const ElementalRuleWire& rule, const ConsumableDef& def) {
  switch (static_cast<ElementalTrigger>(rule.trigger)) {
    case ElementalTrigger::HpBelow:
      return def.healthRestore > 0 && rule.threshold >= 1 && rule.threshold <= 99;
    case ElementalTrigger::MpBelow:
      return def.manaRestore > 0 && rule.threshold >= 1 && rule.threshold <= 99;
    case ElementalTrigger::BuffMissing:
      return !def.buffKey.empty();
  }
  return false;
}

bool elementalRuleFires(const ElementalRuleWire& rule, int32_t hp, int32_t maxHp, int32_t mp,
                        int32_t maxMp, bool buffActive) {
  if (hp <= 0) return false;  // morto: nada
  switch (static_cast<ElementalTrigger>(rule.trigger)) {
    case ElementalTrigger::HpBelow:
      return maxHp > 0 && static_cast<int64_t>(hp) * 100 < static_cast<int64_t>(maxHp) * rule.threshold;
    case ElementalTrigger::MpBelow:
      return maxMp > 0 && static_cast<int64_t>(mp) * 100 < static_cast<int64_t>(maxMp) * rule.threshold;
    case ElementalTrigger::BuffMissing:
      return !buffActive;
  }
  return false;
}

ConsumableService::ConsumableService(std::shared_ptr<Database::MySQLConnector> db,
                                     MovementServer* movementServer, CombatCoreEngine* combat,
                                     CompanionService* companions)
    : db_(std::move(db)), movementServer_(movementServer), combat_(combat), companions_(companions) {}

ConsumableService::~ConsumableService() { stop(); }

int64_t ConsumableService::nowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void ConsumableService::loadTemplates() {
  if (!db_ || !db_->isConnected()) return;
  std::unordered_map<uint32_t, ConsumableDef> next;
  try {
    // Ordem posicional: conferir os índices ao mudar o SELECT.
    auto rows = db_->executePreparedQuery(
        "SELECT item_id, item_name, COALESCE(item_subtype,''), COALESCE(icon_path,''), "
        "COALESCE(stats_json,''), use_cooldown_ms FROM item_templates "
        "WHERE item_type = 'consumable' AND COALESCE(item_subtype,'') <> 'companion'",
        {});
    for (const auto& r : rows) {
      if (r.size() < 6) continue;
      ConsumableDef d;
      d.itemTemplateId = static_cast<uint32_t>(toLL(r[0]));
      d.name = r[1];
      d.subtype = r[2].empty() ? "consumable" : r[2];
      d.iconPath = r[3];
      d.cooldownMs = static_cast<uint32_t>(std::clamp<long long>(toLL(r[5], 5000), 0, 600000));
      if (d.itemTemplateId == 0 || !parseConsumableStats(r[4], d)) continue;
      next[d.itemTemplateId] = std::move(d);
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn("[ConsumableService] load falhou: {}", e.what());
    return;
  }
  size_t n = 0;
  {
    std::lock_guard<std::mutex> lock(defsMu_);
    defs_ = std::move(next);
    n = defs_.size();
  }
  Core::Logger::getInstance().info("[ConsumableService] {} consumíveis carregados", n);
}

size_t ConsumableService::templateCount() const {
  std::lock_guard<std::mutex> lock(defsMu_);
  return defs_.size();
}

bool ConsumableService::findDef(uint32_t itemTemplateId, ConsumableDef& out) const {
  std::lock_guard<std::mutex> lock(defsMu_);
  auto it = defs_.find(itemTemplateId);
  if (it == defs_.end()) return false;
  out = it->second;
  return true;
}

void ConsumableService::start() {
  if (worker_.joinable()) return;
  stop_ = false;
  worker_ = std::thread([this] { workerLoop(); });
}

void ConsumableService::stop() {
  {
    std::lock_guard<std::mutex> lock(queueMu_);
    stop_ = true;
  }
  queueCv_.notify_all();
  if (worker_.joinable()) worker_.join();
}

void ConsumableService::enqueueUse(uint32_t playerId, uint32_t inventoryId, uint32_t itemTemplateId) {
  if (playerId == 0) return;
  Job job;
  job.kind = Job::Kind::Use;
  job.playerId = playerId;
  job.inventoryId = inventoryId;
  job.itemTemplateId = itemTemplateId;
  {
    std::lock_guard<std::mutex> lock(queueMu_);
    if (queue_.size() >= 512) return;  // anti-spam
    queue_.push_back(std::move(job));
  }
  queueCv_.notify_one();
}

void ConsumableService::enqueueSetRules(uint32_t playerId, const ElementalRulesSetPayload& payload) {
  if (playerId == 0) return;
  Job job;
  job.kind = Job::Kind::SetRules;
  job.playerId = playerId;
  job.rules = payload;
  {
    std::lock_guard<std::mutex> lock(queueMu_);
    if (queue_.size() >= 512) return;
    queue_.push_back(std::move(job));
  }
  queueCv_.notify_one();
}

void ConsumableService::onElementalActivated(uint32_t playerId, const CompanionTemplateDef& tpl,
                                             const std::string& rulesJson) {
  ActiveElemental e;
  e.companionId = tpl.companionId;
  e.name = tpl.name;
  e.maxRules = std::clamp<uint8_t>(tpl.maxRules, 1, kElementalMaxRulesCap);
  e.checkMs = std::clamp<uint32_t>(tpl.autoCheckMs, 250, 5000);
  e.nextCheckAtMs = nowMs() + e.checkMs;
  parseElementalRulesJson(rulesJson, e.rules);
  if (e.rules.size() > e.maxRules) e.rules.resize(e.maxRules);

  ElementalRulesStatePayload state;
  state.status = static_cast<uint8_t>(ElementalRulesStatus::Ok);
  state.companionId = e.companionId;
  state.maxRules = e.maxRules;
  state.rules = e.rules;
  {
    std::lock_guard<std::mutex> lock(elemMu_);
    elementals_[playerId] = std::move(e);
  }
  sendRulesState(playerId, state);
}

void ConsumableService::onElementalDeactivated(uint32_t playerId, uint32_t companionId) {
  std::lock_guard<std::mutex> lock(elemMu_);
  auto it = elementals_.find(playerId);
  if (it != elementals_.end() && it->second.companionId == companionId) {
    elementals_.erase(it);
  }
}

void ConsumableService::onPlayerGone(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(elemMu_);
  elementals_.erase(playerId);
  goneQueue_.insert(playerId);  // o worker limpa players_ (só ele toca nesse mapa)
}

void ConsumableService::workerLoop() {
  int64_t nextScan = nowMs() + kElementalScanStepMs;
  while (true) {
    std::deque<Job> jobs;
    {
      std::unique_lock<std::mutex> lock(queueMu_);
      queueCv_.wait_for(lock, std::chrono::milliseconds(kElementalScanStepMs),
                        [this] { return stop_ || !queue_.empty(); });
      if (stop_) break;
      jobs.swap(queue_);
    }
    {
      std::lock_guard<std::mutex> lock(elemMu_);
      for (uint32_t pid : goneQueue_) players_.erase(pid);
      goneQueue_.clear();
    }
    for (const Job& job : jobs) {
      try {
        processJob(job);
      } catch (const std::exception& e) {
        Core::Logger::getInstance().error("[ConsumableService] job player {} falhou: {}", job.playerId, e.what());
      }
    }
    const int64_t now = nowMs();
    if (now >= nextScan) {
      nextScan = now + kElementalScanStepMs;
      try {
        scanElementals(now);
      } catch (const std::exception& e) {
        Core::Logger::getInstance().error("[ConsumableService] scan do elemental falhou: {}", e.what());
      }
    }
  }
}

void ConsumableService::processJob(const Job& job) {
  if (job.kind == Job::Kind::SetRules) {
    handleSetRules(job.playerId, job.rules);
    return;
  }
  useItem(job.playerId, job.inventoryId, job.itemTemplateId, ConsumableSource::Player);
}

void ConsumableService::ensurePlayerLoaded(uint32_t playerId) {
  PlayerConsumableState& st = players_[playerId];
  if (st.loaded || !db_) return;
  st.loaded = true;
  const std::string pid = std::to_string(playerId);
  const int64_t now = nowMs();
  for (const auto& r : db_->executePreparedQuery(
           "SELECT item_subtype, expires_at_ms FROM player_consumable_cooldowns WHERE player_id = ?",
           {pid})) {
    if (r.size() < 2) continue;
    const int64_t exp = toLL(r[1]);
    if (exp > now) st.cooldownBySubtype[r[0]] = exp;
  }
  for (const auto& r : db_->executePreparedQuery(
           "SELECT buff_key, expires_at_ms FROM player_item_buffs WHERE player_id = ?", {pid})) {
    if (r.size() < 2) continue;
    const int64_t exp = toLL(r[1]);
    if (exp > now) st.buffExpiresByKey[r[0]] = exp;
  }
}

void ConsumableService::sendResult(uint32_t playerId, const ConsumableUseResultPayload& result) {
  if (!movementServer_) return;
  const uint32_t cid = movementServer_->getClientIdForPlayer(playerId);
  if (cid > 0) movementServer_->sendBinaryToClient(cid, encodeConsumableUseResult(result));
}

void ConsumableService::sendRulesState(uint32_t playerId, const ElementalRulesStatePayload& state) {
  if (!movementServer_) return;
  const uint32_t cid = movementServer_->getClientIdForPlayer(playerId);
  if (cid > 0) movementServer_->sendBinaryToClient(cid, encodeElementalRulesState(state));
}

ConsumableUseStatus ConsumableService::useItem(uint32_t playerId, uint32_t inventoryId,
                                               uint32_t itemTemplateId, ConsumableSource source,
                                               std::string* outItemName) {
  ConsumableUseResultPayload fail;
  fail.source = static_cast<uint8_t>(source);
  fail.itemTemplateId = itemTemplateId;
  fail.inventoryId = inventoryId;
  auto reject = [&](ConsumableUseStatus st, const std::string& msg) {
    fail.status = static_cast<uint8_t>(st);
    fail.message = msg;
    // O elemental só avisa falta de item (o worker controla o spam com backoff).
    if (source == ConsumableSource::Player || st == ConsumableUseStatus::NotFound) {
      sendResult(playerId, fail);
    }
    return st;
  };

  if (!db_ || !combat_) return reject(ConsumableUseStatus::Error, "Serviço indisponível.");

  // Vivo? (cache do combate; aquece do DB se faltar — estamos no worker, não no tick)
  Combat::CharacterState st;
  auto* loader = combat_->getCharacterStateLoader();
  if (!loader || !loader->getCachedOrWarm(playerId, st)) {
    return reject(ConsumableUseStatus::Error, "Estado do personagem indisponível.");
  }
  if (!st.isAlive || st.buffedStats.currentHealth <= 0) {
    return reject(ConsumableUseStatus::Dead, "Não é possível usar itens morto.");
  }

  // Pilha na bolsa (nunca equipado nem anunciado no leilão).
  const std::string pid = std::to_string(playerId);
  std::vector<std::vector<std::string>> rows;
  if (inventoryId > 0) {
    rows = db_->executePreparedQuery(
        "SELECT inventory_id, item_template_id, quantity FROM player_inventory "
        "WHERE player_id = ? AND inventory_id = ? AND COALESCE(is_equipped,0) = 0 "
        "AND auction_listing_id IS NULL AND quantity > 0 LIMIT 1",
        {pid, std::to_string(inventoryId)});
  } else {
    rows = db_->executePreparedQuery(
        "SELECT inventory_id, item_template_id, quantity FROM player_inventory "
        "WHERE player_id = ? AND item_template_id = ? AND COALESCE(is_equipped,0) = 0 "
        "AND auction_listing_id IS NULL AND quantity > 0 ORDER BY slot_index LIMIT 1",
        {pid, std::to_string(itemTemplateId)});
  }
  ConsumableDef def;
  const bool haveDef = findDef(itemTemplateId, def);
  if (rows.empty() || rows[0].size() < 3) {
    const std::string itemName = haveDef ? def.name : "o item";
    return reject(ConsumableUseStatus::NotFound, "Sem " + itemName + " na bolsa.");
  }
  const uint32_t invId = static_cast<uint32_t>(toLL(rows[0][0]));
  const uint32_t tplId = static_cast<uint32_t>(toLL(rows[0][1]));
  const long long qty = toLL(rows[0][2]);
  fail.inventoryId = invId;
  fail.itemTemplateId = tplId;
  if (!findDef(tplId, def)) {
    return reject(ConsumableUseStatus::Invalid, "Este item não pode ser usado.");
  }
  if (outItemName) *outItemName = def.name;

  // Cooldown por subtipo (compartilhado entre itens do mesmo subtipo, como no PHP).
  ensurePlayerLoaded(playerId);
  PlayerConsumableState& ps = players_[playerId];
  const int64_t now = nowMs();
  auto cdIt = ps.cooldownBySubtype.find(def.subtype);
  if (cdIt != ps.cooldownBySubtype.end() && cdIt->second > now) {
    fail.subtype = def.subtype;
    fail.cooldownExpiresAtMs = cdIt->second;
    fail.cooldownTotalMs = def.cooldownMs;
    const int64_t secs = (cdIt->second - now + 999) / 1000;
    fail.status = static_cast<uint8_t>(ConsumableUseStatus::Cooldown);
    fail.message = "Em recarga. Aguarde " + std::to_string(secs) + "s.";
    if (source == ConsumableSource::Player) sendResult(playerId, fail);
    return ConsumableUseStatus::Cooldown;
  }

  // Consome 1 (condicional em quantity > 0: dois usos simultâneos não ficam negativos).
  bool consumed = false;
  if (qty <= 1) {
    consumed = db_->executePreparedInsert(
        "DELETE FROM player_inventory WHERE inventory_id = ? AND player_id = ? AND quantity <= 1",
        {std::to_string(invId), pid});
  } else {
    consumed = db_->executePreparedInsert(
        "UPDATE player_inventory SET quantity = quantity - 1 WHERE inventory_id = ? AND player_id = ? "
        "AND quantity > 1",
        {std::to_string(invId), pid});
  }
  if (!consumed) {
    return reject(ConsumableUseStatus::Error, "Falha ao usar o item.");
  }

  const int64_t cdExpires = now + def.cooldownMs;
  ps.cooldownBySubtype[def.subtype] = cdExpires;
  db_->executePreparedInsert(
      "INSERT INTO player_consumable_cooldowns (player_id, item_subtype, expires_at_ms) VALUES (?,?,?) "
      "ON DUPLICATE KEY UPDATE expires_at_ms = VALUES(expires_at_ms), updated_at = NOW()",
      {pid, def.subtype, std::to_string(cdExpires)});

  int64_t buffExpires = 0;
  if (!def.buffKey.empty()) {
    buffExpires = now + def.buffDurationMs;
    ps.buffExpiresByKey[def.buffKey] = buffExpires;
    db_->executePreparedInsert(
        "INSERT INTO player_item_buffs (player_id, buff_key, item_template_id, item_subtype, bonus_value, "
        "duration_ms, started_at_ms, expires_at_ms) VALUES (?,?,?,?,?,?,?,?) "
        "ON DUPLICATE KEY UPDATE bonus_value = VALUES(bonus_value), duration_ms = VALUES(duration_ms), "
        "started_at_ms = VALUES(started_at_ms), expires_at_ms = VALUES(expires_at_ms), "
        "item_template_id = VALUES(item_template_id), item_subtype = VALUES(item_subtype)",
        {pid, def.buffKey, std::to_string(tplId), def.subtype, std::to_string(def.buffValue),
         std::to_string(def.buffDurationMs), std::to_string(now), std::to_string(buffExpires)});
  }

  // Vida/mana/buff no combat worker (mesma ordem de dano e regen); ele manda 87/95/85 e o 128.
  CombatCoreEngine::ConsumableEffectJob job;
  job.playerId = playerId;
  job.source = static_cast<uint8_t>(source);
  job.itemTemplateId = tplId;
  job.inventoryId = invId;
  job.quantityRemaining = static_cast<uint32_t>(std::max(0LL, qty - 1));
  job.subtype = def.subtype;
  job.itemName = def.name;
  job.iconPath = def.iconPath;
  job.healthRestore = def.healthRestore;
  job.manaRestore = def.manaRestore;
  job.buffKey = def.buffKey;
  job.buffValue = def.buffValue;
  job.buffDurationMs = def.buffDurationMs;
  job.buffExpiresAtMs = buffExpires;
  job.cooldownExpiresAtMs = cdExpires;
  job.cooldownTotalMs = def.cooldownMs;
  combat_->enqueueConsumableEffect(job);

  // Quests v2: use_consumable (jogador ou elemental, os dois contam).
  if (auto* quests = combat_->getQuestProgressService()) {
    QuestEvent ev;
    ev.kind = QuestObjectiveKind::UseConsumable;
    ev.key = tplId;
    ev.subtype = def.subtype;
    quests->onEvent(playerId, ev);
  }

  Core::Logger::getInstance().info("[ConsumableService] player {} usou {} ({}) fonte={} restam={}", playerId,
                                   tplId, def.name, static_cast<int>(source), job.quantityRemaining);
  return ConsumableUseStatus::Ok;
}

void ConsumableService::scanElementals(int64_t now) {
  struct Due {
    uint32_t playerId;
    ActiveElemental e;
  };
  std::vector<Due> due;
  {
    std::lock_guard<std::mutex> lock(elemMu_);
    for (auto& [pid, e] : elementals_) {
      if (e.rules.empty() || now < e.nextCheckAtMs) continue;
      e.nextCheckAtMs = now + e.checkMs;
      due.push_back({pid, e});
    }
  }
  if (due.empty() || !combat_) return;
  auto* loader = combat_->getCharacterStateLoader();
  if (!loader) return;

  for (Due& d : due) {
    // Só cache (sem DB): sem estado em cache, espera o combate aquecer.
    Combat::CharacterState st;
    if (!loader->tryGetCachedState(d.playerId, st)) continue;
    const int32_t hp = st.buffedStats.currentHealth;
    if (!st.isAlive || hp <= 0) continue;
    ensurePlayerLoaded(d.playerId);
    PlayerConsumableState& ps = players_[d.playerId];

    for (size_t i = 0; i < d.e.rules.size(); ++i) {
      const ElementalRuleWire& rule = d.e.rules[i];
      if (d.e.backoffUntilMs[i] > now) continue;
      ConsumableDef def;
      if (!findDef(rule.itemTemplateId, def) || !elementalRuleMatchesItem(rule, def)) continue;
      const auto cdIt = ps.cooldownBySubtype.find(def.subtype);
      if (cdIt != ps.cooldownBySubtype.end() && cdIt->second > now) continue;
      bool buffActive = false;
      if (!def.buffKey.empty()) {
        const auto bIt = ps.buffExpiresByKey.find(def.buffKey);
        buffActive = bIt != ps.buffExpiresByKey.end() && bIt->second > now;
      }
      if (!elementalRuleFires(rule, hp, st.buffedStats.maxHealth, st.buffedStats.currentMana,
                              st.buffedStats.maxMana, buffActive)) {
        continue;
      }
      const ConsumableUseStatus status = useItem(d.playerId, 0, rule.itemTemplateId, ConsumableSource::Elemental);
      if (status == ConsumableUseStatus::NotFound) {
        std::lock_guard<std::mutex> lock(elemMu_);
        auto it = elementals_.find(d.playerId);
        if (it != elementals_.end() && it->second.companionId == d.e.companionId) {
          it->second.backoffUntilMs[i] = now + kElementalMissingItemBackoffMs;
        }
      }
      if (status == ConsumableUseStatus::Ok) break;  // uma poção por checagem
    }
  }
}

void ConsumableService::handleSetRules(uint32_t playerId, const ElementalRulesSetPayload& payload) {
  ElementalRulesStatePayload state;
  state.companionId = payload.companionId;
  CompanionTemplateDef tpl;
  if (!companions_ || !companions_->getTemplate(payload.companionId, tpl) || tpl.slot != CompanionSlot::Elemental) {
    state.status = static_cast<uint8_t>(ElementalRulesStatus::Invalid);
    sendRulesState(playerId, state);
    return;
  }
  state.maxRules = std::clamp<uint8_t>(tpl.maxRules, 1, kElementalMaxRulesCap);

  auto owned = db_->executePreparedQuery(
      "SELECT COALESCE(auto_rules_json,'') FROM player_companions WHERE player_id = ? AND companion_id = ? "
      "AND (expires_at IS NULL OR expires_at > NOW()) LIMIT 1",
      {std::to_string(playerId), std::to_string(payload.companionId)});
  if (owned.empty()) {
    state.status = static_cast<uint8_t>(ElementalRulesStatus::NotOwned);
    sendRulesState(playerId, state);
    return;
  }

  bool valid = payload.rules.size() <= state.maxRules;
  std::vector<ElementalRuleWire> clean;
  for (const auto& r : payload.rules) {
    ConsumableDef def;
    if (!valid) break;
    ElementalRuleWire w = r;
    if (static_cast<ElementalTrigger>(w.trigger) == ElementalTrigger::BuffMissing) w.threshold = 0;
    if (!findDef(w.itemTemplateId, def) || !elementalRuleMatchesItem(w, def)) {
      valid = false;
      break;
    }
    clean.push_back(w);
  }
  if (!valid) {
    // Devolve o que está gravado para a janela ressincronizar.
    parseElementalRulesJson(owned[0].empty() ? std::string() : owned[0][0], state.rules);
    state.status = static_cast<uint8_t>(ElementalRulesStatus::Invalid);
    sendRulesState(playerId, state);
    return;
  }

  if (!db_->executePreparedInsert(
          "UPDATE player_companions SET auto_rules_json = ? WHERE player_id = ? AND companion_id = ?",
          {elementalRulesToJson(clean), std::to_string(playerId), std::to_string(payload.companionId)})) {
    state.status = static_cast<uint8_t>(ElementalRulesStatus::Error);
    sendRulesState(playerId, state);
    return;
  }
  {
    std::lock_guard<std::mutex> lock(elemMu_);
    auto it = elementals_.find(playerId);
    if (it != elementals_.end() && it->second.companionId == payload.companionId) {
      it->second.rules = clean;
      for (auto& b : it->second.backoffUntilMs) b = 0;
    }
  }
  state.status = static_cast<uint8_t>(ElementalRulesStatus::Ok);
  state.rules = clean;
  sendRulesState(playerId, state);
  Core::Logger::getInstance().info("[ConsumableService] player {} gravou {} regra(s) no elemental {}", playerId,
                                   clean.size(), payload.companionId);
}

}  // namespace Zone
}  // namespace Umbra
