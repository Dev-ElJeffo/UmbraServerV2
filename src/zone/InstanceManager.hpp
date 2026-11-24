#pragma once
// InstanceManager.hpp — instâncias lógicas dentro do processo da zone (etapa 4, opcodes 134-136).
//
// Cada instância é uma "camada" (instanceId > 0) com cópia própria dos NPCs do molde.
// Isolamento:
//  - cada cópia fica num ponto distante (origem do molde + slot * slot_spacing): combate, IA,
//    loot e AOI já se separam pela distância;
//  - broadcasts globais de NPC (spawn/despawn/estado) e snapshots são filtrados pela camada.
// O cliente carrega o level do molde como level instance na origem da cópia (WebSocket continua vivo).

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace Umbra {
namespace Database {
class MySQLConnector;
}
namespace Zone {

class MovementServer;
class CombatCoreEngine;
class DungeonService;

/** instance_templates (Manager → aba Instâncias). Posições de entrada são LOCAIS do level. */
struct InstanceTemplateDef {
  uint32_t id = 0;
  std::string code;
  std::string name;
  std::string description;
  /** Soft path do level: /Game/Maps/Instances/Lvl_X.Lvl_X */
  std::string mapPath;
  float originX = 0.f;
  float originY = 0.f;
  float originZ = 0.f;
  /** Distância entre cópias (eixo Y). Mínimo 20000 (AOI de 3x3 células de 10000 não pode se tocar). */
  float slotSpacing = 200000.f;
  float entryX = 0.f;
  float entryY = 0.f;
  float entryZ = 0.f;
  float entryYaw = 0.f;
  uint32_t minLevel = 1;
  uint32_t maxPlayers = 5;
  bool partyRequired = false;
  /** 0 = sem limite. */
  uint32_t timeLimitMinutes = 0;
  /** Fecha a cópia depois de N segundos vazia. */
  uint32_t emptyCloseSeconds = 120;
  uint32_t maxCopies = 20;
  bool isActive = true;

  // ---- Dungeon (etapa 5) ----
  bool isDungeon = false;
  /** "final_boss" ou "all_bosses". */
  std::string completionMode = "final_boss";
  uint32_t rewardExp = 0;
  uint32_t rewardGold = 0;
  /** "none", "daily" ou "weekly". */
  std::string lockoutReset = "daily";
  uint32_t completionCloseSeconds = 300;
  struct Difficulty {
    std::string key;
    std::string name;
    uint32_t minLevel = 1;
    float hpMult = 1.f;
    float damageMult = 1.f;
    float rewardMult = 1.f;
  };
  /** Ordenadas por sort_order. Vazio = dificuldade única "normal". */
  std::vector<Difficulty> difficulties;
};

/** Dificuldade efetiva (índice fora da faixa ou lista vazia = "normal" x1). */
InstanceTemplateDef::Difficulty instanceDifficultyAt(const InstanceTemplateDef& tpl, size_t index);

// ---- Funções puras (testadas em tests/test_instances.cpp) ----

constexpr uint32_t kNoInstanceSlot = 0xFFFFFFFFu;

/** Menor slot livre em [0, maxCopies); kNoInstanceSlot se todos ocupados. */
uint32_t allocateInstanceSlot(const std::vector<uint32_t>& usedSlots, uint32_t maxCopies);

/** Origem da cópia `slot`: origem do molde deslocada slot * slotSpacing no eixo Y. */
void computeInstanceCopyOrigin(const InstanceTemplateDef& tpl, uint32_t slot, float& outX, float& outY,
                               float& outZ);

/** True se a cópia deve fechar (tempo esgotado, ou vazia há emptyCloseSeconds). */
bool shouldCloseInstance(bool isEmpty, int64_t emptyForMs, uint32_t emptyCloseSeconds, bool timedOut);

/** Aplica limites e defaults (valores do Manager fora da faixa). */
void clampInstanceTemplate(InstanceTemplateDef& tpl);

/** True se a sala pode partir: tem membros, todos prontos e todos com o nível mínimo. */
bool lobbyAllReady(const std::vector<uint32_t>& members, const std::unordered_map<uint32_t, bool>& ready,
                   const std::unordered_map<uint32_t, uint32_t>& levels, uint32_t minLevel);

class InstanceManager {
 public:
  InstanceManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId, MovementServer* movement,
                  CombatCoreEngine* combat);

  /** Moldes da zone (instance_templates) + NPCs dos moldes. Retorna a quantidade de moldes. */
  size_t reload();

  void setResolvePartyMembers(std::function<std::vector<uint32_t>(uint32_t)> cb) {
    resolvePartyMembers_ = std::move(cb);
  }

  /** Segundos até uma sala sem todos prontos se desfazer (config instance.lobby_timeout_seconds). */
  void setLobbyTimeoutSeconds(uint32_t seconds) { lobbyTimeoutSeconds_ = std::max<uint32_t>(15, seconds); }

  /**
   * 134 (sala da instância): Open pelo NPC portal valida portal/nível/grupo e abre a sala para os
   * membros do grupo na zone (137). Ready/Unready marcam; com todos prontos, a zone revalida e
   * teleporta todos para a mesma cópia. Cancel desfaz a sala para todos.
   */
  void handleLobbyAction(uint32_t playerId, uint32_t portalNpcInstanceId, uint8_t action, uint8_t value = 0);

  /** Etapa 5: recompensas e lockout de dungeon. */
  void setDungeonService(DungeonService* service) { dungeonService_ = service; }

  /** Etapa 5: NPC de cópia morreu (combat worker). Atualiza o progresso e conclui a dungeon. */
  void onInstanceNpcKilled(uint32_t instanceId, uint32_t npcInstanceId, uint8_t dungeonRole,
                           uint32_t killerPlayerId);
  /** 136: volta ao ponto de saída (frente do portal). */
  void handleLeave(uint32_t playerId);
  /** Desconectou de vez: só tira da lista (o autosave já grava o ponto de saída). */
  void onPlayerGone(uint32_t playerId);
  /** 1 s: fecha cópias vazias ou com tempo esgotado. */
  void tick();

  /** Admin TCP: list_instances / close_instance. */
  nlohmann::json listJson() const;
  bool closeInstance(uint32_t instanceId, const std::string& reason);

 private:
  struct ExitPoint {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
  };
  struct ActiveInstance {
    uint32_t instanceId = 0;
    uint32_t templateId = 0;
    uint32_t slot = 0;
    uint32_t ownerPlayerId = 0;
    float originX = 0.f;
    float originY = 0.f;
    float originZ = 0.f;
    std::unordered_map<uint32_t, ExitPoint> members;
    std::chrono::steady_clock::time_point createdAt{};
    std::chrono::steady_clock::time_point emptySince{};
    bool hasDeadline = false;
    std::chrono::steady_clock::time_point deadline{};
    uint32_t expiresAtUnix = 0;
    // ---- Dungeon (etapa 5) ----
    std::string difficultyKey = "normal";
    std::string difficultyName = "Normal";
    float rewardMult = 1.f;
    uint32_t enemiesTotal = 0;
    uint32_t bossesTotal = 0;
    uint32_t finalBossesTotal = 0;
    uint32_t enemiesKilled = 0;
    uint32_t bossesKilled = 0;
    uint32_t finalBossesKilled = 0;
    bool completed = false;
    std::chrono::steady_clock::time_point completionCloseAt{};
  };

  /** Sala de espera (ready check) de um grupo para um molde. */
  struct Lobby {
    uint32_t lobbyId = 0;
    uint32_t templateId = 0;
    uint32_t openerPlayerId = 0;
    /** Ordem de exibição (quem abriu primeiro). */
    std::vector<uint32_t> members;
    std::unordered_map<uint32_t, std::string> names;
    std::unordered_map<uint32_t, uint32_t> levels;
    std::unordered_map<uint32_t, bool> ready;
    std::chrono::steady_clock::time_point deadline{};
    /** >0: o grupo já está nesta cópia (quem confirmar entra direto). */
    uint32_t existingInstanceId = 0;
    /** Etapa 5: índice em InstanceTemplateDef::difficulties. */
    size_t difficultyIndex = 0;
    /** Etapa 5: dificuldades em lockout por membro. */
    std::unordered_map<uint32_t, std::vector<std::string>> lockedDifficulties;
  };

  void openLobby(uint32_t playerId, uint32_t portalNpcInstanceId);
  void setReady(uint32_t playerId, bool ready);
  /** Etapa 5: só quem abriu troca; zera os prontos. */
  void setDifficulty(uint32_t playerId, uint8_t index);
  /** Sob mu_: nível mínimo efetivo (molde x dificuldade da sala). */
  uint32_t lobbyMinLevelLocked(const Lobby& lobby) const;
  /** Sob mu_: 138 para os membros da cópia (ou só para `onlyPlayerId`). */
  void sendDungeonProgressLocked(const ActiveInstance& inst, uint8_t state, uint32_t onlyPlayerId = 0);
  void cancelLobby(uint32_t playerId);
  /** Sob mu_: manda o 137 (aberta/atualizada) para todos os membros. */
  void broadcastLobbyLocked(const Lobby& lobby, const std::string& info);
  /** Sob mu_: desfaz a sala, manda o 137 Fechada (reason) e libera os membros. */
  void closeLobbyLocked(uint32_t lobbyId, const std::string& reason);
  /** Sob mu_: todos prontos → revalida e leva todos para a mesma cópia. */
  void launchLobbyLocked(uint32_t lobbyId);
  /**
   * Sob mu_: coloca o jogador na cópia `target` (cria se nullptr). Manda o 135, troca a camada e
   * pede o despawn/snapshot de NPCs. Retorna a cópia usada (nullptr se não havia slot).
   */
  ActiveInstance* enterInstanceLocked(uint32_t playerId, const InstanceTemplateDef& tpl, ActiveInstance* target,
                                      uint32_t ownerPlayerId, size_t difficultyIndex = 0);
  /** Sob mu_: cópia aberta deste molde do próprio jogador ou de alguém do grupo (nullptr se não houver). */
  ActiveInstance* findPartyInstanceLocked(uint32_t templateId, uint32_t playerId,
                                          const std::vector<uint32_t>& party);

  void refuse(uint32_t playerId, uint32_t templateId, const std::string& reason);
  /** Sob mu_: tira o jogador da cópia, manda o 135 Left e o devolve ao mundo aberto. */
  void removeMemberLocked(ActiveInstance& inst, uint32_t playerId, const std::string& reason, bool online);
  uint32_t playerLevel(uint32_t playerId);
  void closeInstanceLocked(uint32_t instanceId, const std::string& reason);

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_ = 0;
  MovementServer* movement_ = nullptr;
  CombatCoreEngine* combat_ = nullptr;
  DungeonService* dungeonService_ = nullptr;
  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;

  mutable std::mutex mu_;
  std::unordered_map<uint32_t, InstanceTemplateDef> templates_;
  std::unordered_map<uint32_t, ActiveInstance> active_;
  std::unordered_map<uint32_t, uint32_t> instanceByPlayer_;
  uint32_t nextInstanceId_ = 1;

  std::unordered_map<uint32_t, Lobby> lobbies_;
  std::unordered_map<uint32_t, uint32_t> lobbyByPlayer_;
  uint32_t nextLobbyId_ = 1;
  uint32_t lobbyTimeoutSeconds_ = 120;
};

}  // namespace Zone
}  // namespace Umbra
