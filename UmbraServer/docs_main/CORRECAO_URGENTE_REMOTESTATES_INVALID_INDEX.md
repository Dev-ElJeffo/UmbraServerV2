# 🚨 **CORREÇÃO URGENTE: Erro RemoteStates [-1/0]**

## 🔴 **PROBLEMA CRÍTICO:**

```
LogScript: Warning: Script Msg: Attempted to set an invalid index on array RemoteStates [-1/0]!
```

**Causa:** `FindPlayerStateIndex` retorna `-1` quando não encontra o player, e o Blueprint está usando esse índice inválido diretamente no `Array_Set` ou `Get Array Item`.

---

## ✅ **SOLUÇÃO IMEDIATA:**

### **SUBSTITUIR `FindPlayerStateIndex` por `GetOrCreatePlayerState`**

**Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Localizar `FindPlayerStateIndex`**

No `ProcessNextFrame`, localize o nó `FindPlayerStateIndex` que está sendo usado para atualizar `RemoteStates`.

### **PASSO 2: Remover `FindPlayerStateIndex` e `Array_Set`**

**REMOVER:**
- `FindPlayerStateIndex`
- `Array_Set` que usa o índice retornado
- Qualquer validação `Index >= 0?` relacionada

### **PASSO 3: Adicionar `GetOrCreatePlayerState`**

**ADICIONAR:**

1. **Clique direito** → Busque: `GetOrCreatePlayerState`
2. **Selecione:** `GetOrCreatePlayerState` (da categoria `Umbra|Net|WS|Binary`)
3. **Conectar:**
   - **StatesArray:** `Get RemoteStates` (conectar como **ref** - pino de saída com seta ←)
   - **PlayerId:** `OutPlayerId` (do `ParseStateUpdateFrame`)
   - **ReturnValue:** `PlayerStateEntry` (estrutura)

### **PASSO 4: Conectar a `UpdatePlayerStateBuffer`**

**ADICIONAR `UpdatePlayerStateBuffer`:**

1. **Clique direito** → Busque: `UpdatePlayerStateBuffer`
2. **Selecione:** `UpdatePlayerStateBuffer`
3. **Conectar:**
   - **Entry:** `ReturnValue` do `GetOrCreatePlayerState` (conectar como **ref** - pino com seta ←)
   - **NewLocation:** `OutLocation` (do `ParseStateUpdateFrame`)
   - **NewYaw:** `OutYawDegrees` (do `ParseStateUpdateFrame`)
   - **NewTimestampMs:** `OutTimestampMs` (do `ParseStateUpdateFrame`)

### **PASSO 5: Adicionar `Set Element` (CRÍTICO!)**

**IMPORTANTE:** Após `UpdatePlayerStateBuffer`, você DEVE fazer `Set Element` para copiar a estrutura modificada de volta para o array!

1. **Adicionar `FindPlayerStateIndex`** (apenas para obter o índice):
   - **StatesArray:** `Get RemoteStates`
   - **PlayerId:** `OutPlayerId`
   - **ReturnValue:** `Index` (Integer)

2. **Adicionar `Greater or Equal (Integer)`:**
   - **A:** `Index` (do `FindPlayerStateIndex`)
   - **B:** `0`
   - **ReturnValue:** Boolean

3. **Adicionar `Branch`:**
   - **Condition:** `ReturnValue` do `Greater or Equal`
   - **then (True):** Index válido (>= 0)

4. **No caminho `True` do Branch, adicionar `Set Element`:**
   - **Target Array:** `Get RemoteStates` (conectar como **ref** - pino com seta ←)
   - **Index:** `Index` (do `FindPlayerStateIndex`)
   - **Item:** `ReturnValue` do `UpdatePlayerStateBuffer` (a Entry modificada)

---

## 🎯 **ESTRUTURA COMPLETA:**

```
ParseStateUpdateFrame
  ↓
Not Equal (Integer): OutPlayerId != MyPlayerId
  ↓
Branch → [True] (é outro player)
  ↓
GetOrCreatePlayerState
  - StatesArray: Get RemoteStates (ref)
  - PlayerId: OutPlayerId
  → ReturnValue: PlayerStateEntry
  ↓
UpdatePlayerStateBuffer
  - Entry: ReturnValue (ref)
  - NewLocation: OutLocation
  - NewYaw: OutYawDegrees
  - NewTimestampMs: OutTimestampMs
  ↓
FindPlayerStateIndex (apenas para obter índice)
  - StatesArray: Get RemoteStates
  - PlayerId: OutPlayerId
  → Index
  ↓
Greater or Equal (Index >= 0?)
  ↓
Branch → [True]
  ↓
Set Element
  - Target Array: Get RemoteStates (ref)
  - Index: Index
  - Item: ReturnValue do UpdatePlayerStateBuffer
  ↓
[Continuar para spawn/update de actor]
```

---

## ⚠️ **IMPORTANTE:**

1. **`GetOrCreatePlayerState`** cria automaticamente se não encontrar
2. **`UpdatePlayerStateBuffer`** modifica a cópia da estrutura
3. **`Set Element`** é **OBRIGATÓRIO** para copiar de volta para o array
4. **Sempre validar `Index >= 0`** antes de usar `Set Element`

---

## 🧪 **TESTE IMEDIATO:**

Após corrigir, o erro **NÃO deve mais aparecer**:

```
❌ ANTES: LogScript: Warning: Attempted to set an invalid index on array RemoteStates [-1/0]!
✅ DEPOIS: (sem erro)
```

---

**Status:** 🚨 **CORREÇÃO URGENTE - IMPLEMENTAR AGORA**

