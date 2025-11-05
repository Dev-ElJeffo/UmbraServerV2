# 🔧 **GUIA VISUAL: Conectar Not Equal ao Format Text**

## 📋 **PROBLEMA:**

Você precisa conectar o resultado do `Not Equal` ao `Format Text` para mostrar se deve processar ou não.

---

## ✅ **SOLUÇÃO PASSO A PASSO:**

### **ESTRUTURA COMPLETA:**

```
ParseStateUpdateFrame
  ↓ (ReturnValue == true)
  ├─ OutPlayerId: Integer
  └─ OutLocation: Vector
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
  ├─ Return Value: Integer (Active Player ID)
  ↓
Not Equal (Integer)
  ├─ Input A: OutPlayerId (do ParseStateUpdateFrame)
  ├─ Input B: Return Value (do Get Active Player ID)
  └─ Output: Boolean ← ESTE É O VALOR QUE VOCÊ PRECISA!
  ↓
Format Text: "🔍 [MyID:{0}] Filtro - Active: {0}, Out: {1}, Processar: {2}"
  ├─ {0}: Return Value (do Get Active Player ID) ← Active Player ID
  ├─ {1}: OutPlayerId (do ParseStateUpdateFrame) ← OutPlayerId
  └─ {2}: Output (do Not Equal) ← CONECTE O OUTPUT DO NOT EQUAL AQUI!
  ↓
Print String
  ↓
Branch
  ├─ Condition: Output (do Not Equal) ← MESMO OUTPUT DO NOT EQUAL
  ├─ True: CONTINUA (outro player)
  └─ False: PARA (próprio player)
```

---

## 🎯 **CONEXÕES ESPECÍFICAS:**

### **1. Criar o `Not Equal`:**

- **Nó:** `Not Equal (Integer)`
- **Input A:** `OutPlayerId` (do `ParseStateUpdateFrame`)
- **Input B:** `Return Value` (do `Get Active Player ID`)
- **Output:** Boolean (True se diferente, False se igual)

### **2. Conectar ao `Format Text`:**

**OPÇÃO 1: Se Format Text aceitar Boolean diretamente:**
- Conecte o **Output do `Not Equal`** diretamente ao pin `{2}` do `Format Text`

**OPÇÃO 2: Se Format Text NÃO aceitar Boolean (mais comum):**
- Adicione um nó `To String (Boolean)` entre o `Not Equal` e o `Format Text`:
  ```
  Not Equal
    └─ Output: Boolean
      ↓
  To String (Boolean)
    ├─ In Boolean: Output (do Not Equal)
    └─ Return Value: String ("True" ou "False")
      ↓
  Format Text
    └─ {2}: Return Value (do To String)
  ```

### **3. Conectar ao `Branch`:**

- O **mesmo Output do `Not Equal`** deve ser conectado ao `Condition` do `Branch`
- Não precisa converter para String aqui (Branch aceita Boolean diretamente)

---

## 📊 **RESULTADO ESPERADO NO LOG:**

**Quando `OutPlayerId: 18` e `MyID: 18`:**
```
[MyID:18] Filtro - Active: 18, Out: 18, Processar: False
```

**Quando `OutPlayerId: 1` e `MyID: 18`:**
```
[MyID:18] Filtro - Active: 18, Out: 1, Processar: True
```

---

## ⚠️ **IMPORTANTE:**

**O `Not Equal` deve ser usado DUAS vezes:**
1. Uma vez para o log (conectado ao `Format Text`)
2. Uma vez para o `Branch` (conectado ao `Condition`)

**Ou você pode usar o mesmo output do `Not Equal` para ambos:**
- Conecte o output do `Not Equal` ao `Format Text` (com `To String` se necessário)
- Conecte o **mesmo output** do `Not Equal` ao `Condition` do `Branch` (sem converter)

---

## 🔍 **VERIFICAÇÃO:**

Após conectar:
- ✅ Compilação deve passar sem erros
- ✅ Log deve mostrar `Processar: True` ou `Processar: False`
- ✅ `Processar: False` deve aparecer quando `OutPlayerId == MyID`
- ✅ `Processar: True` deve aparecer quando `OutPlayerId != MyID`

