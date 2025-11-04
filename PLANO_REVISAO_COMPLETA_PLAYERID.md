# 🔍 Plano de Revisão Completa: Sistema de PlayerID

**Data**: 03/11/2025  
**Objetivo**: Garantir que o PlayerID seja transmitido e processado corretamente em todo o sistema  
**Restrições**: 
- ✅ WebSocket binário (sem TCP)
- ✅ Sistema atual (sem Dedicated Server)
- ✅ Deve funcionar conforme designado

---

## 📋 Estratégia de Debug e Correção

### **Fase 1: Teste da Correção Atual**

Testar a correção implementada:
1. Recompilar projeto Unreal Engine
2. Testar com 1 cliente → Verificar se PlayerID permanece correto
3. Testar com 2 clientes → Verificar se ambos veem um ao outro com PlayerIDs corretos
4. **Se funcionar**: ✅ Problema resolvido!
5. **Se não funcionar**: Ir para Fase 2

---

## 🔍 Fase 2: Revisão Completa do Fluxo de PlayerID

### **2.1. Mapeamento do Fluxo Completo**

Rastrear PlayerID em **TODOS** os pontos:

```
[1] Database MySQL
    └─ player_id (INTEGER) → API PHP
    
[2] API PHP (select_character.php)
    └─ Retorna JSON com player_id
    
[3] Unreal Client - UmbraGameInstance
    └─ Parse JSON → Armazena em CurrentCharacter.PlayerID
    
[4] Unreal Client - Blueprint BP_NetMovementClient
    └─ Lê MyPlayerId de UmbraGameInstance
    └─ Usa em SendMoveUpdate
    
[5] BuildMoveUpdateFrame (C++ WSBinaryBPFL)
    └─ Recebe PlayerId (int32)
    └─ Serializa como uint32 little-endian
    
[6] WebSocket Client → Server
    └─ Envia frame binário (29 bytes)
    
[7] Zone Server - MovementServer
    └─ Recebe via WebSocket
    └─ Decode frame → MovementFrame.playerId (uint32)
    
[8] Zone Server - Broadcast
    └─ Re-serializa MovementFrame
    └─ Envia via broadcastBinary
    
[9] WebSocket Server → Client
    └─ Fragmenta/Concatena frames
    
[10] Unreal Client - OnWSBinaryMessage
    └─ Recebe fragmentos
    
[11] ProcessBinaryBuffer (C++ WSBinaryBPFL)
    └─ Alinha frames → Extrai frame completo
    
[12] ParseStateUpdateFrame (C++ WSBinaryBPFL)
    └─ Deserializa → PlayerID (int32)
    
[13] Blueprint ProcessNextFrame
    └─ Usa OutPlayerId
    └─ Spawn/Update remote player
```

---

### **2.2. Pontos de Verificação**

#### **A. Cliente - Origem do PlayerID**

**Arquivos:**
- `UmbraEternumUE/Source/UmbraEternumUE/GameInstance/UmbraGameInstance.h/cpp`
- Blueprint: `BP_NetMovementClient` → Variável `MyPlayerId`

**Verificações:**
- [ ] `UmbraGameInstance::SelectCharacter` armazena `player_id` corretamente?
- [ ] `UmbraGameInstance::GetActivePlayerID()` retorna o valor correto?
- [ ] Blueprint `BP_NetMovementClient` lê `MyPlayerId` corretamente no `BeginPlay`?
- [ ] Variável `MyPlayerId` não está sendo sobrescrita após `BeginPlay`?

**Logs a Adicionar:**
```cpp
// Em UmbraGameInstance::SelectCharacter
UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] SelectCharacter - PlayerID from JSON: %d"), player_id);

// Em Blueprint BeginPlay (via Print String)
"MyPlayerId após SelectCharacter: [valor]"
```

---

#### **B. Cliente - Serialização (Envio)**

**Arquivos:**
- `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`
- Função: `BuildMoveUpdateFrame`

**Verificações:**
- [ ] `PlayerId` recebido como parâmetro está correto?
- [ ] Serialização `AppendUInt32LE` está correta?
- [ ] Frame tem exatamente 29 bytes?
- [ ] Bytes do PlayerID estão em little-endian correto?

**Logs a Adicionar:**
```cpp
// Em BuildMoveUpdateFrame
UE_LOG(LogTemp, Warning, TEXT("[BuildMoveUpdateFrame] PlayerID recebido: %d"), PlayerId);
UE_LOG(LogTemp, Warning, TEXT("[BuildMoveUpdateFrame] Frame size: %d bytes"), OutBytes.Num());
// Log dos primeiros 10 bytes do frame
UE_LOG(LogTemp, Warning, TEXT("[BuildMoveUpdateFrame] Primeiros bytes: %02X %02X %02X %02X %02X"), 
    OutBytes[0], OutBytes[1], OutBytes[2], OutBytes[3], OutBytes[4]);
```

**Teste Manual:**
- Criar frame com `PlayerID=1` e `PlayerID=999` → Verificar bytes serializados

---

#### **C. Servidor - Recepção e Decodificação**

**Arquivos:**
- `src/zone/MovementServer.hpp`
- `src/zone/MovementProtocol.hpp`
- Função: `decode`

**Verificações:**
- [ ] `decode` recebe dados corretos?
- [ ] `f.playerId` após decode está correto?
- [ ] Logs do servidor mostram PlayerID correto?

**Logs a Adicionar:**
```cpp
// Em MovementServer::handleMoveUpdate (já existe, verificar se está correto)
Umbra::Core::Logger::getInstance().debug("Received MoveUpdate from client {}: player_id={}, pos=({}, {}, {}), yaw={}", 
    cid, f.playerId, f.x, f.y, f.z, f.yaw);
```

**Teste:**
- Servidor deve logar `player_id=1` quando cliente envia `MyPlayerId=1`

---

#### **D. Servidor - Broadcast**

**Arquivos:**
- `src/zone/MovementServer.hpp`
- Função: `broadcastBinary` / `encode`

**Verificações:**
- [ ] `encode` serializa corretamente?
- [ ] Frame enviado tem exatamente 29 bytes?
- [ ] Bytes do PlayerID estão corretos antes do broadcast?

**Logs a Adicionar:**
```cpp
// Em sendFullSnapshotToAllUnlocked ou handleMoveUpdate
auto bytes = encode(f);
Umbra::Core::Logger::getInstance().debug("Broadcasting StateUpdate - PlayerID: {}, Frame size: {} bytes", 
    f.playerId, bytes.size());
if (bytes.size() >= 5) {
    Umbra::Core::Logger::getInstance().debug("Frame bytes [0-4]: %02X %02X %02X %02X %02X", 
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
}
```

---

#### **E. Cliente - Recepção e Buffer**

**Arquivos:**
- `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`
- Função: `ProcessBinaryBuffer`

**Verificações:**
- [ ] Buffer acumula dados corretamente?
- [ ] Frame extraído do buffer tem 29 bytes?
- [ ] `OutFrame.Data[0]` é `2` (StateUpdate)?
- [ ] Bytes do PlayerID no buffer estão corretos ANTES do parse?

**Logs a Adicionar:**
```cpp
// Em ProcessBinaryBuffer, ANTES de extrair frame
if (Buffer.Num() >= FrameSize && Buffer[0] == StateUpdateType) {
    UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Buffer size: %d, Frame[0-4]: %02X %02X %02X %02X %02X"), 
        Buffer.Num(), Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
}
```

---

#### **F. Cliente - Deserialização**

**Arquivos:**
- `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`
- Função: `ParseStateUpdateFrame`

**Verificações:**
- [ ] `ReadU32LE` lê corretamente?
- [ ] `OutPlayerId` após parse está correto?
- [ ] Bytes são lidos em little-endian correto?

**Logs a Adicionar:**
```cpp
// Em ParseStateUpdateFrame, ANTES de retornar
UE_LOG(LogTemp, Warning, TEXT("[ParseStateUpdateFrame] Input bytes [0-4]: %02X %02X %02X %02X %02X"), 
    Data[0], Data[1], Data[2], Data[3], Data[4]);
UE_LOG(LogTemp, Warning, TEXT("[ParseStateUpdateFrame] Parsed PlayerID: %d"), OutPlayerId);
```

---

#### **G. Blueprint - Uso do PlayerID**

**Arquivos:**
- Blueprint: `BP_NetMovementClient` → Event `ProcessNextFrame`

**Verificações:**
- [ ] `OutPlayerId` de `ParseStateUpdateFrame` está correto?
- [ ] `OutPlayerId != MyPlayerId` está funcionando corretamente?
- [ ] PlayerID é usado corretamente em `GetOrCreatePlayerState`?

**Logs a Adicionar:**
- Adicionar `Print String` mostrando `OutPlayerId` e `MyPlayerId` lado a lado
- Verificar se `OutPlayerId` do parse está correto antes de usar

---

## 🔧 Fase 3: Correções Específicas por Ponto

### **3.1. Se Problema no Cliente (A ou G)**

**Problema**: PlayerID incorreto na origem ou uso
**Solução**: 
- Corrigir leitura de `UmbraGameInstance`
- Corrigir variável `MyPlayerId` no Blueprint
- Adicionar validação `if (MyPlayerId <= 0) { log error }`

---

### **3.2. Se Problema na Serialização (B)**

**Problema**: PlayerID serializado incorretamente
**Solução**:
- Verificar `AppendUInt32LE`
- Adicionar teste unitário de serialização
- Comparar bytes gerados com bytes esperados

---

### **3.3. Se Problema no Servidor (C ou D)**

**Problema**: Servidor recebe/envia PlayerID incorreto
**Solução**:
- Verificar `decode` e `encode`
- Adicionar validação no servidor
- Comparar PlayerID recebido com PlayerID enviado

---

### **3.4. Se Problema no Buffer (E ou F)**

**Problema**: Buffer desalinhado ou deserialização incorreta
**Solução**:
- Adicionar logs detalhados do buffer
- Verificar se bytes do PlayerID estão corretos ANTES do parse
- Melhorar alinhamento do buffer (já implementado, verificar se funciona)

---

## 📊 Fase 4: Teste e Validação

### **Teste 1: Cliente Único**
```
1. Cliente conecta com PlayerID=1
2. Envia MoveUpdate
3. Verificar logs em TODOS os pontos
4. Confirmar que PlayerID=1 em todos os logs
```

### **Teste 2: Dois Clientes**
```
1. Cliente 1 (PlayerID=1) conecta
2. Cliente 2 (PlayerID=2) conecta
3. Cliente 1 envia MoveUpdate
4. Cliente 2 deve receber StateUpdate com PlayerID=1
5. Cliente 2 envia MoveUpdate
6. Cliente 1 deve receber StateUpdate com PlayerID=2
7. Verificar logs em TODOS os pontos
```

### **Teste 3: PlayerIDs Diferentes**
```
1. Testar com PlayerID=1, 10, 100, 999, 9999
2. Verificar se todos funcionam corretamente
3. Verificar se bytes serializados estão corretos
```

---

## 🚨 Debug de Emergência

Se o problema persistir após revisão completa, usar **Debug de Emergência**:

### **1. Log Hexadecimal Completo**

Adicionar log que mostra **TODOS** os bytes do frame em hexadecimal:

```cpp
// Em ProcessBinaryBuffer, ANTES de aceitar frame
FString HexString;
for (int32 i = 0; i < FrameSize && i < Buffer.Num(); ++i) {
    HexString += FString::Printf(TEXT("%02X "), Buffer[i]);
}
UE_LOG(LogTemp, Error, TEXT("[ProcessBinaryBuffer] Frame completo (hex): %s"), *HexString);
```

### **2. Comparação Servidor vs Cliente**

Logar bytes no servidor (antes de enviar) e no cliente (ao receber):
- Se bytes são idênticos → Problema no parse
- Se bytes são diferentes → Problema na transmissão/fragmentação

### **3. Magic Number**

Adicionar magic number de 4 bytes antes do frame para facilitar alinhamento:
- Frame antigo: `[Type:1][PlayerID:4][X:4][Y:4][Z:4][Yaw:4][TS:4]` = 29 bytes
- Frame novo: `[Magic:4][Type:1][PlayerID:4][X:4][Y:4][Z:4][Yaw:4][TS:4]` = 33 bytes
- Magic: `0x55 0x4D 0x42 0x52` ("UMBR" em ASCII)

---

## 📝 Checklist de Execução

### **Quando Testar Correção Atual:**
- [ ] Recompilar projeto Unreal Engine
- [ ] Executar servidor (`zone_server.exe 0`)
- [ ] Testar com 1 cliente
- [ ] Verificar logs
- [ ] Testar com 2 clientes
- [ ] Verificar se PlayerIDs estão corretos

### **Se Não Funcionar - Iniciar Fase 2:**
- [ ] Adicionar logs no Cliente (A, B, G)
- [ ] Adicionar logs no Servidor (C, D)
- [ ] Adicionar logs no Buffer (E, F)
- [ ] Executar Teste 1 (Cliente Único)
- [ ] Analisar logs e identificar ponto de falha
- [ ] Executar Teste 2 (Dois Clientes)
- [ ] Corrigir ponto de falha identificado
- [ ] Re-testar

### **Se Ainda Não Funcionar:**
- [ ] Implementar Debug de Emergência
- [ ] Adicionar Magic Number
- [ ] Re-testar

---

## 🎯 Objetivo Final

**Garantir que:**
1. ✅ Cliente envia PlayerID correto
2. ✅ Servidor recebe PlayerID correto
3. ✅ Servidor envia PlayerID correto
4. ✅ Cliente recebe PlayerID correto
5. ✅ Cliente processa PlayerID correto

**Em TODOS os pontos do fluxo!**

---

**Documento criado em**: 03/11/2025  
**Última atualização**: 03/11/2025  
**Versão**: 1.0

