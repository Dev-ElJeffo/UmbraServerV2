#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>

class AdminClient : public QObject {
  Q_OBJECT
 public:
  explicit AdminClient(QObject* parent = nullptr);

  void connectToService(const QString& host, quint16 port, const QString& secret);
  void disconnectFromService();
  bool isConnected() const;
  bool isAuthenticated() const;

  void sendCommand(const QString& cmd, const QJsonObject& args = {});

 signals:
  void connectedChanged(bool connected);
  void authenticatedChanged(bool authenticated);
  void responseReceived(const QString& cmd, const QJsonObject& response);
  void errorOccurred(const QString& message);

 private slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();
  void onError(QAbstractSocket::SocketError error);

 private:
  void sendFrame(const QJsonObject& payload);
  void processBuffer();
  void performHandshake();
  static QByteArray hmacSha256Hex(const QString& key, const QString& message);
  static QByteArray encodeFrame(const QByteArray& body);

  QTcpSocket socket_;
  QByteArray buffer_;
  QString secret_;
  bool authenticated_ = false;
  QString pendingCmd_;
};
