// InstanceManager.cpp — instâncias lógicas (etapa 4, opcodes 134-136). Ver InstanceManager.hpp.
#include "zone/InstanceManager.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>

#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/DungeonService.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"

namespace Umbra {
namespace Zone {

namespace {

float toFloat(const std::string& s, float def) {
  if (s.empty()) return def;
  try {
    return std::stof(s);
  } catch (...) {
    return def;
  }
}

uint32_t toU32(const std::string& s, uint32_t def) {
  if (s.empty()) return def;
  try {
    return static_cast<uint32_t>(std::stoul(s));
  } catch (...) {
    return def;
  }
}

/** Folga além do interaction_radius do NPC portal (rede/latência). */
constexpr float kPortalRangeSlack = 400.f;

}  // namespace

// ---------------- Funções puras ----------------

uint32_t allocateInstanceSlot(const std::vector<uint32_t>& usedSlots, uint32_t maxCopies) {
  for (uint32_t slot = 0; slot < maxCopies; ++slot) {
    if (std::find(usedSlots.begin(), usedSlots.end(), slot) == usedSlots.end()) return slot;
  }
  return kNoInstanceSlot;
}

void computeInstanceCopyOrigin(const InstanceTemplateDef& tpl, uint32_t slot, float& outX, float& outY,
                               float& outZ) {
  outX = tpl.originX;
  outY = tpl.originY + static_cast<float>(slot) * tpl.slotSpacing;
  outZ = tpl.originZ;
}

bool shouldCloseInstance(bool isEmpty, int64_t emptyForMs, uint32_t emptyCloseSeconds, bool timedOut) {
  if (timedOut) return true;
  if (!isEmpty) return false;
  return emptyForMs >= static_cast<int64_t>(emptyCloseSeconds) * 1000;
}

void clampInstanceTemplate(InstanceTemplateDef& tpl) {
  if (tpl.slotSpacing < 20000.f) tpl.slotSpacing = 200000.f;
  if (tpl.minLevel == 0) tpl.minLevel = 1;
  if (tpl.maxPlayers == 0) tpl.maxPlayers = 5;
  tpl.maxPlayers = std::min<uint32_t>(tpl.maxPlayers, 40);
  tpl.timeLimitMinutes = std::min<uint32_t>(tpl.timeLimitMinutes, 1440);
  if (tpl.emptyCloseSeconds < 10) tpl.emptyCloseSeconds = 120;
  tpl.emptyCloseSeconds = std::min<uint32_t>(tpl.emptyCloseSeconds, 3600);
  if (tpl.maxCopies == 0) tpl.maxCopies = 20;
  tpl.maxCopies = std::min<uint32_t>(tpl.maxCopies, 200);
}

// ---------------- InstanceManager ----------------

InstanceManager::InstanceManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId,
                                 MovementServer* movement, CombatCoreEngine* combat)
    : db_(std::move(db)), zoneId_(zoneId), movement_(movement), combat_(combat) {}

size_t InstanceManager::reload() {
  if (!db_ || !db_->isConnected()) return 0;
  if (db_->executeQuery("SHOW TABLES LIKE 'instance_templates'").empty()) {
    Core::Logger::getInstance().warn(
        "[InstanceManager] tabela instance_templates ausente: rode www/umbra_api/scripts/create_instances.sql");
    return 0;
  }
  auto rows = db_->executePreparedQuery(
      "SELECT instance_template_id, code, name, map_path, origin_x, origin_y, origin_z, slot_spacing, "
      "entry_x, entry_y, entry_z, entry_yaw, min_level, max_players, party_required, time_limit_minutes, "
      "empty_close_seconds, max_copies, is_active, description "
      "FROM instance_templates WHERE zone_id = ? OR zone_id = 0",
      {std::to_string(zoneId_)});

  std::unordered_map<uint32_t, InstanceTemplateDef> loaded;
  for (const auto& r : rows) {
    if (r.size() < 20) continue;
    InstanceTemplateDef t;
    t.id = toU32(r[0], 0);
    if (t.id == 0) continue;
    t.code = r[1];
    t.name = r[2];
    t.mapPath = r[3];
    t.originX = toFloat(r[4], 0.f);
    t.originY = toFloat(r[5], 0.f);
    t.originZ = toFloat(r[6], 0.f);
    t.slotSpacing = toFloat(r[7], 200000.f);
    t.entryX = toFloat(r[8], 0.f);
    t.entryY = toFloat(r[9], 0.f);
    t.entryZ = toFloat(r[10], 0.f);
    t.entryYaw = toFloat(r[11], 0.f);
    t.minLevel = toU32(r[12], 1);
    t.maxPlayers = toU32(r[13], 5);
    t.partyRequired = toU32(r[14], 0) != 0;
    t.timeLimitMinutes = toU32(r[15], 0);
    t.emptyCloseSeconds = toU32(r[16], 120);
    t.maxCopies = toU32(r[17], 20);
    t.isActive = toU32(r[18], 1) != 0;
    t.description = r[19];
    clampInstanceTemplate(t);
    loaded[t.id] = std::move(t);
  }

  // Etapa 5: regras de dungeon (add_dungeons.sql). Query própria: colunas opcionais.
  if (!db_->executeQuery("SHOW COLUMNS FROM instance_templates LIKE 'is_dungeon'").empty()) {
    auto drows = db_->executePreparedQuery(
        "SELECT instance_template_id, is_dungeon, completion_mode, reward_exp, reward_gold, lockout_reset, "
        "completion_close_seconds FROM instance_templates WHERE zone_id = ? OR zone_id = 0",
        {std::to_string(zoneId_)});
    for (const auto& r : drows) {
      if (r.size() < 7) continue;
      auto it = loaded.find(toU32(r[0], 0));
      if (it == loaded.end()) continue;
      InstanceTemplateDef& t = it->second;
      t.isDungeon = toU32(r[1], 0) != 0;
      t.completionMode = r[2].empty() ? std::string("final_boss") : r[2];
      t.rewardExp = toU32(r[3], 0);
      t.rewardGold = toU32(r[4], 0);
      t.lockoutReset = r[5].empty() ? std::string("daily") : r[5];
      t.completionCloseSeconds = std::clamp<uint32_t>(toU32(r[6], 300), 10, 3600);
    }
    auto frows = db_->executeQuery(
        "SELECT instance_template_id, difficulty_key, name, min_level, hp_mult, damage_mult, reward_mult "
        "FROM dungeon_difficulties WHERE is_active = 1 ORDER BY instance_template_id, sort_order, difficulty_id");
    for (const auto& r : frows) {
      if (r.size() < 7) continue;
      auto it = loaded.find(toU32(r[0], 0));
      if (it == loaded.end() || r[1].empty()) continue;
      InstanceTemplateDef::Difficulty d;
      d.key = r[1];
      d.name = r[2].empty() ? r[1] : r[2];
      d.minLevel = std::max(1u, toU32(r[3], 1));
      d.hpMult = std::clamp(toFloat(r[4], 1.f), 0.1f, 100.f);
      d.damageMult = std::clamp(toFloat(r[5], 1.f), 0.1f, 100.f);
      d.rewardMult = std::clamp(toFloat(r[6], 1.f), 0.f, 100.f);
      it->second.difficulties.push_back(std::move(d));
    }
  }
  const size_t moldeNpcs = combat_ ? combat_->reloadInstanceMoldes() : 0;
  size_t count = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    templates_ = std::move(loaded);
    count = templates_.size();
  }
  Core::Logger::getInstance().info("[InstanceManager] {} molde(s) de instância, {} NPC(s) de molde (zone {})",
                                   count, moldeNpcs, zoneId_);
  return count;
}

void InstanceManager::refuse(uint32_t playerId, uint32_t templateId, const std::string& reason) {
  if (!movement_) return;
  InstanceStatePayload p;
  p.state = static_cast<uint8_t>(InstanceStateKind::Refused);
  p.templateId = templateId;
  p.reason = reason;
  movement_->sendToPlayer(playerId, encodeInstanceState(p));
}

uint32_t InstanceManager::playerLevel(uint32_t playerId) {
  if (combat_) {
    if (auto* loader = combat_->getCharacterStateLoader()) {
      Combat::CharacterState st;
      if (loader->tryGetCachedState(playerId, st) && st.level > 0) return static_cast<uint32_t>(st.level);
    }
  }
  if (db_ && db_->isConnected()) {
    auto v = db_->executePreparedScalar("SELECT level FROM players WHERE id = ? LIMIT 1",
                                        {std::to_string(playerId)});
    if (v && !v->empty()) return toU32(*v, 1);
  }
  return 1;
}

bool lobbyAllReady(const std::vector<uint32_t>& members, const std::unordered_map<uint32_t, bool>& ready,
                   const std::unordered_map<uint32_t, uint32_t>& levels, uint32_t minLevel) {
  if (members.empty()) return false;
  for (uint32_t m : members) {
    auto r = ready.find(m);
    if (r == ready.end() || !r->second) return false;
    auto l = levels.find(m);
    if (l == levels.end() || l->second < minLevel) return false;
  }
  return true;
}

InstanceTemplateDef::Difficulty instanceDifficultyAt(const InstanceTemplateDef& tpl, size_t index) {
  if (index < tpl.difficulties.size()) return tpl.difficulties[index];
  if (!tpl.difficulties.empty()) return tpl.difficulties.front();
  InstanceTemplateDef::Difficulty d;
  d.key = "normal";
  d.name = "Normal";
  return d;
}

uint32_t InstanceManager::lobbyMinLevelLocked(const Lobby& lobby) const {
  auto it = templates_.find(lobby.templateId);
  if (it == templates_.end()) return 1;
  return std::max(it->second.minLevel, instanceDifficultyAt(it->second, lobby.difficultyIndex).minLevel);
}

void InstanceManager::handleLobbyAction(uint32_t playerId, uint32_t portalNpcInstanceId, uint8_t action,
                                        uint8_t value) {
  if (playerId == 0) return;
  switch (static_cast<InstanceLobbyAction>(action)) {
    case InstanceLobbyAction::SetDifficulty:
      setDifficulty(playerId, value);
      break;
    case InstanceLobbyAction::Open:
      openLobby(playerId, portalNpcInstanceId);
      break;
    case InstanceLobbyAction::Ready:
      setReady(playerId, true);
      break;
    case InstanceLobbyAction::Unready:
      setReady(playerId, false);
      break;
    case InstanceLobbyAction::Cancel:
      cancelLobby(playerId);
      break;
  }
}

InstanceManager::ActiveInstance* InstanceManager::findPartyInstanceLocked(uint32_t templateId, uint32_t playerId,
                                                                          const std::vector<uint32_t>& party) {
  for (auto& [id, inst] : active_) {
    if (inst.templateId != templateId) continue;
    if (inst.ownerPlayerId == playerId) return &inst;
    for (uint32_t m : party) {
      if (m != playerId && (inst.ownerPlayerId == m || inst.members.count(m) > 0)) return &inst;
    }
  }
  return nullptr;
}

void InstanceManager::openLobby(uint32_t playerId, uint32_t portalNpcInstanceId) {
  if (!movement_ || !combat_) return;

  NpcManager::PortalInfo portal;
  if (!combat_->getNpcPortalInfo(portalNpcInstanceId, portal)) {
    refuse(playerId, 0, "Portal inválido.");
    return;
  }
  float px = 0.f, py = 0.f, pz = 0.f;
  if (!movement_->tryGetPlayerPosition(playerId, px, py, pz)) return;
  if (movement_->getPlayerLayer(playerId) != portal.layer) {
    refuse(playerId, portal.instanceTemplateId, "Portal inválido.");
    return;
  }
  const float dx = px - portal.x;
  const float dy = py - portal.y;
  const float maxDist = portal.interactionRadius + kPortalRangeSlack;
  if (dx * dx + dy * dy > maxDist * maxDist) {
    refuse(playerId, portal.instanceTemplateId, "Você está longe do portal.");
    return;
  }

  // Fora do lock: DB (grupo/nível) e MovementServer.
  std::vector<uint32_t> party = resolvePartyMembers_ ? resolvePartyMembers_(playerId) : std::vector<uint32_t>{};
  if (std::find(party.begin(), party.end(), playerId) == party.end()) party.insert(party.begin(), playerId);
  // Candidatos: quem abriu + membros do grupo online nesta zone, no mundo aberto (mesma camada do portal).
  std::vector<uint32_t> candidates;
  candidates.push_back(playerId);
  for (uint32_t m : party) {
    if (m == playerId) continue;
    if (!movement_->isPlayerConnected(m)) continue;
    if (movement_->getPlayerLayer(m) != portal.layer) continue;
    candidates.push_back(m);
  }
  std::unordered_map<uint32_t, uint32_t> levels;
  std::unordered_map<uint32_t, std::string> names;
  for (uint32_t m : candidates) {
    levels[m] = playerLevel(m);
    std::string n = movement_->getPlayerName(m);
    names[m] = n.empty() ? ("Jogador " + std::to_string(m)) : n;
  }
  // Etapa 5: lockout de cada candidato (DB, fora do lock).
  bool tplIsDungeon = false;
  std::string tplLockout;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto t = templates_.find(portal.instanceTemplateId);
    if (t != templates_.end()) {
      tplIsDungeon = t->second.isDungeon;
      tplLockout = t->second.lockoutReset;
    }
  }
  std::unordered_map<uint32_t, std::vector<std::string>> locked;
  if (tplIsDungeon && dungeonService_ && tplLockout != "none") {
    for (uint32_t m : candidates) {
      locked[m] = dungeonService_->lockedDifficulties(m, portal.instanceTemplateId, tplLockout);
    }
  }

  std::lock_guard<std::mutex> lock(mu_);
  if (instanceByPlayer_.count(playerId) > 0) {
    refuse(playerId, portal.instanceTemplateId, "Você já está em uma instância.");
    return;
  }
  // Já está numa sala: reabre o painel (mesmo molde) ou recusa.
  auto existingLobby = lobbyByPlayer_.find(playerId);
  if (existingLobby != lobbyByPlayer_.end()) {
    auto lIt = lobbies_.find(existingLobby->second);
    if (lIt != lobbies_.end()) {
      if (lIt->second.templateId == portal.instanceTemplateId) {
        broadcastLobbyLocked(lIt->second, std::string());
      } else {
        refuse(playerId, portal.instanceTemplateId, "Você já está na sala de outra instância.");
      }
      return;
    }
    lobbyByPlayer_.erase(existingLobby);
  }
  auto tplIt = templates_.find(portal.instanceTemplateId);
  if (tplIt == templates_.end() || !tplIt->second.isActive || tplIt->second.mapPath.empty()) {
    refuse(playerId, portal.instanceTemplateId, "Instância indisponível no momento.");
    return;
  }
  const InstanceTemplateDef& tpl = tplIt->second;
  if (levels[playerId] < tpl.minLevel) {
    refuse(playerId, tpl.id, "Nível mínimo para entrar: " + std::to_string(tpl.minLevel) + ".");
    return;
  }
  if (tpl.partyRequired && party.size() < 2) {
    refuse(playerId, tpl.id, "Esta instância exige um grupo.");
    return;
  }

  Lobby lobby;
  lobby.lobbyId = nextLobbyId_++;
  if (nextLobbyId_ == 0) nextLobbyId_ = 1;
  lobby.templateId = tpl.id;
  lobby.openerPlayerId = playerId;
  lobby.deadline = std::chrono::steady_clock::now() + std::chrono::seconds(lobbyTimeoutSeconds_);

  if (ActiveInstance* existing = findPartyInstanceLocked(tpl.id, playerId, party)) {
    // O grupo já está numa cópia: sala só de quem chegou; ao confirmar entra direto.
    if (existing->members.size() >= tpl.maxPlayers) {
      refuse(playerId, tpl.id, "A instância do seu grupo está cheia.");
      return;
    }
    lobby.existingInstanceId = existing->instanceId;
    // A dificuldade é a da cópia do grupo (não pode trocar).
    for (size_t i = 0; i < tpl.difficulties.size(); ++i) {
      if (tpl.difficulties[i].key == existing->difficultyKey) lobby.difficultyIndex = i;
    }
    candidates.assign(1, playerId);
  } else {
    // Quem já está em outra sala fica de fora desta.
    candidates.erase(std::remove_if(candidates.begin() + 1, candidates.end(),
                                    [&](uint32_t m) { return lobbyByPlayer_.count(m) > 0 || instanceByPlayer_.count(m) > 0; }),
                     candidates.end());
    if (candidates.size() > tpl.maxPlayers) {
      refuse(playerId, tpl.id, "Seu grupo tem mais jogadores que o máximo desta instância (" +
                                   std::to_string(tpl.maxPlayers) + ").");
      return;
    }
  }
  for (uint32_t m : candidates) {
    lobby.members.push_back(m);
    lobby.names[m] = names[m];
    lobby.levels[m] = levels[m];
    lobby.ready[m] = false;
    lobby.lockedDifficulties[m] = locked[m];
    lobbyByPlayer_[m] = lobby.lobbyId;
  }
  const uint32_t lobbyId = lobby.lobbyId;
  auto& stored = lobbies_.emplace(lobbyId, std::move(lobby)).first->second;
  const std::string info =
      stored.members.size() > 1 ? names[playerId] + " abriu a sala de " + tpl.name + "." : std::string();
  broadcastLobbyLocked(stored, info);
  Core::Logger::getInstance().info("[InstanceManager] sala {} aberta por player {} (molde {}, {} membro(s){})",
                                   lobbyId, playerId, tpl.id, stored.members.size(),
                                   stored.existingInstanceId ? ", cópia existente" : "");
}

void InstanceManager::broadcastLobbyLocked(const Lobby& lobby, const std::string& info) {
  if (!movement_) return;
  auto tplIt = templates_.find(lobby.templateId);
  if (tplIt == templates_.end()) return;
  const InstanceTemplateDef& tpl = tplIt->second;
  const uint32_t minLevel = lobbyMinLevelLocked(lobby);
  const auto diff = instanceDifficultyAt(tpl, lobby.difficultyIndex);
  InstanceLobbyStatePayload p;
  p.state = static_cast<uint8_t>(InstanceLobbyStateKind::Open);
  p.lobbyId = lobby.lobbyId;
  p.templateId = tpl.id;
  p.minLevel = static_cast<uint16_t>(std::min<uint32_t>(minLevel, 0xFFFF));
  p.maxPlayers = static_cast<uint8_t>(std::min<uint32_t>(tpl.maxPlayers, 255));
  p.partyRequired = tpl.partyRequired ? 1 : 0;
  p.timeLimitMinutes = static_cast<uint16_t>(tpl.timeLimitMinutes);
  const auto left = std::chrono::duration_cast<std::chrono::seconds>(lobby.deadline - std::chrono::steady_clock::now());
  p.expiresInSeconds = static_cast<uint16_t>(std::clamp<int64_t>(left.count(), 0, 0xFFFF));
  p.joinExisting = lobby.existingInstanceId > 0 ? 1 : 0;
  p.name = tpl.name;
  p.description = tpl.description;
  p.reason = info;
  for (uint32_t m : lobby.members) {
    InstanceLobbyMemberPayload mp;
    mp.playerId = m;
    auto lv = lobby.levels.find(m);
    const uint32_t level = lv != lobby.levels.end() ? lv->second : 1;
    mp.level = static_cast<uint16_t>(std::min<uint32_t>(level, 0xFFFF));
    auto rd = lobby.ready.find(m);
    mp.ready = (rd != lobby.ready.end() && rd->second) ? 1 : 0;
    mp.flags = level < minLevel ? 0x01 : 0x00;
    // bit1: já recebeu a recompensa desta dificuldade no período (entra, mas sem recompensa).
    auto lk = lobby.lockedDifficulties.find(m);
    if (tpl.isDungeon && lk != lobby.lockedDifficulties.end() &&
        std::find(lk->second.begin(), lk->second.end(), diff.key) != lk->second.end()) {
      mp.flags |= 0x02;
    }
    auto nm = lobby.names.find(m);
    mp.name = nm != lobby.names.end() ? nm->second : std::string();
    p.members.push_back(std::move(mp));
  }
  // Trailing (etapa 5).
  p.isDungeon = tpl.isDungeon ? 1 : 0;
  p.leaderPlayerId = lobby.existingInstanceId > 0 ? 0 : lobby.openerPlayerId;
  p.selectedDifficulty = static_cast<uint8_t>(std::min<size_t>(lobby.difficultyIndex, 255));
  if (tpl.isDungeon) {
    for (const auto& d : tpl.difficulties) {
      InstanceLobbyStatePayload::Difficulty pd;
      pd.key = d.key;
      pd.name = d.name;
      pd.minLevel = static_cast<uint16_t>(std::min<uint32_t>(std::max(tpl.minLevel, d.minLevel), 0xFFFF));
      p.difficulties.push_back(std::move(pd));
    }
  }
  const auto frame = encodeInstanceLobbyState(p);
  for (uint32_t m : lobby.members) movement_->sendToPlayer(m, frame);
}

void InstanceManager::closeLobbyLocked(uint32_t lobbyId, const std::string& reason) {
  auto it = lobbies_.find(lobbyId);
  if (it == lobbies_.end()) return;
  Lobby lobby = std::move(it->second);
  lobbies_.erase(it);
  for (uint32_t m : lobby.members) {
    auto b = lobbyByPlayer_.find(m);
    if (b != lobbyByPlayer_.end() && b->second == lobbyId) lobbyByPlayer_.erase(b);
  }
  if (!movement_) return;
  InstanceLobbyStatePayload p;
  p.state = static_cast<uint8_t>(InstanceLobbyStateKind::Closed);
  p.lobbyId = lobbyId;
  p.templateId = lobby.templateId;
  p.reason = reason;
  const auto frame = encodeInstanceLobbyState(p);
  for (uint32_t m : lobby.members) movement_->sendToPlayer(m, frame);
}

void InstanceManager::setReady(uint32_t playerId, bool ready) {
  std::lock_guard<std::mutex> lock(mu_);
  auto b = lobbyByPlayer_.find(playerId);
  if (b == lobbyByPlayer_.end()) return;
  auto it = lobbies_.find(b->second);
  if (it == lobbies_.end()) {
    lobbyByPlayer_.erase(b);
    return;
  }
  Lobby& lobby = it->second;
  auto tplIt = templates_.find(lobby.templateId);
  if (tplIt == templates_.end()) {
    closeLobbyLocked(lobby.lobbyId, "Instância indisponível no momento.");
    return;
  }
  const uint32_t minLevel = lobbyMinLevelLocked(lobby);
  if (ready && lobby.levels[playerId] < minLevel) {
    refuse(playerId, lobby.templateId, "Nível mínimo para entrar: " + std::to_string(minLevel) + ".");
    return;
  }
  lobby.ready[playerId] = ready;
  if (lobbyAllReady(lobby.members, lobby.ready, lobby.levels, minLevel)) {
    launchLobbyLocked(lobby.lobbyId);
    return;
  }
  broadcastLobbyLocked(lobby, std::string());
}

void InstanceManager::setDifficulty(uint32_t playerId, uint8_t index) {
  std::lock_guard<std::mutex> lock(mu_);
  auto b = lobbyByPlayer_.find(playerId);
  if (b == lobbyByPlayer_.end()) return;
  auto it = lobbies_.find(b->second);
  if (it == lobbies_.end()) return;
  Lobby& lobby = it->second;
  auto tplIt = templates_.find(lobby.templateId);
  if (tplIt == templates_.end() || !tplIt->second.isDungeon) return;
  // Só quem abriu, numa sala nova (cópia existente mantém a dificuldade dela).
  if (lobby.openerPlayerId != playerId || lobby.existingInstanceId > 0) {
    refuse(playerId, lobby.templateId, "Só quem abriu a sala escolhe a dificuldade.");
    return;
  }
  if (index >= tplIt->second.difficulties.size() || index == lobby.difficultyIndex) return;
  lobby.difficultyIndex = index;
  for (auto& [pid, r] : lobby.ready) r = false;  // nova dificuldade: todos confirmam de novo
  broadcastLobbyLocked(lobby, "Dificuldade: " + tplIt->second.difficulties[index].name + ".");
}

void InstanceManager::cancelLobby(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(mu_);
  auto b = lobbyByPlayer_.find(playerId);
  if (b == lobbyByPlayer_.end()) return;
  auto it = lobbies_.find(b->second);
  if (it == lobbies_.end()) {
    lobbyByPlayer_.erase(b);
    return;
  }
  auto nm = it->second.names.find(playerId);
  const std::string who = nm != it->second.names.end() ? nm->second : std::string("Um membro");
  closeLobbyLocked(it->second.lobbyId, it->second.members.size() > 1 ? who + " cancelou a entrada." : std::string());
}

void InstanceManager::launchLobbyLocked(uint32_t lobbyId) {
  auto it = lobbies_.find(lobbyId);
  if (it == lobbies_.end() || !movement_) return;
  const Lobby lobby = it->second;  // cópia: closeLobbyLocked apaga
  auto tplIt = templates_.find(lobby.templateId);
  if (tplIt == templates_.end() || !tplIt->second.isActive) {
    closeLobbyLocked(lobbyId, "Instância indisponível no momento.");
    return;
  }
  const InstanceTemplateDef& tpl = tplIt->second;

  // Revalida: todos online, no mundo aberto e fora de instância.
  for (uint32_t m : lobby.members) {
    if (!movement_->isPlayerConnected(m) || instanceByPlayer_.count(m) > 0 || movement_->getPlayerLayer(m) != 0) {
      auto nm = lobby.names.find(m);
      closeLobbyLocked(lobbyId, (nm != lobby.names.end() ? nm->second : std::string("Um membro")) +
                                    " não está disponível.");
      return;
    }
  }
  if (tpl.partyRequired && lobby.existingInstanceId == 0 && lobby.members.size() < 2) {
    closeLobbyLocked(lobbyId, "Esta instância exige um grupo.");
    return;
  }

  ActiveInstance* target = nullptr;
  if (lobby.existingInstanceId > 0) {
    auto e = active_.find(lobby.existingInstanceId);
    if (e == active_.end()) {
      closeLobbyLocked(lobbyId, "A instância do seu grupo foi encerrada.");
      return;
    }
    if (e->second.members.size() + lobby.members.size() > tpl.maxPlayers) {
      closeLobbyLocked(lobbyId, "A instância do seu grupo está cheia.");
      return;
    }
    if (e->second.completed) {
      closeLobbyLocked(lobbyId, "A dungeon do seu grupo já foi concluída.");
      return;
    }
    target = &e->second;
  }

  // Fecha a sala (painel some) antes do 135 de entrada.
  closeLobbyLocked(lobbyId, std::string());
  for (uint32_t m : lobby.members) {
    ActiveInstance* used = enterInstanceLocked(m, tpl, target, lobby.openerPlayerId, lobby.difficultyIndex);
    if (!used) {
      refuse(m, tpl.id, "Todas as cópias desta instância estão ocupadas. Tente em instantes.");
      return;
    }
    target = used;  // os próximos entram na mesma cópia
  }
}

InstanceManager::ActiveInstance* InstanceManager::enterInstanceLocked(uint32_t playerId,
                                                                      const InstanceTemplateDef& tpl,
                                                                      ActiveInstance* target,
                                                                      uint32_t ownerPlayerId,
                                                                      size_t difficultyIndex) {
  if (!movement_ || !combat_) return nullptr;
  float px = 0.f, py = 0.f, pz = 0.f;
  if (!movement_->tryGetPlayerPosition(playerId, px, py, pz)) return target;
  const uint32_t currentLayer = movement_->getPlayerLayer(playerId);

  if (!target) {
    std::vector<uint32_t> used;
    for (const auto& [id, inst] : active_) {
      if (inst.templateId == tpl.id) used.push_back(inst.slot);
    }
    const uint32_t slot = allocateInstanceSlot(used, tpl.maxCopies);
    if (slot == kNoInstanceSlot) return nullptr;
    ActiveInstance inst;
    inst.instanceId = nextInstanceId_++;
    if (nextInstanceId_ == 0) nextInstanceId_ = 1;
    inst.templateId = tpl.id;
    inst.slot = slot;
    inst.ownerPlayerId = ownerPlayerId;
    computeInstanceCopyOrigin(tpl, slot, inst.originX, inst.originY, inst.originZ);
    inst.createdAt = std::chrono::steady_clock::now();
    if (tpl.timeLimitMinutes > 0) {
      inst.hasDeadline = true;
      inst.deadline = inst.createdAt + std::chrono::minutes(tpl.timeLimitMinutes);
      inst.expiresAtUnix = static_cast<uint32_t>(std::time(nullptr)) + tpl.timeLimitMinutes * 60u;
    }
    // Dungeon (etapa 5): dificuldade e totais do progresso.
    const auto diff = instanceDifficultyAt(tpl, difficultyIndex);
    float hpMult = 1.f, damageMult = 1.f;
    if (tpl.isDungeon) {
      inst.difficultyKey = diff.key;
      inst.difficultyName = diff.name;
      inst.rewardMult = diff.rewardMult;
      hpMult = diff.hpMult;
      damageMult = diff.damageMult;
      const auto counts = combat_->instanceMoldeRoleCounts(tpl.id);
      inst.enemiesTotal = counts.enemies;
      inst.bossesTotal = counts.bosses;
      inst.finalBossesTotal = counts.finalBosses;
    }
    const uint32_t newId = inst.instanceId;
    combat_->enqueueSpawnInstanceNpcs(tpl.id, newId, inst.originX, inst.originY, inst.originZ, hpMult, damageMult);
    target = &active_.emplace(newId, std::move(inst)).first->second;
    Core::Logger::getInstance().info(
        "[InstanceManager] criada instância {} (molde {} '{}', slot {}, origem {:.0f},{:.0f},{:.0f}, {}) por player {}",
        newId, tpl.id, tpl.code, slot, target->originX, target->originY, target->originZ,
        tpl.isDungeon ? target->difficultyKey : std::string("sem dificuldade"), ownerPlayerId);
  }

  ActiveInstance& inst = *target;
  inst.members[playerId] = ExitPoint{px, py, pz};
  instanceByPlayer_[playerId] = inst.instanceId;

  const float ex = inst.originX + tpl.entryX;
  const float ey = inst.originY + tpl.entryY;
  const float ez = inst.originZ + tpl.entryZ;

  // 135 antes do 123: o cliente trava o pawn e carrega o level antes de ser teleportado.
  InstanceStatePayload st;
  st.state = static_cast<uint8_t>(InstanceStateKind::Entered);
  st.instanceId = inst.instanceId;
  st.templateId = tpl.id;
  st.originX = inst.originX;
  st.originY = inst.originY;
  st.originZ = inst.originZ;
  st.posX = ex;
  st.posY = ey;
  st.posZ = ez;
  st.yaw = tpl.entryYaw;
  st.expiresAtUnix = inst.expiresAtUnix;
  st.mapPath = tpl.mapPath;
  st.name = tpl.name;
  movement_->sendToPlayer(playerId, encodeInstanceState(st));

  movement_->movePlayerToLayer(playerId, inst.instanceId, ex, ey, ez, tpl.entryYaw, ex, ey, ez, tpl.entryYaw, px,
                               py, pz);
  combat_->enqueueNpcLayerSwitch(playerId, currentLayer, inst.instanceId);
  if (tpl.isDungeon) {
    sendDungeonProgressLocked(inst, inst.completed ? static_cast<uint8_t>(DungeonRunState::Completed)
                                                   : static_cast<uint8_t>(DungeonRunState::Running),
                              playerId);
  }
  Core::Logger::getInstance().info("[InstanceManager] player {} entrou na instância {} ({} jogador(es))", playerId,
                                   inst.instanceId, inst.members.size());
  return target;
}

void InstanceManager::sendDungeonProgressLocked(const ActiveInstance& inst, uint8_t state, uint32_t onlyPlayerId) {
  if (!movement_) return;
  DungeonProgressPayload p;
  p.instanceId = inst.instanceId;
  p.bossesKilled = static_cast<uint16_t>(std::min<uint32_t>(inst.bossesKilled, 0xFFFF));
  p.bossesTotal = static_cast<uint16_t>(std::min<uint32_t>(inst.bossesTotal, 0xFFFF));
  p.enemiesKilled = static_cast<uint16_t>(std::min<uint32_t>(inst.enemiesKilled, 0xFFFF));
  p.enemiesTotal = static_cast<uint16_t>(std::min<uint32_t>(inst.enemiesTotal, 0xFFFF));
  p.state = state;
  if (inst.completed) {
    const auto left =
        std::chrono::duration_cast<std::chrono::seconds>(inst.completionCloseAt - std::chrono::steady_clock::now());
    p.closesInSeconds = static_cast<uint16_t>(std::clamp<int64_t>(left.count(), 0, 0xFFFF));
  }
  p.difficultyName = inst.difficultyName;
  const auto frame = encodeDungeonProgress(p);
  if (onlyPlayerId != 0) {
    movement_->sendToPlayer(onlyPlayerId, frame);
    return;
  }
  for (const auto& [pid, exit] : inst.members) movement_->sendToPlayer(pid, frame);
}

void InstanceManager::onInstanceNpcKilled(uint32_t instanceId, uint32_t npcInstanceId, uint8_t dungeonRole,
                                          uint32_t killerPlayerId) {
  (void)npcInstanceId;
  (void)killerPlayerId;
  std::lock_guard<std::mutex> lock(mu_);
  auto it = active_.find(instanceId);
  if (it == active_.end()) return;
  ActiveInstance& inst = it->second;
  auto tplIt = templates_.find(inst.templateId);
  if (tplIt == templates_.end() || !tplIt->second.isDungeon || inst.completed) return;
  const InstanceTemplateDef& tpl = tplIt->second;

  ++inst.enemiesKilled;
  if (dungeonRole >= 2) ++inst.bossesKilled;
  if (dungeonRole == 3) ++inst.finalBossesKilled;

  if (!dungeonCompletionReached(tpl.completionMode, inst.bossesKilled, inst.bossesTotal, inst.finalBossesKilled,
                                inst.finalBossesTotal, inst.enemiesKilled, inst.enemiesTotal)) {
    sendDungeonProgressLocked(inst, static_cast<uint8_t>(DungeonRunState::Running));
    return;
  }

  // Concluída: recompensa para quem está dentro agora; a cópia fecha em completion_close_seconds.
  inst.completed = true;
  const auto now = std::chrono::steady_clock::now();
  inst.completionCloseAt = now + std::chrono::seconds(tpl.completionCloseSeconds);
  sendDungeonProgressLocked(inst, static_cast<uint8_t>(DungeonRunState::Completed));

  DungeonCompletionJob job;
  job.instanceTemplateId = tpl.id;
  job.dungeonName = tpl.name;
  job.difficultyKey = inst.difficultyKey;
  job.difficultyName = inst.difficultyName;
  job.lockoutReset = tpl.lockoutReset;
  job.durationSeconds =
      static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(now - inst.createdAt).count());
  job.rewardExp = static_cast<int64_t>(std::llround(static_cast<double>(tpl.rewardExp) * inst.rewardMult));
  job.rewardGold = static_cast<uint32_t>(std::llround(static_cast<double>(tpl.rewardGold) * inst.rewardMult));
  for (const auto& [pid, exit] : inst.members) job.players.push_back(pid);
  Core::Logger::getInstance().info("[InstanceManager] dungeon {} concluída na instância {} ({}s, {} jogador(es))",
                                   tpl.code, inst.instanceId, job.durationSeconds, job.players.size());
  if (dungeonService_) dungeonService_->enqueueCompletion(std::move(job));
}

void InstanceManager::removeMemberLocked(ActiveInstance& inst, uint32_t playerId, const std::string& reason,
                                         bool online) {
  auto mIt = inst.members.find(playerId);
  if (mIt == inst.members.end()) return;
  const ExitPoint exit = mIt->second;
  inst.members.erase(mIt);
  instanceByPlayer_.erase(playerId);
  if (inst.members.empty()) inst.emptySince = std::chrono::steady_clock::now();

  if (!online || !movement_) return;
  InstanceStatePayload st;
  st.state = static_cast<uint8_t>(InstanceStateKind::Left);
  st.instanceId = inst.instanceId;
  st.templateId = inst.templateId;
  st.posX = exit.x;
  st.posY = exit.y;
  st.posZ = exit.z;
  st.reason = reason;
  auto tplIt = templates_.find(inst.templateId);
  if (tplIt != templates_.end()) {
    st.mapPath = tplIt->second.mapPath;
    st.name = tplIt->second.name;
  }
  movement_->sendToPlayer(playerId, encodeInstanceState(st));
  movement_->movePlayerToLayer(playerId, 0, exit.x, exit.y, exit.z, 0.f);
  if (combat_) combat_->enqueueNpcLayerSwitch(playerId, inst.instanceId, 0);
}

void InstanceManager::handleLeave(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = instanceByPlayer_.find(playerId);
  if (it == instanceByPlayer_.end()) return;
  auto instIt = active_.find(it->second);
  if (instIt == active_.end()) {
    instanceByPlayer_.erase(it);
    return;
  }
  removeMemberLocked(instIt->second, playerId, std::string(), true);
  Core::Logger::getInstance().info("[InstanceManager] player {} saiu da instância {}", playerId,
                                   instIt->second.instanceId);
}

void InstanceManager::onPlayerGone(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(mu_);
  // Sala de espera: quem desconecta desfaz a sala para os outros.
  auto lb = lobbyByPlayer_.find(playerId);
  if (lb != lobbyByPlayer_.end()) {
    auto lIt = lobbies_.find(lb->second);
    std::string who = "Um membro";
    if (lIt != lobbies_.end()) {
      auto nm = lIt->second.names.find(playerId);
      if (nm != lIt->second.names.end() && !nm->second.empty()) who = nm->second;
    }
    closeLobbyLocked(lb->second, who + " desconectou.");
  }
  auto it = instanceByPlayer_.find(playerId);
  if (it == instanceByPlayer_.end()) return;
  auto instIt = active_.find(it->second);
  if (instIt == active_.end()) {
    instanceByPlayer_.erase(it);
    return;
  }
  removeMemberLocked(instIt->second, playerId, std::string(), false);
}

void InstanceManager::closeInstanceLocked(uint32_t instanceId, const std::string& reason) {
  auto it = active_.find(instanceId);
  if (it == active_.end()) return;
  ActiveInstance& inst = it->second;
  std::vector<uint32_t> members;
  members.reserve(inst.members.size());
  for (const auto& [pid, exit] : inst.members) members.push_back(pid);
  for (uint32_t pid : members) {
    const bool online = movement_ && movement_->isPlayerConnected(pid);
    removeMemberLocked(inst, pid, reason, online);
  }
  if (combat_) combat_->enqueueRemoveInstanceNpcs(instanceId);
  Core::Logger::getInstance().info("[InstanceManager] instância {} fechada ({})", instanceId,
                                   reason.empty() ? "vazia" : reason);
  active_.erase(it);
}

bool InstanceManager::closeInstance(uint32_t instanceId, const std::string& reason) {
  std::lock_guard<std::mutex> lock(mu_);
  if (active_.count(instanceId) == 0) return false;
  closeInstanceLocked(instanceId, reason.empty() ? std::string("Instância encerrada pelo administrador.") : reason);
  return true;
}

void InstanceManager::tick() {
  std::lock_guard<std::mutex> lock(mu_);
  const auto now = std::chrono::steady_clock::now();
  // Salas que passaram do tempo sem todos prontos.
  std::vector<uint32_t> expiredLobbies;
  for (const auto& [id, lobby] : lobbies_) {
    if (now >= lobby.deadline) expiredLobbies.push_back(id);
  }
  for (uint32_t id : expiredLobbies) closeLobbyLocked(id, "O tempo para confirmar acabou.");
  if (active_.empty()) return;
  std::vector<std::pair<uint32_t, std::string>> toClose;
  for (const auto& [id, inst] : active_) {
    auto tplIt = templates_.find(inst.templateId);
    const uint32_t emptyClose = tplIt != templates_.end() ? tplIt->second.emptyCloseSeconds : 120;
    const bool isDungeon = tplIt != templates_.end() && tplIt->second.isDungeon;
    // Dungeon concluída: o tempo limite não vale mais; fecha em completion_close_seconds.
    if (inst.completed) {
      if (now >= inst.completionCloseAt) toClose.emplace_back(id, std::string("A dungeon foi concluída."));
      else if (inst.members.empty()) toClose.emplace_back(id, std::string());
      continue;
    }
    const bool timedOut = inst.hasDeadline && now >= inst.deadline;
    const bool isEmpty = inst.members.empty();
    const int64_t emptyForMs =
        isEmpty ? std::chrono::duration_cast<std::chrono::milliseconds>(now - inst.emptySince).count() : 0;
    if (shouldCloseInstance(isEmpty, emptyForMs, emptyClose, timedOut)) {
      if (timedOut && isDungeon) sendDungeonProgressLocked(inst, static_cast<uint8_t>(DungeonRunState::Failed));
      toClose.emplace_back(id, timedOut ? std::string("O tempo da instância acabou.") : std::string());
    }
  }
  for (const auto& [id, reason] : toClose) closeInstanceLocked(id, reason);
}

nlohmann::json InstanceManager::listJson() const {
  std::lock_guard<std::mutex> lock(mu_);
  nlohmann::json out = nlohmann::json::array();
  const auto now = std::chrono::steady_clock::now();
  for (const auto& [id, inst] : active_) {
    nlohmann::json j;
    j["instance_id"] = id;
    j["template_id"] = inst.templateId;
    auto tplIt = templates_.find(inst.templateId);
    j["code"] = tplIt != templates_.end() ? tplIt->second.code : std::string();
    j["slot"] = inst.slot;
    j["owner_player_id"] = inst.ownerPlayerId;
    nlohmann::json members = nlohmann::json::array();
    for (const auto& [pid, exit] : inst.members) members.push_back(pid);
    j["members"] = members;
    j["age_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(now - inst.createdAt).count();
    j["expires_at_unix"] = inst.expiresAtUnix;
    j["difficulty"] = inst.difficultyKey;
    j["bosses"] = std::to_string(inst.bossesKilled) + "/" + std::to_string(inst.bossesTotal);
    j["enemies"] = std::to_string(inst.enemiesKilled) + "/" + std::to_string(inst.enemiesTotal);
    j["completed"] = inst.completed;
    out.push_back(std::move(j));
  }
  return out;
}

}  // namespace Zone
}  // namespace Umbra
