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
  SystemBroadcast = 73,                // Servidor -> Cliente: [msgType][severity:1][durationMs:2][nameLen:2][name][msgLen:2][msg]
  MailNotify = 74,                     // Servidor -> Cliente: [msgType][mailId:4][fromLen:2][from][subjLen:2][subject]
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
  CombatEventNotify = 92,              // Servidor -> Cliente: [type][targetId:4][sourceId:4][delta:i32][reason:1][isCrit:1][isDouble:1]
  DotTickNotify = 93,                  // Servidor -> Cliente: [type][targetId:4][dotId:8][delta:i32][dotType:1]
  ConsumableEffectNotify = 94,         // Cliente -> Servidor: efeito de poção (HP/MP/buff) para broadcast AOI
  ConsumableEffectUpdate = 95,         // Servidor -> Clientes: mesmo payload que 94
  SkillCastNotify = 96,                // Cliente -> Servidor: intenção de skill
  SkillCastBroadcast = 97,             // Servidor -> Clientes: cast + paths anim/VFX/SFX
  BasicAttackNotify = 98,              // Cliente -> Servidor: ataque básico
  BasicAttackBroadcast = 99,           // Servidor -> Clientes: anim ataque básico
  NpcSpawnNotify = 100,                // Servidor -> Cliente: spawn NPC na zona
  NpcDespawnNotify = 101,              // Servidor -> Cliente: despawn NPC
  NpcStateUpdate = 102,                // Servidor -> Cliente: HP/pos NPC
  NpcCombatEvent = 103,                // Servidor -> Cliente: dano/cura em NPC (floating text)
  SkillBuffSync = 104,                 // Servidor -> Clientes: apply/remove buff de skill
  SkillCastRejected = 105,             // Servidor -> Cliente: cast rejeitado (range/mana/etc.)
  ExpGainNotify = 106,                 // Servidor -> Cliente: ganho de EXP
  LevelUpNotify = 107,                 // Servidor -> Cliente: subiu de nível
  NpcBuffSnapshotRequest = 108,        // Cliente -> Servidor: pede snapshot buffs de um NPC
  SessionAuthNotify = 109,             // Cliente -> Servidor: [msgType][tokenLen:2 LE][token:utf8]
  SessionRevokedNotify = 110,            // Servidor -> Cliente: [msgType][reason:1][msgLen:2 LE][msg:utf8]
  LootWindowOpen = 111,                // Servidor -> Cliente: janela de loot (killer)
  LootWindowClose = 112,               // Servidor -> Cliente: fecha janela
  LootTakeItem = 113,                  // Cliente -> Servidor: loot slot selecionado
  LootTakeAll = 114,                   // Cliente -> Servidor: loot all
  LootWindowUpdate = 115,              // Servidor -> Cliente: slots restantes + gold opcional
  QuestProgressNotify = 116,           // Servidor -> Cliente: progresso/ready de quest (kill)
  WsKeepalive = 250                      // Servidor -> Cliente: heartbeat (1 byte); cliente ignora
};

/** kind em slots de loot: 0=item, 1=gold */
enum class LootEntryKind : uint8_t {
  Item = 0,
  Gold = 1
};

enum class LootCloseReason : uint8_t {
  Empty = 0,
  Expired = 1,
  ClosedByClient = 2,
  Error = 3
};

struct LootSlotPayload {
  uint8_t slotIndex = 0;
  uint8_t kind = 0;           // LootEntryKind
  uint32_t itemTemplateId = 0;  // 0 se gold
  uint32_t quantity = 0;
};

struct LootWindowOpenPayload {
  uint64_t corpseId = 0;
  uint32_t npcTemplateId = 0;
  uint32_t npcInstanceId = 0;
  std::vector<LootSlotPayload> slots;  // <= 10
};

struct LootWindowClosePayload {
  uint64_t corpseId = 0;
  uint8_t reason = 0;
};

struct LootTakeItemPayload {
  uint64_t corpseId = 0;
  uint8_t slotIndex = 0;
};

struct LootTakeAllPayload {
  uint64_t corpseId = 0;
};

struct LootWindowUpdatePayload {
  uint64_t corpseId = 0;
  int64_t playerGold = -1;  // <0 = não atualizar; >=0 = novo saldo
  uint8_t inventoryChanged = 0;
  std::vector<LootSlotPayload> slots;  // slots ainda não taken
};

/** Opcode 116: progresso de quest (kill) — status 1=active, 2=ready; flags bit0=progress, bit1=became_ready */
struct QuestProgressNotifyPayload {
  uint32_t playerId = 0;
  uint32_t questId = 0;
  uint8_t status = 1;
  uint8_t flags = 0;
};

/** Motivo de revogacao de sessao WS (opcode 110) */
enum class SessionRevokeReason : uint8_t {
  Generic = 0,
  DuplicateLogin = 1,
  AuthTimeout = 2,
  InvalidToken = 3
};

/** Motivo de rejeição de skill (opcode 105) */
enum class SkillCastRejectReason : uint8_t {
  Unknown = 0,
  RangeExceeded = 1,
  NoMana = 2,
  OnCooldown = 3,
  CannotCast = 4,
  NoTarget = 5,
  SkillNotFound = 6
};

struct SkillCastRejectedPayload {
  uint32_t playerId = 0;
  uint32_t skillId = 0;
  uint8_t reason = 0;
  std::string message;
};

struct ExpGainNotifyPayload {
  uint32_t playerId = 0;
  int32_t expGained = 0;
  int64_t totalExp = 0;
  int32_t expForNext = 0;
  uint8_t progressPercent = 0;
  int32_t expInCurrentLevel = 0;
};

struct LevelUpNotifyPayload {
  uint32_t playerId = 0;
  uint32_t newLevel = 0;
  uint8_t levelsGained = 0;
  uint16_t statPointsGained = 0;
  uint16_t skillPointsAvail = 0;
};

/** reason em vitals/combate: 0=unknown, 1=DAMAGE, 2=HEAL, 3=SKILL, 4=ENV, 5=DOT */
enum class CombatReason : uint8_t {
  Unknown = 0,
  Damage = 1,
  Heal = 2,
  Skill = 3,
  Env = 4,
  Dot = 5,
  Miss = 6
};

enum class CombatTargetType : uint8_t {
  Player = 1,
  Npc = 2
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

/** action: 0 = apply/refresh, 1 = remove/expired — buffs de skill (opcode 104) */
struct SkillBuffSyncPayload {
  uint8_t action = 0;
  uint32_t targetPlayerId = 0;
  uint64_t buffId = 0;
  uint32_t skillId = 0;
  uint8_t buffType = 0;
  uint8_t stacks = 1;
  int32_t valueFlat = 0;
  int16_t valuePercent = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  std::string targetStat;
  std::string skillName;
  std::string iconPath;
  /** 0 = jogador (targetPlayerId), 1 = NPC (npc_instance_id em targetPlayerId). */
  uint8_t targetType = 0;
  /** STUN/SILENCE/ROOT/SLOW/BUFF_STAT/... — append após targetType (compatível). */
  std::string effectType;
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

// System broadcast toast (73): [msgType][severity:u8][durationMs:u16 LE][nameLen:u16][name][msgLen:u16][msg]
inline std::vector<uint8_t> encodeSystemBroadcast(uint8_t severity, uint16_t durationMs,
                                                   const std::string& fromName,
                                                   const std::string& message) {
  std::vector<uint8_t> out;
  out.reserve(1 + 1 + 2 + 2 + fromName.size() + 2 + message.size());
  out.push_back(static_cast<uint8_t>(MovementMsgType::SystemBroadcast));
  out.push_back(severity);
  out.push_back(static_cast<uint8_t>(durationMs & 0xFF));
  out.push_back(static_cast<uint8_t>((durationMs >> 8) & 0xFF));
  auto write16 = [&out](uint16_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  write16(static_cast<uint16_t>(fromName.size()));
  out.insert(out.end(), fromName.begin(), fromName.end());
  write16(static_cast<uint16_t>(message.size()));
  out.insert(out.end(), message.begin(), message.end());
  return out;
}

inline bool decodeSystemBroadcast(const std::vector<uint8_t>& data, uint8_t& severity,
                                  uint16_t& durationMs, std::string& fromName,
                                  std::string& message) {
  if (data.size() < 6) return false;
  size_t off = 1;
  severity = data[off++];
  durationMs = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
  off += 2;
  if (off + 2 > data.size()) return false;
  uint16_t nameLen = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
  off += 2;
  if (off + nameLen + 2 > data.size()) return false;
  fromName.assign(reinterpret_cast<const char*>(data.data() + off), nameLen);
  off += nameLen;
  uint16_t msgLen = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
  off += 2;
  if (off + msgLen > data.size()) return false;
  message.assign(reinterpret_cast<const char*>(data.data() + off), msgLen);
  return true;
}

// Mail notify (74): [msgType][mailId:u32 LE][fromLen:u16][from][subjLen:u16][subject]
inline std::vector<uint8_t> encodeMailNotify(uint32_t mailId, const std::string& fromName,
                                              const std::string& subject) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 2 + fromName.size() + 2 + subject.size());
  out.push_back(static_cast<uint8_t>(MovementMsgType::MailNotify));
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
  write32(mailId);
  write16(static_cast<uint16_t>(fromName.size()));
  out.insert(out.end(), fromName.begin(), fromName.end());
  write16(static_cast<uint16_t>(subject.size()));
  out.insert(out.end(), subject.begin(), subject.end());
  return out;
}

inline bool decodeMailNotify(const std::vector<uint8_t>& data, uint32_t& mailId,
                             std::string& fromName, std::string& subject) {
  if (data.size() < 7) return false;
  size_t off = 1;
  mailId = static_cast<uint32_t>(data[off]) |
           (static_cast<uint32_t>(data[off + 1]) << 8) |
           (static_cast<uint32_t>(data[off + 2]) << 16) |
           (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  if (off + 2 > data.size()) return false;
  uint16_t fromLen = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
  off += 2;
  if (off + fromLen + 2 > data.size()) return false;
  fromName.assign(reinterpret_cast<const char*>(data.data() + off), fromLen);
  off += fromLen;
  uint16_t subjLen = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
  off += 2;
  if (off + subjLen > data.size()) return false;
  subject.assign(reinterpret_cast<const char*>(data.data() + off), subjLen);
  return true;
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
  /** Opcional em ForeignVitalsNotify (88): delta de HP aplicado pelo PHP (ex.: -10). */
  int32_t deltaAppliedHealth = 0;
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
  uint8_t isDouble = 0;
};

struct DotTickPayload {
  uint32_t targetId = 0;
  uint64_t dotId = 0;
  int32_t delta = 0;
  uint8_t dotType = 0;
};

/** Efeito visual de consumível (poção HP/MP/buff) para floating text multiplayer */
struct ConsumableEffectPayload {
  uint32_t targetPlayerId = 0;
  uint32_t sourcePlayerId = 0;
  int32_t healthRestore = 0;
  int32_t manaRestore = 0;
  int32_t buffValue = 0;
  std::string buffKey;
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
  data.push_back(p.isDouble);
  return data;
}

inline bool decodeCombatEventNotify(const std::vector<uint8_t>& data, CombatEventPayload& p) {
  if (data.size() < 15) return false;
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
  p.isCrit = data[off++];
  p.isDouble = (data.size() > off) ? data[off] : 0;
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

inline std::vector<uint8_t> encodeConsumableEffectUpdate(MovementMsgType msgType, const ConsumableEffectPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(86);
  data.push_back(static_cast<uint8_t>(msgType));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  writeU32(p.targetPlayerId);
  writeU32(p.sourcePlayerId);
  writeI32(p.healthRestore);
  writeI32(p.manaRestore);
  writeI32(p.buffValue);
  appendStringField(data, p.buffKey, 64);
  return data;
}

inline bool decodeConsumableEffectPayload(const std::vector<uint8_t>& data, ConsumableEffectPayload& p) {
  if (data.size() < 1 + 4 + 4 + 4 + 4 + 4 + 2) return false;
  size_t off = 1;
  auto readU32 = [&data, &off]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off])
      | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16)
      | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.targetPlayerId = readU32();
  p.sourcePlayerId = readU32();
  p.healthRestore = static_cast<int32_t>(readU32());
  p.manaRestore = static_cast<int32_t>(readU32());
  p.buffValue = static_cast<int32_t>(readU32());
  if (!readStringField(data, off, p.buffKey, 64)) return false;
  return true;
}

inline bool decodeConsumableEffectNotify(const std::vector<uint8_t>& data, ConsumableEffectPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::ConsumableEffectNotify) return false;
  return decodeConsumableEffectPayload(data, p);
}

inline bool decodePlayerVitalsNotify(const std::vector<uint8_t>& data, PlayerVitalsPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::PlayerVitalsNotify) return false;
  return decodePlayerVitalsPayload(data, p);
}

inline bool decodeForeignVitalsNotify(const std::vector<uint8_t>& data, PlayerVitalsPayload& p) {
  if (data.empty() || static_cast<MovementMsgType>(data[0]) != MovementMsgType::ForeignVitalsNotify) return false;
  if (!decodePlayerVitalsPayload(data, p)) return false;
  p.deltaAppliedHealth = 0;
  // Layout 88: [type][playerId:4][hp:4][maxHp:4][mp:4][maxMp:4][sourceId:4][reason:1][deltaApplied:4]
  if (data.size() >= 30) {
    p.deltaAppliedHealth = static_cast<int32_t>(
      static_cast<uint32_t>(data[26])
      | (static_cast<uint32_t>(data[27]) << 8)
      | (static_cast<uint32_t>(data[28]) << 16)
      | (static_cast<uint32_t>(data[29]) << 24));
  }
  return true;
}

// ============================================================================
// Combat V2 payloads (96-103)
// ============================================================================

struct SkillCastPayload {
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint8_t targetType = 1;
  uint32_t targetId = 0;
  float targetX = 0.f;
  float targetY = 0.f;
  float targetZ = 0.f;
};

struct SkillCastBroadcastPayload {
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint32_t targetId = 0;
  uint32_t castTimeMs = 0;
  std::string castAnimPath;
  std::string vfxPath;
  std::string sfxPath;
};

struct BasicAttackPayload {
  uint32_t sourcePlayerId = 0;
  uint8_t targetType = 1;
  uint32_t targetId = 0;
};

struct BasicAttackBroadcastPayload {
  uint32_t sourcePlayerId = 0;
  uint32_t classId = 0;
  uint32_t targetId = 0;
  uint32_t hitWindowMs = 300;
  std::string castAnimPath;
  /** 1 = player (default legado), 2 = NPC. Byte opcional no fim do frame. */
  uint8_t sourceType = static_cast<uint8_t>(CombatTargetType::Player);
  /** Índice em attacks[]; 255 = default (0). Byte opcional após sourceType. */
  uint8_t animIndex = 0;
};

struct NpcHandAttachOffset {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float pitch = 0.f;
  float yaw = 0.f;
  float roll = 0.f;
  float scale = 1.f;
};

struct NpcSpawnPayload {
  uint32_t npcId = 0;
  uint32_t templateId = 0;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float yaw = 0.f;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  uint32_t level = 1;
  std::string npcName;
  std::string skeletalMeshPath;
  std::string animBlueprintPath;
  uint8_t flags = 0x01;  // bit0=attackable, bit1=vendor, bit2=quest
  float interactionRadius = 300.f;
  uint32_t vendorId = 0;
  float meshScale = 1.f;
  float collisionRadius = 42.f;
  float nameplateRadius = 2000.f;
  std::string rightHandMeshPath;
  std::string leftHandMeshPath;
  NpcHandAttachOffset rightHandOffset;
  NpcHandAttachOffset leftHandOffset;
  std::vector<std::string> attackAnimPaths;
  std::vector<std::string> hitAnimPaths;
  std::string deathAnimPath;
  std::string skillAnimPath;
  std::string idleAnimPath;
  std::string walkAnimPath;
  std::string walkFwdPath;
  std::string walkBwdPath;
  std::string walkLeftPath;
  std::string walkRightPath;
  std::string runFwdPath;
  std::string runBwdPath;
  std::string runLeftPath;
  std::string runRightPath;
  std::vector<std::string> castAnimPaths;
  std::vector<std::string> buffAnimPaths;
  uint16_t deathDurationMs = 0;
};

struct NpcDespawnPayload {
  uint32_t npcId = 0;
  uint8_t reason = 0;
};

struct NpcStatePayload {
  uint32_t npcId = 0;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float yaw = 0.f;
  uint8_t aiState = 0;  // NpcAiState; compat se ausente
};

struct NpcCombatEventPayload {
  uint32_t npcId = 0;
  uint32_t sourcePlayerId = 0;
  int32_t delta = 0;
  uint8_t reason = 1;
  uint8_t isCrit = 0;
  uint8_t isDouble = 0;
  uint8_t animIndex = 255;  // 255 = cliente escolhe (random); futuro: índice em hits[]
};

struct BasicAttackDef {
  uint32_t classId = 0;
  uint16_t powerCoef = 80;
  uint32_t cooldownMs = 800;
  uint16_t rangeMax = 250;
  std::string castAnimPath;
};

inline std::vector<uint8_t> encodeSkillCastNotify(const SkillCastPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(26);
  data.push_back(static_cast<uint8_t>(MovementMsgType::SkillCastNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeF32 = [&data](float f) {
    uint32_t u;
    std::memcpy(&u, &f, sizeof(u));
    data.push_back(static_cast<uint8_t>(u & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
  };
  writeU32(p.sourcePlayerId);
  writeU32(p.skillId);
  data.push_back(p.targetType);
  writeU32(p.targetId);
  writeF32(p.targetX);
  writeF32(p.targetY);
  writeF32(p.targetZ);
  return data;
}

inline bool decodeSkillCastNotify(const std::vector<uint8_t>& data, SkillCastPayload& p) {
  if (data.size() < 26 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SkillCastNotify) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readF32 = [&]() -> float {
    uint32_t u = readU32();
    float f;
    std::memcpy(&f, &u, sizeof(f));
    return f;
  };
  p.sourcePlayerId = readU32();
  p.skillId = readU32();
  p.targetType = data[off++];
  p.targetId = readU32();
  p.targetX = readF32();
  p.targetY = readF32();
  p.targetZ = readF32();
  return true;
}

inline std::vector<uint8_t> encodeSkillCastBroadcast(const SkillCastBroadcastPayload& p) {
  std::vector<uint8_t> data;
  data.push_back(static_cast<uint8_t>(MovementMsgType::SkillCastBroadcast));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.sourcePlayerId);
  writeU32(p.skillId);
  writeU32(p.targetId);
  writeU32(p.castTimeMs);
  appendStringField(data, p.castAnimPath, 255);
  appendStringField(data, p.vfxPath, 255);
  appendStringField(data, p.sfxPath, 255);
  return data;
}

inline bool decodeSkillCastBroadcast(const std::vector<uint8_t>& data, SkillCastBroadcastPayload& p) {
  if (data.size() < 17 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SkillCastBroadcast) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.sourcePlayerId = readU32();
  p.skillId = readU32();
  p.targetId = readU32();
  p.castTimeMs = readU32();
  if (!readStringField(data, off, p.castAnimPath, 255)) return false;
  if (!readStringField(data, off, p.vfxPath, 255)) return false;
  if (!readStringField(data, off, p.sfxPath, 255)) return false;
  return true;
}

inline std::vector<uint8_t> encodeBasicAttackNotify(const BasicAttackPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::BasicAttackNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.sourcePlayerId);
  data.push_back(p.targetType);
  writeU32(p.targetId);
  return data;
}

inline bool decodeBasicAttackNotify(const std::vector<uint8_t>& data, BasicAttackPayload& p) {
  if (data.size() < 10 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::BasicAttackNotify) return false;
  size_t off = 1;
  p.sourcePlayerId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
    | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  p.targetType = data[off++];
  p.targetId = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
    | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  return true;
}

inline std::vector<uint8_t> encodeBasicAttackBroadcast(const BasicAttackBroadcastPayload& p) {
  std::vector<uint8_t> data;
  data.push_back(static_cast<uint8_t>(MovementMsgType::BasicAttackBroadcast));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.sourcePlayerId);
  writeU32(p.classId);
  writeU32(p.targetId);
  writeU32(p.hitWindowMs);
  appendStringField(data, p.castAnimPath, 255);
  data.push_back(p.sourceType == 0 ? static_cast<uint8_t>(CombatTargetType::Player) : p.sourceType);
  data.push_back(p.animIndex);
  return data;
}

inline bool decodeBasicAttackBroadcast(const std::vector<uint8_t>& data, BasicAttackBroadcastPayload& p) {
  if (data.size() < 17 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::BasicAttackBroadcast) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.sourcePlayerId = readU32();
  p.classId = readU32();
  p.targetId = readU32();
  p.hitWindowMs = readU32();
  if (!readStringField(data, off, p.castAnimPath, 255)) return false;
  // Compat: frames antigos sem sourceType → Player.
  p.sourceType = (off < data.size())
                     ? data[off++]
                     : static_cast<uint8_t>(CombatTargetType::Player);
  if (p.sourceType == 0) p.sourceType = static_cast<uint8_t>(CombatTargetType::Player);
  p.animIndex = (off < data.size()) ? data[off] : 0;
  return true;
}

inline std::vector<uint8_t> encodeNpcSpawnNotify(const NpcSpawnPayload& p) {
  std::vector<uint8_t> data;
  data.push_back(static_cast<uint8_t>(MovementMsgType::NpcSpawnNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  auto writeF32 = [&data](float f) {
    uint32_t u;
    std::memcpy(&u, &f, sizeof(u));
    data.push_back(static_cast<uint8_t>(u & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
  };
  writeU32(p.npcId);
  writeU32(p.templateId);
  writeF32(p.x);
  writeF32(p.y);
  writeF32(p.z);
  writeF32(p.yaw);
  writeI32(p.currentHealth);
  writeI32(p.maxHealth);
  writeU32(p.level);
  appendStringField(data, p.npcName, 100);
  appendStringField(data, p.skeletalMeshPath, 255);
  appendStringField(data, p.animBlueprintPath, 255);
  data.push_back(p.flags);
  writeF32(p.interactionRadius);
  writeU32(p.vendorId);
  writeF32(p.meshScale);
  appendStringField(data, p.rightHandMeshPath, 255);
  appendStringField(data, p.leftHandMeshPath, 255);
  auto writeHand = [&](const NpcHandAttachOffset& h) {
    writeF32(h.x);
    writeF32(h.y);
    writeF32(h.z);
    writeF32(h.pitch);
    writeF32(h.yaw);
    writeF32(h.roll);
    writeF32(h.scale);
  };
  writeHand(p.rightHandOffset);
  writeHand(p.leftHandOffset);
  auto writePathList = [&](const std::vector<std::string>& paths) {
    const size_t n = paths.size() > 16 ? 16 : paths.size();
    const uint8_t count = static_cast<uint8_t>(n);
    data.push_back(count);
    for (uint8_t i = 0; i < count; ++i) {
      appendStringField(data, paths[i], 255);
    }
  };
  writePathList(p.attackAnimPaths);
  writePathList(p.hitAnimPaths);
  appendStringField(data, p.deathAnimPath, 255);
  appendStringField(data, p.skillAnimPath, 255);
  appendStringField(data, p.idleAnimPath, 255);
  appendStringField(data, p.walkAnimPath, 255);
  data.push_back(static_cast<uint8_t>(p.deathDurationMs & 0xFF));
  data.push_back(static_cast<uint8_t>((p.deathDurationMs >> 8) & 0xFF));
  appendStringField(data, p.walkFwdPath, 255);
  appendStringField(data, p.walkBwdPath, 255);
  appendStringField(data, p.walkLeftPath, 255);
  appendStringField(data, p.walkRightPath, 255);
  appendStringField(data, p.runFwdPath, 255);
  appendStringField(data, p.runBwdPath, 255);
  appendStringField(data, p.runLeftPath, 255);
  appendStringField(data, p.runRightPath, 255);
  writePathList(p.castAnimPaths);
  writePathList(p.buffAnimPaths);
  writeF32(p.collisionRadius);
  writeF32(p.nameplateRadius);
  return data;
}

inline bool decodeNpcSpawnNotify(const std::vector<uint8_t>& data, NpcSpawnPayload& p) {
  if (data.size() < 37 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::NpcSpawnNotify) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readI32 = [&]() -> int32_t { return static_cast<int32_t>(readU32()); };
  auto readF32 = [&]() -> float {
    uint32_t u = readU32();
    float f;
    std::memcpy(&f, &u, sizeof(f));
    return f;
  };
  p.npcId = readU32();
  p.templateId = readU32();
  p.x = readF32();
  p.y = readF32();
  p.z = readF32();
  p.yaw = readF32();
  p.currentHealth = readI32();
  p.maxHealth = readI32();
  p.level = readU32();
  if (!readStringField(data, off, p.npcName, 100)) return false;
  if (!readStringField(data, off, p.skeletalMeshPath, 255)) return false;
  if (!readStringField(data, off, p.animBlueprintPath, 255)) return false;
  if (off < data.size()) {
    p.flags = data[off++];
    if (off + 4 <= data.size()) {
      p.interactionRadius = readF32();
    }
    if (off + 4 <= data.size()) {
      p.vendorId = readU32();
    }
    if (off + 4 <= data.size()) {
      p.meshScale = readF32();
    }
    if (off < data.size()) {
      if (!readStringField(data, off, p.rightHandMeshPath, 255)) return false;
    }
    if (off < data.size()) {
      if (!readStringField(data, off, p.leftHandMeshPath, 255)) return false;
    }
    auto readHand = [&](NpcHandAttachOffset& h) {
      if (off + 28 > data.size()) return;
      h.x = readF32();
      h.y = readF32();
      h.z = readF32();
      h.pitch = readF32();
      h.yaw = readF32();
      h.roll = readF32();
      h.scale = readF32();
      if (h.scale <= 0.01f) h.scale = 1.f;
    };
    readHand(p.rightHandOffset);
    readHand(p.leftHandOffset);
    auto readPathList = [&](std::vector<std::string>& out) {
      out.clear();
      if (off >= data.size()) return;
      const uint8_t count = data[off++];
      out.reserve(count);
      for (uint8_t i = 0; i < count; ++i) {
        std::string path;
        if (!readStringField(data, off, path, 255)) return;
        out.push_back(std::move(path));
      }
    };
    if (off < data.size()) {
      readPathList(p.attackAnimPaths);
      readPathList(p.hitAnimPaths);
      if (off < data.size()) {
        if (!readStringField(data, off, p.deathAnimPath, 255)) return false;
      }
      if (off < data.size()) {
        if (!readStringField(data, off, p.skillAnimPath, 255)) return false;
      }
      if (off < data.size()) {
        if (!readStringField(data, off, p.idleAnimPath, 255)) return false;
      }
      if (off < data.size()) {
        if (!readStringField(data, off, p.walkAnimPath, 255)) return false;
      }
      if (off + 2 <= data.size()) {
        p.deathDurationMs = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
        off += 2;
      }
      if (off < data.size()) {
        if (!readStringField(data, off, p.walkFwdPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.walkBwdPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.walkLeftPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.walkRightPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.runFwdPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.runBwdPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.runLeftPath, 255)) return false;
        if (off < data.size() && !readStringField(data, off, p.runRightPath, 255)) return false;
        if (off < data.size()) readPathList(p.castAnimPaths);
        if (off < data.size()) readPathList(p.buffAnimPaths);
      }
    }
  }
  if (off + 4 <= data.size()) {
    p.collisionRadius = readF32();
    if (p.collisionRadius < 1.f) p.collisionRadius = 42.f;
  }
  if (off + 4 <= data.size()) {
    p.nameplateRadius = readF32();
    if (p.nameplateRadius < 1.f) p.nameplateRadius = 2000.f;
  }
  return true;
}

inline std::vector<uint8_t> encodeNpcDespawnNotify(const NpcDespawnPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(6);
  data.push_back(static_cast<uint8_t>(MovementMsgType::NpcDespawnNotify));
  data.push_back(static_cast<uint8_t>(p.npcId & 0xFF));
  data.push_back(static_cast<uint8_t>((p.npcId >> 8) & 0xFF));
  data.push_back(static_cast<uint8_t>((p.npcId >> 16) & 0xFF));
  data.push_back(static_cast<uint8_t>((p.npcId >> 24) & 0xFF));
  data.push_back(p.reason);
  return data;
}

inline bool decodeNpcDespawnNotify(const std::vector<uint8_t>& data, NpcDespawnPayload& p) {
  if (data.size() < 6 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::NpcDespawnNotify) return false;
  p.npcId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8)
    | (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  p.reason = data[5];
  return true;
}

inline std::vector<uint8_t> encodeNpcStateUpdate(const NpcStatePayload& p) {
  std::vector<uint8_t> data;
  data.reserve(29);
  data.push_back(static_cast<uint8_t>(MovementMsgType::NpcStateUpdate));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  auto writeF32 = [&data](float f) {
    uint32_t u;
    std::memcpy(&u, &f, sizeof(u));
    data.push_back(static_cast<uint8_t>(u & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
  };
  writeU32(p.npcId);
  writeI32(p.currentHealth);
  writeI32(p.maxHealth);
  writeF32(p.x);
  writeF32(p.y);
  writeF32(p.z);
  writeF32(p.yaw);
  data.push_back(p.aiState);
  return data;
}

inline bool decodeNpcStateUpdate(const std::vector<uint8_t>& data, NpcStatePayload& p) {
  if (data.size() < 29 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::NpcStateUpdate) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readI32 = [&]() -> int32_t { return static_cast<int32_t>(readU32()); };
  auto readF32 = [&]() -> float {
    uint32_t u = readU32();
    float f;
    std::memcpy(&f, &u, sizeof(f));
    return f;
  };
  p.npcId = readU32();
  p.currentHealth = readI32();
  p.maxHealth = readI32();
  p.x = readF32();
  p.y = readF32();
  p.z = readF32();
  p.yaw = readF32();
  p.aiState = (off < data.size()) ? data[off] : 0;
  return true;
}

inline std::vector<uint8_t> encodeNpcCombatEvent(const NpcCombatEventPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(16);
  data.push_back(static_cast<uint8_t>(MovementMsgType::NpcCombatEvent));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  writeU32(p.npcId);
  writeU32(p.sourcePlayerId);
  writeI32(p.delta);
  data.push_back(p.reason);
  data.push_back(p.isCrit);
  data.push_back(p.isDouble);
  data.push_back(p.animIndex);
  return data;
}

inline bool decodeNpcCombatEvent(const std::vector<uint8_t>& data, NpcCombatEventPayload& p) {
  if (data.size() < 15 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::NpcCombatEvent) return false;
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8)
      | (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.npcId = readU32();
  p.sourcePlayerId = readU32();
  p.delta = static_cast<int32_t>(readU32());
  p.reason = data[off++];
  p.isCrit = data[off++];
  p.isDouble = (data.size() > off) ? data[off++] : 0;
  p.animIndex = (data.size() > off) ? data[off] : 255;
  return true;
}

inline std::vector<uint8_t> encodeSkillBuffSync(const SkillBuffSyncPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(96);
  data.push_back(static_cast<uint8_t>(MovementMsgType::SkillBuffSync));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeU64 = [&data](uint64_t v) {
    for (int i = 0; i < 8; ++i) {
      data.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
  };
  auto writeI32 = [&writeU32](int32_t v) { writeU32(static_cast<uint32_t>(v)); };
  auto writeI16 = [&data](int16_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  data.push_back(p.action);
  writeU32(p.targetPlayerId);
  writeU64(p.buffId);
  writeU32(p.skillId);
  data.push_back(p.buffType);
  data.push_back(p.stacks);
  writeI32(p.valueFlat);
  writeI16(p.valuePercent);
  writeU64(static_cast<uint64_t>(p.expiresAtMs));
  writeU32(p.durationMs);
  appendStringField(data, p.targetStat, 32);
  appendStringField(data, p.skillName, 64);
  appendStringField(data, p.iconPath, 128);
  data.push_back(p.targetType);
  appendStringField(data, p.effectType, 16);
  return data;
}

inline bool decodeSkillBuffSync(const std::vector<uint8_t>& data, SkillBuffSyncPayload& p) {
  if (data.size() < 30 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SkillBuffSync) {
    return false;
  }
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
                 (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readU64 = [&]() -> uint64_t {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
      v |= static_cast<uint64_t>(data[off + i]) << (i * 8);
    }
    off += 8;
    return v;
  };
  auto readI16 = [&]() -> int16_t {
    int16_t v = static_cast<int16_t>(static_cast<uint16_t>(data[off]) |
                                       (static_cast<uint16_t>(data[off + 1]) << 8));
    off += 2;
    return v;
  };
  if (off >= data.size()) return false;
  p.action = data[off++];
  p.targetPlayerId = readU32();
  p.buffId = readU64();
  p.skillId = readU32();
  if (off + 1 >= data.size()) return false;
  p.buffType = data[off++];
  p.stacks = data[off++];
  p.valueFlat = static_cast<int32_t>(readU32());
  p.valuePercent = readI16();
  p.expiresAtMs = static_cast<int64_t>(readU64());
  p.durationMs = readU32();
  if (!readStringField(data, off, p.targetStat, 32)) return false;
  if (!readStringField(data, off, p.skillName, 64)) return false;
  if (!readStringField(data, off, p.iconPath, 128)) return false;
  p.targetType = (off < data.size()) ? data[off++] : 0;
  if (off < data.size()) {
    if (!readStringField(data, off, p.effectType, 16)) {
      p.effectType.clear();
    }
  }
  return true;
}

inline std::vector<uint8_t> encodeSkillCastRejected(const SkillCastRejectedPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(32 + p.message.size());
  data.push_back(static_cast<uint8_t>(MovementMsgType::SkillCastRejected));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.playerId);
  writeU32(p.skillId);
  data.push_back(p.reason);
  appendStringField(data, p.message, 128);
  return data;
}

inline bool decodeSkillCastRejected(const std::vector<uint8_t>& data, SkillCastRejectedPayload& p) {
  if (data.size() < 12 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SkillCastRejected) {
    return false;
  }
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
                 (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  p.playerId = readU32();
  p.skillId = readU32();
  if (off >= data.size()) return false;
  p.reason = data[off++];
  if (!readStringField(data, off, p.message, 128)) return false;
  return true;
}

inline std::vector<uint8_t> encodeExpGainNotify(const ExpGainNotifyPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(26);
  data.push_back(static_cast<uint8_t>(MovementMsgType::ExpGainNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeI64 = [&data](int64_t v) {
    for (int i = 0; i < 8; ++i) {
      data.push_back(static_cast<uint8_t>((static_cast<uint64_t>(v) >> (i * 8)) & 0xFF));
    }
  };
  writeU32(p.playerId);
  writeU32(static_cast<uint32_t>(p.expGained));
  writeI64(p.totalExp);
  writeU32(static_cast<uint32_t>(p.expForNext));
  data.push_back(p.progressPercent);
  writeU32(static_cast<uint32_t>(p.expInCurrentLevel));
  return data;
}

inline bool decodeExpGainNotify(const std::vector<uint8_t>& data, ExpGainNotifyPayload& p) {
  if (data.size() < 22 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::ExpGainNotify) {
    return false;
  }
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
                 (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readI32 = [&]() -> int32_t { return static_cast<int32_t>(readU32()); };
  auto readI64 = [&]() -> int64_t {
    int64_t v = 0;
    for (int i = 0; i < 8; ++i) {
      v |= static_cast<int64_t>(data[off + i]) << (i * 8);
    }
    off += 8;
    return v;
  };
  p.playerId = readU32();
  p.expGained = readI32();
  p.totalExp = readI64();
  p.expForNext = readI32();
  if (off >= data.size()) return false;
  p.progressPercent = data[off++];
  p.expInCurrentLevel = 0;
  if (data.size() >= off + 4) {
    p.expInCurrentLevel = readI32();
  }
  return true;
}

inline std::vector<uint8_t> encodeLevelUpNotify(const LevelUpNotifyPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(14);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LevelUpNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  auto writeU16 = [&data](uint16_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  };
  writeU32(p.playerId);
  writeU32(p.newLevel);
  data.push_back(p.levelsGained);
  writeU16(p.statPointsGained);
  writeU16(p.skillPointsAvail);
  return data;
}

inline bool decodeLevelUpNotify(const std::vector<uint8_t>& data, LevelUpNotifyPayload& p) {
  if (data.size() < 14 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LevelUpNotify) {
    return false;
  }
  size_t off = 1;
  auto readU32 = [&]() -> uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
                 (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
    off += 4;
    return v;
  };
  auto readU16 = [&]() -> uint16_t {
    uint16_t v = static_cast<uint16_t>(data[off]) | (static_cast<uint16_t>(data[off + 1]) << 8);
    off += 2;
    return v;
  };
  p.playerId = readU32();
  p.newLevel = readU32();
  if (off >= data.size()) return false;
  p.levelsGained = data[off++];
  p.statPointsGained = readU16();
  p.skillPointsAvail = readU16();
  return true;
}

inline std::vector<uint8_t> encodeNpcBuffSnapshotRequest(uint32_t npcInstanceId) {
  std::vector<uint8_t> data;
  data.reserve(5);
  data.push_back(static_cast<uint8_t>(MovementMsgType::NpcBuffSnapshotRequest));
  data.push_back(static_cast<uint8_t>(npcInstanceId & 0xFF));
  data.push_back(static_cast<uint8_t>((npcInstanceId >> 8) & 0xFF));
  data.push_back(static_cast<uint8_t>((npcInstanceId >> 16) & 0xFF));
  data.push_back(static_cast<uint8_t>((npcInstanceId >> 24) & 0xFF));
  return data;
}

inline bool decodeNpcBuffSnapshotRequest(const std::vector<uint8_t>& data, uint32_t& npcInstanceId) {
  if (data.size() < 5 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::NpcBuffSnapshotRequest) {
    return false;
  }
  npcInstanceId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
                  (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  return npcInstanceId > 0;
}

inline std::vector<uint8_t> encodeSessionAuthNotify(const std::string& token) {
  std::vector<uint8_t> data;
  data.reserve(3 + token.size());
  data.push_back(static_cast<uint8_t>(MovementMsgType::SessionAuthNotify));
  const size_t capped = token.size() > 4096 ? 4096 : token.size();
  const uint16_t len = static_cast<uint16_t>(capped);
  data.push_back(static_cast<uint8_t>(len & 0xFF));
  data.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
  data.insert(data.end(), token.begin(), token.begin() + static_cast<std::ptrdiff_t>(capped));
  return data;
}

inline bool decodeSessionAuthNotify(const std::vector<uint8_t>& data, std::string& outToken) {
  if (data.size() < 3 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SessionAuthNotify) return false;
  const uint16_t len = static_cast<uint16_t>(data[1]) | (static_cast<uint16_t>(data[2]) << 8);
  if (data.size() < 3u + len) return false;
  outToken.assign(reinterpret_cast<const char*>(data.data() + 3), len);
  return !outToken.empty();
}

inline std::vector<uint8_t> encodeSessionRevokedNotify(SessionRevokeReason reason, const std::string& message) {
  std::vector<uint8_t> data;
  data.reserve(4 + message.size());
  data.push_back(static_cast<uint8_t>(MovementMsgType::SessionRevokedNotify));
  data.push_back(static_cast<uint8_t>(reason));
  const size_t capped = message.size() > 512 ? 512 : message.size();
  const uint16_t len = static_cast<uint16_t>(capped);
  data.push_back(static_cast<uint8_t>(len & 0xFF));
  data.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
  data.insert(data.end(), message.begin(), message.begin() + static_cast<std::ptrdiff_t>(capped));
  return data;
}

inline bool decodeSessionRevokedNotify(const std::vector<uint8_t>& data, uint8_t& outReason, std::string& outMessage) {
  if (data.size() < 4 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::SessionRevokedNotify) return false;
  outReason = data[1];
  const uint16_t len = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);
  if (data.size() < 4u + len) return false;
  outMessage.assign(reinterpret_cast<const char*>(data.data() + 4), len);
  return true;
}

// ---- Loot window (111–115) ----
// 111: [type][corpseId:u64][npcTemplateId:u32][npcInstanceId:u32][count:u8]
//      repeating count× [slot:u8][kind:u8][itemId:u32][qty:u32]
// 112: [type][corpseId:u64][reason:u8]
// 113: [type][corpseId:u64][slot:u8]
// 114: [type][corpseId:u64]
// 115: [type][corpseId:u64][playerGold:i64][invChanged:u8][count:u8] + slots...

inline void appendLootU64(std::vector<uint8_t>& data, uint64_t v) {
  for (int i = 0; i < 8; ++i) {
    data.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
  }
}

inline void appendLootU32(std::vector<uint8_t>& data, uint32_t v) {
  data.push_back(static_cast<uint8_t>(v & 0xFF));
  data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
  data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

inline void appendLootI64(std::vector<uint8_t>& data, int64_t v) {
  appendLootU64(data, static_cast<uint64_t>(v));
}

inline uint64_t readLootU64(const std::vector<uint8_t>& data, size_t& off) {
  uint64_t v = 0;
  for (int i = 0; i < 8; ++i) {
    v |= (static_cast<uint64_t>(data[off + static_cast<size_t>(i)]) << (i * 8));
  }
  off += 8;
  return v;
}

inline uint32_t readLootU32(const std::vector<uint8_t>& data, size_t& off) {
  uint32_t v = static_cast<uint32_t>(data[off]) | (static_cast<uint32_t>(data[off + 1]) << 8) |
               (static_cast<uint32_t>(data[off + 2]) << 16) | (static_cast<uint32_t>(data[off + 3]) << 24);
  off += 4;
  return v;
}

inline void appendLootSlots(std::vector<uint8_t>& data, const std::vector<LootSlotPayload>& slots) {
  const uint8_t count = static_cast<uint8_t>(slots.size() > 10 ? 10 : slots.size());
  data.push_back(count);
  for (uint8_t i = 0; i < count; ++i) {
    const auto& s = slots[i];
    data.push_back(s.slotIndex);
    data.push_back(s.kind);
    appendLootU32(data, s.itemTemplateId);
    appendLootU32(data, s.quantity);
  }
}

inline bool readLootSlots(const std::vector<uint8_t>& data, size_t& off, std::vector<LootSlotPayload>& slots) {
  if (off >= data.size()) return false;
  const uint8_t count = data[off++];
  slots.clear();
  slots.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    if (off + 10 > data.size()) return false;
    LootSlotPayload s;
    s.slotIndex = data[off++];
    s.kind = data[off++];
    s.itemTemplateId = readLootU32(data, off);
    s.quantity = readLootU32(data, off);
    slots.push_back(s);
  }
  return true;
}

inline std::vector<uint8_t> encodeLootWindowOpen(const LootWindowOpenPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(1 + 8 + 4 + 4 + 1 + p.slots.size() * 10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LootWindowOpen));
  appendLootU64(data, p.corpseId);
  appendLootU32(data, p.npcTemplateId);
  appendLootU32(data, p.npcInstanceId);
  appendLootSlots(data, p.slots);
  return data;
}

inline bool decodeLootWindowOpen(const std::vector<uint8_t>& data, LootWindowOpenPayload& p) {
  if (data.size() < 18 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LootWindowOpen) return false;
  size_t off = 1;
  p.corpseId = readLootU64(data, off);
  p.npcTemplateId = readLootU32(data, off);
  p.npcInstanceId = readLootU32(data, off);
  return readLootSlots(data, off, p.slots);
}

inline std::vector<uint8_t> encodeLootWindowClose(const LootWindowClosePayload& p) {
  std::vector<uint8_t> data;
  data.reserve(10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LootWindowClose));
  appendLootU64(data, p.corpseId);
  data.push_back(p.reason);
  return data;
}

inline bool decodeLootWindowClose(const std::vector<uint8_t>& data, LootWindowClosePayload& p) {
  if (data.size() < 10 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LootWindowClose) return false;
  size_t off = 1;
  p.corpseId = readLootU64(data, off);
  p.reason = data[off];
  return true;
}

inline std::vector<uint8_t> encodeLootTakeItem(const LootTakeItemPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LootTakeItem));
  appendLootU64(data, p.corpseId);
  data.push_back(p.slotIndex);
  return data;
}

inline bool decodeLootTakeItem(const std::vector<uint8_t>& data, LootTakeItemPayload& p) {
  if (data.size() < 10 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LootTakeItem) return false;
  size_t off = 1;
  p.corpseId = readLootU64(data, off);
  p.slotIndex = data[off];
  return true;
}

inline std::vector<uint8_t> encodeLootTakeAll(const LootTakeAllPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(9);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LootTakeAll));
  appendLootU64(data, p.corpseId);
  return data;
}

inline bool decodeLootTakeAll(const std::vector<uint8_t>& data, LootTakeAllPayload& p) {
  if (data.size() < 9 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LootTakeAll) return false;
  size_t off = 1;
  p.corpseId = readLootU64(data, off);
  return true;
}

inline std::vector<uint8_t> encodeLootWindowUpdate(const LootWindowUpdatePayload& p) {
  std::vector<uint8_t> data;
  data.reserve(1 + 8 + 8 + 1 + 1 + p.slots.size() * 10);
  data.push_back(static_cast<uint8_t>(MovementMsgType::LootWindowUpdate));
  appendLootU64(data, p.corpseId);
  appendLootI64(data, p.playerGold);
  data.push_back(p.inventoryChanged);
  appendLootSlots(data, p.slots);
  return data;
}

inline bool decodeLootWindowUpdate(const std::vector<uint8_t>& data, LootWindowUpdatePayload& p) {
  if (data.size() < 19 || static_cast<MovementMsgType>(data[0]) != MovementMsgType::LootWindowUpdate) return false;
  size_t off = 1;
  p.corpseId = readLootU64(data, off);
  p.playerGold = static_cast<int64_t>(readLootU64(data, off));
  p.inventoryChanged = data[off++];
  return readLootSlots(data, off, p.slots);
}

inline std::vector<uint8_t> encodeQuestProgressNotify(const QuestProgressNotifyPayload& p) {
  std::vector<uint8_t> data;
  data.reserve(11);
  data.push_back(static_cast<uint8_t>(MovementMsgType::QuestProgressNotify));
  auto writeU32 = [&data](uint32_t v) {
    data.push_back(static_cast<uint8_t>(v & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  writeU32(p.playerId);
  writeU32(p.questId);
  data.push_back(p.status);
  data.push_back(p.flags);
  return data;
}

inline bool decodeQuestProgressNotify(const std::vector<uint8_t>& data, QuestProgressNotifyPayload& p) {
  if (data.size() < 11 ||
      static_cast<MovementMsgType>(data[0]) != MovementMsgType::QuestProgressNotify) {
    return false;
  }
  p.playerId = static_cast<uint32_t>(data[1]) | (static_cast<uint32_t>(data[2]) << 8) |
               (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 24);
  p.questId = static_cast<uint32_t>(data[5]) | (static_cast<uint32_t>(data[6]) << 8) |
              (static_cast<uint32_t>(data[7]) << 16) | (static_cast<uint32_t>(data[8]) << 24);
  p.status = data[9];
  p.flags = data[10];
  return true;
}

} // namespace Zone
} // namespace Umbra


