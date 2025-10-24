add_library(umbra_network STATIC
  SocketServer.cpp
  WebSocketServer.cpp
  MessageHandler.cpp
)

target_link_libraries(umbra_network
  umbra_core
  Threads::Threads
)

target_include_directories(umbra_network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

