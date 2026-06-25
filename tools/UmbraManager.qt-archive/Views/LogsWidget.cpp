#include "LogsWidget.hpp"
#include "LogTailer.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QColor>

LogsWidget::LogsWidget(LogTailer* tailer, QWidget* parent)
    : QWidget(parent), tailer_(tailer) {
  auto* layout = new QVBoxLayout(this);
  auto* toolbar = new QHBoxLayout();
  levelFilter_ = new QComboBox(this);
  levelFilter_->addItems({"ALL", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"});
  connect(levelFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogsWidget::onFilterChanged);
  auto* exportBtn = new QPushButton("Exportar aba", this);
  connect(exportBtn, &QPushButton::clicked, this, &LogsWidget::onExport);
  toolbar->addWidget(levelFilter_);
  toolbar->addWidget(exportBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  tabs_ = new QTabWidget(this);
  layout->addWidget(tabs_);
}

void LogsWidget::registerServiceTab(const QString& serviceId, const QString& logPath) {
  auto* editor = new QPlainTextEdit(this);
  editor->setReadOnly(true);
  editor->setMaximumBlockCount(10000);
  editors_[serviceId] = editor;
  tabs_->addTab(editor, serviceId);
  tailer_->watchLog(serviceId, logPath);
  editor->appendPlainText(tailer_->readExisting(serviceId, 300));
}

void LogsWidget::appendLine(const QString& serviceId, const QString& line) {
  if (!editors_.contains(serviceId)) return;
  if (!passesFilter(line)) return;

  auto* editor = editors_[serviceId];
  QTextCharFormat fmt;
  if (line.contains("[error]", Qt::CaseInsensitive) || line.contains("ERROR")) {
    fmt.setForeground(Qt::red);
  } else if (line.contains("WARN")) {
    fmt.setForeground(QColor("#e67e22"));
  } else if (line.contains("INFO")) {
    fmt.setForeground(QColor("#2980b9"));
  } else if (line.contains("DEBUG")) {
    fmt.setForeground(QColor("#7f8c8d"));
  } else if (line.contains("CRITICAL")) {
    fmt.setForeground(Qt::magenta);
  }

  QTextCursor cursor(editor->textCursor());
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(line + "\n", fmt);
}

bool LogsWidget::passesFilter(const QString& line) const {
  const QString lvl = levelFilter_->currentText();
  if (lvl == "ALL") return true;
  return line.contains(lvl, Qt::CaseInsensitive);
}

void LogsWidget::onFilterChanged() { /* filtro aplicado em appendLine */ }

void LogsWidget::onExport() {
  auto* editor = qobject_cast<QPlainTextEdit*>(tabs_->currentWidget());
  if (!editor) return;
  const QString path = QFileDialog::getSaveFileName(this, "Salvar log", {}, "Text (*.txt)");
  if (path.isEmpty()) return;
  QFile f(path);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(editor->toPlainText().toUtf8());
  }
}
