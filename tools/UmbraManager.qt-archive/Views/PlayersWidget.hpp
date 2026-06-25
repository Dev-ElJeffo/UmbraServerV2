#pragma once

#include <QWidget>
#include <QTableWidget>

class PlayersWidget : public QWidget {
  Q_OBJECT
 public:
  explicit PlayersWidget(QWidget* parent = nullptr);

 public slots:
  void updatePlayers(const QJsonArray& players);

 signals:
  void kickPlayerRequested(quint32 playerId, const QString& zoneServiceId);
  void teleportPlayerRequested(quint32 playerId, const QString& zoneServiceId, float x, float y, float z);
  void broadcastRequested(const QString& zoneServiceId, const QString& message);

 private slots:
  void showContextMenu(const QPoint& pos);

 private:
  QTableWidget* table_;
  QJsonArray currentPlayers_;
};
