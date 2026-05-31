#include "ProcessManager.hpp"
#include <QDir>
#include <QTimer>

ProcessManager::ProcessManager(QObject* parent) : QObject(parent) {}

QString ProcessManager::workingDirectory() const {
  return AppConfig::instance().absolutePath(AppConfig::instance().buildDir());
}

QString ProcessManager::executablePath(const ServiceDefinition& def) const {
  return QDir(workingDirectory()).absoluteFilePath(def.executable);
}

void ProcessManager::startService(const ServiceDefinition& def) {
  if (processes_.contains(def.id) && processes_[def.id].process &&
      processes_[def.id].process->state() != QProcess::NotRunning) {
    return;
  }

  definitions_[def.id] = def;
  auto* proc = new QProcess(this);
  ProcessInfo pi;
  pi.serviceId = def.id;
  pi.process = proc;
  pi.startedAt = QDateTime::currentDateTime();
  pi.autoRestart = AppConfig::instance().autoRestartOnCrash();
  processes_[def.id] = pi;

  connect(proc, &QProcess::readyReadStandardOutput, this, &ProcessManager::onReadyRead);
  connect(proc, &QProcess::readyReadStandardError, this, &ProcessManager::onReadyRead);
  connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &ProcessManager::onProcessFinished);

  proc->setWorkingDirectory(workingDirectory());
  proc->setProgram(executablePath(def));
  proc->setArguments(def.arguments);
  proc->start();

  if (proc->waitForStarted(5000)) {
    processes_[def.id].pid = proc->processId();
    emit serviceStateChanged(def.id, true);
  } else {
    emit serviceOutput(def.id, "Falha ao iniciar: " + proc->errorString());
    emit serviceStateChanged(def.id, false);
  }
}

void ProcessManager::stopService(const QString& serviceId, int graceMs) {
  if (!processes_.contains(serviceId)) return;
  auto* proc = processes_[serviceId].process;
  if (!proc || proc->state() == QProcess::NotRunning) return;

  proc->terminate();
  if (!proc->waitForFinished(graceMs)) {
    proc->kill();
    proc->waitForFinished(2000);
  }
}

void ProcessManager::restartService(const ServiceDefinition& def) {
  stopService(def.id);
  startService(def);
}

void ProcessManager::startAll() {
  for (const auto& def : AppConfig::instance().serviceDefinitions()) {
    startService(def);
  }
}

void ProcessManager::stopAll() {
  for (const auto& id : processes_.keys()) {
    stopService(id);
  }
}

bool ProcessManager::isRunning(const QString& serviceId) const {
  if (!processes_.contains(serviceId)) return false;
  const auto* proc = processes_.value(serviceId).process;
  return proc && proc->state() != QProcess::NotRunning;
}

ProcessInfo ProcessManager::info(const QString& serviceId) const {
  return processes_.value(serviceId);
}

QStringList ProcessManager::runningServiceIds() const {
  QStringList ids;
  for (auto it = processes_.cbegin(); it != processes_.cend(); ++it) {
    if (isRunning(it.key())) ids << it.key();
  }
  return ids;
}

QString ProcessManager::consumeOutput(const QString& serviceId) {
  if (!processes_.contains(serviceId)) return {};
  const QString out = processes_[serviceId].lastOutput;
  processes_[serviceId].lastOutput.clear();
  return out;
}

void ProcessManager::setAutoRestart(const QString& serviceId, bool enabled) {
  if (processes_.contains(serviceId)) {
    processes_[serviceId].autoRestart = enabled;
  }
}

void ProcessManager::onReadyRead() {
  auto* proc = qobject_cast<QProcess*>(sender());
  if (!proc) return;

  QString serviceId;
  for (auto it = processes_.begin(); it != processes_.end(); ++it) {
    if (it->process == proc) {
      serviceId = it.key();
      break;
    }
  }
  if (serviceId.isEmpty()) return;

  const QByteArray out = proc->readAllStandardOutput();
  const QByteArray err = proc->readAllStandardError();
  const QString text = QString::fromLocal8Bit(out + err);
  if (text.isEmpty()) return;

  processes_[serviceId].lastOutput += text;
  for (const QString& line : text.split('\n', Qt::SkipEmptyParts)) {
    emit serviceOutput(serviceId, line);
  }
}

void ProcessManager::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
  auto* proc = qobject_cast<QProcess*>(sender());
  if (!proc) return;

  QString serviceId;
  for (auto it = processes_.begin(); it != processes_.end(); ++it) {
    if (it->process == proc) {
      serviceId = it.key();
      break;
    }
  }
  if (serviceId.isEmpty()) return;

  emit serviceStateChanged(serviceId, false);
  emit serviceCrashed(serviceId, exitCode);

  if (status == QProcess::CrashExit || exitCode != 0) {
    if (processes_[serviceId].autoRestart && definitions_.contains(serviceId)) {
      ++processes_[serviceId].restartCount;
      QTimer::singleShot(2000, this, [this, serviceId]() {
        if (definitions_.contains(serviceId)) {
          startService(definitions_[serviceId]);
        }
      });
    }
  }
}
