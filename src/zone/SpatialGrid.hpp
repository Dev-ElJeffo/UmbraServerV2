#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <cmath>
#include <functional>

namespace Umbra {
namespace Zone {

struct GridCoord {
  int32_t x;
  int32_t y;
  bool operator==(const GridCoord& o) const { return x == o.x && y == o.y; }
};

struct GridCoordHash {
  size_t operator()(const GridCoord& c) const {
    return std::hash<int64_t>()(static_cast<int64_t>(c.x) << 32 | static_cast<uint32_t>(c.y));
  }
};

/**
 * Grid espacial 2D para Area of Interest (AOI).
 * Divide o mundo em células de tamanho fixo.
 * Jogadores só recebem updates de jogadores nas células adjacentes (3x3).
 * Thread-safe.
 */
class SpatialGrid {
 public:
  explicit SpatialGrid(float cellSize = 200.0f)
    : cellSize_(cellSize), invCellSize_(1.0f / cellSize) {}

  GridCoord worldToCell(float x, float y) const {
    return { static_cast<int32_t>(std::floor(x * invCellSize_)),
             static_cast<int32_t>(std::floor(y * invCellSize_)) };
  }

  void addPlayer(uint32_t clientId, float x, float y) {
    std::lock_guard<std::mutex> lock(mutex_);
    GridCoord cell = worldToCell(x, y);
    cells_[cell].insert(clientId);
    playerCells_[clientId] = cell;
  }

  void removePlayer(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playerCells_.find(clientId);
    if (it == playerCells_.end()) return;
    cells_[it->second].erase(clientId);
    if (cells_[it->second].empty()) cells_.erase(it->second);
    playerCells_.erase(it);
  }

  void updatePlayer(uint32_t clientId, float x, float y) {
    std::lock_guard<std::mutex> lock(mutex_);
    GridCoord newCell = worldToCell(x, y);
    auto it = playerCells_.find(clientId);
    if (it == playerCells_.end()) {
      cells_[newCell].insert(clientId);
      playerCells_[clientId] = newCell;
      return;
    }
    if (it->second == newCell) return;
    cells_[it->second].erase(clientId);
    if (cells_[it->second].empty()) cells_.erase(it->second);
    cells_[newCell].insert(clientId);
    it->second = newCell;
  }

  /**
   * Retorna todos os clientIds nas 9 células ao redor de (x, y).
   * Este é o "interesse" de um jogador naquela posição.
   */
  std::vector<uint32_t> getNearbyPlayers(float x, float y) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint32_t> result;
    GridCoord center = worldToCell(x, y);
    for (int32_t dx = -1; dx <= 1; ++dx) {
      for (int32_t dy = -1; dy <= 1; ++dy) {
        GridCoord neighbor = { center.x + dx, center.y + dy };
        auto it = cells_.find(neighbor);
        if (it != cells_.end()) {
          result.insert(result.end(), it->second.begin(), it->second.end());
        }
      }
    }
    return result;
  }

  /**
   * Retorna todos os clientIds nas 9 células ao redor do jogador dado.
   */
  std::vector<uint32_t> getNearbyPlayers(uint32_t clientId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playerCells_.find(clientId);
    if (it == playerCells_.end()) return {};
    std::vector<uint32_t> result;
    const GridCoord& center = it->second;
    for (int32_t dx = -1; dx <= 1; ++dx) {
      for (int32_t dy = -1; dy <= 1; ++dy) {
        GridCoord neighbor = { center.x + dx, center.y + dy };
        auto cit = cells_.find(neighbor);
        if (cit != cells_.end()) {
          for (uint32_t pid : cit->second) {
            if (pid != clientId) result.push_back(pid);
          }
        }
      }
    }
    return result;
  }

  /** Executa callback para cada jogador próximo a (x, y), incluindo o próprio. */
  void forEachNearby(float x, float y, const std::function<void(uint32_t)>& callback) const {
    std::lock_guard<std::mutex> lock(mutex_);
    GridCoord center = worldToCell(x, y);
    for (int32_t dx = -1; dx <= 1; ++dx) {
      for (int32_t dy = -1; dy <= 1; ++dy) {
        GridCoord neighbor = { center.x + dx, center.y + dy };
        auto it = cells_.find(neighbor);
        if (it != cells_.end()) {
          for (uint32_t pid : it->second) {
            callback(pid);
          }
        }
      }
    }
  }

  size_t getPlayerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return playerCells_.size();
  }

  size_t getCellCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cells_.size();
  }

  float getCellSize() const { return cellSize_; }

 private:
  float cellSize_;
  float invCellSize_;
  std::unordered_map<GridCoord, std::unordered_set<uint32_t>, GridCoordHash> cells_;
  std::unordered_map<uint32_t, GridCoord> playerCells_;
  mutable std::mutex mutex_;
};

}  // namespace Zone
}  // namespace Umbra
