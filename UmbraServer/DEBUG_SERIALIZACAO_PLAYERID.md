# 🔍 **DEBUG: Serialização do PlayerID**

## 📋 **PROBLEMA:**

**EVIDÊNCIA:**
- ✅ Cliente mostra: "Active Player ID:4" (correto)
- ✅ `MyPlayerId` está sendo setado corretamente (4)
- ✅ `SendMoveUpdate` está usando `MyPlayerId` corretamente
- ❌ Servidor recebe: `player_id=18` (incorreto)

**CONCLUSÃO:**
O problema está na **serialização ou transmissão** do PlayerID do cliente para o servidor.

---

## ✅ **CORREÇÃO APLICADA:**

Adicionados logs de debug em `BuildMoveUpdateFrame` para verificar:

1. **PlayerID recebido como parâmetro:**
   ```cpp
   UE_LOG(LogTemp, Warning, TEXT("[BuildMoveUpdateFrame] PlayerID recebido: %d"), PlayerId);
   ```

2. **Bytes serializados do frame:**
   ```cpp
   UE_LOG(LogTemp, Warning, TEXT("[BuildMoveUpdateFrame] Frame bytes [0-4]: %02X %02X %02X %02X %02X (PlayerID=%d serializado)"), 
          OutBytes[0], OutBytes[1], OutBytes[2], OutBytes[3], OutBytes[4], PlayerId);
   ```

---

## 🔍 **ANÁLISE ESPERADA:**

### **CENÁRIO 1: PlayerID Chegando Corretamente (4)**

**LOGS ESPERADOS:**
```
LogTemp: [BuildMoveUpdateFrame] PlayerID recebido: 4
LogTemp: [BuildMoveUpdateFrame] Frame bytes [0-4]: 01 04 00 00 00 (PlayerID=4 serializado)
```

**SE ISSO ACONTECER:**
- ✅ PlayerID está chegando correto em `BuildMoveUpdateFrame`
- ✅ Serialização está correta
- ❌ Problema está na **transmissão WebSocket** ou **deserialização no servidor**

---

### **CENÁRIO 2: PlayerID Chegando Incorretamente (18)**

**LOGS ESPERADOS:**
```
LogTemp: [BuildMoveUpdateFrame] PlayerID recebido: 18
LogTemp: [BuildMoveUpdateFrame] Frame bytes [0-4]: 01 12 00 00 00 (PlayerID=18 serializado)
```

**SE ISSO ACONTECER:**
- ❌ PlayerID está chegando errado em `BuildMoveUpdateFrame`
- ❌ Problema está no **Blueprint** - `MyPlayerId` não está sendo passado corretamente
- ❌ OU `MyPlayerId` está sendo alterado após `BeginPlay`

---

## 📊 **INTERPRETAÇÃO DOS BYTES:**

**Frame binário (25 bytes):**
```
[0] = 01 (type = MoveUpdate)
[1-4] = PlayerID (uint32 little-endian)
[5-8] = X (float)
[9-12] = Y (float)
[13-16] = Z (float)
[17-20] = Yaw (float)
[21-24] = Timestamp (uint32)
```

**PlayerID = 4:**
- Bytes [1-4]: `04 00 00 00` (little-endian)
- Hex: `0x00000004` = 4 decimal

**PlayerID = 18:**
- Bytes [1-4]: `12 00 00 00` (little-endian)
- Hex: `0x00000012` = 18 decimal

---

## 🚀 **PRÓXIMOS PASSOS:**

1. **Recompilar o projeto Unreal Engine**
2. **Testar novamente:**
   - Verificar logs do cliente: `[BuildMoveUpdateFrame] PlayerID recebido: X`
   - Verificar bytes serializados: `Frame bytes [0-4]: XX XX XX XX XX`
3. **Comparar com logs do servidor:**
   - Se logs do cliente mostram PlayerID=4, mas servidor recebe 18 → problema na transmissão
   - Se logs do cliente mostram PlayerID=18 → problema no Blueprint

---

## ⚠️ **POSSÍVEIS CAUSAS:**

### **1. Problema na Transmissão WebSocket:**
- WebSocket pode estar corrompendo os dados
- Verificar se `SendBytes` está enviando corretamente

### **2. Problema no Servidor (Deserialização):**
- Servidor pode estar lendo bytes errados
- Verificar se `decode` no servidor está correto

### **3. Problema no Blueprint:**
- `MyPlayerId` pode estar sendo alterado após `BeginPlay`
- Verificar se há outras lógicas que alteram `MyPlayerId`

---

## 📝 **NOTAS:**

**O código de serialização está correto:**
```cpp
WriteU32LE(OutBytes, static_cast<uint32>(PlayerId));
```

Isso serializa corretamente em little-endian. Se o PlayerID recebido for 4, os bytes serão `04 00 00 00`.

**Com os logs, poderemos identificar exatamente onde está o problema.**

