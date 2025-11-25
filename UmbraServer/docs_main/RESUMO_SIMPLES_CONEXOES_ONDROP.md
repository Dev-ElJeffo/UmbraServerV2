# 📝 RESUMO SIMPLES: Conexões do OnDrop

## 🎯 **O QUE VOCÊ TEM E O QUE FALTA:**

### ✅ **O QUE ESTÁ CORRETO:**

1. Cast para `UmbraItemDragDropOperation` ✅
2. Verificação se Source Slot Widget é válido ✅
3. Obtenção dos dados do slot de origem ✅
4. Verificação se Inventory ID > 0 ✅
5. Verificação se destino é Storage (mas no lugar errado) ⚠️

### ❌ **O QUE ESTÁ FALTANDO:**

1. **Verificação de ORIGEM** (Storage ou Inventário) ❌
2. **Reorganização da verificação de DESTINO** (deve vir depois da origem) ❌
3. **Funções de movimento** (MoveItemToStorage, MoveItemFromStorage, ProcessItemDrop) ❌

---

## 🔧 **CORREÇÃO RÁPIDA - 3 PASSOS:**

### **PASSO 1: Conectar Verificação de Origem**

**Você já tem os nós criados (`K2Node_VariableGet_3`, `K2Node_CallFunction_17`, `K2Node_IfThenElse_8`), só precisa conectá-los:**

```
K2Node_IfThenElse_6 (Branch Inventory ID > 0?) [then]
  └─ execute (branco) → K2Node_VariableGet_3 [execute]  ← CONECTAR ESTA LINHA!
      └─ then (branco) → K2Node_CallFunction_17 [execute]  ← CONECTAR ESTA LINHA!
          └─ then (branco) → K2Node_IfThenElse_8 [execute]  ← CONECTAR ESTA LINHA!
```

**Como fazer:**
1. Clique no pino `then` (branco) do `K2Node_IfThenElse_6`
2. Arraste até o pino `execute` (branco) do `K2Node_VariableGet_3`
3. Clique no pino `then` (branco) do `K2Node_VariableGet_3`
4. Arraste até o pino `execute` (branco) do `K2Node_CallFunction_17`
5. Clique no pino `then` (branco) do `K2Node_CallFunction_17`
6. Arraste até o pino `execute` (branco) do `K2Node_IfThenElse_8`

---

### **PASSO 2: Reorganizar Verificação de Destino**

**Atualmente:**
- `K2Node_IfThenElse_7` está conectado ao `K2Node_Knot_6` (que vem do `K2Node_IfThenElse_6`)
- Isso está **ERRADO** porque verifica o destino **ANTES** de verificar a origem

**Correto:**
- `K2Node_IfThenElse_7` deve estar conectado ao `then` do `K2Node_IfThenElse_8` (quando origem é Storage)

**Como fazer:**

1. **Desconectar `K2Node_IfThenElse_7` do `K2Node_Knot_6`:**
   - Clique na conexão entre `K2Node_Knot_6` → `K2Node_IfThenElse_7`
   - Pressione `Delete` ou clique com botão direito → "Break Link"

2. **Conectar `K2Node_IfThenElse_7` ao `then` do `K2Node_IfThenElse_8`:**
   - Clique no pino `then` (branco) do `K2Node_IfThenElse_8`
   - Arraste até o pino `execute` (branco) do `K2Node_IfThenElse_7`

3. **Criar novo caminho para quando origem é Inventário:**
   - Após o `else` do `K2Node_IfThenElse_8`, você precisa criar:
     - `Get Parent Storage Widget (Destino)` → `IsValid (Destino)` → `Branch (Destino é Storage?)`
   - Use o `K2Node_VariableGet_2` e `K2Node_CallFunction_18` que você já tem, ou crie novos

---

### **PASSO 3: Implementar Funções de Movimento**

#### **3.1: Storage → Inventário**

**Localização:** `K2Node_IfThenElse_8` [TRUE] → `K2Node_IfThenElse_7` [FALSE]

**Após o `else` do `K2Node_IfThenElse_7`:**

1. **Get Parent Storage Widget (Source) - Para obter WBP_Storage:**
   - Use o `K2Node_VariableGet_3` que você já tem
   - Ou arraste o `ReturnValue` (azul) do `GetSourceSlotWidget` → Digite "Get Parent Storage Widget"

2. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
   - Digite "Is Valid?"
   - **Conectar `execute`:** ao `else` (branco) do `K2Node_IfThenElse_7`

3. **Get Game Instance:**
   - Clique com botão direito → Digite "Get Game Instance"
   - **Conectar `execute`:** ao `then` (branco) do `Is Valid? (WBP_Storage)`

4. **Cast to Umbra Game Instance:**
   - Arraste o `ReturnValue` (azul) do `Get Game Instance`
   - Digite "Cast to Umbra Game Instance"
   - **Conectar `execute`:** ao `then` (branco) do `Get Game Instance`

5. **Get Storage ID By Inventory ID:**
   - Arraste o `As Umbra Game Instance` (azul) do `Cast`
   - Digite "Get Storage ID By Inventory ID"
   - **Conectar:**
     - **`Target` (azul):** ao `As Umbra Game Instance` (azul) do `Cast`
     - **`Inventory ID` (int):** ao `InventoryID` (int) do `K2Node_BreakStruct_0`
     - **`execute` (branco):** ao `then` (branco) do `Cast`

6. **Branch (Storage ID > 0?):**
   - Arraste o `ReturnValue` (int, verde) do `Get Storage ID By Inventory ID`
   - Digite ">" (Greater)
   - **Conectar:**
     - **`A` (int):** ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
     - **`B` (int):** Clique com botão direito → "Make Literal Int" → Digite `0`

7. **Branch (Storage ID > 0?):**
   - Arraste o `ReturnValue` (bool, verde) do `>` (Greater)
   - Digite "Branch"
   - **Conectar:**
     - **`Condition` (bool):** ao `ReturnValue` (bool) do `>` (Greater)
     - **`execute` (branco):** ao `then` (branco) do `Get Storage ID By Inventory ID`

8. **Get Slot Index (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Slot Index"
   - **Conectar:**
     - **`Target` (azul):** ao `Self` (azul)
     - **`execute` (branco):** ao `TRUE` (branco) do `Branch (Storage ID > 0?)`

9. **Move Item From Storage:**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Source)` (K2Node_VariableGet_3)
   - Digite "Move Item From Storage"
   - **Conectar:**
     - **`Target` (azul):** ao `ReturnValue` (azul) do `Get Parent Storage Widget (Source)`
     - **`Storage Item ID` (int):** ao `ReturnValue` (int) do `Get Storage ID By Inventory ID`
     - **`Target Slot Index` (int):** ao `ReturnValue` (int) do `Get Slot Index (Destino)`
     - **`execute` (branco):** ao `then` (branco) do `Get Slot Index (Destino)`

10. **Branch (Return Value):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item From Storage`
    - Digite "Branch"
    - **Conectar:**
      - **`Condition` (bool):** ao `ReturnValue` (bool) do `Move Item From Storage`
      - **`execute` (branco):** ao `then` (branco) do `Move Item From Storage`

11. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

#### **3.2: Inventário → Storage**

**Localização:** `K2Node_IfThenElse_8` [FALSE] → Novo `Branch (Destino é Storage?)` [TRUE]

**Após o `else` do `K2Node_IfThenElse_8`:**

1. **Get Parent Storage Widget (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Parent Storage Widget"
   - **Conectar:**
     - **`Target` (azul):** ao `Self` (azul)
     - **`execute` (branco):** ao `else` (branco) do `K2Node_IfThenElse_8`

2. **IsValid (Destino):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`execute` (branco):** ao `then` (branco) do `Get Parent Storage Widget (Destino)`

3. **Branch (Destino é Storage?):**
   - Arraste o `ReturnValue` (bool, verde) do `Is Valid? (Destino)`
   - Digite "Branch"
   - **Conectar:**
     - **`Condition` (bool):** ao `ReturnValue` (bool) do `Is Valid? (Destino)`
     - **`execute` (branco):** ao `then` (branco) do `Is Valid? (Destino)`

4. **Get Parent Storage Widget (Destino) - Para obter WBP_Storage:**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Parent Storage Widget"
   - **Conectar:**
     - **`Target` (azul):** ao `Self` (azul)
     - **`execute` (branco):** ao `TRUE` (branco) do `Branch (Destino é Storage?)`

5. **IsValid (WBP_Storage):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`execute` (branco):** ao `then` (branco) do `Get Parent Storage Widget (Destino)`

6. **Get Slot Index (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Slot Index"
   - **Conectar:**
     - **`Target` (azul):** ao `Self` (azul)
     - **`execute` (branco):** ao `then` (branco) do `Is Valid? (WBP_Storage)`

7. **Move Item To Storage:**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Move Item To Storage"
   - **Conectar:**
     - **`Target` (azul):** ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`Inventory Item ID` (int):** ao `InventoryID` (int) do `K2Node_BreakStruct_0`
     - **`Target Slot Index` (int):** ao `ReturnValue` (int) do `Get Slot Index (Destino)`
     - **`execute` (branco):** ao `then` (branco) do `Get Slot Index (Destino)`

8. **Branch (Return Value):**
    - Arraste o `ReturnValue` (bool, verde) do `Move Item To Storage`
    - Digite "Branch"
    - **Conectar:**
      - **`Condition` (bool):** ao `ReturnValue` (bool) do `Move Item To Storage`
      - **`execute` (branco):** ao `then` (branco) do `Move Item To Storage`

9. **Return Handled / Return Unhandled:**
    - Clique com botão direito → Digite "Return Handled"
    - **Conectar `execute`:** ao `TRUE` (branco) do `Branch (Return Value)`

    - Clique com botão direito → Digite "Return Unhandled"
    - **Conectar `execute`:** ao `FALSE` (branco) do `Branch (Return Value)`

---

#### **3.3: Inventário → Inventário**

**Localização:** `K2Node_IfThenElse_8` [FALSE] → Novo `Branch (Destino é Storage?)` [FALSE]

**Após o `else` do novo `Branch (Destino é Storage?)`:**

1. **Process Item Drop:**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Process Item Drop"
   - **Conectar:**
     - **`Target` (azul):** ao `Self` (azul)
     - **`Dragged Slot Widget` (azul):** ao `ReturnValue` (azul) do `GetSourceSlotWidget` (K2Node_CallFunction_12)
     - **`execute` (branco):** ao `FALSE` (branco) do novo `Branch (Destino é Storage?)`

2. **Return Handled:**
   - Clique com botão direito → Digite "Return Handled"
   - **Conectar `execute`:** ao `then` (branco) do `Process Item Drop`

---

## 📊 **RESUMO VISUAL DAS CONEXÕES:**

### **FLUXO COMPLETO:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓ (then)
IsValid (Cast)
  ↓ (then)
Branch (Is Valid? Cast)
  ├─ TRUE:
  │   └─ Get Source Slot Widget
  │       └─ then → IsValid (Source Slot Widget)
  │           └─ then → Branch (Is Valid? Source)
  │               ├─ TRUE:
  │               │   └─ Get Slot Data (Source)
  │               │       └─ then → Break Umbra Inventory Slot
  │               │           └─ then → Branch (Inventory ID > 0?)
  │               │               ├─ TRUE:
  │               │               │   └─ Get Parent Storage Widget (Source)  ← CONECTAR!
  │               │               │       └─ then → Is Valid? (Origem)  ← CONECTAR!
  │               │               │           └─ then → Branch (Origem é Storage?)  ← CONECTAR!
  │               │               │               ├─ TRUE: (Origem = Storage)
  │               │               │               │   └─ Get Parent Storage Widget (Destino)  ← JÁ TEM!
  │               │               │               │       └─ then → Is Valid? (Destino)  ← JÁ TEM!
  │               │               │               │           └─ then → Branch (Destino é Storage?)  ← JÁ TEM!
  │               │               │               │               ├─ TRUE: (Destino = Storage)
  │               │               │               │               │   └─ Return Unhandled  ← Storage → Storage
  │               │               │               │               │
  │               │               │               │               └─ FALSE: (Destino = Inventário)
  │               │               │               │                   └─ [Storage → Inventário]  ← IMPLEMENTAR!
  │               │               │               │
  │               │               │               └─ FALSE: (Origem = Inventário)
  │               │               │                   └─ Get Parent Storage Widget (Destino)  ← CRIAR!
  │               │               │                       └─ then → Is Valid? (Destino)  ← CRIAR!
  │               │               │                           └─ then → Branch (Destino é Storage?)  ← CRIAR!
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

## ⚠️ **IMPORTANTE: Sobre "Set Parent Storage Widget"**

**O `Set Parent Storage Widget` NÃO é usado no `OnDrop`!**

Ele é usado apenas no `CreateStorageSlots` para **marcar** que o slot pertence ao storage.

No `OnDrop`, você usa **`Get Parent Storage Widget`** para:
- **Verificar** se o slot pertence ao storage
- **Obter** a referência ao `WBP_Storage` para chamar as funções

---

## ✅ **CHECKLIST FINAL:**

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

**Siga este resumo para completar a implementação!** 🚀

