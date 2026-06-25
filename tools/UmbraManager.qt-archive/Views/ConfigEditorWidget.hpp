#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QLabel>

class ConfigEditorWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ConfigEditorWidget(QWidget* parent = nullptr);

 signals:
  void reloadConfigRequested();
  void saveConfigRequested(const QString& jsonText);

 public slots:
  void loadFromFile(const QString& path);
  void setStatus(const QString& text);

 private slots:
  void onSave();
  void onReload();
  void onValidate();

 private:
  QPlainTextEdit* editor_;
  QLabel* status_;
};
