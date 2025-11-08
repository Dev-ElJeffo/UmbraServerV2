# ⚡ **GUIA RÁPIDO: Logs para Diagnosticar Remote Actors**

## 🎯 **LOGS ESSENCIAIS (5 MINUTOS):**

### **1. Log no Início do ProcessNextFrame**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**APÓS `ParseStateUpdateFrameWithAnimation` (ou `ParseStateUpdateFrame`):**
```
Format Text: "🔵 [ProcessNextFrame] PlayerID: {0}, MyPlayerId: {1}"
  - {0}: OutPlayerId
  - {1}: MyPlayerId (ou Get Active Player ID)
Print String
```

---

### **2. Log no Filtro**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**APÓS `Not Equal (Integer)`:**
```
Format Text: "🔵 [ProcessNextFrame] OutPlayerId != MyPlayerId? {0}"
  - {0}: ReturnValue do Not Equal (usar To String (Boolean))
Print String
```

---

### **3. Log no Array_Find**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**APÓS `Array_Find`:**
```
Format Text: "🔵 [ProcessNextFrame] Array_Find - FoundIndex: {0}, bFound: {1}"
  - {0}: FoundIndex
  - {1}: bFound (usar To String (Boolean))
Print String
```

---

### **4. Log no Branch (Spawn vs Update)**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**APÓS o Branch que decide entre Spawn e Update:**
- **Path `False` (Spawn - novo actor):**
  ```
  Print String: "🆕 [ProcessNextFrame] NOVO ACTOR - Spawnando (PlayerID: {OutPlayerId})"
  ```
- **Path `True` (Update - actor existe):**
  ```
  Print String: "✅ [ProcessNextFrame] ACTOR EXISTE - Atualizando (PlayerID: {OutPlayerId})"
  ```

---

### **5. Log no Spawn**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**APÓS `SpawnActorFromClass`:**
```
Format Text: "✅ [ProcessNextFrame] SpawnActorFromClass EXECUTADO - PlayerID: {0}"
  - {0}: OutPlayerId
Print String
```

---

## 🧪 **TESTE RÁPIDO:**

1. **Adicionar os 5 logs acima**
2. **Compilar**
3. **Conectar 2 clients**
4. **Mover ambos**
5. **VERIFICAR LOGS:**
   - Deve aparecer: "🔵 [ProcessNextFrame] PlayerID: {ID}, MyPlayerId: {ID}"
   - Deve aparecer: "🔵 [ProcessNextFrame] OutPlayerId != MyPlayerId? true" (para remote)
   - Deve aparecer: "🔵 [ProcessNextFrame] Array_Find - FoundIndex: ..., bFound: ..."
   - Deve aparecer: "🆕 [ProcessNextFrame] NOVO ACTOR - Spawnando" (para novos)
   - Deve aparecer: "✅ [ProcessNextFrame] SpawnActorFromClass EXECUTADO"

---

## 🔧 **DIAGNÓSTICO RÁPIDO:**

### **SE NÃO APARECE "ProcessNextFrame":**
- `ProcessNextFrame` não está sendo chamado
- Verificar se `OnWSBinaryMessage` está chamando

### **SE APARECE "OutPlayerId != MyPlayerId? false":**
- Filtro está bloqueando todos (incluindo remotes)
- Verificar se `MyPlayerId` está correto

### **SE APARECE "Array_Find - bFound: true" MAS NÃO APARECE "NOVO ACTOR":**
- Sistema pensa que actor já existe
- Verificar validação de PlayerID

### **SE APARECE "NOVO ACTOR" MAS NÃO APARECE "SpawnActorFromClass EXECUTADO":**
- Spawn não está sendo executado
- Verificar se há erros ou se o fluxo está quebrado

---

**Status:** ⚡ **ADICIONAR LOGS ESSENCIAIS (5 MINUTOS)**

