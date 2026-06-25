#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class PhpAdminClient : public QObject {
  Q_OBJECT
 public:
  explicit PhpAdminClient(QObject* parent = nullptr);

  void setBaseUrl(const QString& base);
  void setAdminUsername(const QString& username);

  void verifyAdmin();
  void listAccounts();
  void banAccount(int accountId, const QString& reason);
  void unbanAccount(int accountId);
  void listItems();
  void deleteItem(int itemId);
  void createItem(const QJsonObject& item);

 signals:
  void verifyAdminFinished(bool ok, const QJsonObject& adminInfo, const QString& error);
  void listAccountsFinished(bool ok, const QJsonArray& accounts, const QJsonObject& stats, const QString& error);
  void banAccountFinished(bool ok, const QString& error);
  void unbanAccountFinished(bool ok, const QString& error);
  void listItemsFinished(bool ok, const QJsonArray& items, const QString& error);
  void deleteItemFinished(bool ok, const QString& error);
  void createItemFinished(bool ok, const QString& error);

 private:
  void postJson(const QString& endpoint, const QJsonObject& body,
                std::function<void(const QJsonObject&)> onSuccess,
                std::function<void(const QString&)> onError);

  QNetworkAccessManager nam_;
  QString baseUrl_;
  QString adminUsername_;
};
