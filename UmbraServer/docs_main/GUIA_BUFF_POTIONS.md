# Guia: Poções de Buff (`buff_potion`)

Sistema de consumíveis que aplicam bônus temporário de status, exibidos na **BuffBar** do `WBP_SkillBar1`.

Complementa [GUIA_SKILLBAR_HOTKEYS_QUANTIDADE_COOLDOWN.md](GUIA_SKILLBAR_HOTKEYS_QUANTIDADE_COOLDOWN.md) e [GUIA_USO_CONSUMIVEIS_SKILLBAR.md](GUIA_USO_CONSUMIVEIS_SKILLBAR.md).

---

## Visão geral

| Camada | Responsabilidade |
|--------|------------------|
| MySQL `player_item_buffs` | Persiste buff ativo por `(player_id, buff_key)` — refresh ao reusar |
| `use_item.php` | Branch `buff_potion`: grava buff + decrementa pilha + cooldown de uso |
| `get_active_buffs.php` | Lista buffs ativos com nome/ícone/descrição |
| `character_info_helper.php` | Soma bônus flat em `stats.total` |
| `UUmbraGameInstance` | `LoadActiveBuffs()` → `ActiveBuffs` → `OnActiveBuffsLoaded` |
| `UUmbraSkillBarWidget` | `PopulateBuffBar()` na `BuffBar_HBox` |
| `UUmbraBuffIconWidget` | Ícone + timer + hover (cria/posiciona tooltip em C++) |
| `UUmbraBuffTooltipWidget` | Layout do tooltip + `ApplyFromBuffData` + tick do tempo |

**Uso:** RMB no inventário, RMB no slot da skillbar, ou hotkey do slot (já existente via `UseConsumableByTemplate`).

---

## 1. Banco de dados

Execute na ordem:

```bash
mysql umbra_eternum < UmbraServer/scripts_main/add_buff_potion_system.sql
mysql umbra_eternum < UmbraServer/scripts_main/insert_buff_potions.sql
```

### Tabela `player_item_buffs`

- **PK:** `(player_id, buff_key)` — uma linha por tipo de buff (ex.: `strength_buff`)
- **Refresh:** `ON DUPLICATE KEY UPDATE` ao usar a mesma poção de novo

### `stats_json` do item

```json
{"duration": 300, "strength_buff": 20}
```

| Campo | Significado |
|-------|-------------|
| `duration` | Duração do efeito em **segundos** (300 = 5 min) |
| `<stat>_buff` | Bônus **flat** somado ao total (ex.: +20 força) |

Chaves suportadas: `strength_buff`, `dexterity_buff`, `intelligence_buff`, `vitality_buff`, `luck_buff`, `attack_buff`, `defense_buff`.

### Cooldown de uso vs duração do buff

| Conceito | Tabela / campo |
|----------|----------------|
| Cooldown entre usos (anti-spam) | `player_consumable_cooldowns` + `use_cooldown_ms` no template (seed: 1000 ms) |
| Duração do efeito | `player_item_buffs.expires_at_ms` |

---

## 2. API PHP

### POST `/api/inventory/use_item.php`

Mesmo contrato dos outros consumíveis. Para `buff_potion`, resposta inclui:

```json
{
  "success": true,
  "buff": {
    "key": "strength_buff",
    "value": 20,
    "expires_at_ms": 1747858800000,
    "duration_ms": 300000,
    "item_template_id": 10
  },
  "cooldown_expires_at_ms": ...,
  "cooldown_total_ms": 1000
}
```

### POST `/api/inventory/get_active_buffs.php`

```json
{ "token": "...", "player_id": 1 }
```

Resposta:

```json
{
  "success": true,
  "server_time_ms": 1747858500000,
  "buffs": [
    {
      "buff_key": "strength_buff",
      "bonus_value": 20,
      "duration_ms": 300000,
      "started_at_ms": 1747858500000,
      "expires_at_ms": 1747858800000,
      "item_template_id": 10,
      "item_name": "Elixir de Força",
      "item_description": "...",
      "icon_path": "/Game/UI/Icons/Items/ICO_StrengthElixir"
    }
  ]
}
```

---

## 3. Cliente UE — C++

### Struct `FUmbraActiveBuff`

Campos extras para poções: `BuffKey`, `ItemTemplateID`, `ItemDescription`, `ExpiresAtMs`, `StartedAtMs`.

### GameInstance

- `LoadActiveBuffs()` → chama `get_active_buffs.php`
- `OnActiveBuffsLoaded` → SkillBar atualiza ícones
- Após `use_item` com buff: `LoadActiveBuffs()` + `LoadCharacterInfo()`

### `UUmbraBuffIconWidget`

Bindings no `WBP_BuffIcon`:

| Nome | Tipo |
|------|------|
| `Icon` | Image |
| `Duration_Text` | TextBlock (mm:ss) |
| `Duration_Fill` | Image (`MI_CooldownRadial`, parâmetro `Progress`) |

Em C++: `NativeOnMouseEnter/Move/Leave` cria, reposiciona e remove `WBP_BuffTooltip` conforme o mouse. `FormatBuffBonusText`: ex. `+20 Strength` a partir de `strength_buff`.

### `UUmbraBuffTooltipWidget`

Bindings no `WBP_BuffTooltip` (todos `BindWidgetOptional`):

| Nome | Tipo |
|------|------|
| `Item_Name` | TextBlock |
| `Description_Text` | TextBlock |
| `Bonus_Text` | TextBlock |
| `Time_Text` | TextBlock (`m:ss restante`, atualizado a cada 0,25 s) |
| `Icon` | Image (opcional) |

`ApplyFromBuffData(FUmbraActiveBuff)` preenche os campos; não é necessário Event Graph.

---

## 4. Blueprint — passo a passo (nó a nó)

### 4.1 `WBP_BuffIcon`

1. **Content Browser** → `Content/Widgets/UI/Buff/` → botão direito → **User Interface** → **Widget Blueprint** → **Common** → **User Widget**. Nome: `WBP_BuffIcon`.
2. Abrir → **File** → **Reparent Blueprint** → `UmbraBuffIconWidget`.
3. Apagar tudo da **Hierarchy** (deixar só o Canvas raiz vazio).
4. Adicionar widgets nesta ordem exata:

```
[Canvas Panel]                    ← raiz
└── [SizeBox]                     Width Override 48, Height Override 48
    └── [Canvas Panel]
        ├── Border_Background     Border — Anchors Fill, ZOrder 0
        │                         Brush Tint: R=0.08 G=0.08 B=0.1 A=0.9
        ├── Icon                  Image — Anchors Fill, Offsets 2 em todos os lados, ZOrder 1
        │                         Visibility: Not Hit-Testable (Self & All Children)
        ├── Duration_Fill         Image — Anchors Fill, Brush Material MI_CooldownRadial, ZOrder 2
        │                         Visibility: Not Hit-Testable
        └── Duration_Text         TextBlock — Anchors Bottom-Right, Position X=-2 Y=-2
                                  Alignment 1,1 — Font Size 10 — Outline Size 1 preto — ZOrder 3
                                  Visibility: Not Hit-Testable
```

5. **Compile**. Bindings verdes obrigatórios: `Icon`, `Duration_Text`, `Duration_Fill`.
6. **Event Graph: vazio** (sem variáveis, sem nós). Hover, tooltip e tick estão no C++.
7. **Class Defaults** → categoria **Buff | Tooltip** (opcional se já configurar no SkillBar):
   - **Tooltip Widget Class** = `WBP_BuffTooltip` (§4.2)
   - **Tooltip Cursor Offset** = `X=16, Y=16` (padrão)

### 4.2 `WBP_BuffTooltip`

1. **Content Browser** → `Content/Widgets/UI/Buff/` → criar **Widget Blueprint** `WBP_BuffTooltip`.
2. **File** → **Reparent Blueprint** → `UmbraBuffTooltipWidget`.
3. Hierarquia:

```
[Canvas Panel]                    ← raiz
└── [SizeBox]                     Width Override 280 (altura automática)
    └── [Border]                  Brush Tint R=0.05 G=0.05 B=0.08 A=0.95 — Padding 10 em todos
        └── [Vertical Box]
            ├── [Horizontal Box]    ← cabeçalho
            │   ├── Icon          Image dentro de SizeBox 32×32 — Slot Padding 0,0,6,0 (opcional)
            │   └── Item_Name     TextBlock — Font Size 14 Bold — Color 1, 0.95, 0.4, 1
            ├── Bonus_Text        TextBlock — Font 12 — Color 0.5, 1, 0.5, 1 — Padding Top 4
            ├── Time_Text         TextBlock — Font 11 — Color 0.8, 0.8, 0.8, 1 — Padding Top 2
            └── Description_Text  TextBlock — Font 11 — Auto Wrap Text **true**
                                  Color 0.85, 0.85, 0.85, 1 — Padding Top 6
```

4. **Compile**. Bindings esperados (verdes): `Item_Name`, `Description_Text`, `Bonus_Text`, `Time_Text`, `Icon` — todos opcionais no C++; podem faltar sem quebrar.
5. **Event Graph: vazio.** `ApplyFromBuffData` e atualização de `Time_Text` estão no C++.

### 4.3 Class Defaults (sem Event Graph)

> **Sem Event Graph.** `UUmbraBuffIconWidget` cria, posiciona e destrói `WBP_BuffTooltip` em `NativeOnMouseEnter`, `NativeOnMouseMove` e `NativeOnMouseLeave`. Configure apenas as classes abaixo.

| Propriedade (`WBP_BuffIcon` — opcional) | Valor |
|----------------------------------------|-------|
| Tooltip Widget Class | `WBP_BuffTooltip` |
| Tooltip Cursor Offset | `X=16, Y=16` |

### 4.4 `WBP_SkillBar1`

1. Abrir `WBP_SkillBar1` → **Class Defaults**.
2. Categoria **Skill Bar | Buffs**:

| Propriedade | Valor |
|-------------|-------|
| Buff Icon Widget Class | `WBP_BuffIcon` |
| Buff Tooltip Widget Class | `WBP_BuffTooltip` |

3. Confirme `BuffBar_HBox` na hierarquia.
4. **Compile** e teste.

**Redundância:** `PopulateBuffBar()` copia `BuffTooltipWidgetClass` do SkillBar para cada ícone criado. Você **não precisa** repetir **Tooltip Widget Class** no `WBP_BuffIcon` se já definiu no `WBP_SkillBar1` — recomendado centralizar **só no SkillBar**.

### 4.5 Checklist final (antes do Play)

- [ ] SQL `add_buff_potion_system.sql` + `insert_buff_potions.sql` executados
- [ ] `WBP_BuffIcon` reparentado para `UmbraBuffIconWidget`, bindings `Icon` / `Duration_Text` / `Duration_Fill` verdes
- [ ] `WBP_BuffTooltip` reparentado para `UmbraBuffTooltipWidget`, nomes dos TextBlocks/Image iguais à tabela
- [ ] Event Graph **vazio** nos dois widgets
- [ ] `WBP_SkillBar1`: **Buff Icon Widget Class** e **Buff Tooltip Widget Class** preenchidos
- [ ] `BuffBar_HBox` presente no `WBP_SkillBar1`
- [ ] Projeto C++ recompilado após adicionar `UmbraBuffTooltipWidget`
- [ ] Hover no ícone da buff bar: tooltip segue o mouse; `Time_Text` conta regressivo; sair do ícone fecha o tooltip

---

## 5. Teste de aceite

1. Rodar SQL de schema + seed.
2. Dar ao personagem um Elixir de Força (id 10).
3. **RMB** no inventário → HP não muda; `get_character_info` mostra `strength` +20 no total.
4. **BuffBar** exibe ícone com timer ~5:00 regressivo.
5. Hover no ícone → tooltip com nome, bônus e tempo.
6. Arrastar poção para skillbar → **RMB** ou hotkey → mesmo efeito.
7. Usar de novo antes de expirar → duração **reinicia** (refresh), valor não acumula.
8. Após 5 min → ícone some; stats voltam ao normal.

---

## 6. Arquivos novos/alterados

**Novos**

- `UmbraServer/scripts_main/add_buff_potion_system.sql`
- `UmbraServer/scripts_main/insert_buff_potions.sql`
- `www/umbra_api/api/inventory/get_active_buffs.php`
- `UmbraEternumUE/.../UI/UmbraBuffIconWidget.h/.cpp`
- `UmbraEternumUE/.../UI/UmbraBuffTooltipWidget.h/.cpp`
- `UmbraServer/docs_main/GUIA_BUFF_POTIONS.md`

**Alterados**

- `www/umbra_api/api/inventory/use_item.php`
- `www/umbra_api/helpers/character_info_helper.php`
- `UmbraEternumUE/.../Data/UmbraSkillDataStructures.h`
- `UmbraEternumUE/.../Core/UmbraGameInstance.h/.cpp`
- `UmbraEternumUE/.../UI/UmbraSkillBarWidget.h/.cpp`
- `UmbraEternumUE/.../UI/UmbraBuffIconWidget.h/.cpp` (tooltip em C++, sem eventos BP)
