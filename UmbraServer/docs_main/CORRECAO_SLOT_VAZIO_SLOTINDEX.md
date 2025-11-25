# 🔧 CORREÇÃO: Item sempre vai para o primeiro slot ao dropar em slot vazio

**PROBLEMA:** Quando você move um item para um slot vazio, ele sempre vai para o primeiro slot (slot 0), substituindo o item que estava lá.

---

## 🔍 **CAUSA RAIZ:**

O problema está na função `RequestMoveItem` em `UmbraInventorySlotWidget.cpp` (linha 176):

```cpp
int32 TargetSlotIndex = SlotData.SlotIndex;
```

**Quando o slot está vazio**, o `SlotData.SlotIndex` pode estar como **0** (valor padrão), fazendo o item sempre ir para o primeiro slot!

---

## ✅ **SOLUÇÃO:**

O `SlotData.SlotIndex` precisa ser **definido corretamente quando o slot é criado** no Blueprint. Vamos garantir isso:

### **PASSO 1: Corrigir `CreateInventorySlots` no WBP_Inventory**

No Blueprint `WBP_Inventory`, na função `CreateInventorySlots`, você DEVE chamar `SetSlotIndex` para cada slot criado:

```
For Loop (0 a 49)
  ↓
Create Widget (WBP_InventorySlot)
  ↓
Set Slot Index (no widget criado)  ← NOVA FUNÇÃO C++!
  └─ New Slot Index: Index (do For Loop)  ← CRÍTICO!
  ↓
Add Child to Uniform Grid
  ↓
Add to Array (SlotWidgets)
```

**IMPORTANTE:** 
1. Após criar cada widget, chame `Set Slot Index` passando o índice do loop
2. Isso garante que `SlotData.SlotIndex` está correto mesmo quando o slot está vazio
3. A função `SetSlotIndex` foi adicionada no C++ especificamente para resolver este problema

### **PASSO 2: Implementação completa no Blueprint**

**Exemplo correto de `CreateInventorySlots`:**

```
For Loop (First: 0, Last: 49)
  Loop Body:
    ├─ Create Widget (Class: WBP_InventorySlot)
    │     └─ Return Value: SlotWidget
    ├─ Set Slot Index (Target: SlotWidget)  ← NOVA FUNÇÃO!
    │     └─ New Slot Index: Index (do For Loop)
    ├─ Add Child to Uniform Grid
    │     ├─ Target: Grid_InventorySlots
    │     ├─ Content: SlotWidget
    │     ├─ Row: Index / 5
    │     └─ Column: Index % 5
    └─ Add to Array (SlotWidgets)
        └─ Array Element: SlotWidget
```

**Como encontrar `Set Slot Index` no Blueprint:**
1. Arraste o widget criado (`SlotWidget`) para o gráfico
2. Clique direito → procure `Set Slot Index`
3. Conecte o `Index` do `For Loop` ao parâmetro `New Slot Index`

### **PASSO 3: Adicionar validação no C++**

Já adicionei uma validação no C++ que vai logar um erro se o `TargetSlotIndex` estiver inválido. Isso vai ajudar a identificar o problema.

---

## 🔍 **VERIFICAÇÃO:**

### **1. Adicione logs de debug no Blueprint:**

No `WBP_Inventory` → **CreateInventorySlots**, após `Set Slot Data`:

```
Set Slot Data
  ↓
Get Slot Data (do widget criado)
  ↓
Break Umbra Inventory Slot
  └─ Slot Index
  ↓
Print String: "Slot criado: Index = X"
```

Isso vai mostrar se o `SlotIndex` está sendo definido corretamente.

### **2. Adicione logs no `OnDrop` do `WBP_InventorySlot`:**

No `WBP_InventorySlot` → **OnDrop**:

```
On Drop
  ↓
Get Slot Data (self)
  ↓
Break Umbra Inventory Slot
  └─ Slot Index
  ↓
Print String: "Drop no slot: Index = X"
```

Isso vai mostrar qual slot está recebendo o drop.

---

## 🛠️ **CORREÇÃO ALTERNATIVA (se o problema persistir):**

Se mesmo definindo o `SlotIndex` no `CreateInventorySlots` o problema persistir, podemos usar uma abordagem diferente:

### **Opção A: Passar o índice do slot como parâmetro**

Modificar `ProcessItemDrop` para receber o `TargetSlotIndex` explicitamente:

```cpp
// No .h
UFUNCTION(BlueprintCallable, Category = "Drag & Drop")
bool ProcessItemDrop(UUmbraInventorySlotWidget* DraggedSlotWidget, int32 TargetSlotIndex);
```

Mas isso requer mudanças no Blueprint também.

### **Opção B: Usar o índice no array `SlotWidgets`**

No `WBP_Inventory`, quando chamar `ProcessItemDrop`, passar também o índice do slot no array:

```
On Drop (no WBP_InventorySlot)
  ↓
Get Parent (self)
  ↓
Cast to WBP_Inventory
  ↓
Get Slot Widgets (do parent)
  ↓
Find Item in Array (Slot Widgets, self)
  └─ Index: SlotIndexNoArray
  ↓
Process Item Drop (self)
  └─ Target Slot Index: SlotIndexNoArray  ← Passar explicitamente
```

Mas isso também requer mudanças no C++.

---

## 📋 **CHECKLIST:**

- [ ] **Compilou o C++** (a função `SetSlotIndex` foi adicionada)
- [ ] `CreateInventorySlots` chama `Set Slot Index` para cada slot criado
- [ ] `Set Slot Index` recebe o `Index` do `For Loop` (0, 1, 2, ..., 49)
- [ ] Logs de debug mostram que `SlotIndex` está correto quando slots são criados
- [ ] Logs de debug mostram que `SlotIndex` está correto quando drop ocorre
- [ ] Testou e verificou que item vai para o slot correto (não sempre para o primeiro)

---

## 🎯 **RESULTADO ESPERADO:**

✅ **Mover item para slot vazio** → Item aparece no slot correto (não no primeiro)  
✅ **Mover item para slot com item** → Itens trocam de lugar corretamente  
✅ **Logs mostram `SlotIndex` correto** em todos os casos

---

**IMPLEMENTE A CORREÇÃO E TESTE!** 🚀

