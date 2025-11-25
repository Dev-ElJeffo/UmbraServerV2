# 🔍 ANÁLISE COMPLETA: Problemas do Storage

## 📋 **PROBLEMAS IDENTIFICADOS:**

1. **Apenas slots de inventário aparecem** (não os slots de storage)
2. **Slots têm fundo branco** (incorreto)
3. **Itens não são atualizados no storage** (mesmo com API retornando item)

---

## 🔴 **PROBLEMA CRÍTICO #1: Bug no `GetStorageSlotByIndex` (C++)**

### **O PROBLEMA:**

O `GetStorageSlotByIndex` no `UmbraGameInstance.cpp` está procurando por slots com `SlotIndex == SlotIndex` (0-99), mas os slots no `CurrentStorage` vêm do banco de dados com `SlotIndex` de **50-149**!

**Código atual (ERRADO):**
```cpp
bool UUmbraGameInstance::GetStorageSlotByIndex(int32 SlotIndex, FUmbraInventorySlot& OutSlot) const
{
    // SlotIndex aqui é 0-99 (índice do storage)
    for (const FUmbraInventorySlot& Slot : CurrentStorage)
    {
        if (Slot.SlotIndex == SlotIndex && Slot.InventoryID > 0)  // ❌ ERRADO! Slot.SlotIndex é 50-149!
        {
            OutSlot = Slot;
            return true;
        }
    }
    // ...
}
```

### **A SOLUÇÃO:**

O `GetStorageSlotByIndex` precisa converter o índice do storage (0-99) para o índice do banco (50-149) antes de procurar:

```cpp
bool UUmbraGameInstance::GetStorageSlotByIndex(int32 SlotIndex, FUmbraInventorySlot& OutSlot) const
{
    // Validar índice (0-99 para storage)
    if (SlotIndex < 0 || SlotIndex >= 100)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] GetStorageSlotByIndex - Índice inválido: %d (deve ser 0-99)"), SlotIndex);
        OutSlot = FUmbraInventorySlot();
        OutSlot.SlotIndex = SlotIndex; // Manter o índice do storage (0-99)
        return false;
    }

    // ✅ CORREÇÃO: Converter índice do storage (0-99) para índice do banco (50-149)
    int32 DatabaseSlotIndex = SlotIndex + 50;

    // Buscar slot no storage
    for (const FUmbraInventorySlot& Slot : CurrentStorage)
    {
        if (Slot.SlotIndex == DatabaseSlotIndex && Slot.InventoryID > 0)  // ✅ CORRETO!
        {
            OutSlot = Slot;
            OutSlot.SlotIndex = SlotIndex; // ✅ Retornar o índice do storage (0-99), não do banco!
            return true;
        }
    }

    // Slot vazio
    OutSlot = FUmbraInventorySlot();
    OutSlot.SlotIndex = SlotIndex; // ✅ Manter o índice do storage (0-99)
    return false;
}
```

**E também corrigir `GetAllStorageSlots` para retornar o índice correto:**

```cpp
TArray<FUmbraInventorySlot> UUmbraGameInstance::GetAllStorageSlots() const
{
    TArray<FUmbraInventorySlot> AllSlots;
    AllSlots.Reserve(100); // Capacidade fixa de 100 slots

    for (int32 i = 0; i < 100; i++)
    {
        FUmbraInventorySlot FoundSlot;
        if (GetStorageSlotByIndex(i, FoundSlot) && FoundSlot.InventoryID > 0)
        {
            // ✅ FoundSlot.SlotIndex já está correto (0-99) após a correção acima
            AllSlots.Add(FoundSlot); // Slot ocupado
        }
        else
        {
            // Slot vazio
            FUmbraInventorySlot EmptySlot;
            EmptySlot.SlotIndex = i; // ✅ 0-99
            EmptySlot.InventoryID = 0;
            EmptySlot.ItemTemplateID = 0;
            EmptySlot.Quantity = 0;
            AllSlots.Add(EmptySlot);
        }
    }
    return AllSlots;
}
```

---

## 🔴 **PROBLEMA #2: `CreateStorageSlots` não está criando os slots corretamente**

### **VERIFICAÇÕES NECESSÁRIAS:**

1. **A variável `StorageSlotWidgets` existe?**
   - Tipo: `Array of WBP Inventory Slot`
   - Deve estar no painel "Variables" do `WBP_Storage`

2. **O `UniformGridPanel_Storage` está conectado?**
   - No Designer, o `UniformGridPanel_Storage` deve estar criado
   - A variável `UniformGridPanel_Storage` (tipo `Uniform Grid Panel`) deve estar conectada ao widget visual

3. **O `CreateStorageSlots` está sendo chamado?**
   - Deve ser chamado no `Event Construct` do `WBP_Storage`

### **ESTRUTURA CORRETA DO `CreateStorageSlots`:**

```
CreateStorageSlots (Function)
  ↓
Clear Array (StorageSlotWidgets)
  ↓
Remove All Children (Get UniformGridPanel_Storage)  ← CRÍTICO: Target deve ser o UniformGridPanel!
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Create Widget (Class: WBP_InventorySlot, Owning Player: Get Owning Player)
    │     └─ Return Value (WBP_InventorySlot)
    │
    ├─ Set Parent Storage Widget (Target: Return Value)
    │     └─ Parent Storage Widget: self (WBP_Storage)
    │
    ├─ Add (Integer + Integer)
    │     ├─ A: Index (do For Loop)  ← 0-99
    │     └─ B: 50 (Make Literal Int)
    │     └─ Return Value (New Slot Index)  ← 50-149 (para o banco)
    │
    ├─ Set Slot Index (Target: Return Value do Create Widget)
    │     └─ New Slot Index: Return Value (do Add)  ← 50-149
    │
    ├─ Add to Array (Target: Get StorageSlotWidgets)
    │     └─ Item: Return Value (do Create Widget)
    │
    ├─ Percent (Integer % Integer)
    │     ├─ A: Index (do For Loop)
    │     └─ B: 10 (Make Literal Int)
    │     └─ Return Value (Column)
    │
    ├─ Divide (Integer / Integer)
    │     ├─ A: Index (do For Loop)
    │     └─ B: 10 (Make Literal Int)
    │     └─ Return Value (Row)
    │
    └─ Add Child to Uniform Grid (Target: Get UniformGridPanel_Storage)  ← CRÍTICO!
          ├─ Content: Return Value (do Create Widget)
          ├─ In Column: Return Value (do Percent)
          └─ In Row: Return Value (do Divide)
```

### **VERIFICAÇÕES ESPECÍFICAS:**

1. **`Target` do `Add Child to Uniform Grid`:**
   - Deve ser `Get UniformGridPanel_Storage` (arraste a variável do painel "Variables")
   - **NÃO** deve ser `self` ou qualquer outro widget

2. **`Set Slot Index`:**
   - Deve usar `Index + 50` (para converter 0-99 para 50-149)
   - Isso é o índice que será usado no banco de dados

3. **`Add to Array`:**
   - Deve adicionar o `Return Value` do `Create Widget` ao array `StorageSlotWidgets`
   - Isso permite acessar os widgets depois no `OnStorageLoaded_Event`

---

## 🔴 **PROBLEMA #3: `OnStorageLoaded_Event` não está atualizando os slots**

### **ESTRUTURA CORRETA DO `OnStorageLoaded_Event`:**

```
OnStorageLoaded_Event (Custom Event)
  ↓
Get All Storage Slots (Target: Get MyGameInstance)  ← Retorna Array de FUmbraInventorySlot (100 elementos, 0-99)
  ↓
ForEachLoop (Array: Return Value do Get All Storage Slots)
  Loop Body:
    ├─ Break Umbra Inventory Slot (Umbra Inventory Slot: Array Element)
    │     └─ Slot Index (0-99), Inventory ID
    │
    ├─ Get StorageSlotWidgets  ← Arraste a variável do painel "Variables"
    │
    ├─ Get Array Item (Target: Get StorageSlotWidgets, Index: Slot Index)  ← CRÍTICO: Usar Slot Index (0-99)!
    │     └─ Output (Object)
    │
    ├─ Cast to WBP Inventory Slot (Object: Output do Get Array Item)
    │     └─ As WBP Inventory Slot
    │
    ├─ Is Valid? (Object: As WBP Inventory Slot)
    │
    └─ Branch (Condition: Return Value do Is Valid?)
        ├─ TRUE:
        │   ├─ Branch (Condition: Inventory ID > 0)  ← Verifica se o slot tem item
        │   │   ├─ TRUE:  (Slot Ocupado)
        │   │   │   ├─ Set Slot Data (Target: As WBP Inventory Slot)
        │   │   │   │     └─ New Slot Data: Array Element (do ForEachLoop)
        │   │   │   └─ Update Slot Visual (Target: As WBP Inventory Slot)  ← CRÍTICO!
        │   │   │
        │   │   └─ FALSE: (Slot Vazio)
        │   │       ├─ Clear Slot (Target: As WBP Inventory Slot)
        │   │       └─ Update Slot Visual (Target: As WBP Inventory Slot)  ← CRÍTICO!
        │   │
        │   └─ FALSE:
        │       └─ Print String (String: "Erro: Widget de slot não é válido no índice {Slot Index}", Color: Red)
  ↓
Completed:
  └─ Update Item Count (Target: self)  ← Sua função para atualizar "Slots: X/100"
```

### **VERIFICAÇÕES ESPECÍFICAS:**

1. **`Get Array Item` - Index:**
   - Deve usar o `Slot Index` do `Break Umbra Inventory Slot` (0-99)
   - **NÃO** deve usar o `Array Index` do `ForEachLoop`!

2. **`Update Slot Visual`:**
   - Deve ser chamado **APÓS** `Set Slot Data` ou `Clear Slot`
   - Sem isso, os slots não atualizam visualmente!

3. **`Clear Slot`:**
   - Deve ser chamado quando `Inventory ID <= 0` (slot vazio)
   - Isso limpa os dados internos do widget

---

## 🔴 **PROBLEMA #4: `UpdateSlotVisual` não está resetando a cor de fundo**

### **VERIFICAÇÃO NO `WBP_InventorySlot`:**

No `WBP_InventorySlot`, na função `UpdateSlotVisual`, o caminho `FALSE` (quando `InventoryID <= 0`) deve fazer **TODAS** estas ações:

```
Branch (InventoryID > 0?)
  ├─ TRUE: (Slot tem item)
  │   └─ (lógica existente para mostrar item)
  │
  └─ FALSE: (Slot está vazio)  ← VERIFIQUE ESTE CAMINHO!
      ├─ Set Visibility (Image_ItemIcon) → Hidden
      ├─ Set Visibility (Text_Quantity) → Hidden
      ├─ Set Visibility (ProgressBar_Durability) → Hidden
      ├─ Get Border_Slot (ou Image_Slot, ou o widget de fundo do slot)
      ├─ Set Brush Color (Border_Slot)
      │     └─ In Color: Make Linear Color (R=1.0, G=1.0, B=1.0, A=1.0)  ← BRANCO!
      └─ (opcional) Set bIsEmpty = true
```

### **SE FALTAR O `Set Brush Color`:**

1. **Identifique o widget de fundo do slot:**
   - Pode ser um `Border` (ex: `Border_Slot`, `BG_Slot`)
   - Pode ser um `Image` (ex: `Image_Slot`, `BG_Image`)
   - Verifique no **Hierarchy** (painel esquerdo) do Designer

2. **Adicione o `Set Brush Color`:**
   - Arraste o widget de fundo para o Event Graph
   - Selecione "Get [Nome do Widget]"
   - Arraste do pino azul e procure por "Set Brush Color"
   - Conecte `Make Linear Color` (R=1.0, G=1.0, B=1.0, A=1.0) ao pin `In Color`

---

## ✅ **ORDEM DE CORREÇÃO RECOMENDADA:**

1. **PRIMEIRO:** Corrigir o bug no C++ (`GetStorageSlotByIndex` e `GetAllStorageSlots`)
2. **SEGUNDO:** Verificar e corrigir o `CreateStorageSlots` no Blueprint
3. **TERCEIRO:** Verificar e corrigir o `OnStorageLoaded_Event` no Blueprint
4. **QUARTO:** Verificar e corrigir o `UpdateSlotVisual` no `WBP_InventorySlot`

---

## 🔍 **COMO VERIFICAR SE ESTÁ FUNCIONANDO:**

1. **Compile o projeto C++** após corrigir `GetStorageSlotByIndex`
2. **Compile os Blueprints** (`WBP_Storage` e `WBP_InventorySlot`)
3. **Execute o jogo** e abra o Storage
4. **Verifique:**
   - ✅ Os 100 slots do storage devem aparecer
   - ✅ Se você tiver itens no storage (verifique via `test_storage.html`), eles devem aparecer nos slots corretos
   - ✅ Slots vazios devem ter fundo branco (não cinza)
   - ✅ A contagem de slots ("Slots: X/100") deve estar correta

---

## 📝 **RESUMO DOS PROBLEMAS:**

| Problema | Localização | Solução |
|----------|-------------|---------|
| Slots não aparecem | `GetStorageSlotByIndex` (C++) | Converter índice 0-99 para 50-149 antes de procurar |
| Slots não aparecem | `CreateStorageSlots` (Blueprint) | Verificar `Target` do `Add Child to Uniform Grid` |
| Itens não atualizam | `OnStorageLoaded_Event` (Blueprint) | Verificar `Get Array Item` usando `Slot Index` (não `Array Index`) |
| Slots brancos/cinza | `UpdateSlotVisual` (Blueprint) | Adicionar `Set Brush Color` no caminho `FALSE` |

---

**Após aplicar todas as correções, o storage deve funcionar corretamente!** 🚀

