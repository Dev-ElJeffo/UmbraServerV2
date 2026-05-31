#include "AccountsWidget.hpp"
#include "PhpAdminClient.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>

AccountsWidget::AccountsWidget(PhpAdminClient* php, QWidget* parent)
    : QWidget(parent), php_(php) {
  auto* layout = new QVBoxLayout(this);
  auto* toolbar = new QHBoxLayout();
  auto* refreshBtn = new QPushButton("Atualizar", this);
  auto* banBtn = new QPushButton("Banir", this);
  auto* unbanBtn = new QPushButton("Desbanir", this);
  connect(refreshBtn, &QPushButton::clicked, this, &AccountsWidget::refresh);
  connect(banBtn, &QPushButton::clicked, this, &AccountsWidget::onBan);
  connect(unbanBtn, &QPushButton::clicked, this, &AccountsWidget::onUnban);
  toolbar->addWidget(refreshBtn);
  toolbar->addWidget(banBtn);
  toolbar->addWidget(unbanBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  table_ = new QTableWidget(this);
  table_->setColumnCount(7);
  table_->setHorizontalHeaderLabels(
      {"ID", "Username", "Email", "Admin", "Banido", "Players", "Criado em"});
  table_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table_);

  connect(php_, &PhpAdminClient::listAccountsFinished, this, &AccountsWidget::onListResult);
}

void AccountsWidget::refresh() { php_->listAccounts(); }

void AccountsWidget::onListResult(bool ok, const QJsonArray& accounts, const QJsonObject& stats,
                                  const QString& error) {
  if (!ok) {
    QMessageBox::warning(this, "Contas", error);
    return;
  }
  accounts_ = accounts;
  table_->setRowCount(accounts.size());
  for (int i = 0; i < accounts.size(); ++i) {
    const QJsonObject a = accounts[i].toObject();
    table_->setItem(i, 0, new QTableWidgetItem(QString::number(a.value("id").toInt())));
    table_->setItem(i, 1, new QTableWidgetItem(a.value("username").toString()));
    table_->setItem(i, 2, new QTableWidgetItem(a.value("email").toString()));
    table_->setItem(i, 3, new QTableWidgetItem(a.value("isadmin").toInt() ? "Sim" : "Não"));
    table_->setItem(i, 4, new QTableWidgetItem(a.value("banned").toInt() ? "Sim" : "Não"));
    table_->setItem(i, 5, new QTableWidgetItem(QString::number(a.value("player_count").toInt())));
    table_->setItem(i, 6, new QTableWidgetItem(a.value("created_at").toString()));
  }
  setToolTip(QString("Total: %1 | Admins: %2 | Banidos: %3")
                 .arg(stats.value("total").toInt())
                 .arg(stats.value("admins").toInt())
                 .arg(stats.value("banned").toInt()));
}

void AccountsWidget::onBan() {
  const int row = table_->currentRow();
  if (row < 0) return;
  const int id = table_->item(row, 0)->text().toInt();
  const QString reason = QInputDialog::getText(this, "Banir conta", "Motivo:");
  if (QMessageBox::question(this, "Confirmar", "Banir conta?") == QMessageBox::Yes) {
    emit banRequested(id, reason);
    php_->banAccount(id, reason);
  }
}

void AccountsWidget::onUnban() {
  const int row = table_->currentRow();
  if (row < 0) return;
  const int id = table_->item(row, 0)->text().toInt();
  if (QMessageBox::question(this, "Confirmar", "Desbanir conta?") == QMessageBox::Yes) {
    emit unbanRequested(id);
    php_->unbanAccount(id);
  }
}
