#include "GmConsoleWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>

GmConsoleWidget::GmConsoleWidget(QWidget* parent) : QWidget(parent) {
  knownCommands_ = {"ping", "stats", "players", "kick_player", "teleport", "broadcast",
                    "reload_config", "shutdown", "set_log_level", "sessions_count", "clients"};

  auto* layout = new QVBoxLayout(this);
  output_ = new QPlainTextEdit(this);
  output_->setReadOnly(true);
  layout->addWidget(output_);

  auto* row = new QHBoxLayout();
  serviceCombo_ = new QComboBox(this);
  cmdEdit_ = new QLineEdit(this);
  cmdEdit_->setPlaceholderText("comando [arg=value ...]  (ex: stats | kick_player player_id=42)");
  auto* sendBtn = new QPushButton("Enviar", this);
  connect(sendBtn, &QPushButton::clicked, this, &GmConsoleWidget::onSubmit);
  row->addWidget(serviceCombo_);
  row->addWidget(cmdEdit_, 1);
  row->addWidget(sendBtn);
  layout->addLayout(row);

  cmdEdit_->installEventFilter(this);
}

void GmConsoleWidget::setServices(const QStringList& serviceIds) {
  serviceCombo_->clear();
  serviceCombo_->addItems(serviceIds);
}

void GmConsoleWidget::appendOutput(const QString& line) {
  output_->appendPlainText(line);
}

void GmConsoleWidget::onSubmit() {
  const QString line = cmdEdit_->text().trimmed();
  if (line.isEmpty()) return;
  history_.append(line);
  historyIndex_ = history_.size();

  const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
  if (parts.isEmpty()) return;
  const QString cmd = parts[0];

  QJsonObject args;
  for (int i = 1; i < parts.size(); ++i) {
    const int eq = parts[i].indexOf('=');
    if (eq > 0) {
      args[parts[i].left(eq)] = parts[i].mid(eq + 1);
    }
  }

  const QString serviceId = serviceCombo_->currentText();
  appendOutput(QString("> [%1] %2").arg(serviceId, line));
  emit commandSubmitted(serviceId, cmd, args);
  cmdEdit_->clear();
}

void GmConsoleWidget::onHistoryUp() {
  if (history_.isEmpty()) return;
  if (historyIndex_ <= 0) historyIndex_ = history_.size();
  --historyIndex_;
  cmdEdit_->setText(history_[historyIndex_]);
}

bool GmConsoleWidget::eventFilter(QObject* obj, QEvent* event) {
  if (obj == cmdEdit_ && event->type() == QEvent::KeyPress) {
    auto* ke = static_cast<QKeyEvent*>(event);
    if (ke->key() == Qt::Key_Up) {
      onHistoryUp();
      return true;
    }
    if (ke->key() == Qt::Key_Tab) {
      const QString prefix = cmdEdit_->text();
      for (const auto& c : knownCommands_) {
        if (c.startsWith(prefix)) {
          cmdEdit_->setText(c + " ");
          break;
        }
      }
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}
