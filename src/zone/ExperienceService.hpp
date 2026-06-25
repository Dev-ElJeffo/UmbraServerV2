#pragma once

#include "database/MySQLConnector.hpp"
#include <cstdint>
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

  ExperienceGrantResult grantExperience(uint32_t playerId, int64_t amount, const std::string& source);

private:
  void ensurePointsRows(uint32_t playerId, uint32_t level);
  void computeExpProgress(uint32_t level, int64_t totalExperience, int32_t& outExpForNext,
                          int32_t& outExpInCurrentLevel, float& outProgressPercent) const;

  std::shared_ptr<Database::MySQLConnector> db_;
  CharacterStateLoader* stateLoader_ = nullptr;
};

}  // namespace Zone
}  // namespace Umbra
