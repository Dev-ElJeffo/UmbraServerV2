# Guia Completo: Sistema de Gold e Drag & Drop de Equipamentos

Este guia cobre a implementação completa de:
1. ✅ Sistema de Gold (inventário ↔ armazém)
2. ✅ Drag & Drop de itens equipados
3. ✅ Botão para deletar itens

---

## 📋 Pré-requisitos

- Projeto C++ compilado com as últimas alterações
- Acesso ao banco de dados MySQL
- Blueprints de inventário e storage já funcionando

---

## 1️⃣ Configurar Banco de Dados (Gold)

### Executar no MySQL Workbench:

```sql
-- 1. Adicionar coluna gold na tabela players
ALTER TABLE players ADD COLUMN gold BIGINT UNSIGNED NOT NULL DEFAULT 0;

-- 2. Adicionar coluna stored_gold na tabela accounts
ALTER TABLE accounts ADD COLUMN stored_gold BIGINT UNSIGNED NOT NULL DEFAULT 0;

-- 3. (Opcional) Dar gold inicial para teste
UPDATE players SET gold = 1000 WHERE id = 1;
UPDATE accounts SET stored_gold = 500 WHERE id = 1;
```

### Verificar se funcionou:

```sql
SELECT id, character_name, gold FROM players;
SELECT id, username, stored_gold FROM accounts;
```

---

## 2️⃣ Sistema de Gold no Blueprint

### 2.1 Adicionar Variáveis de Exibição

No seu Widget de Inventário ou Storage (`WBP_Inventory` ou `WBP_Storage`):

1. Adicione dois **Text Blocks**:
   - `Text_PlayerGold` - Exibir gold do jogador
   - `Text_StoredGold` - Exibir gold no armazém

2. Adicione dois **Buttons** (ou um Editable Text Box + Button):
   - `Button_DepositGold`
   - `Button_WithdrawGold`

3. (Opcional) Adicione um **Editable Text Box**:
   - `TextBox_GoldAmount` - Para digitar a quantidade

### 2.2 Carregar Gold ao Abrir

No **Event Construct** do Widget:

```
Event Construct
    ↓
Get Game Instance → Cast to UmbraGameInstance
    ↓
Call: Load Gold
    ↓
Bind Event to On Gold Loaded
    ↓
[Event] On Gold Loaded (Player Gold, Stored Gold)
    ↓
Set Text (Text_PlayerGold) → Format: "{0} G"
Set Text (Text_StoredGold) → Format: "{0} G"
```

### 2.3 Depositar Gold

No **OnClicked** do `Button_DepositGold`:

```
OnClicked (Button_DepositGold)
    ↓
Get Text (TextBox_GoldAmount) → To Integer
    ↓
Get Game Instance → Cast to UmbraGameInstance
    ↓
Call: Deposit Gold (Amount)
```

### 2.4 Sacar Gold

No **OnClicked** do `Button_WithdrawGold`:

```
OnClicked (Button_WithdrawGold)
    ↓
Get Text (TextBox_GoldAmount) → To Integer
    ↓
Get Game Instance → Cast to UmbraGameInstance
    ↓
Call: Withdraw Gold (Amount)
```

### 2.5 Atualizar UI após Operações

Bind aos eventos de sucesso:

```
Bind Event to On Gold Deposited
    ↓
[Event] On Gold Deposited (Amount, New Player Gold, New Stored Gold)
    ↓
Set Text (Text_PlayerGold)
Set Text (Text_StoredGold)
(Opcional) Show Message: "Depositado {Amount} gold!"

Bind Event to On Gold Withdrawn
    ↓
[Event] On Gold Withdrawn (Amount, New Player Gold, New Stored Gold)
    ↓
Set Text (Text_PlayerGold)
Set Text (Text_StoredGold)
(Opcional) Show Message: "Sacado {Amount} gold!"

Bind Event to On Gold Operation Failed
    ↓
[Event] On Gold Operation Failed (Error Message)
    ↓
Show Error Message
```

### 2.6 Diagrama Visual

```
┌─────────────────────────────────────────────────────────┐
│                    WBP_Storage                          │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐              │
│  │  Inventário: G  │  │   Armazém: G    │              │
│  │    [1000]       │  │     [500]       │              │
│  └─────────────────┘  └─────────────────┘              │
│                                                         │
│  Quantidade: [________]                                 │
│                                                         │
│  [Depositar Gold]     [Sacar Gold]                     │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │              SLOTS DO STORAGE                    │   │
│  │  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐           │   │
│  │  │    │ │    │ │    │ │    │ │    │ ...       │   │
│  │  └────┘ └────┘ └────┘ └────┘ └────┘           │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

## 3️⃣ Drag & Drop de Itens Equipados

### 3.1 Conceito

Itens equipados podem ser **arrastados para o inventário** para serem desequipados automaticamente. O servidor já lida com isso - ao mover um item equipado para um slot do inventário (0-49), ele é automaticamente desequipado.

### 3.2 Criar Widget de Slot de Equipamento

Se você ainda não tem, crie `WBP_EquipmentSlot`:

1. **Create Widget Blueprint** → Parent: `UmbraInventorySlotWidget`
2. Configure o visual (borda dourada, ícone de slot vazio, etc.)

### 3.3 Configurar o Slot de Equipamento

No **Class Defaults** ou **Construction Script**:

```
bCanDragFrom = true      ← CRÍTICO: Permitir arrastar
bCanDropTo = true        ← Permitir soltar itens aqui
bIsStorageSlot = false   ← NÃO é storage
```

### 3.4 Popular Slots de Equipamento

No Widget de Equipamento (`WBP_Equipment`):

```
Event Construct
    ↓
Get Game Instance → Get Current Inventory
    ↓
For Each Loop (Item in Current Inventory)
    ↓
Branch: Item.bIsEquipped == true?
    ↓ (True)
Get Equipment Slot by Type (Item.ItemTemplate.EquipmentSlot)
    ↓
Call: Set Slot Data (Item)
```

### 3.5 Mapeamento de Slots de Equipamento

Crie uma função para mapear tipo de equipamento → widget:

```cpp
// Exemplo de slots:
// Head (Helmet)     → WBP_EquipmentSlot_Head
// Chest (Armor)     → WBP_EquipmentSlot_Chest
// MainHand (Weapon) → WBP_EquipmentSlot_MainHand
// OffHand (Shield)  → WBP_EquipmentSlot_OffHand
// Legs              → WBP_EquipmentSlot_Legs
// Feet              → WBP_EquipmentSlot_Feet
// Ring1             → WBP_EquipmentSlot_Ring1
// Ring2             → WBP_EquipmentSlot_Ring2
// Necklace          → WBP_EquipmentSlot_Necklace
```

### 3.6 Fluxo de Drag do Equipamento → Inventário

```
[Slot de Equipamento com Item]
         │
         │ (Arrastar)
         ▼
    Detecta Drag
         │
         │ CreateItemDragOperation()
         ▼
    Cria DragOperation
         │
         │ (Visual segue o mouse)
         ▼
    Soltar no Inventário
         │
         │ ProcessItemDropFromOperation()
         ▼
    RequestMoveItemByID()
         │
         │ (Chama move_item.php)
         ▼
    Servidor: is_equipped = 0
         │
         ▼
    Item desequipado e no slot do inventário!
```

### 3.7 Restrições do Servidor

O arquivo `move_item.php` já tem estas regras:

| Origem | Destino | Permitido? |
|--------|---------|------------|
| Equipado | Inventário (0-49) | ✅ Sim (desequipa) |
| Equipado | Storage (50-149) | ❌ Não |
| Inventário | Equipamento | ✅ Via equip_item.php |

### 3.8 Equipar Item do Inventário

Para equipar, use a função `EquipItem`:

```
[Slot do Inventário]
         │
         │ (Double Click ou Botão Direito)
         ▼
Get Game Instance
         │
         │ Call: Equip Item (InventoryID)
         ▼
Servidor equipa o item
         │
         ▼
Reload Inventory + Equipment
```

---

## 4️⃣ Botão de Deletar Item

### 4.1 Opção A: Botão no Tooltip/Context Menu

Quando o jogador clica com botão direito no item:

```
OnMouseButtonDown (Right Click)
    ↓
Show Context Menu
    ↓
[Usar] [Equipar] [Deletar] [Cancelar]
    ↓
OnClicked (Button_Delete)
    ↓
Show Confirmation Dialog: "Deletar {ItemName}?"
    ↓
OnConfirmed
    ↓
Get Game Instance → Remove Item (InventoryID, 0)
```

### 4.2 Opção B: Área de "Lixeira"

1. Crie um widget `WBP_TrashSlot` que aceita drops
2. Configure `bCanDropTo = true`

```
ProcessItemDropFromOperation (no TrashSlot)
    ↓
Show Confirmation: "Deseja deletar este item?"
    ↓
OnConfirmed
    ↓
Get Game Instance → Remove Item (InventoryID, 0)
```

### 4.3 Código do Evento de Remoção

No Blueprint, bind ao evento de remoção:

```
Bind Event to On Item Removed
    ↓
[Event] On Item Removed (InventoryID, Removed Quantity)
    ↓
Reload Inventory UI
Show Message: "Item removido!"

Bind Event to On Item Remove Failed
    ↓
[Event] On Item Remove Failed (Error Message)
    ↓
Show Error: Error Message
```

---

## 5️⃣ Funções C++ Disponíveis

### GameInstance - Gold

| Função | Descrição |
|--------|-----------|
| `LoadGold()` | Carrega saldos do servidor |
| `DepositGold(Amount)` | Deposita gold no armazém |
| `WithdrawGold(Amount)` | Saca gold do armazém |

### GameInstance - Propriedades de Gold

| Propriedade | Tipo | Descrição |
|-------------|------|-----------|
| `PlayerGold` | int64 | Gold atual do jogador |
| `StoredGold` | int64 | Gold no armazém |

### GameInstance - Delegates de Gold

| Delegate | Parâmetros |
|----------|------------|
| `OnGoldLoaded` | (PlayerGold, StoredGold) |
| `OnGoldDeposited` | (Amount, NewPlayerGold, NewStoredGold) |
| `OnGoldWithdrawn` | (Amount, NewPlayerGold, NewStoredGold) |
| `OnGoldOperationFailed` | (ErrorMessage) |

### GameInstance - Inventory

| Função | Descrição |
|--------|-----------|
| `RemoveItem(InventoryID, Quantity)` | Remove item (0 = tudo) |
| `EquipItem(InventoryID)` | Equipa um item |
| `UnequipItem(InventoryID)` | Desequipa um item |
| `MoveItem(InventoryID, TargetSlot)` | Move item |

---

## 6️⃣ APIs PHP Disponíveis

### Gold

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/api/gold/get_gold.php` | POST | Busca saldos |
| `/api/gold/deposit_gold.php` | POST | Deposita gold |
| `/api/gold/withdraw_gold.php` | POST | Saca gold |

### Inventory

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/api/inventory/remove_item.php` | POST | Remove item |
| `/api/inventory/equip_item.php` | POST | Equipa/desequipa |
| `/api/inventory/move_item.php` | POST | Move item |

---

## 7️⃣ Checklist de Implementação

### Banco de Dados
- [ ] Executar `ALTER TABLE players ADD COLUMN gold...`
- [ ] Executar `ALTER TABLE accounts ADD COLUMN stored_gold...`
- [ ] Verificar colunas criadas

### Blueprint - Gold
- [ ] Adicionar Text Blocks para exibir gold
- [ ] Adicionar botões Depositar/Sacar
- [ ] Adicionar campo para quantidade
- [ ] Bind `OnGoldLoaded`
- [ ] Bind `OnGoldDeposited`
- [ ] Bind `OnGoldWithdrawn`
- [ ] Bind `OnGoldOperationFailed`
- [ ] Chamar `LoadGold()` no Construct

### Blueprint - Equipamento
- [ ] Criar/configurar slots de equipamento
- [ ] Configurar `bCanDragFrom = true`
- [ ] Popular slots com itens equipados
- [ ] Testar drag → inventário

### Blueprint - Deletar Item
- [ ] Implementar menu de contexto ou lixeira
- [ ] Adicionar confirmação
- [ ] Bind `OnItemRemoved`
- [ ] Bind `OnItemRemoveFailed`

### Testes
- [ ] Depositar gold
- [ ] Sacar gold
- [ ] Tentar depositar mais gold do que tem
- [ ] Tentar sacar mais gold do que tem no armazém
- [ ] Arrastar item equipado para inventário
- [ ] Deletar item
- [ ] Tentar deletar item equipado (deve falhar)

---

## 8️⃣ Troubleshooting

### "Gold insuficiente"
- Verifique se `PlayerGold` está atualizado
- Chame `LoadGold()` antes de operar

### "Não é possível remover item equipado"
- Desequipe o item primeiro usando `UnequipItem()`
- Ou arraste para o inventário (desequipa automaticamente)

### Item equipado não aparece no slot de equipamento
- Verifique se `bIsEquipped = true` no banco
- Verifique se está filtrando corretamente por `EquipmentSlot`

### Drag não funciona no slot de equipamento
- Verifique `bCanDragFrom = true`
- Verifique se o slot tem item (`!IsSlotEmpty()`)

---

## 9️⃣ Exemplo Completo de Blueprint

### Event Graph do WBP_Storage (Gold)

```
┌──────────────────────────────────────────────────────────────┐
│ Event Construct                                              │
│     │                                                        │
│     ├─→ Get Game Instance                                    │
│     │       │                                                │
│     │       └─→ Cast to UmbraGameInstance                    │
│     │               │                                        │
│     │               ├─→ Load Gold                            │
│     │               │                                        │
│     │               ├─→ Bind Event to OnGoldLoaded           │
│     │               │       │                                │
│     │               │       └─→ [UpdateGoldDisplay]          │
│     │               │                                        │
│     │               ├─→ Bind Event to OnGoldDeposited        │
│     │               │       │                                │
│     │               │       └─→ [UpdateGoldDisplay]          │
│     │               │                                        │
│     │               └─→ Bind Event to OnGoldWithdrawn        │
│     │                       │                                │
│     │                       └─→ [UpdateGoldDisplay]          │
│     │                                                        │
│     └─→ Load Storage (existing code)                         │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ [UpdateGoldDisplay] (Custom Event)                           │
│     │                                                        │
│     ├─→ Set Text (Text_PlayerGold)                           │
│     │       └─→ Format Text: "{0} G" with PlayerGold         │
│     │                                                        │
│     └─→ Set Text (Text_StoredGold)                           │
│             └─→ Format Text: "{0} G" with StoredGold         │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ OnClicked (Button_DepositGold)                               │
│     │                                                        │
│     ├─→ Get Text (TextBox_GoldAmount)                        │
│     │       │                                                │
│     │       └─→ To Integer (Amount)                          │
│     │               │                                        │
│     │               └─→ Branch: Amount > 0?                  │
│     │                       │                                │
│     │                       └─→ (True)                       │
│     │                               │                        │
│     │                               └─→ Get Game Instance    │
│     │                                       │                │
│     │                                       └─→ Deposit Gold │
└──────────────────────────────────────────────────────────────┘
```

---

**Pronto!** Siga este guia passo a passo para implementar todas as funcionalidades. Se tiver dúvidas em alguma etapa específica, pergunte!

