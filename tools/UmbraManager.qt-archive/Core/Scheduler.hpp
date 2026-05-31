#pragma once

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <functional>

struct ScheduledTask {
  QString id;
  QString description;
  QDateTime nextRun;
  int intervalMinutes = 0;
  std::function<void()> action;
};

class Scheduler : public QObject {
  Q_OBJECT
 public:
  explicit Scheduler(QObject* parent = nullptr);

  void addTask(const QString& id, const QString& description, int intervalMinutes,
               std::function<void()> action);
  void removeTask(const QString& id);
  QStringList taskIds() const;

 signals:
  void taskExecuted(const QString& id, const QString& description);

 private slots:
  void tick();

 private:
  QTimer timer_;
  QMap<QString, ScheduledTask> tasks_;
};
