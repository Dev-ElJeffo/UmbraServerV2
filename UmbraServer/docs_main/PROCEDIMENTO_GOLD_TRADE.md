# Procedimento: Gold no Trade (Add Gold e Remove Gold)

Implementação simplificada, seguindo o modelo **DepositGold/WithdrawGold** do storage: a lógica fica em C++, o Blueprint só chama `AddTradeGold(Amount)` ou `RemoveTradeGold(Amount)` com o valor do TextBox.

---

## Visão geral

| Storage (Deposit/Withdraw) | Trade (Add/Remove Gold) |
|----------------------------|--------------------------|
| DepositGold(Amount) | AddTradeGold(Amount) |
| WithdrawGold(Amount) | RemoveTradeGold(Amount) |
| Blueprint: Get Text → To Integer → DepositGold(Amount) | Blueprint: Get Text → To Integer → AddTradeGold(Amount) |
| Mesmo padrão | Mesmo padrão |

**C++ faz tudo:** validação, CurrentOffer, MaxGold, clamp, chamada à API. O Blueprint só converte o texto e chama a função.

---

## Parte 1: Funções C++ (já implementadas)

### AddTradeGold(int32 Amount)
- Usa `CurrentTradeState` (cache do `LoadTradeState`).
- Calcula oferta atual (Player1GoldOffer ou Player2GoldOffer conforme MyPlayerID).
- Limite: `Amount` não pode exceder `MyGold - CurrentOffer`.
- Chama `SetTradeGold(TradeSessionID, NewOffer)` internamente.

### RemoveTradeGold(int32 Amount)
- Usa `CurrentTradeState`.
- Calcula oferta atual.
- Limite: `Amount` não pode exceder `CurrentOffer`.
- Chama `SetTradeGold(TradeSessionID, NewOffer)` internamente.

**Requisito:** `LoadTradeState` deve ser chamado antes (o polling do trade já faz isso).

---

## Parte 2: Widgets no Blueprint

Crie **dois widgets** com o mesmo design do GoldDepositManager/GoldWithdrawManager, ou **reutilize** os existentes se fizer sentido no contexto de trade.

### Opção A: Criar WBP_TradeAddGold e WBP_TradeRemoveGold (recomendado)

Cada um tem: **Amount**, **TextBox_GoldAmount**, **BTN_Confirm**, **BTN_Cancel**.

#### WBP_TradeAddGold – BTN_Confirm (On Clicked)

```
[On Clicked (BTN_Confirm)]
    │
    ├─► Get Text (TextBox_GoldAmount)
    ├─► To Integer
    ├─► Get Game Instance → Cast to UmbraGameInstance
    ├─► Add Trade Gold (Amount)
    └─► Remove from Parent
```

#### WBP_TradeAddGold – BTN_Cancel (On Clicked)

```
[On Clicked (BTN_Cancel)]
    └─► Remove from Parent
```

#### WBP_TradeRemoveGold – BTN_Confirm (On Clicked)

```
[On Clicked (BTN_Confirm)]
    │
    ├─► Get Text (TextBox_GoldAmount)
    ├─► To Integer
    ├─► Get Game Instance → Cast to UmbraGameInstance
    ├─► Remove Trade Gold (Amount)
    └─► Remove from Parent
```

#### WBP_TradeRemoveGold – BTN_Cancel (On Clicked)

```
[On Clicked (BTN_Cancel)]
    └─► Remove from Parent
```

### Opção B: Um único modal WBP_TradeGoldAmount

Se preferir um único widget, crie **WBP_TradeGoldAmount** com um parâmetro de inicialização:

- **Variável:** `bIsAdd` (bool) – true = Add, false = Remove.
- **BTN_Confirm:**
  - Get Text → To Integer → Amount
  - Branch `bIsAdd` → TRUE: Add Trade Gold(Amount) | FALSE: Remove Trade Gold(Amount)
  - Remove from Parent

No WBP_Trade, ao criar o widget:
- Add Gold: Create Widget, Set bIsAdd = true, Add to Viewport
- Remove Gold: Create Widget, Set bIsAdd = false, Add to Viewport

---

## Parte 3: Integrar no WBP_Trade

### BTN_AddGold (On Clicked)

```
[On Clicked (BTN_AddGold)]
    │
    ├─► Get Current Trade Session ID (Game Instance)
    ├─► Branch (TradeSessionID > 0)
    │   └─► TRUE:
    │       ├─► Create Widget (WBP_TradeAddGold)  [ou WBP_TradeGoldAmount com bIsAdd=true]
    │       └─► Add to Viewport
```

### BTN_RemoveGold (On Clicked)

```
[On Clicked (BTN_RemoveGold)]
    │
    └─► (Idêntico, mas Create Widget WBP_TradeRemoveGold)
```

Nenhum Bind, Event Dispatcher, variáveis CurrentGoldOffer, GoldModalMode etc. Apenas Create Widget e Add to Viewport.

---

## Parte 4: Atualizar TB_MyGold e TB_PartnerGold

No **On Trade State Loaded** (bind de `OnTradeStateLoaded`), após `Update Trade UI From State`:

1. **Break** o TradeState.
2. **Branch** MyPlayerID == Player1ID.
3. **Set** MyGoldOffer = Player1GoldOffer (ou Player2GoldOffer).
4. **Set** PartnerGoldOffer = Player2GoldOffer (ou Player1GoldOffer).
5. **Set Text** TB_MyGold = ToString(MyGoldOffer).
6. **Set Text** TB_PartnerGold = ToString(PartnerGoldOffer).

O polling continua atualizando o estado; na próxima `LoadTradeState` o texto de gold será atualizado.

---

## Comparação: antes vs depois

| Antes (procedimento antigo) | Depois (simplificado) |
|----------------------------|------------------------|
| WBP_TradeGoldAmount com Mode, CurrentOffer, MaxGold | Só TextBox + Confirm + Cancel |
| Variáveis no modal e no WBP_Trade | Sem variáveis extras |
| Event Dispatcher + Bind + OnTradeGoldAmountConfirmed | Chamada direta AddTradeGold/RemoveTradeGold |
| Validação, clamp, cálculo no Blueprint | Tudo em C++ |
| ~15+ passos no Blueprint | 3–4 nodos por botão |

---

## Checklist final

| Item | Verificação |
|------|-------------|
| AddTradeGold e RemoveTradeGold no UmbraGameInstance (C++) | ✅ |
| WBP_TradeAddGold: Get Text → To Integer → Add Trade Gold → Remove from Parent | ⬜ |
| WBP_TradeRemoveGold: Get Text → To Integer → Remove Trade Gold → Remove from Parent | ⬜ |
| BTN_AddGold: Create Widget → Add to Viewport | ⬜ |
| BTN_RemoveGold: Create Widget → Add to Viewport | ⬜ |
| TB_MyGold e TB_PartnerGold atualizados no OnTradeStateLoaded | ⬜ |
| Colunas player1_gold_offer e player2_gold_offer no banco | ⬜ |
