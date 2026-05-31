#include "DashboardWidget.hpp"
#include <QGridLayout>
#include <QGroupBox>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

DashboardWidget::DashboardWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);

  auto* summary = new QHBoxLayout();
  totalPlayersLabel_ = new QLabel("Players online: 0", this);
  totalZonesLabel_ = new QLabel("Zones ativas: 0", this);
  summary->addWidget(totalPlayersLabel_);
  summary->addWidget(totalZonesLabel_);
  summary->addStretch();
  layout->addLayout(summary);

  auto* grid = new QGridLayout();
  layout->addLayout(grid);

  const QStringList services = {"auth", "world", "chat", "gateway"};
  int row = 0;
  for (const auto& s : services) {
    auto* box = new QGroupBox(s.toUpper(), this);
    auto* v = new QVBoxLayout(box);
    auto* st = new QLabel("Offline", box);
    auto* stats = new QLabel("-", box);
    st->setStyleSheet("font-weight:bold;color:#c0392b;");
    v->addWidget(st);
    v->addWidget(stats);
    statusLabels_[s] = st;
    statsLabels_[s] = stats;
    grid->addWidget(box, row / 2, row % 2);
    ++row;
  }

  auto* chart = new QtCharts::QChart();
  chart->setTitle("CPU agregado (placeholder)");
  cpuChartView_ = new QtCharts::QChartView(chart, this);
  cpuChartView_->setMinimumHeight(220);
  layout->addWidget(cpuChartView_);
}

void DashboardWidget::updateServiceStatus(const QString& serviceId, bool online,
                                          const QJsonObject& stats) {
  if (!statusLabels_.contains(serviceId)) return;
  statusLabels_[serviceId]->setText(online ? "Online" : "Offline");
  statusLabels_[serviceId]->setStyleSheet(
      online ? "font-weight:bold;color:#27ae60;" : "font-weight:bold;color:#c0392b;");
  if (online) {
    statsLabels_[serviceId]->setText(
        QString("CPU: %1% | RAM: %2 MB | Uptime: %3s")
            .arg(stats.value("cpu_pct").toDouble(), 0, 'f', 1)
            .arg(stats.value("mem_mb").toDouble(), 0, 'f', 0)
            .arg(stats.value("uptime_s").toInt()));
  } else {
    statsLabels_[serviceId]->setText("-");
  }
}

void DashboardWidget::setTotalPlayers(int count) {
  totalPlayersLabel_->setText(QString("Players online: %1").arg(count));
}

void DashboardWidget::setTotalZones(int count) {
  totalZonesLabel_->setText(QString("Zones ativas: %1").arg(count));
}
