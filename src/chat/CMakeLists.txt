add_library(umbra_chat STATIC
  ChatServer.cpp
  ChannelManager.cpp
)

target_link_libraries(umbra_chat
  umbra_core
  umbra_network
  Threads::Threads
)

target_include_directories(umbra_chat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

