#pragma once

#include "database/MySQLConnector.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace Umbra {
namespace Zone {

class EnchantRoll {
 public:
  static bool isEquipmentTemplate(const std::string& itemCategory, const std::string& equipmentSlot,
                                  const std::string& itemType) {
    const std::string slot = toLower(equipmentSlot);
    if (slot.empty() || slot == "none") return false;
    const std::string cat = toLower(itemCategory);
    if (cat == "equipment") return true;
    const std::string type = toLower(itemType);
    return type == "weapon" || type == "armor";
  }

  static std::string rollSpawnJson(Database::MySQLConnector& db) {
    const int n = pickWeighted(loadSlotWeights(db));
    nlohmann::json arr = nlohmann::json::array();
    std::unordered_set<std::string> used;
    for (int slot = 0; slot < n; ++slot) {
      nlohmann::json affix = rollOneAffix(db, used);
      if (affix.is_null()) break;
      used.insert(affix.value("stat_key", std::string{}));
      affix["slot"] = slot;
      arr.push_back(affix);
    }
    if (arr.empty()) return "";
    return arr.dump();
  }

 private:
  static std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
  }

  static int randomInt(int minInc, int maxInc) {
    if (maxInc < minInc) std::swap(minInc, maxInc);
    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(minInc, maxInc);
    return dist(rng);
  }

  static int pickWeighted(const std::vector<std::pair<int, int>>& items) {
    int total = 0;
    for (const auto& it : items) total += std::max(0, it.second);
    if (total <= 0) return 0;
    int roll = randomInt(1, total);
    int acc = 0;
    for (const auto& it : items) {
      acc += std::max(0, it.second);
      if (roll <= acc) return it.first;
    }
    return items.empty() ? 0 : items.front().first;
  }

  static std::vector<std::pair<int, int>> loadSlotWeights(Database::MySQLConnector& db) {
    std::vector<std::pair<int, int>> out;
    auto rows = db.executeQuery("SELECT slot_count, weight FROM enchant_slot_chances");
    for (const auto& r : rows) {
      if (r.size() < 2) continue;
      const int n = std::stoi(r[0]);
      if (n < 0 || n > 3) continue;
      out.emplace_back(n, std::stoi(r[1]));
    }
    if (out.empty()) out.emplace_back(0, 1);
    return out;
  }

  static nlohmann::json rollOneAffix(Database::MySQLConnector& db,
                                     const std::unordered_set<std::string>& exclude) {
    struct Row {
      std::string key;
      int weight = 0;
      int vmin = 1;
      int vmax = 1;
    };
    std::vector<Row> rows;
    auto q = db.executeQuery(
        "SELECT stat_key, weight, value_min, value_max FROM enchant_stat_weights");
    std::vector<std::pair<int, int>> weights;
    for (const auto& r : q) {
      if (r.size() < 4) continue;
      Row row;
      row.key = r[0];
      if (row.key.empty() || exclude.count(row.key)) continue;
      row.weight = std::stoi(r[1]);
      row.vmin = std::stoi(r[2]);
      row.vmax = std::stoi(r[3]);
      weights.emplace_back(static_cast<int>(rows.size()), row.weight);
      rows.push_back(std::move(row));
    }
    if (rows.empty()) return nlohmann::json();
    const int idx = pickWeighted(weights);
    if (idx < 0 || idx >= static_cast<int>(rows.size())) return nlohmann::json();
    const Row& picked = rows[static_cast<size_t>(idx)];
    nlohmann::json j;
    j["stat_key"] = picked.key;
    j["value"] = randomInt(picked.vmin, picked.vmax);
    return j;
  }
};

}  // namespace Zone
}  // namespace Umbra
