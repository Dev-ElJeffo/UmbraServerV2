#pragma once

#include "AppConfig.hpp"
#include <QObject>
#include <QProcess>
#include <QMap>
#include <QDateTime>

struct ProcessInfo {
  QString serviceId;
  QProcess* process = nullptr;
  qint64 pid = 0;
  QDateTime startedAt;
  QString lastOutput;
  bool autoRestart = false;
  int restartCount = 0;
};

class ProcessManager : public QObject {
  Q_OBJECT
 public:
  explicit ProcessManager(QObject* parent = nullptr);

  void startService(const ServiceDefinition& def);
  void stopService(const QString& serviceId, int graceMs = 3000);
  void restartService(const ServiceDefinition& def);
  void startAll();
  void stopAll();

  bool isRunning(const QString& serviceId) const;
  ProcessInfo info(const QString& serviceId) const;
  QStringList runningServiceIds() const;
  QString consumeOutput(const QString& serviceId);

  void setAutoRestart(const QString& serviceId, bool enabled);

 signals:
  void serviceStateChanged(const QString& serviceId, bool running);
  void serviceOutput(const QString& serviceId, const QString& line);
  void serviceCrashed(const QString& serviceId, int exitCode);

 private slots:
  void onProcessFinished(int exitCode, QProcess::ExitStatus status);
  void onReadyRead();

 private:
  QString workingDirectory() const;
  QString executablePath(const ServiceDefinition& def) const;

  QMap<QString, ProcessInfo> processes_;
  QMap<QString, ServiceDefinition> definitions_;
};
