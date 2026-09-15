#include "services/StatKeyMapping.hpp"
#include <iostream>
#include <cstdlib>

int main() {
  using Umbra::Combat::StatKeyMapping::mapTargetStatToCanonical;
  if (mapTargetStatToCanonical("physical_attack") != "attack") {
    std::cerr << "physical_attack\n";
    return 1;
  }
  if (mapTargetStatToCanonical("physical_defense") != "defense") {
    std::cerr << "physical_defense\n";
    return 1;
  }
  if (mapTargetStatToCanonical("movement_speed") != "movement") {
    std::cerr << "movement_speed\n";
    return 1;
  }
  if (mapTargetStatToCanonical("all_resistance") != "all_resistance") {
    std::cerr << "all_resistance\n";
    return 1;
  }
  Umbra::Combat::CharacterStats stats;
  stats.physicalAttack = 100;
  Umbra::Combat::StatKeyMapping::applyFlatToCharacterStats("attack", 20, stats);
  Umbra::Combat::StatKeyMapping::applyPercentToCharacterStats("attack", 50, stats);
  if (stats.physicalAttack != 180) {
    std::cerr << "flat_then_percent\n";
    return 1;
  }
  Umbra::Combat::StatKeyMapping::applyFlatToCharacterStats("all_resistance", 15, stats);
  if (stats.physicalRes != 15 || stats.fireRes != 15 || stats.arcaneRes != 15) {
    std::cerr << "all_resistance_apply\n";
    return 1;
  }
  std::cout << "OK\n";
  return 0;
}
