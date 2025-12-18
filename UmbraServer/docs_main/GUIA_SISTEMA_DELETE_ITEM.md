# Guia: Sistema de Deleção de Itens (WBP_DeleteItem)

## 📋 Estrutura Atual

```
WBP_Inventory
    └── BTN_DeleteItem (abre WBP_DeleteItem)

WBP_DeleteItem
    ├── Text_Title ("Delete Item")
    ├── Text_Item_Name (nome do item a deletar)
    ├── Grid_DeleteSlots (UniformGridPanel)
    │   └── WBP_TrashSlot (aceita drop de itens)
    ├── BTN_Delete_Item (confirma deleção)
    ├── BTN_Cancel_Delete (cancela)
    └── Text_Info ("Deleted INFO")
```

---

## 1️⃣ Configurar WBP_TrashSlot

### 1.1 Parent Class

O `WBP_TrashSlot` deve herdar de `UmbraInventorySlotWidget` para ter acesso às funções de drag & drop.

**Se ainda não herda:**
1. Abra `WBP_TrashSlot`
2. File → Reparent Blueprint
3. Selecione `UmbraInventorySlotWidget`

### 1.2 Configurar Propriedades

No **Class Defaults** ou **Construction Script** do `WBP_TrashSlot`:

```
bCanDragFrom = false     ← NÃO pode arrastar daqui
bCanDropTo = true        ← PODE soltar itens aqui
bIsStorageSlot = false   ← NÃO é storage
SlotIndex = -1           ← Índice especial (lixeira)
```

### 1.3 Criar Variáveis no WBP_TrashSlot

Adicione estas variáveis:

| Nome | Tipo | Descrição |
|------|------|-----------|
| `PendingDeleteItem` | `FUmbraInventorySlot` | Item aguardando deleção |
| `bHasItemToDelete` | `Boolean` | Se tem item pendente |
| `ParentDeleteWidget` | `WBP_DeleteItem` (Object Reference) | Referência ao widget pai |

### 1.4 Criar Event Dispatcher

No `WBP_TrashSlot`, crie um **Event Dispatcher**:

```
Nome: OnItemDroppedForDeletion
Parâmetros:
  - ItemData (FUmbraInventorySlot)
```

---

## 2️⃣ Implementar OnDrop no WBP_TrashSlot

### 2.1 Override do OnDrop

No **Event Graph** do `WBP_TrashSlot`:

```
┌──────────────────────────────────────────────────────────────┐
│ Event OnDrop (Geometry, PointerEvent, Operation)             │
│     │                                                        │
│     ├─→ Cast Operation to UmbraItemDragDropOperation         │
│     │       │                                                │
│     │       ├─→ (Success)                                    │
│     │       │       │                                        │
│     │       │       ├─→ Get: DraggedItemData                 │
│     │       │       │       │                                │
│     │       │       │       └─→ Set: PendingDeleteItem       │
│     │       │       │                                        │
│     │       │       ├─→ Set: bHasItemToDelete = true         │
│     │       │       │                                        │
│     │       │       ├─→ Call: SetSlotData (DraggedItemData)  │
│     │       │       │   (Mostra o item visualmente no slot)  │
│     │       │       │                                        │
│     │       │       ├─→ Call: OnItemDroppedForDeletion       │
│     │       │       │       (Broadcast com ItemData)         │
│     │       │       │                                        │
│     │       │       └─→ Return: true (handled)               │
│     │       │                                                │
│     │       └─→ (Failed)                                     │
│     │               └─→ Return: false                        │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 Blueprint Visual (Detalhado)

```
[Event OnDrop]
      │
      ▼
[Cast to UmbraItemDragDropOperation] ─── Operation
      │
      │ (As UmbraItemDragDropOperation)
      ▼
[Get DraggedItemData] ─────────────────► [SET PendingDeleteItem]
      │
      ▼
[SET bHasItemToDelete] ◄─────────────── true
      │
      ▼
[Set Slot Data] ◄──────────────────────── PendingDeleteItem
      │
      ▼
[Call OnItemDroppedForDeletion] ◄──────── PendingDeleteItem
      │
      ▼
[Return Node] ◄────────────────────────── true
```

---

## 3️⃣ Configurar WBP_DeleteItem

### 3.1 Variáveis Necessárias

Adicione no `WBP_DeleteItem`:

| Nome | Tipo | Descrição |
|------|------|-----------|
| `TrashSlot` | `WBP_TrashSlot` (Object Reference) | Referência ao slot de lixeira |
| `ItemToDelete` | `FUmbraInventorySlot` | Item atual para deletar |
| `bCanDelete` | `Boolean` | Se pode deletar |

### 3.2 Obter Referência do TrashSlot

No **Event Construct** do `WBP_DeleteItem`:

```
┌──────────────────────────────────────────────────────────────┐
│ Event Construct                                              │
│     │                                                        │
│     ├─→ Get: Grid_DeleteSlots                                │
│     │       │                                                │
│     │       └─→ Get Child at Index (0)                       │
│     │               │                                        │
│     │               └─→ Cast to WBP_TrashSlot                │
│     │                       │                                │
│     │                       ├─→ SET: TrashSlot               │
│     │                       │                                │
│     │                       └─→ Bind Event to                │
│     │                           OnItemDroppedForDeletion     │
│     │                               │                        │
│     │                               └─→ [HandleItemDropped]  │
│     │                                                        │
│     ├─→ SET: bCanDelete = false                              │
│     │                                                        │
│     └─→ Set Text (Text_Item_Name) = "Arraste um item aqui"   │
└──────────────────────────────────────────────────────────────┘
```

### 3.3 Criar Custom Event: HandleItemDropped

```
┌──────────────────────────────────────────────────────────────┐
│ [HandleItemDropped] (ItemData: FUmbraInventorySlot)          │
│     │                                                        │
│     ├─→ SET: ItemToDelete = ItemData                         │
│     │                                                        │
│     ├─→ SET: bCanDelete = true                               │
│     │                                                        │
│     ├─→ Get: ItemData.ItemTemplate.ItemName                  │
│     │       │                                                │
│     │       └─→ Set Text (Text_Item_Name)                    │
│     │                                                        │
│     └─→ (Opcional) Atualizar Text_Info com quantidade, etc.  │
└──────────────────────────────────────────────────────────────┘
```

### 3.4 Botão Delete (BTN_Delete_Item)

```
┌──────────────────────────────────────────────────────────────┐
│ OnClicked (BTN_Delete_Item)                                  │
│     │                                                        │
│     ├─→ Branch: bCanDelete == true?                          │
│     │       │                                                │
│     │       ├─→ (True)                                       │
│     │       │       │                                        │
│     │       │       ├─→ Get Game Instance                    │
│     │       │       │       │                                │
│     │       │       │       └─→ Cast to UmbraGameInstance    │
│     │       │       │               │                        │
│     │       │       │               └─→ Remove Item          │
│     │       │       │                   (ItemToDelete.       │
│     │       │       │                    InventoryID, 0)     │
│     │       │       │                                        │
│     │       │       ├─→ Set Text (Text_Info) =               │
│     │       │       │   "Item deletado!"                     │
│     │       │       │                                        │
│     │       │       ├─→ Call: ResetTrashSlot                 │
│     │       │       │                                        │
│     │       │       └─→ SET: bCanDelete = false              │
│     │       │                                                │
│     │       └─→ (False)                                      │
│     │               └─→ Set Text (Text_Info) =               │
│     │                   "Nenhum item selecionado!"           │
└──────────────────────────────────────────────────────────────┘
```

### 3.5 Botão Cancel (BTN_Cancel_Delete)

```
┌──────────────────────────────────────────────────────────────┐
│ OnClicked (BTN_Cancel_Delete)                                │
│     │                                                        │
│     ├─→ Call: ResetTrashSlot                                 │
│     │                                                        │
│     ├─→ SET: bCanDelete = false                              │
│     │                                                        │
│     └─→ Remove from Parent (fecha o widget)                  │
│         OU Set Visibility = Collapsed                        │
└──────────────────────────────────────────────────────────────┘
```

### 3.6 Criar Função: ResetTrashSlot

```
┌──────────────────────────────────────────────────────────────┐
│ [ResetTrashSlot] (Function)                                  │
│     │                                                        │
│     ├─→ Is Valid: TrashSlot?                                 │
│     │       │                                                │
│     │       └─→ (Valid)                                      │
│     │               │                                        │
│     │               └─→ TrashSlot → Clear Slot               │
│     │                                                        │
│     ├─→ SET: bCanDelete = false                              │
│     │                                                        │
│     ├─→ Set Text (Text_Item_Name) = "Arraste um item aqui"   │
│     │                                                        │
│     └─→ Set Text (Text_Info) = ""                            │
└──────────────────────────────────────────────────────────────┘
```

---

## 4️⃣ Bind ao Resultado da Deleção

### 4.1 No Event Construct do WBP_DeleteItem

Adicione binds para saber se a deleção foi bem-sucedida:

```
┌──────────────────────────────────────────────────────────────┐
│ Event Construct (continuação)                                │
│     │                                                        │
│     ├─→ Get Game Instance → Cast to UmbraGameInstance        │
│     │       │                                                │
│     │       ├─→ Bind Event to OnItemRemoved                  │
│     │       │       │                                        │
│     │       │       └─→ [OnDeleteSuccess]                    │
│     │       │                                                │
│     │       └─→ Bind Event to OnItemRemoveFailed             │
│     │               │                                        │
│     │               └─→ [OnDeleteFailed]                     │
└──────────────────────────────────────────────────────────────┘
```

### 4.2 Custom Event: OnDeleteSuccess

```
┌──────────────────────────────────────────────────────────────┐
│ [OnDeleteSuccess] (InventoryID, RemovedQuantity)             │
│     │                                                        │
│     ├─→ Set Text (Text_Info) = "✓ Item deletado!"            │
│     │                                                        │
│     ├─→ Call: ResetTrashSlot                                 │
│     │                                                        │
│     └─→ (Opcional) Delay 1s → Remove from Parent             │
└──────────────────────────────────────────────────────────────┘
```

### 4.3 Custom Event: OnDeleteFailed

```
┌──────────────────────────────────────────────────────────────┐
│ [OnDeleteFailed] (ErrorMessage)                              │
│     │                                                        │
│     └─→ Set Text (Text_Info) = ErrorMessage                  │
│         (Ex: "Não é possível remover item equipado")         │
└──────────────────────────────────────────────────────────────┘
```

---

## 5️⃣ Abrir WBP_DeleteItem do Inventário

### 5.1 No WBP_Inventory

```
┌──────────────────────────────────────────────────────────────┐
│ OnClicked (BTN_DeleteItem)                                   │
│     │                                                        │
│     ├─→ Create Widget: WBP_DeleteItem                        │
│     │       │                                                │
│     │       └─→ Add to Viewport                              │
│     │               │                                        │
│     │               └─→ Set Position in Viewport (centro)    │
│     │                                                        │
│     └─→ (Opcional) SET: DeleteItemWidget = Created Widget    │
└──────────────────────────────────────────────────────────────┘
```

---

## 6️⃣ Diagrama de Fluxo Completo

```
┌─────────────────────────────────────────────────────────────────┐
│                         FLUXO COMPLETO                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  [Inventory]                                                    │
│       │                                                         │
│       │ Click "Delete Items"                                    │
│       ▼                                                         │
│  [WBP_DeleteItem] ──────────────────────────┐                   │
│       │                                      │                   │
│       │ Construído                           │                   │
│       ▼                                      │                   │
│  ┌─────────────────────────────┐            │                   │
│  │ "Arraste um item aqui"     │            │                   │
│  │  ┌───────────────────────┐ │            │                   │
│  │  │    WBP_TrashSlot      │ │◄───────────┘                   │
│  │  │    (vazio)            │ │                                 │
│  │  └───────────────────────┘ │                                 │
│  │  [Delete]     [Cancel]     │                                 │
│  └─────────────────────────────┘                                │
│                                                                 │
│  [Usuário arrasta item do Inventory]                            │
│       │                                                         │
│       │ Drop no TrashSlot                                       │
│       ▼                                                         │
│  ┌─────────────────────────────┐                                │
│  │ "Espada de Ferro"          │ ◄── ItemName atualizado        │
│  │  ┌───────────────────────┐ │                                 │
│  │  │    WBP_TrashSlot      │ │                                 │
│  │  │    [🗡️ Item Icon]     │ │ ◄── Mostra o item              │
│  │  └───────────────────────┘ │                                 │
│  │  [Delete]     [Cancel]     │                                 │
│  └─────────────────────────────┘                                │
│       │                                                         │
│       │ Click "Delete"                                          │
│       ▼                                                         │
│  [GameInstance.RemoveItem(InventoryID, 0)]                      │
│       │                                                         │
│       ├─→ Sucesso: "Item deletado!" + Reset                     │
│       │                                                         │
│       └─→ Falha: "Erro: ..." (ex: item equipado)               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7️⃣ Checklist de Implementação

### WBP_TrashSlot
- [ ] Herda de `UmbraInventorySlotWidget`
- [ ] `bCanDropTo = true`
- [ ] `bCanDragFrom = false`
- [ ] Variável `PendingDeleteItem`
- [ ] Event Dispatcher `OnItemDroppedForDeletion`
- [ ] Override `OnDrop` implementado

### WBP_DeleteItem
- [ ] Variável `TrashSlot` (referência)
- [ ] Variável `ItemToDelete`
- [ ] Variável `bCanDelete`
- [ ] Bind ao `OnItemDroppedForDeletion`
- [ ] Handler `HandleItemDropped`
- [ ] `BTN_Delete_Item` chama `RemoveItem`
- [ ] `BTN_Cancel_Delete` fecha o widget
- [ ] Função `ResetTrashSlot`
- [ ] Bind `OnItemRemoved` e `OnItemRemoveFailed`

### WBP_Inventory
- [ ] `BTN_DeleteItem` cria e abre `WBP_DeleteItem`

---

## 8️⃣ Código Alternativo: Sem Event Dispatcher

Se preferir uma abordagem mais simples, você pode fazer o TrashSlot chamar diretamente uma função do pai:

### No WBP_TrashSlot (OnDrop):

```
[Event OnDrop]
      │
      ▼
[Cast to UmbraItemDragDropOperation]
      │
      ▼
[Get DraggedItemData]
      │
      ▼
[Set Slot Data] ◄── DraggedItemData
      │
      ▼
[Get Outer] ou [Get Parent] ─────────► [Cast to WBP_DeleteItem]
      │                                        │
      │                                        ▼
      │                               [Call: OnItemReceived(ItemData)]
      │
      ▼
[Return true]
```

### No WBP_DeleteItem:

Crie uma função pública:

```
[OnItemReceived] (ItemData: FUmbraInventorySlot)
      │
      ▼
[SET ItemToDelete] ◄── ItemData
      │
      ▼
[SET bCanDelete] ◄── true
      │
      ▼
[Set Text: Text_Item_Name] ◄── ItemData.ItemTemplate.ItemName
```

---

## 9️⃣ Dicas Extras

### Mostrar Quantidade do Item

No `HandleItemDropped`, adicione:

```
Format Text: "{0} x{1}"
  - {0} = ItemData.ItemTemplate.ItemName
  - {1} = ItemData.Quantity
```

### Mostrar Ícone do Item

Se o TrashSlot mostra o ícone automaticamente via `SetSlotData`, ótimo!

Se não, no `HandleItemDropped`:
```
Get: ItemData.ItemTemplate.Icon
Set Brush (Image_ItemIcon) = Icon
```

### Prevenir Deletar Item Equipado (Client-Side)

No `HandleItemDropped`:
```
Branch: ItemData.bIsEquipped == true?
  ├─→ (True) Set Text_Info = "Desequipe o item primeiro!"
  │          SET bCanDelete = false
  │
  └─→ (False) [Continua normal...]
```

---

**Pronto!** Siga este guia e seu sistema de deleção estará funcionando. Se tiver dúvidas em alguma parte, me avise!
