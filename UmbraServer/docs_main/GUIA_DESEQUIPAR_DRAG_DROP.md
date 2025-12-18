# Guia: Desequipar Itens via Drag & Drop + Visual de Equipado

## 📋 Objetivo

1. ✅ Arrastar item do `WBP_EquipmentSlot` → `WBP_InventorySlot` = **Desequipa**
2. ✅ Mostrar indicador visual no `WBP_InventorySlot` quando item está equipado

---

## 1️⃣ Configurar WBP_EquipmentSlot para Permitir Drag

### 1.1 Verificar/Configurar Class Defaults

No `WBP_EquipmentSlot`, vá em **Class Defaults** e configure:

```
bCanDragFrom = true      ← CRÍTICO: Permitir arrastar DO slot
bCanDropTo = true        ← Permitir soltar itens aqui (para equipar)
bIsStorageSlot = false   ← NÃO é storage
```

### 1.2 Verificar no Construction Script (se necessário)

Se as propriedades não estiverem disponíveis nos Class Defaults:

```
Event Pre Construct (ou Construct)
    │
    ├─→ Set bCanDragFrom = true
    ├─→ Set bCanDropTo = true
    └─→ Set bIsStorageSlot = false
```

---

## 2️⃣ Configurar WBP_InventorySlot para Aceitar Drop de Equipamento

### 2.1 O Drop Já Deve Funcionar

Se o `WBP_InventorySlot` herda de `UmbraInventorySlotWidget`, ele já tem:
- `OnDrop` implementado
- `ProcessItemDropFromOperation` que chama `RequestMoveItemByID`

### 2.2 Verificar se Aceita Drop

No `WBP_InventorySlot` Class Defaults:

```
bCanDropTo = true        ← Deve estar true
bIsStorageSlot = false   ← NÃO é storage
```

---

## 3️⃣ Fluxo Completo de Desequipar

```
┌─────────────────────────────────────────────────────────────────┐
│                    FLUXO DE DESEQUIPAR                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  [WBP_EquipmentSlot com Item]                                   │
│       │                                                         │
│       │ (1) Usuário arrasta o item                              │
│       ▼                                                         │
│  CreateItemDragOperation()                                      │
│       │                                                         │
│       │ (2) DragOperation criado com:                           │
│       │     - DraggedItemData (dados do item equipado)          │
│       │     - SourceSlotWidget (o EquipmentSlot)                │
│       │     - SourceSlotIndex (índice do slot)                  │
│       ▼                                                         │
│  [Usuário solta no WBP_InventorySlot vazio]                     │
│       │                                                         │
│       │ (3) OnDrop detectado no InventorySlot                   │
│       ▼                                                         │
│  ProcessItemDropFromOperation()                                 │
│       │                                                         │
│       │ (4) Detecta: SourceWidget é EquipmentSlot?              │
│       │     - Origem: bIsEquipped = true                        │
│       │     - Destino: slot de inventário (0-49)                │
│       ▼                                                         │
│  RequestMoveItemByID(InventoryID, TargetSlot)                   │
│       │                                                         │
│       │ (5) Chama move_item.php                                 │
│       ▼                                                         │
│  Servidor: UPDATE ... SET is_equipped = 0, slot_index = X       │
│       │                                                         │
│       │ (6) Sucesso!                                            │
│       ▼                                                         │
│  OnInventoryLoaded → Atualiza UI                                │
│       │                                                         │
│       │ (7) EquipmentSlot fica vazio                            │
│       │     InventorySlot mostra o item                         │
│       ▼                                                         │
│  ✅ Item Desequipado!                                           │
└─────────────────────────────────────────────────────────────────┘
```

---

## 4️⃣ BLUEPRINT EXATO: WBP_EquipmentSlot

### 4.1 Event OnMouseButtonDown (OBRIGATÓRIO)

Este evento inicia a detecção de drag quando o usuário clica no slot.

**Passo a passo no Blueprint:**

1. No Event Graph, clique direito → "Add Event" → "Event On Mouse Button Down"
2. Configure exatamente assim:

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Event On Mouse Button Down                                              │
│ (My Geometry, Mouse Event) → Return: Event Reply                        │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  [Event On Mouse Button Down]                                           │
│       │                                                                 │
│       │ Mouse Event (output pin)                                        │
│       ▼                                                                 │
│  ┌─────────────────────────┐                                            │
│  │ Detect Drag If Pressed  │                                            │
│  ├─────────────────────────┤                                            │
│  │ Pointer Event: ─────────┼─── (conectar Mouse Event aqui)             │
│  │ Widget to Detect: ──────┼─── Self (referência ao próprio widget)     │
│  │ Drag Key: ──────────────┼─── Left Mouse Button                       │
│  └──────────┬──────────────┘                                            │
│             │                                                           │
│             │ Return Value (FEventReply)                                │
│             ▼                                                           │
│  ┌─────────────────────────┐                                            │
│  │     Return Node         │                                            │
│  └─────────────────────────┘                                            │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**IMPORTANTE**: O "Return Node" deve retornar o resultado de `Detect Drag If Pressed`, NÃO um "Handled" manual.

---

### 4.2 Event OnDragDetected (CRÍTICO - ONDE CRIA A OPERAÇÃO)

Este é o evento que CRIA a operação de drag. **Sem isso, nada funciona!**

**Passo a passo no Blueprint:**

1. No Event Graph, clique direito → "Add Event" → "Event On Drag Detected"
2. Configure EXATAMENTE assim:

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Event On Drag Detected                                                  │
│ (Pointer Event) → Return: Drag Drop Operation                           │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  [Event On Drag Detected]                                               │
│       │                                                                 │
│       │ Pointer Event                                                   │
│       ▼                                                                 │
│  ┌─────────────────────────┐                                            │
│  │    Is Slot Empty        │  ← Função da classe pai                    │
│  └──────────┬──────────────┘                                            │
│             │                                                           │
│             │ Return Value (bool)                                       │
│             ▼                                                           │
│  ┌─────────────────────────┐                                            │
│  │       Branch            │                                            │
│  └──────────┬──────────────┘                                            │
│             │                                                           │
│      ┌──────┴──────┐                                                    │
│      │             │                                                    │
│      ▼             ▼                                                    │
│   (True)        (False)                                                 │
│      │             │                                                    │
│      │             ▼                                                    │
│      │    ┌────────────────────────────┐                                │
│      │    │ Create Item Drag Operation │  ← Função da classe pai        │
│      │    └───────────┬────────────────┘                                │
│      │                │                                                 │
│      │                │ Return Value (UmbraItemDragDropOperation*)      │
│      │                ▼                                                 │
│      │    ┌────────────────────────────┐                                │
│      │    │   Is Valid (objeto)?       │                                │
│      │    └───────────┬────────────────┘                                │
│      │                │                                                 │
│      │         ┌──────┴──────┐                                          │
│      │         │             │                                          │
│      │         ▼             ▼                                          │
│      │      (Valid)      (Invalid)                                      │
│      │         │             │                                          │
│      │         │             │                                          │
│      │         ▼             │                                          │
│      │    ┌─────────────┐    │                                          │
│      │    │ Return Node │    │                                          │
│      │    │ (Drag Op)   │    │                                          │
│      │    └─────────────┘    │                                          │
│      │                       │                                          │
│      └───────────────────────┘                                          │
│                  │                                                      │
│                  ▼                                                      │
│         ┌─────────────┐                                                 │
│         │ Return Node │  ← Retorna nullptr/vazio (sem operação)         │
│         │ (nullptr)   │                                                 │
│         └─────────────┘                                                 │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

### 4.3 VERSÃO SIMPLIFICADA (Copie este Blueprint)

Se a versão acima parece complexa, use esta versão mais simples:

```
┌─────────────────────────────────────────────────────────────────────────┐
│ VERSÃO SIMPLIFICADA - Event On Drag Detected                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  [Event On Drag Detected]                                               │
│       │                                                                 │
│       ▼                                                                 │
│  ┌────────────────────────────┐                                         │
│  │ Create Item Drag Operation │                                         │
│  └───────────┬────────────────┘                                         │
│              │                                                          │
│              │ Return Value (Drag Operation)                            │
│              ▼                                                          │
│  ┌─────────────────────────────────────────────────────┐                │
│  │              Return Node                            │                │
│  │  ┌─────────────────────────────────────────────┐   │                │
│  │  │ Return Value: (conectar Drag Operation)     │   │                │
│  │  └─────────────────────────────────────────────┘   │                │
│  └─────────────────────────────────────────────────────┘                │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**NODES EXATOS:**
1. `Event On Drag Detected` (evento)
2. `Create Item Drag Operation` (função - chamar em Self)
3. `Return Node` (conectar o output de Create Item Drag Operation)

---

### 4.4 IMPORTANTE: O Return Node

O **Return Node** do `OnDragDetected` DEVE retornar uma operação de drag válida:

```
                                    ┌─────────────────────┐
Create Item Drag Operation ────────►│    Return Node      │
(UmbraItemDragDropOperation*)       │                     │
                                    │ Return Value: ●─────┼── (conectar aqui)
                                    └─────────────────────┘
```

**SE RETORNAR NULLPTR/VAZIO**: O drag não vai funcionar!

---

### 4.5 Verificação: A função CreateItemDragOperation existe?

Como `WBP_EquipmentSlot` herda de `UmbraInventorySlotWidget`, a função `CreateItemDragOperation` **JÁ EXISTE**.

Para verificar:
1. No Blueprint, clique direito
2. Pesquise "Create Item Drag"
3. Deve aparecer: `Create Item Drag Operation (inherited)`

Se não aparecer, verifique se o parent class está correto.

---

### 4.6 DEBUG: Adicionar Print Strings

Para debugar, adicione prints assim:

```
[Event On Drag Detected]
       │
       ▼
[Print String: "OnDragDetected chamado!"]
       │
       ▼
[Create Item Drag Operation]
       │
       │ Return Value
       ▼
[Is Valid?]
       │
   ┌───┴───┐
   │       │
   ▼       ▼
(Valid) (Invalid)
   │       │
   ▼       ▼
[Print:  [Print:
"Op      "Op
VÁLIDA"] INVÁLIDA!"]
   │       
   ▼       
[Return Node com a operação]
```

---

### 4.7 CÓDIGO DE REFERÊNCIA (C++)

Para referência, é isso que o `CreateItemDragOperation` faz internamente:

```cpp
UUmbraItemDragDropOperation* UUmbraInventorySlotWidget::CreateItemDragOperation()
{
    if (!bCanDragFrom || IsSlotEmpty())
        return nullptr;
    
    UUmbraItemDragDropOperation* DragOp = NewObject<UUmbraItemDragDropOperation>();
    DragOp->SourceSlotWidget = this;
    DragOp->DraggedItemData = SlotData;  // Os dados do item
    DragOp->SourceSlotIndex = SlotData.SlotIndex;
    
    return DragOp;
}
```

---

### 4.8 CHECKLIST DE VERIFICAÇÃO

Antes de testar, confirme:

- [ ] `WBP_EquipmentSlot` herda de `UmbraInventorySlotWidget`
- [ ] `bCanDragFrom = true` nos Class Defaults
- [ ] `Event OnMouseButtonDown` existe e chama `DetectDragIfPressed`
- [ ] `Event OnDragDetected` existe e chama `CreateItemDragOperation`
- [ ] O `Return Node` de `OnDragDetected` está conectado ao output de `CreateItemDragOperation`
- [ ] O slot tem um item (não está vazio) - `SetSlotData` foi chamado

---

### 4.9 PROBLEMA COMUM: Return Node Errado

**ERRADO** (não funciona):
```
[Event On Drag Detected]
       │
       ▼
[Create Item Drag Operation]
       │
       ▼
[Return Node] ← SEM CONEXÃO = retorna nullptr!
```

**CORRETO** (funciona):
```
[Event On Drag Detected]
       │
       ▼
[Create Item Drag Operation] ──────► [Return Node]
                                          │
                              Return Value: ● (CONECTADO!)
```

---

## 5️⃣ Indicador Visual de Item Equipado no Inventário

### 5.1 Conceito

Quando um item no inventário está equipado (`bIsEquipped = true`), mostrar um indicador visual como:
- Borda colorida (verde/dourada)
- Ícone de "E" no canto
- Overlay com cor

### 5.2 Adicionar Elementos Visuais no WBP_InventorySlot

No **Designer** do `WBP_InventorySlot`, adicione:

```
[Canvas Panel] (root)
    │
    ├─→ [Border_Slot] (borda normal)
    │       │
    │       └─→ [Image_ItemIcon]
    │
    ├─→ [Border_Equipped] ← NOVO (borda de equipado)
    │       │
    │       └─→ Visibility: Collapsed (por padrão)
    │       └─→ Brush Color: Gold ou Verde
    │       └─→ Is Hit Test Visible: false
    │
    └─→ [Image_EquippedIcon] ← NOVO (ícone "E" ou similar)
            │
            └─→ Visibility: Collapsed (por padrão)
            └─→ Anchor: Top Right
            └─→ Size: 16x16
            └─→ Is Hit Test Visible: false
```

### 5.3 Atualizar Visual ao Definir SlotData

No **Event Graph** do `WBP_InventorySlot`, override ou estenda a função que atualiza o visual:

**Opção A: Criar função UpdateEquippedVisual**

```
┌──────────────────────────────────────────────────────────────┐
│ [UpdateEquippedVisual] (Function)                            │
│     │                                                        │
│     ├─→ Get: SlotData.bIsEquipped                            │
│     │       │                                                │
│     │       └─→ Branch                                       │
│     │               │                                        │
│     │               ├─→ (True - Equipado)                    │
│     │               │       │                                │
│     │               │       ├─→ Set Visibility               │
│     │               │       │   (Border_Equipped) = Visible  │
│     │               │       │                                │
│     │               │       └─→ Set Visibility               │
│     │               │           (Image_EquippedIcon) = Visible│
│     │               │                                        │
│     │               └─→ (False - Não Equipado)               │
│     │                       │                                │
│     │                       ├─→ Set Visibility               │
│     │                       │   (Border_Equipped) = Collapsed │
│     │                       │                                │
│     │                       └─→ Set Visibility               │
│     │                           (Image_EquippedIcon)=Collapsed│
└──────────────────────────────────────────────────────────────┘
```

**Opção B: Usar Event OnSlotDataChanged (se existir)**

```
┌──────────────────────────────────────────────────────────────┐
│ Event On Slot Data Changed (ou após Set Slot Data)           │
│     │                                                        │
│     └─→ Call: UpdateEquippedVisual                           │
└──────────────────────────────────────────────────────────────┘
```

### 5.4 Chamar ao Popular o Inventário

Certifique-se de chamar `UpdateEquippedVisual` sempre que o slot for atualizado:

```
[Após Set Slot Data no InventorySlot]
    │
    └─→ Call: UpdateEquippedVisual
```

---

## 6️⃣ Exemplo de Visual de Equipado

### Opção 1: Borda Dourada

```
┌────────────────┐
│ ╔════════════╗ │  ← Borda dourada
│ ║   🗡️       ║ │  ← Ícone do item
│ ║         [E]║ │  ← Indicador "E"
│ ╚════════════╝ │
└────────────────┘
```

### Opção 2: Overlay Verde

```
┌────────────────┐
│ ┌────────────┐ │
│ │   🗡️   ▓▓▓│ │  ← Overlay verde no canto
│ │        ▓▓▓│ │
│ └────────────┘ │
└────────────────┘
```

### Opção 3: Ícone de Check

```
┌────────────────┐
│ [✓]           │  ← Check no canto superior
│    🗡️         │
│               │
└────────────────┘
```

---

## 7️⃣ Blueprint Detalhado: UpdateEquippedVisual

```
┌──────────────────────────────────────────────────────────────┐
│ FUNCTION: UpdateEquippedVisual                               │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────┐                                             │
│  │ Get SlotData│                                             │
│  └──────┬──────┘                                             │
│         │                                                    │
│         ▼                                                    │
│  ┌─────────────────┐                                         │
│  │ Break Struct    │                                         │
│  │ FUmbraInventory │                                         │
│  │ Slot            │                                         │
│  └────────┬────────┘                                         │
│           │                                                  │
│           │ bIsEquipped                                      │
│           ▼                                                  │
│     ┌──────────┐                                             │
│     │  Branch  │                                             │
│     └────┬─────┘                                             │
│          │                                                   │
│    ┌─────┴─────┐                                             │
│    │           │                                             │
│    ▼           ▼                                             │
│  (True)     (False)                                          │
│    │           │                                             │
│    │           │                                             │
│    ▼           ▼                                             │
│ ┌────────┐  ┌────────┐                                       │
│ │Set Vis │  │Set Vis │                                       │
│ │Visible │  │Collapse│                                       │
│ │Border  │  │Border  │                                       │
│ │Equipped│  │Equipped│                                       │
│ └────────┘  └────────┘                                       │
│    │           │                                             │
│    ▼           ▼                                             │
│ ┌────────┐  ┌────────┐                                       │
│ │Set Vis │  │Set Vis │                                       │
│ │Visible │  │Collapse│                                       │
│ │Image   │  │Image   │                                       │
│ │Equipped│  │Equipped│                                       │
│ │Icon    │  │Icon    │                                       │
│ └────────┘  └────────┘                                       │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## 8️⃣ Atualizar Visual Após Equipar/Desequipar

### 8.1 Bind aos Eventos do GameInstance

No `WBP_Inventory` ou no slot individual:

```
Event Construct
    │
    ├─→ Get Game Instance → Cast to UmbraGameInstance
    │       │
    │       ├─→ Bind Event to OnInventoryLoaded
    │       │       │
    │       │       └─→ [RefreshAllSlots]
    │       │
    │       └─→ Bind Event to OnItemMoved
    │               │
    │               └─→ [RefreshAllSlots]
```

### 8.2 RefreshAllSlots

```
[RefreshAllSlots] (Custom Event)
    │
    ├─→ For Each: InventorySlots
    │       │
    │       └─→ Call: UpdateEquippedVisual
    │
    └─→ (Se tiver Equipment) → Refresh Equipment Slots
```

---

## 9️⃣ Checklist de Implementação

### WBP_EquipmentSlot
- [ ] `bCanDragFrom = true`
- [ ] `bCanDropTo = true`  
- [ ] `bIsStorageSlot = false`
- [ ] `OnMouseButtonDown` → `DetectDragIfPressed`
- [ ] `OnDragDetected` → `CreateItemDragOperation`

### WBP_InventorySlot
- [ ] `bCanDropTo = true`
- [ ] Adicionar `Border_Equipped` (visual)
- [ ] Adicionar `Image_EquippedIcon` (opcional)
- [ ] Criar função `UpdateEquippedVisual`
- [ ] Chamar `UpdateEquippedVisual` após `SetSlotData`

### WBP_Inventory / WBP_CharacterInfo
- [ ] Bind `OnInventoryLoaded` para refresh
- [ ] Bind `OnItemMoved` para refresh
- [ ] Atualizar equipamentos após mudanças

### Testes
- [ ] Arrastar item do EquipmentSlot para InventorySlot vazio
- [ ] Verificar se item é desequipado no servidor
- [ ] Verificar se visual de equipado aparece/desaparece
- [ ] Testar arrastar para slot ocupado (deve trocar ou falhar)

---

## 🔟 Troubleshooting

### "Não consigo arrastar do EquipmentSlot"
- Verifique `bCanDragFrom = true`
- Verifique se `OnMouseButtonDown` tem `DetectDragIfPressed`
- Verifique se `OnDragDetected` chama `CreateItemDragOperation`

### "O drop não funciona no InventorySlot"
- Verifique `bCanDropTo = true` no InventorySlot
- Verifique se `OnDrop` está implementado (herda de UmbraInventorySlotWidget)

### "Item desequipa mas visual não atualiza"
- Certifique-se de chamar `UpdateEquippedVisual` após recarregar inventário
- Bind ao evento `OnInventoryLoaded` do GameInstance

### "Erro: Não é possível mover item equipado para storage"
- Isso é comportamento esperado do servidor
- Itens equipados só podem ir para inventário (0-49), não para storage (50-149)

---

**Pronto!** Siga este guia para implementar o desequipar via drag & drop e o visual de item equipado!
