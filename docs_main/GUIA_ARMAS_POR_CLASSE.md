# Guia: itens por classe (armas e armaduras)

Restrição autoritativa no equip: coluna JSON `item_templates.allowed_class_ids`.

- `null` ou `[]` → **todas** as classes podem equipar
- `[1, 2]` → só essas `class_id`

Aplica-se a **weapon** e **armor** com `equipment_slot != none`. Consumíveis/materiais não são restringidos.

Off-hand / escudo do Templar fica para sequência futura.

## Mapa canônico (main_hand — subtype de arma)

| class_id | Nome DB | Label (Manager / tooltip) | subtype arma |
|----------|---------|---------------------------|--------------|
| 1 | Barbarian | Barbarian | `axe` |
| 2 | Templar | Templar | `sword` |
| 3 | DarkMage | Dark Mage | `scythe` |
| 4 | Cleric | Alchemist | `staff` |
| 5 | Assassin | Assassin | `dagger` |
| 6 | Monk | Martial | `cestus` |

Armaduras usam a mesma coluna; no Manager **não** há pré-seleção por subtype — só “Todas as classes” ou multi-select manual.

## SQL

1. Migration (idempotente):

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_item_allowed_class_ids.sql
```

2. Seed armas de teste:

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/seed_class_test_weapons.sql
```

3. Seed armaduras de teste:

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/seed_class_test_armor.sql
```

| Item | Restrição |
|------|-----------|
| Test Axe (Barbarian) | `[1]` |
| … (outras armas de teste) | por classe |
| Test Universal Blade | `[]` (todas) |
| Test Chest (Barbarian) | `[1]` |
| Test Universal Chest | `[]` (todas) |

## API

Helper: `www/umbra_api/helpers/item_weapon_class_helper.php`

- `item_can_be_equipped_by_class` — validação no equip
- `append_allowed_class_fields` / `item_templates_allowed_class_select_sql` — listagens

Equip: `equip_item.php` → 403 `"Este item não pode ser usado pela sua classe"`

Admin create/update/list: `allowed_class_ids` (+ `allow_all_classes`)

Listagens que devolvem `allowed_class_ids` / `allow_all_classes`:

| Endpoint / helper | Uso |
|-------------------|-----|
| `get_inventory.php` | Bolsa |
| `get_storage.php` | Armazém |
| `get_item_templates.php` | Cache / loot |
| `get_npc_vendor_catalog.php` | Compra NPC |
| `get_personal_shop.php` | Loja pessoal (comprador) |
| `list_auction_listings.php` / `my_auction_listings.php` | Leilão |
| `mailFetchAttachments` | Anexos do correio |
| `get_trade_state.php` | Trade |
| `character_info_helper` (equipped) | Login / inspect (evita poluir cache) |
| `quest_helper` rewards/choices/grants | Quests (choices multi-grupo; sem filtro por classe — jogador pode pegar qualquer opção) |

Seed de quest demo (arma+armadura por classe): `www/umbra_api/scripts/seed_quest_class_gear_choice.sql`. Ver também `GUIA_SISTEMA_QUESTS_UE561.md` §2.1.

## Manager (WPF)

Editor de itens: **Classes permitidas (armas/armaduras)** — checkbox **Todas as classes**; se desmarcado, multi-select das 6 classes. Em armas, mudar subtype pode pré-selecionar a classe default.

## Cliente UE

- `FUmbraItemTemplate`: `AllowedClassIds`, `bAllowAllClasses`
- Listings (`FUmbraNpcVendorStockEntry`, personal shop, auction, trade) também carregam classe; `CreateInventorySlotFrom*` sobrescreve o template do slot
- `MergeAllowedClassesIntoItemCache` ao parsear listagens / quests / mail
- Loot: `RequestLootItemTemplatesIfNeeded` refetch se stub ou weapon/armor com restrição incompleta no cache
- Tooltip: `Text_AllowedClasses` (BindWidgetOptional em `WBP_ItemTooltipRefined`)
- Slots: tint vermelho se `!CanPlayerEquipItemByClass`
- Autoridade continua na API

## Teste manual sugerido

1. Rodar migration + seeds
2. Vendor: abrir com Test Axe/Chest **sem** o item na bolsa → tint vermelho na classe errada
3. Loja pessoal / leilão / mail / trade: listing restrito → mesmo tint
4. Login com equipado restrito → vendor subsequente já mostra restrição (cache não poluído)
5. Universal → sem tint em todos os contextos
6. Barbarian equipa Test Axe → OK; Assassin → 403
