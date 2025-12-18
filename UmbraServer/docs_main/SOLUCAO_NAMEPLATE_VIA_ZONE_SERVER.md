# ✅ **SOLUÇÃO: Sincronizar Nameplate via Zone Server (WebSocket)**

## 🎯 **PROBLEMA**

Cada cliente só vê o próprio nome. O nome precisa ser sincronizado via Zone Server (WebSocket), assim como o movimento.

---

## 📋 **ARQUITETURA ATUAL**

### **Como Funciona o Movimento:**

1. **Cliente → Servidor:** Cliente envia `MoveUpdate` (34 bytes) via WebSocket
2. **Servidor:** Recebe, valida, armazena em `PlayerStateNet`
3. **Servidor → Clientes:** Faz broadcast de `StateUpdate` (34 bytes) para todos os clientes
4. **Clientes:** Recebem `StateUpdate` e atualizam posição dos remote actors

### **O Que Precisamos Fazer:**

1. **Adicionar `characterName` e `characterTitle` ao `PlayerStateNet` no servidor C++**
2. **Criar novo tipo de mensagem:** `PlayerInfoUpdate = 4` (nome/título)
3. **Cliente envia nome/título quando conecta** (ou quando muda)
4. **Servidor armazena e faz broadcast para todos os clientes**
5. **Clientes recebem e atualizam nameplate**

---

## ✅ **PASSO 1: Modificar Servidor C++**

### **1.1. Adicionar Campos ao `PlayerStateNet`**

**Arquivo:** `src/zone/MovementServer.hpp`

```cpp
struct PlayerStateNet {
  uint32_t playerId;
  float x, y, z;
  float yaw;
  uint32_t tsMs;
  // Dados de animação
  float speed = 0.0f;
  float velocityZ = 0.0f;
  bool isInAir = false;
  // NOVO: Dados do personagem
  std::string characterName;
  std::string characterTitle;
};
```

### **1.2. Adicionar Novo Tipo de Mensagem**

**Arquivo:** `src/zone/MovementProtocol.hpp`

```cpp
enum class MovementMsgType : uint8_t {
  MoveUpdate = 1,
  StateUpdate = 2,
  PlayerDisconnected = 3,
  PlayerInfoUpdate = 4  // NOVO
};
```

### **1.3. Criar Função para Codificar PlayerInfoUpdate**

**Arquivo:** `src/zone/MovementProtocol.hpp`

```cpp
// Frame PlayerInfoUpdate: [msgType:uint8][playerId:uint32][nameLen:uint16][name:bytes][titleLen:uint16][title:bytes]
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
```

### **1.4. Criar Função para Decodificar PlayerInfoUpdate**

**Arquivo:** `src/zone/MovementProtocol.hpp`

```cpp
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
```

### **1.5. Modificar `MovementServer` para Processar PlayerInfoUpdate**

**Arquivo:** `src/zone/MovementServer.hpp` (no `setBinaryCallback`)

```cpp
ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data){
  // Verificar tipo de mensagem
  if (data.empty()) return;
  MovementMsgType msgType = static_cast<MovementMsgType>(data[0]);
  
  if (msgType == MovementMsgType::PlayerInfoUpdate) {
    // Processar PlayerInfoUpdate
    uint32_t playerId;
    std::string name, title;
    if (decodePlayerInfoUpdate(data, playerId, name, title)) {
      std::lock_guard<std::mutex> lock(mu_);
      
      // Atualizar PlayerStateNet
      if (players_.find(playerId) != players_.end()) {
        players_[playerId].characterName = name;
        players_[playerId].characterTitle = title;
      } else {
        // Criar novo PlayerStateNet se não existir
        PlayerStateNet newPlayer;
        newPlayer.playerId = playerId;
        newPlayer.characterName = name;
        newPlayer.characterTitle = title;
        players_[playerId] = newPlayer;
      }
      
      // Fazer broadcast para todos os clientes
      auto broadcastMsg = encodePlayerInfoUpdate(playerId, name, title);
      ws_.broadcastBinary(broadcastMsg);
    }
    return;
  }
  
  // Processar MoveUpdate (código existente)
  MovementFrame f{};
  float speed = 0.0f;
  float velocityZ = 0.0f;
  bool isInAir = false;
  // ... resto do código existente
});
```

### **1.6. Modificar `sendInitialSnapshotLocked` para Incluir Nome/Título**

**Arquivo:** `src/zone/MovementServer.cpp`

```cpp
void sendInitialSnapshotLocked(uint32_t clientId) {
  // ... código existente para movimento ...
  
  // Enviar PlayerInfoUpdate de todos os players existentes
  for (const auto& [pid, st] : players_) {
    if (!st.characterName.empty()) {
      auto infoMsg = encodePlayerInfoUpdate(pid, st.characterName, st.characterTitle);
      ws_.sendBinary(clientId, infoMsg);
    }
  }
}
```

---

## ✅ **PASSO 2: Modificar Cliente Unreal Engine**

### **2.1. Criar Função Blueprint para Enviar PlayerInfoUpdate**

**No `BP_NetMovementClient`:**

1. **Crie uma função:** `SendPlayerInfoUpdate`
2. **Parâmetros:**
   - `CharacterName` (String)
   - `CharacterTitle` (String)
3. **Lógica:**

```
[SendPlayerInfoUpdate]
    Inputs: CharacterName, CharacterTitle
    ↓
[Get WebSocketRef]
    ↓
[Is Valid]
    ↓
[Branch]
    True → [WSBinaryBPFL::EncodePlayerInfoUpdate]
              PlayerID: MyPlayerId
              CharacterName: CharacterName
              CharacterTitle: CharacterTitle
           → [Send Binary] (do WebSocketRef)
              Data: Array de bytes retornado
```

### **2.2. Criar Função C++ para Codificar PlayerInfoUpdate**

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.h`

```cpp
UFUNCTION(BlueprintCallable, Category = "WebSocket|Binary")
static TArray<uint8> EncodePlayerInfoUpdate(int32 PlayerID, const FString& CharacterName, const FString& CharacterTitle);
```

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`

```cpp
TArray<uint8> UWSBinaryBPFL::EncodePlayerInfoUpdate(int32 PlayerID, const FString& CharacterName, const FString& CharacterTitle)
{
    TArray<uint8> Result;
    
    // Converter FString para std::string
    std::string nameStr = TCHAR_TO_UTF8(*CharacterName);
    std::string titleStr = TCHAR_TO_UTF8(*CharacterTitle);
    
    // msgType (1 byte)
    Result.Add(static_cast<uint8>(4)); // PlayerInfoUpdate
    
    // playerId (4 bytes, little-endian)
    Result.Add(static_cast<uint8>(PlayerID & 0xFF));
    Result.Add(static_cast<uint8>((PlayerID >> 8) & 0xFF));
    Result.Add(static_cast<uint8>((PlayerID >> 16) & 0xFF));
    Result.Add(static_cast<uint8>((PlayerID >> 24) & 0xFF));
    
    // nameLen (2 bytes, little-endian)
    uint16 nameLen = static_cast<uint16>(nameStr.size());
    Result.Add(static_cast<uint8>(nameLen & 0xFF));
    Result.Add(static_cast<uint8>((nameLen >> 8) & 0xFF));
    
    // name (bytes)
    for (char c : nameStr) {
        Result.Add(static_cast<uint8>(c));
    }
    
    // titleLen (2 bytes, little-endian)
    uint16 titleLen = static_cast<uint16>(titleStr.size());
    Result.Add(static_cast<uint8>(titleLen & 0xFF));
    Result.Add(static_cast<uint8>((titleLen >> 8) & 0xFF));
    
    // title (bytes)
    for (char c : titleStr) {
        Result.Add(static_cast<uint8>(c));
    }
    
    return Result;
}
```

### **2.3. Criar Função para Decodificar PlayerInfoUpdate**

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.h`

```cpp
UFUNCTION(BlueprintCallable, Category = "WebSocket|Binary")
static bool ParsePlayerInfoUpdate(const TArray<uint8>& Data, int32& OutPlayerID, FString& OutCharacterName, FString& OutCharacterTitle);
```

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`

```cpp
bool UWSBinaryBPFL::ParsePlayerInfoUpdate(const TArray<uint8>& Data, int32& OutPlayerID, FString& OutCharacterName, FString& OutCharacterTitle)
{
    if (Data.Num() < 7) return false;  // Mínimo: msgType(1) + playerId(4) + nameLen(2)
    
    int32 Offset = 0;
    
    // Verificar msgType
    uint8 msgType = Data[Offset++];
    if (msgType != 4) return false;  // PlayerInfoUpdate
    
    // Ler playerId (4 bytes, little-endian)
    OutPlayerID = Data[Offset] | 
                  (Data[Offset+1] << 8) | 
                  (Data[Offset+2] << 16) | 
                  (Data[Offset+3] << 24);
    Offset += 4;
    
    // Ler nameLen (2 bytes, little-endian)
    uint16 nameLen = Data[Offset] | (Data[Offset+1] << 8);
    Offset += 2;
    
    if (Data.Num() < Offset + nameLen + 2) return false;
    
    // Ler name
    TArray<uint8> NameBytes;
    NameBytes.Append(Data.GetData() + Offset, nameLen);
    OutCharacterName = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(NameBytes.GetData())));
    Offset += nameLen;
    
    // Ler titleLen (2 bytes, little-endian)
    uint16 titleLen = Data[Offset] | (Data[Offset+1] << 8);
    Offset += 2;
    
    if (Data.Num() < Offset + titleLen) return false;
    
    // Ler title
    TArray<uint8> TitleBytes;
    TitleBytes.Append(Data.GetData() + Offset, titleLen);
    OutCharacterTitle = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(TitleBytes.GetData())));
    
    return true;
}
```

### **2.4. Modificar `BP_NetMovementClient` para Processar PlayerInfoUpdate**

**No `OnWSBinaryMessage` do `BP_NetMovementClient`:**

```
[OnWSBinaryMessage]
    Data: Array de bytes
    ↓
[WSBinaryBPFL::ParsePlayerInfoUpdate]
    Data: Data
    OutPlayerID: PlayerID
    OutCharacterName: CharacterName
    OutCharacterTitle: CharacterTitle
    ↓
[Branch] (ReturnValue = sucesso)
    True → [Find Remote Actor by PlayerID]
              PlayerID: PlayerID
           → [Get NameplateWidgetComponent]
           → [Get User Widget Object]
           → [Cast to WBP Player Nameplate]
           → [Update Nameplate]
                  CharacterName: CharacterName
                  TitleName: CharacterTitle
```

### **2.5. Enviar PlayerInfoUpdate Quando Conectar**

**No `BP_ThirdPersonCharacter`, no `Event BeginPlay`, APÓS obter `CurrentCharacterInfo`:**

```
[Event BeginPlay]
    ↓
... (obter CurrentCharacterInfo)
    ↓
[Break Umbra Character Info]
    ↓
[Get All Actors of Class]
    Class: BP_NetMovementClient
    ↓
[Get Array Element] (primeiro elemento)
    ↓
[Send Player Info Update]
    CharacterName: Character Name
    CharacterTitle: Title Name
```

---

## 📋 **RESUMO DA IMPLEMENTAÇÃO**

### **Servidor C++:**
1. ✅ Adicionar `characterName` e `characterTitle` ao `PlayerStateNet`
2. ✅ Adicionar `PlayerInfoUpdate = 4` ao enum
3. ✅ Criar `encodePlayerInfoUpdate` e `decodePlayerInfoUpdate`
4. ✅ Processar `PlayerInfoUpdate` no `setBinaryCallback`
5. ✅ Enviar `PlayerInfoUpdate` no `sendInitialSnapshotLocked`

### **Cliente Unreal:**
1. ✅ Criar `EncodePlayerInfoUpdate` e `ParsePlayerInfoUpdate` em C++
2. ✅ Criar função `SendPlayerInfoUpdate` no Blueprint
3. ✅ Processar `PlayerInfoUpdate` no `OnWSBinaryMessage`
4. ✅ Enviar `PlayerInfoUpdate` quando conectar (no `BeginPlay`)

---

**FIM DA SOLUÇÃO**

