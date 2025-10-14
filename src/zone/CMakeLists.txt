add_library(umbra_zone STATIC
  ZoneServer.cpp
  PlayerManager.cpp
  EntitySystem.cpp
)

target_link_libraries(umbra_zone
  umbra_core
  umbra_network
  umbra_database
  Threads::Threads
)

target_include_directories(umbra_zone PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

