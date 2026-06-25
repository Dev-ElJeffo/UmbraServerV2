#pragma once

#include <QDialog>
#include <QLineEdit>

class LoginDialog : public QDialog {
  Q_OBJECT
 public:
  explicit LoginDialog(QWidget* parent = nullptr);
  QString username() const;

 private:
  QLineEdit* usernameEdit_;
};
