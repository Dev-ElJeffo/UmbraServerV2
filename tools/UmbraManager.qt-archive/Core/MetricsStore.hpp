#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>
#include <QJsonObject>

class MetricsStore : public QObject {
  Q_OBJECT
 public:
  explicit MetricsStore(QObject* parent = nullptr);
  ~MetricsStore() override;

  bool initialize();
  void recordMetric(const QString& serviceId, const QJsonObject& stats);
  QList<QPair<QDateTime, double>> cpuHistory(const QString& serviceId, int minutes = 60) const;
  QList<QPair<QDateTime, double>> memHistory(const QString& serviceId, int minutes = 60) const;
  int totalPlayersOnline() const;

  void setTotalPlayersOnline(int count);

 private:
  QSqlDatabase db_;
  int totalPlayersOnline_ = 0;
  bool ensureSchema();
};
