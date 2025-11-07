# 🚨 **PROBLEMA CRÍTICO: Sobreposição de Atores e Controle Bugado**

## 📋 **PROBLEMAS IDENTIFICADOS:**

### **Problema 1: Sobreposição quando múltiplos clients spawnam sem movimento**
- Quando mais de um client spawna um personagem sem que o anterior tenha se movido, os dois actors se sobrepõem
- O controle buga

### **Problema 2: Spawn incorreto com múltiplos clients**
- **2 clients:** Funciona corretamente ✅
- **3+ clients:** O personagem que já estava logado é spawnado junto ao actor do terceiro client
- O problema se repete para cada novo client

---

## 🔍 **ANÁLISE DOS LOGS:**

### **Padrão Observado:**

```
[ProcessNextFrame] Actor já existe - FoundIndex: 0, atualizando
```

**PROBLEMA CRÍTICO:** `FoundIndex: 0` está aparecendo **sempre**, mesmo para diferentes PlayerIDs!

**Isso indica:**
- `Array_Find` está sempre retornando `0` (primeiro elemento)
- OU `Array_Find` está sendo executado antes dos arrays serem atualizados
- OU os arrays não estão sincronizados corretamente

---

## 🚨 **CAUSA RAIZ PROVÁVEL:**

### **Hipótese 1: Race Condition - Múltiplos Frames Simultâneos**

**Cenário:**
1. Client 1 spawna → `Array_Add (RemoteActorIds, PlayerID_1)` → `Array_Add (RemoteActors, Actor1)`
2. Client 2 spawna → `Array_Add (RemoteActorIds, PlayerID_2)` → `Array_Add (RemoteActors, Actor2)`
3. **ANTES** que os `Array_Add` sejam completados, chegam múltiplos frames:
   - Frame de PlayerID_1 → `Array_Find` → encontra índice 0 ✅
   - Frame de PlayerID_2 → `Array_Find` → **AINDA encontra índice 0** ❌ (deveria encontrar índice 1!)
   - Frame de PlayerID_3 → `Array_Find` → **AINDA encontra índice 0** ❌

**Resultado:**
- Todos os frames usam `RemoteActors[0]` (primeiro actor)
- Múltiplos players são atualizados no mesmo actor
- Actors se sobrepõem

### **Hipótese 2: `Array_Find` Usando Array Errado ou Desatualizado**

**Cenário:**
- `Array_Find` pode estar usando um `Get RemoteActorIds` que retorna uma **cópia** do array em vez da referência
- OU `Array_Find` está sendo executado antes de `Array_Add` completar

### **Hipótese 3: Arrays Não Sincronizados**

**Cenário:**
- `RemoteActorIds` e `RemoteActors` não estão sendo atualizados na mesma ordem
- OU um dos arrays está sendo limpo/resetado em algum momento

---

## 🔧 **SOLUÇÃO 1: Verificar Conexão do Array_Find**

### **Verificação Crítica:**

**No Blueprint Editor:**

1. **Localize `Array_Find` no `ProcessNextFrame`**
2. **Verifique o pin `Target Array`:**
   - Deve estar conectado a: `Get RemoteActorIds` → `Return Value`
   - **NÃO** deve estar conectado a: `Get RemoteActors` ou outro array
   - **NÃO** deve estar usando `Make Array` ou `Set Variable`

3. **Verifique o pin `Item To Find`:**
   - Deve estar conectado a: `OutPlayerId` (do `ParseStateUpdateFrame`)
   - Deve ser do tipo `Integer`

4. **Verifique se há múltiplos `Array_Find`:**
   - Deve haver **APENAS UM** `Array_Find` para `RemoteActorIds`
   - Se houver múltiplos, pode estar causando confusão

---

## 🔧 **SOLUÇÃO 2: Adicionar Double-Check Antes de Usar FoundIndex**

### **Problema:**

Mesmo que `Array_Find` retorne um índice válido, precisamos **verificar** se o PlayerID no índice encontrado realmente corresponde ao `OutPlayerId`:

**Adicionar validação:**

```
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Branch (FoundIndex >= 0?)
  ├─ True:
  │   ↓
  │   Get Array Item (RemoteActorIds, FoundIndex) → FoundPlayerId
  │   ↓
  │   Equal (FoundPlayerId == OutPlayerId?)  ← VALIDAÇÃO CRÍTICA!
  │   ↓
  │   Branch
  │   ├─ True: Actor existe e PlayerID confere ✅
  │   │   ↓
  │   │   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │   │
  │   └─ False: PlayerID não confere ❌
  │       ↓
  │       [TRATAR COMO ACTOR NÃO EXISTE - spawnar novo]
  │
  └─ False: Actor não existe
      ↓
      SpawnActorFromClass
```

---

## 🔧 **SOLUÇÃO 3: Garantir Sincronização dos Arrays**

### **Verificação:**

**No Blueprint Editor:**

1. **Verifique a ordem de `Array_Add`:**
   ```
   SpawnActorFromClass
     ↓
   Array_Add (RemoteActorIds, OutPlayerId)  ← PRIMEIRO
     ↓ (then)
   Array_Add (RemoteActors, RemoteActorRef)  ← DEPOIS
   ```

2. **Verifique se ambos os `Array_Add` estão conectados corretamente:**
   - `Array_Add` para `RemoteActorIds`:
     - `Target Array`: `Get RemoteActorIds` → `Return Value`
     - `New Item`: `OutPlayerId`
   - `Array_Add` para `RemoteActors`:
     - `Target Array`: `Get RemoteActors` → `Return Value`
     - `New Item`: `RemoteActorRef` (do `SpawnActorFromClass`)

3. **Verifique se NÃO há `Make Array` ou `Set Variable` após `Array_Add`:**
   - `Array_Add` modifica o array diretamente
   - Não precisa de `Set Variable` após

---

## 🔧 **SOLUÇÃO 4: Adicionar Logs Detalhados**

### **Logs Críticos para Diagnóstico:**

**1. ANTES de `Array_Find`:**
```
Format Text: "[ProcessNextFrame] ANTES Array_Find - PlayerID: {0}, RemoteActorIds tamanho: {1}, conteúdo: {2}"
- {0}: OutPlayerId
- {1}: Get Array Length (RemoteActorIds)
- {2}: [OPCIONAL] Concatenação de todos os PlayerIDs no array
```

**2. APÓS `Array_Find`:**
```
Format Text: "[ProcessNextFrame] APÓS Array_Find - PlayerID: {0}, FoundIndex: {1}"
- {0}: OutPlayerId
- {1}: FoundIndex
```

**3. ANTES de `Get Array Item` (quando FoundIndex >= 0):**
```
Format Text: "[ProcessNextFrame] ANTES Get Array Item - FoundIndex: {0}, RemoteActorIds[FoundIndex]: {1}"
- {0}: FoundIndex
- {1}: Get Array Item (RemoteActorIds, FoundIndex) → FoundPlayerId
```

**4. VALIDAÇÃO (quando FoundIndex >= 0):**
```
Format Text: "[ProcessNextFrame] VALIDAÇÃO - OutPlayerId: {0}, FoundPlayerId: {1}, Match: {2}"
- {0}: OutPlayerId
- {1}: FoundPlayerId (do Get Array Item acima)
- {2}: Equal (OutPlayerId == FoundPlayerId?) → To String (Boolean)
```

---

## 🔧 **SOLUÇÃO 5: Implementar Validação Completa**

### **Fluxo Corrigido:**

```
ParseStateUpdateFrameWithAnimation → OutPlayerId
  ↓
[FILTRO: OutPlayerId != Active Player ID]
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Branch (FoundIndex >= 0?)
  ├─ True (índice encontrado):
  │   ↓
  │   Get Array Item (RemoteActorIds, FoundIndex) → FoundPlayerId
  │   ↓
  │   Equal (FoundPlayerId == OutPlayerId?)  ← VALIDAÇÃO CRÍTICA!
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
  └─ False (índice não encontrado):
      ↓
      SpawnActorFromClass
```

---

## ✅ **VERIFICAÇÃO IMEDIATA:**

### **Checklist:**

1. **`Array_Find`:**
   - [ ] Está usando `Get RemoteActorIds` corretamente?
   - [ ] Está buscando `OutPlayerId` corretamente?
   - [ ] Há apenas **UM** `Array_Find`?

2. **`Array_Add`:**
   - [ ] `Array_Add` para `RemoteActorIds` está conectado corretamente?
   - [ ] `Array_Add` para `RemoteActors` está conectado corretamente?
   - [ ] Ambos estão na ordem correta (Ids primeiro, depois Actors)?
   - [ ] **NÃO** há `Make Array` ou `Set Variable` após `Array_Add`?

3. **Validação:**
   - [ ] Há validação de `FoundPlayerId == OutPlayerId` antes de usar `FoundIndex`?
   - [ ] Se não houver, **ADICIONAR AGORA**!

---

## 🎯 **AÇÃO IMEDIATA:**

### **PASSO 1: Adicionar Validação de PlayerID**

**No Blueprint Editor:**

1. **Após `Array_Find` e `Branch (FoundIndex >= 0)` → `then` (True):**
   - Adicione `Get Array Item (RemoteActorIds, FoundIndex)` → `FoundPlayerId`
   - Adicione `Equal (Integer)` comparando `FoundPlayerId == OutPlayerId`
   - Adicione `Branch` com a condição do `Equal`
   - **Branch True:** Continue com `Get Array Item (RemoteActors, FoundIndex)`
   - **Branch False:** Trate como actor não existe (spawnar novo)

### **PASSO 2: Adicionar Logs**

Adicione logs detalhados conforme descrito acima para diagnosticar o problema.

### **PASSO 3: Verificar Conexões**

Verifique todas as conexões conforme o checklist acima.

---

## 📝 **RESUMO:**

**Problema:** `Array_Find` está sempre retornando `FoundIndex: 0`, causando que múltiplos players sejam mapeados para o mesmo actor.

**Causa provável:** Race condition ou falta de validação de `FoundPlayerId == OutPlayerId`.

**Solução imediata:** Adicionar validação de PlayerID antes de usar `FoundIndex` e adicionar logs detalhados.

**Próximo passo:** Implementar validação completa conforme descrito acima.

