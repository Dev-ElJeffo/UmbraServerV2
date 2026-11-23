#pragma once

#include "database/MySQLConnector.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace Umbra {
namespace Zone {

class CharacterStateLoader;

struct ExperienceGrantResult {
  int64_t expGranted = 0;
  int64_t totalExperience = 0;
  uint32_t oldLevel = 0;
  uint32_t newLevel = 0;
  uint32_t levelsGained = 0;
  uint32_t statPointsGained = 0;
  uint32_t skillPointsGained = 0;
  int32_t expForNextLevel = 0;
  int32_t expInCurrentLevel = 0;
  float expProgressPercent = 0.f;
  std::string source;
  bool success = false;
};

class ExperienceService {
public:
  explicit ExperienceService(std::shared_ptr<Database::MySQLConnector> db);

  void setStateLoader(CharacterStateLoader* loader) { stateLoader_ = loader; }

  /** Multiplicador global de EXP (1.0 = normal). Carregado de game_rates. */
  void loadGameRates();
  double getExpMultiplier() const { return expMultiplier_; }

  ExperienceGrantResult grantExperience(uint32_t playerId, int64_t amount, const std::string& source);

  /** Subiu de nível (Quests v2: reach_level). Chamado fora de qualquer lock. */
  void setLevelUpCallback(std::function<void(uint32_t playerId, uint32_t newLevel)> cb) {
    onLevelUp_ = std::move(cb);
  }

private:
  void ensurePointsRows(uint32_t playerId, uint32_t level);
  void computeExpProgress(uint32_t level, int64_t totalExperience, int32_t& outExpForNext,
                          int32_t& outExpInCurrentLevel, float& outProgressPercent) const;

  std::shared_ptr<Database::MySQLConnector> db_;
  CharacterStateLoader* stateLoader_ = nullptr;
  double expMultiplier_ = 1.0;
  std::function<void(uint32_t, uint32_t)> onLevelUp_;
};

}  // namespace Zone
}  // namespace Umbra
