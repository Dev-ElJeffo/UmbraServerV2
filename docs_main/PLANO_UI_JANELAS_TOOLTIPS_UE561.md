# Plano — Janelas, tooltips e consistência visual da UI (UE 5.6.1)

Branch: `feat/ui-janelas-tooltips` (principal e submódulo `UmbraEternumUE`).
Origem: análise via MCP (EngineWire) de 27 WBPs (HUD, janelas, tooltips, menus, popups) + leitura do C++ em `Source/UmbraEternumUE/UI`.

## Decisões fechadas

| Tema | Decisão |
|------|---------|
| Idioma | **pt-BR como cultura nativa**, preparado para EN (ver §Localização) |
| Visual | **Banner vermelho** na barra de título, título dourado, X ornamentado escuro |
| Migração dos WBPs | **Via MCP** (EngineWire), lote a lote, depois que o C++ compilar |
| Lógica | Toda em C++; WBP só layout/estilo (regra do Blueprint mínimo) |

## Localização (pt-BR nativo, EN depois)

- Texto em C++ sempre como `FText` com `LOCTEXT`/`NSLOCTEXT` (namespace `"UmbraUI"`); nunca `FText::FromString` para texto fixo de UI.
- Texto de WBP fica como `FText` localizável (padrão do UMG); nada de texto "culture invariant".
- Textos compartilhados (Fechar, Confirmar, Cancelar, títulos de janela) vão para a StringTable `/Game/UI/Localization/ST_UmbraUI` e são referenciados por chave.
- Localization Dashboard: target `Game`, **cultura nativa `pt-BR`**, gather de `Source/` (C++) e `/Game/Widgets` + `/Game/UI` (assets). Para EN: adicionar a cultura `en`, rodar Gather → Export (.po) → traduzir → Import → Compile.
- Strings vindas do servidor (nomes de item, skill, quest) continuam dados; a tradução delas é outro escopo.

## Diagnóstico resumido

### Janelas
- **Nenhuma janela arrasta**: só Inventory, QuestJournal e Minimap herdam `UUmbraDraggableWindowWidget`, e só o minimapa define a área de arraste.
- **Bug de coordenada**: `GetAbsolutePosition()` (desktop) é passado para `SetPositionInViewport(.., false)` (pixels do viewport); a janela pula em modo janela/PIE.
- Sem clamp na tela; raiz de cada WBP é um CanvasPanel de tela cheia (mover o widget inteiro impede clamp).
- Posições padrão aleatórias e sobrepostas (CharInfo, Storage, Guild, Auction e Vendor no mesmo canto; Party colado na borda).
- ZOrder sem regra (0, 15, 80, 100, 101, 250–280, 450, 500, 1000); clicar não traz a janela para frente.
- ESC sempre abre o Game Menu, em vez de fechar a janela de cima.
- Só o minimapa persiste posição.
- Quest Interaction abre em cima da barra de HP/MP.

### Tooltips (5 sistemas)
- `SetToolTip` nativo (item, stat, quest): posição do Slate, sem âncora; o tooltip de item tem título e botão X.
- Buff: widget no viewport com flip/clamp (o único correto).
- Skill slot: `BlueprintImplementableEvent` de hover (lógica no BP).
- Refinement: tooltip próprio.
- `SetToolTipText` em botões (estilo padrão da UE).

### Visual
- 3 estilos de botão de fechar, 3 de barra de título, 4 de botão (inclusive cinza padrão da UE).
- ComboBox/TextBox/Slider sem estilo.
- Placeholders brancos (ícones e slots) e textos de designer visíveis ("Bloco de Texto", "Minimized_Summary", "Text_AttachLabel", "Trade Success!", "000").
- Erros de digitação: "Equipped Itens", "Browse Itens", "Avaliable Points", "Appy Crystal", "Sucess Rate", "Scalling".
- Layouts quebrados: rodapé do SkillBook cortado, "Type"/"Element" sobrepostos no SkillTooltip, linhas vazias e colunas desiguais no CharInfo, Refinement desalinhado e com moldura dupla, "Leave Party" como título, metade vazia no Trade.
- HUD: skill bar no topo-direita (lugar do minimapa); Level Up no centro tapando a visão.

## Fases

### Fase 1 — Infra de janelas (C++)
- **`UUmbraWindowWidget`** (evolui `UUmbraDraggableWindowWidget`, que vira alias):
  - binds `BindWidgetOptional`: `Border_Window` (painel que se move), `Border_TitleBar` (arraste), `Text_Title`, `Btn_Close`;
  - arraste move o `UCanvasPanelSlot` do `Border_Window` dentro do canvas raiz (coordenadas locais do canvas);
  - clamp mantendo a barra de título visível; reaplica ao mudar resolução;
  - clique na janela → `BringToFront`; `Btn_Close`/ESC → `CloseWindow`;
  - config `EditDefaultsOnly`: `WindowId`, `DefaultPlacement`, `bDraggable`, `bRememberPosition`, `WindowLayer`;
  - canvas raiz em `SelfHitTestInvisible`.
- **`UUmbraWindowManagerSubsystem`** (`ULocalPlayerSubsystem`):
  - registro e pilha de foco; camadas fixas de ZOrder:

    | Camada | ZOrder |
    |--------|--------|
    | HUD | 0–9 |
    | Janelas | 100–199 (re-stack ao focar) |
    | Popup/convite | 300 |
    | Modal/confirmação | 350 |
    | Game Menu | 450 |
    | Options | 460 |
    | Tooltip | 1000 |
    | Drag visual | 1100 |

  - ESC fecha a janela do topo; só abre o Game Menu com a pilha vazia;
  - posição: salva → `DefaultPlacement` → cascata de +32 px se ocupado;
  - `CloseAllWindows()` e `ResetWindowPositions()`.
- **Persistência**: `WindowPositions` (`TMap<FName, FVector2D>` normalizado) no `UmbraClientSettings`.

### Fase 2 — Tooltip único (C++)
- **`UUmbraTooltipSubsystem`**: `ShowTooltip(Content, Anchor, Placement)`, `ShowTextTooltip(Text, Anchor)`, `HideTooltip(Anchor)`.
- Placement `AnchorRight` (item/skill/equipamento: ao lado do slot, flip à esquerda na borda) e `FollowCursor` (buff/stat/texto: offset 16 px, flip e clamp).
- Atraso de 150 ms, um tooltip por vez, Z1000, `HitTestInvisible`.
- Migra buff, skill slot (remove os eventos de BP), SkillBook entry, slots de item/equipamento/loja, stat, quest, Refinement e textos de botão.
- Tooltip de item ganha `bHoverMode` (esconde título e X).

### Fase 3 — Estilo centralizado
- `UUmbraUIStyle` (`UDataAsset` em `/Game/UI/Style/DA_UmbraUIStyle`): fontes (título 22, seção 18, corpo 14, pequeno 12), paleta (dourado, texto, positivo, negativo, raridades), brushes (moldura, banner vermelho, seção, slot), estilos de botão/combo/slider/textbox, métricas (padding 16, gap 8, título 48, botão 36, slot 64).
- Widgets C++ que leem o estilo em `SynchronizeProperties`: `UUmbraButton` (Primary/Secondary/Danger/Close/Tab), `UUmbraText` (Title/Section/Body/Small/Value), `UUmbraComboBox`, `UUmbraSlider`, `UUmbraEditableText`.
- Tamanhos de janela: **S 420** (Friend, Party, Loot, Popups, Menu), **M 560** (Inventory, Vendor, Trade, Refine, Enchant, Storage, Mail), **L 900** (CharInfo, SkillBook, Guild, Auction, Crafting, Journal, Options).

### Fase 4 — Mapa de tela (1920x1080, safe zone 24 px)

| Região | Conteúdo |
|--------|----------|
| Topo-esquerda | Party frames |
| Topo-centro | Target frame; toasts/broadcast/Level Up abaixo |
| Topo-direita | Minimapa; buffs abaixo |
| Direita-meio | Quest tracker |
| Base-centro | EXP na borda; skill bar acima do HP/MP; cast bar acima da skill bar |
| Base-esquerda | Chat e combat log |
| Base-direita | Micro-menu |
| Centro | Só modais (confirmação, Death Screen) |

| Placement | Janelas |
|-----------|---------|
| `RightColumn` | Inventory |
| `NextToInventory` | Storage, Vendor, Trade, Refine, Enchant, Player Store, Buyer |
| `LeftColumn` | CharInfo, Friend List, Party panel |
| `Center` | SkillBook, Guild, Auction, Crafting, Journal, Mail, Options, Game Menu |
| `TopCenter` | Popups de convite, Quest Interaction, NPC Dialog |
| `NearCursor` | Loot, Context Menu, Split Input |

### Fase 5 — Migração dos WBPs (via MCP)
Por janela: reparent para a classe `Umbra*` (base `UUmbraWindowWidget`), painel renomeado para `Border_Window`, título e botões estilizados, placeholders removidos, correções da lista de diagnóstico.

| Lote | Janelas |
|------|---------|
| 1 | Inventory, CharInfo, SkillBook |
| 2 | Storage, Vendor, Trade, Refinement, Enchant |
| 3 | Friend, Party, Guild, Mail, Whisper |
| 4 | Auction, Crafting, Journal, Options, Game Menu |
| 5 | Popups e HUD |

### Fase 6 — Guia e QA
- `docs_main/GUIA_UI_JANELAS_TOOLTIPS_UE561.md` (template do repo).
- QA em PIE com 2 players, modo janela: arrastar sem pular, clamp, ESC em pilha, posição salva entre sessões, tooltip nas 4 bordas, 1280x720 e 2560x1440.
- `/conferir-binds-ue` a cada lote.

## Status

| Fase | Status |
|------|--------|
| 1 — Infra de janelas | Pronto (build ok); aguarda QA em PIE |
| 2 — Tooltip único | Pronto para item, skill e buff (stat, quest e Refinement pendentes) |
| 3 — Estilo | pendente (lote 1 recebeu banner vermelho e X ornamentado por estilo direto no WBP) |
| 4 — Mapa de tela | pendente |
| 5 — Migração WBPs | Lote 1 feito (Inventory, CharacterInfo1, SkillBook, QuestJournal; BP_ThirdPersonCharacter e WBP_SkillSlot limpos). Lote 2 feito no Editor (Storage, NpcVendor, Trade, Refinement, Enchant) + tracker de quests do HUD; falta gravar DefaultPlacement/WindowTitle de Vendor/Trade/Refinement/Enchant após o rebuild. Lote 3 feito (social). Lote 4 feito no Editor (Auction, Crafting, Guild, Options, Game Menu); defaults de Options e Game Menu gravados (Center, "Configurações"/"Menu") — aguarda QA |
| 6 — Guia e QA | em andamento (guia atualizado a cada lote) |

### Correções incluídas no lote 2

| Bug | Causa | Correção |
|-----|-------|----------|
| Loja pessoal: fechar o painel pelo X com a loja aberta dessincronizava o vendedor | O X liberava o movimento local enquanto a loja seguia aberta | Trava de vendedor: `AUmbraEternumUEPlayerController::IsVendorMovementBlocked` (loja aberta = parado até "Fechar loja"), aplicada em WASD, pulo, follow e auto-ataque; ao abrir a loja, `StopLocalPawnForPersonalShop` para tudo |
| Nome do item não aparecia no tooltip | O modo hover escondia `Border_ItemTooltip`, que contém `Text_Title` (o nome do item) | Modo hover esconde só `Btn_Close` |
| Tracker de quests fora da tela em outras resoluções e sem arraste | Slot com posição absoluta (1580,352) no WBP_PlayerHUD | Base `UUmbraWindowWidget`, slot em tela cheia, barra `Border_TitleBar` ("Missões"), clamp e posição salva |
| ESC na Troca deixaria a sessão aberta no servidor | Fechar pela camada só removia o widget | Gancho `OnWindowCloseRequested` (base) → Troca chama `CancelTrade` uma vez |
| Loja: depois do X não havia como chegar ao "Fechar loja" | Balão do próprio nameplate abria a tela de comprador; HUD podia abrir 2ª instância | Balão do próprio vendedor → `OpenOwnPersonalShopPanel` (painel do dono); `TogglePanel` da HUD consulta o manager |
| Minimapa não se ajustava à resolução | Posição só era aplicada uma vez | Reaplica fração salva/canto padrão com clamp quando o viewport muda |
| Auction, Craft e Guild não arrastavam | Ainda eram `UUserWidget` | Base `UUmbraWindowWidget` + `Border_Window`/`Text_WindowTitle` no WBP (adiantados do lote 4) |

### Lote 3 (social)

| WBP | Classe | Situação |
|-----|--------|----------|
| WBP_Mail | `UmbraMailWidget` → `UmbraWindowWidget` | Migrado; título continua em `Text_Title` (bind C++) |
| WBP_Whisper | `UmbraWhisperWidget` → `UmbraWindowWidget` | Migrado; X renomeado para `Btn_WindowClose` |
| WBP_ReportPlayer | `UmbraReportPlayerWidget` → `UmbraWindowWidget` | Migrado |
| WBP_FriendList / WBP_BlockedPlayersList | `UmbraSimpleWindowWidget` (lógica ainda no Event Graph) | Migrados (coluna esquerda). **Pendente:** levar a lógica do Graph para C++ |

Posições: Friend/Blocked/Whisper `LeftColumn`; Mail/Report `Center`.

Ajustes de apoio: o ESC ignora janelas invisíveis (Mail/Whisper "fecham" com Collapsed); a raiz de tela cheia do Mail e do Whisper não força mais Visible; o toggle da HUD trata Collapsed como fechado; o handler de fechar da base virou `HandleWindowCloseButtonClicked` (o nome antigo colidia com a Report).

### Lote 4 (Auction, Crafting, Guild, Options, Game Menu)

Auction, Crafting e Guild já tinham sido adiantados no lote 2 (ver tabela acima).

| WBP | Classe | Situação |
|-----|--------|----------|
| WBP_GameMenu | `UmbraGameMenuWidget` → `UmbraWindowWidget` | `Border_MenuPanel` → `Border_Window`, `Txt_MenuTitle` → `Text_WindowTitle` ("Menu"), X ornamentado, botões em pt-BR |
| WBP_Options | `UmbraOptionsWidget` → `UmbraWindowWidget` | `Border_BackDrop` → `Border_Window`, `TextTitle` → `Text_WindowTitle` ("Configurações"), X ornamentado, rótulos em pt-BR, botão novo `BTN_ResetWindows` ("Redefinir janelas") |

C++:
- `CloseMenu()` e `CloseOptions()` passam por `CloseWindow()`. Os avisos `OnGameMenuClosed` e `OnOptionsClosed` saem do gancho `OnWindowCloseRequested`, uma vez por abertura (`bCloseNotified`), então X, ESC e botão dão o mesmo resultado.
- `BTN_ResetWindows` chama `UUmbraWindowManagerSubsystem::ResetWindowPositions()` (apaga as posições salvas e reposiciona as janelas abertas) e devolve a própria Options ao padrão.
- Menu e Options continuam fora da camada do manager (Z 450/500 no viewport), porque o PlayerController controla o input deles. O arraste funciona igual.
