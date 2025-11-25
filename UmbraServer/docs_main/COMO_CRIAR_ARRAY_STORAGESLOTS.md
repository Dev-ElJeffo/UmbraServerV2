# 🔧 COMO CRIAR E OBTER O ARRAY StorageSlotWidgets

## ❌ **PROBLEMA:**

Você não consegue encontrar um array do tipo `Array of WBP Inventory Slot` porque **essa variável ainda não foi criada** no seu `WBP_Storage`.

**⚠️ IMPORTANTE:** A variável `StorageSlots` que você vê no C++ é do tipo `Array of FUmbraInventorySlot` (structs/dados), não widgets! Você precisa criar uma variável Blueprint separada com um nome diferente.

---

## ✅ **SOLUÇÃO:**

Você precisa **criar uma variável** no `WBP_Storage` para armazenar os widgets dos slots.

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Criar a Variável `StorageSlotWidgets`**

**⚠️ IMPORTANTE:** Use um nome diferente de `StorageSlots` porque já existe uma variável C++ com esse nome (mas ela é de structs, não widgets)!

1. **Abra** o `WBP_Storage` no Unreal Engine
2. **Vá para** a aba **"Variables"** (no painel esquerdo, ao lado de "Graph" e "Designer")
3. **Clique** no botão **"+ Variable"** (ou pressione `Ctrl + K`)
4. **Configure a variável:**
   - **Nome:** `StorageSlotWidgets` (ou `SlotWidgets`, ou `StorageWidgets`)
   - **Tipo:** Clique em **"Variable Type"** → **"Array"** → **"WBP Inventory Slot"**
     - Se não aparecer "WBP Inventory Slot", digite no campo de busca
   - **Editable:** ✅ (marcado)
   - **Instance Editable:** ✅ (marcado)
   - **Expose on Spawn:** ❌ (desmarcado)
   - **Private:** ❌ (desmarcado - deve ser público)
5. **Pressione Enter** para confirmar

**Resultado:** Você terá uma variável `StorageSlotWidgets` do tipo `Array of WBP Inventory Slot`

**Sugestões de nomes:**
- `StorageSlotWidgets` (recomendado)
- `SlotWidgets`
- `StorageWidgets`

---

### **PASSO 2: Preencher o Array na Função `CreateStorageSlots`**

**No `WBP_Storage` → Functions → CreateStorageSlots:**

Certifique-se de que a função `CreateStorageSlots` adiciona os widgets ao array:

```
CreateStorageSlots (Function)
  ↓
Clear Array (StorageSlotWidgets)  ← Limpa o array (use o nome que você escolheu!)
  ↓
Remove All Children (UniformGridPanel_Storage)
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Create Widget (WBP_InventorySlot)
    │     └─ Owning Player: Get Owning Player
    ├─ Set Slot Index
    │     └─ Target: Created Widget
    │     └─ New Slot Index: Index + 50
    ├─ Set Parent Storage Widget
    │     └─ Target: Created Widget
    │     └─ Parent Storage Widget: self
    ├─ Add to Array (StorageSlotWidgets)  ← ADICIONA O WIDGET AO ARRAY!
    │     └─ Array: StorageSlotWidgets (use o nome que você escolheu!)
    │     └─ Item: Created Widget
    └─ Add Child to Uniform Grid (UniformGridPanel_Storage)
          └─ Content: Created Widget
          └─ Column: Index % 10
          └─ Row: Index / 10
```

**Como adicionar o nó "Add to Array":**

1. **No `Loop Body` do `For Loop`**, após `Set Parent Storage Widget`
2. **Clique com botão direito** → **"Add to Array"**
3. **Configure:**
   - **Array:** Arraste a variável `StorageSlots` do painel **"Variables"** para o pin `Array`
   - **Item:** Conecte ao **Return Value** do `Create Widget`
4. **Conecte o fluxo:**
   - **execute:** ao **then** de `Set Parent Storage Widget`
   - **then:** ao **execute** de `Add Child to Uniform Grid`

---

### **PASSO 3: Obter o Array no `OnStorageLoaded_Event`**

**Agora você pode usar o array `StorageSlotWidgets` no `OnStorageLoaded_Event`:**

1. **No `OnStorageLoaded_Event`**, após o `ForEachLoop`
2. **Arraste** a variável `StorageSlotWidgets` do painel **"Variables"** para o gráfico
3. **Isso criará** um nó **"Get StorageSlotWidgets"** (ou **"StorageSlotWidgets"**)
4. **Use este nó** como o `Array` do `Get Array Item`

**Estrutura completa:**

```
OnStorageLoaded_Event
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
ForEachLoop (GetAllStorageSlots result)
  Loop Body:
    ├─ Get Array Element  ← Struct (dados)
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index
    │
    ├─ Get StorageSlotWidgets  ← ARRASTE A VARIÁVEL DO PAINEL!
    │     └─ (retorna Array of WBP Inventory Slot)
    │
    ├─ Get Array Item (StorageSlotWidgets, Index: Slot Index)
    │     └─ Array: Get StorageSlotWidgets (do passo acima)
    │     └─ Dimension 1: Slot Index (do Break)
    │     └─ Output: Object
    │
    └─ Cast to WBP Inventory Slot
          └─ Object: Output (do Get Array Item)
          └─ then:
              ├─ Set Slot Data
              └─ Update Slot Visual
```

---

## 🎯 **COMO OBTER O NÓ "Get StorageSlots":**

### **Método 1: Arrastar do Painel Variables**

1. **Abra** o `WBP_Storage`
2. **Vá para** a aba **"Variables"** (painel esquerdo)
3. **Encontre** a variável `StorageSlotWidgets` (ou o nome que você escolheu)
4. **Arraste** ela para o **Event Graph**
5. **Isso criará** automaticamente um nó **"Get StorageSlotWidgets"**

### **Método 2: Criar Manualmente**

1. **No Event Graph**, **clique com botão direito**
2. **Digite** "Get StorageSlotWidgets" ou "StorageSlotWidgets" (ou o nome que você escolheu)
3. **Selecione** a variável da lista
4. **Isso criará** o nó **"Get StorageSlotWidgets"**

---

## ⚠️ **IMPORTANTE:**

### **Diferença entre os três arrays:**

1. **`GetAllStorageSlots()`** (do GameInstance):
   - Tipo: `Array of FUmbraInventorySlot` (structs/dados)
   - Retorna os **dados** dos itens do storage
   - Use `ForEachLoop` para iterar

2. **`StorageSlots`** (variável C++ do UmbraStorageWidget):
   - Tipo: `Array of FUmbraInventorySlot` (structs/dados)
   - Contém os **dados** dos itens (vem do C++)
   - **NÃO use para widgets!**

3. **`StorageSlotWidgets`** (variável Blueprint que você cria):
   - Tipo: `Array of WBP Inventory Slot` (widgets)
   - Contém os **widgets visuais** dos slots
   - Criado por `CreateStorageSlots`
   - Use `Get Array Item` com `SlotIndex` para obter o widget

### **Fluxo completo:**

```
1. CreateStorageSlots cria 100 widgets e adiciona ao array StorageSlotWidgets (Blueprint)
2. GetAllStorageSlots retorna os dados do servidor (structs)
3. OnStorageLoaded_Event itera sobre os dados (structs)
4. Para cada struct, usa SlotIndex para obter o widget correspondente do StorageSlotWidgets
5. Atualiza o widget com os dados da struct
```

---

## 📋 **VERIFICAÇÃO:**

Após criar a variável `StorageSlotWidgets`:

1. **No Event Graph**, **clique com botão direito**
2. **Digite** "StorageSlotWidgets" (ou o nome que você escolheu)
3. **Você deve ver** a variável na lista
4. **Selecione** ela para criar o nó **"Get StorageSlotWidgets"**

**Se não aparecer:**
- Verifique se a variável foi criada corretamente
- Verifique se o tipo está correto: `Array of WBP Inventory Slot`
- Compile o Blueprint (pressione `F7`)

---

## 🎯 **RESUMO:**

1. **Crie** a variável `StorageSlotWidgets` (tipo: `Array of WBP Inventory Slot`) - **use um nome diferente de `StorageSlots`!**
2. **Preencha** o array na função `CreateStorageSlots` usando `Add to Array`
3. **Use** `Get StorageSlotWidgets` no `OnStorageLoaded_Event` para obter o array
4. **Use** `Get Array Item (StorageSlotWidgets, Index: Slot Index)` para obter o widget
5. **Faça** `Cast to WBP Inventory Slot` para confirmar o tipo

**Agora você pode usar o array `StorageSlotWidgets` em qualquer lugar do `WBP_Storage`!**

**⚠️ LEMBRE-SE:**
- **`StorageSlots`** (C++) = Dados (structs) - não use para widgets!
- **`StorageSlotWidgets`** (Blueprint) = Widgets - use esta para acessar os widgets!

