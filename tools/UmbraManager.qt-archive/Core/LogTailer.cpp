#include "LogTailer.hpp"
#include <QTextStream>

LogTailer::LogTailer(QObject* parent) : QObject(parent) {
  connect(&watcher_, &QFileSystemWatcher::fileChanged, this, &LogTailer::onFileChanged);
}

void LogTailer::watchLog(const QString& serviceId, const QString& filePath) {
  if (tails_.contains(serviceId)) {
    watcher_.removePath(tails_[serviceId].path);
  }

  TailState st;
  st.path = filePath;
  st.file.setFileName(filePath);
  if (st.file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    st.position = st.file.size();
  }
  tails_[serviceId] = st;
  pathToService_[filePath] = serviceId;
  watcher_.addPath(filePath);
}

void LogTailer::unwatchLog(const QString& serviceId) {
  if (!tails_.contains(serviceId)) return;
  watcher_.removePath(tails_[serviceId].path);
  pathToService_.remove(tails_[serviceId].path);
  tails_.remove(serviceId);
}

QString LogTailer::readExisting(const QString& serviceId, int maxLines) const {
  if (!tails_.contains(serviceId)) return {};
  QFile f(tails_.value(serviceId).path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};

  QStringList lines;
  QTextStream in(&f);
  while (!in.atEnd()) {
    lines.append(in.readLine());
    if (lines.size() > maxLines) lines.removeFirst();
  }
  return lines.join('\n');
}

void LogTailer::onFileChanged(const QString& path) {
  const QString serviceId = pathToService_.value(path);
  if (serviceId.isEmpty()) return;

  QFileInfo info(path);
  if (!info.exists()) {
    emit logRotated(serviceId);
    watcher_.addPath(path);
    if (tails_.contains(serviceId)) tails_[serviceId].position = 0;
    return;
  }

  if (tails_.contains(serviceId)) {
    const qint64 size = info.size();
    if (size < tails_[serviceId].position) {
      tails_[serviceId].position = 0;
      emit logRotated(serviceId);
    }
  }
  readNewContent(serviceId);
  watcher_.addPath(path);
}

void LogTailer::readNewContent(const QString& serviceId) {
  if (!tails_.contains(serviceId)) return;
  auto& st = tails_[serviceId];
  QFile f(st.path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  f.seek(st.position);
  QTextStream in(&f);
  while (!in.atEnd()) {
    const QString line = in.readLine();
    if (!line.isEmpty()) emit lineAppended(serviceId, line);
  }
  st.position = f.pos();
}
