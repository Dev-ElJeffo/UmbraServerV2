# GUIA — Trade e denúncia em C++, janela de deletar item e party na zone (UE 5.6.1)

Rodada 2 do `docs_main/PLANO_RODADAS_UI_UE561.md`. A R2.3 (drag&drop dos slots em C++) ficou fora: o BP atual funciona e ela entra quando houver motivo para mexer nos slots.

O que mudou:
- A lógica da janela de troca (`WBP_Trade`) e da denúncia (`WBP_ReportPlayer`) foi para C++. O HUD não chama mais funções de BP por reflexão.
- O botão "Delete Items" abre uma janela só.
- Na zone, sair do grupo por desconexão funciona igual ao Leave Party, e os avisos de party vão só para os membros do grupo.

## Objetivo

- **Trade:**
  - polling do estado num timer C++ que só roda com a janela aberta;
  - erro de ação aparece na própria janela (antes era um `Print String`);
  - quem não clicou por último também recarrega o inventário quando a troca conclui;
  - o modal de ouro abre uma vez só.
- **Denúncia:** valida o motivo (mínimo de 10 caracteres), mostra "Denúncia registrada." e fecha sozinha.
- **Deletar item:** clicar de novo em "Delete Items" fecha a janela, sem empilhar cópias.
- **Party na zone:**
  - se um membro cai e sobra 1, o grupo se desfaz (igual ao `leave_party.php`);
  - os avisos **13**, **14** e **27** vão só para os membros conhecidos no cache de party da zone. Sem cache (grupo recém-criado), continuam em broadcast, e o cliente filtra desde a Rodada 1.
- **Protocolo:** nenhum opcode novo ou alterado; os formatos dos opcodes 13/14/27 são os mesmos.

---

## 0) Pré-requisitos

1. **Fechar o editor** e fazer **rebuild** de `UmbraEternumUEEditor` (Development Editor). Há classe nova (`UUmbraReportPlayerWidget`) e `.h` alterados.
2. Reabrir o editor. Pelo MCP, Claude faz o **reparent** do `WBP_ReportPlayer` para `UmbraReportPlayerWidget`, compila e salva. Só dá para fazer depois do rebuild.
3. `zone_server` recompilado (já feito: `cmake --build . --config Release --target zone_server`) e **reiniciado**.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| Janela de troca | `/Game/Widgets/UI/PlayerInteraction/Trade/WBP_Trade` | `UmbraTradeWidget` (já era) |
| Denúncia | `/Game/Widgets/UI/PlayerInteraction/Report/WBP_ReportPlayer` | `UmbraReportPlayerWidget` (reparent após o rebuild) |
| Inventário | `/Game/Widgets/UI/Inventory/WBP_Inventory` | `UmbraInventoryWidget` (sem troca) |

O que saiu dos BPs (via MCP):
- `WBP_Trade`: 121 nós do EventGraph, as funções `InitializeTrade`, `UpdateTradeUI` e `CreateTradeSlots`, e 13 variáveis.
- `WBP_ReportPlayer`: 39 nós, a função `ShowReportDialog` e 2 variáveis.
- `WBP_Inventory`: o `On Clicked (BTN_Delete_Item)` com o `Create Widget`.

## 1) Designer — nomes de bind (já existem, não renomear)

### `WBP_Trade`

| Nome do bind | Tipo | Função |
|---|---|---|
| `Grid_TradeSlot1` / `Grid_TradeSlot2` | `UniformGridPanel` | O C++ monta 10 slots em cada (5 colunas): minha oferta / oferta recebida |
| `Text_Gold_Info` / `Text_Gold_Info_2` | `TextBlock` | `Gold: N` meu / do parceiro |
| `Text_ItemCount` | `TextBlock` | `Slots: N/10` |
| `Text_TradeInfo` | `TextBlock` | Status ("Trade Accepted", "Trade Success!") e mensagens de erro |
| `BTN_Accept_Trade` | `Button` | Marca pronto (`SetTradeReady`) |
| `BTN_Decline_Trade` / `Btn_Close` | `Button` | Cancela a troca e fecha |
| `BTN_Add_Gold` / `BTN_Remove_Gold` | `Button` | Abre `WBP_TradeGoldAmount` / `WBP_TradeRemoveGold` (um modal por vez) |

**Class Defaults** (`Umbra|Trade`, opcionais):
- `SlotWidgetClass`, `AddGoldModalClass`, `RemoveGoldModalClass`. Vazios: carregam os paths padrão.
- `PollingIntervalSeconds` (1,5), `SlotsPerSide` (10), `SlotColumns` (5), `CloseDelayAfterSuccess` (1,0).

### `WBP_ReportPlayer`

| Nome do bind | Tipo | Função |
|---|---|---|
| `TX_PlayerName` | `TextBlock` | "Denunciar {nome}" |
| `EDT_Reason` | `MultiLineEditableText` | Motivo |
| `TX_Error` | `TextBlock` | Validação, erro do servidor ou "Denúncia registrada." |
| `BTN_Submit` | `Button` | Envia (`ReportPlayer`) |
| `BTN_Cancel` / `Btn_Close` | `Button` | Fecha |

**Class Defaults** (`Umbra|Report`): `MinReasonLength` (10) e `CloseDelayAfterSuccess` (1,5).

### `WBP_Inventory`

| Nome do bind | Tipo | Função |
|---|---|---|
| `BTN_Delete_Item` | `Button` | Abre/fecha o `WBP_DeleteItem` (instância única). Class Default `DeleteItemWindowClass` opcional |

## 2) O que o C++ e a zone já fazem

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraTradeWidget` | `Source/UmbraEternumUE/UI/UmbraTradeWidget.cpp` | Monta os slots no Construct. Os meus aceitam drop do inventário e drag de volta; os do parceiro são só leitura. `StartTrade` carrega o estado e liga o polling. Atualiza slots, ouro e contador; fecha ao concluir ou cancelar e recarrega o inventário. Erro vai para `Text_TradeInfo`. Unbind e parada do timer no Destruct |
| `UUmbraReportPlayerWidget` | `Source/UmbraEternumUE/UI/UmbraReportPlayerWidget.cpp` | `OpenReport(PlayerID, Nome)`; valida o motivo; envia `ReportPlayer`; trata `OnPlayerReported` / `OnPlayerReportFailed` |
| HUD | `Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` | `OnTradeStarted` → `StartTrade`; `OnReportPlayerRequested` → `OpenReport`. A ponte por reflexão (`CallWidgetFunction`) foi removida |
| `UUmbraInventoryWidget` | `Source/UmbraEternumUE/UI/UmbraInventoryWidget.cpp` | `BTN_Delete_Item` → `ToggleDeleteItemWindow` (abre/fecha uma instância). Binds do GI com `AddUniqueDynamic` |
| Zone — desconexão | `src/zone/ZoneServer.cpp` (`removePlayerFromParty`) | Com 1 membro (ou menos) restante, apaga o grupo e o membro restante |
| Zone — avisos de party | `src/zone/MovementServer.hpp` (`sendPartyNoticeUnlocked`) | 13/14/27 para os membros no cache de party (`getPartyMembersCached`); cache vazio → broadcast. Na desconexão, os membros são lidos do cache **antes** de sair do grupo |

> **Não** adicione lógica no Event Graph desses WBPs. Os modais de ouro (`WBP_TradeGoldAmount` / `WBP_TradeRemoveGold`) continuam em BP: só chamam `AddTradeGold` / `RemoveTradeGold` do GameInstance.

---

## 6) QA (PIE)

PIE com **Number of Players = 3** (o 3º serve para os cenários de party).

| Cenário | Esperado |
|---------|----------|
| P1 pede troca a P2, P2 aceita | Janela de troca nos dois, com 10 + 10 slots, `Gold: 0` e `Slots: 0/10` |
| Arrastar item do inventário para "My Offer" | O item aparece nos dois clientes em até ~1,5 s (no outro, em "Received Offer") |
| Arrastar o item da troca de volta ao inventário | Sai da oferta nos dois |
| Add Gold / Remove Gold | Abre um modal só; o ouro atualiza nos dois |
| Os dois clicam Accept | "Trade Success!", a janela fecha e o inventário dos **dois** atualiza |
| Decline ou X | A janela fecha nos dois |
| Logo depois de uma troca (concluída ou recusada), o **outro** jogador pede uma nova | O pedido chega e a troca abre normalmente |
| Ação inválida (ex.: ouro acima do saldo) | A mensagem aparece em `Text_TradeInfo`, sem texto azul na tela |
| Abrir e fechar a troca várias vezes | Output Log sem `get_trade_state` depois que a janela fecha |
| Menu de contexto → Report: motivo com menos de 10 caracteres | "O motivo deve ter pelo menos 10 caracteres." |
| Report com motivo válido | "Denúncia registrada." e a janela fecha sozinha |
| Clicar 2x em "Delete Items" | Abre e fecha; nunca duas janelas |
| Grupo de 2: P2 fecha o client (desconexão) | P1 fica sem grupo e o painel some |
| Grupo de 3: P3 desconecta | P1 e P2 continuam no grupo; o painel atualiza |
| P3 fora do grupo durante tudo | Nenhum painel de party e nenhum 403 no log do P3 |

Logs esperados:
- `[UmbraTradeWidget] Troca iniciada: sessão N`;
- na zone, `Sent PartyMemberLeft for party N (player X disconnected, K membros no cache)`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Janela de troca abre sem slots | `Grid_TradeSlot1/2` renomeado ou `WBP_InventorySlot` movido | Conferir o nome do bind; preencher `SlotWidgetClass` |
| Log `TradeWindowClass não herda de UmbraTradeWidget` | Parent do `WBP_Trade` trocado | Parent = `UmbraTradeWidget` |
| Report não faz nada e o log diz `ReportWidgetClass não herda de UmbraReportPlayerWidget` | Reparent do `WBP_ReportPlayer` não aplicado | Reparent para `UmbraReportPlayerWidget` (passo 0.2) |
| Itens da troca não aparecem no outro cliente | Polling parado (janela fechada) ou erro no `get_trade_state.php` | Ver `Text_TradeInfo` e o Output Log |
| Quem ficou sozinho após desconexão continua no grupo | `zone_server` antigo | Reiniciar a zone com o binário novo |
| Segunda troca com o mesmo jogador não sai (nada acontece ao pedir) | Binário antigo: quem não clicou Accept por último (ou não clicou Decline) ficava com a sessão presa no GameInstance ("Já está em uma troca") | Rebuild. O `OnLoadTradeStateComplete` agora limpa a sessão quando o polling vê `completed`/`cancelled` e dispara `OnTradeCompleted`/`OnTradeCancelled` |

## 8) Referência de código

- Cliente:
  - `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraTradeWidget.cpp`;
  - `UmbraReportPlayerWidget.cpp`;
  - `UmbraInventoryWidget.cpp` (`ToggleDeleteItemWindow`);
  - `UmbraPlayerHudWidget.cpp` (`HandleTradeStarted`, `HandleReportPlayerRequested`).
- Servidor:
  - `src/zone/MovementServer.hpp` (`sendPartyNoticeUnlocked`, opcodes 13/14/27, `finishClientDisconnect`);
  - `src/zone/ZoneServer.cpp` (`removePlayerFromParty`).

**Fim do guia.**
