#pragma once

#include <QWidget>
#include <QTableWidget>

class ZonesWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ZonesWidget(QWidget* parent = nullptr);

 public slots:
  void updateZones(const QJsonArray& zones);

 private:
  QTableWidget* table_;
};
