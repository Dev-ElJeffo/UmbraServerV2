include_directories(${CMAKE_SOURCE_DIR}/src)

# Test executables
add_executable(test_auth
  test_auth.cpp
)

target_link_libraries(test_auth
  umbra_core
  umbra_auth
  umbra_database
  gtest
  gtest_main
  Threads::Threads
)

add_executable(test_db
  test_db.cpp
)

target_link_libraries(test_db
  umbra_core
  umbra_database
  gtest
  gtest_main
  Threads::Threads
)

add_executable(test_zone
  test_zone.cpp
)

target_link_libraries(test_zone
  umbra_core
  umbra_zone
  umbra_database
  gtest
  gtest_main
  Threads::Threads
)

add_executable(test_network
  test_network.cpp
)

target_link_libraries(test_network
  umbra_core
  umbra_network
  gtest
  gtest_main
  Threads::Threads
)

# Teste das implementações da Fase 1
find_package(OpenSSL REQUIRED)
add_executable(test_fase1 ../scripts_main/test_fase1_implementations.cpp)
target_link_libraries(test_fase1
  umbra_core
  umbra_auth
  umbra_database
  OpenSSL::Crypto
  Threads::Threads
)

# Teste das implementações da Fase 2 - PlayerDAO Parsing
add_executable(test_playerdao ../scripts_main/test_playerdao_parsing.cpp)
target_link_libraries(test_playerdao
  umbra_core
  umbra_database
  Threads::Threads
)

# Add tests to CTest
add_test(NAME AuthTests COMMAND test_auth)
add_test(NAME DatabaseTests COMMAND test_db)
add_test(NAME ZoneTests COMMAND test_zone)
add_test(NAME NetworkTests COMMAND test_network)
add_test(NAME Fase1Tests COMMAND test_fase1)
add_test(NAME PlayerDAOTests COMMAND test_playerdao)

