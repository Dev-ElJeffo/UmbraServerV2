#include "MainWindow.hpp"
#include "AppConfig.hpp"
#include "ProcessManager.hpp"
#include "LogTailer.hpp"
#include "MetricsStore.hpp"
#include "PhpAdminClient.hpp"
#include "AuditLog.hpp"
#include "Scheduler.hpp"
#include "AdminClient.hpp"
#include "DashboardWidget.hpp"
#include "ServersWidget.hpp"
#include "ZonesWidget.hpp"
#include "PlayersWidget.hpp"
#include "LogsWidget.hpp"
#include "AccountsWidget.hpp"
#include "ItemsWidget.hpp"
#include "ConfigEditorWidget.hpp"
#include "GmConsoleWidget.hpp"
#include "SchedulerWidget.hpp"

#include <QTabWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle("UmbraManager — Controle UmbraEternum");
  resize(1280, 800);

  AppConfig::instance().load();

  processManager_ = new ProcessManager(this);
  logTailer_ = new LogTailer(this);
  metricsStore_ = new MetricsStore(this);
  metricsStore_->initialize();
  phpClient_ = new PhpAdminClient(this);
  phpClient_->setBaseUrl(AppConfig::instance().phpApiBase());
  phpClient_->setAdminUsername(AppConfig::instance().adminUsername());
  auditLog_ = new AuditLog(this);
  auditLog_->initialize();
  scheduler_ = new Scheduler(this);

  auto* tabs = new QTabWidget(this);
  setCentralWidget(tabs);

  dashboard_ = new DashboardWidget(this);
  servers_ = new ServersWidget(processManager_, this);
  zones_ = new ZonesWidget(this);
  players_ = new PlayersWidget(this);
  logs_ = new LogsWidget(logTailer_, this);
  accounts_ = new AccountsWidget(phpClient_, this);
  items_ = new ItemsWidget(phpClient_, this);
  configEditor_ = new ConfigEditorWidget(this);
  gmConsole_ = new GmConsoleWidget(this);
  schedulerWidget_ = new SchedulerWidget(scheduler_, this);

  tabs->addTab(dashboard_, "Dashboard");
  tabs->addTab(servers_, "Servers");
  tabs->addTab(zones_, "Zones");
  tabs->addTab(players_, "Players");
  tabs->addTab(logs_, "Logs");
  tabs->addTab(accounts_, "Accounts");
  tabs->addTab(items_, "Items");
  tabs->addTab(configEditor_, "Config");
  tabs->addTab(gmConsole_, "GM Console");
  tabs->addTab(schedulerWidget_, "Scheduler");

  const QString logDir = AppConfig::instance().absolutePath(AppConfig::instance().logDir());
  QStringList serviceIds;
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    serviceIds << def.id;
    logs_->registerServiceTab(def.id, logDir + "/" + def.logFile);
  }
  gmConsole_->setServices(serviceIds);

  configEditor_->loadFromFile(AppConfig::instance().absolutePath(AppConfig::instance().configPath()));

  setupAdminClients();

  connect(&pollTimer_, &QTimer::timeout, this, &MainWindow::pollAdminChannels);
  connect(&playersTimer_, &QTimer::timeout, this, &MainWindow::pollPlayers);
  pollTimer_.start(AppConfig::instance().pollIntervalMs());
  playersTimer_.start(AppConfig::instance().playersPollMs());

  connect(processManager_, &ProcessManager::serviceOutput, this, &MainWindow::onProcessOutput);
  connect(logTailer_, &LogTailer::lineAppended, this, &MainWindow::onLogLine);
  connect(gmConsole_, &GmConsoleWidget::commandSubmitted, this, &MainWindow::onGmCommand);
  connect(configEditor_, &ConfigEditorWidget::reloadConfigRequested, this, &MainWindow::onReloadConfig);
  connect(configEditor_, &ConfigEditorWidget::saveConfigRequested, this, &MainWindow::onSaveConfig);
  connect(players_, &PlayersWidget::kickPlayerRequested, this, &MainWindow::onKickPlayer);
  connect(players_, &PlayersWidget::teleportPlayerRequested, this, &MainWindow::onTeleportPlayer);
  connect(players_, &PlayersWidget::broadcastRequested, this, &MainWindow::onBroadcast);

  connect(servers_, &ServersWidget::startAllRequested, this, [this]() {
    auditLog_->logAction(AppConfig::instance().adminUsername(), "start_all", "stack");
  });
  connect(servers_, &ServersWidget::stopAllRequested, this, [this]() {
    auditLog_->logAction(AppConfig::instance().adminUsername(), "stop_all", "stack");
  });

  scheduler_->addTask("manager_poll", "Refresh stats", 1, [this]() { pollAdminChannels(); });

  connect(schedulerWidget_, &SchedulerWidget::scheduleRestartAll, this, [this](int mins) {
    scheduler_->addTask("restart_all", "Restart stack", mins, [this]() {
      processManager_->stopAll();
      QTimer::singleShot(3000, this, [this]() { processManager_->startAll(); });
    });
  });
  connect(schedulerWidget_, &SchedulerWidget::scheduleBroadcast, this, [this](int mins) {
    scheduler_->addTask("broadcast_global", "Broadcast GM", mins, [this]() {
      for (const auto& def : AppConfig::instance().serviceDefinitions()) {
        if (def.id.startsWith("zone_")) {
          QJsonObject args;
          args["message"] = "[GM] Mensagem agendada do UmbraManager";
          sendAdminCommand(def.id, "broadcast", args);
        }
      }
    });
  });

  statusBar()->showMessage("UmbraManager pronto");
  phpClient_->verifyAdmin();
  accounts_->refresh();
}

MainWindow::~MainWindow() {
  for (auto* c : adminClients_) {
    c->disconnectFromService();
  }
}

void MainWindow::setupAdminClients() {
  const QString secret = AppConfig::instance().adminSecret();
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    auto* client = new AdminClient(this);
    adminClients_[def.id] = client;
    connect(client, &AdminClient::responseReceived, this,
            [this, id = def.id](const QString& cmd, const QJsonObject& resp) {
              onAdminResponse(id, cmd, resp);
            });
    client->connectToService("127.0.0.1", def.adminPort, secret);
  }
}

AdminClient* MainWindow::adminClient(const QString& serviceId) {
  return adminClients_.value(serviceId, nullptr);
}

void MainWindow::sendAdminCommand(const QString& serviceId, const QString& cmd,
                                  const QJsonObject& args) {
  if (auto* c = adminClient(serviceId)) {
    if (!c->isAuthenticated()) {
      gmConsole_->appendOutput(QString("[WARN] %1 não autenticado").arg(serviceId));
      return;
    }
    c->sendCommand(cmd, args);
  }
}

QStringList MainWindow::allServiceIds() const {
  QStringList ids;
  for (const auto& def : AppConfig::instance().serviceDefinitions()) ids << def.id;
  return ids;
}

void MainWindow::pollAdminChannels() {
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    sendAdminCommand(def.id, "stats");
    if (def.id.startsWith("zone_")) {
      sendAdminCommand(def.id, "zone_info");
    }
  }
  servers_->refresh();
}

void MainWindow::pollPlayers() {
  QJsonArray allPlayers;
  QJsonArray zoneRows;
  int totalPlayers = 0;
  int activeZones = 0;

  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    if (!def.id.startsWith("zone_")) continue;
    auto* c = adminClient(def.id);
    const bool online = c && c->isAuthenticated();
    if (online) ++activeZones;

    QJsonObject zrow;
    zrow["zone_id"] = def.arguments.isEmpty() ? 0 : def.arguments[0].toInt();
    zrow["zone_name"] = def.displayName;
    zrow["port"] = def.gamePort;
    zrow["admin_port"] = def.adminPort;
    zrow["online"] = online;
    zrow["players_online"] = 0;
    zoneRows.append(zrow);

    if (online) sendAdminCommand(def.id, "players");
  }

  zones_->updateZones(zoneRows);
  dashboard_->setTotalZones(activeZones);
  dashboard_->setTotalPlayers(totalPlayers);
  metricsStore_->setTotalPlayersOnline(totalPlayers);
}

void MainWindow::onAdminResponse(const QString& serviceId, const QString& cmd,
                                 const QJsonObject& response) {
  if (!response.value("success").toBool()) {
    gmConsole_->appendOutput(QString("[%1] ERRO %2: %3")
                                 .arg(serviceId, cmd, response.value("error").toString()));
    return;
  }

  const QJsonObject data = response.value("data").toObject();
  if (cmd == "stats") {
    lastStats_[serviceId] = data;
    metricsStore_->recordMetric(serviceId, data);
    const QString baseId = serviceId.startsWith("zone_") ? QString() : serviceId.split('_').first();
    if (!serviceId.startsWith("zone_")) {
      dashboard_->updateServiceStatus(serviceId, true, data);
    }
  } else if (cmd == "zone_info") {
    (void)data;
  } else if (cmd == "players") {
    QJsonArray current;
    const QJsonArray players = data.value("players").toArray();
    for (const auto p : players) {
      QJsonObject obj = p.toObject();
      obj["zone_service"] = serviceId;
      current.append(obj);
    }
    playersByZone_[serviceId] = current;

    QJsonArray merged;
    int total = 0;
    for (const auto& arr : playersByZone_) {
      for (const auto p : arr) merged.append(p);
      total += arr.size();
    }
    players_->updatePlayers(merged);
    dashboard_->setTotalPlayers(total);
    metricsStore_->setTotalPlayersOnline(total);
  } else if (cmd == "ping") {
    gmConsole_->appendOutput(QString("[%1] pong v%2").arg(serviceId, data.value("version").toString()));
  } else {
    gmConsole_->appendOutput(QString("[%1] %2 OK: %3")
                                 .arg(serviceId, cmd, QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact))));
  }
}

void MainWindow::onGmCommand(const QString& serviceId, const QString& cmd, const QJsonObject& args) {
  auditLog_->logAction(AppConfig::instance().adminUsername(), "gm_command",
                       serviceId + " " + cmd);
  sendAdminCommand(serviceId, cmd, args);
}

void MainWindow::onReloadConfig() {
  for (const auto& id : allServiceIds()) {
    sendAdminCommand(id, "reload_config");
  }
  auditLog_->logAction(AppConfig::instance().adminUsername(), "reload_config", "all");
  configEditor_->setStatus("reload_config enviado a todos servidores");
}

void MainWindow::onSaveConfig(const QString& jsonText) {
  const QString path = AppConfig::instance().absolutePath(AppConfig::instance().configPath());
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Config", "Falha ao salvar " + path);
    return;
  }
  f.write(jsonText.toUtf8());
  configEditor_->setStatus("Salvo: " + path);
  auditLog_->logAction(AppConfig::instance().adminUsername(), "save_config", path);
}

void MainWindow::onKickPlayer(quint32 playerId, const QString& zoneServiceId) {
  QJsonObject args;
  args["player_id"] = static_cast<int>(playerId);
  sendAdminCommand(zoneServiceId, "kick_player", args);
  auditLog_->logAction(AppConfig::instance().adminUsername(), "kick_player",
                       QString("%1 in %2").arg(playerId).arg(zoneServiceId));
}

void MainWindow::onTeleportPlayer(quint32 playerId, const QString& zoneServiceId, float x, float y,
                                  float z) {
  QJsonObject args;
  args["player_id"] = static_cast<int>(playerId);
  args["x"] = x;
  args["y"] = y;
  args["z"] = z;
  sendAdminCommand(zoneServiceId, "teleport", args);
}

void MainWindow::onBroadcast(const QString& zoneServiceId, const QString& message) {
  QJsonObject args;
  args["message"] = message;
  sendAdminCommand(zoneServiceId, "broadcast", args);
}

void MainWindow::onProcessOutput(const QString& serviceId, const QString& line) {
  logs_->appendLine(serviceId, line);
}

void MainWindow::onLogLine(const QString& serviceId, const QString& line) {
  logs_->appendLine(serviceId, line);
}
