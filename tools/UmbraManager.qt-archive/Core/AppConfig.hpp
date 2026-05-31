#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct ServiceDefinition {
  QString id;
  QString displayName;
  QString executable;
  QStringList arguments;
  quint16 gamePort = 0;
  quint16 adminPort = 0;
  QString logFile;
};

class AppConfig {
 public:
  static AppConfig& instance();

  bool load(const QString& path = QString());
  bool save(const QString& path = QString()) const;

  QString projectRoot() const { return projectRoot_; }
  QString buildDir() const { return buildDir_; }
  QString configPath() const { return configPath_; }
  QString adminSecret() const { return adminSecret_; }
  QString phpApiBase() const { return phpApiBase_; }
  QString logDir() const { return logDir_; }
  QString managerConfigPath() const { return managerConfigPath_; }
  int pollIntervalMs() const { return pollIntervalMs_; }
  int playersPollMs() const { return playersPollMs_; }
  bool autoRestartOnCrash() const { return autoRestartOnCrash_; }
  QVector<int> zoneInstances() const { return zoneInstances_; }

  QString absolutePath(const QString& relative) const;
  QVector<ServiceDefinition> serviceDefinitions() const;

  void setAdminUsername(const QString& u) { adminUsername_ = u; }
  QString adminUsername() const { return adminUsername_; }

 private:
  AppConfig() = default;

  QString projectRoot_;
  QString buildDir_;
  QString configPath_;
  QString adminSecret_;
  QString phpApiBase_;
  QString logDir_;
  QString managerConfigPath_;
  int pollIntervalMs_ = 1000;
  int playersPollMs_ = 2000;
  bool autoRestartOnCrash_ = false;
  QVector<int> zoneInstances_{0};
  QString adminUsername_;

  QJsonObject serverJson_;
  void loadServerJson();
};
