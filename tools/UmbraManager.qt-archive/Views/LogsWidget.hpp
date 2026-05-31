#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QComboBox>

class LogTailer;

class LogsWidget : public QWidget {
  Q_OBJECT
 public:
  explicit LogsWidget(LogTailer* tailer, QWidget* parent = nullptr);

 public slots:
  void appendLine(const QString& serviceId, const QString& line);
  void registerServiceTab(const QString& serviceId, const QString& logPath);

 private slots:
  void onFilterChanged();
  void onExport();

 private:
  LogTailer* tailer_;
  QTabWidget* tabs_;
  QComboBox* levelFilter_;
  QMap<QString, QPlainTextEdit*> editors_;
  bool passesFilter(const QString& line) const;
};
