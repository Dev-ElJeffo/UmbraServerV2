#pragma once

#include <QObject>
#include <QFile>
#include <QFileSystemWatcher>
#include <QMap>

class LogTailer : public QObject {
  Q_OBJECT
 public:
  explicit LogTailer(QObject* parent = nullptr);

  void watchLog(const QString& serviceId, const QString& filePath);
  void unwatchLog(const QString& serviceId);
  QString readExisting(const QString& serviceId, int maxLines = 500) const;

 signals:
  void lineAppended(const QString& serviceId, const QString& line);
  void logRotated(const QString& serviceId);

 private slots:
  void onFileChanged(const QString& path);
  void readNewContent(const QString& serviceId);

 private:
  struct TailState {
    QString path;
    QFile file;
    qint64 position = 0;
  };

  QFileSystemWatcher watcher_;
  QMap<QString, TailState> tails_;
  QMap<QString, QString> pathToService_;
};
