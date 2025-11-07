# 🔧 **GUIA PRÁTICO: Corrigir Sobreposição de Atores**

## 🚨 **PROBLEMA CRÍTICO:**

`Array_Find` está sempre retornando `FoundIndex: 0`, causando que múltiplos players sejam mapeados para o mesmo actor.

---

## ✅ **SOLUÇÃO: Adicionar Validação de PlayerID**

### **PASSO 1: Localizar o Branch Após Array_Find**

**No Blueprint Editor:**

1. Abra `BP_NetMovementClient` → `ProcessNextFrame`
2. Localize o `Branch` após `Array_Find` (geralmente `K2Node_IfThenElse_6`)
3. Este Branch tem:
   - **Condition:** `FoundIndex >= 0?`
   - **then (True):** Conectado a `Get Array Item (RemoteActors, FoundIndex)`
   - **else (False):** Conectado a `SpawnActorFromClass`

---

### **PASSO 2: Adicionar Validação ANTES de Get Array Item**

**IMPORTANTE:** A validação deve ser adicionada **ANTES** de `Get Array Item (RemoteActors, FoundIndex)`.

**Passo a passo:**

1. **Desconecte temporariamente** a conexão do pin `then` (True) do Branch para `Get Array Item (RemoteActors, FoundIndex)`

2. **Adicione `Get Array Item` para `RemoteActorIds`:**
   - Clique direito → "Get Array Item"
   - **Array:** `Get RemoteActorIds` → `Return Value`
   - **Index:** `FoundIndex` (do `Array_Find`)
   - **Output:** `FoundPlayerId` (Integer)

3. **Adicione `Equal` (Integer):**
   - Clique direito → "Equal (Integer)"
   - **A:** `FoundPlayerId` (do `Get Array Item` acima)
   - **B:** `OutPlayerId` (do `ParseStateUpdateFrame`)
   - **Output:** `ReturnValue` (Boolean)

4. **Adicione `Branch`:**
   - Clique direito → "Branch"
   - **Condition:** `ReturnValue` (do `Equal`)
   - **then (True):** PlayerID confere ✅
   - **else (False):** PlayerID NÃO confere ❌

5. **Conecte o fluxo:**
   - **Branch original (`FoundIndex >= 0`) → `then` (True):** Conecte ao `execute` do novo `Get Array Item (RemoteActorIds)`
   - **Novo `Branch` (`FoundPlayerId == OutPlayerId`) → `then` (True):** Conecte ao `execute` do `Get Array Item (RemoteActors, FoundIndex)` existente
   - **Novo `Branch` → `else` (False):** Conecte ao mesmo fluxo do `SpawnActorFromClass` (tratar como actor não existe)

---

### **PASSO 3: Estrutura Final**

**Fluxo visual:**

```
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Branch (FoundIndex >= 0?)
  ├─ True:
  │   ↓
  │   Get Array Item (RemoteActorIds, FoundIndex) → FoundPlayerId  ← NOVO!
  │   ↓
  │   Equal (FoundPlayerId == OutPlayerId?)  ← NOVO!
  │   ↓
  │   Branch
  │   ├─ True (PlayerID confere): ✅
  │   │   ↓
  │   │   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │   │   ↓
  │   │   Set Variable: RemoteActorRef
  │   │   ↓
  │   │   [ATUALIZAR ACTOR EXISTENTE]
  │   │
  │   └─ False (PlayerID NÃO confere): ❌
  │       ↓
  │       [TRATAR COMO ACTOR NÃO EXISTE]
  │       ↓
  │       SpawnActorFromClass
  │
  └─ False:
      ↓
      SpawnActorFromClass
```

---

### **PASSO 4: Adicionar Logs para Diagnóstico**

**Adicione logs nos seguintes pontos:**

**1. ANTES de `Array_Find`:**
```
Format Text: "[ProcessNextFrame] ANTES Array_Find - PlayerID: {0}, RemoteActorIds tamanho: {1}"
- {0}: OutPlayerId
- {1}: Get Array Length (RemoteActorIds)
```

**2. APÓS `Array_Find`:**
```
Format Text: "[ProcessNextFrame] APÓS Array_Find - PlayerID: {0}, FoundIndex: {1}"
- {0}: OutPlayerId
- {1}: FoundIndex
```

**3. APÓS `Get Array Item (RemoteActorIds)`:**
```
Format Text: "[ProcessNextFrame] VALIDAÇÃO - OutPlayerId: {0}, FoundPlayerId: {1}, Match: {2}"
- {0}: OutPlayerId
- {1}: FoundPlayerId
- {2}: Equal (OutPlayerId == FoundPlayerId?) → To String (Boolean)
```

**4. No Branch de validação:**
```
Branch → then (True): Log "PlayerID confere - usando actor existente"
Branch → else (False): Log "PlayerID NÃO confere - spawnando novo actor"
```

---

## 🔍 **VERIFICAÇÃO ADICIONAL:**

### **Verificar se `Array_Add` está correto:**

**No Blueprint Editor:**

1. **Localize `Array_Add` para `RemoteActorIds`:**
   - Deve estar **APÓS** `SpawnActorFromClass`
   - **Target Array:** `Get RemoteActorIds` → `Return Value`
   - **New Item:** `OutPlayerId`
   - **NÃO** deve ter `Make Array` ou `Set Variable` após

2. **Localize `Array_Add` para `RemoteActors`:**
   - Deve estar **APÓS** `Array_Add (RemoteActorIds)`
   - **Target Array:** `Get RemoteActors` → `Return Value`
   - **New Item:** `RemoteActorRef` (do `SpawnActorFromClass`)
   - **NÃO** deve ter `Make Array` ou `Set Variable` após

---

## ✅ **RESULTADO ESPERADO:**

Após aplicar a correção:

1. **Validação de PlayerID:**
   - `Array_Find` encontra índice
   - Verifica se o PlayerID no índice encontrado confere com `OutPlayerId`
   - Se não conferir, trata como actor não existe e spawna novo

2. **Logs detalhados:**
   - Mostram `FoundIndex` e `FoundPlayerId`
   - Mostram se a validação passou ou falhou
   - Ajudam a diagnosticar problemas futuros

3. **Sincronização:**
   - `RemoteActorIds` e `RemoteActors` permanecem sincronizados
   - Cada PlayerID tem seu próprio actor

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Implementar validação** conforme descrito acima
2. **Adicionar logs** para diagnóstico
3. **Testar com múltiplos clients** (2, 3, 4+)
4. **Verificar logs** para confirmar que a validação está funcionando

---

**Esta correção deve resolver o problema de sobreposição de atores!**

