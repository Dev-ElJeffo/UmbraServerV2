#pragma once

#include <cstdint>
#include <vector>
#include <cstring>

namespace Umbra {
namespace Zone {

// Frame binário: little-endian
// [msgType: uint8]
// 1 = move_update (client->server)
// 2 = state_update (server->clients)
// 
// Frame antigo (25 bytes):
// [player_id:uint32][x:float][y:float][z:float][yaw:float][ts_ms:uint32]
//
// Frame novo com animação (34 bytes):
// [player_id:uint32][x:float][y:float][z:float][yaw:float][speed:float][velocityZ:float][isInAir:uint8][ts_ms:uint32]

enum class MovementMsgType : uint8_t {
  MoveUpdate = 1,
  StateUpdate = 2,
  PlayerDisconnected = 3,
  PlayerInfoUpdate = 4,
  // ========== MENSAGENS SOCIAIS ==========
  PartyInvite = 10,           // Cliente -> Servidor: Enviar convite
  PartyInviteReceived = 11,   // Servidor -> Cliente: Receber convite
  PartyInviteResponse = 12,   // Cliente -> Servidor: Aceitar/Recusar
  PartyMemberJoined = 13,     // Servidor -> Clientes: Membro entrou
  PartyMemberLeft = 14,       // Servidor -> Clientes: Membro saiu
  PartyDisbanded = 15,        // Servidor -> Clientes: Grupo dissolvido
  TradeRequest = 20,          // Cliente -> Servidor: Solicitar troca
  TradeRequestReceived = 21, // Servidor -> Cliente: Receber solicitação
  TradeRequestResponse = 22,  // Cliente -> Servidor: Aceitar/Recusar
  TradeStarted = 23,          // Servidor -> Clientes: Troca iniciada
  TradeCancelled = 24,        // Servidor -> Clientes: Troca cancelada
  TradeStartedNotify = 25,    // Cliente -> Servidor: Notificar troca aceita (após HTTP)
  FriendRequest = 30,         // Cliente -> Servidor: Solicitar amizade
  FriendRequestReceived = 31,// Servidor -> Cliente: Receber solicitação
  FriendRequestResponse = 32, // Cliente -> Servidor: Aceitar/Recusar
  FriendAdded = 33,           // Servidor -> Clientes: Amigo adicionado
  WhisperMessage = 40,        // Cliente -> Servidor: Enviar whisper
  WhisperReceived = 41,       // Servidor -> Cliente: Receber whisper
  DuelRequest = 50,           // Cliente -> Servidor: Desafiar duelo
  DuelRequestReceived = 51,   // Servidor -> Cliente: Receber desafio
  DuelRequestResponse = 52,   // Cliente -> Servidor: Aceitar/Recusar
  DuelStarted = 53,           // Servidor -> Clientes: Duelo iniciado
  DuelEnded = 54              // Servidor -> Clientes: Duelo terminado
};

struct MovementFrame {
  MovementMsgType type;
  uint32_t playerId;
  float x;
  float y;
  float z;
  float yaw;
  uint32_t tsMs;
};

inline std::vector<uint8_t> encode(const MovementFrame& f) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4*4 + 4);  // 25 bytes (frame antigo)
  out.push_back(static_cast<uint8_t>(f.type));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeF = [&out, &write32](float fv){
    static_assert(sizeof(float)==4, "float not 4 bytes");
    uint32_t u; std::memcpy(&u, &fv, 4);
    write32(u);
  };
  write32(f.playerId);
  writeF(f.x); writeF(f.y); writeF(f.z); writeF(f.yaw);
  write32(f.tsMs);
  return out;
}

// Codificar frame com animação (34 bytes)
inline std::vector<uint8_t> encodeWithAnimation(const MovementFrame& f,
                                                float speed,
                                                float velocityZ,
                                                bool isInAir) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4*4 + 4 + 4 + 4 + 1 + 4);  // 34 bytes (frame novo)
  out.push_back(static_cast<uint8_t>(f.type));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto writeF = [&out, &write32](float fv){
    static_assert(sizeof(float)==4, "float not 4 bytes");
    uint32_t u;
    std::memcpy(&u, &fv, 4);
    write32(u);
  };
  
  write32(f.playerId);
  writeF(f.x);
  writeF(f.y);
  writeF(f.z);
  writeF(f.yaw);
  writeF(speed);        // NOVO: Velocidade horizontal
  writeF(velocityZ);    // NOVO: Velocidade vertical
  out.push_back(isInAir ? 1u : 0u);  // NOVO: Estado no ar
  write32(f.tsMs);
  
  return out;
}

inline bool decode(const std::vector<uint8_t>& data, MovementFrame& f) {
  const size_t frameSizeOld = 1 + 4 + 4*4 + 4;  // 25 bytes (frame antigo)
  if (data.size() < frameSizeOld) return false;
  
  size_t off = 0;
  f.type = static_cast<MovementMsgType>(data[off++]);
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  auto readF = [&read32](size_t& off)->float{
    uint32_t u = read32(off); float fv; std::memcpy(&fv, &u, 4); return fv;
  };
  f.playerId = read32(off);
  f.x = readF(off); f.y = readF(off); f.z = readF(off); f.yaw = readF(off);
  f.tsMs = read32(off);
  // Ignorar campos de animação se presentes (compatibilidade retroativa)
  return true;
}

// Decodificar frame com animação (34 bytes)
// Retorna true se o frame tinha animação, false se era frame antigo (25 bytes)
inline bool decodeWithAnimation(const std::vector<uint8_t>& data, 
                                 MovementFrame& f,
                                 float& speed,
                                 float& velocityZ,
                                 bool& isInAir) {
  const size_t frameSizeOld = 1 + 4 + 4*4 + 4;      // 25 bytes (frame antigo)
  const size_t frameSizeNew = frameSizeOld + 4 + 4 + 1;  // 34 bytes (frame novo)
  
  if (data.size() < frameSizeOld) return false;
  
  size_t off = 0;
  f.type = static_cast<MovementMsgType>(data[off++]);
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  auto readF = [&read32](size_t& off)->float{
    uint32_t u = read32(off);
    float fv;
    std::memcpy(&fv, &u, 4);
    return fv;
  };
  
  f.playerId = read32(off);
  f.x = readF(off);
  f.y = readF(off);
  f.z = readF(off);
  f.yaw = readF(off);
  
  // Se há pelo menos 34 bytes, tentar ler campos de animação
  if (data.size() >= frameSizeNew) {
    speed = readF(off);        // Velocidade horizontal
    velocityZ = readF(off);    // Velocidade vertical
    isInAir = (data[off++] != 0);  // Estado no ar
    f.tsMs = read32(off);
    return true;  // Frame com animação
  } else {
    // Frame antigo (25 bytes) - campos de animação com valores padrão
    f.tsMs = read32(off);
    speed = 0.0f;
    velocityZ = 0.0f;
    isInAir = false;
    return false;  // Frame sem animação
  }
}

// Codificar mensagem de desconexão de player (5 bytes: [msgType:uint8][playerId:uint32])
inline std::vector<uint8_t> encodePlayerDisconnected(uint32_t playerId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PlayerDisconnected));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(playerId);
  return out;
}

// Codificar mensagem PlayerInfoUpdate: [msgType:uint8][playerId:uint32][nameLen:uint16][name:bytes][titleLen:uint16][title:bytes]
inline std::vector<uint8_t> encodePlayerInfoUpdate(uint32_t playerId, 
                                                    const std::string& name, 
                                                    const std::string& title) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 2 + name.size() + 2 + title.size());
  
  out.push_back(static_cast<uint8_t>(MovementMsgType::PlayerInfoUpdate));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto write16 = [&out](uint16_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  
  write32(playerId);
  write16(static_cast<uint16_t>(name.size()));
  out.insert(out.end(), name.begin(), name.end());
  write16(static_cast<uint16_t>(title.size()));
  out.insert(out.end(), title.begin(), title.end());
  
  return out;
}

// Decodificar mensagem PlayerInfoUpdate
inline bool decodePlayerInfoUpdate(const std::vector<uint8_t>& data,
                                   uint32_t& playerId,
                                   std::string& name,
                                   std::string& title) {
  if (data.size() < 7) return false;  // Mínimo: msgType(1) + playerId(4) + nameLen(2)
  
  size_t off = 0;
  MovementMsgType type = static_cast<MovementMsgType>(data[off++]);
  if (type != MovementMsgType::PlayerInfoUpdate) return false;
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  auto read16 = [&data](size_t& off)->uint16_t{
    uint16_t v = static_cast<uint16_t>(data[off]) |
                 (static_cast<uint16_t>(data[off+1])<<8);
    off += 2; return v;
  };
  
  playerId = read32(off);
  uint16_t nameLen = read16(off);
  if (data.size() < off + nameLen + 2) return false;
  
  name.assign(reinterpret_cast<const char*>(data.data() + off), nameLen);
  off += nameLen;
  
  uint16_t titleLen = read16(off);
  if (data.size() < off + titleLen) return false;
  
  title.assign(reinterpret_cast<const char*>(data.data() + off), titleLen);
  return true;
}

// ============================================================================
// MENSAGENS SOCIAIS - ENCODE/DECODE
// ============================================================================

// Codificar mensagem de convite de grupo: [msgType:uint8][fromPlayerId:uint32][toPlayerId:uint32][partyId:uint32]
inline std::vector<uint8_t> encodePartyInvite(uint32_t fromPlayerId, uint32_t toPlayerId, uint32_t partyId = 0) {
  std::vector<uint8_t> out;
  out.reserve(13);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyInvite));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  write32(partyId);
  return out;
}

// Decodificar mensagem de convite de grupo
inline bool decodePartyInvite(const std::vector<uint8_t>& data, uint32_t& fromPlayerId, uint32_t& toPlayerId, uint32_t& partyId) {
  if (data.size() < 13) return false;
  size_t off = 1; // Skip msgType
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  fromPlayerId = read32(off);
  toPlayerId = read32(off);
  partyId = read32(off);
  return true;
}

// Codificar mensagem de solicitação de troca: [msgType:uint8][fromPlayerId:uint32][toPlayerId:uint32]
inline std::vector<uint8_t> encodeTradeRequest(uint32_t fromPlayerId, uint32_t toPlayerId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::TradeRequest));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  return out;
}

// Decodificar mensagem de solicitação de troca
inline bool decodeTradeRequest(const std::vector<uint8_t>& data, uint32_t& fromPlayerId, uint32_t& toPlayerId) {
  if (data.size() < 9) return false;
  size_t off = 1;
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  fromPlayerId = read32(off);
  toPlayerId = read32(off);
  return true;
}

// Codificar mensagem de solicitação de amizade: [msgType:uint8][fromPlayerId:uint32][toPlayerId:uint32]
inline std::vector<uint8_t> encodeFriendRequest(uint32_t fromPlayerId, uint32_t toPlayerId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::FriendRequest));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  return out;
}

// Decodificar mensagem de solicitação de amizade
inline bool decodeFriendRequest(const std::vector<uint8_t>& data, uint32_t& fromPlayerId, uint32_t& toPlayerId) {
  if (data.size() < 9) return false;
  size_t off = 1;
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  fromPlayerId = read32(off);
  toPlayerId = read32(off);
  return true;
}

// Codificar mensagem de whisper: [msgType:uint8][fromPlayerId:uint32][toPlayerId:uint32][msgLen:uint16][message:bytes]
inline std::vector<uint8_t> encodeWhisper(uint32_t fromPlayerId, uint32_t toPlayerId, const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4 + 2 + message.size());
  
  out.push_back(static_cast<uint8_t>(MovementMsgType::WhisperMessage));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto write16 = [&out](uint16_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  
  return out;
}

// Decodificar mensagem de whisper
inline bool decodeWhisper(const std::vector<uint8_t>& data, uint32_t& fromPlayerId, uint32_t& toPlayerId, std::string& message) {
  if (data.size() < 11) return false; // Mínimo: msgType(1) + fromId(4) + toId(4) + msgLen(2)
  
  size_t off = 1;
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  auto read16 = [&data](size_t& off)->uint16_t{
    uint16_t v = static_cast<uint16_t>(data[off]) |
                 (static_cast<uint16_t>(data[off+1])<<8);
    off += 2; return v;
  };
  
  fromPlayerId = read32(off);
  toPlayerId = read32(off);
  uint16_t msgLen = read16(off);
  
  if (data.size() < off + msgLen) return false;
  
  message.assign(reinterpret_cast<const char*>(data.data() + off), msgLen);
  return true;
}

// Codificar resposta de convite/solicitação: [msgType:uint8][requestId:uint32][accepted:uint8]
inline std::vector<uint8_t> encodeSocialResponse(MovementMsgType responseType, uint32_t requestId, bool accepted) {
  std::vector<uint8_t> out;
  out.reserve(6);
  out.push_back(static_cast<uint8_t>(responseType));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(requestId);
  out.push_back(accepted ? 1u : 0u);
  return out;
}

// Decodificar resposta de convite/solicitação
inline bool decodeSocialResponse(const std::vector<uint8_t>& data, uint32_t& requestId, bool& accepted) {
  if (data.size() < 6) return false;
  size_t off = 1;
  
  auto read32 = [&data](size_t& off)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  requestId = read32(off);
  accepted = (data[off] != 0);
  return true;
}

// Codificar mensagens de recebimento (servidor -> cliente)
inline std::vector<uint8_t> encodePartyInviteReceived(uint32_t fromPlayerId, uint32_t toPlayerId, uint32_t partyId) {
  std::vector<uint8_t> out;
  out.reserve(13);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyInviteReceived));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  write32(partyId);
  return out;
}

inline std::vector<uint8_t> encodeTradeRequestReceived(uint32_t fromPlayerId, uint32_t toPlayerId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::TradeRequestReceived));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  return out;
}

// Cliente -> Servidor: Notificar troca aceita (após HTTP)
// [msgType:25][tradeSessionId:uint32][player1Id:uint32][player2Id:uint32]
inline std::vector<uint8_t> encodeTradeStartedNotify(uint32_t tradeSessionId, uint32_t player1Id, uint32_t player2Id) {
  std::vector<uint8_t> out;
  out.reserve(13);
  out.push_back(static_cast<uint8_t>(MovementMsgType::TradeStartedNotify));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(tradeSessionId);
  write32(player1Id);
  write32(player2Id);
  return out;
}

// Servidor -> Clientes: Troca iniciada
// [msgType:23][tradeSessionId:uint32][player1Id:uint32][player2Id:uint32]
inline std::vector<uint8_t> encodeTradeStarted(uint32_t tradeSessionId, uint32_t player1Id, uint32_t player2Id) {
  std::vector<uint8_t> out;
  out.reserve(13);
  out.push_back(static_cast<uint8_t>(MovementMsgType::TradeStarted));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(tradeSessionId);
  write32(player1Id);
  write32(player2Id);
  return out;
}

// Decodificar TradeStartedNotify (cliente -> servidor)
inline bool decodeTradeStartedNotify(const std::vector<uint8_t>& data, uint32_t& tradeSessionId, uint32_t& player1Id, uint32_t& player2Id) {
  if (data.size() < 13) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[o]) |
                 (static_cast<uint32_t>(data[o+1])<<8) |
                 (static_cast<uint32_t>(data[o+2])<<16) |
                 (static_cast<uint32_t>(data[o+3])<<24);
    o += 4; return v;
  };
  tradeSessionId = read32(off);
  player1Id = read32(off);
  player2Id = read32(off);
  return true;
}

// Decodificar TradeStarted (servidor -> cliente)
inline bool decodeTradeStarted(const std::vector<uint8_t>& data, uint32_t& tradeSessionId, uint32_t& player1Id, uint32_t& player2Id) {
  if (data.size() < 13) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[o]) |
                 (static_cast<uint32_t>(data[o+1])<<8) |
                 (static_cast<uint32_t>(data[o+2])<<16) |
                 (static_cast<uint32_t>(data[o+3])<<24);
    o += 4; return v;
  };
  tradeSessionId = read32(off);
  player1Id = read32(off);
  player2Id = read32(off);
  return true;
}

inline std::vector<uint8_t> encodeFriendRequestReceived(uint32_t fromPlayerId, uint32_t toPlayerId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::FriendRequestReceived));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  return out;
}

inline std::vector<uint8_t> encodeWhisperReceived(uint32_t fromPlayerId, uint32_t toPlayerId, const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4 + 2 + message.size());
  
  out.push_back(static_cast<uint8_t>(MovementMsgType::WhisperReceived));
  
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto write16 = [&out](uint16_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  
  write32(fromPlayerId);
  write32(toPlayerId);
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  
  return out;
}

} // namespace Zone
} // namespace Umbra


