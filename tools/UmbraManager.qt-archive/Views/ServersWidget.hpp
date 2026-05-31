#pragma once

#include <QWidget>
#include "ProcessManager.hpp"

class ServersWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ServersWidget(ProcessManager* pm, QWidget* parent = nullptr);

 signals:
  void startAllRequested();
  void stopAllRequested();

 public slots:
  void refresh();

 private slots:
  void onStartClicked();
  void onStopClicked();
  void onRestartClicked();
  void onStartAll();
  void onStopAll();

 private:
  ProcessManager* pm_;
  QTableWidget* table_;
  QMap<int, QString> rowToServiceId_;
};
