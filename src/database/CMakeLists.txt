add_library(umbra_database STATIC
  MySQLConnector.cpp
  PlayerDAO.cpp
  AccountDAO.cpp
)

target_link_libraries(umbra_database
  umbra_core
  Threads::Threads
)

# Link MySQL connector if available
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp")
  target_link_libraries(umbra_database mysqlcppconn)
endif()

target_include_directories(umbra_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

