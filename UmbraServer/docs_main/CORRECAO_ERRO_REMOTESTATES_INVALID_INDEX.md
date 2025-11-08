# 🔧 **CORREÇÃO: Erro "Attempted to set an invalid index on array RemoteStates [-1/0]"**

## 🚨 **PROBLEMA:**

```
LogScript: Warning: Script Msg: Attempted to set an invalid index on array RemoteStates [-1/0]!
```

**Causa:** O Blueprint está tentando usar um índice `-1` (retornado por `FindPlayerStateIndex` quando não encontra) diretamente no array `RemoteStates` sem validar primeiro.

---

## 🔍 **ANÁLISE:**

### **Função `FindPlayerStateIndex`:**
- **Retorna:** `-1` se não encontrar o PlayerID no array
- **Retorna:** `0` ou maior se encontrar

### **Problema no Blueprint:**
Quando `FindPlayerStateIndex` retorna `-1`, o Blueprint está tentando fazer:
- `Array_Set` com índice `-1` ❌
- `Get Array Item` com índice `-1` ❌

---

## ✅ **SOLUÇÃO:**

### **OPÇÃO 1: Usar `GetOrCreatePlayerState` (RECOMENDADO)**

**Esta função já trata o caso de índice inválido automaticamente:**

```
GetOrCreatePlayerState
  - StatesArray: Get RemoteStates (ref)
  - PlayerId: OutPlayerId
  → ReturnValue: PlayerStateEntry
```

**Vantagens:**
- ✅ Cria automaticamente se não existir
- ✅ Retorna o estado existente se encontrar
- ✅ Não precisa validar índice manualmente

**Como usar:**

1. **Substituir `FindPlayerStateIndex` + validação por `GetOrCreatePlayerState`:**
   ```
   [ANTES] ❌
   FindPlayerStateIndex → Branch (Index >= 0?) → Array_Set
   
   [DEPOIS] ✅
   GetOrCreatePlayerState → UpdatePlayerStateBuffer
   ```

2. **Exemplo completo:**
   ```
   ParseStateUpdateFrame
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
   ```

---

### **OPÇÃO 2: Validar Índice Antes de Usar (SE PRECISAR DO ÍNDICE)**

**Se você realmente precisa do índice para outra operação:**

```
FindPlayerStateIndex
  - StatesArray: Get RemoteStates
  - PlayerId: OutPlayerId
  → ReturnValue: Index (Integer)
  ↓
Greater or Equal (Integer)
  - A: Index
  - B: 0
  → ReturnValue: Boolean
  ↓
Branch
  ├─ True (Index >= 0): ✅ Usar Array_Set ou Get Array Item
  └─ False (Index == -1): ❌ Criar novo estado ou ignorar
```

**Exemplo:**

```
FindPlayerStateIndex → Index
  ↓
Greater or Equal (Index >= 0?)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Get Array Item (RemoteStates, Index)
  │    ↓
  │   UpdatePlayerStateBuffer
  │
  └─ False:
       ↓
      Make PlayerStateEntry (novo)
       ↓
      Array Add (RemoteStates, novo)
       ↓
      UpdatePlayerStateBuffer
```

---

## 🛠️ **PASSO A PASSO PARA CORRIGIR:**

### **PASSO 1: Localizar onde `FindPlayerStateIndex` está sendo usado**

1. Abra `BP_NetMovementClient`
2. Busque por `FindPlayerStateIndex` no Blueprint
3. Verifique onde o `ReturnValue` (Index) está sendo usado

### **PASSO 2: Verificar se há validação**

**Se NÃO houver validação (`Greater or Equal` ou `Branch`):**

**SOLUÇÃO RÁPIDA:** Substituir por `GetOrCreatePlayerState`

1. **Remover `FindPlayerStateIndex`**
2. **Adicionar `GetOrCreatePlayerState`:**
   - Clique direito → Busque: `GetOrCreatePlayerState`
   - **StatesArray:** `Get RemoteStates` (conectar como **ref** - pino de saída com seta)
   - **PlayerId:** `OutPlayerId`
   - **ReturnValue:** `PlayerStateEntry`

3. **Conectar diretamente a `UpdatePlayerStateBuffer`:**
   - **Entry:** `ReturnValue` do `GetOrCreatePlayerState` (conectar como **ref**)
   - **NewLocation:** `OutLocation`
   - **NewYaw:** `OutYawDegrees`
   - **NewTimestampMs:** `OutTimestampMs`

### **PASSO 3: Se precisar manter `FindPlayerStateIndex`**

**Adicionar validação obrigatória:**

1. **Após `FindPlayerStateIndex`:**
   - Adicionar `Greater or Equal (Integer)`
   - **A:** `ReturnValue` (Index) do `FindPlayerStateIndex`
   - **B:** `0` (literal Integer)

2. **Adicionar `Branch`:**
   - **Condition:** `ReturnValue` do `Greater or Equal`
   - **then (True):** Index válido (>= 0)
   - **else (False):** Index inválido (-1)

3. **No caminho `True`:**
   - Usar `Get Array Item` ou `Array_Set` normalmente

4. **No caminho `False`:**
   - Criar novo `PlayerStateEntry`
   - Adicionar ao array com `Array Add`
   - Ou usar `GetOrCreatePlayerState` diretamente

---

## 📋 **CHECKLIST:**

- [ ] Localizei onde `FindPlayerStateIndex` está sendo usado
- [ ] Verifiquei se há validação antes de usar o índice
- [ ] Se não houver validação: Substituí por `GetOrCreatePlayerState`
- [ ] Se precisar manter: Adicionei validação (`Index >= 0?`)
- [ ] Testei e o erro não aparece mais

---

## 🎯 **RECOMENDAÇÃO FINAL:**

**Use `GetOrCreatePlayerState` sempre que possível.** Esta função foi criada exatamente para evitar esse tipo de erro, pois:
- ✅ Trata automaticamente o caso de não encontrar
- ✅ Cria novo estado se necessário
- ✅ Retorna estado existente se encontrar
- ✅ Não precisa validar índice manualmente

---

## 📝 **NOTAS:**

- **Erro ocorre quando:** `FindPlayerStateIndex` retorna `-1` e o Blueprint tenta usar esse índice diretamente
- **Solução mais simples:** Usar `GetOrCreatePlayerState` em vez de `FindPlayerStateIndex`
- **Se precisar do índice:** Sempre validar com `Index >= 0?` antes de usar

