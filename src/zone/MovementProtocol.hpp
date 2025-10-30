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
// Campos comuns:
// [player_id:uint32][x:float][y:float][z:float][yaw:float][ts_ms:uint32]

enum class MovementMsgType : uint8_t {
  MoveUpdate = 1,
  StateUpdate = 2
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
  out.reserve(1 + 4 + 4*4 + 4);
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

inline bool decode(const std::vector<uint8_t>& data, MovementFrame& f) {
  if (data.size() < 1 + 4 + 4*4 + 4) return false;
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
  return true;
}

} // namespace Zone
} // namespace Umbra


