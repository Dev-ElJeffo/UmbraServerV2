// DungeonService.cpp — conclusão de dungeon (etapa 5). Ver DungeonService.hpp.
#include "zone/DungeonService.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "zone/LootService.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"
#include "zone/QuestProgressService.hpp"

namespace Umbra {
namespace Zone {

namespace {
uint32_t toU32(const std::string& s, uint32_t def) {
  if (s.empty()) return def;
  try {
    return static_cast<uint32_t>(std::stoul(s));
  } catch (...) {
    return def;
  }
}
float toF(const std::string& s, float def) {
  if (s.empty()) return def;
  try {
    return std::stof(s);
  } catch (...) {
    return def;
  }
}
}  // namespace

// ---------------- Funções puras ----------------

std::vector<DungeonRolledItem> rollDungeonRewards(const std::vector<DungeonRewardEntry>& entries,
                                                  const std::string& difficultyKey, std::mt19937& rng) {
  std::vector<DungeonRolledItem> out;
  std::uniform_real_distribution<float> chance(0.f, 1.f);
  for (const auto& e : entries) {
    if (e.itemTemplateId == 0) continue;
    if (!e.difficultyKey.empty() && e.difficultyKey != difficultyKey) continue;
    const float c = std::clamp(e.dropChance, 0.f, 1.f);
    if (c <= 0.f) continue;
    if (c < 1.f && chance(rng) >= c) continue;
    const uint32_t lo = std::max(1u, e.minQty);
    const uint32_t hi = std::max(lo, e.maxQty);
    std::uniform_int_distribution<uint32_t> qty(lo, hi);
    out.push_back({e.itemTemplateId, qty(rng)});
  }
  return out;
}

std::string dungeonPeriodKeySql(const std::string& lockoutReset) {
  if (lockoutReset == "weekly") return "DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY)";
  if (lockoutReset == "none") return "'2000-01-01'";
  return "CURDATE()";
}

bool dungeonCompletionReached(const std::string& mode, uint32_t bossesKilled, uint32_t bossesTotal,
                              uint32_t finalKilled, uint32_t finalTotal, uint32_t enemiesKilled,
                              uint32_t enemiesTotal) {
  if (mode != "all_bosses" && finalTotal > 0) return finalKilled >= finalTotal;
  if (bossesTotal > 0) return bossesKilled >= bossesTotal;
  return enemiesTotal > 0 && enemiesKilled >= enemiesTotal;
}

// ---------------- DungeonService ----------------

DungeonService::DungeonService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movement,
                               LootService* loot, QuestProgressService* quests)
    : db_(std::move(db)), movement_(movement), loot_(loot), quests_(quests) {}

DungeonService::~DungeonService() { stop(); }

void DungeonService::start() {
  std::lock_guard<std::mutex> lock(mu_);
  if (running_) return;
  running_ = true;
  worker_ = std::thread([this]() { workerLoop(); });
}

void DungeonService::stop() {
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (!running_) return;
    running_ = false;
  }
  cv_.notify_all();
  if (worker_.joinable()) worker_.join();
}

void DungeonService::enqueueCompletion(DungeonCompletionJob job) {
  {
    std::lock_guard<std::mutex> lock(mu_);
    queue_.push_back(std::move(job));
  }
  cv_.notify_one();
}

void DungeonService::workerLoop() {
  for (;;) {
    DungeonCompletionJob job;
    {
      std::unique_lock<std::mutex> lock(mu_);
      cv_.wait(lock, [this]() { return !running_ || !queue_.empty(); });
      if (queue_.empty()) {
        if (!running_) return;
        continue;
      }
      job = std::move(queue_.front());
      queue_.pop_front();
    }
    try {
      processCompletion(job);
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("[DungeonService] exceção na conclusão: {}", e.what());
    } catch (...) {
      Core::Logger::getInstance().error("[DungeonService] exceção desconhecida na conclusão");
    }
  }
}

std::vector<std::string> DungeonService::lockedDifficulties(uint32_t playerId, uint32_t instanceTemplateId,
                                                            const std::string& lockoutReset) {
  std::vector<std::string> out;
  if (!db_ || !db_->isConnected() || playerId == 0) return out;
  auto rows = db_->executePreparedQuery(
      "SELECT DISTINCT difficulty_key FROM dungeon_completions WHERE player_id = ? AND instance_template_id = ? "
      "AND rewarded = 1 AND period_key = " + dungeonPeriodKeySql(lockoutReset),
      {std::to_string(playerId), std::to_string(instanceTemplateId)});
  for (const auto& r : rows) {
    if (!r.empty()) out.push_back(r[0]);
  }
  return out;
}

std::vector<DungeonRewardEntry> DungeonService::loadRewardEntries(uint32_t instanceTemplateId) {
  std::vector<DungeonRewardEntry> out;
  if (!db_) return out;
  auto rows = db_->executePreparedQuery(
      "SELECT item_template_id, difficulty_key, drop_chance, min_qty, max_qty FROM dungeon_reward_items "
      "WHERE instance_template_id = ? AND is_active = 1",
      {std::to_string(instanceTemplateId)});
  for (const auto& r : rows) {
    if (r.size() < 5) continue;
    DungeonRewardEntry e;
    e.itemTemplateId = toU32(r[0], 0);
    e.difficultyKey = r[1];
    e.dropChance = toF(r[2], 1.f);
    e.minQty = toU32(r[3], 1);
    e.maxQty = toU32(r[4], 1);
    out.push_back(std::move(e));
  }
  return out;
}

std::string DungeonService::itemName(uint32_t itemTemplateId) {
  if (!db_) return {};
  auto v = db_->executePreparedScalar("SELECT item_name FROM item_templates WHERE item_id = ? LIMIT 1",
                                      {std::to_string(itemTemplateId)});
  return v ? *v : std::string();
}

uint64_t DungeonService::mailItems(uint32_t playerId, const std::string& subject, const std::string& body,
                                   const std::vector<DungeonRolledItem>& items) {
  if (!db_ || items.empty()) return 0;
  const size_t n = std::min<size_t>(items.size(), 10);
  if (!db_->executePreparedInsert(
          "INSERT INTO mail_messages (sender_player_id, recipient_player_id, subject, body, attachment_count) "
          "VALUES (NULL, ?, ?, ?, ?)",
          {std::to_string(playerId), subject, body, std::to_string(n)})) {
    return 0;
  }
  const uint64_t mailId = db_->getLastInsertId();
  if (mailId == 0) return 0;
  for (size_t i = 0; i < n; ++i) {
    db_->executePreparedInsert(
        "INSERT INTO mail_attachments (mail_id, slot_index, item_template_id, quantity, refinement_level, durability) "
        "VALUES (?, ?, ?, ?, 0, 100)",
        {std::to_string(mailId), std::to_string(i), std::to_string(items[i].itemTemplateId),
         std::to_string(items[i].quantity)});
  }
  if (movement_) movement_->notifyMailToPlayer(playerId, static_cast<uint32_t>(mailId), "Sistema", subject);
  return mailId;
}

void DungeonService::processCompletion(const DungeonCompletionJob& job) {
  if (!db_ || !db_->isConnected()) return;
  const auto entries = loadRewardEntries(job.instanceTemplateId);
  const std::string periodSql = dungeonPeriodKeySql(job.lockoutReset);
  std::unordered_map<uint32_t, std::string> nameCache;

  for (uint32_t pid : job.players) {
    // Lockout: já recebeu a recompensa desta dificuldade no período?
    auto cnt = db_->executePreparedScalar(
        "SELECT COUNT(*) FROM dungeon_completions WHERE player_id = ? AND instance_template_id = ? "
        "AND difficulty_key = ? AND rewarded = 1 AND period_key = " + periodSql,
        {std::to_string(pid), std::to_string(job.instanceTemplateId), job.difficultyKey});
    const bool locked = cnt && toU32(*cnt, 0) > 0;
    db_->executePreparedInsert(
        "INSERT INTO dungeon_completions (player_id, instance_template_id, difficulty_key, period_key, rewarded, "
        "duration_seconds) VALUES (?, ?, ?, " + periodSql + ", ?, ?)",
        {std::to_string(pid), std::to_string(job.instanceTemplateId), job.difficultyKey, locked ? "0" : "1",
         std::to_string(job.durationSeconds)});

    DungeonCompletedPayload out;
    out.templateId = job.instanceTemplateId;
    out.durationSeconds = job.durationSeconds;
    out.dungeonName = job.dungeonName;
    out.difficultyName = job.difficultyName;
    out.rewarded = locked ? 0 : 1;

    if (!locked && loot_) {
      if (job.rewardExp > 0) {
        out.exp = static_cast<uint32_t>(std::min<int64_t>(
            loot_->grantExperienceAndNotify(pid, job.rewardExp, "dungeon:" + std::to_string(job.instanceTemplateId)),
            0xFFFFFFFF));
      }
      if (job.rewardGold > 0) {
        out.playerGold = loot_->grantGold(pid, job.rewardGold);
        out.gold = job.rewardGold;
      }
      std::vector<DungeonRolledItem> toMail;
      for (const auto& item : rollDungeonRewards(entries, job.difficultyKey, rng_)) {
        DungeonRewardItemPayload ip;
        ip.itemTemplateId = item.itemTemplateId;
        ip.quantity = static_cast<uint16_t>(std::min<uint32_t>(item.quantity, 0xFFFF));
        auto nm = nameCache.find(item.itemTemplateId);
        if (nm == nameCache.end()) nm = nameCache.emplace(item.itemTemplateId, itemName(item.itemTemplateId)).first;
        ip.name = nm->second;
        // Só tenta a bolsa com slot livre (evita entrega parcial); senão vai para o correio.
        if (loot_->hasFreeInventorySlot(pid) && loot_->grantItem(pid, item.itemTemplateId, item.quantity)) {
          out.inventoryChanged = 1;
        } else {
          ip.mailed = 1;
          toMail.push_back(item);
        }
        out.items.push_back(std::move(ip));
      }
      if (!toMail.empty()) {
        mailItems(pid, "Recompensa: " + job.dungeonName,
                  "Sua bolsa estava cheia ao concluir " + job.dungeonName + ". Os itens estão anexados.", toMail);
      }
    }
    if (movement_) movement_->sendToPlayer(pid, encodeDungeonCompleted(out));

    // Quests v2 / missões de guilda: complete_dungeon.
    if (quests_) {
      QuestEvent ev;
      ev.kind = QuestObjectiveKind::CompleteDungeon;
      ev.key = job.instanceTemplateId;
      ev.subtype = job.difficultyKey;
      quests_->onEvent(pid, ev);
    }
    Core::Logger::getInstance().info(
        "[DungeonService] player {} concluiu {} ({}) em {}s: {} (exp={} ouro={} itens={})", pid, job.dungeonName,
        job.difficultyKey, job.durationSeconds, locked ? "lockout" : "recompensado", out.exp, out.gold,
        out.items.size());
  }
}

}  // namespace Zone
}  // namespace Umbra
