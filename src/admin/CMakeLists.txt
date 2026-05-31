add_library(umbra_admin STATIC
  AdminServer.cpp
  AdminCommands.cpp
  AdminCrypto.cpp
  AdminStats.cpp
  ServiceAdminRegister.cpp
)

find_package(OpenSSL REQUIRED)

target_link_libraries(umbra_admin
  umbra_core
  umbra_network
  umbra_auth
  umbra_gateway
  umbra_zone
  umbra_world
  umbra_chat
  OpenSSL::Crypto
  Threads::Threads
)

target_include_directories(umbra_admin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  target_link_libraries(umbra_admin psapi)
endif()
