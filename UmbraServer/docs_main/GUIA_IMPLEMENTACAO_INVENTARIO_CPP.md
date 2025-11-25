# Guia de Implementação - Sistema de Inventário C++

## Resumo do Progresso

✅ **Completo**:
1. Tabelas MySQL (`item_templates`, `player_inventory`)
2. Endpoints PHP REST (GET, ADD, REMOVE, MOVE, EQUIP)
3. Estruturas de dados C++ (`FUmbraItemStats`, `FUmbraItemTemplate`, `FUmbraInventorySlot`)
4. Enums C++ (`EUmbraItemType`, `EUmbraItemRarity`, `EUmbraEquipmentSlot`)
5. Delegates C++ (eventos de inventário)
6. Declarações de funções no `UmbraGameInstance.h`

🚧 **Em andamento**:
- Implementação das funções no `UmbraGameInstance.cpp`

⏳ **Pendente**:
- Widget Blueprint UI (`WBP_Inventory`)
- Testes do sistema completo

---

## Estruturas Criadas

### 1. Enums

#### `EUmbraItemType`
```cpp
enum class EUmbraItemType : uint8
{
	Weapon,      // Arma
	Armor,       // Armadura
	Consumable,  // Consumível
	Material,    // Material
	Quest,       // Quest
	Misc         // Diversos
};
```

#### `EUmbraItemRarity`
```cpp
enum class EUmbraItemRarity : uint8
{
	Common,      // Comum (branco)
	Uncommon,    // Incomum (verde)
	Rare,        // Raro (azul)
	Epic,        // Épico (roxo)
	Legendary    // Lendário (laranja)
};
```

#### `EUmbraEquipmentSlot`
```cpp
enum class EUmbraEquipmentSlot : uint8
{
	None, Head, Chest, Legs, Feet, Hands,
	MainHand, OffHand, Ring, Amulet
};
```

### 2. Structs

#### `FUmbraItemStats`
Armazena todas as estatísticas de um item (damage, defense, buffs, etc.)

#### `FUmbraItemTemplate`
Definição base de um item no jogo (corresponde à tabela `item_templates`)

#### `FUmbraInventorySlot`
Instância de um item no inventário do jogador (corresponde à tabela `player_inventory`)

---

## Delegates (Eventos)

### Eventos de Sucesso
- `FOnInventoryLoaded`: Inventário carregado com sucesso
- `FOnItemAdded(InventoryID, ItemSlot)`: Item adicionado
- `FOnItemRemoved(InventoryID, RemovedQuantity)`: Item removido
- `FOnItemMoved(InventoryID, FromSlot, ToSlot)`: Item movido
- `FOnItemEquipped(InventoryID, EquipmentSlot)`: Item equipado
- `FOnItemUnequipped(InventoryID, EquipmentSlot)`: Item desequipado

### Eventos de Falha
- `FOnInventoryLoadFailed(ErrorMessage)`
- `FOnItemAddFailed(ErrorMessage)`
- `FOnItemRemoveFailed(ErrorMessage)`
- `FOnItemMoveFailed(ErrorMessage)`
- `FOnItemEquipFailed(ErrorMessage)`

---

## Funções Públicas (Blueprint Callable)

### 1. `LoadInventory()`
**Descrição**: Carrega o inventário completo do jogador ativo via API REST.

**Fluxo**:
1. Verificar se há jogador ativo
2. Criar requisição GET para `/api/inventory/get_inventory.php`
3. Adicionar header `Authorization: Bearer {JWT}`
4. Processar resposta em `OnLoadInventoryRequestComplete`

**Callback**: `OnLoadInventoryRequestComplete`
- Parsear array JSON `inventory`
- Para cada item:
  - Converter JSON → `FUmbraInventorySlot`
  - Adicionar ao `CurrentInventory`
- Atualizar `EquippedItems` map
- Disparar `OnInventoryLoaded`

---

### 2. `AddItem(ItemTemplateID, Quantity, SlotIndex)`
**Descrição**: Adiciona um item ao inventário.

**Parâmetros**:
- `ItemTemplateID`: ID do template do item
- `Quantity`: Quantidade (padrão: 1)
- `SlotIndex`: Slot específico (-1 para auto-atribuir)

**Fluxo**:
1. Verificar autenticação
2. Criar requisição POST para `/api/inventory/add_item.php`
3. Body JSON:
```json
{
  "item_template_id": 7,
  "quantity": 5,
  "slot_index": null
}
```
4. Processar resposta em `OnAddItemRequestComplete`

**Callback**: `OnAddItemRequestComplete`
- Parsear item adicionado do JSON
- Se `stacked == true`: atualizar slot existente
- Se `stacked == false`: adicionar novo slot
- Disparar `OnItemAdded(InventoryID, ItemSlot)`

---

### 3. `RemoveItem(InventoryID, Quantity)`
**Descrição**: Remove um item (ou quantidade) do inventário.

**Parâmetros**:
- `InventoryID`: ID da instância do item
- `Quantity`: Quantidade a remover (0 = tudo)

**Fluxo**:
1. Verificar se o item existe localmente
2. Criar requisição POST para `/api/inventory/remove_item.php`
3. Body JSON:
```json
{
  "inventory_id": 25,
  "quantity": 2
}
```
4. Processar resposta em `OnRemoveItemRequestComplete`

**Callback**: `OnRemoveItemRequestComplete`
- Se `remaining_quantity == 0`: remover slot do array
- Senão: atualizar quantidade
- Atualizar `EquippedItems` se necessário
- Disparar `OnItemRemoved(InventoryID, RemovedQuantity)`

---

### 4. `MoveItem(InventoryID, TargetSlotIndex)`
**Descrição**: Move um item para outro slot (ou faz swap).

**Parâmetros**:
- `InventoryID`: ID do item a mover
- `TargetSlotIndex`: Slot de destino (0-49)

**Fluxo**:
1. Validar slot index (0-49)
2. Criar requisição POST para `/api/inventory/move_item.php`
3. Body JSON:
```json
{
  "inventory_id": 10,
  "target_slot_index": 25
}
```
4. Processar resposta em `OnMoveItemRequestComplete`

**Callback**: `OnMoveItemRequestComplete`
- Atualizar `SlotIndex` dos itens movidos
- Se houver swap, atualizar ambos
- Disparar `OnItemMoved(InventoryID, FromSlot, ToSlot)`

---

### 5. `EquipItem(InventoryID)` / `UnequipItem(InventoryID)`
**Descrição**: Equipa ou desequipa um item.

**Fluxo**:
1. Obter slot do item localmente
2. Verificar se é equipável
3. Criar requisição POST para `/api/inventory/equip_item.php`
4. Body JSON:
```json
{
  "inventory_id": 10,
  "equip": true
}
```
5. Processar resposta em `OnEquipItemRequestComplete`

**Callback**: `OnEquipItemRequestComplete`
- Atualizar `bIsEquipped` do item
- Atualizar `EquippedItems` map
- Se `unequipped_other == true`: remover item anterior do map
- Disparar `OnItemEquipped` ou `OnItemUnequipped`

---

## Funções de Consulta (Blueprint Pure)

### `GetInventorySlot(InventoryID, OutSlot) → bool`
Busca um slot por `InventoryID`.

### `GetInventorySlotByIndex(SlotIndex, OutSlot) → bool`
Busca um slot por índice (0-49).

### `GetEquippedItem(EquipmentSlot, OutSlot) → bool`
Retorna o item equipado em um slot específico (ex: `MainHand`, `Chest`).

### `IsSlotEmpty(SlotIndex) → bool`
Verifica se um slot está vazio.

### `GetOccupiedSlotsCount() → int32`
Retorna o número de slots ocupados.

---

## Funções Helper Privadas

### 1. `ParseItemType(TypeString) → EUmbraItemType`
Converte string (`"weapon"`, `"armor"`, etc.) para enum.

**Mapeamento**:
```cpp
"weapon" → EUmbraItemType::Weapon
"armor" → EUmbraItemType::Armor
"consumable" → EUmbraItemType::Consumable
"material" → EUmbraItemType::Material
"quest" → EUmbraItemType::Quest
"misc" → EUmbraItemType::Misc (padrão)
```

### 2. `ParseItemRarity(RarityString) → EUmbraItemRarity`
Converte string (`"common"`, `"rare"`, etc.) para enum.

### 3. `ParseEquipmentSlot(SlotString) → EUmbraEquipmentSlot`
Converte string (`"main_hand"`, `"chest"`, etc.) para enum.

**Mapeamento**:
```cpp
"head" → EUmbraEquipmentSlot::Head
"chest" → EUmbraEquipmentSlot::Chest
"legs" → EUmbraEquipmentSlot::Legs
"feet" → EUmbraEquipmentSlot::Feet
"hands" → EUmbraEquipmentSlot::Hands
"main_hand" → EUmbraEquipmentSlot::MainHand
"off_hand" → EUmbraEquipmentSlot::OffHand
"ring" → EUmbraEquipmentSlot::Ring
"amulet" → EUmbraEquipmentSlot::Amulet
"none" → EUmbraEquipmentSlot::None (padrão)
```

### 4. `ParseItemStats(StatsObject) → FUmbraItemStats`
Parseia o objeto JSON `stats` para a struct `FUmbraItemStats`.

**Campos a extrair**:
- `damage`, `attack_speed`, `defense`, `fire_damage`
- `health`, `mana`, `speed` (bônus de atributos)
- `health_restore`, `mana_restore` (restauração)
- `strength_buff`, `duration` (buffs temporários)

### 5. `ParseItemTemplate(ItemObject) → FUmbraItemTemplate`
Parseia um objeto JSON completo de template de item.

**Campos a extrair**:
- `item_id`, `item_name`, `item_description`
- `item_type`, `item_subtype`
- `icon_path`
- `max_stack_size`, `equipment_slot`, `required_level`
- `stats` (usar `ParseItemStats`)
- `rarity`, `value`, `weight`

### 6. `ParseInventorySlot(SlotObject) → FUmbraInventorySlot`
Parseia um slot de inventário completo do JSON da API.

**Campos a extrair**:
- `inventory_id`, `player_id`, `item_template_id`
- `quantity`, `slot_index`, `is_equipped`, `durability`
- `acquired_at`
- **Objeto aninhado**: todos os campos do `item_template` (usar `ParseItemTemplate`)

### 7. `UpdateEquippedItemsMap()`
Reconstrói o mapa `EquippedItems` a partir do `CurrentInventory`.

**Lógica**:
```cpp
EquippedItems.Empty();
for (const FUmbraInventorySlot& Slot : CurrentInventory)
{
    if (Slot.bIsEquipped && Slot.ItemTemplate.EquipmentSlot != EUmbraEquipmentSlot::None)
    {
        EquippedItems.Add(Slot.ItemTemplate.EquipmentSlot, Slot);
    }
}
```

---

## Exemplo de Uso em Blueprint

### Carregar Inventário ao Entrar no Jogo
```
Event BeginPlay
  → Get Game Instance (cast to UmbraGameInstance)
  → Bind Event to OnInventoryLoaded
  → LoadInventory()

On Inventory Loaded:
  → For Each (CurrentInventory)
      → Create Widget (WBP_InventorySlot)
      → Set Item Data
      → Add to Grid Panel
```

### Adicionar Item (Drop de Loot)
```
On Enemy Killed:
  → Get Game Instance
  → AddItem(ItemTemplateID = 7, Quantity = 1)

On Item Added:
  → Show Notification ("Você obteve: {ItemName}")
  → Play Sound (Loot_Pickup)
```

### Equipar Item (Clique Duplo no Slot)
```
On Inventory Slot Double-Click:
  → Get Game Instance
  → EquipItem(InventoryID)

On Item Equipped:
  → Update Character Mesh
  → Recalculate Stats
  → Update Equipment UI
```

---

## Próximos Passos

### 1. Implementar Funções em `UmbraGameInstance.cpp` ✅
- ✅ Funções públicas (LoadInventory, AddItem, etc.)
- ✅ Callbacks de requisições HTTP
- ✅ Funções helper de parse

### 2. Criar Widget Blueprint `WBP_Inventory` 🚧
- Grid de 50 slots (5x10 ou 10x5)
- Drag & drop entre slots
- Tooltips com informações do item
- Botões de equipar/usar/descartar
- Visual de raridade (cores)

### 3. Criar Widget Blueprint `WBP_InventorySlot` 🚧
- Imagem do ícone do item
- Texto de quantidade (se empilhável)
- Barra de durabilidade (se aplicável)
- Borda com cor da raridade
- Suporte a drag & drop

### 4. Testes ⏳
- Testar carregamento de inventário
- Testar adição/remoção de itens
- Testar movimentação e swap
- Testar equipar/desequipar
- Testar empilhamento
- Testar limites (50 slots, max_stack_size)

---

## Notas de Implementação

### Performance
- `CurrentInventory` é um `TArray`, então buscar por `InventoryID` é O(n)
- Para melhor performance em buscas frequentes, considerar adicionar um `TMap<int32, int32>` (InventoryID → Array Index)
- `EquippedItems` é um `TMap`, então busca por slot é O(1)

### Sincronização
- **API é a fonte da verdade**: sempre confiar na resposta da API
- **Atualização local**: atualizar arrays locais apenas após confirmação da API
- **Rollback**: se requisição falhar, não modificar arrays locais

### Thread Safety
- Todas as requisições HTTP são assíncronas
- Callbacks são chamados na Game Thread (seguro)
- Não é necessário sincronização adicional

### Validação
- Sempre validar `bIsAuthenticated` antes de operações de inventário
- Sempre validar `HasActiveCharacter()` antes de LoadInventory
- Validar limites de slot (0-49) antes de MoveItem

---

**Data de criação**: 14/11/2025  
**Versão**: 1.0  
**Autor**: Dev-ElJeffo

