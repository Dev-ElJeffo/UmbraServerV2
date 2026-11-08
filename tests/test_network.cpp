#include <gtest/gtest.h>
#include "network/MessageHandler.hpp"
#include "network/SocketServer.hpp"
#include "network/LengthPrefixedFramer.hpp"
#include "network/WebSocketServer.hpp"

#include <string>

using namespace Umbra::Network;

namespace {
std::vector<uint8_t> bytesOf(const std::string& s) {
  return std::vector<uint8_t>(s.begin(), s.end());
}
std::string strOf(const std::vector<uint8_t>& v) {
  return std::string(v.begin(), v.end());
}
}  // namespace

// Prefixo + payload no mesmo recv (caso comum).
TEST(LengthPrefixedFramerTest, SingleFrameInOneChunk) {
  LengthPrefixedFramer framer;
  auto frame = LengthPrefixedFramer::encode(bytesOf("SGVsbG8="));
  std::vector<std::vector<uint8_t>> out;
  EXPECT_EQ(framer.feed(frame.data(), frame.size(), out), LengthPrefixedFramer::Result::Ok);
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(strOf(out[0]), "SGVsbG8=");
  EXPECT_TRUE(framer.isFramed());
  EXPECT_EQ(framer.pendingBytes(), 0u);
}

// UmbraTCPClient faz dois Send(): prefixo e payload podem chegar separados.
TEST(LengthPrefixedFramerTest, PrefixAndPayloadInSeparateChunks) {
  LengthPrefixedFramer framer;
  auto frame = LengthPrefixedFramer::encode(bytesOf("payload-123"));
  std::vector<std::vector<uint8_t>> out;
  framer.feed(frame.data(), 4, out);
  EXPECT_TRUE(out.empty());
  framer.feed(frame.data() + 4, frame.size() - 4, out);
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(strOf(out[0]), "payload-123");
}

// Pior caso: um byte por recv.
TEST(LengthPrefixedFramerTest, ByteByByte) {
  LengthPrefixedFramer framer;
  auto frame = LengthPrefixedFramer::encode(bytesOf("{\"action\":\"ping\"}"));
  std::vector<std::vector<uint8_t>> out;
  for (uint8_t b : frame) {
    framer.feed(&b, 1, out);
  }
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(strOf(out[0]), "{\"action\":\"ping\"}");
}

// Bug antigo: byte de tamanho 0x50 ('P') entrava na string Base64. Agora o prefixo é consumido.
TEST(LengthPrefixedFramerTest, SizeByteInsideBase64AlphabetIsNotLeaked) {
  LengthPrefixedFramer framer;
  std::string payload(0x150, 'A');  // 336 bytes → prefixo 00 00 01 50
  auto frame = LengthPrefixedFramer::encode(bytesOf(payload));
  ASSERT_EQ(frame[3], 0x50);
  std::vector<std::vector<uint8_t>> out;
  framer.feed(frame.data(), frame.size(), out);
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].size(), payload.size());
  EXPECT_EQ(strOf(out[0]), payload);
}

TEST(LengthPrefixedFramerTest, MultipleFramesInOneChunk) {
  LengthPrefixedFramer framer;
  auto a = LengthPrefixedFramer::encode(bytesOf("first"));
  auto b = LengthPrefixedFramer::encode(bytesOf("second"));
  std::vector<uint8_t> chunk(a);
  chunk.insert(chunk.end(), b.begin(), b.end());
  chunk.push_back(0x00);  // início de um 3º frame (parcial)
  std::vector<std::vector<uint8_t>> out;
  framer.feed(chunk.data(), chunk.size(), out);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(strOf(out[0]), "first");
  EXPECT_EQ(strOf(out[1]), "second");
  EXPECT_EQ(framer.pendingBytes(), 1u);
}

// Scripts antigos (JSON cru, sem prefixo) continuam funcionando.
TEST(LengthPrefixedFramerTest, LegacyRawJsonPassThrough) {
  LengthPrefixedFramer framer;
  auto raw = bytesOf("{\"action\":\"ping\"}");
  std::vector<std::vector<uint8_t>> out;
  EXPECT_EQ(framer.feed(raw.data(), raw.size(), out), LengthPrefixedFramer::Result::Ok);
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(strOf(out[0]), "{\"action\":\"ping\"}");
  EXPECT_EQ(framer.mode(), LengthPrefixedFramer::Mode::Legacy);
}

TEST(LengthPrefixedFramerTest, RejectsOversizedAndZeroFrames) {
  {
    LengthPrefixedFramer framer(1024);
    const uint8_t big[] = {0x00, 0x00, 0x08, 0x00};  // 2048 > 1024
    std::vector<std::vector<uint8_t>> out;
    EXPECT_EQ(framer.feed(big, sizeof(big), out),
              LengthPrefixedFramer::Result::InvalidFrameSize);
    EXPECT_TRUE(out.empty());
  }
  {
    LengthPrefixedFramer framer;
    const uint8_t zero[] = {0x00, 0x00, 0x00, 0x00};
    std::vector<std::vector<uint8_t>> out;
    EXPECT_EQ(framer.feed(zero, sizeof(zero), out),
              LengthPrefixedFramer::Result::InvalidFrameSize);
  }
}

TEST(LengthPrefixedFramerTest, EncodeIsBigEndian) {
  auto frame = LengthPrefixedFramer::encode(std::vector<uint8_t>(0x010203, 'x'));
  ASSERT_GE(frame.size(), 4u);
  EXPECT_EQ(frame[0], 0x00);
  EXPECT_EQ(frame[1], 0x01);
  EXPECT_EQ(frame[2], 0x02);
  EXPECT_EQ(frame[3], 0x03);
}

TEST(MessageHandlerTest, SerializeDeserialize) {
  NetworkMessage message;
  message.type = MessageType::LOGIN_REQUEST;
  message.payload = MessageHandler::createStringPayload("test_data");
  
  auto serialized = MessageHandler::serialize(message);
  EXPECT_FALSE(serialized.empty());
  
  auto deserialized = MessageHandler::deserialize(serialized);
  ASSERT_TRUE(deserialized.has_value());
  EXPECT_EQ(deserialized->type, MessageType::LOGIN_REQUEST);
  
  std::string payload = MessageHandler::extractString(deserialized->payload);
  EXPECT_EQ(payload, "test_data");
}

TEST(MessageHandlerTest, Validation) {
  NetworkMessage message;
  message.type = MessageType::HEARTBEAT;
  
  EXPECT_TRUE(MessageHandler::validate(message));
}

TEST(MessageHandlerTest, CreateError) {
  auto errorMsg = MessageHandler::createError("Test error");
  
  EXPECT_EQ(errorMsg.type, MessageType::ERROR);
  std::string payload = MessageHandler::extractString(errorMsg.payload);
  EXPECT_EQ(payload, "Test error");
}

TEST(MessageHandlerTest, TypeNames) {
  EXPECT_EQ(MessageHandler::getTypeName(MessageType::LOGIN_REQUEST), "LOGIN_REQUEST");
  EXPECT_EQ(MessageHandler::getTypeName(MessageType::HEARTBEAT), "HEARTBEAT");
  EXPECT_EQ(MessageHandler::getTypeName(MessageType::ERROR), "ERROR");
}

TEST(MessageHandlerTest, UInt32Payload) {
  std::vector<uint8_t> payload;
  MessageHandler::appendUInt32(payload, 12345);
  
  uint32_t value = MessageHandler::extractUInt32(payload, 0);
  EXPECT_EQ(value, 12345);
}

// ---------------------------------------------------------------------------
// SocketServer (TCP loopback real)
// ---------------------------------------------------------------------------
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using TestSocket = SOCKET;
#define TEST_CLOSE_SOCKET closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using TestSocket = int;
#define TEST_CLOSE_SOCKET close
#endif

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace {

struct WinsockGuard {
#ifdef _WIN32
  WinsockGuard() { WSADATA d; WSAStartup(MAKEWORD(2, 2), &d); }
  ~WinsockGuard() { WSACleanup(); }
#endif
};

TestSocket connectLoopback(uint16_t port) {
  TestSocket s = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  for (int i = 0; i < 50; ++i) {
    if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) return s;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  TEST_CLOSE_SOCKET(s);
  return static_cast<TestSocket>(-1);
}

// Lê até `expected` bytes ou até o peer fechar.
size_t recvUntil(TestSocket s, size_t expected, std::vector<uint8_t>& out) {
  std::vector<uint8_t> buf(64 * 1024);
  while (out.size() < expected) {
    const int n = recv(s, reinterpret_cast<char*>(buf.data()), static_cast<int>(buf.size()), 0);
    if (n <= 0) break;
    out.insert(out.end(), buf.begin(), buf.begin() + n);
  }
  return out.size();
}

bool waitFor(const std::function<bool()>& cond, int timeoutMs = 3000) {
  const auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
  while (std::chrono::steady_clock::now() < end) {
    if (cond()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return cond();
}

}  // namespace

// sendToClient deve entregar tudo mesmo quando o send() do SO é parcial (resposta admin grande).
TEST(SocketServerTest, LargeSendIsDeliveredCompletely) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19471;
  SocketServer server(ProtocolType::TCP, kPort, "127.0.0.1");
  std::atomic<uint32_t> clientId{0};
  server.setConnectionCallback([&](uint32_t id, bool connected) {
    if (connected) clientId = id;
  });
  ASSERT_TRUE(server.start());

  TestSocket c = connectLoopback(kPort);
  ASSERT_NE(c, static_cast<TestSocket>(-1));
  ASSERT_TRUE(waitFor([&] { return clientId.load() != 0; }));

  std::vector<uint8_t> payload(4 * 1024 * 1024);
  for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<uint8_t>(i * 31u);

  std::vector<uint8_t> received;
  auto reader = std::async(std::launch::async, [&] { return recvUntil(c, payload.size(), received); });
  EXPECT_TRUE(server.sendToClient(clientId, payload));
  ASSERT_EQ(reader.wait_for(std::chrono::seconds(10)), std::future_status::ready);
  EXPECT_EQ(reader.get(), payload.size());
  EXPECT_EQ(received, payload);

  TEST_CLOSE_SOCKET(c);
  server.stop();
}

// stop() travava: dava join no accept()/recv() bloqueados antes de fechar os sockets.
TEST(SocketServerTest, StopReturnsWithIdleClientConnected) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19472;
  auto server = std::make_unique<SocketServer>(ProtocolType::TCP, kPort, "127.0.0.1");
  std::atomic<int> connects{0};
  std::atomic<int> disconnects{0};
  server->setConnectionCallback([&](uint32_t, bool connected) {
    (connected ? connects : disconnects)++;
  });
  ASSERT_TRUE(server->start());

  TestSocket c = connectLoopback(kPort);
  ASSERT_NE(c, static_cast<TestSocket>(-1));
  ASSERT_TRUE(waitFor([&] { return connects.load() == 1; }));

  std::promise<void> stopped;
  auto stoppedFuture = stopped.get_future();
  std::thread stopper([&] {
    server->stop();
    stopped.set_value();
  });
  const bool ok = stoppedFuture.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
  if (ok) {
    stopper.join();
  } else {
    stopper.detach();  // evita travar a suíte; o teste já falhou
  }
  ASSERT_TRUE(ok) << "SocketServer::stop() não retornou com cliente ocioso conectado";
  EXPECT_FALSE(server->isRunning());
  EXPECT_EQ(disconnects.load(), 1);

  // Cliente percebe o fechamento.
  std::vector<uint8_t> tmp;
  EXPECT_EQ(recvUntil(c, 1, tmp), 0u);
  TEST_CLOSE_SOCKET(c);
}

// Desconexão iniciada pelo servidor (ex.: frame inválido no Gateway) fecha o socket do cliente.
TEST(SocketServerTest, ServerSideDisconnectClosesClient) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19473;
  SocketServer server(ProtocolType::TCP, kPort, "127.0.0.1");
  std::atomic<uint32_t> clientId{0};
  std::atomic<int> disconnects{0};
  server.setConnectionCallback([&](uint32_t id, bool connected) {
    if (connected) clientId = id; else disconnects++;
  });
  ASSERT_TRUE(server.start());

  TestSocket c = connectLoopback(kPort);
  ASSERT_TRUE(waitFor([&] { return clientId.load() != 0; }));
  EXPECT_TRUE(server.disconnectClient(clientId));
  EXPECT_FALSE(server.disconnectClient(clientId));     // já removido
  EXPECT_FALSE(server.disconnectClient(999999));       // id inexistente (kick_client do Manager)

  std::vector<uint8_t> tmp;
  EXPECT_EQ(recvUntil(c, 1, tmp), 0u);
  EXPECT_TRUE(waitFor([&] { return server.getClientCount() == 0; }));
  EXPECT_EQ(disconnects.load(), 1);  // callback uma vez só (worker não duplica)
  EXPECT_FALSE(server.sendToClient(clientId, {1, 2, 3}));
  TEST_CLOSE_SOCKET(c);
  server.stop();
}

// Muitas conexões curtas: mensagens chegam e o servidor segue respondendo (workers reciclados).
TEST(SocketServerTest, ManySequentialConnectionsEcho) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19474;
  SocketServer server(ProtocolType::TCP, kPort, "127.0.0.1");
  server.setMessageCallback([&](uint32_t id, const std::vector<uint8_t>& data) {
    server.sendToClient(id, data);
  });
  ASSERT_TRUE(server.start());

  for (int i = 0; i < 50; ++i) {
    TestSocket c = connectLoopback(kPort);
    ASSERT_NE(c, static_cast<TestSocket>(-1)) << "conexão " << i;
    const std::string msg = "ping-" + std::to_string(i);
    send(c, msg.data(), static_cast<int>(msg.size()), 0);
    std::vector<uint8_t> echo;
    recvUntil(c, msg.size(), echo);
    EXPECT_EQ(std::string(echo.begin(), echo.end()), msg);
    TEST_CLOSE_SOCKET(c);
  }
  EXPECT_TRUE(waitFor([&] { return server.getClientCount() == 0; }));
  server.stop();
}

// ---------------------------------------------------------------------------
// WebSocketServer (zone) — handshake real em loopback
// ---------------------------------------------------------------------------
namespace {

// Conecta e faz o upgrade HTTP → WebSocket; retorna socket pronto ou -1.
TestSocket connectWebSocket(uint16_t port) {
  TestSocket s = connectLoopback(port);
  if (s == static_cast<TestSocket>(-1)) return s;
  const std::string req =
      "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
  send(s, req.data(), static_cast<int>(req.size()), 0);
  std::vector<uint8_t> resp;
  std::vector<uint8_t> buf(1024);
  while (true) {
    const int n = recv(s, reinterpret_cast<char*>(buf.data()), static_cast<int>(buf.size()), 0);
    if (n <= 0) break;
    resp.insert(resp.end(), buf.begin(), buf.begin() + n);
    const std::string text(resp.begin(), resp.end());
    if (text.find("\r\n\r\n") != std::string::npos) {
      if (text.find("101") == std::string::npos) break;
      return s;
    }
  }
  TEST_CLOSE_SOCKET(s);
  return static_cast<TestSocket>(-1);
}

// Frame cliente → servidor (mascarado, como exige o RFC 6455).
void sendMaskedBinary(TestSocket s, const std::vector<uint8_t>& payload) {
  std::vector<uint8_t> f;
  f.push_back(0x82);
  f.push_back(static_cast<uint8_t>(0x80 | payload.size()));  // < 126 bytes
  const uint8_t mask[4] = {0x11, 0x22, 0x33, 0x44};
  f.insert(f.end(), mask, mask + 4);
  for (size_t i = 0; i < payload.size(); ++i) f.push_back(payload[i] ^ mask[i % 4]);
  send(s, reinterpret_cast<const char*>(f.data()), static_cast<int>(f.size()), 0);
}

}  // namespace

// stop() travava com cliente WS ocioso (join do accept/recv antes de fechar os sockets).
TEST(WebSocketServerTest, StopReturnsWithIdleClient) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19481;
  auto server = std::make_unique<WebSocketServer>(kPort);
  std::atomic<int> connects{0};
  std::atomic<int> disconnects{0};
  server->setConnectionCallback([&](uint32_t, bool c) { (c ? connects : disconnects)++; });
  ASSERT_TRUE(server->start());

  TestSocket c = connectWebSocket(kPort);
  ASSERT_NE(c, static_cast<TestSocket>(-1));
  ASSERT_TRUE(waitFor([&] { return connects.load() == 1; }));

  std::promise<void> stopped;
  auto fut = stopped.get_future();
  std::thread stopper([&] { server->stop(); stopped.set_value(); });
  const bool ok = fut.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
  if (ok) stopper.join(); else stopper.detach();
  ASSERT_TRUE(ok) << "WebSocketServer::stop() não retornou com cliente ocioso";
  EXPECT_EQ(disconnects.load(), 1);
  TEST_CLOSE_SOCKET(c);
}

// disconnect() vindo de outra thread (writer: overflow/stall, ou kick da zone) precisa
// acordar o recv() da thread do cliente — no Linux só close() não acordava.
TEST(WebSocketServerTest, DisconnectFromOtherThreadWakesClientThread) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19482;
  WebSocketServer server(kPort);
  std::atomic<uint32_t> clientId{0};
  std::atomic<int> disconnects{0};
  std::atomic<int> received{0};
  server.setConnectionCallback([&](uint32_t id, bool c) {
    if (c) clientId = id; else disconnects++;
  });
  server.setBinaryCallback([&](uint32_t, const std::vector<uint8_t>&) { received++; });
  ASSERT_TRUE(server.start());

  TestSocket c = connectWebSocket(kPort);
  ASSERT_NE(c, static_cast<TestSocket>(-1));
  ASSERT_TRUE(waitFor([&] { return clientId.load() != 0; }));
  sendMaskedBinary(c, {250});
  ASSERT_TRUE(waitFor([&] { return received.load() == 1; }));

  std::thread kicker([&] { server.disconnect(clientId); });
  kicker.join();

  std::vector<uint8_t> tmp;
  EXPECT_EQ(recvUntil(c, 1, tmp), 0u);  // cliente vê o fechamento
  EXPECT_TRUE(waitFor([&] { return server.getClientCount() == 0; }));
  EXPECT_EQ(disconnects.load(), 1);  // callback uma vez só
  TEST_CLOSE_SOCKET(c);
  server.stop();
}

// Conexões curtas em sequência (login/logout/relog): servidor segue aceitando e as
// threads encerradas são recicladas.
TEST(WebSocketServerTest, ManySequentialConnections) {
  WinsockGuard wsa;
  constexpr uint16_t kPort = 19483;
  WebSocketServer server(kPort);
  std::atomic<int> connects{0};
  std::atomic<int> disconnects{0};
  server.setConnectionCallback([&](uint32_t, bool c) { (c ? connects : disconnects)++; });
  ASSERT_TRUE(server.start());

  for (int i = 0; i < 40; ++i) {
    TestSocket c = connectWebSocket(kPort);
    ASSERT_NE(c, static_cast<TestSocket>(-1)) << "conexão " << i;
    ASSERT_TRUE(waitFor([&] { return connects.load() == i + 1; }));
    TEST_CLOSE_SOCKET(c);
    ASSERT_TRUE(waitFor([&] { return disconnects.load() == i + 1; })) << "disconnect " << i;
  }
  EXPECT_EQ(server.getClientCount(), 0u);
  server.stop();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

