#include <gtest/gtest.h>
#include "network/MessageHandler.hpp"
#include "network/SocketServer.hpp"

using namespace Umbra::Network;

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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

