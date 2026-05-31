#include "LoginDialog.hpp"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Login Admin — UmbraManager");
  resize(360, 140);

  usernameEdit_ = new QLineEdit(this);
  usernameEdit_->setPlaceholderText("username admin");

  auto* layout = new QFormLayout(this);
  layout->addRow(new QLabel("Conta admin (isadmin=1):"), usernameEdit_);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addRow(buttons);
}

QString LoginDialog::username() const {
  return usernameEdit_->text().trimmed();
}
