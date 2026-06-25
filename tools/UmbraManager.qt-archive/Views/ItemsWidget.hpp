#pragma once

#include <QWidget>
#include <QTableWidget>

class PhpAdminClient;

class ItemsWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ItemsWidget(PhpAdminClient* php, QWidget* parent = nullptr);

 public slots:
  void refresh();
  void onListResult(bool ok, const QJsonArray& items, const QString& error);

 private slots:
  void onDelete();
  void onCreate();

 private:
  PhpAdminClient* php_;
  QTableWidget* table_;
};
