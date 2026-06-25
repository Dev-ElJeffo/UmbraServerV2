#pragma once

#include <QWidget>
#include <QTableWidget>

class Scheduler;

class SchedulerWidget : public QWidget {
  Q_OBJECT
 public:
  explicit SchedulerWidget(Scheduler* scheduler, QWidget* parent = nullptr);

 signals:
  void scheduleRestartAll(int intervalMinutes);
  void scheduleBroadcast(int intervalMinutes);

 private slots:
  void onAddRestartAll();
  void onAddBroadcast();
  void onTaskExecuted(const QString& id, const QString& description);

 private:
  Scheduler* scheduler_;
  QTableWidget* table_;
};
