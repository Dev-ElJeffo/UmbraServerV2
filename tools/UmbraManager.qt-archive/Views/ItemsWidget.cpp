#include "ItemsWidget.hpp"
#include "PhpAdminClient.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonObject>

ItemsWidget::ItemsWidget(PhpAdminClient* php, QWidget* parent)
    : QWidget(parent), php_(php) {
  auto* layout = new QVBoxLayout(this);
  auto* toolbar = new QHBoxLayout();
  auto* refreshBtn = new QPushButton("Atualizar", this);
  auto* createBtn = new QPushButton("Criar item", this);
  auto* deleteBtn = new QPushButton("Deletar", this);
  connect(refreshBtn, &QPushButton::clicked, this, &ItemsWidget::refresh);
  connect(createBtn, &QPushButton::clicked, this, &ItemsWidget::onCreate);
  connect(deleteBtn, &QPushButton::clicked, this, &ItemsWidget::onDelete);
  toolbar->addWidget(refreshBtn);
  toolbar->addWidget(createBtn);
  toolbar->addWidget(deleteBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  table_ = new QTableWidget(this);
  table_->setColumnCount(5);
  table_->setHorizontalHeaderLabels({"ID", "Nome", "Tipo", "Raridade", "Slot"});
  table_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table_);

  connect(php_, &PhpAdminClient::listItemsFinished, this, &ItemsWidget::onListResult);
  connect(php_, &PhpAdminClient::deleteItemFinished, this, [this](bool ok, const QString& err) {
    if (!ok) QMessageBox::warning(this, "Itens", err);
    else refresh();
  });
  connect(php_, &PhpAdminClient::createItemFinished, this, [this](bool ok, const QString& err) {
    if (!ok) QMessageBox::warning(this, "Itens", err);
    else refresh();
  });
}

void ItemsWidget::refresh() { php_->listItems(); }

void ItemsWidget::onListResult(bool ok, const QJsonArray& items, const QString& error) {
  if (!ok) {
    QMessageBox::warning(this, "Itens", error);
    return;
  }
  table_->setRowCount(items.size());
  for (int i = 0; i < items.size(); ++i) {
    const QJsonObject it = items[i].toObject();
    table_->setItem(i, 0, new QTableWidgetItem(QString::number(it.value("id").toInt(it.value("item_id").toInt()))));
    table_->setItem(i, 1, new QTableWidgetItem(it.value("name").toString(it.value("item_name").toString())));
    table_->setItem(i, 2, new QTableWidgetItem(it.value("type").toString(it.value("item_type").toString())));
    table_->setItem(i, 3, new QTableWidgetItem(it.value("rarity").toString()));
    table_->setItem(i, 4, new QTableWidgetItem(it.value("equipment_slot").toString()));
  }
}

void ItemsWidget::onDelete() {
  const int row = table_->currentRow();
  if (row < 0) return;
  const int id = table_->item(row, 0)->text().toInt();
  if (QMessageBox::question(this, "Confirmar", "Deletar item?") == QMessageBox::Yes) {
    php_->deleteItem(id);
  }
}

void ItemsWidget::onCreate() {
  const QString name = QInputDialog::getText(this, "Criar item", "Nome:");
  if (name.isEmpty()) return;
  QJsonObject item;
  item["name"] = name;
  item["item_name"] = name;
  item["type"] = "consumable";
  item["item_type"] = "consumable";
  php_->createItem(item);
}
