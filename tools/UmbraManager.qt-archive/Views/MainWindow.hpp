#pragma once

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <memory>

class ProcessManager;
class LogTailer;
class MetricsStore;
class PhpAdminClient;
class AuditLog;
class Scheduler;
class AdminClient;
class DashboardWidget;
class ServersWidget;
class ZonesWidget;
class PlayersWidget;
class LogsWidget;
class AccountsWidget;
class ItemsWidget;
class ConfigEditorWidget;
class GmConsoleWidget;
class SchedulerWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

 private slots:
  void pollAdminChannels();
  void pollPlayers();
  void onAdminResponse(const QString& serviceId, const QString& cmd, const QJsonObject& response);
  void onGmCommand(const QString& serviceId, const QString& cmd, const QJsonObject& args);
  void onReloadConfig();
  void onSaveConfig(const QString& jsonText);
  void onKickPlayer(quint32 playerId, const QString& zoneServiceId);
  void onTeleportPlayer(quint32 playerId, const QString& zoneServiceId, float x, float y, float z);
  void onBroadcast(const QString& zoneServiceId, const QString& message);
  void onProcessOutput(const QString& serviceId, const QString& line);
  void onLogLine(const QString& serviceId, const QString& line);

 private:
  void setupAdminClients();
  AdminClient* adminClient(const QString& serviceId);
  void sendAdminCommand(const QString& serviceId, const QString& cmd, const QJsonObject& args = {});
  QStringList allServiceIds() const;

  ProcessManager* processManager_;
  LogTailer* logTailer_;
  MetricsStore* metricsStore_;
  PhpAdminClient* phpClient_;
  AuditLog* auditLog_;
  Scheduler* scheduler_;

  DashboardWidget* dashboard_;
  ServersWidget* servers_;
  ZonesWidget* zones_;
  PlayersWidget* players_;
  LogsWidget* logs_;
  AccountsWidget* accounts_;
  ItemsWidget* items_;
  ConfigEditorWidget* configEditor_;
  GmConsoleWidget* gmConsole_;
  SchedulerWidget* schedulerWidget_;

  QMap<QString, AdminClient*> adminClients_;
  QMap<QString, QJsonArray> playersByZone_;
  QTimer pollTimer_;
  QTimer playersTimer_;
  QJsonObject lastStats_;
};
