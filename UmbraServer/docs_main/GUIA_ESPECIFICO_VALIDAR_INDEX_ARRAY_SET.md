# 🔧 **GUIA ESPECÍFICO: Validar Index Antes de Array_Set**

## 🎯 **OBJETIVO:**

Adicionar validação `Index >= 0?` antes de executar `Array_Set` no `ProcessNextFrame`.

---

## 📍 **LOCALIZAÇÃO DOS NÓS:**

Baseado no XML fornecido:

- **`K2Node_CallFunction_63`**: `FindPlayerStateIndex`
  - **ReturnValue (Index)**: `373D93B54A2965D7D82F8CA19D332272`
  
- **`K2Node_CallArrayFunction_1`**: `Array_Set`
  - **execute**: `5DC08C2447784EF5CF998F80F0CB9BD9` (conectado ao `then` de `FindPlayerStateIndex`)
  - **Index**: `FAAD6A7F4BA75BA8232EDFA77510697E` (conectado ao `ReturnValue` de `FindPlayerStateIndex`)

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Desconectar `Array_Set` de `FindPlayerStateIndex`**

1. Localize o nó `K2Node_CallArrayFunction_1` (`Array_Set`)
2. **DESCONECTE** o pino `execute` (`5DC08C2447784EF5CF998F80F0CB9BD9`) do `then` (`CEF9FDBE45C85AD51EB7BB85E0DDB839`) de `K2Node_CallFunction_63`

### **PASSO 2: Adicionar `Greater or Equal (Integer)`**

1. **Clique direito** → Busque: `Greater or Equal (Integer)`
2. **Selecione:** `Greater or Equal (Integer)` (categoria `Math|Integer`)
3. **Posicione** entre `FindPlayerStateIndex` e `Array_Set`
4. **Conectar:**
   - **A:** `ReturnValue` (`373D93B54A2965D7D82F8CA19D332272`) do `FindPlayerStateIndex`
   - **B:** `0` (literal Integer - digite `0` no campo)
   - **ReturnValue:** Boolean

### **PASSO 3: Adicionar `Branch`**

1. **Clique direito** → Busque: `Branch`
2. **Selecione:** `Branch` (categoria `Flow Control`)
3. **Posicione** após `Greater or Equal`
4. **Conectar:**
   - **Condition:** `ReturnValue` do `Greater or Equal`
   - **execute:** `then` (`CEF9FDBE45C85AD51EB7BB85E0DDB839`) do `FindPlayerStateIndex`

### **PASSO 4: Conectar `Array_Set` ao `then` do `Branch`**

1. **Conectar:**
   - **execute** de `Array_Set` → **then** do `Branch`
   - **Index** de `Array_Set` → `ReturnValue` do `FindPlayerStateIndex` (mesmo valor de antes)

---

## 🎯 **ESTRUTURA FINAL:**

```
K2Node_CallFunction_63 (FindPlayerStateIndex)
  - ReturnValue: Index
  ↓ (then)
Greater or Equal (Integer)
  - A: Index
  - B: 0
  → Boolean
  ↓
Branch
  - Condition: Boolean
  - execute: then do FindPlayerStateIndex
  ├─ then (True): Index >= 0 ✅
  │    ↓
  │   K2Node_CallArrayFunction_1 (Array_Set)
  │    - execute: then do Branch
  │    - Index: Index (do FindPlayerStateIndex)
  │    - Item: ReturnValue do UpdatePlayerStateBuffer
  │
  └─ else (False): Index == -1 ❌
       (NÃO fazer nada)
```

---

## ⚠️ **IMPORTANTE:**

1. **Mantenha todas as outras conexões intactas:**
   - `Index` de `Array_Set` continua conectado ao `ReturnValue` de `FindPlayerStateIndex`
   - `Item` de `Array_Set` continua conectado ao `ReturnValue` de `UpdatePlayerStateBuffer`
   - `Target Array` de `Array_Set` continua conectado ao `Get RemoteStates`

2. **O `else` do `Branch` pode ficar desconectado** (não fazer nada se índice inválido)

3. **Após `Array_Set`, o fluxo continua normalmente** para `Set FoundRemoteActorID` e spawn/update do actor

---

## 🧪 **VERIFICAÇÃO:**

Após implementar:

1. **Compile o Blueprint**
2. **Execute 2 clientes**
3. **Mova ambos os clientes**
4. **Verifique os logs:**
   - ✅ Não deve aparecer: `Attempted to set an invalid index on array RemoteStates [-1/0]!`
   - ✅ Ambos os clientes devem se mover corretamente
   - ✅ Remote actors devem aparecer e se mover

---

**Status:** 🚨 **IMPLEMENTAR AGORA**

