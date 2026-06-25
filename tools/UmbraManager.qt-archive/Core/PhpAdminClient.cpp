#include "PhpAdminClient.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QUrl>

PhpAdminClient::PhpAdminClient(QObject* parent) : QObject(parent) {}

void PhpAdminClient::setBaseUrl(const QString& base) { baseUrl_ = base; }
void PhpAdminClient::setAdminUsername(const QString& username) { adminUsername_ = username; }

void PhpAdminClient::postJson(const QString& endpoint, const QJsonObject& body,
                              std::function<void(const QJsonObject&)> onSuccess,
                              std::function<void(const QString&)> onError) {
  QUrl url(baseUrl_ + endpoint);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply* reply = nam_.post(req, QJsonDocument(body).toJson());
  connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError]() {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      onError(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (!obj.value("success").toBool()) {
      onError(obj.value("message").toString("Erro desconhecido"));
      return;
    }
    onSuccess(obj);
  });
}

void PhpAdminClient::verifyAdmin() {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  postJson("/admin/verify_admin.php", body,
           [this](const QJsonObject& obj) {
             emit verifyAdminFinished(true, obj.value("admin").toObject(), {});
           },
           [this](const QString& err) { emit verifyAdminFinished(false, {}, err); });
}

void PhpAdminClient::listAccounts() {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  postJson("/admin/list_accounts.php", body,
           [this](const QJsonObject& obj) {
             emit listAccountsFinished(true, obj.value("accounts").toArray(),
                                       obj.value("stats").toObject(), {});
           },
           [this](const QString& err) { emit listAccountsFinished(false, {}, {}, err); });
}

void PhpAdminClient::banAccount(int accountId, const QString& reason) {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  body["account_id"] = accountId;
  body["reason"] = reason;
  postJson("/admin/ban_account.php", body,
           [this](const QJsonObject&) { emit banAccountFinished(true, {}); },
           [this](const QString& err) { emit banAccountFinished(false, err); });
}

void PhpAdminClient::unbanAccount(int accountId) {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  body["account_id"] = accountId;
  postJson("/admin/unban_account.php", body,
           [this](const QJsonObject&) { emit unbanAccountFinished(true, {}); },
           [this](const QString& err) { emit unbanAccountFinished(false, err); });
}

void PhpAdminClient::listItems() {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  postJson("/admin/list_items.php", body,
           [this](const QJsonObject& obj) {
             emit listItemsFinished(true, obj.value("items").toArray(), {});
           },
           [this](const QString& err) { emit listItemsFinished(false, {}, err); });
}

void PhpAdminClient::deleteItem(int itemId) {
  QJsonObject body;
  body["admin_username"] = adminUsername_;
  body["item_id"] = itemId;
  postJson("/admin/delete_item.php", body,
           [this](const QJsonObject&) { emit deleteItemFinished(true, {}); },
           [this](const QString& err) { emit deleteItemFinished(false, err); });
}

void PhpAdminClient::createItem(const QJsonObject& item) {
  QJsonObject body = item;
  body["admin_username"] = adminUsername_;
  postJson("/admin/create_item.php", body,
           [this](const QJsonObject&) { emit createItemFinished(true, {}); },
           [this](const QString& err) { emit createItemFinished(false, err); });
}
