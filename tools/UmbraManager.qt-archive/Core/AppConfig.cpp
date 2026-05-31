#include "AppConfig.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

AppConfig& AppConfig::instance() {
  static AppConfig cfg;
  return cfg;
}

bool AppConfig::load(const QString& path) {
  QString cfgPath = path;
  if (cfgPath.isEmpty()) {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        appDir + "/config/manager.json",
        appDir + "/../config/manager.json",
        appDir + "/../../config/manager.json",
        "config/manager.json"};
    for (const auto& c : candidates) {
      if (QFile::exists(c)) {
        cfgPath = c;
        break;
      }
    }
    if (cfgPath.isEmpty()) cfgPath = "config/manager.json";
  }

  QFile file(cfgPath);
  if (!file.open(QIODevice::ReadOnly)) {
    projectRoot_ = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../..");
    buildDir_ = "build/bin/Release";
    configPath_ = "config/server.json";
    adminSecret_ = "umbra_dev_admin_secret_change_in_production";
    phpApiBase_ = "http://localhost/umbra_api/api";
    logDir_ = "logs";
    managerConfigPath_ = cfgPath;
    loadServerJson();
    return false;
  }

  const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
  managerConfigPath_ = cfgPath;
  projectRoot_ = root.value("project_root").toString(projectRoot_);
  buildDir_ = root.value("build_dir").toString("build/bin/Release");
  configPath_ = root.value("config_path").toString("config/server.json");
  adminSecret_ = root.value("admin_secret").toString(adminSecret_);
  phpApiBase_ = root.value("php_api_base").toString(phpApiBase_);
  logDir_ = root.value("log_dir").toString("logs");
  pollIntervalMs_ = root.value("poll_interval_ms").toInt(1000);
  playersPollMs_ = root.value("players_poll_ms").toInt(2000);
  autoRestartOnCrash_ = root.value("auto_restart_on_crash").toBool(false);

  zoneInstances_.clear();
  for (const auto v : root.value("zone_instances").toArray()) {
    zoneInstances_.append(v.toInt());
  }
  if (zoneInstances_.isEmpty()) zoneInstances_.append(0);

  loadServerJson();
  return true;
}

bool AppConfig::save(const QString& path) const {
  const QString out = path.isEmpty() ? managerConfigPath_ : path;
  QJsonObject root;
  root["project_root"] = projectRoot_;
  root["build_dir"] = buildDir_;
  root["config_path"] = configPath_;
  root["admin_secret"] = adminSecret_;
  root["php_api_base"] = phpApiBase_;
  root["log_dir"] = logDir_;
  root["poll_interval_ms"] = pollIntervalMs_;
  root["players_poll_ms"] = playersPollMs_;
  root["auto_restart_on_crash"] = autoRestartOnCrash_;
  QJsonArray zones;
  for (int z : zoneInstances_) zones.append(z);
  root["zone_instances"] = zones;

  QFile file(out);
  if (!file.open(QIODevice::WriteOnly)) return false;
  file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  return true;
}

QString AppConfig::absolutePath(const QString& relative) const {
  QDir root(projectRoot_);
  return root.absoluteFilePath(relative);
}

void AppConfig::loadServerJson() {
  const QString full = absolutePath(configPath_);
  QFile f(full);
  if (!f.open(QIODevice::ReadOnly)) return;
  serverJson_ = QJsonDocument::fromJson(f.readAll()).object();
  const QJsonObject admin = serverJson_.value("admin").toObject();
  if (!admin.isEmpty() && adminSecret_.isEmpty()) {
    adminSecret_ = admin.value("shared_secret").toString();
  }
}

QVector<ServiceDefinition> AppConfig::serviceDefinitions() const {
  QVector<ServiceDefinition> list;
  const QJsonObject admin = serverJson_.value("admin").toObject();

  auto add = [&](const QString& id, const QString& name, const QString& exe,
                 const QStringList& args, quint16 gamePort, quint16 adminPort,
                 const QString& log) {
    ServiceDefinition s;
    s.id = id;
    s.displayName = name;
    s.executable = exe;
    s.arguments = args;
    s.gamePort = gamePort;
    s.adminPort = adminPort;
    s.logFile = log;
    list.append(s);
  };

  add("auth", "Auth Server", "auth_server.exe", {}, serverJson_.value("auth").toObject().value("port").toInt(8080),
      admin.value("auth_port").toInt(9100), "auth_server.log");
  add("world", "World Server", "world_server.exe", {}, serverJson_.value("world").toObject().value("port").toInt(8081),
      admin.value("world_port").toInt(9101), "world_server.log");
  add("chat", "Chat Server", "chat_server.exe", {}, serverJson_.value("chat").toObject().value("port").toInt(8083),
      admin.value("chat_port").toInt(9103), "chat_server.log");
  add("gateway", "Gateway", "gateway_server.exe", {}, serverJson_.value("gateway").toObject().value("port").toInt(9000),
      admin.value("gateway_port").toInt(9104), "gateway_server.log");

  const int zoneBase = serverJson_.value("zone").toObject().value("base_port").toInt(8082);
  const int adminZoneBase = admin.value("zone_base_port").toInt(9102);
  for (int zid : zoneInstances_) {
    ServiceDefinition z;
    z.id = QString("zone_%1").arg(zid);
    z.displayName = QString("Zone %1").arg(zid);
    z.executable = "zone_server.exe";
    z.arguments = {QString::number(zid)};
    z.gamePort = static_cast<quint16>(zoneBase + zid);
    z.adminPort = static_cast<quint16>(adminZoneBase + zid);
    z.logFile = "zone_server.log";
    list.append(z);
  }
  return list;
}
