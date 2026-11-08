# Guia - Implementação Completa de Refinação (Blueprint + C++)

## Objetivo

Implementar a janela de refinação no UE 5.6.1 com regra de negócio em C++, usando `UUmbraRefinementWidget` e `UUmbraInventorySlotWidget`.

## Assets versionados (git)

Os Blueprints ficam no submódulo `UmbraEternumUE`:

- `Content/Widgets/UI/Refinement/WBP_RefinementWindow.uasset`
- `Content/Widgets/UI/Refinement/WBP_RefinementTooltip.uasset`

Commitar alterações no branch `develop` do submódulo após editar no editor.

---

## Passo 1 - Abrir `WBP_RefinementWindow`

1. No Content Browser: `Content/Widgets/UI/Refinement/WBP_RefinementWindow`.
2. Em **Class Settings**, `Parent Class` = `UmbraRefinementWidget`.
3. No Designer, ajuste o layout conforme abaixo.

## Passo 2 - Layout: migração gradual (não quebrar o WBP atual)

O C++ usa **dual-path**: se `InventorySlot_*` existir no WBP, o slot recebe `SetSlotData`; caso contrário, `ItemIcon` / `MaterialIcon` continuam funcionando.

### Containers de layout (nunca remover no C++)

- `ItemSlotBorder` e `MaterialSlotBorder` são **containers** que agrupam ícone, textos e stats.
- **Não** ocultar esses borders no C++ — esconder um `Border` colapsa todo o subtree (área de inserção some).

### Área do item a refinar

**WBP legado (funciona hoje):**

- `ItemSlotBorder` (visível) contendo `ItemIcon`, `ItemNameText`, `CurrentLevelText`, etc.

**WBP migrado (opcional):**

- Dentro de `ItemSlotBorder`, adicionar `InventorySlot_TargetItem` (`WBP_InventorySlot`).
- Só **depois de testar**, remover `ItemIcon` duplicado no Designer (o C++ oculta `ItemIcon` automaticamente quando o slot está visível).

### Área do material

Mesma regra: `MaterialSlotBorder` permanece; adicione `InventorySlot_RefineMaterial` quando quiser unificar; remova `MaterialIcon` só após validar.

## Passo 3 - Nomes BindWidget

Marque `Is Variable = true`:

| Nome | Tipo | Obrigatório |
|------|------|-------------|
| `ItemNameText` | Text | Sim |
| `CurrentLevelText` | Text | Sim |
| `CurrentStatsText` | Text | Sim |
| `NextLevelStatsText` | Text | Sim |
| `SuccessRateText` | Text | Sim |
| `RequiredMaterialText` | Text | Sim |
| `RefineButton` | Button | Sim |
| `CancelButton` | Button | Sim |
| `InventorySlot_TargetItem` | `WBP_InventorySlot` | Opcional (recomendado) |
| `InventorySlot_RefineMaterial` | `WBP_InventorySlot` | Opcional (recomendado) |
| `Text_Info` | Text | Opcional |
| `ItemIcon` | Image | Opcional (fallback legado) |
| `MaterialIcon` | Image | Opcional (fallback legado) |
| `ItemSlotBorder` | Border | Opcional (container layout) |
| `MaterialSlotBorder` | Border | Opcional (container layout) |

**Labels no Designer:** rótulo de `CurrentStatsText` = "Stats atuais"; `NextLevelStatsText` = "Próximo nível".

## Passo 4 - Event Graph vazio

- sem `OnDrop` manual
- sem limpar slots no clique de REFINAR
- sem binding manual de stats

## Passo 5 - `WBP_RefinementTooltip`

1. Asset: `Content/Widgets/UI/Refinement/WBP_RefinementTooltip`.
2. Parent: `UmbraRefinementTooltipWidget`.
3. Variáveis: `ItemNameText`, `RefinementLevelText`, `SuccessRateText`, `StatsContainer`, `BonusStatsContainer`.

## Passo 6 - Tooltip na janela

`RefinementTooltipClass` = `WBP_RefinementTooltip`.

## Passo 7 - Abrir a janela

`Create Widget (WBP_RefinementWindow)` → `Add to Viewport`.

---

## Fluxo em C++

### Drag & drop

1. Drop no slot alvo ou área legada → `SetItemToRefine` → `SyncRefinementSlotVisuals`.
2. Drop no material → `SetRefinementMaterial`.
3. Se `InventorySlot_*` visível: ícone no slot; senão: `ItemIcon` / `MaterialIcon`.

### Refinar e pós-refino

1. REFINAR → API PHP.
2. Item permanece no slot; material permanece se houver quantidade.
3. `LoadInventory()` → `SyncSelectionFromInventory` → UI atualizada.

### Diagnóstico

No Output Log, ao abrir a janela:

`[RefinementUI] TargetSlot=OK|NULL MaterialSlot=... ItemIcon=... ItemBorder=...`

---

## Checklist final

- [ ] `ItemSlotBorder` / `MaterialSlotBorder` visíveis no Designer
- [ ] `InventorySlot_*` adicionados **antes** de remover ícones legados
- [ ] Event Graph sem lógica de negócio
- [ ] Material persiste entre tentativas
- [ ] Inventário atualiza após REFINAR

## Referências

- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraRefinementWidget.*`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraInventorySlotWidget.*`
- `docs_main/GUIA_SISTEMA_REFINACAO_UE561.md`
