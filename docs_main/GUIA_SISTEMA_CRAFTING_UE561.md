# Guia — Sistema de Crafting (UE 5.6.1)

## Visão geral

- **1 widget** (`UUmbraCraftingWidget` / `WBP_CraftingMenu`): lista receitas → grade de 5 ingredientes → Craft / Aprender.
- Categorias: `material` | `consumable` | `equipment`.
- Receita **padrão** (`recipe_item_template_id` NULL): sempre listada.
- **material/consumable** com scroll: consome 1 scroll em `learn_recipe` → unlock permanente (`player_known_craft_recipes`).
- **equipment** com scroll: **sem unlock**; cada craft consome 1 scroll × `craft_count`.
- Sucesso 100%, custo em ouro, batch via `craft_count`, inventário cheio bloqueia.

## Setup DB

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/create_crafting_system.sql
```

Cria tabelas + seed (6 receitas: 2 por categoria).

## APIs jogador (JWT)

| Endpoint | Body principal |
|----------|----------------|
| `POST /api/crafting/list_recipes.php` | `token`, `player_id`, `craft_category?` |
| `POST /api/crafting/get_recipe.php` | `token`, `player_id`, `recipe_id` |
| `POST /api/crafting/learn_recipe.php` | `token`, `player_id`, `recipe_id` |
| `POST /api/crafting/craft.php` | `token`, `player_id`, `recipe_id`, `craft_count`, `ingredient_slots?` `[{slot_index, inventory_id}]` |

## Admin / UmbraManager

- Aba **Crafting** (content).
- PHP: `list/create/update/delete_craft_recipe.php` + `craft_admin_helpers.php`.
- Campos: key, nome, categoria, resultado, qty, gold, level, recipe_item (opcional), classes, ativos, até 5 ingredientes.

## Cliente UE — C++

| Arquivo | Papel |
|---------|--------|
| `UI/UmbraCraftingWidget.h/.cpp` | UI: lista, grade, craft/learn |
| `UI/UmbraCraftingRecipeClickProxy.*` | Clique nas linhas de receita |
| `UmbraInventorySlotWidget::ConfigureAsCraftingDropTarget` | Drop espelha ícone (não move a bag) |
| `UmbraGameInstance::LoadCraftRecipes` / `RequestCraft` / `RequestLearnCraftRecipe` | HTTP |
| `FUmbraCraftRecipe`, `FUmbraCraftIngredient` | Structs |

A grade **não** usa 5 BindWidgets individuais. O Blueprint só vincula `Grid_CraftSlots` (`UniformGridPanel`). No `NativeConstruct`, o C++ spawna 5× `WBP_InventorySlot` (1 linha × 5 colunas), igual vendor/loot.

---

## Passo a passo — Blueprint `WBP_CraftingMenu` (UE 5.6.1)

### 0) Pré-requisitos

1. Compilar o módulo `UmbraEternumUE` (Live Coding ou Build) até aparecer `UmbraCraftingWidget` no Editor.
2. Confirmar que existe `Content/Widgets/UI/Inventory/WBP_InventorySlot` (ou o path do slot do inventário do projeto).
3. APIs no WAMP (`C:\wamp64\www\umbra_api\api\crafting\`) e SQL de crafting aplicados.

### 1) Criar o Widget Blueprint

1. Content Browser → pasta de UI (ex.: `Content/Widgets/UI/Crafting/`).
2. **Add** → **User Interface** → **Widget Blueprint**.
3. Nome: **`WBP_CraftingMenu`**.
4. Abrir → **Class Settings** / **Parent Class** → **`UmbraCraftingWidget`** (não `UserWidget` genérico).
5. **Compile** + **Save**.

### 2) Class Defaults (Details do WBP, sem widget selecionado)

| Propriedade | Valor |
|-------------|--------|
| **Craft Slot Widget Class** | `WBP_InventorySlot` (`UUmbraInventorySlotWidget`) |
| **Item Tooltip Widget Class** | `WBP_ItemTooltipRefined` (opcional; C++ tem fallback) |

**Obrigatório na prática:** sem `Craft Slot Widget Class` a grade fica vazia (igual loot/mail). Abra o WBP → **Class Defaults** → **Crafting** → **Craft Slot Widget Class** = `WBP_InventorySlot`.

Se ainda estiver None, o C++ tenta `LoadClass` em `/Game/Widgets/UI/Inventory/WBP_InventorySlot`. Confira no Output Log: `[Crafting] Grade criada com 5 slots`.

### 3) Designer — hierarquia sugerida

Aba **Designer**. Palette + Content Browser. Nomes abaixo são **exatos** (`BindWidgetOptional`).

```
Root (Canvas Panel ou Overlay)
└── Border / Size Box (painel central, ex. 720x480)
    └── Vertical Box
        ├── Horizontal Box (topo)
        │   ├── Combo_Category          ← Combo Box (String)
        │   ├── BTN_Refresh             ← Button
        │   └── BTN_Close               ← Button
        ├── Horizontal Box (corpo)
        │   ├── Scroll_Recipes          ← Scroll Box (largura ~260)
        │   │   └── Box_RecipeList      ← Vertical Box  (C++: HBox ícone 48px + botão)
        │   └── Vertical Box (direita)
        │       ├── Text_RecipeTitle    ← Text
        │       ├── Text_Result         ← Text (resultado x qty)
        │       ├── Text_GoldCost       ← Text
        │       ├── Slider_CraftCount   ← Slider (1–99)
        │       ├── Text_CraftCount     ← Text (opcional)
        │       ├── Box_IngredientHints ← Vertical Box  ★ requisitos em texto (C++ preenche)
        │       ├── Grid_CraftSlots     ← Uniform Grid Panel  ★ VAZIO; preview + drop
        │       ├── Horizontal Box
        │       │   ├── BTN_Craft       ← Button ("Craftar")
        │       │   └── BTN_Learn       ← Button ("Aprender")
        │       └── Text_Status         ← Text
```

#### Comportamento C++ (ícones / tooltips)

- **Lista esquerda:** cada receita = ícone do **resultado** + botão; hover no ícone usa `SetToolTip` C++ (ícone runtime sem hover BP).
- **Box_IngredientHints:** linhas `S0: Folha Umbra x2` (qty × slider).
- **Grid_CraftSlots:** sem drop mostra **preview** do material exigido (ícone) com **tint âmbar** (`SetCraftingInsertHint`); após drop da bag remove o tint e mostra a pilha. Tooltip dos craft slots = **somente** o hover do `WBP_InventorySlot` (não usar `SetToolTip` C++ aí — evita double tooltip).
- **Catálogo / description / stats:** `list_recipes` envia `result_item` + campos de catálogo nos ingredientes; ao carregar, o GameInstance chama `ParseItemTemplate` e hidrata o cache para o tooltip completo (mesmo padrão das recompensas de quest).

#### Uniform Grid Panel (`Grid_CraftSlots`)

1. Palette → **Uniform Grid Panel** → solte no painel direito.
2. F2 / Details → **Name** = **`Grid_CraftSlots`** (obrigatório).
3. **Não** coloque `WBP_InventorySlot` dentro no Designer. O C++ limpa e preenche no `NativeConstruct`.
4. Details úteis: Slot Size / Alignment Fill; altura mínima ~72–80 para caber os slots.

### 4) Tabela de BindWidgetOptional

| Nome no Designer | Tipo UMG | Obrigatório? |
|------------------|----------|------------|--------------|
| `Combo_Category` | ComboBoxString | Recomendado |
| `Scroll_Recipes` | ScrollBox | Opcional (só layout) |
| `Box_RecipeList` | VerticalBox | **Sim** (lista: ícone + botão) |
| `Box_IngredientHints` | VerticalBox | **Sim** (textos dos requisitos) |
| `Text_RecipeTitle` | TextBlock | Recomendado |
| `Text_Result` | TextBlock | Recomendado |
| `Text_GoldCost` | TextBlock | Recomendado |
| `Text_Status` | TextBlock | Recomendado |
| `Slider_CraftCount` | Slider | Recomendado (batch 1–99) |
| `Text_CraftCount` | TextBlock | Opcional (mostra o valor do slider) |
| `BTN_Craft` | Button | **Sim** |
| `BTN_Learn` | Button | Recomendado |
| `BTN_Close` | Button | Recomendado |
| `BTN_Refresh` | Button | Opcional |
| **`Grid_CraftSlots`** | **UniformGridPanel** | **Sim** (5 slots runtime) |

Não use mais `InventorySlot_Craft0`…`Craft4`.

### 5) Graph (Event Graph)

Pode ficar **vazio**. O C++ em `NativeConstruct`:

- preenche `Combo_Category` (`all` / `material` / `consumable` / `equipment`)
- liga cliques dos botões / slider
- spawna os 5 slots em `Grid_CraftSlots` e chama `ConfigureAsCraftingDropTarget`
- monta lista com ícones + tooltips; hints + preview ao selecionar receita
- chama `LoadCraftRecipes` e escuta `OnCraftRecipesLoaded` / `OnCraftResult` / `OnCraftLearnResult`

### 6) Abrir a partir do HUD

No `WBP_PlayerHUD` (ou atalho), no clique do botão Craft:

1. **Create Widget** → Class = `WBP_CraftingMenu` → Owning Player = Get Player Controller.
2. **Add to Viewport** (ZOrder acima do inventário se precisar).
3. Opcional: **Set Input Mode UI Only** / **Show Mouse Cursor** = true (mesmo padrão mail/refine).
4. Ao fechar: `BTN_Close` chama `RemoveFromParent` no C++.

Não precisa chamar `RefreshRecipes` no Graph — o construct já carrega.

### 7) Fluxo de uso (Play)

1. Abrir inventário + crafting.
2. Escolher categoria (Combo) ou deixar `all`.
3. Ver ícones na lista; hover → tooltip do resultado.
4. Clicar uma receita: hints à direita + preview nos slots; hover no slot → tooltip do material.
5. Arrastar da **bag** para cada célula exigida (substitui o preview).
6. Ajustar quantidade no `Slider_CraftCount` se quiser batch (hints/qty atualizam).
7. **Craftar** — ou **Aprender** se a receita mostrar `[Aprender]`.
8. Sucesso → inventário/ouro recarregam; falha → `Text_Status`.

### 8) Checklist de erros comuns

| Sintoma | Causa provável | Correção |
|---------|----------------|----------|
| Grade vazia / sem slots | Nome ≠ `Grid_CraftSlots` **ou** `Craft Slot Widget Class` = None | Renomear grid; Class Defaults → `WBP_InventorySlot`; log `[Crafting] Grade criada` |
| Slots sem ícone/visual | Classe C++ pura / path WBP errado | Class Defaults → `WBP_InventorySlot` |
| Sem textos de requisito | Falta `Box_IngredientHints` | Vertical Box com esse nome exato |
| Sem ícone na lista | Template sem ícone / DT de ícones | Conferir `GetItemIconByID` / DataTable |
| Tooltip não aparece | Tooltip class inválida / PC nulo | Class Defaults → `WBP_ItemTooltipRefined`; abrir via PlayerController |
| Dois tooltips no craft slot | `SetToolTip` C++ + hover do WBP | Craft slots só usam hover BP; C++ chama `SetToolTip(nullptr)` |
| Description/stats vazios | API antiga sem `result_item` / cache stub | Atualizar `craft_helper` + `list_recipes` no WAMP; recompilar UE (`ParseItemTemplate` no load) |
| Drop não cola | Item errado / receita não selecionada | Template do slot deve bater com o hint |
| Lista de receitas vazia | API/WAMP/JWT / SQL não rodado | Conferir `list_recipes.php` e seed |
| Learn some | Sem scroll / já conhecida / categoria equipment | Só material/consumable aprendíveis |
| Craft “Inventário cheio” | Bag 0–49 sem espaço | Liberar slot ou stack do resultado |

---

## Fluxo de teste (dados)

1. Rodar SQL seed.
2. Dar itens seed + ouro (Manager Give Item).
3. Compilar UE → criar/atualizar `WBP_CraftingMenu` → abrir no Play.
4. Craft padrão (ex.: Pó de Folha).
5. Scroll “Receita: Essência…” → Aprender → craftar Essência.
6. Equipment com scroll: cada craft consome 1 “Receita: Lâmina Temperada”.
7. Encher bag → craft deve falhar com “Inventário cheio”.
