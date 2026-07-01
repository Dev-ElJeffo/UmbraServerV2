add_library(umbra_zone STATIC
  ZoneServer.cpp
  ZoneCombatService.cpp
  PlayerManager.cpp
  EntitySystem.cpp
  ZoneOrchestrator.cpp
  NpcManager.cpp
  CombatCoreEngine.cpp
  CharacterStateLoader.cpp
  ReactionEngine.cpp
  ExperienceService.cpp
  ExpZoneManager.cpp
  MovementServer.hpp
  MovementProtocol.hpp
  SpatialGrid.hpp
  ZoneOrchestrator.hpp
  ZoneCombatService.hpp
  NpcManager.hpp
  CombatCoreEngine.hpp
  CharacterStateLoader.hpp
  ReactionEngine.hpp
)

target_link_libraries(umbra_zone
  umbra_core
  umbra_network
  umbra_database
  umbra_services
  umbra_auth
  Threads::Threads
)

target_include_directories(umbra_zone PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
