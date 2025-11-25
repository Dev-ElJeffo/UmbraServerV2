# 📘 GUIA DETALHADO: OnDrop - Passo a Passo Completo

## 🎯 **OBJETIVO:**

Este guia mostra **EXATAMENTE** como implementar o `OnDrop` no `WBP_InventorySlot`, incluindo:
- **Onde obter cada nó**
- **De onde vem cada conexão**
- **Para onde vai cada conexão**
- **O que conectar em cada pino**

---

## 📋 **PRÉ-REQUISITOS:**

✅ C++ compilado  
✅ `WBP_InventorySlot` aberto no Unreal Engine  
✅ Variável `ParentStorageWidget` existe no `WBP_InventorySlot`  

---

## 🔧 **PARTE 1: Estrutura Base do OnDrop**

### **PASSO 1.1: Localizar ou Criar o OnDrop**

1. **Abra o `WBP_InventorySlot`** no Unreal Engine
2. **Vá para o Event Graph** (aba "Graph")
3. **No painel direito, procure por "Override Functions"**
4. **Procure por `OnDrop`** na lista
5. **Se existir, clique nele** para abrir
6. **Se NÃO existir:**
   - Clique com botão direito no Event Graph
   - Digite "OnDrop"
   - Selecione **"Add Override"** → **"OnDrop"**

### **PASSO 1.2: Estrutura Inicial**

O `OnDrop` já deve ter estes inputs:
- **MyGeometry** (Geometry)
- **PointerEvent** (Pointer Event)
- **Operation** (Drag Drop Operation)

---

## 🔧 **PARTE 2: Cast para Umbra Item Drag Drop Operation**

### **PASSO 2.1: Obter o Nó "Cast to Umbra Item Drag Drop Operation"**

1. **Arraste o pino `Operation`** (azul) do `OnDrop`
2. **Solte no espaço vazio** do Event Graph
3. **No menu que aparece, digite "Cast to Umbra Item Drag Drop Operation"**
4. **Selecione** o nó que aparece

### **PASSO 2.2: Conectar o Cast**

**Conexões:**
- **`Object` (pino azul de entrada do Cast):** Conecte ao **`Operation`** (pino azul de saída do `OnDrop`)
- **`execute` (pino branco de entrada do Cast):** Conecte ao **`execute`** (pino branco de saída do `OnDrop`)

**Resultado:**
```
OnDrop
  ├─ Operation (azul) → Cast to Umbra Item Drag Drop Operation → Object (azul)
  └─ execute (branco) → Cast to Umbra Item Drag Drop Operation → execute (branco)
```

---

## 🔧 **PARTE 3: Get Source Slot Widget**

### **PASSO 3.1: Obter o Nó "Get Source Slot Widget"**

1. **Arraste o pino `As Umbra Item Drag Drop Operation`** (azul) do `Cast`
2. **Solte no espaço vazio**
3. **No menu, digite "Get Source Slot Widget"**
4. **Selecione** o nó

### **PASSO 3.2: Conectar o Get Source Slot Widget**

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`As Umbra Item Drag Drop Operation`** (pino azul de saída do `Cast`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Cast` - caminho de sucesso)

**Resultado:**
```
Cast to Umbra Item Drag Drop Operation
  ├─ then (branco) → Get Source Slot Widget → execute (branco)
  └─ As Umbra Item Drag Drop Operation (azul) → Get Source Slot Widget → Target (azul)
```

---

## 🔧 **PARTE 4: Is Valid? (Source Slot Widget)**

### **PASSO 4.1: Obter o Nó "Is Valid?"**

1. **Arraste o pino `Return Value`** (azul) do `Get Source Slot Widget`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

### **PASSO 4.2: Conectar o Is Valid?**

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Source Slot Widget`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Source Slot Widget`)

**Resultado:**
```
Get Source Slot Widget
  ├─ then (branco) → Is Valid? → execute (branco)
  └─ Return Value (azul) → Is Valid? → Object (azul)
```

---

## 🔧 **PARTE 5: Branch (Is Valid?)**

### **PASSO 5.1: Obter o Nó "Branch"**

1. **Arraste o pino `Return Value`** (bool, verde) do `Is Valid?`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

### **PASSO 5.2: Conectar o Branch**

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Is Valid?`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid?`)

**Resultado:**
```
Is Valid?
  ├─ then (branco) → Branch → execute (branco)
  └─ Return Value (bool) → Branch → Condition (bool)
```

---

## 🔧 **PARTE 6: Obter Dados do Slot de Origem (Source)**

### **PASSO 6.1: Get Slot Data (Source)**

1. **Arraste o pino `Return Value`** (azul) do `Get Source Slot Widget`
2. **Solte no espaço vazio**
3. **No menu, digite "Get Slot Data"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Source Slot Widget`)
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch`)

### **PASSO 6.2: Break Umbra Inventory Slot (Source)**

1. **Arraste o pino `Return Value`** (azul) do `Get Slot Data`
2. **Solte no espaço vazio**
3. **No menu, digite "Break Umbra Inventory Slot"**
4. **Selecione** o nó

**Conexões:**
- **`Umbra Inventory Slot` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Slot Data`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Slot Data`)

**Resultado:**
```
Branch (TRUE)
  └─ execute (branco) → Get Slot Data → execute (branco)
      └─ Return Value (azul) → Break Umbra Inventory Slot → Umbra Inventory Slot (azul)
```

---

## 🔧 **PARTE 7: Verificar se há Item (Inventory ID > 0)**

### **PASSO 7.1: Branch (Inventory ID > 0)**

1. **Arraste o pino `Inventory ID`** (int, verde) do `Break Umbra Inventory Slot`
2. **Solte no espaço vazio**
3. **No menu, digite ">" (Greater)**
4. **Selecione** o nó **"> (Integer > Integer)"**

**Conexões:**
- **`A` (pino int de entrada):** Conecte ao **`Inventory ID`** (pino int de saída do `Break Umbra Inventory Slot`)
- **`B` (pino int de entrada):** Clique com botão direito → **"Make Literal Int"** → Digite `0`

5. **Arraste o pino `Return Value`** (bool, verde) do **">"**
6. **Solte no espaço vazio**
7. **No menu, digite "Branch"**
8. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do **">"**)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Break Umbra Inventory Slot`)

**Resultado:**
```
Break Umbra Inventory Slot
  ├─ then (branco) → Branch (Inventory ID > 0) → execute (branco)
  └─ Inventory ID (int) → > (Integer > Integer) → A (int)
      └─ Return Value (bool) → Branch → Condition (bool)
```

---

## 🔧 **PARTE 8: Verificar Origem (Storage ou Inventário)**

### **PASSO 8.1: Get Parent Storage Widget (Source)**

1. **Arraste o pino `Return Value`** (azul) do `Get Source Slot Widget`
2. **Solte no espaço vazio**
3. **No menu, digite "Get Parent Storage Widget"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Source Slot Widget`)
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch (Inventory ID > 0)`)

### **PASSO 8.2: Is Valid? (Parent Storage Widget da Origem)**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Source)`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Source)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Parent Storage Widget (Source)`)

### **PASSO 8.3: Branch (Origem é Storage?)**

1. **Arraste o pino `Return Value`** (bool, verde) do `Is Valid? (Parent Storage Widget da Origem)`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Is Valid? (Parent Storage Widget da Origem)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid? (Parent Storage Widget da Origem)`)

**Resultado:**
```
Branch (Inventory ID > 0) [TRUE]
  └─ execute (branco) → Get Parent Storage Widget (Source) → execute (branco)
      └─ Return Value (azul) → Is Valid? → Object (azul)
          └─ Return Value (bool) → Branch (Origem é Storage?) → Condition (bool)
```

---

## 🔧 **PARTE 9: CENÁRIO 1 - Storage → Inventário**

### **PASSO 9.1: Verificar Destino (Inventário)**

1. **Arraste um nó "Self"** (clique com botão direito → "Self")
2. **Arraste o pino azul do `Self`**
3. **Solte no espaço vazio**
4. **No menu, digite "Get Parent Storage Widget"**
5. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Origem é Storage?)`)

**⚠️ IMPORTANTE:** Este `FALSE` significa que a origem **NÃO é Storage**, então é Inventário. Mas estamos verificando o **destino** agora.

### **PASSO 9.2: Is Valid? (Parent Storage Widget do Destino)**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Parent Storage Widget (Destino)`)

### **PASSO 9.3: Branch (Destino é Storage?)**

1. **Arraste o pino `Return Value`** (bool, verde) do `Is Valid? (Parent Storage Widget do Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Is Valid? (Parent Storage Widget do Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid? (Parent Storage Widget do Destino)`)

**⚠️ LÓGICA:**
- Se `Is Valid?` retorna `FALSE` → Destino é **Inventário** (não tem ParentStorageWidget)
- Se `Is Valid?` retorna `TRUE` → Destino é **Storage** (tem ParentStorageWidget)

### **PASSO 9.4: Get Parent Storage Widget (Source) - Para Obter WBP_Storage**

1. **Arraste o pino `Return Value`** (azul) do `Get Source Slot Widget`
2. **Solte no espaço vazio**
3. **No menu, digite "Get Parent Storage Widget"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Source Slot Widget`)
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Destino é Storage?)`)

**⚠️ IMPORTANTE:** Este `FALSE` significa que o destino **NÃO é Storage**, então é **Inventário**. Estamos no caminho **Storage → Inventário**.

### **PASSO 9.5: Is Valid? (WBP_Storage)**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Source)`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Source)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Parent Storage Widget (Source)`)

### **PASSO 9.6: Get Game Instance**

1. **Clique com botão direito** no Event Graph
2. **Digite "Get Game Instance"**
3. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid? (WBP_Storage)`)

### **PASSO 9.7: Cast to Umbra Game Instance**

1. **Arraste o pino `Return Value`** (azul) do `Get Game Instance`
2. **Solte no espaço vazio**
3. **No menu, digite "Cast to Umbra Game Instance"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Game Instance`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Game Instance`)

### **PASSO 9.8: Get Storage ID By Inventory ID**

1. **Arraste o pino `As Umbra Game Instance`** (azul) do `Cast`
2. **Solte no espaço vazio**
3. **No menu, digite "Get Storage ID By Inventory ID"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`As Umbra Game Instance`** (pino azul de saída do `Cast`)
- **`Inventory ID` (pino int de entrada):** Conecte ao **`Inventory ID`** (pino int de saída do `Break Umbra Inventory Slot (Source)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Cast`)

### **PASSO 9.9: Branch (Storage ID > 0?)**

1. **Arraste o pino `Return Value`** (int, verde) do `Get Storage ID By Inventory ID`
2. **Solte no espaço vazio**
3. **No menu, digite ">" (Greater)**
4. **Selecione** o nó **"> (Integer > Integer)"**

**Conexões:**
- **`A` (pino int de entrada):** Conecte ao **`Return Value`** (pino int de saída do `Get Storage ID By Inventory ID`)
- **`B` (pino int de entrada):** Clique com botão direito → **"Make Literal Int"** → Digite `0`

5. **Arraste o pino `Return Value`** (bool, verde) do **">"**
6. **Solte no espaço vazio**
7. **No menu, digite "Branch"**
8. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do **">"**)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Storage ID By Inventory ID`)

### **PASSO 9.10: Get Slot Index (Destino)**

1. **Arraste um nó "Self"** (clique com botão direito → "Self")
2. **Arraste o pino azul do `Self`**
3. **Solte no espaço vazio**
4. **No menu, digite "Get Slot Index"**
5. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch (Storage ID > 0?)`)

### **PASSO 9.11: Move Item From Storage**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Source)` (o que obtém o WBP_Storage)
2. **Solte no espaço vazio**
3. **No menu, digite "Move Item From Storage"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Source)`)
- **`Storage Item ID` (pino int de entrada):** Conecte ao **`Return Value`** (pino int de saída do `Get Storage ID By Inventory ID`)
- **`Target Slot Index` (pino int de entrada):** Conecte ao **`Return Value`** (pino int de saída do `Get Slot Index (Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Slot Index (Destino)`)

### **PASSO 9.12: Branch (Return Value do Move Item From Storage)**

1. **Arraste o pino `Return Value`** (bool, verde) do `Move Item From Storage`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Move Item From Storage`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Move Item From Storage`)

### **PASSO 9.13: Return Handled / Return Unhandled**

1. **Clique com botão direito** no Event Graph
2. **Digite "Return Handled"**
3. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch (Return Value do Move Item From Storage)`)

4. **Clique com botão direito** no Event Graph
5. **Digite "Return Unhandled"**
6. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Return Value do Move Item From Storage)`)

---

## 🔧 **PARTE 10: CENÁRIO 2 - Inventário → Storage**

### **PASSO 10.1: Verificar Destino (Storage)**

**⚠️ IMPORTANTE:** Este caminho começa no **`FALSE`** do `Branch (Origem é Storage?)`, que significa que a origem **NÃO é Storage**, então é **Inventário**.

1. **Arraste um nó "Self"** (clique com botão direito → "Self")
2. **Arraste o pino azul do `Self`**
3. **Solte no espaço vazio**
4. **No menu, digite "Get Parent Storage Widget"**
5. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Origem é Storage?)`)

### **PASSO 10.2: Is Valid? (Parent Storage Widget do Destino)**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Parent Storage Widget (Destino)`)

### **PASSO 10.3: Branch (Destino é Storage?)**

1. **Arraste o pino `Return Value`** (bool, verde) do `Is Valid? (Parent Storage Widget do Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Is Valid? (Parent Storage Widget do Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid? (Parent Storage Widget do Destino)`)

**⚠️ LÓGICA:**
- Se `Is Valid?` retorna `TRUE` → Destino é **Storage** (tem ParentStorageWidget)
- Se `Is Valid?` retorna `FALSE` → Destino é **Inventário** (não tem ParentStorageWidget)

### **PASSO 10.4: Get Parent Storage Widget (Destino) - Para Obter WBP_Storage**

1. **Arraste um nó "Self"** (clique com botão direito → "Self")
2. **Arraste o pino azul do `Self`**
3. **Solte no espaço vazio**
4. **No menu, digite "Get Parent Storage Widget"**
5. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch (Destino é Storage?)`)

**⚠️ IMPORTANTE:** Este `TRUE` significa que o destino **É Storage**. Estamos no caminho **Inventário → Storage**.

### **PASSO 10.5: Is Valid? (WBP_Storage)**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Is Valid?"**
4. **Selecione** o nó

**Conexões:**
- **`Object` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Parent Storage Widget (Destino)`)

### **PASSO 10.6: Get Slot Index (Destino)**

1. **Arraste um nó "Self"** (clique com botão direito → "Self")
2. **Arraste o pino azul do `Self`**
3. **Solte no espaço vazio**
4. **No menu, digite "Get Slot Index"**
5. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Is Valid? (WBP_Storage)`)

### **PASSO 10.7: Move Item To Storage**

1. **Arraste o pino `Return Value`** (azul) do `Get Parent Storage Widget (Destino)`
2. **Solte no espaço vazio**
3. **No menu, digite "Move Item To Storage"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Parent Storage Widget (Destino)`)
- **`Inventory Item ID` (pino int de entrada):** Conecte ao **`Inventory ID`** (pino int de saída do `Break Umbra Inventory Slot (Source)`)
- **`Target Slot Index` (pino int de entrada):** Conecte ao **`Return Value`** (pino int de saída do `Get Slot Index (Destino)`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Get Slot Index (Destino)`)

### **PASSO 10.8: Branch (Return Value do Move Item To Storage)**

1. **Arraste o pino `Return Value`** (bool, verde) do `Move Item To Storage`
2. **Solte no espaço vazio**
3. **No menu, digite "Branch"**
4. **Selecione** o nó

**Conexões:**
- **`Condition` (pino bool de entrada):** Conecte ao **`Return Value`** (pino bool de saída do `Move Item To Storage`)
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Move Item To Storage`)

### **PASSO 10.9: Return Handled / Return Unhandled**

1. **Clique com botão direito** no Event Graph
2. **Digite "Return Handled"**
3. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`TRUE`** (pino branco de saída do `Branch (Return Value do Move Item To Storage)`)

4. **Clique com botão direito** no Event Graph
5. **Digite "Return Unhandled"**
6. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Return Value do Move Item To Storage)`)

---

## 🔧 **PARTE 11: CENÁRIO 3 - Inventário → Inventário**

### **PASSO 11.1: Process Item Drop**

**⚠️ IMPORTANTE:** Este caminho começa no **`FALSE`** do `Branch (Destino é Storage?)`, que significa que o destino **NÃO é Storage**, então é **Inventário**.

1. **Arraste o pino `Return Value`** (azul) do `Get Source Slot Widget`
2. **Solte no espaço vazio**
3. **No menu, digite "Process Item Drop"**
4. **Selecione** o nó

**Conexões:**
- **`Target` (pino azul de entrada):** Conecte ao **`Self`** (pino azul)
- **`Dragged Slot Widget` (pino azul de entrada):** Conecte ao **`Return Value`** (pino azul de saída do `Get Source Slot Widget`)
- **`execute` (pino branco de entrada):** Conecte ao **`FALSE`** (pino branco de saída do `Branch (Destino é Storage?)`)

### **PASSO 11.2: Return Handled**

1. **Clique com botão direito** no Event Graph
2. **Digite "Return Handled"**
3. **Selecione** o nó

**Conexões:**
- **`execute` (pino branco de entrada):** Conecte ao **`then`** (pino branco de saída do `Process Item Drop`)

---

## 📊 **RESUMO DAS CONEXÕES:**

### **Fluxo Principal:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → Get Source Slot Widget
  │         ├─ then → Is Valid? (Source Slot Widget)
  │         │         ├─ then → Branch (Is Valid?)
  │         │         │         ├─ TRUE → Get Slot Data (Source)
  │         │         │         │         └─ then → Break Umbra Inventory Slot (Source)
  │         │         │         │                   └─ then → Branch (Inventory ID > 0)
  │         │         │         │                             ├─ TRUE → Get Parent Storage Widget (Source)
  │         │         │         │                             │         └─ then → Is Valid? (Parent Storage Widget da Origem)
  │         │         │         │                             │                   └─ then → Branch (Origem é Storage?)
  │         │         │         │                             │                             ├─ TRUE → [Storage → Inventário]
  │         │         │         │                             │                             └─ FALSE → [Inventário → Storage/Inventário]
  │         │         │         │                             └─ FALSE → Return Unhandled
  │         │         │         └─ FALSE → Return Unhandled
  │         │         └─ CastFailed → Return Unhandled
```

---

## ✅ **VERIFICAÇÃO FINAL:**

Após implementar, verifique:

1. **Todos os nós estão conectados corretamente**
2. **Não há nós desconectados**
3. **Os caminhos `TRUE` e `FALSE` dos Branches estão corretos**
4. **Todos os `Return Handled` e `Return Unhandled` estão conectados**

---

**Com este guia detalhado, você deve conseguir implementar o OnDrop completamente!** 🚀
