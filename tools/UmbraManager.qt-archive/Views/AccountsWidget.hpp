#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QJsonArray>
#include <QJsonObject>

class PhpAdminClient;

class AccountsWidget : public QWidget {
  Q_OBJECT
 public:
  explicit AccountsWidget(PhpAdminClient* php, QWidget* parent = nullptr);

 public slots:
  void refresh();
  void onListResult(bool ok, const QJsonArray& accounts, const QJsonObject& stats, const QString& error);

 signals:
  void banRequested(int accountId, const QString& reason);
  void unbanRequested(int accountId);

 private slots:
  void onBan();
  void onUnban();

 private:
  PhpAdminClient* php_;
  QTableWidget* table_;
  QJsonArray accounts_;
};
