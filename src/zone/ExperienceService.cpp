#include "zone/ExperienceService.hpp"
#include "zone/CharacterStateLoader.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <climits>

namespace Umbra {
namespace Zone {

namespace {
constexpr uint32_t kMaxPlayerLevel = 50;
constexpr uint32_t kDefaultStatPointsPerLevel = 10;
}  // namespace

ExperienceService::ExperienceService(std::shared_ptr<Database::MySQLConnector> db)
    : db_(std::move(db)) {}

void ExperienceService::ensurePointsRows(uint32_t playerId, uint32_t level) {
  if (!db_ || !db_->isConnected()) return;
  const std::string pid = std::to_string(playerId);
  const std::string totalSkill = std::to_string(level * 3);

  auto statExists = db_->executePreparedScalar(
      "SELECT id FROM player_stat_points WHERE player_id = ? LIMIT 1", {pid});
  if (!statExists) {
    db_->executePreparedInsert(
        "INSERT INTO player_stat_points (player_id, unspent_points) VALUES (?, 0)", {pid});
  }

  auto skillExists = db_->executePreparedScalar(
      "SELECT player_id FROM player_skill_points WHERE player_id = ? LIMIT 1", {pid});
  if (!skillExists) {
    db_->executePreparedInsert(
        "INSERT INTO player_skill_points (player_id, total_points_earned, points_spent, points_available) "
        "VALUES (?, ?, 0, ?)",
        {pid, totalSkill, totalSkill});
  }
}

void ExperienceService::computeExpProgress(uint32_t level, int64_t totalExperience,
                                           int32_t& outExpForNext,
                                           int32_t& outExpInCurrentLevel,
                                           float& outProgressPercent) const {
  outExpForNext = 1000;
  outExpInCurrentLevel = 0;
  outProgressPercent = 0.f;
  if (!db_ || !db_->isConnected()) return;

  auto rows = db_->executePreparedQuery(
      "SELECT exp_required, exp_for_next_level FROM player_levels WHERE level_number = ?",
      {std::to_string(level)});
  if (rows.empty() || rows[0].size() < 2) return;

  const int64_t expRequiredCurrent = std::stoll(rows[0][0]);
  outExpForNext = std::stoi(rows[0][1]);
  int64_t expInLevel = totalExperience - expRequiredCurrent;
  if (expInLevel < 0) expInLevel = 0;
  outExpInCurrentLevel = static_cast<int32_t>(std::min<int64_t>(expInLevel, INT32_MAX));
  if (outExpForNext > 0) {
    outProgressPercent = static_cast<float>(expInLevel) / static_cast<float>(outExpForNext) * 100.f;
  }
  outProgressPercent = std::clamp(outProgressPercent, 0.f, 100.f);
}

ExperienceGrantResult ExperienceService::grantExperience(uint32_t playerId, int64_t amount,
                                                         const std::string& source) {
  ExperienceGrantResult result;
  result.source = source;

  if (amount <= 0 || !db_ || !db_->isConnected()) {
    return result;
  }

  const std::string pid = std::to_string(playerId);
  auto playerRows = db_->executePreparedQuery(
      "SELECT level, experience FROM players WHERE id = ?", {pid});
  if (playerRows.empty() || playerRows[0].size() < 2) {
    Core::Logger::getInstance().warn("[ExperienceService] player {} not found", playerId);
    return result;
  }

  uint32_t oldLevel = static_cast<uint32_t>(std::stoul(playerRows[0][0]));
  int64_t experience = std::stoll(playerRows[0][1]);
  result.oldLevel = oldLevel;
  result.expGranted = amount;
  experience += amount;

  ensurePointsRows(playerId, oldLevel);

  int32_t skillBefore = 0;
  if (auto skillOpt = db_->executePreparedScalar(
          "SELECT points_available FROM player_skill_points WHERE player_id = ?", {pid})) {
    skillBefore = std::stoi(*skillOpt);
  }

  uint32_t newLevel = oldLevel;
  uint32_t statPointsFromTable = 0;
  while (newLevel < kMaxPlayerLevel) {
    auto nextRows = db_->executePreparedQuery(
        "SELECT exp_required, stat_points_gained FROM player_levels WHERE level_number = ?",
        {std::to_string(newLevel + 1)});
    if (nextRows.empty() || nextRows[0].size() < 2) break;

    const int64_t required = std::stoll(nextRows[0][0]);
    if (experience < required) break;

    statPointsFromTable += static_cast<uint32_t>(std::stoul(nextRows[0][1]));
    ++newLevel;
  }

  computeExpProgress(newLevel, experience, result.expForNextLevel, result.expInCurrentLevel,
                     result.expProgressPercent);

  const bool updated = db_->executePreparedInsert(
      "UPDATE players SET level = ?, experience = ?, next_level_exp = ? WHERE id = ?",
      {std::to_string(newLevel), std::to_string(experience),
       std::to_string(result.expForNextLevel), pid});

  if (!updated) {
    Core::Logger::getInstance().error("[ExperienceService] failed to update player {}", playerId);
    return result;
  }

  if (newLevel > oldLevel) {
    const std::string totalSkill = std::to_string(newLevel * 3);
    db_->executePreparedInsert(
        "UPDATE player_skill_points SET total_points_earned = ?, points_available = ? - points_spent "
        "WHERE player_id = ?",
        {totalSkill, totalSkill, pid});
  }

  result.newLevel = newLevel;
  result.levelsGained = newLevel - oldLevel;
  result.totalExperience = experience;
  result.statPointsGained = statPointsFromTable > 0
                                ? statPointsFromTable
                                : result.levelsGained * kDefaultStatPointsPerLevel;
  result.success = true;

  int32_t skillAfter = skillBefore;
  if (auto skillOpt = db_->executePreparedScalar(
          "SELECT points_available FROM player_skill_points WHERE player_id = ?", {pid})) {
    skillAfter = std::stoi(*skillOpt);
  }
  result.skillPointsGained =
      static_cast<uint32_t>(std::max(0, skillAfter - skillBefore));

  if (stateLoader_) {
    stateLoader_->invalidate(playerId);
  }

  Core::Logger::getInstance().info(
      "[ExperienceService] grant player={} amount={} source={} level {}->{} stat+={} skill+={}",
      playerId, amount, source, oldLevel, newLevel, result.statPointsGained,
      result.skillPointsGained);

  return result;
}

}  // namespace Zone
}  // namespace Umbra
