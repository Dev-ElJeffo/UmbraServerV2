# Guia Passo a Passo: WBP_Trade - Sistema de Troca Completo

Este guia detalha a implementação do **WBP_Trade** no Unreal Engine para que o sistema de troca funcione conforme arquitetado.

---

## Pré-requisitos (já concluídos)

- [x] SQL `add_trade_gold_columns.sql` executado
- [x] APIs: add_trade_item, remove_trade_item, set_trade_gold, get_trade_state, set_trade_ready
- [x] UmbraGameInstance: AddTradeItem, RemoveTradeItem, SetTradeGold, SetTradeReady, LoadTradeState, CancelTrade
- [x] WBP_TradeRequested + fluxo de aceitar → OnTradeStarted
- [x] OnTradeStateLoaded passa **FUmbraTradeState** (parse em C++, sem VaRest no Blueprint) — ver `GUIA_ANALISE_TRADE_API_PATTERN.md`

---

## Passo 1: Abrir WBP_Trade ao Iniciar a Troca

### 1.1 HUD (WBP_PlayerHUD ou equivalente)

No **Event Construct** ou **Event BeginPlay** do HUD:

1. **Get Game Instance** → **Cast to UmbraGameInstance** (guardar em `MyGameInstance`)
2. **Bind Event to On Trade Started**  
   - Target = `MyGameInstance`  
   - Event = **Custom Event** (criar: `OnTradeStarted_Custom`)

### 1.2 Custom Event OnTradeStarted_Custom

Crie um Custom Event com um parâmetro **Trade Session ID** (Integer).

```
[Custom Event: OnTradeStarted_Custom(TradeSessionID)]
    │
    ├──► Create Widget
    │         Class = WBP_Trade
    │         Owning Player = Get Owning Player (ou Get Player Controller 0)
    │         [guardar em TradeWidgetRef]
    │
    ├──► Chamar função do widget: InitializeTrade
    │         Target = TradeWidgetRef
    │         Trade Session ID = TradeSessionID (do parâmetro)
    │
    └──► Add to Viewport
              Target = TradeWidgetRef
```

**Importante:** O WBP_Trade precisa de uma função `InitializeTrade(TradeSessionID)` que será criada no Passo 2.

---

## Passo 2: Estrutura do WBP_Trade

### 2.1 Variáveis do Widget

Crie no WBP_Trade:

| Nome | Tipo | Editable | Descrição |
|------|------|----------|-----------|
| `TradeSessionID` | Integer | Não | ID da sessão de troca |
| `PollingTimerHandle` | Timer Handle | Não | Para o timer de polling |
| `MySlots` | Array of Widget | Não | 10 slots para meus itens |
| `PartnerSlots` | Array of Widget | Não | 10 slots para itens do parceiro |

### 2.2 Função InitializeTrade

Crie a função **InitializeTrade** com parâmetro `TradeSessionID` (Integer):

```
[InitializeTrade(TradeSessionID)]
    │
    ├──► Set TradeSessionID = TradeSessionID
    │
    ├──► Clear Timer by Handle (PollingTimerHandle)  ← limpar timer anterior se houver
    │
    ├──► Load Trade State
    │         Target = Get Game Instance → Cast to UmbraGameInstance
    │         Trade Session ID = TradeSessionID
    │
    └──► Set Timer by Event
              Time = 2.0  (ou 1.5 segundos)
              Looping = true
              Event = [Custom Event: OnPollingTimer]
              [guardar em PollingTimerHandle]
```

### 2.3 Bind de OnTradeStateLoaded

No **Event Construct** do WBP_Trade:

1. **Get Game Instance** → **Cast to UmbraGameInstance**
2. **Bind Event to On Trade State Loaded**  
   - Target = MyGameInstance  
   - Event = **Custom Event** `OnTradeStateLoaded_Custom`

### 2.4 Custom Event OnTradeStateLoaded_Custom

Parâmetro: `TradeState` (FUmbraTradeState)

O C++ faz o parse do JSON e passa o struct **FUmbraTradeState** no delegate. O Blueprint **nunca** chama VaRest – segue o mesmo padrão das outras APIs (LoadInventory, LoadCharacterInfo, etc.).

```
[Custom Event: OnTradeStateLoaded_Custom(TradeState)]
    │
    └──► UpdateTradeUI
              Target = self
              TradeState = TradeState (parâmetro do evento)
```

A função **UpdateTradeUI** será criada no Passo 4 para preencher os slots e gold usando o struct.

### 2.5 Custom Event OnPollingTimer

```
[Custom Event: OnPollingTimer]
    │
    └──► Load Trade State
              Target = Get Game Instance → Cast to UmbraGameInstance
              Trade Session ID = TradeSessionID (variável do widget)
```

---

## Passo 3: Layout dos Slots e Campos

### 3.1 Estrutura Visual

Organize o widget conforme o layout existente:

```
┌─────────────────────────────────────────────────────────────┐
│  Trade                                              [BTN_X]  │
├─────────────────────────────────────────────────────────────┤
│  MEUS ITENS          │  ITENS DO PARCEIRO                   │
│  [Slot1][Slot2]...   │  [Slot1][Slot2]...                    │
│  Slots: X/10         │  Slots: X/10                          │
├─────────────────────────────────────────────────────────────┤
│  MEU GOLD: [TB_MyGold]     PARCEIRO GOLD: [TB_PartnerGold]  │
├─────────────────────────────────────────────────────────────┤
│  [Accept Trade]              [Decline Trade]                │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Nomes dos Elementos

| Elemento | Nome sugerido | Tipo |
|----------|---------------|------|
| Botão fechar | BTN_Close | Button |
| 10 slots meus | MySlot_1 a MySlot_10 (ou em uma Uniform Grid) | Border/Image |
| 10 slots parceiro | PartnerSlot_1 a PartnerSlot_10 | Border/Image |
| Input gold | TB_MyGold | Editable Text Box |
| Gold parceiro | TB_PartnerGold | Text Block |
| Accept | BTN_Accept | Button |
| Decline | BTN_Decline | Button |
| Contador | Txt_MySlots | Text Block ("Slots: 0/10") |

### 3.3 Preencher Arrays MySlots e PartnerSlots

No **Event Construct**, adicione aos arrays (na ordem 1–10):

- `MySlots.Add(MySlot_1)` ... `MySlots.Add(MySlot_10)`
- `PartnerSlots.Add(PartnerSlot_1)` ... `PartnerSlots.Add(PartnerSlot_10)`

Ou use **Get All Widgets of Class** se os slots forem um tipo específico.

---

## Passo 4: Função UpdateTradeUI (Struct → Preencher UI)

> **Guia detalhado com drag & drop:** Ver `GUIA_UPDATE_TRADE_UI_DRAG_DROP.md` para implementação completa usando Grid_TradeSlot1, Grid_TradeSlot2 e o sistema de drag existente (inventário/armazém).

### 4.1 Estrutura FUmbraTradeState (passada pelo delegate)

O C++ parseia o JSON e passa o struct. O Blueprint recebe:

| Campo | Tipo | Descrição |
|-------|------|-----------|
| MyPlayerID | int | ID do jogador local |
| Player1ID | int | ID do jogador 1 |
| Player2ID | int | ID do jogador 2 |
| Player1Items | Array of FUmbraTradeItem | Itens do jogador 1 |
| Player2Items | Array of FUmbraTradeItem | Itens do jogador 2 |
| Player1GoldOffer | int | Gold oferecido por player 1 |
| Player2GoldOffer | int | Gold oferecido por player 2 |
| Status | String | "active", "completed", "cancelled" |

**FUmbraTradeItem:** TradeItemID, InventoryID, Quantity, ItemName, IconPath, ItemType, Rarity

### 4.2 Lógica da UpdateTradeUI

1. Se `TradeState.Status == "completed"` ou `"cancelled"`: parar timer e fechar widget
2. Determinar "meus itens" e "meu gold":
   - Se `TradeState.MyPlayerID == TradeState.Player1ID`: Meus = Player1Items, MeuGold = Player1GoldOffer; Parceiro = Player2Items, ParceiroGold = Player2GoldOffer
   - Caso contrário: Meus = Player2Items, MeuGold = Player2GoldOffer; Parceiro = Player1Items, ParceiroGold = Player1GoldOffer
3. Limpar todos os slots (esconder ícones/textos)
4. Para cada item em "meus itens": mostrar ícone no slot correspondente (guardar `TradeItemID` para remover)
5. Para cada item em "parceiro itens": mostrar ícone no slot correspondente (só leitura)
6. Setar `TB_MyGold` = meu gold offer
7. Setar `TB_PartnerGold` = gold do parceiro
8. Atualizar contador "Slots: X/10"

### 4.3 Exemplo Blueprint (conceitual)

- **Break FUmbraTradeState** → MyPlayerID, Player1ID, Player2ID, Player1Items, Player2Items, etc.
- **Equal (MyPlayerID, Player1ID)** → Branch
  - True: MeusItems = Player1Items, MeuGold = Player1GoldOffer, ParceiroItems = Player2Items, ParceiroGold = Player2GoldOffer
  - False: MeusItems = Player2Items, MeuGold = Player2GoldOffer, ParceiroItems = Player1Items, ParceiroGold = Player1GoldOffer
- **For Each** em MeusItems → preencher slot (guardar TradeItemID para RemoveTradeItem)
- **For Each** em ParceiroItems → preencher slot parceiro

---

## Passo 5: Botões Accept, Decline e Close

### 5.1 BTN_Accept – OnClicked

```
[BTN_Accept OnClicked]
    │
    └──► Set Trade Ready
              Target = Get Game Instance → Cast to UmbraGameInstance
              Trade Session ID = TradeSessionID
              Ready = true
```

**Nota:** Quando ambos clicam Accept, a API executa a troca e retorna `trade_completed: true`. O GameInstance dispara `OnTradeCompleted`. O HUD deve fazer bind de `OnTradeCompleted` para fechar o WBP_Trade (Passo 7).

### 5.2 BTN_Decline – OnClicked

```
[BTN_Decline OnClicked]
    │
    ├──► Cancel Trade
    │         Target = Get Game Instance → Cast to UmbraGameInstance
    │
    └──► Remove from Parent
```

### 5.3 BTN_Close – OnClicked

Mesmo fluxo do Decline:

```
[BTN_Close OnClicked]
    │
    ├──► Cancel Trade
    │         Target = Get Game Instance → Cast to UmbraGameInstance
    │
    └──► Remove from Parent
```

---

## Passo 6: Adicionar e Remover Itens

### 6.1 Adicionar Item (do inventário)

**Opção A – Clique no inventário:**  
Quando o jogador clicar em um item do inventário enquanto o WBP_Trade está aberto:

1. Obter `inventory_id` do item clicado
2. **Add Trade Item**
   - Target = Get Game Instance → Cast to UmbraGameInstance
   - Trade Session ID = TradeSessionID
   - Inventory ID = inventory_id
   - Quantity = 1
3. O próximo `LoadTradeState` (polling) atualizará a UI

**Opção B – Drag & Drop:**  
Se o inventário suportar drag, configure o WBP_Trade como zona de drop. No evento **OnDrop**:

1. Obter `inventory_id` do payload
2. Chamar **Add Trade Item** como acima

### 6.2 Remover Item (clique no slot)

Quando o jogador clicar em um slot que contém um item **seu**:

1. Obter `trade_item_id` guardado no slot
2. **Remove Trade Item**
   - Target = Get Game Instance → Cast to UmbraGameInstance
   - Trade Session ID = TradeSessionID
   - Trade Item ID = trade_item_id
3. O polling atualizará a UI

### 6.3 Campo de Gold

No **TB_MyGold** – evento **OnTextCommitted** ou **OnTextChanged** (com debounce):

1. **To Integer** do texto
2. Se valor >= 0:
   - **Set Trade Gold**
     - Target = Get Game Instance → Cast to UmbraGameInstance
     - Trade Session ID = TradeSessionID
     - Gold Amount = valor

---

## Passo 7: Bind OnTradeCompleted e OnTradeCancelled

### 7.1 No HUD (Event Construct)

1. **Bind Event to On Trade Completed**  
   - Target = MyGameInstance  
   - Event = Custom Event `OnTradeCompleted_Custom`

2. **Bind Event to On Trade Cancelled**  
   - Target = MyGameInstance  
   - Event = Custom Event `OnTradeCancelled_Custom` (ou outro que feche o trade)

### 7.2 Custom Event OnTradeCompleted_Custom

```
[OnTradeCompleted_Custom]
    │
    └──► Encontrar WBP_Trade no viewport (ou ter referência guardada)
              │
              └──► Remove from Parent
```

**Alternativa:** O WBP_Trade pode fazer bind de `OnTradeCompleted` no próprio **Event Construct** e chamar **Remove from Parent** em si mesmo. Assim, quando a troca for concluída, o widget se remove.

### 7.3 OnTradeCancelled

O `CancelTrade` do GameInstance já chama a API e dispara `OnTradeCancelled`. O WBP_Trade deve:

- Fazer bind de `OnTradeCancelled` no Event Construct
- No Custom Event: **Remove from Parent**

Assim, ao clicar Decline/X, o widget fecha e o estado é limpo.

---

## Passo 8: Parar o Timer ao Fechar

No WBP_Trade, ao ser removido (**Remove from Parent** ou **On Trade Cancelled**):

```
[Event: Destruct ou Remove from Parent]
    │
    └──► Clear Timer by Handle
              Handle = PollingTimerHandle
```

Ou chame **Clear Timer** na função que trata Decline/Close, antes de **Remove from Parent**.

---

## Passo 9: Resumo do Fluxo Completo

```
1. Jogador B aceita solicitação (WBP_TradeRequested)
   → AcceptTradeRequest(FromPlayerID)
   → API accept_trade_request.php
   → OnTradeStarted(TradeSessionID)

2. HUD recebe OnTradeStarted
   → Create WBP_Trade
   → InitializeTrade(TradeSessionID)
   → Add to Viewport
   → LoadTradeState (primeira vez)
   → Inicia timer de polling (2s)

3. LoadTradeState retorna
   → OnTradeStateLoaded(Json)
   → UpdateTradeUI (preenche slots, gold)

4. Jogador adiciona itens
   → AddTradeItem (clique no inventário)
   → Polling atualiza UI

5. Jogador define gold
   → TB_MyGold OnTextCommitted
   → SetTradeGold

6. Jogador clica Accept Trade
   → SetTradeReady(true)
   → Se ambos ready: API executa troca
   → OnTradeCompleted
   → Widget Remove from Parent

7. Jogador clica Decline ou X
   → CancelTrade
   → API cancel_trade.php
   → OnTradeCancelled
   → Widget Remove from Parent
   → Clear Timer
```

---

## Checklist Final

| Item | Onde | Status |
|------|------|--------|
| HUD: Bind OnTradeStarted | Event Construct | ⬜ |
| HUD: OnTradeStarted → Create WBP_Trade + InitializeTrade + AddToViewport | Custom Event | ⬜ |
| WBP_Trade: InitializeTrade(TradeSessionID) | Função | ⬜ |
| WBP_Trade: Bind OnTradeStateLoaded | Event Construct | ⬜ |
| WBP_Trade: OnTradeStateLoaded → UpdateTradeUI | Custom Event | ⬜ |
| WBP_Trade: Timer polling 2s → LoadTradeState | Set Timer by Event | ⬜ |
| WBP_Trade: BTN_Accept → SetTradeReady(true) | OnClicked | ⬜ |
| WBP_Trade: BTN_Decline → CancelTrade + RemoveFromParent | OnClicked | ⬜ |
| WBP_Trade: BTN_Close → CancelTrade + RemoveFromParent | OnClicked | ⬜ |
| WBP_Trade: Slots meus clicáveis → RemoveTradeItem | OnClicked (por slot) | ⬜ |
| Inventário: Clique em item com WBP_Trade aberto → AddTradeItem | Integração | ⬜ |
| TB_MyGold → SetTradeGold | OnTextCommitted | ⬜ |
| WBP_Trade: Bind OnTradeCompleted → RemoveFromParent | Event Construct | ⬜ |
| WBP_Trade: Bind OnTradeCancelled → RemoveFromParent + ClearTimer | Event Construct | ⬜ |

---

## Troubleshooting

**Widget não abre ao aceitar:**
- Verifique bind de OnTradeStarted no HUD
- Confirme que o Custom Event recebe o TradeSessionID
- Veja no Output Log: `✅ Troca aceita - TradeStartedNotify enviado`

**Slots não atualizam:**
- Verifique bind de OnTradeStateLoaded
- Confirme que LoadTradeState é chamado (polling)
- Teste get_trade_state.php manualmente (test_trade.html ou Postman)

**Accept não conclui:**
- Os dois jogadores precisam clicar Accept
- Verifique player1_ready e player2_ready no JSON

**Gold não aceito:**
- Execute add_trade_gold_columns.sql se ainda não executou
- Verifique se my_gold >= valor digitado
