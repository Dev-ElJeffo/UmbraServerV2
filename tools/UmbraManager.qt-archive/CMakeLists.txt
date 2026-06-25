cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(UMBRA_MANAGER_SOURCES
  main.cpp
  Core/AppConfig.cpp
  Core/AdminClient.cpp
  Core/ProcessManager.cpp
  Core/LogTailer.cpp
  Core/MetricsStore.cpp
  Core/PhpAdminClient.cpp
  Core/AuditLog.cpp
  Core/Scheduler.cpp
  Views/MainWindow.cpp
  Views/LoginDialog.cpp
  Views/DashboardWidget.cpp
  Views/ServersWidget.cpp
  Views/ZonesWidget.cpp
  Views/PlayersWidget.cpp
  Views/LogsWidget.cpp
  Views/AccountsWidget.cpp
  Views/ItemsWidget.cpp
  Views/ConfigEditorWidget.cpp
  Views/GmConsoleWidget.cpp
  Views/SchedulerWidget.cpp
)

add_executable(UmbraManager ${UMBRA_MANAGER_SOURCES})

target_include_directories(UmbraManager PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Core
  ${CMAKE_CURRENT_SOURCE_DIR}/Views
)

target_link_libraries(UmbraManager PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  Qt6::Sql
  Qt6::Charts
)

if(WIN32)
  set_target_properties(UmbraManager PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS UmbraManager RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/manager.json.example DESTINATION config)
