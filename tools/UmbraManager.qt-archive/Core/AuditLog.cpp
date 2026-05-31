#include "AuditLog.hpp"
#include <QSqlQuery>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

AuditLog::AuditLog(QObject* parent) : QObject(parent) {}

bool AuditLog::initialize() {
  const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir().mkpath(dir);
  db_ = QSqlDatabase::addDatabase("QSQLITE", "umbra_audit");
  db_.setDatabaseName(dir + "/manager.db");
  if (!db_.open()) return false;

  QSqlQuery q(db_);
  return q.exec(
      "CREATE TABLE IF NOT EXISTS audit_log ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "ts INTEGER NOT NULL,"
      "operator_name TEXT,"
      "action TEXT,"
      "details TEXT);");
}

void AuditLog::logAction(const QString& operatorName, const QString& action, const QString& details) {
  QSqlQuery q(db_);
  q.prepare("INSERT INTO audit_log (ts, operator_name, action, details) VALUES (?, ?, ?, ?)");
  q.addBindValue(QDateTime::currentSecsSinceEpoch());
  q.addBindValue(operatorName);
  q.addBindValue(action);
  q.addBindValue(details);
  q.exec();
}
