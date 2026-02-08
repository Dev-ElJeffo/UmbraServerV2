add_library(umbra_database STATIC
  MySQLConnector.cpp
  PlayerDAO.cpp
  AccountDAO.cpp
)

# Find MySQL (libmysqlclient)
# Prioridade: MySQL Server 8.0 standalone (não WAMP)
find_path(MYSQL_INCLUDE_DIR mysql.h
  PATHS
  "C:/Program Files/MySQL/MySQL Server 8.0/include"
  "C:/Program Files/MySQL/MySQL Server */include"
  /usr/include/mysql
  /usr/local/include/mysql
  /usr/include
  "C:/wamp64/bin/mysql/mysql*/include"
  REQUIRED
)

find_library(MYSQL_LIBRARY
  NAMES libmysql mysqlclient mysqlclient_r
  PATHS
  "C:/Program Files/MySQL/MySQL Server 8.0/lib"
  "C:/Program Files/MySQL/MySQL Server */lib"
  /usr/lib/mysql
  /usr/local/lib/mysql
  /usr/lib
  "C:/wamp64/bin/mysql/mysql*/lib"
  REQUIRED
)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  message(STATUS "MySQL found: ${MYSQL_INCLUDE_DIR}")
  target_include_directories(umbra_database PRIVATE ${MYSQL_INCLUDE_DIR})
  target_link_libraries(umbra_database ${MYSQL_LIBRARY})
  set(MYSQL_LIBRARY ${MYSQL_LIBRARY} PARENT_SCOPE)
else()
  message(WARNING "MySQL not found. MySQLConnector will not work. Install MySQL development libraries.")
endif()

target_link_libraries(umbra_database
  umbra_core
  Threads::Threads
)

target_include_directories(umbra_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

