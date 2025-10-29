add_library(umbra_auth STATIC
  AuthServer.cpp
  JWTManager.cpp
  SessionManager.cpp
)

# Find OpenSSL for HMAC-SHA256
find_package(OpenSSL REQUIRED)

target_link_libraries(umbra_auth
  umbra_core
  umbra_network
  umbra_database
  OpenSSL::SSL
  OpenSSL::Crypto
  Threads::Threads
)

target_include_directories(umbra_auth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

