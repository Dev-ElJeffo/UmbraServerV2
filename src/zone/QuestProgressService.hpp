#pragma once
// QuestProgressService.hpp — progresso de quest decidido pela zone (Quests v2, etapa 3a).
// Eventos (kill, skill, consumível, ouro, nível, área, usar item no local) só enfileiram;
// uma thread própria mantém o cache dos objetivos ativos por jogador e grava no DB só
// quando um objetivo casa. Notifica o cliente pelo opcode 116.

#include "database/MySQLConnector.hpp"
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;
class GuildMissionService;

/** Tipos de objetivo que a zone credita (quest_objectives.objective_type). */
enum class QuestObjectiveKind : uint8_t {
  Other = 0,       // talk/collect/deliver/craft_item/refine_item: PHP
  Kill,
  KillAny,
  UseSkill,
  UseConsumable,
  EarnGold,
  ReachLevel,
  ReachArea,
  UseItemAt,
};

QuestObjectiveKind questKindFromString(const std::string& type);

/** Parâmetros já decodificados do params_json (só o que a zone usa). */
struct QuestObjectiveParams {
  uint32_t npcTemplateId = 0;
  uint32_t npcInstanceId = 0;
  std::vector<uint32_t> npcTemplateIds;
  uint32_t minLevel = 0;
  uint32_t maxLevel = 0;
  uint32_t zoneId = 0;
  uint32_t skillId = 0;
  uint32_t itemTemplateId = 0;
  std::string itemSubtype;
  float posX = 0.f, posY = 0.f, posZ = 0.f, radius = 300.f;
  bool consume = false;
  int32_t required = 1;  // required_count / amount / level
};

QuestObjectiveParams parseQuestObjectiveParams(QuestObjectiveKind kind, const std::string& paramsJson);

/** Evento de jogo que pode avançar objetivos. */
struct QuestEvent {
  QuestObjectiveKind kind = QuestObjectiveKind::Other;
  uint32_t key = 0;          // npcTemplateId / skillId / itemTemplateId
  uint32_t npcInstanceId = 0;
  uint32_t level = 0;        // nível do NPC (kill_any) ou novo nível (reach_level)
  uint32_t zoneId = 0;
  std::string subtype;       // item_subtype (use_consumable)
  int32_t amount = 1;        // ouro (earn_gold); demais = 1
};

/** O evento casa com o objetivo? (puro; testado em test_quest_events). */
bool questEventMatches(QuestObjectiveKind objKind, const QuestObjectiveParams& params, const QuestEvent& ev,
                       uint32_t serverZoneId);

/** Novo current_count depois do evento (reach_level seta o nível; os outros somam). */
int32_t questApplyEventCount(QuestObjectiveKind objKind, int32_t current, int32_t required, const QuestEvent& ev);

class QuestProgressService {
public:
  explicit QuestProgressService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);
  ~QuestProgressService();

  void setMovementServer(MovementServer* movement) { movementServer_ = movement; }
  void setResolvePartyMembers(std::function<std::vector<uint32_t>(uint32_t)> cb) {
    resolvePartyMembers_ = std::move(cb);
  }
  void setShareRadiusUu(float radiusUu) { shareRadiusUu_ = radiusUu; }

  /** Missões de guilda: recebem os mesmos eventos, na thread deste serviço. */
  void setGuildMissionService(GuildMissionService* guildMissions) { guildMissions_ = guildMissions; }

  void start();
  void stop();

  /** NPC morreu: kill/kill_any para o matador + grupo no raio (só enfileira). */
  void onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                   uint32_t zoneId, uint32_t npcLevel = 0);

  /** Evento genérico de um jogador (skill, consumível, ouro, nível). Só enfileira. */
  void onEvent(uint32_t playerId, const QuestEvent& ev);

  /** Opcode 131: aceitou/abandonou/entregou pelo HTTP → recarrega o cache do jogador. */
  void onQuestStateChanged(uint32_t playerId);

  /** Opcode 132: usar item no local (posição real da zone + item na bolsa). */
  void onQuestInteract(uint32_t playerId, uint32_t questId, uint32_t itemTemplateId);

  /** Saiu da zone: descarta o cache. */
  void onPlayerGone(uint32_t playerId);

private:
  struct ActiveObjective {
    uint64_t playerObjectiveId = 0;
    uint64_t playerQuestId = 0;
    uint32_t questId = 0;
    QuestObjectiveKind kind = QuestObjectiveKind::Other;
    QuestObjectiveParams params;
    int32_t current = 0;
    int64_t expiresAtMs = 0;  // 0 = sem tempo limite
    int64_t nextTryMs = 0;    // use_item_at na varredura: espera entre consultas da bolsa
  };
  struct PlayerCache {
    int64_t loadedAtMs = 0;
    std::vector<ActiveObjective> objectives;
  };
  struct Job {
    enum class Kind : uint8_t { Event, Invalidate, Interact, Gone } kind = Kind::Event;
    uint32_t playerId = 0;
    QuestEvent event;
    uint32_t questId = 0;
    uint32_t itemTemplateId = 0;
  };

  void enqueue(Job job);
  void workerLoop();
  PlayerCache& ensureCache(uint32_t playerId, int64_t nowMs);
  void loadCache(uint32_t playerId, PlayerCache& cache, int64_t nowMs);
  void processEvent(uint32_t playerId, const QuestEvent& ev);
  void processInteract(uint32_t playerId, uint32_t questId, uint32_t itemTemplateId);
  /** use_item_at: posição real + item na bolsa (+ consome). false = recusado (why preenchido). */
  bool tryUseItemAt(uint32_t playerId, ActiveObjective& obj, float x, float y, float z, const char*& why);
  void scanAreas(int64_t nowMs);
  /** Grava o novo count; se completou, promove a quest e tira do cache. Notifica 116. */
  void commitProgress(uint32_t playerId, ActiveObjective& obj, int32_t newCount);
  void promoteReadyQuest(uint64_t playerQuestId);
  void notifyQuestProgress(uint32_t playerId, uint32_t questId, uint8_t status, uint8_t flags);
  static int64_t nowMs();

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_ = 0;
  MovementServer* movementServer_ = nullptr;
  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;
  float shareRadiusUu_ = 5000.f;
  GuildMissionService* guildMissions_ = nullptr;

  // Só a thread do worker mexe no cache.
  std::unordered_map<uint32_t, PlayerCache> cache_;

  std::mutex queueMu_;
  std::condition_variable queueCv_;
  std::deque<Job> queue_;
  bool stop_ = false;
  std::thread worker_;
};

}  // namespace Zone
}  // namespace Umbra
