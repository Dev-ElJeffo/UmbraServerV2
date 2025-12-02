# 🔧 GUIA COMPLETO: Unequip de Itens (Drag & Drop + Double Click)

## 🎯 **OBJETIVOS:**

1. ✅ **Drag and Drop:** Arrastar item equipado para slot do inventário → Desequipar
2. ✅ **Double Click:** Duplo clique no slot de equipamento → Desequipar
3. ✅ **Tooltip:** Mostrar tooltip ao passar o mouse sobre item equipado
4. ✅ **Atualização Visual:** Remover item do slot após desequipar

---

## 📋 **PARTE 1: Drag and Drop (Equipment Slot → Inventory Slot)**

### **1.1 Modificar OnDrop do WBP_InventorySlot**

**OBJETIVO:** Aceitar itens arrastados de `WBP_EquipmentSlot` e desequipá-los automaticamente.

**LOCALIZAÇÃO:** `WBP_InventorySlot` → Event Graph → `OnDrop`

**ESTRUTURA COMPLETA COM CÓDIGO EXATO:**

```
[OnDrop]
  ├─ My Geometry: (FGeometry)
  ├─ Pointer Event: (FPointerEvent)
  ├─ Operation: (Drag Drop Operation)
  ↓
[Cast to Umbra Item Drag Drop Operation]
  ├─ Object: Operation
  └─ Success: As Umbra Item Drag Drop Operation
       ↓
[Is Valid] (As Umbra Item Drag Drop Operation)
  ├─ True:
  │    ↓
  │  [Get Source Slot Widget]
  │    ├─ Target: As Umbra Item Drag Drop Operation
  │    └─ Return Value: (User Widget)
  │         ↓
  │    [Is Valid] (Source Slot Widget)
  │      ├─ True:
  │      │    ↓
  │      │  ═══════════════════════════════════════════════════════
  │      │  🔴 ADICIONAR AQUI: Verificação para WBP_EquipmentSlot
  │      │  ═══════════════════════════════════════════════════════
  │      │    ↓
  │      │  [Cast to WBP Equipment Slot]
  │      │    ├─ Object: Source Slot Widget
  │      │    └─ Success: As WBP Equipment Slot
  │      │         ↓
  │      │    [Get Dragged Item Data]
  │      │      ├─ Target: As Umbra Item Drag Drop Operation
  │      │      └─ Return Value: (FUmbraInventorySlot)
  │      │           ↓
  │      │      [Break Umbra Inventory Slot]
  │      │        └─ Inventory ID
  │      │             ↓
  │      │      [Branch] (Inventory ID > 0?)
  │      │        ├─ True:
  │      │        │    ↓
  │      │        │  [Get Game Instance]
  │      │        │    ↓
  │      │        │  [Cast to Umbra Game Instance]
  │      │        │    ↓
  │      │        │  [Unequip Item]
  │      │        │    └─ Inventory ID: Inventory ID
  │      │        │         ↓
  │      │        │  [Make Literal Bool] (true)
  │      │        │    └─ Return Value: true
  │      │        │         ↓
  │      │        │  [Function Result] (Return Value: true)
  │      │        │
  │      │        └─ False:
  │      │             ↓
  │      │        [Make Literal Bool] (false)
  │      │          └─ Return Value: false
  │      │               ↓
  │      │        [Function Result] (Return Value: false)
  │      │
  │      │    └─ Cast Failed: (Source não é Equipment Slot)
  │      │         ↓
  │      │    ═══════════════════════════════════════════════════════
  │      │    🟢 CONTINUAR AQUI: Lógica normal do inventário
  │      │    ═══════════════════════════════════════════════════════
  │      │         ↓
  │      │    [Cast to WBP Inventory Slot]
  │      │      ├─ Object: Source Slot Widget
  │      │      └─ Success: As WBP Inventory Slot
  │      │           ↓
  │      │      [Is Slot Empty] (self)
  │      │        ├─ True: (Slot está vazio)
  │      │        │    ↓
  │      │        │  [Process Item Drop]
  │      │        │    ├─ Target: self
  │      │        │    └─ Dragged Slot Widget: Source Slot Widget
  │      │        │         ↓
  │      │        │  [Function Result] (Return Value: Return Value)
  │      │        │
  │      │        └─ False: (Slot não está vazio)
  │      │             ↓
  │      │      [Get Slot Data] (self)
  │      │        └─ Return Value: (FUmbraInventorySlot)
  │      │             ↓
  │      │      [Can Stack With]
  │      │        ├─ Target: Source Slot Widget
  │      │        └─ Other Item Data: Return Value (Get Slot Data)
  │      │             ↓
  │      │      [Branch] (Can Stack With?)
  │      │        ├─ True:
  │      │        │    ↓
  │      │        │  [Request Stack Item]
  │      │        │    ├─ Target: Source Slot Widget
  │      │        │    └─ Target Slot Widget: self
  │      │        │         ↓
  │      │        │  [Function Result] (Return Value: true)
  │      │        │
  │      │        └─ False:
  │      │             ↓
  │      │        [Process Item Drop]
  │      │          ├─ Target: self
  │      │          └─ Dragged Slot Widget: Source Slot Widget
  │      │               ↓
  │      │        [Function Result] (Return Value: Return Value)
  │      │
  │      └─ False: (Source Slot Widget é inválido)
  │           ↓
  │      [Make Literal Bool] (false)
  │        └─ Return Value: false
  │             ↓
  │      [Function Result] (Return Value: false)
  │
  └─ False: (Cast falhou)
       ↓
  [Make Literal Bool] (false)
    └─ Return Value: false
         ↓
  [Function Result] (Return Value: false)
```

**📍 ONDE ADICIONAR A VERIFICAÇÃO:**

Após `[Is Valid] (Source Slot Widget)` → `True`, **ANTES** de `[Cast to WBP Inventory Slot]`, adicione:

1. **`[Cast to WBP Equipment Slot]`**
   - **Object:** `Source Slot Widget` (do `Get Source Slot Widget`)
   - **Success:** Conecte ao próximo nó

2. **`[Get Dragged Item Data]`**
   - **Target:** `As Umbra Item Drag Drop Operation` (do primeiro cast)
   - **Return Value:** `FUmbraInventorySlot`

3. **`[Break Umbra Inventory Slot]`**
   - **Input:** `Return Value` do `Get Dragged Item Data`
   - **Output:** `Inventory ID`

4. **`[Branch]`**
   - **Condition:** `Inventory ID > 0?`
   - **True:** Chama `Unequip Item`
   - **False:** Retorna `false`

5. **`[Get Game Instance]`** → **`[Cast to Umbra Game Instance]`** → **`[Unequip Item]`**
   - **Inventory ID:** `Inventory ID` do `Break Umbra Inventory Slot`

6. **`[Make Literal Bool]`** (true)
   - Conecte ao `Function Result` → `Return Value`

**IMPORTANTE:**
- A verificação para `WBP_EquipmentSlot` deve ser feita **ANTES** da verificação `Is Slot Empty`
- Se o cast para `WBP_EquipmentSlot` for bem-sucedido, chama `Unequip Item` e retorna `true`
- Se o cast falhar, continua com a lógica normal do inventário (verificar se está vazio, stack, etc.)
- O PHP automaticamente coloca o item no primeiro slot vazio do inventário após desequipar
- Não é necessário chamar `Move Item` - o `UnequipItem` já faz isso no backend

---

### **1.2 Diagrama Visual: Onde Adicionar a Verificação**

```
[OnDrop]
  ↓
[Cast to Umbra Item Drag Drop Operation]
  ↓
[Is Valid] (As Umbra Item Drag Drop Operation)
  ├─ False → [Make Literal Bool] (false) → [Function Result]
  │
  └─ True
       ↓
  [Get Source Slot Widget]
       ↓
  [Is Valid] (Source Slot Widget)
       ├─ False → [Make Literal Bool] (false) → [Function Result]
       │
       └─ True
            ↓
       ════════════════════════════════════════════════════════════
       🔴 INSERIR AQUI: Verificação para WBP_EquipmentSlot
       ════════════════════════════════════════════════════════════
            ↓
       [Cast to WBP Equipment Slot]
            ├─ Success (é Equipment Slot):
            │    ↓
            │  [Get Dragged Item Data]
            │    ↓
            │  [Break Umbra Inventory Slot] → Inventory ID
            │    ↓
            │  [Branch] (Inventory ID > 0?)
            │    ├─ True:
            │    │    ↓
            │    │  [Get Game Instance]
            │    │    ↓
            │    │  [Cast to Umbra Game Instance]
            │    │    ↓
            │    │  [Unequip Item] (Inventory ID)
            │    │    ↓
            │    │  [Make Literal Bool] (true)
            │    │    ↓
            │    │  [Function Result] (Return Value: true)
            │    │
            │    └─ False:
            │         ↓
            │    [Make Literal Bool] (false)
            │         ↓
            │    [Function Result] (Return Value: false)
            │
            └─ Cast Failed (não é Equipment Slot):
                 ↓
            ════════════════════════════════════════════════════════════
            🟢 CONTINUAR AQUI: Lógica normal do inventário
            ════════════════════════════════════════════════════════════
                 ↓
            [Cast to WBP Inventory Slot]
                 ↓
            [Is Slot Empty] (self)
                 ├─ True → [Process Item Drop] → [Function Result]
                 │
                 └─ False
                      ↓
                 [Get Slot Data] (self)
                      ↓
                 [Can Stack With]
                      ├─ True → [Request Stack Item] → [Function Result]
                      │
                      └─ False → [Process Item Drop] → [Function Result]
```

---

### **1.3 Passo a Passo Detalhado**

**PASSO 1:** Após `[Is Valid] (Source Slot Widget)` → `True`, adicione:

1. **Nó:** `Cast to WBP Equipment Slot`
   - **Object:** Conecte ao `Return Value` de `Get Source Slot Widget`
   - **Success:** Conecte ao próximo nó (verificação de Inventory ID)
   - **Cast Failed:** Conecte ao `Cast to WBP Inventory Slot` (lógica normal)

**PASSO 2:** No `Success` do `Cast to WBP Equipment Slot`:

1. **Nó:** `Get Dragged Item Data`
   - **Target:** Conecte ao `As Umbra Item Drag Drop Operation` (do primeiro cast)
   - **Return Value:** `FUmbraInventorySlot`

2. **Nó:** `Break Umbra Inventory Slot`
   - **Input:** Conecte ao `Return Value` de `Get Dragged Item Data`
   - **Output:** `Inventory ID`

3. **Nó:** `Branch`
   - **Condition:** `Inventory ID > 0?`
   - **True:** Conecte ao `Get Game Instance`
   - **False:** Conecte ao `Make Literal Bool` (false) → `Function Result`

**PASSO 3:** No `True` do `Branch`:

1. **Nó:** `Get Game Instance`
2. **Nó:** `Cast to Umbra Game Instance`
3. **Nó:** `Unequip Item`
   - **Inventory ID:** Conecte ao `Inventory ID` do `Break Umbra Inventory Slot`
4. **Nó:** `Make Literal Bool` (true)
   - Conecte ao `Function Result` → `Return Value`

**PASSO 4:** No `Cast Failed` do `Cast to WBP Equipment Slot`:

- Conecte ao `Cast to WBP Inventory Slot` (lógica normal do inventário)

---

### **1.2 Verificar OnDragDetected do WBP_EquipmentSlot**

**JÁ DEVE ESTAR IMPLEMENTADO**, mas verifique se está assim:

```
[OnDragDetected]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Get Equipped Item] ← Variável
  ↓
[Break Umbra Inventory Slot]
  └─ Item Template ID
       ↓
[Greater (Int Int)]
  ├─ A: Item Template ID
  ├─ B: 0
  └─ Return Value: (bool)
       ↓
[Branch]
  ├─ True:
  │    ↓
  │  [Create Drag Drop Operation]
  │    ├─ Operation Class: Umbra Item Drag Drop Operation
  │    └─ Return Value: (Drag Drop Operation)
  │         ↓
  │    [Set Dragged Item Data]
  │      ├─ Target: Return Value
  │      └─ Dragged Item Data: Equipped Item
  │           ↓
  │    [Set Source Slot Widget]
  │      ├─ Target: Return Value
  │      └─ Source Slot Widget: self
  │           ↓
  │    [Return] (Drag Drop Operation)
  │
  └─ False:
       ↓
  [Return] (None)
```

---

## 📋 **PARTE 2: Double Click (Desequipar)**

### **2.1 OnMouseButtonDoubleClick (WBP_EquipmentSlot)**

**LOCALIZAÇÃO:** `WBP_EquipmentSlot` → Event Graph → `OnMouseButtonDoubleClick`

**ESTRUTURA COMPLETA:**

```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  ├─ In Mouse Event: (FPointerEvent)
  ↓
[Get Equipped Item] ← Variável
  ↓
[Break Umbra Inventory Slot]
  └─ Inventory ID
       ↓
[Branch] (Inventory ID > 0?)
  ├─ True:
  │    ↓
  │  [Get Game Instance]
  │    ↓
  │  [Cast to Umbra Game Instance]
  │    ↓
  │  [Unequip Item]
  │    └─ Inventory ID: Inventory ID
  │         ↓
  │  [Get Game Instance]
  │    ↓
  │  [Cast to Umbra Game Instance]
  │    ↓
  │  [Load Inventory] ← Recarregar inventário para mostrar o item
  │
  └─ False:
       ↓
  (nada - slot vazio)
```

**IMPORTANTE:**
- `Unequip Item` remove o item do slot de equipamento
- O PHP automaticamente coloca o item no primeiro slot vazio do inventário
- `Load Inventory` e `Load Character Info` são chamados automaticamente pelo C++ após `UnequipItem`

---

## 📋 **PARTE 3: Tooltip para Itens Equipados**

### **3.1 OnMouseEnter (WBP_EquipmentSlot)**

**LOCALIZAÇÃO:** `WBP_EquipmentSlot` → Event Graph → `OnMouseEnter`

**ESTRUTURA:**

```
[OnMouseEnter]
  ├─ My Geometry: (FGeometry)
  ├─ Mouse Event: (FPointerEvent)
  ↓
[Get Equipped Item] ← Variável
  ↓
[Break Umbra Inventory Slot]
  └─ Item Template ID
       ↓
[Branch] (Item Template ID > 0?)
  ├─ True:
  │    ↓
  │  [Create Widget]
  │    ├─ Class: WBP_ItemTooltip
  │    └─ Return Value: (WBP_ItemTooltip)
  │         ↓
  │    [Set Tooltip Widget]
  │      ├─ Target: self
  │      └─ Content: Return Value (WBP_ItemTooltip)
  │           ↓
  │    [Set Tooltip Item Data]
  │      ├─ Target: Return Value (WBP_ItemTooltip)
  │      └─ Item Slot: Equipped Item
  │
  └─ False:
       ↓
  (nada - slot vazio)
```

**IMPORTANTE:**
- `WBP_ItemTooltip` já deve existir (usado no inventário)
- `Set Tooltip Item Data` deve aceitar `FUmbraInventorySlot`
- O tooltip será criado e destruído automaticamente pelo Unreal

---

### **3.2 OnMouseLeave (WBP_EquipmentSlot)**

**LOCALIZAÇÃO:** `WBP_EquipmentSlot` → Event Graph → `OnMouseLeave`

**ESTRUTURA:**

```
[OnMouseLeave]
  ├─ Mouse Event: (FPointerEvent)
  ↓
[Set Tooltip Widget]
  ├─ Target: self
  └─ Content: None
```

**IMPORTANTE:**
- Remove o tooltip quando o mouse sai do slot

---

## 📋 **PARTE 4: Conectar Delegates para Atualização Visual**

### **4.1 No WBP_CharacterInfo - Event Construct**

**Conectar o delegate `OnItemUnequipped`:**

```
[Event Construct]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ↓
[Bind Event to OnItemUnequipped]
  ├─ Target: Cast (As Umbra Game Instance)
  └─ Event: OnItemUnequipped_Event (Custom Event)
       ↓
[OnItemUnequipped_Event]
  ├─ Inventory ID: (int32)
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Load Character Info] ← Recarregar para atualizar EquippedItems
  ├─ Target: Cast (As Umbra Game Instance)
```

**IMPORTANTE:**
- `OnItemUnequipped_Event` é um **Custom Event** que você cria
- Após desequipar, recarrega `Character Info` para atualizar o map `EquippedItems`
- Isso fará com que `Update Equipment Slots` seja chamado automaticamente

---

## 📋 **PARTE 5: Verificar C++ - UnequipItem**

**O C++ já deve estar implementado**, mas verifique se `OnEquipItemRequestComplete` chama `LoadCharacterInfo()`:

```cpp
void UUmbraGameInstance::OnEquipItemRequestComplete(UVaRestRequestJSON* Request)
{
    // ... parsing ...
    
    if (ResponseObj->GetBoolField(TEXT("is_equipped")))
    {
        // Equipado
        OnItemEquipped.Broadcast(InventoryID, EquipmentSlot);
    }
    else
    {
        // Desequipado
        OnItemUnequipped.Broadcast(InventoryID, EquipmentSlot);
    }
    
    // IMPORTANTE: Recarregar ambos!
    LoadInventory();      // Para atualizar inventário
    LoadCharacterInfo();  // Para atualizar EquippedItems map
}
```

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **WBP_EquipmentSlot:**
- [ ] `OnDragDetected` implementado (já deve estar)
- [ ] `OnMouseButtonDoubleClick` implementado
- [ ] `OnMouseEnter` implementado (tooltip)
- [ ] `OnMouseLeave` implementado (remover tooltip)

### **WBP_InventorySlot:**
- [ ] `OnDrop` modificado para aceitar itens de `WBP_EquipmentSlot`
- [ ] Chama `Unequip Item` quando origem é `WBP_EquipmentSlot`
- [ ] Chama `Move Item` após desequipar

### **WBP_CharacterInfo:**
- [ ] `OnItemUnequipped_Event` criado
- [ ] Delegate `OnItemUnequipped` conectado no `Event Construct`
- [ ] `Load Character Info` chamado no `OnItemUnequipped_Event`

### **C++ (Verificar):**
- [ ] `OnEquipItemRequestComplete` chama `LoadCharacterInfo()` após desequipar

---

## 🔧 **ORDEM DE IMPLEMENTAÇÃO:**

1. **PRIMEIRO:** Implementar `OnMouseButtonDoubleClick` no `WBP_EquipmentSlot` (mais simples)
2. **SEGUNDO:** Modificar `OnDrop` do `WBP_InventorySlot` para aceitar equipment slots
3. **TERCEIRO:** Implementar tooltip (`OnMouseEnter` e `OnMouseLeave`)
4. **QUARTO:** Conectar delegate `OnItemUnequipped` no `WBP_CharacterInfo`

---

## 🐛 **TROUBLESHOOTING:**

### **Problema: Item não aparece no inventário após desequipar**

**Solução:**
- Verifique se `Load Inventory` está sendo chamado após `Unequip Item`
- Verifique se o PHP está colocando o item no primeiro slot vazio

### **Problema: Slot de equipamento não limpa após desequipar**

**Solução:**
- Verifique se `OnItemUnequipped_Event` está chamando `Load Character Info`
- Verifique se `Update Equipment Slots` está sendo chamado após recarregar

### **Problema: Tooltip não aparece**

**Solução:**
- Verifique se `WBP_ItemTooltip` existe
- Verifique se `Set Tooltip Item Data` aceita `FUmbraInventorySlot`
- Verifique se `OnMouseEnter` está conectado corretamente

