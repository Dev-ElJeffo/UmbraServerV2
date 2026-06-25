#include "MetricsStore.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>

MetricsStore::MetricsStore(QObject* parent) : QObject(parent) {}

MetricsStore::~MetricsStore() {
  if (db_.isOpen()) {
    db_.close();
  }
}

bool MetricsStore::initialize() {
  const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir().mkpath(dir);
  const QString dbPath = dir + "/manager.db";

  db_ = QSqlDatabase::addDatabase("QSQLITE", "umbra_metrics");
  db_.setDatabaseName(dbPath);
  if (!db_.open()) return false;
  return ensureSchema();
}

bool MetricsStore::ensureSchema() {
  QSqlQuery q(db_);
  const char* sql =
      "CREATE TABLE IF NOT EXISTS metrics ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "service_id TEXT NOT NULL,"
      "ts INTEGER NOT NULL,"
      "cpu_pct REAL,"
      "mem_mb REAL,"
      "uptime_s INTEGER);"
      "CREATE INDEX IF NOT EXISTS idx_metrics_service_ts ON metrics(service_id, ts);";
  return q.exec(sql);
}

void MetricsStore::recordMetric(const QString& serviceId, const QJsonObject& stats) {
  QSqlQuery q(db_);
  q.prepare("INSERT INTO metrics (service_id, ts, cpu_pct, mem_mb, uptime_s) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(serviceId);
  q.addBindValue(QDateTime::currentSecsSinceEpoch());
  q.addBindValue(stats.value("cpu_pct").toDouble());
  q.addBindValue(stats.value("mem_mb").toDouble());
  q.addBindValue(stats.value("uptime_s").toInt());
  q.exec();
}

QList<QPair<QDateTime, double>> MetricsStore::cpuHistory(const QString& serviceId, int minutes) const {
  QList<QPair<QDateTime, double>> out;
  const qint64 since = QDateTime::currentSecsSinceEpoch() - minutes * 60;
  QSqlQuery q(db_);
  q.prepare("SELECT ts, cpu_pct FROM metrics WHERE service_id=? AND ts>=? ORDER BY ts");
  q.addBindValue(serviceId);
  q.addBindValue(since);
  if (!q.exec()) return out;
  while (q.next()) {
    out.append({QDateTime::fromSecsSinceEpoch(q.value(0).toLongLong()), q.value(1).toDouble()});
  }
  return out;
}

QList<QPair<QDateTime, double>> MetricsStore::memHistory(const QString& serviceId, int minutes) const {
  QList<QPair<QDateTime, double>> out;
  const qint64 since = QDateTime::currentSecsSinceEpoch() - minutes * 60;
  QSqlQuery q(db_);
  q.prepare("SELECT ts, mem_mb FROM metrics WHERE service_id=? AND ts>=? ORDER BY ts");
  q.addBindValue(serviceId);
  q.addBindValue(since);
  if (!q.exec()) return out;
  while (q.next()) {
    out.append({QDateTime::fromSecsSinceEpoch(q.value(0).toLongLong()), q.value(1).toDouble()});
  }
  return out;
}

int MetricsStore::totalPlayersOnline() const { return totalPlayersOnline_; }
void MetricsStore::setTotalPlayersOnline(int count) { totalPlayersOnline_ = count; }
