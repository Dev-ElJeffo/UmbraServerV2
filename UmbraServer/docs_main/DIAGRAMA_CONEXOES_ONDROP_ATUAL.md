# 📊 DIAGRAMA DE CONEXÕES: OnDrop Atual

## 🎯 **ANÁLISE VISUAL DAS CONEXÕES ATUAIS:**

### **ESTRUTURA ATUAL (O QUE VOCÊ TEM):**

```
OnDrop
  ↓
K2Node_DynamicCast_1 (Cast to Umbra Item Drag Drop Operation)
  ├─ Object: Operation (do OnDrop) ✅
  ├─ execute: (do OnDrop) ✅
  ├─ then → K2Node_IfThenElse_4 ✅
  └─ AsUmbra Item Drag Drop Operation → K2Node_CallFunction_11 (IsValid) ✅
                                      └─ K2Node_Knot_3 ✅

K2Node_CallFunction_11 (IsValid - Verifica Cast)
  ├─ Object: AsUmbra Item Drag Drop Operation (do Cast) ✅
  └─ ReturnValue → K2Node_IfThenElse_4 (Condition) ✅
                 └─ K2Node_CallFunction_15 (MakeLiteralBool) ✅

K2Node_IfThenElse_4 (Branch - Is Valid? Cast)
  ├─ execute: then (do Cast) ✅
  ├─ Condition: ReturnValue (do IsValid Cast) ✅
  ├─ then → K2Node_IfThenElse_5 ✅
  └─ else → K2Node_FunctionResult_4 (Return Unhandled) ✅

K2Node_IfThenElse_5 (Branch - Is Valid? Source Slot Widget)
  ├─ execute: then (do K2Node_IfThenElse_4) ✅
  ├─ Condition: ReturnValue (do IsValid Source) ✅
  ├─ then → K2Node_IfThenElse_6 ✅
  └─ else → (NÃO CONECTADO) ❌

K2Node_CallFunction_12 (GetSourceSlotWidget)
  ├─ self: K2Node_Knot_2 (que vem do Cast) ✅
  └─ ReturnValue → K2Node_CallFunction_13 (IsValid Source) ✅
                  └─ K2Node_CallFunction_10 (GetSlotData Source) ✅
                  └─ K2Node_Knot_4 ✅
                  └─ K2Node_Knot_8 ✅

K2Node_CallFunction_13 (IsValid - Source Slot Widget)
  ├─ Object: ReturnValue (do GetSourceSlotWidget) ✅
  └─ ReturnValue → K2Node_IfThenElse_5 (Condition) ✅

K2Node_CallFunction_10 (GetSlotData - Source)
  ├─ self: ReturnValue (do GetSourceSlotWidget) ✅
  └─ ReturnValue → K2Node_BreakStruct_0 ✅

K2Node_BreakStruct_0 (Break Umbra Inventory Slot)
  ├─ UmbraInventorySlot: ReturnValue (do GetSlotData) ✅
  └─ InventoryID → K2Node_PromotableOperator_0 (>) ✅

K2Node_PromotableOperator_0 (> - Inventory ID > 0?)
  ├─ A: InventoryID (do Break) ✅
  ├─ B: 0 (Make Literal Int) ✅
  └─ ReturnValue → K2Node_IfThenElse_6 (Condition) ✅

K2Node_IfThenElse_6 (Branch - Inventory ID > 0?)
  ├─ execute: then (do K2Node_IfThenElse_5) ✅
  ├─ Condition: ReturnValue (do >) ✅
  ├─ then → K2Node_Knot_6 ✅
  └─ else → (NÃO CONECTADO) ❌

K2Node_Knot_6
  ├─ InputPin: then (do K2Node_IfThenElse_6) ✅
  └─ OutputPin → K2Node_IfThenElse_7 (execute) ✅

K2Node_IfThenElse_7 (Branch - Destino é Storage?)
  ├─ execute: OutputPin (do K2Node_Knot_6) ✅
  ├─ Condition: ReturnValue (do K2Node_CallFunction_16) ✅
  ├─ then → (NÃO CONECTADO) ❌
  └─ else → K2Node_Knot_7 ✅

K2Node_CallFunction_16 (IsValid - Parent Storage Widget do Destino)
  ├─ Object: ReturnValue (do K2Node_VariableGet_0) ✅
  └─ ReturnValue → K2Node_IfThenElse_7 (Condition) ✅

K2Node_VariableGet_0 (Get Parent Storage Widget - Destino)
  ├─ self: (do K2Node_Knot_4) ✅
  └─ ParentStorageWidget → K2Node_CallFunction_16 (Object) ✅

K2Node_Knot_4
  ├─ InputPin: ReturnValue (do GetSourceSlotWidget) ✅
  └─ OutputPin → K2Node_VariableGet_0 (self) ✅

K2Node_IfThenElse_9 (Branch - Destino é Storage? - Outro)
  ├─ execute: OutputPin (do K2Node_Knot_7) ✅
  ├─ Condition: ReturnValue (do K2Node_CallFunction_18) ✅
  ├─ then → (NÃO CONECTADO) ❌
  └─ else → (NÃO CONECTADO) ❌

K2Node_CallFunction_18 (IsValid - Parent Storage Widget do Destino - Outro)
  ├─ Object: ReturnValue (do K2Node_VariableGet_2) ✅
  └─ ReturnValue → K2Node_IfThenElse_9 (Condition) ✅

K2Node_VariableGet_2 (Get Parent Storage Widget - Destino - Outro)
  ├─ self: K2Node_Self_0 ✅
  └─ ParentStorageWidget → K2Node_CallFunction_18 (Object) ✅

K2Node_VariableGet_3 (Get Parent Storage Widget - Source - Para Storage → Inventário)
  ├─ self: (do K2Node_Knot_8) ✅
  └─ ParentStorageWidget → K2Node_CallFunction_17 (Object) ✅

K2Node_CallFunction_17 (IsValid - Parent Storage Widget da Origem)
  ├─ Object: ReturnValue (do K2Node_VariableGet_3) ✅
  └─ ReturnValue → K2Node_IfThenElse_8 (Condition) ✅

K2Node_IfThenElse_8 (Branch - Origem é Storage?)
  ├─ execute: (NÃO CONECTADO) ❌
  ├─ Condition: ReturnValue (do K2Node_CallFunction_17) ✅
  ├─ then → (NÃO CONECTADO) ❌
  └─ else → (NÃO CONECTADO) ❌
```

---

## ❌ **PROBLEMAS IDENTIFICADOS:**

### **PROBLEMA 1: Verificação de Origem está desconectada**

**Você tem:**
- `K2Node_VariableGet_3` (Get Parent Storage Widget - Source)
- `K2Node_CallFunction_17` (IsValid - Origem)
- `K2Node_IfThenElse_8` (Branch - Origem é Storage?)

**Mas:**
- `K2Node_IfThenElse_8` não tem `execute` conectado! ❌
- `K2Node_IfThenElse_8` não tem `then` ou `else` conectados! ❌

**CORREÇÃO:**
- Conecte o `execute` do `K2Node_IfThenElse_8` ao `then` do `K2Node_IfThenElse_6` (Branch Inventory ID > 0?)
- Conecte o `then` do `K2Node_CallFunction_17` ao `execute` do `K2Node_IfThenElse_8`

---

### **PROBLEMA 2: Verificação de Destino está no lugar errado**

**Você tem:**
- `K2Node_IfThenElse_7` conectado ao `K2Node_Knot_6` (que vem do `K2Node_IfThenElse_6`)
- Isso significa que a verificação de destino está sendo feita **ANTES** de verificar a origem!

**CORREÇÃO:**
- A verificação de destino deve vir **DEPOIS** da verificação de origem
- `K2Node_IfThenElse_7` deve estar conectado ao `then` ou `else` do `K2Node_IfThenElse_8` (Branch Origem é Storage?)

---

### **PROBLEMA 3: Falta implementar as funções de movimento**

**Você tem todas as verificações, mas falta:**
- `MoveItemFromStorage` (Storage → Inventário)
- `MoveItemToStorage` (Inventário → Storage)
- `ProcessItemDrop` (Inventário → Inventário)

---

## 🔧 **CORREÇÕES ESPECÍFICAS:**

### **CORREÇÃO 1: Conectar Verificação de Origem**

**Localização:** Após `K2Node_IfThenElse_6` (Branch Inventory ID > 0?) → `then`

**Você já tem os nós criados, só precisa conectá-los:**

1. **Conectar `K2Node_VariableGet_3` (Get Parent Storage Widget - Source):**
   - **`self` (azul):** Já está conectado ao `K2Node_Knot_8` ✅
   - **`execute` (branco):** Conecte ao `then` (branco) do `K2Node_IfThenElse_6`

2. **Conectar `K2Node_CallFunction_17` (IsValid - Origem):**
   - **`Object` (azul):** Já está conectado ao `ReturnValue` do `K2Node_VariableGet_3` ✅
   - **`execute` (branco):** Conecte ao `then` (branco) do `K2Node_VariableGet_3`

3. **Conectar `K2Node_IfThenElse_8` (Branch - Origem é Storage?):**
   - **`Condition` (bool):** Já está conectado ao `ReturnValue` do `K2Node_CallFunction_17` ✅
   - **`execute` (branco):** Conecte ao `then` (branco) do `K2Node_CallFunction_17`

---

### **CORREÇÃO 2: Reorganizar Verificação de Destino**

**Atualmente:**
- `K2Node_IfThenElse_7` está conectado ao `K2Node_Knot_6` (que vem do `K2Node_IfThenElse_6`)
- Isso significa que está verificando o destino **ANTES** de verificar a origem

**Correto:**
- `K2Node_IfThenElse_7` deve estar conectado ao `then` ou `else` do `K2Node_IfThenElse_8` (Branch Origem é Storage?)

**Como fazer:**

1. **Desconectar `K2Node_IfThenElse_7` do `K2Node_Knot_6`:**
   - Remova a conexão do `execute` do `K2Node_IfThenElse_7`

2. **Conectar `K2Node_IfThenElse_7` ao `then` do `K2Node_IfThenElse_8` (quando origem é Storage):**
   - Conecte o `execute` do `K2Node_IfThenElse_7` ao `then` (branco) do `K2Node_IfThenElse_8`

3. **Criar outro `Get Parent Storage Widget (Destino)` para quando origem é Inventário:**
   - Após o `else` do `K2Node_IfThenElse_8`, adicione:
     - `Get Parent Storage Widget (Destino)` → `IsValid (Destino)` → `Branch (Destino é Storage?)`

---

### **CORREÇÃO 3: Implementar Storage → Inventário**

**Localização:** `K2Node_IfThenElse_8` [TRUE] → `K2Node_IfThenElse_7` [FALSE]

**Após o `else` do `K2Node_IfThenElse_7` (quando destino é Inventário):**

1. **Get Parent Storage Widget (Source) - Para obter WBP_Storage:**
   - Use o `K2Node_VariableGet_3` que você já tem
   - Ou crie um novo conectado ao `ReturnValue` do `GetSourceSlotWidget`

2. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
     - **`execute` (branco):** Conecte ao `else` (branco) do `K2Node_IfThenElse_7`

3. **Get Game Instance:**
   - Clique com botão direito → Digite "Get Game Instance"
   - **Conectar:**
     - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (WBP_Storage)`

4. **Cast to Umbra Game Instance:**
   - Arraste o `ReturnValue` (azul) do `Get Game Instance`
   - Digite "Cast to Umbra Game Instance"
   - **Conectar:**
     - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Game Instance`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Game Instance`

5. **Get Storage ID By Inventory ID:**
   - Arraste o `As Umbra Game Instance` (azul) do `Cast`
   - Digite "Get Storage ID By Inventory ID"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `As Umbra Game Instance` (azul) do `Cast`
     - **`Inventory ID` (int):** Conecte ao `InventoryID` (int) do `K2Node_BreakStruct_0`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Cast`

6. **Branch (Storage ID > 0?):**
   - Arraste o `ReturnValue` (int, verde) do `Get Storage ID By Inventory ID`
   - Digite ">" (Greater)
   - **Conectar:**
     - **`A` (int):** Conecte ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
     - **`B` (int):** Clique com botão direito → "Make Literal Int" → Digite `0`

7. **Branch (Storage ID > 0?):**
   - Arraste o `ReturnValue` (bool, verde) do `>` (Greater)
   - Digite "Branch"
   - **Conectar:**
     - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `>` (Greater)
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Storage ID By Inventory ID`

8. **Get Slot Index (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Slot Index"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch (Storage ID > 0?)`

9. **Move Item From Storage:**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)` (K2Node_VariableGet_3)
   - Digite "Move Item From Storage"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
     - **`Storage Item ID` (int):** Conecte ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
     - **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `Get Slot Index (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Slot Index (Destino)`

10. **Branch (Return Value do Move Item From Storage):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item From Storage`
    - Digite "Branch"
    - **Conectar:**
      - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Move Item From Storage`
      - **`execute` (branco):** Conecte ao `then` (branco) do `Move Item From Storage`

11. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

### **CORREÇÃO 4: Implementar Inventário → Storage**

**Localização:** `K2Node_IfThenElse_8` [FALSE] → Novo `Branch (Destino é Storage?)` [TRUE]

**Você precisa criar um novo caminho após o `else` do `K2Node_IfThenElse_8`:**

1. **Get Parent Storage Widget (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Parent Storage Widget"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`execute` (branco):** Conecte ao `else` (branco) do `K2Node_IfThenElse_8`

2. **IsValid (Parent Storage Widget do Destino):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Destino)`

3. **Branch (Destino é Storage?):**
   - Arraste o `ReturnValue` (bool, verde) do `Is Valid? (Destino)`
   - Digite "Branch"
   - **Conectar:**
     - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Is Valid? (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (Destino)`

4. **Get Parent Storage Widget (Destino) - Para obter WBP_Storage:**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Parent Storage Widget"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch (Destino é Storage?)`

5. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Destino)`

6. **Get Slot Index (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Slot Index"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (WBP_Storage)`

7. **Move Item To Storage:**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Move Item To Storage"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`Inventory Item ID` (int):** Conecte ao `InventoryID` (int) do `K2Node_BreakStruct_0`
     - **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `Get Slot Index (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Slot Index (Destino)`

8. **Branch (Return Value do Move Item To Storage):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item To Storage`
    - Digite "Branch"
    - **Conectar:**
      - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Move Item To Storage`
      - **`execute` (branco):** Conecte ao `then` (branco) do `Move Item To Storage`

9. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

### **CORREÇÃO 5: Implementar Inventário → Inventário**

**Localização:** `K2Node_IfThenElse_8` [FALSE] → Novo `Branch (Destino é Storage?)` [FALSE]

**Após o `else` do novo `Branch (Destino é Storage?)`:**

1. **Process Item Drop:**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Process Item Drop"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`Dragged Slot Widget` (azul):** Conecte ao `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
     - **`execute` (branco):** Conecte ao `FALSE` (branco) do novo `Branch (Destino é Storage?)`

2. **Return Handled:**
   - Clique com botão direito → Digite "Return Handled"
   - **Conectar `execute`:** ao `then` (branco) do `Process Item Drop`

---

## 📊 **RESUMO DAS CONEXÕES QUE FALTAM:**

### **1. Conectar Verificação de Origem:**

```
K2Node_IfThenElse_6 (Branch Inventory ID > 0?) [then]
  └─ execute (branco) → K2Node_VariableGet_3 (Get Parent Storage Widget - Source) [execute]  ← CONECTAR!
      └─ then (branco) → K2Node_CallFunction_17 (IsValid - Origem) [execute]  ← CONECTAR!
          └─ then (branco) → K2Node_IfThenElse_8 (Branch - Origem é Storage?) [execute]  ← CONECTAR!
```

### **2. Reorganizar Verificação de Destino:**

**Para quando Origem = Storage:**
```
K2Node_IfThenElse_8 (Branch - Origem é Storage?) [then]
  └─ execute (branco) → K2Node_IfThenElse_7 (Branch - Destino é Storage?) [execute]  ← CONECTAR!
```

**Para quando Origem = Inventário:**
```
K2Node_IfThenElse_8 (Branch - Origem é Storage?) [else]
  └─ execute (branco) → Get Parent Storage Widget (Destino) [execute]  ← CRIAR E CONECTAR!
      └─ then → IsValid (Destino) [execute]  ← CRIAR E CONECTAR!
          └─ then → Branch (Destino é Storage?) [execute]  ← CRIAR E CONECTAR!
```

### **3. Implementar Funções de Movimento:**

- **Storage → Inventário:** Após `K2Node_IfThenElse_7` [else]
- **Inventário → Storage:** Após novo `Branch (Destino é Storage?)` [TRUE] quando origem é Inventário
- **Inventário → Inventário:** Após novo `Branch (Destino é Storage?)` [FALSE] quando origem é Inventário

---

## ✅ **CHECKLIST DE CORREÇÃO:**

- [ ] Conectar `execute` do `K2Node_VariableGet_3` ao `then` do `K2Node_IfThenElse_6`
- [ ] Conectar `execute` do `K2Node_CallFunction_17` ao `then` do `K2Node_VariableGet_3`
- [ ] Conectar `execute` do `K2Node_IfThenElse_8` ao `then` do `K2Node_CallFunction_17`
- [ ] Desconectar `K2Node_IfThenElse_7` do `K2Node_Knot_6`
- [ ] Conectar `execute` do `K2Node_IfThenElse_7` ao `then` do `K2Node_IfThenElse_8`
- [ ] Criar novo caminho após `else` do `K2Node_IfThenElse_8` para quando origem é Inventário
- [ ] Implementar `MoveItemFromStorage` (Storage → Inventário)
- [ ] Implementar `MoveItemToStorage` (Inventário → Storage)
- [ ] Implementar `ProcessItemDrop` (Inventário → Inventário)
- [ ] Conectar todos os `Return Handled` e `Return Unhandled`

---

**Com estas correções específicas, o OnDrop deve funcionar corretamente!** 🚀

