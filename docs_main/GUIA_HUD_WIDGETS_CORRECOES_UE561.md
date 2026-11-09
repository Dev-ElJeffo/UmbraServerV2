# GUIA — Correções do HUD e dos widgets de item (UE 5.6.1)

A lógica do `WBP_PlayerHUD`, dos popups de pedido (trade/party/amizade) e do tooltip dos slots de item saiu do Event Graph e foi para o C++ (`UUmbraPlayerHudWidget`, `UUmbraRequestPopupWidget`, `UUmbraInventorySlotWidget`). Os WBPs passam a definir só **Parent Class**, layout e Class Defaults.

## Objetivo

- Corrigir bugs encontrados na auditoria dos widgets via MCP:
  - tela de morte que não sumia no respawn;
  - `WBP_PartyPanel` duplicado;
  - binds acumulados ao recriar o HUD;
  - painéis duplicados a cada clique;
  - sussurro para um 2º jogador ignorado;
  - tooltip de item preso na tela;
  - `Print String` na tela;
  - `WBP_DeleteItem` com parent errado.
- **Sem mudança de protocolo, servidor, PHP ou Manager.** Nenhum opcode novo ou alterado.

---

## 0) Pré-requisitos

1. **Fechar o editor** e fazer **rebuild** de `UmbraEternumUEEditor` (VS 2022, *Development Editor*). Classes UCLASS novas não entram por Live Coding.
2. Reabrir o editor. A parte de assets (reparent, limpeza de graph, Class Defaults) é aplicada via MCP **depois** do rebuild.
3. **Parar o PIE** antes de qualquer edição de WBP.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| HUD | `/Game/Widgets/HUD/WBP_PlayerHUD` | `UmbraPlayerHudWidget` |
| Pedido de trade | `/Game/Widgets/UI/PlayerInteraction/Trade/WBP_TradeRequested` | `UmbraRequestPopupWidget` (Kind = Trade) |
| Convite de grupo | `/Game/Widgets/UI/PlayerInteraction/Party/WBP_PartyRequested` | `UmbraRequestPopupWidget` (Kind = Party) |
| Pedido de amizade | `/Game/Widgets/UI/PlayerInteraction/FriendList/WBP_FriendRequested` | `UmbraRequestPopupWidget` (Kind = Friend) |
| Janela de deletar item | `/Game/Widgets/UI/Inventory/WBP_DeleteItem` | `UmbraDraggableWindowWidget` |
| Slots de item | `WBP_InventorySlot`, `WBP_EquipmentSlot`, `WBP_StoreSlot` | (sem troca; `UmbraInventorySlotWidget` e filhos) |

---

## 1) Designer — nomes de bind

### 1.1 `WBP_PlayerHUD` (nomes atuais, não renomear)

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `PB_HP` / `PB_MP` | `UProgressBar` | Não | Percentual de HP/MP (`OnCharacterInfoLoaded`) |
| `TXT_HP` / `TXT_MP` | `UTextBlock` | Não | Texto `HP x/y` / `MP x/y` |
| `BTN_Settings` | `UButton` | Não | `PlayerController->ToggleGameMenu()` |
| `BTN_Skillbook` | `UButton` | Não | `PlayerController->OpenSkillBook()` |
| `BTN_QuestJournal` | `UButton` | Não | `PlayerController->OpenQuestJournal()` (journal único do GameInstance) |
| `BTN_FriendList` | `UButton` | Não | Abre/fecha `WBP_FriendList`; auto-refresh só enquanto aberto |
| `BTN_BlockedPlayerList` | `UButton` | Não | Abre/fecha `WBP_BlockedPlayersList`; auto-refresh só enquanto aberto |
| `BTN_Mailbox` | `UButton` | Não | Abre/fecha `WBP_Mail` + `OpenMail()` |
| `BTN_PlayerStore`, `BTN_Auction`, `BTN_GuildPanel`, `BTN_RefinementPanel`, `BTN_StoragePanel`, `BTN_OpenEnchant`, `BTN_Craft` | `UButton` | Não | Abre/fecha o painel (instância única) |

### 1.2 Popups de pedido

| Nome do bind | Tipo | Função |
|--------------|------|--------|
| `BTN_Accept` | `UButton` | Aceita (`AcceptTradeRequest` / `AcceptPartyInvite` / `AcceptFriendRequest`) |
| `BTN_Decline` | `UButton` | Recusa (`Decline*` correspondente) |
| `Btn_Close` | `UButton` | Só fecha; o pedido continua pendente no servidor |
| `Text_RequestFrom` / `Text_TradeFrom` / `Text_PartyFrom` | `UTextBlock` | Texto do pedido (o C++ usa o primeiro que existir) |

**Class Defaults** dos popups: `RequestKind` (Trade/Party/Friend) e `MessageFormat` (opcional, `{0}` = nome). Se vazio, usa o texto padrão do tipo.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraPlayerHudWidget` | `Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` | Bind de 12 delegates do GI + menu de contexto do `PlayerSelectionComponent` no `NativeConstruct`, **unbind** no `NativeDestruct`; vitais; chat e skill bar criados **uma vez**; painéis com instância única (`TogglePanel`); tela de morte guardada e removida no respawn; whisper troca de conversa sempre; inspect reaproveita a janela (`UpdateInspectedPlayerInfo`); dedupe de pedidos por (tipo, jogador); desselecionar o jogador fecha o menu de contexto. **Não** cria `WBP_PartyPanel` (o PlayerController é o dono). |
| `UUmbraRequestPopupWidget` | `Source/UmbraEternumUE/UI/UmbraRequestPopupWidget.cpp` | Texto, aceitar, recusar e fechar dos 3 popups; avisa o HUD ao fechar. O `WBP_FriendList` (pedidos pendentes) chama o `ShowRequest` do C++. |
| `UUmbraInventorySlotWidget` | `Source/UmbraEternumUE/UI/UmbraInventorySlotWidget.cpp` | Tooltip de item por `SetToolTip` nativo: uma instância por slot, atualizada no hover, some ao sair. Vale para inventário, equipamento, lixeira e loja. Desligável por `bShowItemTooltip`. |
| `UUmbraDraggableWindowWidget` | `Source/UmbraEternumUE/UI/UmbraDraggableWindowWidget.cpp` | Área de arraste **opt-in**: se `DragAreaWidget` não foi definido e `DefaultDragAreaName` está preenchido nos Class Defaults (ex.: `Border_TitleBar` no `WBP_DeleteItem`), usa esse widget. Vazio por padrão, porque no inventário/storage o arraste da janela compete com o drag&drop de itens. |

> **Não** adicione lógica no Event Graph desses WBPs.

**Ponte temporária:** o `WBP_Trade` (`InitializeTrade`) e o `WBP_ReportPlayer` (`ShowReportDialog`) ainda têm funções só em BP. O HUD chama essas funções por reflexão (`CallWidgetFunction`) até esses dois widgets irem para C++.

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---------|----------|
| Morrer e renascer 2 vezes | A tela de morte some sozinha nas duas vezes; nunca empilha |
| Formar grupo | Um único painel de party |
| Clicar 2x em cada botão do HUD | Abre e fecha; nunca duas janelas iguais |
| Fechar a lista de amigos | Output Log sem `get_friend_list` a cada 5s |
| Pedir trade/party/amizade (P2 → P1) | Popup aparece; aceitar e recusar funcionam; pedido repetido não empilha |
| Sussurrar para A e depois para B | A janela de whisper troca para B |
| Inspecionar outro jogador | Abre a janela de info com os dados dele (somente leitura) |
| Passar o mouse rápido por inventário/equipamento/loja | Tooltip aparece no hover e nunca fica preso |
| Learn/upgrade de skill com o livro aberto | Painel de detalhe atualiza |
| Arrastar a janela de deletar item pelo título; deletar um item | Arrasta; o item some |
| Trocar de mapa / relogar | Nenhum popup ou bind duplicado (eventos chegam 1x) |
| Abrir o menu de contexto de um jogador e clicar no chão (desselecionar) | O menu fecha |
| Abrir a lista de amigos com pedido pendente | O popup mostra o nome e aceitar/recusar funcionam |
| FriendList aberta → clicar num amigo → usar o painel de opções | O painel responde sem fechar a lista |
| Abrir e fechar a FriendList 3 vezes | Output Log sem `Ensure ... InvocationList` |
| Arrastar do inventário para trade, refino, storage, enchant e crafting | O item entra no slot de destino |
| Clicar em Leave Party | Sai do grupo |
| Usar skill pela tecla (`1`, `Q+W`) depois de clicar no mundo | A skill dispara; com o cursor no campo do chat, não dispara |
| P2 clica no botão da loja pessoal de P1 | Abre a loja do vendedor |
| Arrastar o `WBP_DeleteItem` pelo título | A janela move |

Logs esperados: `"[UmbraPlayerHudWidget] HUD construído."` (1x por HUD).

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| `[UmbraPlayerHudWidget] Classe não encontrada: ...` | Asset movido/renomeado | Preencher a classe nos Class Defaults do `WBP_PlayerHUD` (categoria `Umbra|HUD|Classes`) |
| HUD não aparece e o clique direito não abre o menu do jogador | Level BP do mapa ainda chama uma função BP antiga do HUD (`Initialize HUD`) e não compila, então o BeginPlay não roda | Abrir o Level BP do mapa, apagar o nó antigo e salvar o mapa. O Level BP só precisa de Create Widget + Add to Viewport |
| Cliques em outras janelas (Leave Party, botões no mundo) ou hotkeys de skill param com o HUD aberto | Raiz de um widget em tela cheia (chat, painel) ficou `Visible` e captura o clique; no chat o clique dá foco ao campo de texto e as hotkeys são ignoradas | O C++ não força `Visible` na raiz; conferir se algum BP chama `Set Visibility (Visible)` no próprio widget e trocar por `Self Hit Test Invisible` |
| Arrastar item do inventário não chega no trade/refino/storage/enchant/crafting | Janela de inventário/storage com área de arraste (`DefaultDragAreaName`) preenchida | Deixar `DefaultDragAreaName` vazio nessas janelas |
| Painel aberto por outro painel (ex.: opções do amigo) não responde | ZOrder do painel pai acima do filho | Painéis do HUD usam ZOrder 0; a janela mais recente fica por cima |
| Botão do HUD não faz nada | Nome do botão diferente do bind | Conferir o nome exato na tabela 1.1 (`/conferir-binds-ue`) |
| `Função 'InitializeTrade' não existe` | Função BP do `WBP_Trade` renomeada | Restaurar o nome ou migrar o `WBP_Trade` para C++ |
| Dois tooltips no slot | Sobrou `On Mouse Enter`/`Leave` no BP do slot | Apagar esses eventos no WBP |
| Aviso `ValidateVariableNames ... already taken` | Variável BP com nome igual a uma propriedade C++ | Remover a variável BP e usar a herdada |
| Botão de journal não abre | `QuestJournalWidgetClass` vazio no `BP_UmbraGameInstance` | Preencher com `WBP_QuestJournal` |

## 8) Assets removidos e pendências

- **Removidos** (sem referenciadores no Asset Registry nem no C++, conferido via MCP antes de apagar; depois, `/Game/Widgets` compilou sem erros): `WBP_ItemTooltip` (substituído pelo `WBP_ItemTooltipRefined`), `WBP_login` (o mapa usa `WBP_Login2`) e `WBP_CharacterInfo` (a inspeção usa `WBP_CharacterInfo1`).
- **Mantido:** `WBP_SkillSlot` continua em uso. O `WBP_SkillBar1` cria os slots da barra com ele (`PopulateMainSkillBar`/`PopulateSecondarySkillBar`).
- Removida a variável `WBPMainMenu` do `BP_ThirdPersonGameMode` (só segurava o `WBP_login`).
- **Pendente:** 38 referências a sons de `/Engine/VREditor/...` (Login2, CreateCharacter, `WBP_SkillBookEntry`, Storage, slots). O cooker inclui esses sons, mas o ideal é trocar por SoundWaves do projeto em `/Game/Audio/UI/`.
- **Pendente:** o botão de deletar do `WBP_Inventory` cria um `WBP_DeleteItem` novo a cada clique (mesmo padrão dos painéis do HUD antigo).
- **Próximo ciclo:** drag&drop do inventário, `WBP_Trade` e `WBP_ReportPlayer` em C++, e Login2/Register/CharacterSelection/CreateCharacter.

## 9) Referência de código

- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` (`NativeConstruct`, `TogglePanel`, `ShowRequestPopup`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraRequestPopupWidget.cpp`
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraInventorySlotWidget.cpp` (`RefreshSlotTooltip`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraDraggableWindowWidget.cpp` (`NativeConstruct`)

**Fim do guia.**
