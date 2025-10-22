add_library(umbra_gateway STATIC
  GatewayServer.cpp
  LoadBalancer.cpp
  AuthClient.cpp
  AuthConnectionPool.cpp
)

target_link_libraries(umbra_gateway
  umbra_core
  umbra_network
  umbra_auth
  Threads::Threads
)

target_include_directories(umbra_gateway PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

