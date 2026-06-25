#include "SchedulerWidget.hpp"
#include "Scheduler.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>

SchedulerWidget::SchedulerWidget(Scheduler* scheduler, QWidget* parent)
    : QWidget(parent), scheduler_(scheduler) {
  auto* layout = new QVBoxLayout(this);
  auto* toolbar = new QHBoxLayout();
  auto* restartBtn = new QPushButton("Agendar restart stack (min)", this);
  auto* broadcastBtn = new QPushButton("Agendar broadcast global (min)", this);
  connect(restartBtn, &QPushButton::clicked, this, &SchedulerWidget::onAddRestartAll);
  connect(broadcastBtn, &QPushButton::clicked, this, &SchedulerWidget::onAddBroadcast);
  toolbar->addWidget(restartBtn);
  toolbar->addWidget(broadcastBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  table_ = new QTableWidget(this);
  table_->setColumnCount(3);
  table_->setHorizontalHeaderLabels({"ID", "Descrição", "Intervalo (min)"});
  layout->addWidget(table_);

  connect(scheduler_, &Scheduler::taskExecuted, this, &SchedulerWidget::onTaskExecuted);
}

void SchedulerWidget::onAddRestartAll() {
  bool ok = false;
  const int mins = QInputDialog::getInt(this, "Intervalo", "Minutos:", 60, 1, 100000, 1, &ok);
  if (!ok) return;
  emit scheduleRestartAll(mins);
  const int row = table_->rowCount();
  table_->insertRow(row);
  table_->setItem(row, 0, new QTableWidgetItem("restart_all"));
  table_->setItem(row, 1, new QTableWidgetItem("Restart stack completo"));
  table_->setItem(row, 2, new QTableWidgetItem(QString::number(mins)));
}

void SchedulerWidget::onAddBroadcast() {
  bool ok = false;
  const int mins = QInputDialog::getInt(this, "Intervalo", "Minutos:", 30, 1, 100000, 1, &ok);
  if (!ok) return;
  emit scheduleBroadcast(mins);
  const int row = table_->rowCount();
  table_->insertRow(row);
  table_->setItem(row, 0, new QTableWidgetItem("broadcast_global"));
  table_->setItem(row, 1, new QTableWidgetItem("Broadcast GM global"));
  table_->setItem(row, 2, new QTableWidgetItem(QString::number(mins)));
}

void SchedulerWidget::onTaskExecuted(const QString& id, const QString& description) {
  table_->setToolTip(QString("Última execução: %1 — %2").arg(id, description));
}
