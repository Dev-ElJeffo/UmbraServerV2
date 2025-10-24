#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <optional>

namespace Umbra {
namespace Network {

/**
 * @brief Tipo de mensagem de rede
 */
enum class MessageType : uint16_t {
  // Auth messages
  LOGIN_REQUEST = 1000,
  LOGIN_RESPONSE = 1001,
  LOGOUT = 1002,
  
  // Player messages
  PLAYER_MOVE = 2000,
  PLAYER_ACTION = 2001,
  PLAYER_UPDATE = 2002,
  
  // Chat messages
  CHAT_MESSAGE = 3000,
  CHAT_WHISPER = 3001,
  CHAT_GUILD = 3002,
  
  // World messages
  WORLD_EVENT = 4000,
  SPAWN_ENTITY = 4001,
  DESPAWN_ENTITY = 4002,
  
  // System messages
  HEARTBEAT = 9000,
  ERROR = 9999
};

/**
 * @brief Estrutura de mensagem de rede
 * 
 * Formato: [Type:2][Length:4][Payload:N]
 */
struct NetworkMessage {
  MessageType type;
  std::vector<uint8_t> payload;
};

/**
 * @brief Handler de parsing e validação de mensagens
 */
class MessageHandler {
 public:
  /**
   * @brief Serializa mensagem para bytes
   * @param message Mensagem a serializar
   * @return Bytes serializados
   */
  static std::vector<uint8_t> serialize(const NetworkMessage& message);
  
  /**
   * @brief Deserializa bytes para mensagem
   * @param data Bytes a deserializar
   * @return Mensagem deserializada ou nullopt se inválido
   */
  static std::optional<NetworkMessage> deserialize(const std::vector<uint8_t>& data);
  
  /**
   * @brief Cria mensagem de erro
   * @param errorMessage Mensagem de erro
   * @return Mensagem de erro
   */
  static NetworkMessage createError(const std::string& errorMessage);
  
  /**
   * @brief Cria mensagem de heartbeat
   * @return Mensagem de heartbeat
   */
  static NetworkMessage createHeartbeat();
  
  /**
   * @brief Valida mensagem
   * @param message Mensagem a validar
   * @return true se válida
   */
  static bool validate(const NetworkMessage& message);
  
  /**
   * @brief Obtém string de tipo de mensagem
   * @param type Tipo da mensagem
   * @return Nome do tipo
   */
  static std::string getTypeName(MessageType type);
  
  /**
   * @brief Extrai string do payload
   * @param payload Payload da mensagem
   * @return String extraída
   */
  static std::string extractString(const std::vector<uint8_t>& payload);
  
  /**
   * @brief Cria payload de string
   * @param str String a incluir
   * @return Payload
   */
  static std::vector<uint8_t> createStringPayload(const std::string& str);
  
  /**
   * @brief Extrai uint32 do payload
   * @param payload Payload da mensagem
   * @param offset Offset no payload
   * @return Valor uint32
   */
  static uint32_t extractUInt32(const std::vector<uint8_t>& payload, size_t offset = 0);
  
  /**
   * @brief Adiciona uint32 ao payload
   * @param payload Payload de destino
   * @param value Valor a adicionar
   */
  static void appendUInt32(std::vector<uint8_t>& payload, uint32_t value);
};

}  // namespace Network
}  // namespace Umbra

