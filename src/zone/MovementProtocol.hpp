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
  PlayerInfoUpdate = 4
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

} // namespace Zone
} // namespace Umbra


