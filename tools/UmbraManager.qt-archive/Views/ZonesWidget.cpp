#include "ZonesWidget.hpp"
#include <QVBoxLayout>
#include <QHeaderView>

ZonesWidget::ZonesWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  table_ = new QTableWidget(this);
  table_->setColumnCount(6);
  table_->setHorizontalHeaderLabels({"Zone ID", "Nome", "Porta", "Admin", "Players", "Status"});
  table_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table_);
}

void ZonesWidget::updateZones(const QJsonArray& zones) {
  table_->setRowCount(zones.size());
  for (int i = 0; i < zones.size(); ++i) {
    const QJsonObject z = zones[i].toObject();
    table_->setItem(i, 0, new QTableWidgetItem(QString::number(z.value("zone_id").toInt())));
    table_->setItem(i, 1, new QTableWidgetItem(z.value("zone_name").toString()));
    table_->setItem(i, 2, new QTableWidgetItem(QString::number(z.value("port").toInt())));
    table_->setItem(i, 3, new QTableWidgetItem(QString::number(z.value("admin_port").toInt())));
    table_->setItem(i, 4, new QTableWidgetItem(QString::number(z.value("players_online").toInt())));
    table_->setItem(i, 5, new QTableWidgetItem(z.value("online").toBool() ? "Online" : "Offline"));
  }
}
