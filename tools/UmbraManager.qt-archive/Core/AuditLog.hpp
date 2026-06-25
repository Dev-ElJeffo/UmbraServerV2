#pragma once

#include <QObject>
#include <QSqlDatabase>

class AuditLog : public QObject {
  Q_OBJECT
 public:
  explicit AuditLog(QObject* parent = nullptr);
  bool initialize();
  void logAction(const QString& operatorName, const QString& action, const QString& details);

 private:
  QSqlDatabase db_;
};
