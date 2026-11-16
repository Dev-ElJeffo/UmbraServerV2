# GUIA — Janelas e tooltips da UI (UE 5.6.1)

Todas as janelas do jogo passam por uma **camada de janelas** em C++: elas arrastam pela barra de título, ficam presas à tela, vêm para frente quando você clica nelas, fecham com ESC e lembram a posição entre sessões. Os tooltips usam um sistema único que posiciona ao lado do slot ou segue o cursor. O WBP só define a parent class, o layout e os nomes dos binds.

Plano completo e status: `docs_main/PLANO_UI_JANELAS_TOOLTIPS_UE561.md`.

## Objetivo

- Janelas arrastáveis pela barra de título, sem "pular" em PIE ou em modo janela.
- Clique em qualquer ponto de uma janela traz ela para frente; clique no painel não atravessa para o mundo.
- ESC fecha a janela do topo; com nenhuma aberta, abre o Menu.
- Posição salva por janela (SaveGame `UmbraClientSettings`).
- Inventário (I) e Ficha (C) abrem por C++ (antes: Event Graph do `BP_ThirdPersonCharacter`); K alterna o Skill Book.
- Tooltips de item, skill e buff pelo mesmo sistema, sem tooltip duplicado e sem lógica no BP.
- **Sem mudança** de protocolo, PHP, SQL ou Manager.

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (classes novas: `UmbraWindowWidget`, `UmbraWindowManagerSubsystem`, `UmbraTooltipSubsystem`).
2. **Parar o PIE** antes de editar o Designer.
3. A migração dos WBPs é feita pelo Claude via MCP (EngineWire), lote a lote; este guia diz o que conferir.

**Assets do lote 1:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| WBP_Inventory | `/Game/Widgets/UI/Inventory/WBP_Inventory` | `UmbraInventoryWidget` (base `UmbraWindowWidget`) |
| WBP_CharacterInfo1 | `/Game/Widgets/UI/CharacterInfo/WBP_CharacterInfo1` | `UmbraCharacterInfoWidget` (base `UmbraWindowWidget`) |
| WBP_SkillBook | `/Game/Widgets/UI/Skills/WBP_SkillBook` | `UmbraSkillBookWidget` (base `UmbraWindowWidget`) |
| WBP_QuestJournal | `/Game/Widgets/UI/Quest/WBP_QuestJournal` | `UmbraQuestJournalWidget` (base `UmbraWindowWidget`) |
| WBP_ItemTooltipRefined | `/Game/Widgets/UI/Inventory/WBP_ItemTooltipRefined` | `UmbraItemTooltipWidget` |

---

## 1) Designer — padrão de toda janela

### 1.1 Hierarquia

```
[CanvasPanel raiz — tela cheia]          ← o C++ deixa "Not Hit-Testable"
  Border_Window                           ← painel que se move (filho direto do canvas)
    VerticalBox
      Border_TitleBar                     ← área de arraste (banner vermelho)
        HorizontalBox
          Text_WindowTitle                ← título (opcional; WindowTitle do Class Defaults sobrescreve)
          Btn_WindowClose  (ou Btn_Close) ← fecha a janela
      ...conteúdo...
```

### 1.2 Binds

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `Border_Window` | `UBorder` | Não (`BindWidgetOptional`) | Painel arrastado/posicionado. Sem ele, usa o 1º filho do canvas raiz. |
| `Border_TitleBar` | `UBorder` | Não | Área de arraste. Sem ele, a janela não arrasta. |
| `Text_WindowTitle` | `UTextBlock` | Não | Recebe `WindowTitle` (FText localizável) quando ele não está vazio. |
| `Btn_WindowClose` | `UButton` | Não | Fecha pela camada. Sem ele, usa um `UButton` chamado `Btn_Close`. |

### 1.3 Class Defaults (Umbra|Window)

| Propriedade | Padrão | Uso |
|-------------|--------|-----|
| `WindowId` | vazio (= nome do WBP) | Chave da posição salva |
| `WindowTitle` | vazio | Título em pt-BR (localizável) |
| `DefaultPlacement` | `Designer` | `Center`, `LeftColumn`, `RightColumn`, `NextToInventory`, `TopCenter`, `NearCursor` |
| `WindowLayer` | `Window` | `Popup` / `Modal` |
| `bRememberPosition` | ligado | Salva a posição arrastada |
| `bCloseOnEscape` | ligado | ESC fecha quando está no topo |
| `bBlockClickThrough` | ligado | Clique no painel não chega ao mundo |

Valores por janela:

| WBP | DefaultPlacement | WindowTitle |
|-----|------------------|-------------|
| WBP_Inventory | `RightColumn` | Inventário |
| WBP_CharacterInfo1 | `LeftColumn` | Personagem |
| WBP_SkillBook | `Center` | Livro de Habilidades |
| WBP_QuestJournal | `Center` | Diário de Missões |
| WBP_Storage | `NextToInventory` | Baú |
| WBP_NpcVendor | `NextToInventory` | (vazio: o C++ escreve o nome do NPC em `Text_VendorTitle`) |
| WBP_Trade | `NextToInventory` | Troca |
| WBP_RefinementWindow | `NextToInventory` | Refinamento |
| WBP_ItemEnchant | `NextToInventory` | Encantamento de Cristal |
| WBP_QuestHudTracker (HUD) | `Designer` (direita-meio) | Missões |
| WBP_GameMenu | `Center` | Menu |
| WBP_Options | `Center` | Configurações |

### Tracker de quests do HUD

- `WBP_QuestHudTracker` herda `UmbraQuestHudTrackerWidget` → `UmbraWindowWidget`. O painel que se move é `Panel_Expanded` (1º filho do canvas raiz); a barra de arraste é `Border_TitleBar`.
- No `WBP_PlayerHUD`, o slot do tracker ocupa a tela inteira (anchors 0..1, offsets 0). Não volte para posição absoluta.
- Não entra na pilha de janelas: ESC não fecha, e a posição é salva com o id `QuestHudTracker`.

### Fechar janela com regra de negócio

- Todo fechamento pedido (X, ESC, toggle) passa por `UUmbraWindowWidget::CloseWindow`, que chama `OnWindowCloseRequested` antes de remover.
- A Troca usa esse gancho para cancelar a sessão no servidor. Não ligue o X da Troca em outro handler.

### Menu (ESC) e Configurações

- `WBP_GameMenu` e `WBP_Options` herdam `UmbraWindowWidget`: arrastam pela barra `Border_TitleBar` e fecham pelo X `Btn_Close` (fallback da base; na Options ele também é o bind `BTN_Close`, e os dois caminhos acabam em `CloseWindow()`).
- O X, o ESC e os botões **Continuar**/**Fechar** dão o mesmo resultado: o C++ avisa o PlayerController uma vez e restaura o input.
- Botão `BTN_ResetWindows` (**Redefinir janelas**, rodapé da Options): apaga as posições salvas e devolve todas as janelas abertas à posição padrão. Não precisa de Graph.

### Loja pessoal

- Com a loja aberta, o vendedor fica parado (WASD, pulo, follow e auto-ataque bloqueados) até **Fechar loja**. O X só fecha o painel, e a loja continua vendendo.
- O painel pode ser reaberto pelo balão do nameplate ou pelo botão da HUD.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraWindowWidget` | `UI/Window/UmbraWindowWidget.cpp` | Arraste pelo `Border_TitleBar` (move o slot do `Border_Window`), clamp na tela, re-clamp ao mudar resolução, foco no clique (preview), bloqueio de click-through, botão fechar, posição inicial (salva → padrão → cascata). |
| `UUmbraWindowManagerSubsystem` | `Systems/UmbraWindowManagerSubsystem.cpp` | Canvas único das janelas (viewport Z150), ZOrder por camada (Window 1000+, Popup 3000+, Modal 3500+), pilha de foco, ESC, uma instância por classe, posições salvas, "redefinir posições". |
| `UUmbraTooltipSubsystem` | `Systems/UmbraTooltipSubsystem.cpp` | Tooltip único (viewport Z1000): `AnchorRight` (item), `AnchorAbove` (skill bar), `FollowCursor` (buff). Atraso de 0,15 s, flip e clamp nas bordas; some quando o slot sai de baixo do mouse, fecha ou começa drag&drop. |
| `UUmbraDraggableWindowWidget` | `UI/UmbraDraggableWindowWidget.cpp` | Arraste por delta do mouse (corrige o "pulo" em PIE/modo janela); clamp; base do minimapa. |
| `AUmbraEternumUEPlayerController` | `UmbraEternumUEPlayerController.cpp` | `ToggleInventory`, `ToggleCharacterInfo`, `ToggleSkillBook`, `HandleEscapeKey`. |
| `AUmbraEternumUECharacter` | `UmbraEternumUECharacter.cpp` | Liga `IA_OpenInventory` e `IA_Character_Status` (carrega por path se o BP não definir). |
| HUD / GameInstance | `UmbraPlayerHudWidget.cpp`, `UmbraGameInstance.cpp` | Painéis, convites, troca, whisper, inspeção, loot, vendor, diálogo de NPC, quests e baú abrem pela camada (`UmbraUI::ShowWindow`). |
| Slots | `UmbraInventorySlotWidget.cpp`, `UmbraSkillSlotWidget.cpp`, `UmbraBuffIconWidget.cpp` | Tooltips pelo subsystem; o tooltip de item entra em modo hover (esconde a faixa "Item Info" e o X). |

> **Não** adicione lógica no Event Graph. Os nós de abrir Inventário/Ficha no `BP_ThirdPersonCharacter` e os eventos `OnSlotHoverBegin/End` do `WBP_SkillSlot` são removidos pela migração (via MCP), porque o C++ já faz isso.

### Camadas de ZOrder (viewport)

| Camada | ZOrder |
|--------|--------|
| HUD (vitais, chat, skill bar, minimapa, party) | 0 |
| Target frames / números de dano | 100 |
| **Camada de janelas** (Window/Popup/Modal) | 150 |
| Menu de contexto, chances de CC, tela de morte | 300 |
| Menu ESC | 450 |
| Options | 460–500 |
| Captura de atalho | 500 |
| **Tooltips** | 1000 |

---

## 3) Localização (pt-BR nativo, EN depois)

- Títulos e textos fixos como `FText` localizável (padrão do UMG). No C++, `LOCTEXT`/`NSLOCTEXT` com namespace `UmbraUI`.
- Para EN: **Tools → Localization Dashboard** → target `Game`, cultura nativa `pt-BR`, adicionar `en` → **Gather Text** → **Export** → traduzir o `.po` → **Import** → **Compile**.

---

## 6) QA (PIE)

PIE com **Number of Players = 2**, **New Editor Window (PIE)**, janela fora do canto (0,0) da tela.

| Cenário | Esperado |
|---------|----------|
| I / C / K | Abre e fecha a janela (uma instância só) |
| Arrastar pela barra de título | Move sem pular; não sai da tela |
| Arrastar pelo conteúdo/slot | Não move a janela; drag&drop de item normal |
| Clicar numa janela atrás de outra | Ela vem para frente |
| ESC com 2 janelas abertas | Fecha a de cima, depois a outra, depois abre o Menu |
| ESC com Menu/Options abertos | Fecha Options, depois o Menu |
| Fechar e reabrir após arrastar | Abre na posição arrastada; vale também depois de reiniciar o PIE |
| Mudar a resolução (Options) | Janelas continuam dentro da tela |
| Hover em item do inventário perto da borda direita | Tooltip vira para a esquerda; sem "Item Info"/X |
| Hover em skill da barra | Tooltip acima do slot |
| Hover em buff | Tooltip segue o cursor e vira nas bordas |
| Começar a arrastar item com tooltip aberto | Tooltip some |
| Clicar no painel de uma janela | Personagem não anda/não seleciona alvo atrás |
| Player 2 | Mesmo comportamento, independente do player 1 |
| Loja aberta → X do painel → tentar andar/pular | Personagem não sai do lugar; P2 vê o vendedor parado no mesmo ponto |
| Loja aberta → "Fechar loja" | Movimento volta; P2 vê o balão sumir |
| Troca aberta → ESC | Janela fecha e P2 recebe o cancelamento |
| Hover em item | Nome do item aparece na faixa do tooltip, sem o X |
| Tracker de quests: arrastar pela barra "Missões" e mudar a resolução | Move, não sai da tela e mantém a posição relativa |
| Menu (ESC) → X do menu | Menu fecha, cursor e movimento voltam ao normal |
| Menu → Opções → X da Options | Volta ao Menu com o foco nele |
| Arrastar Menu e Options pela barra | Movem sem pular e não saem da tela |
| Arrastar inventário → Options → **Redefinir janelas** | Inventário volta à coluna direita; ao reabrir, continua no padrão |

Logs esperados: `"[UmbraPlayerController] WBP_SkillBook aberto (camada de janelas)."`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Inventário abre e fecha na mesma tecla | Nós `IA_OpenInventory` ainda no Event Graph do `BP_ThirdPersonCharacter` | Remover os nós (migração MCP) |
| Janela não arrasta | Sem `Border_TitleBar` ou parent class antiga | Renomear a barra de título; conferir parent |
| Janela invisível ao abrir | Sem tick e sem layout no 1º frame | O manager revela em 0,25 s; se persistir, conferir se `Border_Window` é filho direto do canvas raiz |
| Clique no mundo não funciona com a janela aberta | Canvas raiz ou um painel de tela cheia com Visibility = Visible | Deixar o canvas raiz "Not Hit-Testable (Self Only)" |
| Dois tooltips no item | `SetToolTip` manual em algum WBP | Remover; o slot usa o subsystem |
| Tooltip de skill duplicado | Nós `OnSlotHoverBegin` ainda no `WBP_SkillSlot` | Remover os nós (o evento não é mais disparado) |
| Janela atrás de outra janela legada | Widget aberto por `AddToViewport` com Z < 150 | Abrir por `UmbraUI::ShowWindow` |

## 8) Referência de código

- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/Window/` (`UmbraWindowTypes.h`, `UmbraWindowWidget`, `UmbraUILayerWidget`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Systems/UmbraWindowManagerSubsystem.*`, `UmbraTooltipSubsystem.*`
- Cliente: `UmbraEternumUEPlayerController.cpp` (`ToggleInventory`, `ToggleCharacterInfo`, `HandleEscapeKey`)
- SaveGame: `Core/UmbraClientSettings.h` (`WindowPositions`)

**Fim do guia.**
