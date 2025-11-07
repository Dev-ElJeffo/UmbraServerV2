# 🎯 **PROBLEMA CRÍTICO: Servidor Truncando Frames de 34 Bytes para 25 Bytes**

## 📋 **PROBLEMA IDENTIFICADO:**

**EVIDÊNCIA DOS LOGS:**
```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] [SendMoveUpdate] Frame size: 34 bytes
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Received binary message, size:25expected=25
```

**CAUSA RAIZ:**
- O cliente (`SendMoveUpdate`) está gerando frames de **34 bytes** corretamente ✅
- Mas o servidor (`MovementServer`) está **truncando** os frames para **25 bytes** ao reenviar ❌

---

## 🔍 **ANÁLISE DO CÓDIGO DO SERVIDOR:**

### **1. `MovementProtocol.hpp` - Estrutura Antiga (25 bytes):**

```cpp
struct MovementFrame {
  MovementMsgType type;      // 1 byte
  uint32_t playerId;         // 4 bytes
  float x;                   // 4 bytes
  float y;                   // 4 bytes
  float z;                   // 4 bytes
  float yaw;                 // 4 bytes
  uint32_t tsMs;             // 4 bytes
  // FALTANDO: Speed, VelocityZ, IsInAir ← PROBLEMA!
};
// Total: 1 + 4 + 16 + 4 = 25 bytes
```

### **2. `decode()` - Espera Apenas 25 Bytes:**

```cpp
inline bool decode(const std::vector<uint8_t>& data, MovementFrame& f) {
  if (data.size() < 1 + 4 + 4*4 + 4) return false;  // ← Espera apenas 25 bytes
  // ... decode apenas campos de 25 bytes ...
  return true;
}
```

**PROBLEMA:** Se o cliente enviar 34 bytes, o `decode()` pode:
- Falhar (se `data.size() < 25`)
- Ou ler apenas os primeiros 25 bytes (ignorando Speed, VelocityZ, IsInAir)

### **3. `encode()` - Gera Apenas 25 Bytes:**

```cpp
inline std::vector<uint8_t> encode(const MovementFrame& f) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4*4 + 4);  // ← Reserva apenas 25 bytes
  // ... encode apenas campos de 25 bytes ...
  return out;  // ← Retorna apenas 25 bytes
}
```

**PROBLEMA:** Quando o servidor reenvia, sempre gera apenas 25 bytes porque `MovementFrame` não tem campos para animação.

---

## ✅ **SOLUÇÃO: Atualizar Protocolo do Servidor**

### **ETAPA 1: Atualizar `MovementProtocol.hpp`**

**Adicionar campos de animação à estrutura `MovementFrame`:**

```cpp
struct MovementFrame {
  MovementMsgType type;      // 1 byte
  uint32_t playerId;         // 4 bytes
  float x;                   // 4 bytes
  float y;                   // 4 bytes
  float z;                   // 4 bytes
  float yaw;                 // 4 bytes
  uint32_t tsMs;             // 4 bytes
  
  // NOVOS CAMPOS DE ANIMAÇÃO (opcionais):
  bool hasAnimation;         // Indica se campos de animação estão presentes
  float speed;               // 4 bytes - Velocidade horizontal
  float velocityZ;            // 4 bytes - Velocidade vertical
  bool isInAir;              // 1 byte - Estado no ar
  
  // Total sem animação: 25 bytes
  // Total com animação: 25 + 9 = 34 bytes
};
```

**OU criar estruturas separadas para compatibilidade:**

```cpp
// Frame antigo (25 bytes) - compatibilidade retroativa
struct MovementFrame {
  MovementMsgType type;
  uint32_t playerId;
  float x, y, z;
  float yaw;
  uint32_t tsMs;
};

// Frame novo (34 bytes) - com animação
struct MovementFrameWithAnimation {
  MovementMsgType type;      // 1 byte
  uint32_t playerId;         // 4 bytes
  float x, y, z;             // 12 bytes
  float yaw;                 // 4 bytes
  float speed;               // 4 bytes - NOVO
  float velocityZ;            // 4 bytes - NOVO
  bool isInAir;              // 1 byte - NOVO
  uint32_t tsMs;             // 4 bytes
  // Total: 34 bytes
};
```

---

### **ETAPA 2: Atualizar `decode()` para Detectar Tamanho Automaticamente**

**Modificar `decode()` para suportar ambos os formatos:**

```cpp
// Decode com detecção automática de tamanho
inline bool decode(const std::vector<uint8_t>& data, MovementFrame& f) {
  const size_t minSize = 1 + 4 + 4*4 + 4;  // 25 bytes mínimo
  if (data.size() < minSize) return false;
  
  size_t off = 0;
  f.type = static_cast<MovementMsgType>(data[off++]);
  
  auto read32 = [&data](size_t& off)->uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  auto readF = [&read32](size_t& off)->float {
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
  f.tsMs = read32(off);
  
  // Se há mais bytes (34 bytes total), é um frame com animação
  // Mas não vamos ler aqui porque MovementFrame não tem campos para animação
  // Isso será tratado em decodeWithAnimation()
  
  return true;
}

// Nova função para decodificar frame com animação
inline bool decodeWithAnimation(const std::vector<uint8_t>& data, 
                                MovementFrame& f,
                                float& speed,
                                float& velocityZ,
                                bool& isInAir) {
  const size_t frameSizeWithAnimation = 1 + 4 + 4*4 + 4 + 4 + 4 + 1 + 4;  // 34 bytes
  if (data.size() < frameSizeWithAnimation) {
    // Tentar decodificar como frame antigo (25 bytes)
    return decode(data, f);
  }
  
  size_t off = 0;
  f.type = static_cast<MovementMsgType>(data[off++]);
  
  auto read32 = [&data](size_t& off)->uint32_t {
    uint32_t v = static_cast<uint32_t>(data[off]) |
                 (static_cast<uint32_t>(data[off+1])<<8) |
                 (static_cast<uint32_t>(data[off+2])<<16) |
                 (static_cast<uint32_t>(data[off+3])<<24);
    off += 4; return v;
  };
  
  auto readF = [&read32](size_t& off)->float {
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
  speed = readF(off);        // NOVO: Velocidade horizontal
  velocityZ = readF(off);    // NOVO: Velocidade vertical
  isInAir = (data[off++] != 0);  // NOVO: Estado no ar
  f.tsMs = read32(off);
  
  return true;
}
```

---

### **ETAPA 3: Atualizar `encode()` para Suportar Animação**

**Modificar `encode()` para aceitar campos opcionais de animação:**

```cpp
// Encode básico (25 bytes) - compatibilidade retroativa
inline std::vector<uint8_t> encode(const MovementFrame& f) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4*4 + 4);
  out.push_back(static_cast<uint8_t>(f.type));
  
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto writeF = [&out, &write32](float fv) {
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
  write32(f.tsMs);
  
  return out;
}

// Nova função para codificar com animação (34 bytes)
inline std::vector<uint8_t> encodeWithAnimation(const MovementFrame& f,
                                                float speed,
                                                float velocityZ,
                                                bool isInAir) {
  std::vector<uint8_t> out;
  out.reserve(1 + 4 + 4*4 + 4 + 4 + 4 + 1 + 4);  // 34 bytes
  out.push_back(static_cast<uint8_t>(f.type));
  
  auto write32 = [&out](uint32_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  };
  
  auto writeF = [&out, &write32](float fv) {
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
  writeF(velocityZ);   // NOVO: Velocidade vertical
  out.push_back(isInAir ? 1u : 0u);  // NOVO: Estado no ar
  write32(f.tsMs);
  
  return out;
}
```

---

### **ETAPA 4: Atualizar `MovementServer.hpp`**

**Modificar `handleMoveUpdate` para ler e reenviar campos de animação:**

```cpp
void handleMoveUpdate(uint32_t cid, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(mu_);
  
  MovementFrame f{};
  float speed = 0.0f;
  float velocityZ = 0.0f;
  bool isInAir = false;
  
  // Tentar decodificar como frame com animação primeiro (34 bytes)
  bool hasAnimation = decodeWithAnimation(data, f, speed, velocityZ, isInAir);
  
  if (f.type != MovementMsgType::MoveUpdate) {
    Umbra::Core::Logger::getInstance().debug("Received non-MoveUpdate frame from client {} (type: {})", 
                                             cid, static_cast<int>(f.type));
    return;
  }
  
  // ... lógica de validação existente ...
  
  // Atualizar PlayerStateNet (pode precisar adicionar campos de animação também)
  players_[f.playerId] = PlayerStateNet{f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
  
  // Broadcast: usar encodeWithAnimation se o frame original tinha animação
  if (hasAnimation) {
    auto broadcastBytes = encodeWithAnimation(
      MovementFrame{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp},
      speed, velocityZ, isInAir
    );
    ws_.broadcastBinary(broadcastBytes);
  } else {
    // Fallback para frame antigo (25 bytes)
    MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
    auto broadcastBytes = encode(out);
    ws_.broadcastBinary(broadcastBytes);
  }
}
```

---

## 📊 **RESUMO DA CORREÇÃO:**

### **Arquivos a Modificar:**

1. **`src/zone/MovementProtocol.hpp`:**
   - Adicionar função `decodeWithAnimation()` para ler frames de 34 bytes
   - Adicionar função `encodeWithAnimation()` para gerar frames de 34 bytes
   - Manter `decode()` e `encode()` originais para compatibilidade retroativa

2. **`src/zone/MovementServer.hpp`:**
   - Modificar `setBinaryCallback` para usar `decodeWithAnimation()` primeiro
   - Modificar `handleMoveUpdate` para reenviar com `encodeWithAnimation()` se o frame original tinha animação
   - Manter compatibilidade retroativa para frames de 25 bytes

---

## 🎯 **RESULTADO ESPERADO:**

Após as correções:

**✅ CORRETO:**
```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] [SendMoveUpdate] Frame size: 34 bytes
LogTemp: Warning: [MovementServer] Received MoveUpdate with animation from client X
LogTemp: Warning: [MovementServer] Broadcasting StateUpdate with animation (34 bytes)
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Received binary message, size:34expected=34
```

---

**Fim da Solução**

