add_library(umbra_core STATIC
  ConfigManager.cpp
  Logger.cpp
  Timer.cpp
  Utils.cpp
)

# Find OpenSSL for password hashing (PBKDF2)
find_package(OpenSSL REQUIRED)

target_link_libraries(umbra_core
  nlohmann_json::nlohmann_json
  spdlog::spdlog
  OpenSSL::Crypto
  Threads::Threads
)

target_include_directories(umbra_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

