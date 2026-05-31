#include "Scheduler.hpp"

Scheduler::Scheduler(QObject* parent) : QObject(parent) {
  connect(&timer_, &QTimer::timeout, this, &Scheduler::tick);
  timer_.start(5000);
}

void Scheduler::addTask(const QString& id, const QString& description, int intervalMinutes,
                         std::function<void()> action) {
  ScheduledTask t;
  t.id = id;
  t.description = description;
  t.intervalMinutes = intervalMinutes;
  t.action = std::move(action);
  t.nextRun = QDateTime::currentDateTime().addSecs(intervalMinutes * 60);
  tasks_[id] = t;
}

void Scheduler::removeTask(const QString& id) { tasks_.remove(id); }

QStringList Scheduler::taskIds() const { return tasks_.keys(); }

void Scheduler::tick() {
  const QDateTime now = QDateTime::currentDateTime();
  for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
    if (now >= it->nextRun) {
      if (it->action) it->action();
      emit taskExecuted(it->id, it->description);
      it->nextRun = now.addSecs(it->intervalMinutes * 60);
    }
  }
}
