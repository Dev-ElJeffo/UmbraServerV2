# 🔍 **DIAGNÓSTICO COMPLETO: Fluxo StateUpdate**

## 🚨 **PROBLEMA:**
Mesmo após corrigir o Blueprint (remover verificação `==5` e conectar corretamente), o problema persiste:
- Quando ambos clients logam ao mesmo tempo → **FUNCIONA** ✅
- Quando um client loga depois do outro → **NÃO FUNCIONA** ❌

---

## 📋 **CHECKLIST DE VERIFICAÇÃO:**

### **1. VERIFICAR SE ProcessBinaryBuffer ESTÁ SENDO CHAMADO**

**No Blueprint `BP_NetMovementClient2`, evento `OnWSBinaryMessage`:**

Adicione logs **IMEDIATAMENTE ANTES** de chamar `ProcessBinaryBuffer`:

```
[Print String]
  InString: "[OnWSBinaryMessage] ANTES de ProcessBinaryBuffer - Data.Num()={0}"
  {0}: Array Length (Data)
```

**E IMEDIATAMENTE DEPOIS:**

```
[Print String]
  InString: "[OnWSBinaryMessage] DEPOIS de ProcessBinaryBuffer - ReturnValue={0}"
  {0}: ReturnValue (de ProcessBinaryBuffer)
```

**Verificar nos logs:**
- ✅ Se aparecer `[OnWSBinaryMessage] ANTES de ProcessBinaryBuffer` → Blueprint está chegando até ProcessBinaryBuffer
- ❌ Se NÃO aparecer → O problema está ANTES (verificação Data[0] == 4 ou conexão do Knot_81)

---

### **2. VERIFICAR SE ProcessNextFrame ESTÁ SENDO CHAMADO**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **NO INÍCIO** da função:

```
[Print String]
  InString: "[ProcessNextFrame] CHAMADO - OutFrame.Data.Num()={0}, OutFrame.Type={1}"
  {0}: Get Array Item (OutFrame.Data, 0) → Array Length
  {1}: Get Array Item (OutFrame.Data, 0) → To String
```

**Verificar nos logs:**
- ✅ Se aparecer `[ProcessNextFrame] CHAMADO` → ProcessNextFrame está sendo executado
- ❌ Se NÃO aparecer → ProcessBinaryBuffer está retornando `false` ou não está conectado corretamente

---

### **3. VERIFICAR SE ParseStateUpdateFrame ESTÁ FUNCIONANDO**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **APÓS** `ParseStateUpdateFrameWithAnimation`:

```
[Print String]
  InString: "[ProcessNextFrame] ParseStateUpdateFrame - ReturnValue={0}, OutPlayerId={1}, OutLocation=({2}, {3}, {4})"
  {0}: ReturnValue (de ParseStateUpdateFrameWithAnimation) → To String (Boolean)
  {1}: OutPlayerId
  {2}: Break Vector (OutLocation) → X
  {3}: Break Vector (OutLocation) → Y
  {4}: Break Vector (OutLocation) → Z
```

**Verificar nos logs:**
- ✅ Se `ReturnValue=true` e `OutPlayerId > 0` → Parse está funcionando
- ❌ Se `ReturnValue=false` → Frame está corrompido ou formato incorreto

---

### **4. VERIFICAR SE O FILTRO DE PLAYER ID ESTÁ CORRETO**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **ANTES** do Branch que verifica `OutPlayerId != Active Player ID`:

```
[Print String]
  InString: "[ProcessNextFrame] FILTRO - OutPlayerId={0}, ActivePlayerID={1}, Vai processar? {2}"
  {0}: OutPlayerId
  {1}: Get Active Player ID (do GameInstance)
  {2}: Not Equal (OutPlayerId != ActivePlayerID) → To String (Boolean)
```

**Verificar nos logs:**
- ✅ Se `Vai processar?=true` para remote players → Filtro está correto
- ❌ Se `Vai processar?=false` para remote players → Filtro está bloqueando incorretamente

---

### **5. VERIFICAR SE Array_Find ESTÁ ENCONTRANDO O ACTOR**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **APÓS** `Array_Find (RemoteActorIds, OutPlayerId)`:

```
[Print String]
  InString: "[ProcessNextFrame] Array_Find - OutPlayerId={0}, FoundIndex={1}, RemoteActorIds.Num()={2}"
  {0}: OutPlayerId
  {1}: FoundIndex
  {2}: Array Length (RemoteActorIds)
```

**Verificar nos logs:**
- ✅ Se `FoundIndex >= 0` → Actor já existe (deve atualizar)
- ❌ Se `FoundIndex < 0` → Actor não existe (deve spawnar)

---

### **6. VERIFICAR SE O SPAWN ESTÁ ACONTECENDO**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **APÓS** `Spawn Actor from Class`:

```
[Print String]
  InString: "[ProcessNextFrame] SPAWN - OutPlayerId={0}, SpawnedActor válido? {1}, Actor Name={2}"
  {0}: OutPlayerId
  {1}: Is Valid (SpawnedActor) → To String (Boolean)
  {2}: Get Display Name (SpawnedActor)
```

**Verificar nos logs:**
- ✅ Se `SpawnedActor válido?=true` → Spawn está funcionando
- ❌ Se `SpawnedActor válido?=false` → Spawn está falhando

---

### **7. VERIFICAR SE RegisterRemoteActorInGameInstance ESTÁ SENDO CHAMADO**

**No Blueprint `BP_NetMovementClient2`, função `ProcessNextFrame`:**

Adicione log **ANTES** de chamar `Register Remote Actor In Game Instance`:

```
[Print String]
  InString: "[ProcessNextFrame] ANTES RegisterRemoteActorInGameInstance - OutPlayerId={0}, SpawnedActor={1}"
  {0}: OutPlayerId
  {1}: Get Display Name (SpawnedActor)
```

**E DEPOIS:**

```
[Print String]
  InString: "[ProcessNextFrame] DEPOIS RegisterRemoteActorInGameInstance - Chamado com sucesso"
```

**Verificar nos logs:**
- ✅ Se aparecer `[ProcessNextFrame] DEPOIS RegisterRemoteActorInGameInstance` → Registro está sendo chamado
- ❌ Se NÃO aparecer → Registro não está sendo chamado (verificar conexão no Blueprint)

---

### **8. VERIFICAR SE O SERVIDOR ESTÁ ENVIANDO O SNAPSHOT INICIAL**

**No servidor (logs do MovementServer):**

Quando o segundo client conecta, deve aparecer:

```
📤 Sending initial PlayerInfoUpdate to client X: PlayerID=Y, name=..., title=...
Sending initial snapshot to client X: PlayerID=Y, pos=(...), yaw=...
Sent initial snapshot to client X (N PlayerInfoUpdate, M StateUpdate)
```

**Verificar:**
- ✅ Se aparecer → Servidor está enviando corretamente
- ❌ Se NÃO aparecer ou aparecer com `0 StateUpdate` → Servidor não está enviando StateUpdate (player ainda não tem posição válida)

---

## 🔧 **TESTE PASSO A PASSO:**

1. **Client 1 loga primeiro**
   - Verificar logs: Deve aparecer `PlayerInfoUpdate ENVIADO` e `StateUpdate` sendo recebido

2. **Client 2 loga depois**
   - Verificar logs do servidor: Deve aparecer `Sent initial snapshot to client X`
   - Verificar logs do Client 2:
     - `[OnWSBinaryMessage] ANTES de ProcessBinaryBuffer` → ✅
     - `[ProcessNextFrame] CHAMADO` → ✅
     - `[ProcessNextFrame] ParseStateUpdateFrame - ReturnValue=true` → ✅
     - `[ProcessNextFrame] FILTRO - Vai processar?=true` → ✅
     - `[ProcessNextFrame] Array_Find - FoundIndex=-1` → ✅ (não existe ainda)
     - `[ProcessNextFrame] SPAWN - SpawnedActor válido?=true` → ✅
     - `[ProcessNextFrame] DEPOIS RegisterRemoteActorInGameInstance` → ✅

3. **Se algum passo falhar:**
   - Identificar qual passo está falhando
   - Verificar a conexão no Blueprint para esse passo específico

---

## 🎯 **POSSÍVEIS CAUSAS:**

1. **ProcessBinaryBuffer não está sendo chamado:**
   - Verificar conexão do `else` do `K2Node_IfThenElse_41` ao `K2Node_Knot_81`
   - Verificar se `K2Node_Knot_81` está conectado ao `execute` do `ProcessBinaryBuffer`

2. **ProcessBinaryBuffer está retornando false:**
   - Frame está corrompido ou desalinhado
   - Verificar logs do C++ `ProcessBinaryBuffer` para ver por que está rejeitando

3. **ProcessNextFrame não está sendo chamado:**
   - Verificar conexão do `then` do `K2Node_IfThenElse_39` ao `ProcessNextFrame`

4. **ParseStateUpdateFrame está falhando:**
   - Frame está corrompido
   - Verificar se o frame tem 34 bytes

5. **Filtro está bloqueando:**
   - `ActivePlayerID` está incorreto
   - Verificar se `Get Active Player ID` está retornando o valor correto

6. **Spawn está falhando:**
   - `BP_RemotePlayer` não existe ou não está configurado corretamente
   - Verificar se a classe está definida no Blueprint

7. **RegisterRemoteActorInGameInstance não está sendo chamado:**
   - Verificar se está conectado após `Add Item to Array (RemoteActors)`
   - Verificar se não há nenhum Branch bloqueando

---

**Fim do Guia de Diagnóstico**

