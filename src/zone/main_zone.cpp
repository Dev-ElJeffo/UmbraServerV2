#include "ZoneServer.hpp"
#include "zone/AgentDebugLog.hpp"
#include "admin/AdminBootstrap.hpp"
#include "admin/ServiceAdminRegister.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main(int argc, char* argv[]) {
  Umbra::Core::Logger::getInstance().initialize("logs/zone_server.log");

  int zoneId = (argc > 1) ? std::atoi(argv[1]) : 0;
  std::string zoneName = "Zone_" + std::to_string(zoneId);

  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  configManager.loadConfig("config/server.json");

  // Aplicar o nível de log do config (o initialize() acima deixa em DEBUG fixo).
  // Sem isto, logs DEBUG por frame de movimento inundam o arquivo (I/O no hot path).
  {
    const std::string lvl = configManager.get<std::string>("logging.level", "INFO");
    Umbra::Core::Logger::Level lg = Umbra::Core::Logger::Level::INFO;
    if (lvl == "DEBUG") lg = Umbra::Core::Logger::Level::DEBUG;
    else if (lvl == "WARN") lg = Umbra::Core::Logger::Level::WARN;
    else if (lvl == "ERROR") lg = Umbra::Core::Logger::Level::ERROR;
    else if (lvl == "CRITICAL") lg = Umbra::Core::Logger::Level::CRITICAL;
    Umbra::Core::Logger::getInstance().setLevel(lg);
  }

  const uint16_t zoneBase = configManager.get<uint16_t>("zone.base_port", 8082);
  uint16_t zonePort = static_cast<uint16_t>(zoneBase + zoneId);

  std::cout << "===========================================\n";
  std::cout << "  UmbraEternum Zone Server                \n";
  std::cout << "  Zone: " << zoneName << " | Port: " << zonePort << "\n";
  std::cout << "===========================================\n\n";

  Umbra::Core::Logger::getInstance().info("Starting Zone Server '{}' on port {}...", zoneName, zonePort);

  std::shared_ptr<Umbra::Database::MySQLConnector> dbConnector;
  {
    Umbra::Database::MySQLConnector::Config dbConfig;
    dbConfig.host = configManager.get<std::string>("database.host", "localhost");
    dbConfig.port = static_cast<uint16_t>(configManager.get<uint32_t>("database.port", 3306));
    dbConfig.database = configManager.get<std::string>("database.name", "umbra_eternum");
    dbConfig.username = configManager.get<std::string>("database.user", "root");
    dbConfig.password = configManager.get<std::string>("database.password", "");
    dbConfig.poolSize = configManager.get<uint32_t>("database.pool_size", 5);

    dbConnector = std::make_shared<Umbra::Database::MySQLConnector>(dbConfig);
    if (!dbConnector->connect()) {
      Umbra::Core::Logger::getInstance().warn("Zone: Database connection failed");
      std::cout << "[WARN] Database connection failed\n";
      dbConnector.reset();
    } else {
      std::cout << "[OK] MySQL connected (pool: " << dbConfig.poolSize << " connections)\n";
    }
  }

  Umbra::Zone::ZoneServer::Config config;
  config.zoneId = static_cast<uint32_t>(zoneId);
  config.zoneName = zoneName;
  config.port = zonePort;
  config.maxPlayers = configManager.get<uint32_t>("zone.max_players_per_zone", 1000);
  config.dbConnector = dbConnector;

  Umbra::Zone::ZoneServer zoneServer(config);

  const uint16_t adminBase = configManager.get<uint16_t>("admin.zone_base_port", 9102);
  const uint16_t adminPort = static_cast<uint16_t>(adminBase + zoneId);
  auto adminServer = Umbra::Admin::createFromConfig(
      zoneName, adminPort,
      [](int) { running = false; });
  if (adminServer) {
    Umbra::Admin::registerZoneCommands(adminServer->getRegistry(), zoneServer);
    adminServer->start();
  }

  if (!zoneServer.start()) {
    std::cerr << "[FAIL] Zone Server failed to start\n";
    return 1;
  }

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  std::cout << "[OK] Zone Server '" << zoneName << "' running on port " << zonePort << "\n";
  std::cout << "  Admin port: " << adminPort << "\n";
  std::cout << "\n  Press Ctrl+C to stop.\n\n";

  auto lastUpdate = std::chrono::steady_clock::now();
  auto nextFullUpdate = lastUpdate;
  // Pump leve a cada ~3ms (inbound + hits adiados); update pesado a ~16ms (~62Hz).
  // Antes o loop dormia 16ms fixos, então um cast que chegasse logo após o drain
  // esperava até ~16ms só para ser enfileirado, e cada skill em sequência
  // acumulava esse atraso. Agora o inbound é drenado quase na hora.
  constexpr auto kPumpPeriod = std::chrono::milliseconds(3);
  constexpr auto kFullUpdatePeriod = std::chrono::milliseconds(16);

  // #region agent log
  int64_t lastPumpMs = Umbra::Zone::agentNowMs();
  // #endregion
  auto nextPumpAt = std::chrono::steady_clock::now();
  while (running) {
    auto now = std::chrono::steady_clock::now();

    // #region agent log
    const int64_t loopT0 = Umbra::Zone::agentNowMs();
    {
      const int64_t gap = loopT0 - lastPumpMs;
      if (gap >= 200) {
        Umbra::Zone::agentDebugLog(
            "H-PUMP-GAP", "main_zone.cpp:loop", "pump_gap_ms",
            std::string("{\"gapMs\":") + std::to_string(gap) + "}", "post-fix");
      }
    }
    // #endregion
    zoneServer.pumpInbound();
    // #region agent log
    const int64_t afterPump = Umbra::Zone::agentNowMs();
    const int64_t pumpMs = afterPump - loopT0;
    // #endregion

    int64_t updateMs = 0;
    if (now >= nextFullUpdate) {
      float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
      lastUpdate = now;
      // #region agent log
      const int64_t u0 = Umbra::Zone::agentNowMs();
      // #endregion
      zoneServer.update(deltaTime);
      // #region agent log
      updateMs = Umbra::Zone::agentNowMs() - u0;
      // #endregion
      // Agenda o próximo update pesado a partir do alvo (não do fim do trabalho),
      // para não acumular drift quando um update pontual passar de 16ms.
      nextFullUpdate += kFullUpdatePeriod;
      if (nextFullUpdate < now) nextFullUpdate = now + kFullUpdatePeriod;
    }

    // #region agent log
    // Instrumentação leve: NÃO ler /proc no hot path (amplificou lag no Proxmox).
    int64_t sleepMs = 0;
    // #endregion
    // Catch-up: evidência no Proxmox mostrou sleep_for(3ms) retornando em 2.6–3.0s
    // (sleepMs dominante; pump/update ~0) a cada ~30s — pause do host/VM.
    // Se já estamos atrasados, NÃO dormir de novo: bombeia hits pendentes na hora.
    nextPumpAt += kPumpPeriod;
    now = std::chrono::steady_clock::now();
    if (nextPumpAt > now) {
      // #region agent log
      const int64_t sleepT0 = Umbra::Zone::agentNowMs();
      // #endregion
      std::this_thread::sleep_until(nextPumpAt);
      // #region agent log
      sleepMs = Umbra::Zone::agentNowMs() - sleepT0;
      // #endregion
    } else {
      // #region agent log
      sleepMs = 0;
      // #endregion
      // Reseta a agenda para não tentar “pagar” segundos de atraso do hypervisor.
      nextPumpAt = now;
    }
    // #region agent log
    {
      if (pumpMs >= 50 || updateMs >= 50 || sleepMs >= 50) {
        Umbra::Zone::agentDebugLog(
            "H-LOOP-PHASE", "main_zone.cpp:loop", "loop_phase_ms",
            std::string("{\"pumpMs\":") + std::to_string(pumpMs) +
                ",\"updateMs\":" + std::to_string(updateMs) +
                ",\"sleepMs\":" + std::to_string(sleepMs) + "}",
            "post-fix");
      }
      lastPumpMs = Umbra::Zone::agentNowMs();
    }
    // #endregion
  }

  std::cout << "\nShutting down Zone Server...\n";
  if (adminServer) adminServer->stop();
  zoneServer.stop();
  std::cout << "[OK] Zone Server '" << zoneName << "' stopped\n";

  return 0;
}
