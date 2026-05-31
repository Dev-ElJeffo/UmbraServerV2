#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>

class GmConsoleWidget : public QWidget {
  Q_OBJECT
 public:
  explicit GmConsoleWidget(QWidget* parent = nullptr);

  void setServices(const QStringList& serviceIds);

 signals:
  void commandSubmitted(const QString& serviceId, const QString& cmd, const QJsonObject& args);

 public slots:
  void appendOutput(const QString& line);

 protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

 private slots:
  void onSubmit();
  void onHistoryUp();

 private:
  QComboBox* serviceCombo_;
  QLineEdit* cmdEdit_;
  QPlainTextEdit* output_;
  QStringList history_;
  int historyIndex_ = -1;
  QStringList knownCommands_;
};
