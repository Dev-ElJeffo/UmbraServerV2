add_library(umbra_services STATIC
  Matchmaking.cpp
  InventoryService.cpp
  CombatService.cpp
  SkillService.cpp
)

target_link_libraries(umbra_services
  umbra_core
  umbra_database
  Threads::Threads
)

target_include_directories(umbra_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

