#include "MainWindow.hpp"
#include "LoginDialog.hpp"
#include "AppConfig.hpp"
#include "PhpAdminClient.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName("UmbraManager");
  QApplication::setOrganizationName("UmbraEternum");

  LoginDialog login;
  if (login.exec() != QDialog::Accepted || login.username().isEmpty()) {
    return 0;
  }

  AppConfig::instance().setAdminUsername(login.username());

  MainWindow window;
  window.show();
  return app.exec();
}
