add_library(umbra_world STATIC
  WorldServer.cpp
  EventManager.cpp
  TimeManager.cpp
)

target_link_libraries(umbra_world
  umbra_core
  umbra_network
  umbra_database
  Threads::Threads
)

target_include_directories(umbra_world PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

