# 🔍 Correção: Logs de Debug para Rastrear PlayerID

**Data**: 03/11/2025  
**Problema**: PlayerID incorreto no primeiro frame e frames subsequentes não processados

---

## 🔴 Problema Identificado

### **Sintomas:**
1. Cliente com PlayerID=2 recebe PlayerID=1 no primeiro frame ❌
2. Nenhum frame subsequente é processado (todos "Binary Buffer Process failed")
3. A validação restritiva do segundo frame está bloqueando frames válidos

---

## ✅ Correções Implementadas

### **1. Removida Validação Bloqueante do Segundo Frame**

**Antes**: Exigia que o segundo frame fosse válido, bloqueando frames quando não havia segundo frame completo.

**Agora**: 
- Se há 2 frames completos e ambos são válidos → Confirma alinhamento ✅
- Se não há segundo frame → Aceita o primeiro se passou em todas as validações ✅
- Validação do segundo frame é apenas **confirmatória**, não **bloqueante**

### **2. Adicionados Logs Detalhados no Cliente**

**Arquivo**: `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`

**Logs Adicionados**:
```cpp
// Antes de aceitar frame
UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Frame aceito - PlayerID: %d, Location: (%f, %f, %f), Yaw: %f, Timestamp: %d"), 
    FinalPlayerId, FinalLocation.X, FinalLocation.Y, FinalLocation.Z, FinalYaw, FinalTimestamp);

// Primeiros 10 bytes em hexadecimal
UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Frame bytes [0-9]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"), 
    OutFrame.Data[0], OutFrame.Data[1], ...);
```

**O que os logs vão mostrar**:
- PlayerID lido do frame
- Coordenadas lidas do frame
- Bytes brutos em hexadecimal (permite comparar com servidor)

---

### **3. Adicionados Logs Detalhados no Servidor**

**Arquivo**: `src/zone/MovementServer.hpp`

**Logs Adicionados em 3 pontos**:

#### **A. sendInitialSnapshotLocked**
```cpp
Umbra::Core::Logger::getInstance().info("Sending initial snapshot to client {}: PlayerID={}, pos=({}, {}, {}), yaw={}, frame_size={} bytes", 
    clientId, f.playerId, f.x, f.y, f.z, f.yaw, bytes.size());
Umbra::Core::Logger::getInstance().info("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
    bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
```

#### **B. sendFullSnapshotToAllUnlocked**
```cpp
Umbra::Core::Logger::getInstance().debug("Broadcasting snapshot: PlayerID={}, pos=({}, {}, {}), yaw={}, frame_size={} bytes", 
    f.playerId, f.x, f.y, f.z, f.yaw, bytes.size());
Umbra::Core::Logger::getInstance().debug("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
    bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
```

#### **C. handleMoveUpdate (broadcast)**
```cpp
Umbra::Core::Logger::getInstance().debug("Broadcasting StateUpdate: PlayerID={}, pos=({}, {}, {}), yaw={}, ts={}, frame_size={} bytes", 
    out.playerId, out.x, out.y, out.z, out.yaw, finalTimestamp, broadcastBytes.size());
Umbra::Core::Logger::getInstance().debug("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
    broadcastBytes[0], broadcastBytes[1], broadcastBytes[2], broadcastBytes[3], broadcastBytes[4]);
```

**O que os logs vão mostrar**:
- PlayerID que o servidor está enviando
- Coordenadas que o servidor está enviando
- Bytes brutos em hexadecimal (permite comparar com cliente)

---

## 🧪 Como Usar os Logs

### **Passo 1: Recompilar**

```bash
# Recompilar servidor C++
cd UmbraServer
# Build normalmente

# Recompilar projeto Unreal Engine
# No Unreal Editor: Build → Build Solution (ou Ctrl+F7)
```

### **Passo 2: Executar Teste**

1. **Iniciar servidor**:
   ```bash
   zone_server.exe 0
   ```

2. **Conectar cliente com PlayerID=2**

3. **Observar logs**:

#### **No Servidor**:
```
[INFO] Sending initial snapshot to client 1: PlayerID=2, pos=(-320, 550, 92), yaw=0, frame_size=29 bytes
[INFO]   Frame bytes [0-4]: 02 02 00 00 00
```

#### **No Cliente**:
```
[ProcessBinaryBuffer] Frame aceito - PlayerID: 2, Location: (-320.0, 550.0, 92.0), Yaw: 0.0, Timestamp: 12345
[ProcessBinaryBuffer] Frame bytes [0-9]: 02 02 00 00 00 ...
```

### **Passo 3: Comparar Logs**

#### **Se Bytes são Idênticos mas PlayerID Diferente**:
- ❌ Problema no **parse** do cliente
- Verificar `ParseStateUpdateFrame` e `ReadU32LE`

#### **Se Bytes são Diferentes**:
- ❌ Problema na **transmissão/fragmentação** WebSocket
- Verificar como o WebSocket está fragmentando mensagens

#### **Se Servidor Envia PlayerID=1 mas Cliente Espera PlayerID=2**:
- ❌ Problema no **servidor** (está enviando PlayerID errado)
- Verificar `handleMoveUpdate` e como `players_` está sendo populado

---

## 📊 Análise Esperada

### **Cenário 1: Servidor Envia PlayerID Correto**

**Servidor loga**:
```
Sending initial snapshot to client 1: PlayerID=2
Frame bytes [0-4]: 02 02 00 00 00
```

**Cliente loga**:
```
Frame aceito - PlayerID: 1  ← ERRADO!
Frame bytes [0-4]: 02 01 00 00 00  ← Byte[1] é 01, não 02!
```

**Diagnóstico**: Cliente está lendo bytes diferentes do que servidor enviou → Problema na fragmentação/recepção WebSocket

---

### **Cenário 2: Servidor Envia PlayerID Errado**

**Servidor loga**:
```
Sending initial snapshot to client 1: PlayerID=1  ← ERRADO! Deveria ser 2
Frame bytes [0-4]: 02 01 00 00 00
```

**Cliente loga**:
```
Frame aceito - PlayerID: 1
Frame bytes [0-4]: 02 01 00 00 00  ← Correto (igual ao servidor)
```

**Diagnóstico**: Servidor está enviando PlayerID errado → Problema em `handleMoveUpdate` ou `players_` map

---

### **Cenário 3: Parse Incorreto**

**Servidor loga**:
```
Sending initial snapshot to client 1: PlayerID=2
Frame bytes [0-4]: 02 02 00 00 00
```

**Cliente loga**:
```
Frame aceito - PlayerID: 56128  ← ERRADO!
Frame bytes [0-4]: 02 02 00 00 00  ← Bytes corretos, mas parse incorreto!
```

**Diagnóstico**: Bytes corretos mas parse incorreto → Problema em `ParseStateUpdateFrame` ou `ReadU32LE`

---

## 🔧 Próximos Passos

1. **Recompilar servidor e cliente**
2. **Executar teste com PlayerID=2**
3. **Coletar logs do servidor e cliente**
4. **Comparar bytes**:
   - Se bytes são idênticos → Problema no parse
   - Se bytes são diferentes → Problema na transmissão
   - Se servidor envia errado → Problema no servidor
5. **Corrigir baseado na análise**

---

## 📝 Notas Importantes

- Os logs estão em `Warning`/`info`/`debug` para serem visíveis
- Logs hexadecimais mostram os primeiros 5 bytes (Type + PlayerID completo)
- Comparar `bytes[1-4]` do servidor com `bytes[1-4]` do cliente para confirmar PlayerID

---

**Documento criado em**: 03/11/2025  
**Última atualização**: 03/11/2025  
**Versão**: 1.0

