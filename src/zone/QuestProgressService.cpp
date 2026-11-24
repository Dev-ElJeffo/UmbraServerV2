#include "zone/QuestProgressService.hpp"
#include "zone/GuildMissionService.hpp"
#include "zone/PartyShare.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace Umbra {
namespace Zone {

namespace {

/** Cache de objetivos por jogador: recarrega sozinho depois disso (131 recarrega na hora). */
constexpr int64_t kCacheTtlMs = 60000;
/** Varredura de área (reach_area) e de jogadores online sem cache. */
constexpr int64_t kAreaScanMs = 1000;
/** Folga do raio (mesma do PHP: QUEST_AREA_MARGIN). */
constexpr float kAreaMarginUu = 50.f;

constexpr uint8_t kQuestFlagProgress = 0x01;
constexpr uint8_t kQuestFlagBecameReady = 0x02;
constexpr uint8_t kQuestFlagInteractFailed = 0x04;

long long toLL(const std::string& s, long long def = 0) {
  try {
    return s.empty() ? def : std::stoll(s);
  } catch (...) {
    return def;
  }
}

uint32_t jsonU32(const nlohmann::json& j, const char* key) {
  auto it = j.find(key);
  if (it == j.end()) return 0;
  if (it->is_number()) return static_cast<uint32_t>(std::max<long long>(0, it->get<long long>()));
  if (it->is_string()) return static_cast<uint32_t>(std::max<long long>(0, toLL(it->get<std::string>())));
  return 0;
}

float jsonF(const nlohmann::json& j, const char* key, float def) {
  auto it = j.find(key);
  if (it == j.end()) return def;
  if (it->is_number()) return it->get<float>();
  if (it->is_string()) {
    try {
      return std::stof(it->get<std::string>());
    } catch (...) {
      return def;
    }
  }
  return def;
}

}  // namespace

QuestObjectiveKind questKindFromString(const std::string& type) {
  if (type == "kill") return QuestObjectiveKind::Kill;
  if (type == "kill_any") return QuestObjectiveKind::KillAny;
  if (type == "use_skill") return QuestObjectiveKind::UseSkill;
  if (type == "use_consumable") return QuestObjectiveKind::UseConsumable;
  if (type == "earn_gold") return QuestObjectiveKind::EarnGold;
  if (type == "reach_level") return QuestObjectiveKind::ReachLevel;
  if (type == "reach_area") return QuestObjectiveKind::ReachArea;
  if (type == "use_item_at") return QuestObjectiveKind::UseItemAt;
  if (type == "complete_dungeon") return QuestObjectiveKind::CompleteDungeon;
  return QuestObjectiveKind::Other;
}

QuestObjectiveParams parseQuestObjectiveParams(QuestObjectiveKind kind, const std::string& paramsJson) {
  QuestObjectiveParams p;
  auto j = nlohmann::json::parse(paramsJson.empty() ? std::string("{}") : paramsJson, nullptr, false);
  if (j.is_discarded() || !j.is_object()) return p;
  p.npcTemplateId = jsonU32(j, "npc_template_id");
  p.npcInstanceId = jsonU32(j, "npc_instance_id");
  if (auto it = j.find("npc_template_ids"); it != j.end() && it->is_array()) {
    for (const auto& v : *it) {
      if (v.is_number()) p.npcTemplateIds.push_back(static_cast<uint32_t>(std::max<long long>(0, v.get<long long>())));
    }
  }
  p.minLevel = jsonU32(j, "min_level");
  p.maxLevel = jsonU32(j, "max_level");
  p.zoneId = jsonU32(j, "zone_id");
  p.skillId = jsonU32(j, "skill_id");
  p.itemTemplateId = jsonU32(j, "item_template_id");
  if (auto it = j.find("item_subtype"); it != j.end() && it->is_string()) p.itemSubtype = it->get<std::string>();
  p.posX = jsonF(j, "pos_x", 0.f);
  p.posY = jsonF(j, "pos_y", 0.f);
  p.posZ = jsonF(j, "pos_z", 0.f);
  p.radius = jsonF(j, "radius", 300.f);
  p.instanceTemplateId = jsonU32(j, "instance_template_id");
  if (auto it = j.find("difficulty_key"); it != j.end() && it->is_string()) p.difficultyKey = it->get<std::string>();
  if (auto it = j.find("consume"); it != j.end()) {
    p.consume = it->is_boolean() ? it->get<bool>() : (it->is_number() && it->get<int>() != 0);
  }
  long long required = jsonU32(j, "required_count");
  if (kind == QuestObjectiveKind::EarnGold) required = jsonU32(j, "amount");
  if (kind == QuestObjectiveKind::ReachLevel) required = jsonU32(j, "level");
  p.required = static_cast<int32_t>(std::max<long long>(1, required));
  return p;
}

bool questEventMatches(QuestObjectiveKind objKind, const QuestObjectiveParams& p, const QuestEvent& ev,
                       uint32_t serverZoneId) {
  if (objKind != ev.kind) return false;
  switch (objKind) {
    case QuestObjectiveKind::Kill:
      if (p.npcInstanceId > 0 && ev.npcInstanceId == p.npcInstanceId) return true;
      if (p.npcTemplateId > 0 && ev.key == p.npcTemplateId) return true;
      return ev.key > 0 && std::find(p.npcTemplateIds.begin(), p.npcTemplateIds.end(), ev.key) != p.npcTemplateIds.end();
    case QuestObjectiveKind::KillAny: {
      if (p.minLevel > 0 && ev.level < p.minLevel) return false;
      if (p.maxLevel > 0 && ev.level > p.maxLevel) return false;
      const uint32_t zone = ev.zoneId > 0 ? ev.zoneId : serverZoneId;
      return p.zoneId == 0 || zone == 0 || p.zoneId == zone;
    }
    case QuestObjectiveKind::UseSkill:
      return p.skillId == 0 || p.skillId == ev.key;
    case QuestObjectiveKind::UseConsumable:
      if (p.itemTemplateId > 0) return p.itemTemplateId == ev.key;
      if (!p.itemSubtype.empty()) return p.itemSubtype == ev.subtype;
      return true;  // qualquer consumível
    case QuestObjectiveKind::EarnGold:
      return ev.amount > 0;
    case QuestObjectiveKind::ReachLevel:
      return ev.level > 0;
    case QuestObjectiveKind::CompleteDungeon:
      if (p.instanceTemplateId > 0 && p.instanceTemplateId != ev.key) return false;
      return p.difficultyKey.empty() || p.difficultyKey == ev.subtype;
    default:
      return false;
  }
}

int32_t questApplyEventCount(QuestObjectiveKind objKind, int32_t current, int32_t required, const QuestEvent& ev) {
  if (objKind == QuestObjectiveKind::ReachLevel) {
    return std::min(required, std::max(current, static_cast<int32_t>(ev.level)));
  }
  const int64_t add = objKind == QuestObjectiveKind::EarnGold ? std::max(0, ev.amount) : 1;
  return static_cast<int32_t>(std::min<int64_t>(required, static_cast<int64_t>(current) + add));
}

QuestProgressService::QuestProgressService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

QuestProgressService::~QuestProgressService() { stop(); }

int64_t QuestProgressService::nowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void QuestProgressService::start() {
  if (worker_.joinable()) return;
  stop_ = false;
  worker_ = std::thread([this] { workerLoop(); });
}

void QuestProgressService::stop() {
  {
    std::lock_guard<std::mutex> lock(queueMu_);
    stop_ = true;
  }
  queueCv_.notify_all();
  if (worker_.joinable()) worker_.join();
}

void QuestProgressService::enqueue(Job job) {
  {
    std::lock_guard<std::mutex> lock(queueMu_);
    if (queue_.size() >= 4096) return;  // fila cheia: descarta (progresso não é crítico)
    queue_.push_back(std::move(job));
  }
  queueCv_.notify_one();
}

void QuestProgressService::onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                                       uint32_t zoneId, uint32_t npcLevel) {
  if (killerPlayerId == 0 || (npcTemplateId == 0 && npcInstanceId == 0)) return;
  if (zoneId > 0 && zoneId_ > 0 && zoneId != zoneId_) return;
  const auto recipients =
      collectPartyShareRecipients(killerPlayerId, shareRadiusUu_, movementServer_, resolvePartyMembers_);
  for (uint32_t pid : recipients) {
    for (QuestObjectiveKind kind : {QuestObjectiveKind::Kill, QuestObjectiveKind::KillAny}) {
      Job job;
      job.kind = Job::Kind::Event;
      job.playerId = pid;
      job.event.kind = kind;
      job.event.key = npcTemplateId;
      job.event.npcInstanceId = npcInstanceId;
      job.event.level = npcLevel;
      job.event.zoneId = zoneId;
      enqueue(std::move(job));
    }
  }
}

void QuestProgressService::onEvent(uint32_t playerId, const QuestEvent& ev) {
  if (playerId == 0 || ev.kind == QuestObjectiveKind::Other) return;
  Job job;
  job.kind = Job::Kind::Event;
  job.playerId = playerId;
  job.event = ev;
  enqueue(std::move(job));
}

void QuestProgressService::onQuestStateChanged(uint32_t playerId) {
  if (playerId == 0) return;
  Job job;
  job.kind = Job::Kind::Invalidate;
  job.playerId = playerId;
  enqueue(std::move(job));
}

void QuestProgressService::onQuestInteract(uint32_t playerId, uint32_t questId, uint32_t itemTemplateId) {
  if (playerId == 0 || questId == 0) return;
  Job job;
  job.kind = Job::Kind::Interact;
  job.playerId = playerId;
  job.questId = questId;
  job.itemTemplateId = itemTemplateId;
  enqueue(std::move(job));
}

void QuestProgressService::onPlayerGone(uint32_t playerId) {
  if (playerId == 0) return;
  Job job;
  job.kind = Job::Kind::Gone;
  job.playerId = playerId;
  enqueue(std::move(job));
}

void QuestProgressService::workerLoop() {
  int64_t nextScan = nowMs() + kAreaScanMs;
  while (true) {
    std::deque<Job> jobs;
    {
      std::unique_lock<std::mutex> lock(queueMu_);
      queueCv_.wait_for(lock, std::chrono::milliseconds(200), [this] { return stop_ || !queue_.empty(); });
      if (stop_) break;
      jobs.swap(queue_);
    }
    for (const Job& job : jobs) {
      try {
        switch (job.kind) {
          case Job::Kind::Event:
            processEvent(job.playerId, job.event);
            break;
          case Job::Kind::Invalidate:
            cache_.erase(job.playerId);
            ensureCache(job.playerId, nowMs());
            break;
          case Job::Kind::Interact:
            processInteract(job.playerId, job.questId, job.itemTemplateId);
            break;
          case Job::Kind::Gone:
            cache_.erase(job.playerId);
            break;
        }
      } catch (const std::exception& e) {
        Core::Logger::getInstance().error("[QuestProgressService] job player {} falhou: {}", job.playerId, e.what());
      }
    }
    const int64_t now = nowMs();
    if (now >= nextScan) {
      nextScan = now + kAreaScanMs;
      try {
        scanAreas(now);
      } catch (const std::exception& e) {
        Core::Logger::getInstance().error("[QuestProgressService] scan de área falhou: {}", e.what());
      }
    }
  }
}

QuestProgressService::PlayerCache& QuestProgressService::ensureCache(uint32_t playerId, int64_t now) {
  PlayerCache& c = cache_[playerId];
  if (c.loadedAtMs == 0 || now - c.loadedAtMs > kCacheTtlMs) {
    loadCache(playerId, c, now);
  }
  return c;
}

void QuestProgressService::loadCache(uint32_t playerId, PlayerCache& cache, int64_t now) {
  cache.loadedAtMs = now;
  cache.objectives.clear();
  if (!db_ || !db_->isConnected()) return;
  // Ordem posicional: conferir os índices abaixo ao mudar o SELECT.
  const auto rows = db_->executePreparedQuery(
      "SELECT pqo.player_objective_id, pq.player_quest_id, pq.quest_id, qo.objective_type, "
      "COALESCE(qo.params_json, '{}'), pqo.current_count, COALESCE(UNIX_TIMESTAMP(pq.accepted_at), 0), "
      "COALESCE(q.time_limit_minutes, 0) "
      "FROM player_quest_objectives pqo "
      "INNER JOIN player_quests pq ON pq.player_quest_id = pqo.player_quest_id "
      "INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id "
      "INNER JOIN quests q ON q.quest_id = pq.quest_id "
      "WHERE pq.player_id = ? AND pq.status = 'active' AND pqo.is_completed = 0",
      {std::to_string(playerId)});
  for (const auto& r : rows) {
    if (r.size() < 8) continue;
    ActiveObjective o;
    o.kind = questKindFromString(r[3]);
    if (o.kind == QuestObjectiveKind::Other) continue;
    o.playerObjectiveId = static_cast<uint64_t>(toLL(r[0]));
    o.playerQuestId = static_cast<uint64_t>(toLL(r[1]));
    o.questId = static_cast<uint32_t>(toLL(r[2]));
    o.params = parseQuestObjectiveParams(o.kind, r[4]);
    o.current = static_cast<int32_t>(toLL(r[5]));
    const long long acceptedSec = toLL(r[6]);
    const long long limitMin = toLL(r[7]);
    o.expiresAtMs = (limitMin > 0 && acceptedSec > 0) ? (acceptedSec + limitMin * 60) * 1000 : 0;
    cache.objectives.push_back(std::move(o));
  }
}

void QuestProgressService::processEvent(uint32_t playerId, const QuestEvent& ev) {
  if (guildMissions_) {
    try {
      guildMissions_->onEvent(playerId, ev);
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("[QuestProgressService] missão de guilda falhou player {}: {}", playerId, e.what());
    }
  }
  const int64_t now = nowMs();
  PlayerCache& cache = ensureCache(playerId, now);
  if (cache.objectives.empty()) return;
  // Cópia dos índices: commitProgress pode tirar objetivos concluídos do vetor.
  std::vector<uint64_t> matched;
  for (const ActiveObjective& o : cache.objectives) {
    if (o.expiresAtMs > 0 && now > o.expiresAtMs) continue;  // PHP marca failed
    if (questEventMatches(o.kind, o.params, ev, zoneId_)) matched.push_back(o.playerObjectiveId);
  }
  for (uint64_t id : matched) {
    auto it = std::find_if(cache.objectives.begin(), cache.objectives.end(),
                           [id](const ActiveObjective& o) { return o.playerObjectiveId == id; });
    if (it == cache.objectives.end()) continue;
    const int32_t next = questApplyEventCount(it->kind, it->current, it->params.required, ev);
    if (next != it->current) commitProgress(playerId, *it, next);
  }
}

void QuestProgressService::commitProgress(uint32_t playerId, ActiveObjective& obj, int32_t newCount) {
  const bool done = newCount >= obj.params.required;
  if (!db_->executePreparedInsert(
          "UPDATE player_quest_objectives SET current_count = ?, is_completed = ? "
          "WHERE player_objective_id = ? AND is_completed = 0",
          {std::to_string(newCount), done ? "1" : "0", std::to_string(obj.playerObjectiveId)})) {
    Core::Logger::getInstance().warn("[QuestProgressService] falha ao gravar objetivo {} do player {}",
                                     obj.playerObjectiveId, playerId);
    return;
  }
  obj.current = newCount;
  const uint32_t questId = obj.questId;
  const uint64_t playerQuestId = obj.playerQuestId;
  uint8_t status = 1;
  uint8_t flags = kQuestFlagProgress;
  if (done) {
    auto& list = cache_[playerId].objectives;
    const uint64_t doneId = obj.playerObjectiveId;
    list.erase(std::remove_if(list.begin(), list.end(),
                              [doneId](const ActiveObjective& o) { return o.playerObjectiveId == doneId; }),
               list.end());
    promoteReadyQuest(playerQuestId);
    const auto st = db_->executePreparedScalar("SELECT status FROM player_quests WHERE player_quest_id = ?",
                                               {std::to_string(playerQuestId)});
    if (st && *st == "ready") {
      status = 2;
      flags |= kQuestFlagBecameReady;
    }
  }
  notifyQuestProgress(playerId, questId, status, flags);
  Core::Logger::getInstance().info("[QuestProgressService] player {} quest {} objetivo {} → {} ({})", playerId,
                                   questId, playerQuestId, newCount, status == 2 ? "pronta" : "ativa");
}

void QuestProgressService::promoteReadyQuest(uint64_t playerQuestId) {
  db_->executePreparedInsert(
      "UPDATE player_quests pq SET pq.status = 'ready' "
      "WHERE pq.player_quest_id = ? AND pq.status = 'active' "
      "AND NOT EXISTS (SELECT 1 FROM player_quest_objectives pqo "
      "  WHERE pqo.player_quest_id = pq.player_quest_id AND pqo.is_completed = 0) "
      "AND NOT EXISTS (SELECT 1 FROM quest_objectives qo WHERE qo.quest_id = pq.quest_id "
      "  AND qo.objective_id NOT IN (SELECT pqo2.objective_id FROM player_quest_objectives pqo2 "
      "    WHERE pqo2.player_quest_id = pq.player_quest_id))",
      {std::to_string(playerQuestId)});
}

void QuestProgressService::scanAreas(int64_t now) {
  if (!movementServer_) return;
  const auto players = movementServer_->getPlayerStates();
  for (const auto& [pid, st] : players) {
    if (pid == 0 || st.isDead) continue;
    PlayerCache& cache = ensureCache(pid, now);  // aquece quem entrou sem nenhum evento ainda
    std::vector<uint64_t> reached;
    for (const ActiveObjective& o : cache.objectives) {
      if (o.kind != QuestObjectiveKind::ReachArea) continue;
      if (o.expiresAtMs > 0 && now > o.expiresAtMs) continue;
      if (o.params.zoneId > 0 && zoneId_ > 0 && o.params.zoneId != zoneId_) continue;
      const float dx = st.x - o.params.posX;
      const float dy = st.y - o.params.posY;
      const float dz = st.z - o.params.posZ;
      const float r = o.params.radius + kAreaMarginUu;
      if (dx * dx + dy * dy + dz * dz <= r * r) reached.push_back(o.playerObjectiveId);
    }
    for (uint64_t id : reached) {
      auto& list = cache.objectives;
      auto it = std::find_if(list.begin(), list.end(), [id](const ActiveObjective& o) { return o.playerObjectiveId == id; });
      if (it != list.end()) commitProgress(pid, *it, it->params.required);
    }
    // use_item_at automático: no local com o item na bolsa (consulta a bolsa no máx. a cada 5 s).
    std::vector<uint64_t> useAt;
    for (ActiveObjective& o : cache.objectives) {
      if (o.kind != QuestObjectiveKind::UseItemAt || now < o.nextTryMs) continue;
      if (o.expiresAtMs > 0 && now > o.expiresAtMs) continue;
      const float dx = st.x - o.params.posX, dy = st.y - o.params.posY, dz = st.z - o.params.posZ;
      const float r = o.params.radius + kAreaMarginUu;
      if (dx * dx + dy * dy + dz * dz <= r * r) useAt.push_back(o.playerObjectiveId);
    }
    for (uint64_t id : useAt) {
      auto& list = cache.objectives;
      auto it = std::find_if(list.begin(), list.end(), [id](const ActiveObjective& o) { return o.playerObjectiveId == id; });
      if (it == list.end()) continue;
      it->nextTryMs = now + 5000;
      const char* why = nullptr;
      tryUseItemAt(pid, *it, st.x, st.y, st.z, why);  // falha silenciosa: tenta de novo em 5 s
    }
  }
  // Quem saiu da zone sem o hook (cache antigo) cai pelo TTL.
  for (auto it = cache_.begin(); it != cache_.end();) {
    if (players.find(it->first) == players.end() && now - it->second.loadedAtMs > kCacheTtlMs) {
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }
}

bool QuestProgressService::tryUseItemAt(uint32_t playerId, ActiveObjective& obj, float x, float y, float z,
                                        const char*& why) {
  if (obj.params.zoneId > 0 && zoneId_ > 0 && obj.params.zoneId != zoneId_) {
    why = "outra zone";
    return false;
  }
  const float dx = x - obj.params.posX, dy = y - obj.params.posY, dz = z - obj.params.posZ;
  const float r = obj.params.radius + kAreaMarginUu;
  if (dx * dx + dy * dy + dz * dz > r * r) {
    why = "fora do local";
    return false;
  }
  const uint32_t needItem = obj.params.itemTemplateId;
  if (needItem > 0) {
    const std::string pid = std::to_string(playerId);
    auto rows = db_->executePreparedQuery(
        "SELECT inventory_id, quantity FROM player_inventory WHERE player_id = ? AND item_template_id = ? "
        "AND COALESCE(is_equipped,0) = 0 AND auction_listing_id IS NULL AND quantity > 0 ORDER BY slot_index LIMIT 1",
        {pid, std::to_string(needItem)});
    if (rows.empty() || rows[0].size() < 2) {
      why = "sem o item";
      return false;
    }
    if (obj.params.consume) {
      const std::string invId = rows[0][0];
      if (toLL(rows[0][1]) <= 1) {
        db_->executePreparedInsert("DELETE FROM player_inventory WHERE inventory_id = ? AND player_id = ?", {invId, pid});
      } else {
        db_->executePreparedInsert(
            "UPDATE player_inventory SET quantity = quantity - 1 WHERE inventory_id = ? AND player_id = ? AND quantity > 1",
            {invId, pid});
      }
    }
  }
  commitProgress(playerId, obj, std::min(obj.params.required, obj.current + 1));
  return true;
}

void QuestProgressService::processInteract(uint32_t playerId, uint32_t questId, uint32_t itemTemplateId) {
  const int64_t now = nowMs();
  PlayerCache& cache = ensureCache(playerId, now);
  auto it = std::find_if(cache.objectives.begin(), cache.objectives.end(), [&](const ActiveObjective& o) {
    return o.kind == QuestObjectiveKind::UseItemAt && o.questId == questId &&
           (itemTemplateId == 0 || o.params.itemTemplateId == 0 || o.params.itemTemplateId == itemTemplateId);
  });
  const char* why = "sem objetivo ativo";
  bool ok = false;
  float x = 0.f, y = 0.f, z = 0.f;
  if (it != cache.objectives.end()) {
    if (it->expiresAtMs > 0 && now > it->expiresAtMs) {
      why = "tempo esgotado";
    } else if (!movementServer_ || !movementServer_->tryGetPlayerPosition(playerId, x, y, z)) {
      why = "sem posição";
    } else {
      ok = tryUseItemAt(playerId, *it, x, y, z, why);
    }
  }
  if (!ok) {
    notifyQuestProgress(playerId, questId, 1, kQuestFlagInteractFailed);
    Core::Logger::getInstance().info("[QuestProgressService] usar item no local recusado player {} quest {}: {}",
                                     playerId, questId, why);
  }
}

void QuestProgressService::notifyQuestProgress(uint32_t playerId, uint32_t questId, uint8_t status, uint8_t flags) {
  if (!movementServer_ || playerId == 0) return;
  QuestProgressNotifyPayload p;
  p.playerId = playerId;
  p.questId = questId;
  p.status = status;
  p.flags = flags;
  movementServer_->sendToPlayer(playerId, encodeQuestProgressNotify(p));
}

}  // namespace Zone
}  // namespace Umbra
