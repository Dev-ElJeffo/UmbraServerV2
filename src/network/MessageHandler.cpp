#include "MessageHandler.hpp"
#include "core/Logger.hpp"
#include <cstring>

namespace Umbra {
namespace Network {

std::vector<uint8_t> MessageHandler::serialize(const NetworkMessage& message) {
  std::vector<uint8_t> data;
  
  // Message type (2 bytes)
  uint16_t type = static_cast<uint16_t>(message.type);
  data.push_back(static_cast<uint8_t>(type >> 8));
  data.push_back(static_cast<uint8_t>(type & 0xFF));
  
  // Payload length (4 bytes)
  uint32_t length = static_cast<uint32_t>(message.payload.size());
  data.push_back(static_cast<uint8_t>((length >> 24) & 0xFF));
  data.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
  data.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
  data.push_back(static_cast<uint8_t>(length & 0xFF));
  
  // Payload
  data.insert(data.end(), message.payload.begin(), message.payload.end());
  
  return data;
}

std::optional<NetworkMessage> MessageHandler::deserialize(const std::vector<uint8_t>& data) {
  if (data.size() < 6) {
    Core::Logger::getInstance().warn("Message too short: {} bytes", data.size());
    return std::nullopt;
  }
  
  NetworkMessage message;
  
  // Extract type
  uint16_t type = (static_cast<uint16_t>(data[0]) << 8) | data[1];
  message.type = static_cast<MessageType>(type);
  
  // Extract length
  uint32_t length = (static_cast<uint32_t>(data[2]) << 24) |
                    (static_cast<uint32_t>(data[3]) << 16) |
                    (static_cast<uint32_t>(data[4]) << 8) |
                    static_cast<uint32_t>(data[5]);
  
  // Validate length
  if (data.size() != 6 + length) {
    Core::Logger::getInstance().warn("Message length mismatch: expected {}, got {}", 
                                     6 + length, data.size());
    return std::nullopt;
  }
  
  // Extract payload
  message.payload.assign(data.begin() + 6, data.end());
  
  return message;
}

NetworkMessage MessageHandler::createError(const std::string& errorMessage) {
  NetworkMessage message;
  message.type = MessageType::ERROR;
  message.payload = createStringPayload(errorMessage);
  return message;
}

NetworkMessage MessageHandler::createHeartbeat() {
  NetworkMessage message;
  message.type = MessageType::HEARTBEAT;
  // Empty payload
  return message;
}

bool MessageHandler::validate(const NetworkMessage& message) {
  // Check if payload is reasonable size (< 1MB)
  if (message.payload.size() > 1024 * 1024) {
    return false;
  }
  
  // Add more validation rules as needed
  return true;
}

std::string MessageHandler::getTypeName(MessageType type) {
  switch (type) {
    case MessageType::LOGIN_REQUEST: return "LOGIN_REQUEST";
    case MessageType::LOGIN_RESPONSE: return "LOGIN_RESPONSE";
    case MessageType::LOGOUT: return "LOGOUT";
    case MessageType::PLAYER_MOVE: return "PLAYER_MOVE";
    case MessageType::PLAYER_ACTION: return "PLAYER_ACTION";
    case MessageType::PLAYER_UPDATE: return "PLAYER_UPDATE";
    case MessageType::CHAT_MESSAGE: return "CHAT_MESSAGE";
    case MessageType::CHAT_WHISPER: return "CHAT_WHISPER";
    case MessageType::CHAT_GUILD: return "CHAT_GUILD";
    case MessageType::WORLD_EVENT: return "WORLD_EVENT";
    case MessageType::SPAWN_ENTITY: return "SPAWN_ENTITY";
    case MessageType::DESPAWN_ENTITY: return "DESPAWN_ENTITY";
    case MessageType::HEARTBEAT: return "HEARTBEAT";
    case MessageType::ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

std::string MessageHandler::extractString(const std::vector<uint8_t>& payload) {
  if (payload.empty()) {
    return "";
  }
  
  return std::string(payload.begin(), payload.end());
}

std::vector<uint8_t> MessageHandler::createStringPayload(const std::string& str) {
  return std::vector<uint8_t>(str.begin(), str.end());
}

uint32_t MessageHandler::extractUInt32(const std::vector<uint8_t>& payload, size_t offset) {
  if (payload.size() < offset + 4) {
    return 0;
  }
  
  return (static_cast<uint32_t>(payload[offset]) << 24) |
         (static_cast<uint32_t>(payload[offset + 1]) << 16) |
         (static_cast<uint32_t>(payload[offset + 2]) << 8) |
         static_cast<uint32_t>(payload[offset + 3]);
}

void MessageHandler::appendUInt32(std::vector<uint8_t>& payload, uint32_t value) {
  payload.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
  payload.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
  payload.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  payload.push_back(static_cast<uint8_t>(value & 0xFF));
}

}  // namespace Network
}  // namespace Umbra

