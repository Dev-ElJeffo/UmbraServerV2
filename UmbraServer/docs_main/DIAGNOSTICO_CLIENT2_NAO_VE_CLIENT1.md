# 🔍 DIAGNÓSTICO: Client 2 não vê Client 1 (Login Sequencial)

## 🎯 PROBLEMA

Quando Client 1 loga primeiro e Client 2 loga depois:
- ✅ Client 1 **VÊ** Client 2 (nameplate aparece)
- ❌ Client 2 **NÃO VÊ** Client 1 (nameplate não aparece)

**Logs do servidor confirmam:** O servidor está enviando `StateUpdate` do PlayerID 1 para Client 2.

**Logs do C++ confirmam:** Client 2 está recebendo `StateUpdate` do PlayerID 1 no `HandleWebSocketBinaryMessage`.

**Problema:** O Blueprint não está processando/spawnando/registrando o actor do PlayerID 1 no Client 2.

---

## 🔧 LOGS ADICIONADOS

Foram adicionados logs detalhados em:

1. **`NetMovementClient.cpp::HandleWebSocketBinaryMessage`**
   - Log quando `StateUpdate` é recebido
   - Log quando passa para Blueprint
   - Verificação de `GetActivePlayerID()` antes de passar para Blueprint

2. **`NetMovementClient.cpp::RegisterRemoteActorInGameInstance`**
   - Log quando a função é chamada
   - Verificação se PlayerID está nos arrays `RemoteActorIds`
   - Log quando o registro é bem-sucedido

3. **`WSBinaryBPFL.cpp::ParseStateUpdateFrameWithAnimation`**
   - Log quando a função é chamada
   - Log de cada etapa do parse
   - Log quando o parse é bem-sucedido com todos os valores

4. **`WSBinaryBPFL.cpp::ProcessBinaryBuffer`**
   - Log quando retorna `true` (frame aceito)
   - Log quando retorna `false` (nenhum frame válido encontrado)
   - Log detalhado dos valores parseados

---

## 📋 CHECKLIST DE DIAGNÓSTICO

Após recompilar e testar, verifique os logs na seguinte ordem:

### **1. Verificar se `StateUpdate` está sendo recebido pelo C++**

**No Client 2, procure por:**
```
[NetMovementClient] 🚨🚨🚨 StateUpdate RECEBIDO: Tamanho=34 bytes, PlayerID=1, MyPlayerId=23 🚨🚨🚨
```

**Se NÃO aparecer:**
- O problema está na conexão WebSocket ou no servidor
- Verifique se o servidor está fazendo broadcast corretamente

**Se aparecer:**
- ✅ C++ está recebendo corretamente
- Continue para o próximo passo

---

### **2. Verificar se está passando para Blueprint**

**No Client 2, procure por:**
```
[NetMovementClient] 🔵 Passando StateUpdate para Blueprint: PlayerID=1, MyPlayerId=23, ActivePlayerID=23
```

**Se NÃO aparecer:**
- O problema está no `HandleWebSocketBinaryMessage`
- Verifique se há algum `return` antes de `OnWSBinaryMessage(Data)`

**Se aparecer:**
- ✅ C++ está passando para Blueprint corretamente
- Continue para o próximo passo

---

### **3. Verificar se `ProcessBinaryBuffer` está retornando `true`**

**No Client 2, procure por:**
```
[ProcessBinaryBuffer] ✅✅✅ RETORNANDO TRUE - Blueprint deve processar este frame! PlayerID=1 ✅✅✅
```

**Se NÃO aparecer:**
- O problema está no `ProcessBinaryBuffer`
- Verifique se há logs de "NENHUM FRAME VÁLIDO ENCONTRADO"
- Pode ser problema de alinhamento de buffer ou tamanho de frame

**Se aparecer:**
- ✅ `ProcessBinaryBuffer` está retornando `true`
- Continue para o próximo passo

---

### **4. Verificar se `ParseStateUpdateFrameWithAnimation` está parseando corretamente**

**No Client 2, procure por:**
```
[ParseStateUpdateFrameWithAnimation] ✅✅✅ PARSE SUCESSO: PlayerID=1, Location=(X, Y, Z), Yaw=Y, Speed=S, VelocityZ=V, IsInAir=I, Timestamp=T ✅✅✅
```

**Se NÃO aparecer:**
- O problema está no parse do frame
- Verifique se há logs de "Type != 2" ou "Falha ao ler"

**Se aparecer:**
- ✅ Parse está funcionando corretamente
- Continue para o próximo passo

---

### **5. Verificar se `RegisterRemoteActorInGameInstance` está sendo chamado**

**No Client 2, procure por:**
```
[NetMovementClient] 🔵🔵🔵 RegisterRemoteActorInGameInstance CHAMADO: PlayerID=1, Actor=BP_RemotePlayer_C_X, MyPlayerId=23 🔵🔵🔵
```

**Se NÃO aparecer:**
- ⚠️ **PROBLEMA IDENTIFICADO:** O Blueprint não está chamando `RegisterRemoteActorInGameInstance`
- Isso significa que:
  - O `ProcessNextFrame` não está sendo chamado, OU
  - A verificação `OutPlayerId != GetActivePlayerID()` está falhando, OU
  - A verificação `Data[0] == 2` está falhando, OU
  - O spawn está falhando silenciosamente

**Se aparecer:**
- ✅ `RegisterRemoteActorInGameInstance` está sendo chamado
- Verifique se há logs de sucesso:
  ```
  [NetMovementClient] ✅✅✅ Remote actor registrado com sucesso no GameInstance! PlayerID=1 ✅✅✅
  ```

---

## 🔍 POSSÍVEIS CAUSAS

### **Causa 1: `ProcessNextFrame` não está sendo chamado**

**Sintoma:** Logs mostram `ProcessBinaryBuffer` retornando `true`, mas não há logs de `ParseStateUpdateFrameWithAnimation`.

**Solução:** Verificar no Blueprint se o `Branch` após `ProcessBinaryBuffer` está conectado corretamente ao `ProcessNextFrame`.

---

### **Causa 2: Verificação `OutPlayerId != GetActivePlayerID()` está falhando**

**Sintoma:** Logs mostram `ParseStateUpdateFrameWithAnimation` com sucesso, mas não há spawn/registro.

**Solução:** Verificar no Blueprint se `GetActivePlayerID()` está retornando o valor correto. Pode ser que `GetActivePlayerID()` esteja retornando `0` ou um valor inválido no Client 2.

**Teste:** Adicione um `Print String` no Blueprint para verificar:
- `OutPlayerId` (deve ser 1)
- `GetActivePlayerID()` (deve ser 23)
- Resultado da comparação `OutPlayerId != GetActivePlayerID()` (deve ser `true`)

---

### **Causa 3: Verificação `Data[0] == 2` está falhando**

**Sintoma:** Logs mostram `ParseStateUpdateFrameWithAnimation` com sucesso, mas o Blueprint não processa.

**Solução:** Verificar no Blueprint se `Data[0]` está realmente sendo `2`. Pode ser que o `Break BinaryFrame` esteja retornando dados incorretos.

**Teste:** Adicione um `Print String` no Blueprint para verificar:
- `Data.Num()` (deve ser 34)
- `Data[0]` (deve ser 2)

---

### **Causa 4: Spawn está falhando silenciosamente**

**Sintoma:** Logs mostram todas as verificações passando, mas não há spawn.

**Solução:** Verificar no Blueprint se o `Spawn Actor from Class` está retornando um actor válido. Pode ser que o spawn esteja falhando por algum motivo (colisão, classe inválida, etc.).

**Teste:** Adicione um `Print String` após `Spawn Actor from Class` para verificar:
- `ReturnValue` (deve ser um actor válido, não `None`)

---

## ✅ PRÓXIMOS PASSOS

1. **Recompile o projeto C++**
2. **Teste com 2 clients (Client 1 primeiro, Client 2 depois)**
3. **Copie TODOS os logs do Client 2** (especialmente os que começam com `[NetMovementClient]` e `[ProcessBinaryBuffer]`)
4. **Envie os logs** para análise

Os logs mostrarão exatamente onde o fluxo está parando!

---

**FIM DO GUIA**
