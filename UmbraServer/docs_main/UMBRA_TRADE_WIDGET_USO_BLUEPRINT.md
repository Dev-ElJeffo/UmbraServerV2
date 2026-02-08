# UUmbraTradeWidget – Uso no Blueprint

Classe C++ que centraliza a lógica de UpdateTradeUI. Duas opções:

---

## Opção A: WBP_Trade como UserWidget (recomendado – evita conflitos)

**Mantenha o parent como User Widget.** Use a função estática do GameInstance.

### 1. Variáveis no WBP_Trade (Blueprint)

| Variável | Tipo | Descrição |
|----------|------|------------|
| MySlotWidgets | Array of UmbraInventorySlotWidget | Slots do Grid_TradeSlot1 |
| PartnerSlotWidgets | Array of UmbraInventorySlotWidget | Slots do Grid_TradeSlot2 |
| PollingTimerHandle | Timer Handle | Preenchido pelo timer de polling |

### 2. Bind de OnTradeStateLoaded

No **Event Construct**:
1. **Get Game Instance** → **Cast to UmbraGameInstance** → **Bind Event to On Trade State Loaded**.
2. O Event recebe **TradeState** (FUmbraTradeState).
3. Chame **Update Trade UI From Arrays** (Target = **Game Instance**):
   - World Context Object = **self**
   - Trade State = parâmetro do Event
   - My Slot Widgets = **MySlotWidgets**
   - Partner Slot Widgets = **PartnerSlotWidgets**

### 3. Limpar timer e fechar

No bind de **On Trade Completed** e **On Trade Cancelled** (no HUD ou no próprio WBP_Trade):
- Clear Timer by Handle (PollingTimerHandle)
- Remove from Parent

---

## Opção B: WBP_Trade herdando de Umbra Trade Widget

1. **Class Settings** → **Parent Class** → **Umbra Trade Widget**.
2. Variáveis **MySlotWidgets** e **PartnerSlotWidgets** já existem na classe C++.
3. No bind de OnTradeStateLoaded: **Update Trade UI From State** (Target = self).

---

## Variáveis (Opção B – já existem na classe C++)

| Variável | Tipo | Descrição |
|----------|------|-----------|
| MySlotWidgets | Array of UmbraInventorySlotWidget | Slots do Grid_TradeSlot1 |
| PartnerSlotWidgets | Array of UmbraInventorySlotWidget | Slots do Grid_TradeSlot2 |
| MyGameInstance | UmbraGameInstance | (Opcional) null = pega do World |
| PollingTimerHandle | Timer Handle | Preenchido pelo timer de polling |

---

## 3. CreateTradeSlots (manter no Blueprint)

Mantenha a função que cria os slots e preenche **MySlotWidgets** e **PartnerSlotWidgets**:

- Create Widget (WBP_InventorySlot) para cada slot.
- Set Slot Index, Set Parent Trade Widget = self nos meus slots.
- Add Child to Grid.
- MySlotWidgets.Add(widget) / PartnerSlotWidgets.Add(widget).

---

## 4. Bind de OnTradeStateLoaded

No **Event Construct**:

1. **Get Game Instance** → **Cast to UmbraGameInstance** → **Bind Event to On Trade State Loaded**.
2. O Event recebe **TradeState** (FUmbraTradeState).
3. Faça um único nó: **Update Trade UI From State** (Target = **self**, Trade State = parâmetro do Event).

**Fluxo:**
```
Event Construct
  → CreateTradeSlots (preenche MySlotWidgets, PartnerSlotWidgets)
  → Get Game Instance → Cast to UmbraGameInstance
  → Bind Event to On Trade State Loaded
       → Event: TradeState recebido
       → Update Trade UI From State (self, TradeState)
```

---

## 5. Polling e fechamento

No início do trade (ex.: ao abrir o widget):

1. **Set Timer by Event** (ou Set Timer by Function Name) para chamar **Load Trade State** a cada ~1,5 s.
2. Guarde o retorno em **PollingTimerHandle** (variável da classe).

Quando o trade termina (completed/cancelled), **UpdateTradeUIFromState** já:
- Limpa o timer.
- Remove o widget de cena.

---

## Resumo

| Antes | Depois |
|-------|--------|
| Função UpdateTradeUI com dezenas de nós | Uma chamada a **Update Trade UI From State** |
| Branches, ForEach, Set SlotData, etc. no Blueprint | Tudo em C++ |
| Erros de target, ordem de loops | Implementação centralizada |

---

## 6. WBP_InventorySlot – RefreshSlotDisplay (obrigatório para trade)

Para os slots de trade atualizarem o visual, o **WBP_InventorySlot** precisa implementar o evento **Refresh Slot Display** (herdado de `UUmbraInventorySlotWidget`).

1. Abra **WBP_InventorySlot**.
2. Em **Functions**, clique em **Override** → **Refresh Slot Display**.
3. Dentro do evento, chame **Update Slot Visual** (a função local que já existe).

Assim o C++ consegue disparar a atualização visual sem conflitar com a função `UpdateSlotVisual` já usada por inventário/storage.

### 6.1 Update Slot Visual – ícone para trade

O C++ agora preenche `SlotData.ItemTemplate.ItemIcon` quando há `item_template_id` na API. A função **Update Slot Visual** deve usar:

- **Branch:** `InventoryID > 0` (caso TRUE = slot com item)
- **Ícone:** `SlotData.ItemTemplate.ItemIcon` OU `Get Item Template` → `Item Icon` — se for null, usar `Get Item Icon By ID`(ItemTemplateID) do GameInstance
- **Quantidade:** `SlotData.Quantity` → Set Text no contador

Se o slot usa **Set Brush from Texture** (ou similar) para o ícone, garanta que usa `ItemTemplate.ItemIcon` quando disponível. O C++ Define ItemIcon para itens de trade via `GetItemIconByID`.

---

## Checklist

- [ x] WBP_InventorySlot implementa **Refresh Slot Display** → chama **Update Slot Visual**
- [x ] WBP_Trade herda de **Umbra Trade Widget**
- [x ] CreateTradeSlots preenche **MySlotWidgets** e **PartnerSlotWidgets**
- [x ] Set Parent Trade Widget = self nos meus slots
- [x ] Bind On Trade State Loaded → **Update Trade UI From State**
- [ x] Polling chama Load Trade State e guarda handle em **PollingTimerHandle**
