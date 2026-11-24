#pragma once
// DungeonService.hpp — conclusão de dungeon (etapa 5, opcode 139): lockout por período, EXP/ouro e itens
// sorteados da dungeon_reward_items. Roda numa thread própria (DB fora do tick e do combat worker).

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace Umbra {
namespace Database {
class MySQLConnector;
}
namespace Zone {

class MovementServer;
class LootService;
class QuestProgressService;

/** Linha de dungeon_reward_items. */
struct DungeonRewardEntry {
  uint32_t itemTemplateId = 0;
  std::string difficultyKey;  // vazio = todas
  float dropChance = 1.f;
  uint32_t minQty = 1;
  uint32_t maxQty = 1;
};

struct DungeonRolledItem {
  uint32_t itemTemplateId = 0;
  uint32_t quantity = 0;
};

/** Sorteia os itens de conclusão de UM jogador (puro; rng injetável para teste). */
std::vector<DungeonRolledItem> rollDungeonRewards(const std::vector<DungeonRewardEntry>& entries,
                                                  const std::string& difficultyKey, std::mt19937& rng);

/** Expressão SQL do período do lockout: daily = hoje; weekly = segunda-feira; none = data fixa. */
std::string dungeonPeriodKeySql(const std::string& lockoutReset);

/** A dungeon terminou? mode "all_bosses" = todos os chefes; "final_boss" = chefes finais (sem eles, todos os
 *  chefes; sem chefes, todos os inimigos). */
bool dungeonCompletionReached(const std::string& mode, uint32_t bossesKilled, uint32_t bossesTotal,
                              uint32_t finalKilled, uint32_t finalTotal, uint32_t enemiesKilled,
                              uint32_t enemiesTotal);

struct DungeonCompletionJob {
  uint32_t instanceTemplateId = 0;
  std::string dungeonName;
  std::string difficultyKey;
  std::string difficultyName;
  std::string lockoutReset = "daily";
  uint32_t durationSeconds = 0;
  int64_t rewardExp = 0;
  uint32_t rewardGold = 0;
  std::vector<uint32_t> players;
};

class DungeonService {
 public:
  DungeonService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movement, LootService* loot,
                 QuestProgressService* quests);
  ~DungeonService();

  void start();
  void stop();

  /** Enfileira a conclusão (recompensa + 139 + evento complete_dungeon). */
  void enqueueCompletion(DungeonCompletionJob job);

  /** Dificuldades deste molde em que o jogador já recebeu a recompensa no período (síncrono, DB). */
  std::vector<std::string> lockedDifficulties(uint32_t playerId, uint32_t instanceTemplateId,
                                              const std::string& lockoutReset);

 private:
  void workerLoop();
  void processCompletion(const DungeonCompletionJob& job);
  std::vector<DungeonRewardEntry> loadRewardEntries(uint32_t instanceTemplateId);
  std::string itemName(uint32_t itemTemplateId);
  /** Correio do sistema com os itens que não couberam. Retorna o mail_id (0 se falhou). */
  uint64_t mailItems(uint32_t playerId, const std::string& subject, const std::string& body,
                     const std::vector<DungeonRolledItem>& items);

  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movement_ = nullptr;
  LootService* loot_ = nullptr;
  QuestProgressService* quests_ = nullptr;

  std::mutex mu_;
  std::condition_variable cv_;
  std::deque<DungeonCompletionJob> queue_;
  std::thread worker_;
  bool running_ = false;
  std::mt19937 rng_{std::random_device{}()};
};

}  // namespace Zone
}  // namespace Umbra
