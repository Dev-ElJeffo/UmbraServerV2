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
target_include_directories(test_fase1 PRIVATE ${CMAKE_SOURCE_DIR})
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

# Teste de Integração AuthServer + PlayerDAO - Fase 2
add_executable(test_auth_playerdao ../scripts_main/test_auth_playerdao_integration.cpp)
target_link_libraries(test_auth_playerdao
  umbra_core
  umbra_auth
  umbra_database
  OpenSSL::Crypto
  Threads::Threads
)

add_executable(test_stat_mapping
  test_stat_mapping.cpp
)
target_link_libraries(test_stat_mapping
  umbra_core
  Threads::Threads
)

add_executable(test_combat_effects
  test_combat_effects.cpp
)
target_link_libraries(test_combat_effects
  umbra_zone
  gtest
  gtest_main
  Threads::Threads
)

find_package(OpenSSL REQUIRED)
add_executable(test_admin_crypto
  test_admin_crypto.cpp
  ${CMAKE_SOURCE_DIR}/src/admin/AdminCrypto.cpp
)
target_include_directories(test_admin_crypto PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_admin_crypto
  umbra_core
  OpenSSL::Crypto
  Threads::Threads
)

# Add tests to CTest
add_test(NAME AuthTests COMMAND test_auth)
add_test(NAME DatabaseTests COMMAND test_db)
add_test(NAME ZoneTests COMMAND test_zone)
add_test(NAME NetworkTests COMMAND test_network)
add_test(NAME Fase1Tests COMMAND test_fase1)
add_test(NAME PlayerDAOTests COMMAND test_playerdao)
add_test(NAME AuthPlayerDAOIntegrationTests COMMAND test_auth_playerdao)
add_test(NAME StatMappingTests COMMAND test_stat_mapping)
add_test(NAME CombatEffectTests COMMAND test_combat_effects)
add_test(NAME AdminCryptoTests COMMAND test_admin_crypto)

