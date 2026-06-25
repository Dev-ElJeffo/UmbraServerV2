#include "ServersWidget.hpp"
#include "AppConfig.hpp"
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>

ServersWidget::ServersWidget(ProcessManager* pm, QWidget* parent)
    : QWidget(parent), pm_(pm) {
  auto* layout = new QVBoxLayout(this);

  auto* toolbar = new QHBoxLayout();
  auto* startAllBtn = new QPushButton("Start All", this);
  auto* stopAllBtn = new QPushButton("Stop All", this);
  connect(startAllBtn, &QPushButton::clicked, this, &ServersWidget::onStartAll);
  connect(stopAllBtn, &QPushButton::clicked, this, &ServersWidget::onStopAll);
  toolbar->addWidget(startAllBtn);
  toolbar->addWidget(stopAllBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  table_ = new QTableWidget(this);
  table_->setColumnCount(7);
  table_->setHorizontalHeaderLabels(
      {"Serviço", "Porta", "Admin", "PID", "Status", "Auto-restart", "Ações"});
  table_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table_);

  refresh();
}

void ServersWidget::refresh() {
  table_->setRowCount(0);
  rowToServiceId_.clear();
  int row = 0;
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    table_->insertRow(row);
    rowToServiceId_[row] = def.id;
    table_->setItem(row, 0, new QTableWidgetItem(def.displayName));
    table_->setItem(row, 1, new QTableWidgetItem(QString::number(def.gamePort)));
    table_->setItem(row, 2, new QTableWidgetItem(QString::number(def.adminPort)));

    const bool running = pm_->isRunning(def.id);
    const auto info = pm_->info(def.id);
    table_->setItem(row, 3, new QTableWidgetItem(running ? QString::number(info.pid) : "-"));
    table_->setItem(row, 4, new QTableWidgetItem(running ? "Running" : "Stopped"));

    auto* autoBox = new QCheckBox(this);
    autoBox->setChecked(AppConfig::instance().autoRestartOnCrash());
    connect(autoBox, &QCheckBox::toggled, this, [this, id = def.id](bool v) {
      pm_->setAutoRestart(id, v);
    });
    table_->setCellWidget(row, 5, autoBox);

    auto* actions = new QWidget(this);
    auto* h = new QHBoxLayout(actions);
    h->setContentsMargins(0, 0, 0, 0);
    auto* start = new QPushButton("Start", actions);
    auto* stop = new QPushButton("Stop", actions);
    auto* restart = new QPushButton("Restart", actions);
    start->setProperty("serviceRow", row);
    stop->setProperty("serviceRow", row);
    restart->setProperty("serviceRow", row);
    connect(start, &QPushButton::clicked, this, &ServersWidget::onStartClicked);
    connect(stop, &QPushButton::clicked, this, &ServersWidget::onStopClicked);
    connect(restart, &QPushButton::clicked, this, &ServersWidget::onRestartClicked);
    h->addWidget(start);
    h->addWidget(stop);
    h->addWidget(restart);
    table_->setCellWidget(row, 6, actions);
    ++row;
  }
}

void ServersWidget::onStartClicked() {
  const int row = sender()->property("serviceRow").toInt();
  const QString id = rowToServiceId_.value(row);
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    if (def.id == id) pm_->startService(def);
  }
  refresh();
}

void ServersWidget::onStopClicked() {
  const int row = sender()->property("serviceRow").toInt();
  pm_->stopService(rowToServiceId_.value(row));
  refresh();
}

void ServersWidget::onRestartClicked() {
  const int row = sender()->property("serviceRow").toInt();
  const QString id = rowToServiceId_.value(row);
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    if (def.id == id) pm_->restartService(def);
  }
  refresh();
}

void ServersWidget::onStartAll() {
  pm_->startAll();
  refresh();
  emit startAllRequested();
}

void ServersWidget::onStopAll() {
  pm_->stopAll();
  refresh();
  emit stopAllRequested();
}
