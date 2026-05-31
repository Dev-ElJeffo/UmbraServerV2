#include "PlayersWidget.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

PlayersWidget::PlayersWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  table_ = new QTableWidget(this);
  table_->setColumnCount(8);
  table_->setHorizontalHeaderLabels(
      {"Player ID", "Nome", "Zone", "X", "Y", "Z", "Morto", "TS"});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table_, &QTableWidget::customContextMenuRequested, this, &PlayersWidget::showContextMenu);
  layout->addWidget(table_);
}

void PlayersWidget::updatePlayers(const QJsonArray& players) {
  currentPlayers_ = players;
  table_->setRowCount(players.size());
  for (int i = 0; i < players.size(); ++i) {
    const QJsonObject p = players[i].toObject();
    table_->setItem(i, 0, new QTableWidgetItem(QString::number(p.value("player_id").toInt())));
    table_->setItem(i, 1, new QTableWidgetItem(p.value("name").toString()));
    table_->setItem(i, 2, new QTableWidgetItem(p.value("zone_service").toString()));
    table_->setItem(i, 3, new QTableWidgetItem(QString::number(p.value("x").toDouble(), 'f', 1)));
    table_->setItem(i, 4, new QTableWidgetItem(QString::number(p.value("y").toDouble(), 'f', 1)));
    table_->setItem(i, 5, new QTableWidgetItem(QString::number(p.value("z").toDouble(), 'f', 1)));
    table_->setItem(i, 6, new QTableWidgetItem(p.value("is_dead").toBool() ? "Sim" : "Não"));
    table_->setItem(i, 7, new QTableWidgetItem(QString::number(p.value("ts_ms").toInt())));
  }
}

void PlayersWidget::showContextMenu(const QPoint& pos) {
  const int row = table_->rowAt(pos.y());
  if (row < 0 || row >= currentPlayers_.size()) return;

  const QJsonObject p = currentPlayers_[row].toObject();
  const quint32 playerId = static_cast<quint32>(p.value("player_id").toInt());
  const QString zoneId = p.value("zone_service").toString();

  QMenu menu(this);
  QAction* kick = menu.addAction("Kick");
  QAction* tp = menu.addAction("Teleport...");
  QAction* bc = menu.addAction("Broadcast na zone...");
  const QAction* chosen = menu.exec(table_->viewport()->mapToGlobal(pos));
  if (!chosen) return;

  if (chosen == kick) {
    if (QMessageBox::question(this, "Confirmar", "Kick player?") == QMessageBox::Yes) {
      emit kickPlayerRequested(playerId, zoneId);
    }
  } else if (chosen == tp) {
    bool ok = false;
    const double x = QInputDialog::getDouble(this, "Teleport", "X", p.value("x").toDouble(), -1e9, 1e9, 1, &ok);
    if (!ok) return;
    const double y = QInputDialog::getDouble(this, "Teleport", "Y", p.value("y").toDouble(), -1e9, 1e9, 1, &ok);
    if (!ok) return;
    const double z = QInputDialog::getDouble(this, "Teleport", "Z", p.value("z").toDouble(), -1e9, 1e9, 1, &ok);
    if (!ok) return;
    emit teleportPlayerRequested(playerId, zoneId, static_cast<float>(x), static_cast<float>(y),
                                 static_cast<float>(z));
  } else if (chosen == bc) {
    const QString msg = QInputDialog::getText(this, "Broadcast", "Mensagem:");
    if (!msg.isEmpty()) emit broadcastRequested(zoneId, msg);
  }
}
