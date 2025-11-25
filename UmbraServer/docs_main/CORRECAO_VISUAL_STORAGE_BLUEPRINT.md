# 🔧 CORREÇÃO VISUAL: Storage Blueprint - Passo a Passo

Este guia mostra **exatamente** como corrigir cada problema no Blueprint, com imagens mentais e conexões detalhadas.

---

## 🎯 **PROBLEMA #1: `CreateStorageSlots` - Slots não aparecem**

### **VERIFICAÇÃO 1.1: Variável `UniformGridPanel_Storage`**

1. **No Designer do `WBP_Storage`:**
   - Verifique se existe um `UniformGridPanel` no Hierarchy
   - Se não existir, **crie um** e nomeie como `UniformGridPanel_Storage`

2. **No painel "Variables" do `WBP_Storage`:**
   - Verifique se existe uma variável chamada `UniformGridPanel_Storage` (tipo: `Uniform Grid Panel`)
   - Se não existir, **crie uma:**
     - Clique em `+ Variable`
     - Nome: `UniformGridPanel_Storage`
     - Tipo: `Uniform Grid Panel`
     - **Compile** o Blueprint

3. **Conectar no Designer:**
   - No Designer, **selecione** o `UniformGridPanel_Storage` (o widget visual)
   - No painel "Details" (lado direito), encontre a seção **"Bindings"** ou **"Variables"**
   - **Arraste** a variável `UniformGridPanel_Storage` do painel "Variables" para o campo correspondente
   - **OU** no Designer, **selecione** o widget e no "Details", procure por **"Variable"** e selecione `UniformGridPanel_Storage`

---

### **VERIFICAÇÃO 1.2: Variável `StorageSlotWidgets`**

1. **No painel "Variables" do `WBP_Storage`:**
   - Verifique se existe uma variável chamada `StorageSlotWidgets` (tipo: `Array of WBP Inventory Slot`)
   - Se não existir, **crie uma:**
     - Clique em `+ Variable`
     - Nome: `StorageSlotWidgets`
     - Tipo: `WBP_InventorySlot`
     - **Mude para Array:** Clique no ícone de "Single" ao lado do tipo e selecione **"Array"**
     - **Compile** o Blueprint

---

### **CORREÇÃO 1.3: Função `CreateStorageSlots`**

**Abra a função `CreateStorageSlots` no `WBP_Storage`:**

#### **NÓ 1: Clear Array**

1. **Arraste** a variável `StorageSlotWidgets` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get StorageSlotWidgets"**
3. **Arraste** do pino azul do `Get StorageSlotWidgets` e procure por **"Clear"**
4. Selecione **"Clear Array"**
5. **Conecte:**
   - **execute:** ao **execute** de entrada da função (`Function Entry`)
   - **Array:** ao **Get StorageSlotWidgets**

#### **NÓ 2: Remove All Children**

1. **Arraste** a variável `UniformGridPanel_Storage` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get UniformGridPanel_Storage"**
3. **Arraste** do pino azul do `Get UniformGridPanel_Storage` e procure por **"Remove All Children"**
4. **Conecte:**
   - **execute:** ao **then** do `Clear Array`
   - **Target:** ao **Get UniformGridPanel_Storage**

#### **NÓ 3: For Loop**

1. **Clique com botão direito** no Event Graph → Digite **"For Loop"**
2. **Configure:**
   - **First Index:** Clique com botão direito no pino `First Index` → **"Promote to Variable"** ou **"Make Literal Int"** → Digite `0`
   - **Last Index:** Clique com botão direito no pino `Last Index` → **"Promote to Variable"** ou **"Make Literal Int"** → Digite `99`
3. **Conecte:**
   - **execute:** ao **then** do `Remove All Children`

#### **NÓ 4: Create Widget (dentro do Loop Body)**

1. **Clique com botão direito** no Event Graph → Digite **"Create Widget"**
2. **Configure:**
   - **Class:** Selecione `WBP_InventorySlot`
   - **Owning Player:** Arraste do pino `Owning Player` e procure por **"Get Owning Player"**
3. **Conecte:**
   - **execute:** ao **Loop Body** do `For Loop`

#### **NÓ 5: Set Parent Storage Widget**

1. **Arraste** do pino azul **`Return Value`** do `Create Widget` e procure por **"Set Parent Storage Widget"**
2. **Configure:**
   - **Parent Storage Widget:** Arraste um nó **"Self"** (clique com botão direito → "Self") para o pin `Parent Storage Widget`
3. **Conecte:**
   - **execute:** ao **then** do `Create Widget`
   - **Target:** ao **Return Value** do `Create Widget`

#### **NÓ 6: Add (Integer + Integer) - Calcular Slot Index do Banco**

1. **Arraste** do pino verde **`Index`** do `For Loop` e procure por **"+"** (Add)
2. **Configure:**
   - **A:** Conecte ao **Index** do `For Loop`
   - **B:** Clique com botão direito no pino `B` → **"Make Literal Int"** → Digite `50`
3. **Resultado:** `Index + 50` (converte 0-99 para 50-149)

#### **NÓ 7: Set Slot Index**

1. **Arraste** do pino azul **`Return Value`** do `Create Widget` e procure por **"Set Slot Index"**
2. **Configure:**
   - **New Slot Index:** Conecte ao **Return Value** do `Add` (NÓ 6)
3. **Conecte:**
   - **execute:** ao **then** do `Set Parent Storage Widget`
   - **Target:** ao **Return Value** do `Create Widget`

#### **NÓ 8: Add to Array**

1. **Arraste** a variável `StorageSlotWidgets` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get StorageSlotWidgets"**
3. **Arraste** do pino azul do `Get StorageSlotWidgets` e procure por **"Add"**
4. Selecione **"Add to Array"**
5. **Configure:**
   - **Item:** Conecte ao **Return Value** do `Create Widget`
6. **Conecte:**
   - **execute:** ao **then** do `Set Slot Index`
   - **Array:** ao **Get StorageSlotWidgets**

#### **NÓ 9: Percent (Integer % Integer) - Calcular Coluna**

1. **Arraste** do pino verde **`Index`** do `For Loop` e procure por **"%"** (Percent)
2. **Configure:**
   - **A:** Conecte ao **Index** do `For Loop`
   - **B:** Clique com botão direito no pino `B` → **"Make Literal Int"** → Digite `10`
3. **Resultado:** `Index % 10` (coluna de 0-9)

#### **NÓ 10: Divide (Integer / Integer) - Calcular Linha**

1. **Arraste** do pino verde **`Index`** do `For Loop` e procure por **"/"** (Divide)
2. **Configure:**
   - **A:** Conecte ao **Index** do `For Loop`
   - **B:** Clique com botão direito no pino `B` → **"Make Literal Int"** → Digite `10`
3. **Resultado:** `Index / 10` (linha de 0-9)

#### **NÓ 11: Add Child to Uniform Grid**

1. **Arraste** a variável `UniformGridPanel_Storage` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get UniformGridPanel_Storage"**
3. **Arraste** do pino azul do `Get UniformGridPanel_Storage` e procure por **"Add Child to Uniform Grid"**
4. **Configure:**
   - **Content:** Conecte ao **Return Value** do `Create Widget`
   - **In Column:** Conecte ao **Return Value** do `Percent` (NÓ 9)
   - **In Row:** Conecte ao **Return Value** do `Divide` (NÓ 10)
5. **Conecte:**
   - **execute:** ao **then** do `Add to Array`
   - **Target:** ao **Get UniformGridPanel_Storage`** ← **CRÍTICO!**

---

## 🎯 **PROBLEMA #2: `OnStorageLoaded_Event` - Itens não atualizam**

### **VERIFICAÇÃO 2.1: Custom Event `OnStorageLoaded_Event`**

1. **No Event Graph do `WBP_Storage`:**
   - Verifique se existe um **Custom Event** chamado `OnStorageLoaded_Event`
   - Se não existir, **crie um:**
     - Clique com botão direito → **"Add Custom Event"**
     - Nome: `OnStorageLoaded_Event`
     - **Compile** o Blueprint

2. **No `Event Construct`:**
   - Verifique se o delegate `OnStorageLoaded` do `MyGameInstance` está conectado ao `OnStorageLoaded_Event`
   - Deve haver um nó **"Assign On Storage Loaded"** conectando o delegate

---

### **CORREÇÃO 2.2: Implementação do `OnStorageLoaded_Event`**

**Abra o `OnStorageLoaded_Event` no Event Graph:**

#### **NÓ 1: Get All Storage Slots**

1. **Arraste** a variável `MyGameInstance` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get MyGameInstance"**
3. **Arraste** do pino azul do `Get MyGameInstance` e procure por **"Get All Storage Slots"**
4. **Conecte:**
   - **execute:** ao **execute** de entrada do `OnStorageLoaded_Event`
   - **Target:** ao **Get MyGameInstance**

#### **NÓ 2: ForEachLoop**

1. **Arraste** do pino azul **`Return Value`** do `Get All Storage Slots` e procure por **"For Each Loop"**
2. **Conecte:**
   - **Array:** ao **Return Value** do `Get All Storage Slots`
   - **execute:** ao **then** do `Get All Storage Slots`

#### **NÓ 3: Break Umbra Inventory Slot**

1. **Arraste** do pino azul **`Array Element`** do `ForEachLoop` e procure por **"Break Umbra Inventory Slot"**
2. **Conecte:**
   - **Umbra Inventory Slot:** ao **Array Element** do `ForEachLoop`
   - **execute:** ao **Loop Body** do `ForEachLoop`

#### **NÓ 4: Get StorageSlotWidgets**

1. **Arraste** a variável `StorageSlotWidgets` do painel "Variables" para o Event Graph
2. Isso criará um nó **"Get StorageSlotWidgets"**

#### **NÓ 5: Get Array Item**

1. **Arraste** do pino azul do `Get StorageSlotWidgets` e procure por **"Get (a copy)"**
2. Selecione **"Get Array Item"**
3. **Configure:**
   - **Dimension 1 (Index):** Conecte ao **Slot Index** do `Break Umbra Inventory Slot` ← **CRÍTICO: Usar Slot Index, não Array Index!**
4. **Conecte:**
   - **Array:** ao **Get StorageSlotWidgets`

#### **NÓ 6: Cast to WBP Inventory Slot**

1. **Arraste** do pino azul **`Output`** (Object) do `Get Array Item` e procure por **"Cast to WBP Inventory Slot"**
2. **Conecte:**
   - **Object:** ao **Output** do `Get Array Item`
   - **execute:** ao **then** do `Break Umbra Inventory Slot`

#### **NÓ 7: Is Valid?**

1. **Arraste** do pino azul **`As WBP Inventory Slot`** do `Cast` e procure por **"Is Valid?"**
2. **Conecte:**
   - **Object:** ao **As WBP Inventory Slot** do `Cast`
   - **execute:** ao **then** do `Cast`

#### **NÓ 8: Branch (Is Valid?)**

1. **Arraste** do pino **`Return Value`** (bool) do `Is Valid?` e procure por **"Branch"**
2. **Conecte:**
   - **Condition:** ao **Return Value** do `Is Valid?`
   - **execute:** ao **then** do `Is Valid?`

#### **NÓ 9: Greater (Integer > Integer) - Verificar Inventory ID**

1. **Arraste** do pino **`Inventory ID`** (int) do `Break Umbra Inventory Slot` e procure por **">"** (Greater)
2. **Configure:**
   - **A:** Conecte ao **Inventory ID** do `Break Umbra Inventory Slot`
   - **B:** Clique com botão direito no pino `B` → **"Make Literal Int"** → Digite `0`
3. **Conecte:**
   - **execute:** ao **TRUE** do `Branch` (NÓ 8)

#### **NÓ 10: Branch (Inventory ID > 0?)**

1. **Arraste** do pino **`Return Value`** (bool) do `Greater` e procure por **"Branch"`
2. **Conecte:**
   - **Condition:** ao **Return Value** do `Greater`
   - **execute:** ao **then** do `Greater`

#### **NÓ 11: Set Slot Data (Caminho TRUE - Slot Ocupado)**

1. **Arraste** do pino azul **`As WBP Inventory Slot`** do `Cast` e procure por **"Set Slot Data"**
2. **Configure:**
   - **New Slot Data:** Conecte ao **Array Element** do `ForEachLoop`
3. **Conecte:**
   - **execute:** ao **TRUE** do `Branch` (NÓ 10)
   - **Target:** ao **As WBP Inventory Slot** do `Cast`

#### **NÓ 12: Update Slot Visual (Caminho TRUE - Slot Ocupado)**

1. **Arraste** do pino azul **`As WBP Inventory Slot`** do `Cast` e procure por **"Update Slot Visual"**
2. **Conecte:**
   - **execute:** ao **then** do `Set Slot Data`
   - **Target:** ao **As WBP Inventory Slot** do `Cast`

#### **NÓ 13: Clear Slot (Caminho FALSE - Slot Vazio)**

1. **Arraste** do pino azul **`As WBP Inventory Slot`** do `Cast` e procure por **"Clear Slot"**
2. **Conecte:**
   - **execute:** ao **FALSE** do `Branch` (NÓ 10)
   - **Target:** ao **As WBP Inventory Slot`** do `Cast`

#### **NÓ 14: Update Slot Visual (Caminho FALSE - Slot Vazio)**

1. **Arraste** do pino azul **`As WBP Inventory Slot`** do `Cast` e procure por **"Update Slot Visual"`
2. **Conecte:**
   - **execute:** ao **then** do `Clear Slot`
   - **Target:** ao **As WBP Inventory Slot`** do `Cast`

#### **NÓ 15: Print String (Erro - Widget Inválido)**

1. **Clique com botão direito** → Digite **"Print String"**
2. **Configure:**
   - **In String:** Digite `"Erro: Widget de slot não é válido no índice {Slot Index}"`
   - **In Color:** Selecione **Red**
3. **Conecte:**
   - **execute:** ao **FALSE** do `Branch` (NÓ 8)

---

## 🎯 **PROBLEMA #3: `UpdateSlotVisual` - Slots brancos/cinza**

### **VERIFICAÇÃO 3.1: Função `UpdateSlotVisual` no `WBP_InventorySlot`**

1. **No `WBP_InventorySlot`:**
   - Vá para **Functions** → **UpdateSlotVisual**
   - Verifique se existe um `Branch` com condição `InventoryID > 0`

2. **No caminho `FALSE` (quando `InventoryID <= 0`):**
   - Verifique se existe um `Set Brush Color` para o widget de fundo do slot
   - Se não existir, **adicione:**

#### **CORREÇÃO 3.2: Adicionar `Set Brush Color` no Caminho FALSE**

1. **Identifique o widget de fundo:**
   - No Designer do `WBP_InventorySlot`, identifique qual widget é o fundo (ex: `Border_Slot`, `Image_Slot`, `BG_Slot`)

2. **No caminho `FALSE` do `Branch` (InventoryID > 0?):**
   - **Arraste** o widget de fundo do Hierarchy para o Event Graph
   - Isso criará um nó **"Get [Nome do Widget]"**
   - **Arraste** do pino azul e procure por **"Set Brush Color"**

3. **Criar `Make Linear Color`:**
   - **Clique com botão direito** → Digite **"Make Linear Color"**
   - **Configure:**
     - **R:** `1.0`
     - **G:** `1.0`
     - **B:** `1.0`
     - **A:** `1.0`

4. **Conectar:**
   - **In Color:** ao **Return Value** do `Make Linear Color`
   - **Target:** ao **Get [Nome do Widget]**
   - **execute:** após os `Set Visibility` (Hidden) no caminho `FALSE`

---

## ✅ **VERIFICAÇÃO FINAL:**

1. **Compile o projeto C++** (após corrigir `GetStorageSlotByIndex`)
2. **Compile os Blueprints** (`WBP_Storage` e `WBP_InventorySlot`)
3. **Execute o jogo** e abra o Storage
4. **Verifique:**
   - ✅ Os 100 slots do storage aparecem
   - ✅ Itens do storage aparecem nos slots corretos
   - ✅ Slots vazios têm fundo branco (não cinza)
   - ✅ Contagem de slots ("Slots: X/100") está correta

---

**Com estas correções, o storage deve funcionar perfeitamente!** 🚀

