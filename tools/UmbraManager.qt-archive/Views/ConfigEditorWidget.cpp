#include "ConfigEditorWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>

ConfigEditorWidget::ConfigEditorWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  auto* toolbar = new QHBoxLayout();
  auto* validateBtn = new QPushButton("Validar JSON", this);
  auto* saveBtn = new QPushButton("Salvar arquivo", this);
  auto* reloadBtn = new QPushButton("Reload runtime (todos servidores)", this);
  connect(validateBtn, &QPushButton::clicked, this, &ConfigEditorWidget::onValidate);
  connect(saveBtn, &QPushButton::clicked, this, &ConfigEditorWidget::onSave);
  connect(reloadBtn, &QPushButton::clicked, this, &ConfigEditorWidget::onReload);
  toolbar->addWidget(validateBtn);
  toolbar->addWidget(saveBtn);
  toolbar->addWidget(reloadBtn);
  toolbar->addStretch();
  layout->addLayout(toolbar);

  editor_ = new QPlainTextEdit(this);
  layout->addWidget(editor_);
  status_ = new QLabel(this);
  layout->addWidget(status_);
}

void ConfigEditorWidget::loadFromFile(const QString& path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    status_->setText("Falha ao abrir: " + path);
    return;
  }
  editor_->setPlainText(QString::fromUtf8(f.readAll()));
  status_->setText("Carregado: " + path);
}

void ConfigEditorWidget::setStatus(const QString& text) { status_->setText(text); }

void ConfigEditorWidget::onValidate() {
  const QJsonDocument doc = QJsonDocument::fromJson(editor_->toPlainText().toUtf8());
  if (doc.isNull()) {
    QMessageBox::warning(this, "JSON inválido", "Não foi possível parsear o JSON.");
    return;
  }
  QMessageBox::information(this, "OK", "JSON válido.");
}

void ConfigEditorWidget::onSave() {
  emit saveConfigRequested(editor_->toPlainText());
}

void ConfigEditorWidget::onReload() {
  if (QMessageBox::question(this, "Confirmar", "Enviar reload_config a todos servidores?") ==
      QMessageBox::Yes) {
    emit reloadConfigRequested();
  }
}
