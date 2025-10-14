#pragma once

#include "database/Models.hpp"
#include <vector>
#include <cstdint>

namespace Umbra {
namespace Services {

class InventoryService {
 public:
  static constexpr uint32_t MAX_INVENTORY_SLOTS = 50;
  
  bool addItem(uint64_t playerId, const Database::Item& item);
  bool removeItem(uint64_t playerId, uint64_t itemId, uint32_t quantity = 1);
  bool moveItem(uint64_t playerId, uint32_t fromSlot, uint32_t toSlot);
  
  std::vector<Database::InventorySlot> getInventory(uint64_t playerId);
};

}  // namespace Services
}  // namespace Umbra

