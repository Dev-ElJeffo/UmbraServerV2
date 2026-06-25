# Guia Unico de Criacao e Adocao do Novo Tooltip de Inventario

## 1) Objetivo e regra de ouro

Este guia cria e adota um **novo tooltip** de inventario baseado em `UUmbraItemTooltipWidget`, sem alterar o widget legado que ja funciona.

- Legado (permanece intacto): `WBP_ItemTooltip`
- Novo (a ser criado e usado): `WBP_ItemTooltipRefined`

Resultado esperado:
- Itens normais continuam exibindo stats base.
- Itens refinados exibem nome com `+X`, indicador de nivel e stats totais.
- Troca de uso feita no ponto de criacao do tooltip no fluxo de hover do slot.

---

## 2) Base tecnica usada por este guia

Classe C++ nova (ja criada):
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraItemTooltipWidget.h`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraItemTooltipWidget.cpp`

Ponto de chamada do tooltip (fluxo de slot):
- Blueprint `WBP_InventorySlot` (evento de hover, normalmente `On Mouse Enter`)
- Classe base do slot: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraInventorySlotWidget.h`

Comportamento da classe nova:
- `SetTooltipData(const FUmbraInventorySlot& ItemSlot)` decide automaticamente:
  - `RefinementLevel == 0` -> tooltip normal
  - `RefinementLevel > 0` -> tooltip refinado

---

## 3) Pre-requisitos

1. Compilacao C++ concluida com sucesso no Visual Studio:
   - abrir `UmbraEternumUE.sln`
   - `Build > Build Solution`
2. Reabrir Unreal Engine para garantir hot reload das classes C++.
3. Verificar no editor se a parent class `UmbraItemTooltipWidget` aparece para selecao.

---

## 4) Criar o novo widget sem tocar no legado

## Passo 4.1 - Duplicar o asset visual atual

No Content Browser:
1. Localize `WBP_ItemTooltip`.
2. Duplique.
3. Renomeie para `WBP_ItemTooltipRefined`.

Observacao: isso preserva visual/layout do tooltip atual e evita regressao no legado.

## Passo 4.2 - Trocar Parent Class somente no novo widget

No `WBP_ItemTooltipRefined`:
1. Abra o Blueprint.
2. `Class Settings`.
3. Em `Parent Class`, troque para `UmbraItemTooltipWidget`.
4. Compile.

## Passo 4.3 - Remover conflito de funcao Blueprint

Se existir funcao Blueprint chamada `SetTooltipData` dentro do novo widget, remova-a.

Motivo:
- A funcao C++ `SetTooltipData` ja existe na parent class.
- Manter uma funcao Blueprint de mesmo nome gera conflito de assinatura/pins.

## Passo 4.4 - Garantir BindWidgets obrigatorios

No Designer do `WBP_ItemTooltipRefined`, confirme os 3 obrigatorios:
- `ItemNameText` (TextBlock, Is Variable = true)
- `ItemTypeText` (TextBlock, Is Variable = true)
- `RarityText` (TextBlock, Is Variable = true)

Importante:
- Nome precisa ser exato (case-sensitive).
- Se faltar qualquer um, a compilacao do Blueprint falha.

## Passo 4.5 - Configurar opcionais recomendados

Recomendado:
- `RefinementLevelText` (TextBlock)
  - Is Variable = true
  - Visibility = Collapsed
  - cor amarela

Demais opcionais (aparecem se existirem):
- `StrengthText`, `DexterityText`, `IntelligenceText`, `VitalityText`, `LuckText`
- `PhysicalAttackText`, `MagicAttackText`, `PhysicalDefenseText`, `MagicDefenseText`
- `AccuracyText`, `DodgeText`, `CriticalText`, `CriticalResistanceText`
- `HealthBonusText`, `ManaBonusText`, `MovementText`
- `ItemIcon`

---

## 5) Onde chamar o novo widget (troca de uso)

Troca deve ser feita no fluxo de hover do slot de inventario (`WBP_InventorySlot`), no trecho que hoje cria tooltip.

## Passo 5.1 - Encontrar o ponto

No `WBP_InventorySlot`:
1. Abra o Event Graph.
2. Encontre o fluxo de `On Mouse Enter` (ou equivalente usado no projeto).
3. Localize o node `Create Widget` que aponta para `WBP_ItemTooltip`.

## Passo 5.2 - Substituir apenas a classe do Create Widget

Troque:
- de: `WBP_ItemTooltip`
- para: `WBP_ItemTooltipRefined`

Nao altere o widget legado.

## Passo 5.3 - Chamar SetTooltipData com struct completo

No fluxo logo apos criar o widget:
1. Chame `SetTooltipData`.
2. Passe **um unico parametro** `ItemSlot` do tipo `FUmbraInventorySlot`.
3. Conecte o struct completo vindo do slot (`SlotData`/`GetSlotData`).

Nao usar varios parametros separados (nome, tipo, stats etc.).

Fluxo recomendado:
1. `On Mouse Enter`
2. `Create Widget (WBP_ItemTooltipRefined)`
3. `SetTooltipData(ItemSlot = SlotData completo)`
4. `Add to Viewport` (ou attach no container de tooltip usado no projeto)

---

## 6) Checklist de validacao

## 6.1 Compilacao do novo tooltip
- `WBP_ItemTooltipRefined` compila sem erro.
- Nao existe conflito de funcao `SetTooltipData`.
- Nao existe erro de BindWidget obrigatorio ausente.

## 6.2 Teste funcional item normal
- Nome sem `+X`.
- `RefinementLevelText` oculto.
- Stats base exibidos.

## 6.3 Teste funcional item refinado (+12)
- Nome com `+12`.
- Indicador de nivel visivel.
- Stats totais (base + refinacao) exibidos.

## 6.4 Log esperado
No Output Log, filtro `UmbraItemTooltip`, devem aparecer linhas como:
- Tooltip construido
- Item refinado sim/nao e nivel
- Populando tooltip normal/refinado

---

## 7) Troubleshooting rapido

## Erro: BindWidget obrigatorio nao encontrado

Causa comum:
- nome errado ou `Is Variable` desmarcado.

Correcao:
- criar/renomear exatamente:
  - `ItemNameText`
  - `ItemTypeText`
  - `RarityText`
- marcar `Is Variable = true`.

## Erro: "The function name SetTooltipData is already used"

Causa:
- funcao Blueprint `SetTooltipData` ainda existe no `WBP_ItemTooltipRefined`.

Correcao:
- remover a funcao Blueprint duplicada e usar a funcao C++ herdada.

## Erro: pin antigo incompativel

Causa:
- grafo antigo esperava assinatura anterior.

Correcao:
- refazer node de `SetTooltipData` e conectar apenas `ItemSlot` (struct completo).

## Tooltip nao aparece

Verificar:
1. classe no `Create Widget` realmente e `WBP_ItemTooltipRefined`;
2. node `SetTooltipData` executa antes de adicionar ao viewport;
3. fluxo de hover ainda esta sendo disparado.

---

## 8) Rollback em 1 passo

Se precisar voltar instantaneamente:
- no `WBP_InventorySlot`, altere a classe do `Create Widget` de volta para `WBP_ItemTooltip`.

Como o legado nao foi alterado, rollback e imediato e seguro.

---

## 9) Estrategia recomendada de migracao

1. Criar `WBP_ItemTooltipRefined`.
2. Validar localmente em poucos slots.
3. Trocar chamada no fluxo principal de hover.
4. Testar inventario inteiro.
5. Manter `WBP_ItemTooltip` como fallback.

Assim, a adocao e incremental e sem risco de quebrar o tooltip atual.

