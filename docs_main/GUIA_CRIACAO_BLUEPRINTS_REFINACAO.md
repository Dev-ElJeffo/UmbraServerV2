# Guia - Implementação Completa de Refinação (Blueprint + C++)

## Objetivo

Implementar a janela de refinação no UE 5.6.1 com regra de negócio em C++, usando `UUmbraRefinementWidget` e `UUmbraInventorySlotWidget`.

## Passo 1 - Criar `WBP_RefinementWindow`

1. No Content Browser, crie `Widget Blueprint` com nome `WBP_RefinementWindow`.
2. Em `Class Settings`, defina `Parent Class` = `UmbraRefinementWidget`.
3. No Designer, crie o layout visual da janela.

## Passo 2 - Criar os widgets com nomes exatos (BindWidget)

No `WBP_RefinementWindow`, marque `Is Variable = true` e use os nomes abaixo exatamente:

- `InventorySlot_TargetItem` (tipo: `WBP_InventorySlot` herdando de `UUmbraInventorySlotWidget`)
- `InventorySlot_RefineMaterial` (tipo: `WBP_InventorySlot` herdando de `UUmbraInventorySlotWidget`)
- `ItemIcon`
- `ItemNameText`
- `CurrentLevelText`
- `MaterialIcon`
- `MaterialQuantityText`
- `SuccessRateText`
- `RequiredMaterialText`
- `CurrentStatsText`
- `NextLevelStatsText`
- `RefineButton`
- `CancelButton`
- `Text_Info` (nome novo)

Observação:
- `InfoMessageText` ainda é suportado no C++ como fallback.
- Para o fluxo novo, prefira `Text_Info`.

## Passo 3 - Garantir que os dois slots são `InventorySlot`

Os dois slots de drop (`InventorySlot_TargetItem` e `InventorySlot_RefineMaterial`) devem ser widgets baseados em `UUmbraInventorySlotWidget` (não usar `Border` puro para drop).

## Passo 4 - Deixar o Event Graph do `WBP_RefinementWindow` sem lógica

Não implemente regra de negócio no graph desse WBP:

- não criar `OnDrop` manual
- não criar validação de item/material em Blueprint
- não criar binding manual de texto de status

Toda a lógica já está em C++ em `UUmbraRefinementWidget` + `UUmbraInventorySlotWidget`.

## Passo 5 - Criar `WBP_RefinementTooltip`

1. Crie `Widget Blueprint` com nome `WBP_RefinementTooltip`.
2. Parent class: `UmbraRefinementTooltipWidget`.
3. Crie os widgets com `Is Variable = true`:
   - `ItemNameText`
   - `RefinementLevelText`
   - `SuccessRateText`
   - `StatsContainer`
   - `BonusStatsContainer`

## Passo 6 - Ligar a classe do tooltip no widget de refinação

No `WBP_RefinementWindow` (instância da classe `UUmbraRefinementWidget`), configure a propriedade:

- `RefinementTooltipClass` = `WBP_RefinementTooltip`

Sem isso, o tooltip não será criado por `EnsureRefinementTooltip()`.

## Passo 7 - Integração com o inventário (abertura da janela)

No widget que abre a refinação (inventário/menu):

1. `Create Widget` (class `WBP_RefinementWindow`)
2. `Add to Viewport`

Somente isso já basta para inicialização da lógica C++ (`NativeConstruct`).

## Passo 8 - Fluxo de drag & drop esperado (já implementado em C++)

1. Jogador arrasta item no inventário (`UUmbraInventorySlotWidget::CreateItemDragOperation`).
2. Solta no `InventorySlot_TargetItem`.
3. O slot chama `HandleRefinementSlotDrop(false, DragOperation)`.
4. `SetItemToRefine(...)` valida se item é refinável.
5. Tooltip é criado/atualizado.
6. `Text_Info` mostra próximo passo.
7. Jogador arrasta material e solta em `InventorySlot_RefineMaterial`.
8. O slot chama `HandleRefinementSlotDrop(true, DragOperation)`.
9. `SetRefinementMaterial(...)` valida material/quantidade para o nível.
10. `RefineButton` habilita só com par válido.

## Passo 9 - Fluxo de status (`Text_Info`)

O estado textual da janela é controlado por `UpdateInfoText()` em C++:

- sem item alvo: "Arraste o item do inventário para o slot de refinação."
- com item e sem material: "Agora arraste o material de refinação."
- com par válido: "Itens válidos. Clique em REFINAR para continuar."

## Passo 10 - Executar refinação

1. Com os dois slots válidos, clique `RefineButton`.
2. `RefineItem()` chama `UUmbraRefinementSubsystem::RequestRefineItem(...)`.
3. `OnRefinementComplete(...)` atualiza mensagem e estado visual.

## Passo 11 - Checklist final de implementação

- [ ] `WBP_RefinementWindow` herda de `UmbraRefinementWidget`
- [ ] `InventorySlot_TargetItem` e `InventorySlot_RefineMaterial` são `WBP_InventorySlot` (InventorySlotWidget)
- [ ] Todos os nomes de variáveis de UI batem com `BindWidget`
- [ ] `Text_Info` criado e com `Is Variable = true`
- [ ] `RefinementTooltipClass` apontando para `WBP_RefinementTooltip`
- [ ] Event Graph do `WBP_RefinementWindow` sem regra de negócio
- [ ] Janela abre via `Create Widget -> Add to Viewport`
- [ ] Drop do item alvo funciona
- [ ] Drop do material funciona
- [ ] Botão `RefineButton` só habilita com par válido
- [ ] Tooltip atualiza ao inserir item alvo

## Passo 12 - Teste rápido no jogo

1. Login.
2. Abrir inventário.
3. Abrir janela de refinação.
4. Arrastar item refinável para `InventorySlot_TargetItem`.
5. Arrastar material correto para `InventorySlot_RefineMaterial`.
6. Confirmar mensagem em `Text_Info`.
7. Clicar em `REFINAR`.

## Referências

- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraRefinementWidget.*`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraInventorySlotWidget.*`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraRefinementTooltipWidget.*`
- `UmbraEternumUE/Source/UmbraEternumUE/Systems/UmbraRefinementSubsystem.*`
