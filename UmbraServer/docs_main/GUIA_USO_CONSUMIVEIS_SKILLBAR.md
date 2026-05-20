# Guia: Uso de consumíveis e skillbar (UE 5.6.1)

## Visão geral

- **Clique direito** em consumível no inventário → `UseInventoryItem` → `POST /api/inventory/use_item.php`
- **Arrastar** consumível do inventário para slot da skillbar → `SetSkillbarItem` → `POST /api/skills/set_skillbar.php` com `item_template_id`
- **Clique direito** no slot da skillbar com consumível → `UseConsumableByTemplate` (mesmo endpoint `use_item.php` com `item_template_id`)
- **Cooldown** agrupado por `item_subtype` (ex.: `health_potion`), duração em `item_templates.use_cooldown_ms` (padrão 5000 ms)
- HUD atualiza via `OnCharacterInfoLoaded` após uso bem-sucedido

## Banco de dados

Rodar (idempotente):

```bash
mysql -u root -p umbra_eternum < UmbraServer/scripts_main/add_consumable_use_system.sql
```

Alterações:

| Objeto | Descrição |
|--------|-----------|
| `item_templates.use_cooldown_ms` | Cooldown em ms por template (agrupado por subtype no PHP) |
| `player_consumable_cooldowns` | `(player_id, item_subtype)` → `expires_at_ms` |
| `player_skillbar.item_template_id` | Consumível no slot (mutuamente exclusivo com `skill_id`) |

## API PHP

### `POST /api/inventory/use_item.php`

Body (um dos identificadores de pilha):

```json
{
  "token": "...",
  "player_id": 1,
  "inventory_id": 123
}
```

Ou pela skillbar:

```json
{
  "token": "...",
  "player_id": 1,
  "item_template_id": 7
}
```

Resposta de sucesso inclui `health`, `max_health`, `mana`, `max_mana`, `quantity_remaining`, `cooldown_expires_at_ms`, `item_subtype`.

### `POST /api/skills/set_skillbar.php`

Atribuir consumível (limpa `skill_id`):

```json
{
  "token": "...",
  "player_id": 1,
  "slot_index": 5,
  "skill_id": 0,
  "item_template_id": 7
}
```

### `POST /api/skills/get_skillbar.php`

Slots podem retornar `item_template_id` e objeto `item` (`item_name`, `icon_path`, `item_subtype`, `max_stack_size`).

## Cliente UE (C++)

| Arquivo | Função |
|---------|--------|
| `UmbraGameInstance` | `UseInventoryItem`, `UseConsumableByTemplate`, `SetSkillbarItem`, `IsConsumableOnCooldown` |
| `UmbraInventorySlotWidget` | Clique direito em consumível |
| `UmbraSkillSlotWidget` | Drop de `UUmbraItemDragDropOperation`, clique direito em item/skill |
| `UmbraSkillDataStructures.h` | `EUmbraSkillbarEntryType`, campos em `FUmbraSkillbarSlot` |

Delegates: `OnItemUsed`, `OnItemUseFailed`, `OnConsumableCooldownStarted`.

## Teste de aceite

1. Poção id **7** com `stats_json = {"health_restore": 50}`, `use_cooldown_ms = 5000`
2. HP 150 / max calculado → clique direito no inventário → HP +50, pilha -1
3. Repetir em &lt; 5 s → mensagem de cooldown, sem decremento
4. Arrastar poção para slot 5 da skillbar → ícone persiste após relog
5. Clique direito no slot 5 → mesmo efeito e cooldown
6. `WBP_PlayerHUD` reflete HP/MP via `OnCharacterInfoLoaded`

## Blueprint

`WBP_SkillSlot` deve herdar de `UUmbraSkillSlotWidget`. Veja o guia completo de hotkeys, quantidade e cooldown:

**[GUIA_SKILLBAR_HOTKEYS_QUANTIDADE_COOLDOWN.md](GUIA_SKILLBAR_HOTKEYS_QUANTIDADE_COOLDOWN.md)**

Resumo: adicionar `Quantity_Text`, `Keybind_Button`, criar `WBP_HotkeyCapture`, configurar `HotkeyCaptureWidgetClass` no GameInstance, e overlay `Cooldown_*_Inv` no inventário.
