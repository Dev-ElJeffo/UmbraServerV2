# 🔍 ANÁLISE E CORREÇÃO: OnDrop Atual

## 📊 **ANÁLISE DO CÓDIGO ATUAL:**

### ✅ **O QUE ESTÁ CORRETO:**

1. **Cast to Umbra Item Drag Drop Operation:**
   - ✅ `Object` conectado ao `Operation` do OnDrop
   - ✅ `execute` conectado ao `then` do Cast
   - ✅ `then` conectado ao `K2Node_IfThenElse_4`

2. **IsValid (Primeiro - Verifica Cast):**
   - ✅ `Object` conectado ao `AsUmbra Item Drag Drop Operation`
   - ✅ `ReturnValue` conectado ao `Condition` do `K2Node_IfThenElse_4`

3. **Branch (Is Valid? - Primeiro):**
   - ✅ `execute` conectado ao `then` do Cast
   - ✅ `Condition` conectado ao `ReturnValue` do IsValid
   - ✅ `then` conectado ao `K2Node_IfThenElse_5`
   - ✅ `else` conectado ao `K2Node_FunctionResult_4` (Return Unhandled)

4. **GetSourceSlotWidget:**
   - ✅ `self` conectado ao `K2Node_Knot_2` (que vem do Cast)
   - ✅ `ReturnValue` conectado corretamente a vários nós

5. **IsValid (Source Slot Widget):**
   - ✅ `Object` conectado ao `ReturnValue` do GetSourceSlotWidget
   - ✅ `ReturnValue` conectado ao `Condition` do `K2Node_IfThenElse_5`

6. **Branch (Is Valid? Source Slot Widget):**
   - ✅ `execute` conectado ao `then` do `K2Node_IfThenElse_4`
   - ✅ `Condition` conectado ao `ReturnValue` do IsValid
   - ✅ `then` conectado ao `K2Node_IfThenElse_6`

7. **GetSlotData (Source):**
   - ✅ `self` conectado ao `ReturnValue` do GetSourceSlotWidget
   - ✅ `ReturnValue` conectado ao `K2Node_BreakStruct_0`

8. **Break Umbra Inventory Slot:**
   - ✅ `UmbraInventorySlot` conectado ao `ReturnValue` do GetSlotData
   - ✅ `InventoryID` conectado ao `>` (Greater)

9. **Branch (Inventory ID > 0?):**
   - ✅ `execute` conectado ao `then` do `K2Node_IfThenElse_5`
   - ✅ `Condition` conectado ao `ReturnValue` do `>` (Greater)
   - ✅ `then` conectado ao `K2Node_Knot_6`

10. **Get Parent Storage Widget (Destino - Self):**
    - ✅ `self` conectado ao `K2Node_Self_0`
    - ✅ `ReturnValue` conectado ao `K2Node_CallFunction_18` (IsValid)

11. **IsValid (Parent Storage Widget do Destino):**
    - ✅ `Object` conectado ao `ReturnValue` do Get Parent Storage Widget (Destino)
    - ✅ `ReturnValue` conectado ao `Condition` do `K2Node_IfThenElse_9`

12. **Branch (Destino é Storage?):**
    - ✅ `execute` conectado ao `K2Node_Knot_7` (que vem do `K2Node_IfThenElse_7`)
    - ✅ `Condition` conectado ao `ReturnValue` do IsValid (Destino)

---

## ❌ **PROBLEMAS IDENTIFICADOS:**

### **PROBLEMA 1: Falta conexão de `execute` em vários nós**

Os nós `IsValid` e `GetSourceSlotWidget` são **funções puras** (não precisam de `execute`), mas alguns nós que dependem deles precisam de conexão de execução.

### **PROBLEMA 2: Lógica de verificação de origem está incorreta**

Você está verificando o `ParentStorageWidget` do **destino** (`self`) para determinar a **origem**, mas deveria verificar o `ParentStorageWidget` do **source** (origem).

### **PROBLEMA 3: Falta verificar origem antes de verificar destino**

A estrutura atual verifica:
1. Se há item (Inventory ID > 0) ✅
2. Se o destino é Storage (verificando `self`) ✅
3. Mas **NÃO verifica se a origem é Storage ou Inventário** ❌

### **PROBLEMA 4: Falta implementar as funções de movimento**

Após as verificações, faltam:
- `MoveItemToStorage` (Inventário → Storage)
- `MoveItemFromStorage` (Storage → Inventário)
- `ProcessItemDrop` (Inventário → Inventário)

---

## 🔧 **CORREÇÕES NECESSÁRIAS:**

### **CORREÇÃO 1: Adicionar Verificação de Origem**

**Após o `Branch (Inventory ID > 0?)` → `then`:**

Você precisa verificar se a **origem** (Source Slot Widget) pertence ao Storage:

```
Branch (Inventory ID > 0?) [then]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Source Slot Widget)  ← ADICIONAR!
          └─ Target: Return Value (do GetSourceSlotWidget)
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget da Origem)  ← ADICIONAR!
                  └─ Object: Return Value (do Get Parent Storage Widget Source)
                  └─ then (branco)
                      └─ Branch (Origem é Storage?)  ← ADICIONAR!
                          └─ Condition: Return Value (do Is Valid? Origem)
                          └─ TRUE = Origem É Storage
                          └─ FALSE = Origem NÃO É Storage (é Inventário)
```

**Como fazer:**

1. **Após o `K2Node_IfThenElse_6` (Branch Inventory ID > 0?) → `then`:**
   - Arraste o `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
   - Solte no espaço vazio
   - Digite "Get Parent Storage Widget"
   - Selecione o nó

2. **Conectar:**
   - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
   - **`execute` (branco):** Conecte ao `then` (branco) do `K2Node_IfThenElse_6`

3. **Criar IsValid (Parent Storage Widget da Origem):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - Solte no espaço vazio
   - Digite "Is Valid?"
   - Selecione o nó

4. **Conectar:**
   - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Source)`

5. **Criar Branch (Origem é Storage?):**
   - Arraste o `ReturnValue` (bool, verde) do `Is Valid? (Origem)`
   - Solte no espaço vazio
   - Digite "Branch"
   - Selecione o nó

6. **Conectar:**
   - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Is Valid? (Origem)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (Origem)`

---

### **CORREÇÃO 2: Reorganizar Verificação de Destino**

**A verificação de destino deve vir DEPOIS da verificação de origem:**

```
Branch (Origem é Storage?) [TRUE]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Destino - Self)  ← JÁ EXISTE!
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget do Destino)  ← JÁ EXISTE!
                  └─ then (branco)
                      └─ Branch (Destino é Storage?)  ← JÁ EXISTE!
                          └─ FALSE (Destino = Inventário)
                              └─ [Storage → Inventário]  ← IMPLEMENTAR AQUI!
```

**E também:**

```
Branch (Origem é Storage?) [FALSE]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Destino - Self)  ← JÁ EXISTE!
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget do Destino)  ← JÁ EXISTE!
                  └─ then (branco)
                      └─ Branch (Destino é Storage?)  ← JÁ EXISTE!
                          └─ TRUE (Destino = Storage)
                              └─ [Inventário → Storage]  ← IMPLEMENTAR AQUI!
                          └─ FALSE (Destino = Inventário)
                              └─ [Inventário → Inventário]  ← IMPLEMENTAR AQUI!
```

---

### **CORREÇÃO 3: Conectar Verificação de Destino Corretamente**

**Atualmente você tem:**

- `K2Node_IfThenElse_7` - Branch (Destino é Storage?) - conectado ao `K2Node_IfThenElse_6` (Inventory ID > 0?)
- `K2Node_IfThenElse_9` - Branch (Destino é Storage?) - conectado ao `K2Node_IfThenElse_7`

**PROBLEMA:** A verificação de destino está sendo feita **ANTES** de verificar a origem!

**SOLUÇÃO:** Mover a verificação de destino para **DEPOIS** da verificação de origem.

---

## 🔧 **ESTRUTURA CORRETA:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓ (then)
IsValid (Cast)  ← Verifica se o Cast foi bem-sucedido
  ↓ (then)
Branch (Is Valid? Cast)
  ├─ TRUE:
  │   └─ Get Source Slot Widget
  │       └─ then (branco) → IsValid (Source Slot Widget)
  │           └─ then (branco) → Branch (Is Valid? Source)
  │               ├─ TRUE:
  │               │   └─ Get Slot Data (Source)
  │               │       └─ then → Break Umbra Inventory Slot
  │               │           └─ then → Branch (Inventory ID > 0?)
  │               │               ├─ TRUE:
  │               │               │   └─ Get Parent Storage Widget (Source)  ← ADICIONAR!
  │               │               │       └─ then → Is Valid? (Origem)
  │               │               │           └─ then → Branch (Origem é Storage?)
  │               │               │               ├─ TRUE: (Origem = Storage)
  │               │               │               │   └─ Get Parent Storage Widget (Destino)
  │               │               │               │       └─ then → Is Valid? (Destino)
  │               │               │               │           └─ then → Branch (Destino é Storage?)
  │               │               │               │               ├─ TRUE: (Destino = Storage)
  │               │               │               │               │   └─ Return Unhandled  ← Storage → Storage (não implementar)
  │               │               │               │               │
  │               │               │               │               └─ FALSE: (Destino = Inventário)
  │               │               │               │                   └─ [Storage → Inventário]  ← IMPLEMENTAR!
  │               │               │               │
  │               │               │               └─ FALSE: (Origem = Inventário)
  │               │               │                   └─ Get Parent Storage Widget (Destino)
  │               │               │                       └─ then → Is Valid? (Destino)
  │               │               │                           └─ then → Branch (Destino é Storage?)
  │               │               │                               ├─ TRUE: (Destino = Storage)
  │               │               │                               │   └─ [Inventário → Storage]  ← IMPLEMENTAR!
  │               │               │                               │
  │               │               │                               └─ FALSE: (Destino = Inventário)
  │               │               │                                   └─ [Inventário → Inventário]  ← IMPLEMENTAR!
  │               │               │
  │               │               └─ FALSE:
  │               │                   └─ Return Unhandled
  │               │
  │               └─ FALSE:
  │                   └─ Return Unhandled
  │
  └─ FALSE:
      └─ Return Unhandled
```

---

## 🔧 **IMPLEMENTAÇÃO DETALHADA:**

### **PASSO 1: Adicionar Verificação de Origem**

**Localização:** Após `K2Node_IfThenElse_6` (Branch Inventory ID > 0?) → `then`

1. **Criar Get Parent Storage Widget (Source):**
   - Arraste o `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
   - Solte no espaço vazio
   - Digite "Get Parent Storage Widget"
   - Selecione o nó

2. **Conectar:**
   - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
   - **`execute` (branco):** Conecte ao `then` (branco) do `K2Node_IfThenElse_6`

3. **Criar IsValid (Parent Storage Widget da Origem):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - Solte no espaço vazio
   - Digite "Is Valid?"
   - Selecione o nó

4. **Conectar:**
   - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Source)`

5. **Criar Branch (Origem é Storage?):**
   - Arraste o `ReturnValue` (bool, verde) do `Is Valid? (Origem)`
   - Solte no espaço vazio
   - Digite "Branch"
   - Selecione o nó

6. **Conectar:**
   - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Is Valid? (Origem)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (Origem)`

---

### **PASSO 2: Reorganizar Verificação de Destino**

**Atualmente você tem `K2Node_IfThenElse_7` e `K2Node_IfThenElse_9` que verificam o destino, mas eles estão conectados ao lugar errado.**

**Você precisa:**

1. **Desconectar** `K2Node_IfThenElse_7` do `K2Node_IfThenElse_6`
2. **Conectar** `K2Node_IfThenElse_7` ao `then` do novo `Branch (Origem é Storage?)` quando `TRUE`
3. **Conectar** outro `Get Parent Storage Widget (Destino)` ao `FALSE` do `Branch (Origem é Storage?)`

---

### **PASSO 3: Implementar Storage → Inventário**

**Localização:** `Branch (Origem é Storage?)` [TRUE] → `Branch (Destino é Storage?)` [FALSE]

1. **Get Parent Storage Widget (Source) - Para obter WBP_Storage:**
   - Use o `Get Parent Storage Widget (Source)` que você criou no Passo 1
   - Ou crie um novo conectado ao `ReturnValue` do `GetSourceSlotWidget`

2. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - Solte no espaço vazio
   - Digite "Is Valid?"
   - Selecione o nó

3. **Conectar:**
   - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - **`execute` (branco):** Conecte ao `FALSE` (branco) do `Branch (Destino é Storage?)` quando origem é Storage

4. **Get Game Instance:**
   - Clique com botão direito → Digite "Get Game Instance"
   - Selecione o nó

5. **Conectar:**
   - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (WBP_Storage)`

6. **Cast to Umbra Game Instance:**
   - Arraste o `ReturnValue` (azul) do `Get Game Instance`
   - Solte no espaço vazio
   - Digite "Cast to Umbra Game Instance"
   - Selecione o nó

7. **Conectar:**
   - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Game Instance`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Get Game Instance`

8. **Get Storage ID By Inventory ID:**
   - Arraste o `As Umbra Game Instance` (azul) do `Cast`
   - Solte no espaço vazio
   - Digite "Get Storage ID By Inventory ID"
   - Selecione o nó

9. **Conectar:**
   - **`Target` (azul):** Conecte ao `As Umbra Game Instance` (azul) do `Cast`
   - **`Inventory ID` (int):** Conecte ao `InventoryID` (int) do `Break Umbra Inventory Slot`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Cast`

10. **Branch (Storage ID > 0?):**
    - Arraste o `ReturnValue` (int, verde) do `Get Storage ID By Inventory ID`
    - Solte no espaço vazio
    - Digite ">" (Greater)
    - Selecione o nó

11. **Conectar:**
    - **`A` (int):** Conecte ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
    - **`B` (int):** Clique com botão direito → "Make Literal Int" → Digite `0`

12. **Branch (Storage ID > 0?):**
    - Arraste o `ReturnValue` (bool, verde) do `>` (Greater)
    - Solte no espaço vazio
    - Digite "Branch"
    - Selecione o nó

13. **Conectar:**
    - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `>` (Greater)
    - **`execute` (branco):** Conecte ao `then` (branco) do `Get Storage ID By Inventory ID`

14. **Get Slot Index (Destino):**
    - Arraste um nó "Self" (clique com botão direito → "Self")
    - Arraste o pino azul do `Self`
    - Solte no espaço vazio
    - Digite "Get Slot Index"
    - Selecione o nó

15. **Conectar:**
    - **`Target` (azul):** Conecte ao `Self` (azul)
    - **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch (Storage ID > 0?)`

16. **Move Item From Storage:**
    - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
    - Solte no espaço vazio
    - Digite "Move Item From Storage"
    - Selecione o nó

17. **Conectar:**
    - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
    - **`Storage Item ID` (int):** Conecte ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
    - **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `Get Slot Index (Destino)`
    - **`execute` (branco):** Conecte ao `then` (branco) do `Get Slot Index (Destino)`

18. **Branch (Return Value do Move Item From Storage):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item From Storage`
    - Solte no espaço vazio
    - Digite "Branch"
    - Selecione o nó

19. **Conectar:**
    - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Move Item From Storage`
    - **`execute` (branco):** Conecte ao `then` (branco) do `Move Item From Storage`

20. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - Selecione o nó
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - Selecione o nó
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

### **PASSO 4: Implementar Inventário → Storage**

**Localização:** `Branch (Origem é Storage?)` [FALSE] → `Branch (Destino é Storage?)` [TRUE]

1. **Get Parent Storage Widget (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Solte no espaço vazio
   - Digite "Get Parent Storage Widget"
   - Selecione o nó

2. **Conectar:**
   - **`Target` (azul):** Conecte ao `Self` (azul)
   - **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch (Destino é Storage?)` quando origem é Inventário

3. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Solte no espaço vazio
   - Digite "Is Valid?"
   - Selecione o nó

4. **Conectar:**
   - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Destino)`

5. **Get Slot Index (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Solte no espaço vazio
   - Digite "Get Slot Index"
   - Selecione o nó

6. **Conectar:**
   - **`Target` (azul):** Conecte ao `Self` (azul)
   - **`execute` (branco):** Conecte ao `then` (branco) do `Is Valid? (WBP_Storage)`

7. **Move Item To Storage:**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Solte no espaço vazio
   - Digite "Move Item To Storage"
   - Selecione o nó

8. **Conectar:**
   - **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - **`Inventory Item ID` (int):** Conecte ao `InventoryID` (int) do `Break Umbra Inventory Slot`
   - **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `Get Slot Index (Destino)`
   - **`execute` (branco):** Conecte ao `then` (branco) do `Get Slot Index (Destino)`

9. **Branch (Return Value do Move Item To Storage):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item To Storage`
    - Solte no espaço vazio
    - Digite "Branch"
    - Selecione o nó

10. **Conectar:**
    - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Move Item To Storage`
    - **`execute` (branco):** Conecte ao `then` (branco) do `Move Item To Storage`

11. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - Selecione o nó
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - Selecione o nó
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

### **PASSO 5: Implementar Inventário → Inventário**

**Localização:** `Branch (Origem é Storage?)` [FALSE] → `Branch (Destino é Storage?)` [FALSE]

1. **Process Item Drop:**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Solte no espaço vazio
   - Digite "Process Item Drop"
   - Selecione o nó

2. **Conectar:**
   - **`Target` (azul):** Conecte ao `Self` (azul)
   - **`Dragged Slot Widget` (azul):** Conecte ao `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
   - **`execute` (branco):** Conecte ao `FALSE` (branco) do `Branch (Destino é Storage?)` quando origem é Inventário

3. **Return Handled:**
   - Clique com botão direito → Digite "Return Handled"
   - Selecione o nó
   - **Conectar `execute`:** ao `then` (branco) do `Process Item Drop`

---

## 📊 **RESUMO DAS CONEXÕES QUE FALTAM:**

### **1. Verificação de Origem (ADICIONAR):**

```
K2Node_IfThenElse_6 (Branch Inventory ID > 0?) [then]
  └─ Get Parent Storage Widget (Source)  ← CRIAR!
      └─ Target: ReturnValue do GetSourceSlotWidget
      └─ then → Is Valid? (Origem)  ← CRIAR!
          └─ Object: ReturnValue do Get Parent Storage Widget (Source)
          └─ then → Branch (Origem é Storage?)  ← CRIAR!
              └─ Condition: ReturnValue do Is Valid? (Origem)
```

### **2. Reorganizar Verificação de Destino:**

**Atualmente:**
- `K2Node_IfThenElse_7` está conectado ao `K2Node_IfThenElse_6` (errado!)

**Correto:**
- `K2Node_IfThenElse_7` deve estar conectado ao `then` do novo `Branch (Origem é Storage?)` quando `TRUE`
- Outro `Get Parent Storage Widget (Destino)` deve estar conectado ao `FALSE` do `Branch (Origem é Storage?)`

### **3. Implementar Funções de Movimento:**

- **Storage → Inventário:** Após `Branch (Destino é Storage?)` [FALSE] quando origem é Storage
- **Inventário → Storage:** Após `Branch (Destino é Storage?)` [TRUE] quando origem é Inventário
- **Inventário → Inventário:** Após `Branch (Destino é Storage?)` [FALSE] quando origem é Inventário

---

## ✅ **CHECKLIST FINAL:**

- [ ] Adicionar `Get Parent Storage Widget (Source)` após `Branch (Inventory ID > 0?)` [then]
- [ ] Adicionar `Is Valid? (Origem)` após `Get Parent Storage Widget (Source)`
- [ ] Adicionar `Branch (Origem é Storage?)` após `Is Valid? (Origem)`
- [ ] Reorganizar `Get Parent Storage Widget (Destino)` para vir DEPOIS da verificação de origem
- [ ] Implementar `MoveItemFromStorage` (Storage → Inventário)
- [ ] Implementar `MoveItemToStorage` (Inventário → Storage)
- [ ] Implementar `ProcessItemDrop` (Inventário → Inventário)
- [ ] Conectar todos os `Return Handled` e `Return Unhandled`

---

**Com estas correções, o OnDrop deve funcionar corretamente!** 🚀

