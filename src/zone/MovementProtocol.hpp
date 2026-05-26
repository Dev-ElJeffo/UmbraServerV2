#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <string>

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
  PartyAcceptNotify = 26,     // Cliente -> Servidor: Notificar aceite de grupo (após HTTP)
  PartyStatsRefresh = 27,     // Cliente -> Servidor: HP/MP mudou (equip/unequip), broadcast para refrescar
  PartyMemberLeftNotify = 28, // Cliente -> Servidor: Saiu do grupo (após HTTP), broadcast para outros
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
  DuelEnded = 54,             // Servidor -> Clientes: Duelo terminado
  PersonalShopOpenNotify = 60,   // Cliente -> Servidor: loja aberta (após HTTP)
  PersonalShopOpened = 61,       // Servidor -> Clientes: broadcast loja
  PersonalShopCloseNotify = 62,  // Cliente -> Servidor: loja fechada
  PersonalShopClosed = 63,       // Servidor -> Clientes: broadcast fim loja
  // Loja pessoal: após compra HTTP bem-sucedida, o comprador notifica o Zone; todos refrescam listagens via HTTP
  PersonalShopListingSoldNotify = 64,  // Cliente -> Servidor: [buyerId:4][sellerId:4][shopId:4][listingId:4]
  PersonalShopListingsChanged = 65,    // Servidor -> Clientes: [sellerId:4][shopId:4][listingId:4]
  ChatLocalMessage = 66,               // Cliente -> Servidor: [msgType][fromPlayerId:4][msgLen:2][msg:utf8]
  ChatGlobalMessage = 67,              // Cliente -> Servidor: [msgType][fromPlayerId:4][msgLen:2][msg:utf8]
  ChatGroupMessage = 68,               // Cliente -> Servidor: [msgType][fromPlayerId:4][msgLen:2][msg:utf8]
  ChatLocalReceived = 69,              // Servidor -> Cliente: [msgType][fromPlayerId:4][nameLen:2][name:utf8][msgLen:2][msg:utf8]
  ChatGlobalReceived = 70,             // Servidor -> Cliente: [msgType][fromPlayerId:4][nameLen:2][name:utf8][msgLen:2][msg:utf8]
  ChatGroupReceived = 71,              // Servidor -> Cliente: [msgType][fromPlayerId:4][nameLen:2][name:utf8][msgLen:2][msg:utf8]
  ChatServerError = 72,                // Servidor -> Cliente: [msgType][errorCode:2][msgLen:2][error:utf8]
  GuildInviteReceived = 80,            // Servidor -> Cliente: [msgType][inviteId:4][guildId:4][fromPlayerId:4]
  GuildStateRefresh = 81,              // Servidor -> Cliente: [msgType][guildId:4]
  GuildMemberUpdated = 82,             // Servidor -> Cliente: [msgType][guildId:4][playerId:4]
  GuildMemberKicked = 83,              // Servidor -> Cliente: [msgType][guildId:4][playerId:4]
  BuffAppliedNotify = 84,              // Cliente -> Servidor: payload RemoteBuffUpdate
  RemoteBuffUpdate = 85,               // Servidor -> Clientes: mesmo payload
  PlayerVitalsNotify = 86,             // Cliente -> Servidor: HP/MP do próprio jogador
  PlayerVitalsUpdate = 87,             // Servidor -> Clientes: HP/MP de outro jogador
  ForeignVitalsNotify = 88,            // Cliente -> Servidor: HP/MP de alvo (após apply_vitals.php)
  PlayerDeathNotify = 89,              // Servidor -> Cliente: [type][playerId:4][killerId:4][reason:1]
  RespawnRequest = 90,                 // Cliente -> Servidor: [type][playerId:4][zoneId:4][spawnKeyLen:1][spawnKey]
  PlayerRespawnedNotify = 91,          // Servidor -> Cliente: [type][playerId:4][x:f][y:f][z:f][yaw:f][hp:i32][maxHp:i32][mp:i32][maxMp:i32]
  CombatEventNotify = 92,              // Servidor -> Cliente: [type][targetId:4][sourceId:4][delta:i32][reason:1][isCrit:1]
  DotTickNotify = 93                   // Servidor -> Cliente: [type][targetId:4][dotId:8][delta:i32][dotType:1]
};

/** reason em vitals/combate: 0=unknown, 1=DAMAGE, 2=HEAL, 3=SKILL, 4=ENV, 5=DOT */
enum class CombatReason : uint8_t {
  Unknown = 0,
  Damage = 1,
  Heal = 2,
  Skill = 3,
  Env = 4,
  Dot = 5
};

/** action: 0 = applied/refresh, 1 = removed/expired */
struct RemoteBuffPayload {
  uint32_t playerId = 0;
  uint8_t action = 0;
  std::string buffKey;
  int32_t bonusValue = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  uint32_t itemTemplateId = 0;
  std::string itemName;
  std::string iconPath;
};

enum class ChatChannel : uint8_t {
  Local = 1,
  Global = 2,
  Group = 3
};

enum class ChatErrorCode : uint16_t {
  Unknown = 0,
  InvalidPayload = 1,
  EmptyMessage = 2,
  MessageTooLong = 3,
  RateLimitExceeded = 4,
  NotAuthenticated = 5
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

// Codificar mensagem PlayerInfoUpdate:
// [msgType:uint8][playerId:uint32][nameLen:uint16][name:bytes][titleLen:uint16][title:bytes][guildLen:uint16][guild:bytes]
// Compatível: clientes antigos ignoram dados extras.
inline std::vector<uint8_t> encodePlayerInfoUpdate(uint32_t playerId, 
                                                    const std::string& name, 
                                                    const std::string& title,
                                                    const std::string& guildName = "") {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 2 + name.size() + 2 + title.size() + 2 + guildName.size());
  
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
  write16(static_cast<uint16_t>(guildName.size()));
  out.insert(out.end(), guildName.begin(), guildName.end());
  
  return out;
}

// Decodificar mensagem PlayerInfoUpdate
inline bool decodePlayerInfoUpdate(const std::vector<uint8_t>& data,
                                   uint32_t& playerId,
                                   std::string& name,
                                   std::string& title,
                                   std::string& guildName) {
  guildName.clear();
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
  off += titleLen;

  // Campo opcional no final do payload (compat com clientes/servidores antigos)
  if (data.size() >= off + 2) {
    uint16_t guildLen = read16(off);
    if (data.size() < off + guildLen) return false;
    guildName.assign(reinterpret_cast<const char*>(data.data() + off), guildLen);
    off += guildLen;
  }
  return true;
}

// Compatibilidade com chamadas antigas sem guildName.
inline bool decodePlayerInfoUpdate(const std::vector<uint8_t>& data,
                                   uint32_t& playerId,
                                   std::string& name,
                                   std::string& title) {
  std::string ignoredGuild;
  return decodePlayerInfoUpdate(data, playerId, name, title, ignoredGuild);
}

inline std::vector<uint8_t> encodeGuildNotify(MovementMsgType msgType, uint32_t guildId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(msgType));
  out.push_back(static_cast<uint8_t>(guildId & 0xFF));
  out.push_back(static_cast<uint8_t>((guildId >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>((guildId >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((guildId >> 24) & 0xFF));
  return out;
}

inline std::vector<uint8_t> encodeGuildMemberNotify(MovementMsgType msgType, uint32_t guildId, uint32_t playerId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(msgType));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(guildId);
  write32(playerId);
  return out;
}

inline bool decodeGuildNotify(const std::vector<uint8_t>& data, uint32_t& guildId) {
  if (data.size() < 5) return false;
  guildId = static_cast<uint32_t>(data[1]) |
            (static_cast<uint32_t>(data[2]) << 8) |
            (static_cast<uint32_t>(data[3]) << 16) |
            (static_cast<uint32_t>(data[4]) << 24);
  return true;
}

inline std::vector<uint8_t> encodeGuildInviteReceived(uint32_t inviteId, uint32_t guildId, uint32_t fromPlayerId, uint32_t toPlayerId) {
  std::vector<uint8_t> out;
  out.reserve(17);
  out.push_back(static_cast<uint8_t>(MovementMsgType::GuildInviteReceived));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(inviteId);
  write32(guildId);
  write32(fromPlayerId);
  write32(toPlayerId);
  return out;
}

inline bool decodeGuildInviteReceived(const std::vector<uint8_t>& data, uint32_t& inviteId, uint32_t& guildId, uint32_t& fromPlayerId, uint32_t& toPlayerId) {
  if (data.size() < 17) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[o]) |
                 (static_cast<uint32_t>(data[o+1])<<8) |
                 (static_cast<uint32_t>(data[o+2])<<16) |
                 (static_cast<uint32_t>(data[o+3])<<24);
    o += 4; return v;
  };
  inviteId = read32(off);
  guildId = read32(off);
  fromPlayerId = read32(off);
  toPlayerId = read32(off);
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

// Chat (cliente -> servidor): [msgType][fromPlayerId:4][msgLen:2][message:utf8]
inline std::vector<uint8_t> encodeChatClientMessage(MovementMsgType msgType, uint32_t fromPlayerId, const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 2 + message.size());
  out.push_back(static_cast<uint8_t>(msgType));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto write16 = [&out](uint16_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  write32(fromPlayerId);
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  return out;
}

inline bool decodeChatClientMessage(const std::vector<uint8_t>& data, uint32_t& fromPlayerId, std::string& message) {
  if (data.size() < 7) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[o]) |
                 (static_cast<uint32_t>(data[o+1])<<8) |
                 (static_cast<uint32_t>(data[o+2])<<16) |
                 (static_cast<uint32_t>(data[o+3])<<24);
    o += 4;
    return v;
  };
  auto read16 = [&data](size_t& o)->uint16_t{
    uint16_t v = static_cast<uint16_t>(data[o]) |
                 (static_cast<uint16_t>(data[o+1])<<8);
    o += 2;
    return v;
  };
  fromPlayerId = read32(off);
  uint16_t msgLen = read16(off);
  if (data.size() < off + msgLen) return false;
  message.assign(reinterpret_cast<const char*>(data.data() + off), msgLen);
  return true;
}

// Chat recebido (servidor -> cliente): [msgType][fromPlayerId:4][nameLen:2][name:utf8][msgLen:2][msg:utf8]
inline std::vector<uint8_t> encodeChatReceived(MovementMsgType msgType, uint32_t fromPlayerId,
                                                const std::string& fromPlayerName, const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 2 + fromPlayerName.size() + 2 + message.size());
  out.push_back(static_cast<uint8_t>(msgType));
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
  write16(static_cast<uint16_t>(fromPlayerName.size()));
  out.insert(out.end(), fromPlayerName.begin(), fromPlayerName.end());
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  return out;
}

inline bool decodeChatReceived(const std::vector<uint8_t>& data, uint32_t& fromPlayerId,
                               std::string& fromPlayerName, std::string& message) {
  if (data.size() < 9) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o)->uint32_t{
    uint32_t v = static_cast<uint32_t>(data[o]) |
                 (static_cast<uint32_t>(data[o+1])<<8) |
                 (static_cast<uint32_t>(data[o+2])<<16) |
                 (static_cast<uint32_t>(data[o+3])<<24);
    o += 4;
    return v;
  };
  auto read16 = [&data](size_t& o)->uint16_t{
    uint16_t v = static_cast<uint16_t>(data[o]) |
                 (static_cast<uint16_t>(data[o+1])<<8);
    o += 2;
    return v;
  };
  fromPlayerId = read32(off);
  uint16_t nameLen = read16(off);
  if (data.size() < off + nameLen + 2) return false;
  fromPlayerName.assign(reinterpret_cast<const char*>(data.data() + off), nameLen);
  off += nameLen;
  uint16_t msgLen = read16(off);
  if (data.size() < off + msgLen) return false;
  message.assign(reinterpret_cast<const char*>(data.data() + off), msgLen);
  return true;
}

// Erro de chat (servidor -> cliente): [msgType][errorCode:2][msgLen:2][error:utf8]
inline std::vector<uint8_t> encodeChatServerError(ChatErrorCode errorCode, const std::string& errorMessage) {
  std::vector<uint8_t> out;
  out.reserve(1 + 2 + 2 + errorMessage.size());
  out.push_back(static_cast<uint8_t>(MovementMsgType::ChatServerError));
  uint16_t code = static_cast<uint16_t>(errorCode);
  out.push_back(static_cast<uint8_t>(code & 0xFF));
  out.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
  uint16_t msgLen = static_cast<uint16_t>(errorMessage.size());
  out.push_back(static_cast<uint8_t>(msgLen & 0xFF));
  out.push_back(static_cast<uint8_t>((msgLen >> 8) & 0xFF));
  out.insert(out.end(), errorMessage.begin(), errorMessage.end());
  return out;
}

inline bool decodeChatServerError(const std::vector<uint8_t>& data, ChatErrorCode& errorCode, std::string& errorMessage) {
  if (data.size() < 5) return false;
  uint16_t code = static_cast<uint16_t>(data[1]) | (static_cast<uint16_t>(data[2]) << 8);
  uint16_t msgLen = static_cast<uint16_t>(data[3]) | (static_cast<uint16_t>(data[4]) << 8);
  if (data.size() < 5 + msgLen) return false;
  errorCode = static_cast<ChatErrorCode>(code);
  errorMessage.assign(reinterpret_cast<const char*>(data.data() + 5), msgLen);
  return true;
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

// WhisperReceived: [msgType=41][fromId:4][toId:4][nameLen:2][name:UTF-8][msgLen:2][message:UTF-8]
inline std::vector<uint8_t> encodeWhisperReceived(uint32_t fromPlayerId, uint32_t toPlayerId,
  const std::string& fromPlayerName, const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4 + 2 + fromPlayerName.size() + 2 + message.size());
  
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
  write16(static_cast<uint16_t>(fromPlayerName.size()));
  out.insert(out.end(), fromPlayerName.begin(), fromPlayerName.end());
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  
  return out;
}

// Cliente -> Servidor: Notificar aceite de grupo (após HTTP)
inline std::vector<uint8_t> encodePartyAcceptNotify(uint32_t partyId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyAcceptNotify));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(partyId);
  return out;
}

inline bool decodePartyAcceptNotify(const std::vector<uint8_t>& data, uint32_t& partyId) {
  if (data.size() < 5) return false;
  partyId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
            (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  return true;
}

// Servidor -> Clientes: Membro entrou no grupo (broadcast para todos refrescarem)
inline std::vector<uint8_t> encodePartyMemberJoined(uint32_t partyId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyMemberJoined));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(partyId);
  return out;
}

inline bool decodePartyMemberJoined(const std::vector<uint8_t>& data, uint32_t& partyId) {
  if (data.size() < 5) return false;
  partyId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
            (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  return true;
}

// Servidor -> Clientes: Membro saiu (broadcast para refrescarem)
inline std::vector<uint8_t> encodePartyMemberLeft(uint32_t partyId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyMemberLeft));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(partyId);
  return out;
}

inline bool decodePartyMemberLeft(const std::vector<uint8_t>& data, uint32_t& partyId) {
  if (data.size() < 5) return false;
  partyId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
            (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  return true;
}

// PartyStatsRefresh e PartyMemberLeftNotify usam mesmo formato [msgType][partyId]
inline std::vector<uint8_t> encodePartyStatsRefresh(uint32_t partyId) {
  std::vector<uint8_t> out;
  out.reserve(5);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PartyStatsRefresh));
  auto write32 = [&out](uint32_t v){
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(partyId);
  return out;
}

inline bool decodePartyStatsRefresh(const std::vector<uint8_t>& data, uint32_t& partyId) {
  if (data.size() < 5) return false;
  partyId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
            (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  return true;
}

inline bool decodePartyMemberLeftNotify(const std::vector<uint8_t>& data, uint32_t& partyId) {
  return decodePartyStatsRefresh(data, partyId);  // mesmo formato
}

// Loja pessoal: [msgType][sellerId:4][shopId:4][nameLen:2 LE][name UTF-8]
inline std::vector<uint8_t> encodePersonalShopOpenPayload(MovementMsgType msgType, uint32_t sellerId,
                                                          uint32_t shopId, const std::string& shopNameUtf8) {
  std::vector<uint8_t> out;
  const size_t maxNameBytes = 256;
  size_t nb = shopNameUtf8.size();
  if (nb > maxNameBytes) nb = maxNameBytes;
  uint16_t nameLen = static_cast<uint16_t>(nb);
  out.reserve(1 + 4 + 4 + 2 + nb);
  out.push_back(static_cast<uint8_t>(msgType));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(sellerId);
  write32(shopId);
  out.push_back(static_cast<uint8_t>(nameLen & 0xFF));
  out.push_back(static_cast<uint8_t>((nameLen >> 8) & 0xFF));
  for (uint16_t i = 0; i < nameLen; ++i) {
    out.push_back(static_cast<uint8_t>(shopNameUtf8[i]));
  }
  return out;
}

inline std::vector<uint8_t> encodePersonalShopOpenNotify(uint32_t sellerId, uint32_t shopId,
                                                         const std::string& shopNameUtf8) {
  return encodePersonalShopOpenPayload(MovementMsgType::PersonalShopOpenNotify, sellerId, shopId, shopNameUtf8);
}

inline std::vector<uint8_t> encodePersonalShopOpened(uint32_t sellerId, uint32_t shopId,
                                                     const std::string& shopNameUtf8) {
  return encodePersonalShopOpenPayload(MovementMsgType::PersonalShopOpened, sellerId, shopId, shopNameUtf8);
}

inline bool decodePersonalShopOpenPayload(const std::vector<uint8_t>& data, uint32_t& sellerId, uint32_t& shopId,
                                          std::string& shopNameUtf8) {
  if (data.size() < 1 + 4 + 4 + 2) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o) -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[o]) | (static_cast<uint32_t>(data[o + 1]) << 8) |
                 (static_cast<uint32_t>(data[o + 2]) << 16) | (static_cast<uint32_t>(data[o + 3]) << 24);
    o += 4;
    return v;
  };
  sellerId = read32(off);
  shopId = read32(off);
  uint16_t nameLen = static_cast<uint16_t>(data[off]) | (static_cast<uint16_t>(data[off + 1]) << 8);
  off += 2;
  if (data.size() < off + nameLen) return false;
  shopNameUtf8.assign(reinterpret_cast<const char*>(data.data() + off), nameLen);
  return true;
}

// [msgType][sellerId:4][shopId:4]
inline std::vector<uint8_t> encodePersonalShopCloseNotify(uint32_t sellerId, uint32_t shopId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PersonalShopCloseNotify));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(sellerId);
  write32(shopId);
  return out;
}

inline std::vector<uint8_t> encodePersonalShopClosed(uint32_t sellerId, uint32_t shopId) {
  std::vector<uint8_t> out;
  out.reserve(9);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PersonalShopClosed));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(sellerId);
  write32(shopId);
  return out;
}

inline bool decodePersonalShopClosePayload(const std::vector<uint8_t>& data, uint32_t& sellerId, uint32_t& shopId) {
  if (data.size() < 9) return false;
  size_t off = 1;
  sellerId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
             (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  shopId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
           (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  return true;
}

// [64][buyerId:4][sellerId:4][shopId:4][listingId:4]
inline std::vector<uint8_t> encodePersonalShopListingSoldNotify(uint32_t buyerId, uint32_t sellerId, uint32_t shopId,
                                                                uint32_t listingId) {
  std::vector<uint8_t> out;
  out.reserve(17);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PersonalShopListingSoldNotify));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(buyerId);
  write32(sellerId);
  write32(shopId);
  write32(listingId);
  return out;
}

inline bool decodePersonalShopListingSoldNotify(const std::vector<uint8_t>& data, uint32_t& buyerId, uint32_t& sellerId,
                                                uint32_t& shopId, uint32_t& listingId) {
  if (data.size() < 17) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o) -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[o]) | (static_cast<uint32_t>(data[o + 1]) << 8) |
                 (static_cast<uint32_t>(data[o + 2]) << 16) | (static_cast<uint32_t>(data[o + 3]) << 24);
    o += 4;
    return v;
  };
  buyerId = read32(off);
  sellerId = read32(off);
  shopId = read32(off);
  listingId = read32(off);
  return true;
}

// [65][sellerId:4][shopId:4][listingId:4]
inline std::vector<uint8_t> encodePersonalShopListingsChanged(uint32_t sellerId, uint32_t shopId, uint32_t listingId) {
  std::vector<uint8_t> out;
  out.reserve(13);
  out.push_back(static_cast<uint8_t>(MovementMsgType::PersonalShopListingsChanged));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  write32(sellerId);
  write32(shopId);
  write32(listingId);
  return out;
}

inline bool decodePersonalShopListingsChanged(const std::vector<uint8_t>& data, uint32_t& sellerId, uint32_t& shopId,
                                              uint32_t& listingId) {
  if (data.size() < 13) return false;
  size_t off = 1;
  sellerId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
             (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  shopId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
           (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  listingId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
              (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  return true;
}

// Remote buff: [msgType][playerId:4][action:1][buffKeyLen:2 LE][buffKey][bonusValue:4 LE][expiresAtMs:8 LE]
// [durationMs:4 LE][itemTemplateId:4 LE][itemNameLen:2 LE][itemName][iconPathLen:2 LE][iconPath]
inline void appendStringField(std::vector<uint8_t>& out, const std::string& s, size_t maxBytes = 128) {
  size_t nb = s.size();
  if (nb > maxBytes) nb = maxBytes;
  const uint16_t len = static_cast<uint16_t>(nb);
  out.push_back(static_cast<uint8_t>(len & 0xFF));
  out.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
  for (uint16_t i = 0; i < len; ++i) {
    out.push_back(static_cast<uint8_t>(s[i]));
  }
}

inline bool readStringField(const std::vector<uint8_t>& data, size_t& off, std::string& out, size_t maxBytes = 128) {
  if (off + 2 > data.size()) return false;
  const uint16_t len = static_cast<uint16_t>(data[off]) | (static_cast<uint16_t>(data[off + 1]) << 8);
  off += 2;
  if (len > maxBytes || off + len > data.size()) return false;
  out.assign(reinterpret_cast<const char*>(&data[off]), len);
  off += len;
  return true;
}

inline std::vector<uint8_t> encodeRemoteBuffUpdate(MovementMsgType msgType, const RemoteBuffPayload& p) {
  std::vector<uint8_t> out;
  out.reserve(64);
  out.push_back(static_cast<uint8_t>(msgType));
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto write64 = [&out](int64_t v) {
    for (int i = 0; i < 8; ++i) {
      out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
  };
  write32(p.playerId);
  out.push_back(p.action);
  appendStringField(out, p.buffKey, 64);
  write32(static_cast<uint32_t>(p.bonusValue));
  write64(p.expiresAtMs);
  write32(p.durationMs);
  write32(p.itemTemplateId);
  appendStringField(out, p.itemName, 64);
  appendStringField(out, p.iconPath, 128);
  return out;
}

inline bool decodeRemoteBuffUpdate(const std::vector<uint8_t>& data, RemoteBuffPayload& p) {
  if (data.size() < 1 + 4 + 1 + 2 + 4 + 8 + 4 + 4 + 2 + 2) return false;
  size_t off = 1;
  auto read32 = [&data](size_t& o) -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[o]) | (static_cast<uint32_t>(data[o + 1]) << 8) |
                 (static_cast<uint32_t>(data[o + 2]) << 16) | (static_cast<uint32_t>(data[o + 3]) << 24);
    o += 4;
    return v;
  };
  auto read64 = [&data](size_t& o) -> int64_t {
    int64_t v = 0;
    for (int i = 0; i < 8; ++i) {
      v |= static_cast<int64_t>(data[o + i]) << (i * 8);
    }
    o += 8;
    return v;
  };
  p.playerId = read32(off);
  if (off >= data.size()) return false;
  p.action = data[off++];
  if (!readStringField(data, off, p.buffKey, 64)) return false;
  p.bonusValue = static_cast<int32_t>(read32(off));
  p.expiresAtMs = read64(off);
  p.durationMs = read32(off);
  p.itemTemplateId = read32(off);
  if (!readStringField(data, off, p.itemName, 64)) return false;
  if (!readStringField(data, off, p.iconPath, 128)) return false;
  return true;
}

inline bool decodeBuffAppliedNotify(const std::vector<uint8_t>& data, RemoteBuffPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::BuffAppliedNotify) return false;
  return decodeRemoteBuffUpdate(data, p);
}

/** HP/MP em tempo real para party/AOI */
struct PlayerVitalsPayload {
  uint32_t playerId = 0;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  int32_t currentMana = 0;
  int32_t maxMana = 0;
  uint32_t sourcePlayerId = 0;
  uint8_t reason = 0;
};

struct PlayerDeathPayload {
  uint32_t playerId = 0;
  uint32_t killerId = 0;
  uint8_t reason = 0;
};

struct PlayerRespawnPayload {
  uint32_t playerId = 0;
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float yaw = 0.0f;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  int32_t currentMana = 0;
  int32_t maxMana = 0;
};

struct CombatEventPayload {
  uint32_t targetId = 0;
  uint32_t sourceId = 0;
  int32_t delta = 0;
  uint8_t reason = 0;
  uint8_t isCrit = 0;
};

struct DotTickPayload {
  uint32_t targetId = 0;
  uint64_t dotId = 0;
  int32_t delta = 0;
  uint8_t dotType = 0;
};

inline std::vector<uint8_t> encodePlayerVitalsUpdate(MovementMsgType msgType, const PlayerVitalsPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(26);
  data.push_back(static_cast<uint8_t>(msgType));
  auto writeI32 = [&data](int32_t v) {
    const uint32_t u = static_cast<uint32_t>(v);
    data.push_back(static_cast<uint8_t>(u & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
  };
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.playerId);
  writeI32(p.currentHealth);
  writeI32(p.maxHealth);
  writeI32(p.currentMana);
  writeI32(p.maxMana);
  writeU32(p.sourcePlayerId);
  data.push_back(p.reason);
  return data;
}

inline bool decodePlayerVitalsPayload(const std::vector<uint8_t>& data, PlayerVitalsPayload& p) {
  if (data.size() < 21) return false;
  p.sourcePlayerId = 0;
  p.reason = 0;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.playerId = readU32();
  p.currentHealth = static_cast<int32_t>(readU32());
  p.maxHealth = static_cast<int32_t>(readU32());
  p.currentMana = static_cast<int32_t>(readU32());
  p.maxMana = static_cast<int32_t>(readU32());
  if (data.size() >= off + 5) {
    p.sourcePlayerId = readU32();
    p.reason = data[off];
    ++off;
  }
  return true;
}

inline std::vector<uint8_t> encodePlayerDeathNotify(const PlayerDeathPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::PlayerDeathNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.playerId);
  writeU32(p.killerId);
  data.push_back(p.reason);
  return data;
}

inline bool decodePlayerDeathNotify(const std::vector<uint8_t>& data, PlayerDeathPayload& p) {
  if (data.size() < 10) return false;
  if (static_cast<MovementMsgType>(data[0]) != MovementMsgType::PlayerDeathNotify) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.playerId = readU32();
  p.killerId = readU32();
  p.reason = data[off];
  return true;
}

inline std::vector<uint8_t> encodeRespawnRequest(uint32_t playerId, uint32_t zoneId, const std::string& spawnKey) {
  std::vector<uint8_t> data;
  data.reserve(10 + spawnKey.size());
  data.push_back(static_cast<uint8_t>(MovementMsgType::RespawnRequest));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(playerId);
  writeU32(zoneId);
  const uint8_t keyLen = static_cast<uint8_t>(std::min<size_t>(spawnKey.size(), 64));
  data.push_back(keyLen);
  data.insert(data.end(), spawnKey.begin(), spawnKey.begin() + keyLen);
  return data;
}

inline bool decodeRespawnRequest(const std::vector<uint8_t>& data, uint32_t& playerId, uint32_t& zoneId, std::string& spawnKey) {
  if (data.size() < 10) return false;
  if (static_cast<MovementMsgType>(data[0]) != MovementMsgType::RespawnRequest) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  playerId = readU32();
  zoneId = readU32();
  const uint8_t keyLen = data[off++];
  if (data.size() < off + keyLen) return false;
  spawnKey.assign(reinterpret_cast<const char*>(data.data() + off), keyLen);
  return true;
}

inline std::vector<uint8_t> encodePlayerRespawnedNotify(const PlayerRespawnPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(38);
  data.push_back(static_cast<uint8_t>(MovementMsgType::PlayerRespawnedNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  auto writeF32 = [&writeU32](float v) {
    uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(float));
    writeU32(bits);
  };
  writeU32(p.playerId);
  writeF32(p.x);
  writeF32(p.y);
  writeF32(p.z);
  writeF32(p.yaw);
  writeI32(p.currentHealth);
  writeI32(p.maxHealth);
  writeI32(p.currentMana);
  writeI32(p.maxMana);
  return data;
}

inline bool decodePlayerRespawnedNotify(const std::vector<uint8_t>& data, PlayerRespawnPayload& p) {
  if (data.size() < 38) return false;
  if (static_cast<MovementMsgType>(data[0]) != MovementMsgType::PlayerRespawnedNotify) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readF32 = [&readU32]() -> float {
    uint32_t bits = readU32();
    float v = 0.0f;
    std::memcpy(&v, &bits, sizeof(float));
    return v;
  };
  p.playerId = readU32();
  p.x = readF32();
  p.y = readF32();
  p.z = readF32();
  p.yaw = readF32();
  p.currentHealth = static_cast<int32_t>(readU32());
  p.maxHealth = static_cast<int32_t>(readU32());
  p.currentMana = static_cast<int32_t>(readU32());
  p.maxMana = static_cast<int32_t>(readU32());
  return true;
}

inline std::vector<uint8_t> encodeCombatEventNotify(const CombatEventPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(16);
  data.push_back(static_cast<uint8_t>(MovementMsgType::CombatEventNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  writeU32(p.targetId);
  writeU32(p.sourceId);
  writeI32(p.delta);
  data.push_back(p.reason);
  data.push_back(p.isCrit);
  return data;
}

inline bool decodeCombatEventNotify(const std::vector<uint8_t>& data, CombatEventPayload& p) {
  if (data.size() < 16) return false;
  if (static_cast<MovementMsgType>(data[0]) != MovementMsgType::CombatEventNotify) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.targetId = readU32();
  p.sourceId = readU32();
  p.delta = static_cast<int32_t>(readU32());
  p.reason = data[off++];
  p.isCrit = data[off];
  return true;
}

inline std::vector<uint8_t> encodeDotTickNotify(const DotTickPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(19);
  data.push_back(static_cast<uint8_t>(MovementMsgType::DotTickNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeU64 = [&data](uint64_t v) {
    for (int i = 0; i < 8; ++i) {
      data.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
    }
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  writeU32(p.targetId);
  writeU64(p.dotId);
  writeI32(p.delta);
  data.push_back(p.dotType);
  return data;
}

inline bool decodeDotTickNotify(const std::vector<uint8_t>& data, DotTickPayload& p) {
  if (data.size() < 19) return false;
  if (static_cast<MovementMsgType>(data[0]) != MovementMsgType::DotTickNotify) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.targetId = readU32();
  uint64_t dotId = 0;
  for (int i = 0; i < 8; ++i) {
    dotId |= static_cast<uint64_t>(data[off + i]) << (8 * i);
  }
  off += 8;
  p.dotId = dotId;
  p.delta = static_cast<int32_t>(readU32());
  p.dotType = data[off];
  return true;
}

inline bool decodePlayerVitalsNotify(const std::vector<uint8_t>& data, PlayerVitalsPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::PlayerVitalsNotify) return false;
  return decodePlayerVitalsPayload(data, p);
}

inline bool decodeForeignVitalsNotify(const std::vector<uint8_t>& data, PlayerVitalsPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::ForeignVitalsNotify) return false;
  return decodePlayerVitalsPayload(data, p);
}

} // namespace Zone
} // namespace Umbra


