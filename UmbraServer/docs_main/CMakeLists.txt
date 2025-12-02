cmake_minimum_required(VERSION 3.20)
project(UmbraServer VERSION 1.3.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
  add_compile_options(/W4 /O2)
  add_definitions(-D_WIN32_WINNT=0x0601)
else()
  add_compile_options(-Wall -Wextra -O2 -g)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Third-party libraries
add_subdirectory(third_party/json EXCLUDE_FROM_ALL)
add_subdirectory(third_party/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)

# Find packages
find_package(Threads REQUIRED)

# MySQL Connector (optional, configure path)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp")
  include_directories(${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp/include)
  link_directories(${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp/lib64)
endif()

# Source subdirectories
add_subdirectory(src/core)
add_subdirectory(src/network)
add_subdirectory(src/database)
add_subdirectory(src/auth)
add_subdirectory(src/world)
add_subdirectory(src/zone)
add_subdirectory(src/chat)
add_subdirectory(src/gateway)
add_subdirectory(src/services)

# Main executable (Gateway entry point)
add_executable(umbra_server src/main.cpp)
target_link_libraries(umbra_server 
  umbra_core
  umbra_network
  umbra_gateway
  umbra_auth
  umbra_world
  umbra_zone
  umbra_chat
  umbra_database
  umbra_services
  Threads::Threads
)

# Individual server executables
add_executable(auth_server src/auth/main_auth.cpp)
target_link_libraries(auth_server 
  umbra_core
  umbra_network
  umbra_auth
  umbra_database
  Threads::Threads
)

add_executable(world_server src/world/main_world.cpp)
target_link_libraries(world_server 
  umbra_core
  umbra_network
  umbra_world
  umbra_database
  Threads::Threads
)

add_executable(zone_server src/zone/main_zone.cpp)
target_link_libraries(zone_server 
  umbra_core
  umbra_network
  umbra_zone
  umbra_database
  umbra_services
  Threads::Threads
)

add_executable(chat_server src/chat/main_chat.cpp)
target_link_libraries(chat_server 
  umbra_core
  umbra_network
  umbra_chat
  Threads::Threads
)

add_executable(gateway_server src/gateway/main_gateway.cpp)
target_link_libraries(gateway_server 
  umbra_core
  umbra_network
  umbra_gateway
  umbra_auth
  Threads::Threads
)

# Tests
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS umbra_server auth_server world_server zone_server chat_server gateway_server
  RUNTIME DESTINATION bin
)

install(DIRECTORY config/ DESTINATION config
  PATTERN "*.key" EXCLUDE
  PATTERN "db.json" EXCLUDE
)

install(DIRECTORY scripts/ DESTINATION scripts
  USE_SOURCE_PERMISSIONS
)

# Print configuration
message(STATUS "UmbraServer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")

