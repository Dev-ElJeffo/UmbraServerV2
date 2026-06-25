#pragma once

#include <QWidget>
#include <QLabel>
#include <QJsonObject>
#include <QtCharts/QChartView>

class MetricsStore;

class DashboardWidget : public QWidget {
  Q_OBJECT
 public:
  explicit DashboardWidget(QWidget* parent = nullptr);

  void updateServiceStatus(const QString& serviceId, bool online, const QJsonObject& stats);
  void setTotalPlayers(int count);
  void setTotalZones(int count);

 private:
  QLabel* totalPlayersLabel_;
  QLabel* totalZonesLabel_;
  QMap<QString, QLabel*> statusLabels_;
  QMap<QString, QLabel*> statsLabels_;
  QtCharts::QChartView* cpuChartView_;
};
