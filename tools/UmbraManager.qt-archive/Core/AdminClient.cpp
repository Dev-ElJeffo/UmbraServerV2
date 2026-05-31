#include "AdminClient.hpp"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QMessageAuthenticationCode>
#include <QUuid>

AdminClient::AdminClient(QObject* parent) : QObject(parent) {
  connect(&socket_, &QTcpSocket::readyRead, this, &AdminClient::onReadyRead);
  connect(&socket_, &QTcpSocket::connected, this, &AdminClient::onConnected);
  connect(&socket_, &QTcpSocket::disconnected, this, &AdminClient::onDisconnected);
  connect(&socket_, &QTcpSocket::errorOccurred, this, &AdminClient::onError);
}

void AdminClient::connectToService(const QString& host, quint16 port, const QString& secret) {
  secret_ = secret;
  authenticated_ = false;
  buffer_.clear();
  socket_.connectToHost(host, port);
}

void AdminClient::disconnectFromService() {
  socket_.disconnectFromHost();
}

bool AdminClient::isConnected() const {
  return socket_.state() == QAbstractSocket::ConnectedState;
}

bool AdminClient::isAuthenticated() const {
  return authenticated_;
}

void AdminClient::sendCommand(const QString& cmd, const QJsonObject& args) {
  if (!authenticated_) {
    emit errorOccurred("Não autenticado no canal admin");
    return;
  }
  pendingCmd_ = cmd;
  QJsonObject payload;
  payload["type"] = "command";
  payload["cmd"] = cmd;
  payload["args"] = args;
  sendFrame(payload);
}

void AdminClient::sendFrame(const QJsonObject& payload) {
  const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  socket_.write(encodeFrame(body));
}

QByteArray AdminClient::encodeFrame(const QByteArray& body) {
  QByteArray frame;
  const quint32 size = static_cast<quint32>(body.size());
  frame.append(reinterpret_cast<const char*>(&size), 4);
  frame.append(body);
  return frame;
}

QByteArray AdminClient::hmacSha256Hex(const QString& key, const QString& message) {
  QMessageAuthenticationCode mac(QCryptographicHash::Sha256);
  mac.setKey(key.toUtf8());
  mac.addData(message.toUtf8());
  return mac.result().toHex();
}

void AdminClient::performHandshake() {
  const QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const QByteArray hmac = hmacSha256Hex(secret_, nonce);
  QJsonObject payload;
  payload["type"] = "handshake";
  payload["nonce"] = nonce;
  payload["hmac"] = QString::fromLatin1(hmac);
  sendFrame(payload);
}

void AdminClient::onConnected() {
  emit connectedChanged(true);
  performHandshake();
}

void AdminClient::onDisconnected() {
  authenticated_ = false;
  emit authenticatedChanged(false);
  emit connectedChanged(false);
}

void AdminClient::onError(QAbstractSocket::SocketError) {
  emit errorOccurred(socket_.errorString());
}

void AdminClient::onReadyRead() {
  buffer_.append(socket_.readAll());
  processBuffer();
}

void AdminClient::processBuffer() {
  while (buffer_.size() >= 4) {
    quint32 frameSize = 0;
    memcpy(&frameSize, buffer_.constData(), 4);
    if (frameSize == 0 || frameSize > 1024 * 1024) {
      emit errorOccurred("Frame inválido");
      socket_.disconnectFromHost();
      return;
    }
    if (buffer_.size() < static_cast<int>(4 + frameSize)) break;

    const QByteArray body = buffer_.mid(4, static_cast<int>(frameSize));
    buffer_.remove(0, 4 + static_cast<int>(frameSize));

    const QJsonObject json = QJsonDocument::fromJson(body).object();
    const QString type = json.value("type").toString();

    if (type == "handshake_ok" || (!authenticated_ && json.value("success").toBool() && type == "handshake_ok")) {
      authenticated_ = true;
      emit authenticatedChanged(true);
      continue;
    }
    if (!authenticated_ && json.value("success").toBool() && json.contains("service")) {
      authenticated_ = true;
      emit authenticatedChanged(true);
      continue;
    }

    const QString cmd = json.value("cmd").toString(pendingCmd_);
    emit responseReceived(cmd, json);
  }
}
