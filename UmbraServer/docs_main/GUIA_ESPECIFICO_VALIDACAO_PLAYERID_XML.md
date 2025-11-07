# 🔧 **GUIA ESPECÍFICO: Adicionar Validação de PlayerID no ProcessNextFrame**

## 📋 **ESTRUTURA BLUEPRINT ATUAL:**

### **Nós Existentes no Blueprint:**

1. **`Array Find`** (`K2Node_CallArrayFunction_4`)
   - **Função:** Array Find (KismetArrayLibrary)
   - **Target Array:** Conectado a `Get RemoteActorIds`
   - **Item To Find:** Conectado a `OutPlayerId` (do ParseStateUpdateFrameWithAnimation)
   - **Return Value:** `FoundIndex` (Integer)

2. **`Greater or Equal (Integer)`** (`K2Node_PromotableOperator_10`)
   - **A:** `FoundIndex` (do Array Find)
   - **B:** `0` (constante)
   - **Return Value:** Boolean (`FoundIndex >= 0`)

3. **`Branch`** (`K2Node_IfThenElse_6`)
   - **Condition:** `FoundIndex >= 0` (do Greater or Equal)
   - **True (then):** **ATUALMENTE CONECTADO DIRETAMENTE AO FLUXO DE ATUALIZAÇÃO** ← DESCONECTAR DAQUI!
   - **False (else):** Conectado ao fluxo de spawnar novo actor

4. **`Get RemoteActorIds`** (`K2Node_VariableGet_11`)
   - **Variável:** RemoteActorIds (Array of Integer)
   - **Return Value:** Array de PlayerIDs

5. **`Knot` com FoundIndex** (`K2Node_Knot_48`)
   - **Input:** `FoundIndex` (do Array Find)
   - **Output:** `FoundIndex` (para uso em outros nós)

6. **`Knot` com OutPlayerId** (`K2Node_Knot_126`)
   - **Input:** `OutPlayerId` (do ParseStateUpdateFrameWithAnimation)
   - **Output:** `OutPlayerId` (para uso em outros nós)

7. **`Knot` no fluxo de atualização** (`K2Node_Knot_23`)
   - **Input:** **ATUALMENTE CONECTADO AO `then` DO BRANCH** ← DESCONECTAR DAQUI!
   - **Output:** Continua o fluxo de atualização do actor existente

8. **`Branch` no fluxo de spawn** (`K2Node_IfThenElse_3`)
   - **execute:** Conectado ao `else` do Branch principal
   - Continua o fluxo de spawnar novo actor

---

## ✅ **CORREÇÃO: Adicionar Validação de PlayerID**

### **PASSO 1: Desconectar o Pin `then` do Branch**

**No Blueprint Editor:**

1. Localize o nó **`Branch`** (`K2Node_IfThenElse_6`)
2. Localize o pin **`True`** (then)
3. **Desconecte** este pin do nó seguinte no fluxo (geralmente um `Knot` ou `Get Array Item` para `RemoteActors`)
   - Isso permitirá inserir a validação antes de continuar

---

### **PASSO 2: Adicionar `Get Array Item` para `RemoteActorIds`**

**No Blueprint Editor:**

1. **Adicione `Get Array Item`:**
   - Clique direito → "Get Array Item"
   - O nó será criado automaticamente

2. **Conecte os pins de DADOS:**
   - **Pin `Array`:**
     - Conecte ao pin **`Return Value`** do nó **`Get RemoteActorIds`**
     - Este é o array que contém os PlayerIDs dos actors remotos
   - **Pin `Index`:**
     - Conecte ao pin **`Return Value`** do nó **`Array Find`** (ou ao `Knot` que contém `FoundIndex`)
     - Este é o índice encontrado pelo `Array Find`
   - **Pin `Output`:** Este será o `FoundPlayerId` (Integer) - será usado no próximo passo

3. **Conecte o fluxo de EXECUÇÃO:**
   - **Pin `execute`:** Conecte ao pin **`True`** (then) do nó **`Branch`** (`K2Node_IfThenElse_6`)
   - Isso fará com que o `Get Array Item` execute quando `FoundIndex >= 0`

---

### **PASSO 3: Adicionar `Equal (Integer)`**

**No Blueprint Editor:**

1. **Adicione `Equal (Integer)`:**
   - Clique direito → "Equal (Integer)"
   - O nó será criado automaticamente

2. **Conecte os pins de DADOS:**
   - **Pin `A`:**
     - Conecte ao pin **`Output`** do nó **`Get Array Item`** (criado no Passo 2)
     - Este é o `FoundPlayerId` (Integer) - o PlayerID encontrado no array no índice `FoundIndex`
   - **Pin `B`:**
     - Conecte ao pin **`OutPlayerId`** do nó **`ParseStateUpdateFrameWithAnimation`** (ou ao `Knot` que contém `OutPlayerId`)
     - Este é o `OutPlayerId` (Integer) - o PlayerID do frame recebido
   - **Pin `Return Value`:** Boolean - será usado no Branch do próximo passo

3. **Conecte o fluxo de EXECUÇÃO:**
   - **Pin `execute`:** Conecte ao pin **`then`** (output de execução) do nó **`Get Array Item`**
   - Isso fará com que o `Equal` execute após o `Get Array Item`

---

### **PASSO 4: Adicionar `Branch` para Validação**

**No Blueprint Editor:**

1. **Adicione `Branch`:**
   - Clique direito → "Branch"
   - O nó será criado automaticamente

2. **Conecte os pins:**
   - **Pin `Condition`:**
     - Conecte ao pin **`Return Value`** do nó **`Equal (Integer)`** (criado no Passo 3)
     - Este é o resultado da comparação `FoundPlayerId == OutPlayerId`
   - **Pin `execute`:**
     - Conecte ao pin **`then`** (output de execução) do nó **`Get Array Item`**
     - Isso fará com que o Branch execute após o `Get Array Item`
   - **Pin `True` (then):** PlayerID confere ✅
     - Conecte ao nó que estava originalmente conectado ao `then` do Branch principal
     - Este é o fluxo original quando o actor existe (geralmente um `Knot` ou `Get Array Item` para `RemoteActors`)
   - **Pin `False` (else):** PlayerID NÃO confere ❌
     - Conecte ao mesmo fluxo do `SpawnActorFromClass` (ver Passo 5)

---

### **PASSO 5: Conectar o Fluxo do `False` (else)**

**No Blueprint Editor:**

1. **Localize o Branch** que está conectado ao `False` (else) do Branch principal (`K2Node_IfThenElse_3`)
2. **Conecte o `False` do novo Branch ao mesmo fluxo:**
   - Conecte o pin **`False`** (else) do novo Branch ao pin **`execute`** do Branch de spawn
   - Isso fará com que, quando o PlayerID não conferir, o código trate como actor não existe e spawna novo

---

## 📊 **ESTRUTURA FINAL VISUAL:**

```
Array Find
  ├─ Target Array: Get RemoteActorIds
  ├─ Item To Find: OutPlayerId
  └─ Return Value: FoundIndex
      ↓
Greater or Equal (Integer)
  ├─ A: FoundIndex
  ├─ B: 0
  └─ Return Value: FoundIndex >= 0?
      ↓
Branch (FoundIndex >= 0?)
  ├─ Condition: FoundIndex >= 0?
  │
  ├─ True (then): FoundIndex >= 0
  │   │
  │   └─ [NOVO] Get Array Item (RemoteActorIds)
  │       ├─ Array: Get RemoteActorIds → Return Value
  │       ├─ Index: FoundIndex (do Array Find)
  │       ├─ execute: Conectado ao True do Branch acima
  │       └─ Output: FoundPlayerId (Integer)
  │           │
  │           └─ [NOVO] Equal (Integer)
  │               ├─ A: FoundPlayerId (do Get Array Item)
  │               ├─ B: OutPlayerId (do ParseStateUpdateFrameWithAnimation)
  │               ├─ execute: Conectado ao then do Get Array Item
  │               └─ Return Value: Boolean
  │                   │
  │                   └─ [NOVO] Branch (Validação)
  │                       ├─ Condition: FoundPlayerId == OutPlayerId?
  │                       ├─ execute: Conectado ao then do Get Array Item
  │                       │
  │                       ├─ True (then): PlayerID confere ✅
  │                       │   │
  │                       │   └─ [FLUXO ORIGINAL] Get Array Item (RemoteActors)
  │                       │       └─ Continua atualização do actor existente
  │                       │
  │                       └─ False (else): PlayerID NÃO confere ❌
  │                           │
  │                           └─ [FLUXO DE SPAWN] Branch de spawn
  │                               └─ SpawnActorFromClass
  │
  └─ False (else): FoundIndex < 0
      │
      └─ [FLUXO DE SPAWN] Branch de spawn
          └─ SpawnActorFromClass
```

---

## 🔍 **REFERÊNCIAS DOS NÓS EXISTENTES:**

### **Nós que você já tem no Blueprint:**

1. **`Get RemoteActorIds`** (`K2Node_VariableGet_11`)
   - **Variável:** RemoteActorIds (Array of Integer)
   - **Return Value:** Array de PlayerIDs
   - **Uso:** Conecte ao pin `Array` do novo `Get Array Item`

2. **`Array Find`** (`K2Node_CallArrayFunction_4`)
   - **Return Value:** `FoundIndex` (Integer)
   - **Uso:** Conecte ao pin `Index` do novo `Get Array Item`

3. **`ParseStateUpdateFrameWithAnimation`** (`K2Node_CallFunction_38`)
   - **OutPlayerId:** Integer
   - **Uso:** Conecte ao pin `B` do novo `Equal (Integer)`

4. **Nó no fluxo de atualização** (geralmente `Knot` ou `Get Array Item` para `RemoteActors`)
   - **Uso:** Conecte o pin `True` do novo Branch aqui (fluxo original)

5. **Branch no fluxo de spawn** (`K2Node_IfThenElse_3`)
   - **Uso:** Conecte o pin `False` do novo Branch aqui (spawnar novo)

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **Passo 1: Desconectar**
- [ ] Localizei o `Branch` após `Array Find`
- [ ] Desconectei o pin `True` (then) do nó seguinte no fluxo

### **Passo 2: Adicionar Get Array Item**
- [ ] Adicionei `Get Array Item`
- [ ] Conectei o pin `Array` ao `Return Value` de `Get RemoteActorIds`
- [ ] Conectei o pin `Index` ao `Return Value` de `Array Find` (FoundIndex)
- [ ] Conectei o pin `execute` ao pin `True` do Branch principal

### **Passo 3: Adicionar Equal**
- [ ] Adicionei `Equal (Integer)`
- [ ] Conectei o pin `A` ao `Output` do `Get Array Item` (FoundPlayerId)
- [ ] Conectei o pin `B` ao `OutPlayerId` do `ParseStateUpdateFrameWithAnimation`
- [ ] Conectei o pin `execute` ao `then` do `Get Array Item`

### **Passo 4: Adicionar Branch**
- [ ] Adicionei `Branch`
- [ ] Conectei o pin `Condition` ao `Return Value` do `Equal`
- [ ] Conectei o pin `execute` ao `then` do `Get Array Item`
- [ ] Conectei o pin `True` ao nó do fluxo original (atualizar actor existente)
- [ ] Conectei o pin `False` ao Branch de spawn (spawnar novo)

---

## 🎯 **RESULTADO ESPERADO:**

Após implementar:

1. **Quando `FoundIndex >= 0`:**
   - O código executa `Get Array Item` para obter `FoundPlayerId = RemoteActorIds[FoundIndex]`
   - Compara `FoundPlayerId == OutPlayerId` usando `Equal`
   - Se **SIM** (Branch True): Usa o actor existente (fluxo original)
   - Se **NÃO** (Branch False): Trata como actor não existe e spawna novo

2. **Isso previne:**
   - Múltiplos players sendo mapeados para o mesmo actor
   - Sobreposição de atores
   - Controle bugado

---

**Esta correção deve resolver o problema de sobreposição de atores!**
