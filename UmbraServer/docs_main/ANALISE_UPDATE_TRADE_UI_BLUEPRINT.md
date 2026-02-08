# Análise do Blueprint UpdateTradeUI (WBP_Trade)

Este documento analisa o código Blueprint da função **UpdateTradeUI** do WBP_Trade, identifica problemas e sugere correções.

---

## 1. Descrição do fluxo atual

### 1.1 Entrada e estrutura

A função `UpdateTradeUI` recebe:
- **TradeState** (FUmbraTradeState) — saída do parâmetro

Variáveis locais:
- **MyItems** (Array of FUmbraTradeItem)
- **MyGoldOffer** (int)
- **PartnerItems** (Array of FUmbraTradeItem)
- **PartnerGoldOffer** (int)

### 1.2 Sequência de execução

1. **Break TradeState** → extrai todos os campos da estrutura
2. **Verificação de Status** (OR)
   - `Status == "completed"` OU `Status == "cancelled"`
   - **True** → Clear Timer → RemoveFromParent → Return
   - **False** → continua
3. **Branch MyPlayerID == Player1ID**
   - **True** (eu sou Player1): MyItems=Player1Items, MyGoldOffer=Player1GoldOffer, PartnerItems=Player2Items, PartnerGoldOffer=Player2GoldOffer
   - **False** (eu sou Player2): MyItems=Player2Items, MyGoldOffer=Player2GoldOffer, PartnerItems=Player1Items, PartnerGoldOffer=Player1GoldOffer
4. **For Each MySlotWidgets** → Loop Body: ClearSlot
5. **ClearSlot then** → **For Each MyItems**
6. **For Each MyItems** → Loop Body:
   - CreateInventorySlotFromTradeItem(TradeItem, Array Index)
   - Set SlotData (Target = ???, valor = Return Value)
   - Set TradeItemID (Target = ???, valor = TradeItemID)
   - Get MySlotWidgets[Array Index] → Cast to WBP_InventorySlot → UpdateSlotVisual

---

## 2. Problemas identificados

### 2.1 BUG CRÍTICO: Target incorreto do Set SlotData e Set TradeItemID

**Estado atual:** O target dos nós `Set SlotData` e `Set TradeItemID` vem de `K2Node_Knot_18` → `K2Node_Knot_19` → **Array Element do For Each MySlotWidgets**.

**Problema:** O `For Each MyItems` é executado após **cada** `ClearSlot` (o "then" de ClearSlot está ligado ao Exec do For Each MyItems). Assim:

- Na 1ª iteração do For Each MySlotWidgets: Array Element = slot 0 → ClearSlot → For Each MyItems roda inteiro
- Durante o For Each MyItems, o Array Element do primeiro loop continua sendo o slot 0
- **Todos** os itens de MyItems são atribuídos ao **mesmo slot** (slot 0)
- Sobrescrita: só o último item permanece visível

**Correção:** O target deve vir de **Get MySlotWidgets[Array Index]**, onde `Array Index` é do **For Each MyItems**, não do For Each MySlotWidgets.

- Use a saída de **Get (a copy)** ou **Array Get** (`MySlotWidgets`, `Array Index` do For Each MyItems)
- Conecte essa saída ao **Target** de `Set SlotData`, `Set TradeItemID` e ao **Object** do Cast que alimenta `UpdateSlotVisual`

---

### 2.2 BUG: Ordem de execução dos loops

**Estado atual:** O "then" de `ClearSlot` liga ao `Exec` do `For Each MyItems`. Isso faz o For Each MyItems rodar **uma vez por slot** do For Each MySlotWidgets (após cada ClearSlot).

**Correção:** O For Each MyItems deve rodar **apenas uma vez**, depois de **todos** os slots terem sido limpos.

- Use o pin **Completed** do For Each MySlotWidgets (não o Loop Body)
- Conecte: **For Each MySlotWidgets → Completed** → **For Each MyItems → Exec**

Ou então mantenha apenas um For Each MySlotWidgets para ClearSlot, e depois um For Each MyItems independente, sequencial.

---

### 2.3 FALTA: Loop para PartnerItems

**Estado atual:** Existe loop para `MyItems` → `MySlotWidgets`, mas **não existe** loop para `PartnerItems` → `PartnerSlotWidgets`.

**Correção:** Adicionar **For Each PartnerItems** com:

- CreateInventorySlotFromTradeItem(TradeItem, Array Index)
- Get PartnerSlotWidgets[Array Index] → Slot
- Set SlotData (Target = Slot)
- Cast to WBP_InventorySlot → UpdateSlotVisual
- (Não precisa Set TradeItemID para slots do parceiro)

---

### 2.4 FALTA: Clear nos slots do parceiro

**Estado atual:** Só há `For Each MySlotWidgets → ClearSlot`. Não há `For Each PartnerSlotWidgets → ClearSlot`.

**Correção:** Adicionar **For Each PartnerSlotWidgets → ClearSlot** antes de popular os slots (ou na mesma sequência, conforme o guia).

---

### 2.5 Set SlotData (variável) vs SetSlotData (função)

**Estado atual:** Usa `K2Node_VariableSet` para a propriedade `SlotData` (Set SlotData em variável).

**Guia:** Recomenda usar a função `SetSlotData`.

**Problema com a função:** Na C++, `SetSlotData()` faz:

```cpp
if (NewSlotData.InventoryID <= 0 || NewSlotData.ItemTemplateID <= 0 || NewSlotData.Quantity <= 0)
{
    ClearSlot();
    return;
}
```

`CreateInventorySlotFromTradeItem` define `ItemTemplateID = 0`, então a chamada à função `SetSlotData` faria `ClearSlot` e retornaria, e o item não seria exibido.

**Correções possíveis:**

1. **C++ (preferível):** Incluir `item_template_id` na API de trade e em `FUmbraTradeItem`; ajustar `CreateInventorySlotFromTradeItem` para preencher `ItemTemplateID` corretamente; e usar a função `SetSlotData` no Blueprint.
2. **Manter variável:** Continuar usando Set SlotData (variável) contorna o problema, mas ignora a lógica adicional de `SetSlotData` (EnsureStatsSerialization, etc.).

---

### 2.6 Erro no nó Update Slot Visual

O nó `K2Node_CallFunction_8` (Update Slot Visual) exibe:

```
ErrorType=1
ErrorMsg="Este Blueprint (próprio) não é WBP_InventorySlot_C, por isso \" Target \" deve ter uma conexão."
```

O Target está ligado à saída "As WBP Inventory Slot" do `K2Node_DynamicCast_0`. O erro pode ser antigo ou de contexto. Verifique no editor que o Target está conectado corretamente e que o Cast está sendo feito a partir do slot correto (Get MySlotWidgets[Array Index]).

---

## 3. Resumo: o que ajustar

| # | Item | Prioridade | Ação |
|---|------|------------|------|
| 1 | Target de Set SlotData e Set TradeItemID | CRÍTICA | Usar Get MySlotWidgets[Array Index] do For Each MyItems, não Array Element do For Each MySlotWidgets |
| 2 | Ordem dos loops | CRÍTICA | For Each MyItems deve rodar **após** o For Each MySlotWidgets terminar (pin Completed) |
| 3 | Loop PartnerItems | ALTA | Adicionar For Each PartnerItems → preencher PartnerSlotWidgets |
| 4 | Clear PartnerSlotWidgets | ALTA | Adicionar For Each PartnerSlotWidgets → ClearSlot |
| 5 | SetSlotData vs Set SlotData | MÉDIA | Corrigir ItemTemplateID no C++ e API e usar `SetSlotData` (função) |
| 6 | Erro Update Slot Visual | BAIXA | Conferir conexões no editor |

---

## 4. Fluxo correto sugerido

```
1. Break TradeState
2. Branch Status (completed/cancelled) → True: Clear Timer, RemoveFromParent, Return
3. Branch MyPlayerID == Player1ID → definir MyItems, MyGoldOffer, PartnerItems, PartnerGoldOffer
4. For Each MySlotWidgets → Loop Body: ClearSlot
5. For Each MySlotWidgets → Completed → For Each PartnerSlotWidgets → Loop Body: ClearSlot
6. For Each PartnerSlotWidgets → Completed → For Each MyItems → Loop Body:
   - CreateInventorySlotFromTradeItem(Array Element, Array Index)
   - Get MySlotWidgets[Array Index] → Slot
   - Set SlotData (Target = Slot, valor = Return Value) ou SetSlotData (função)
   - Set TradeItemID (Target = Slot)
   - Cast to WBP_InventorySlot (Object = Slot)
   - Update Slot Visual (Target = As WBP Inventory Slot)
7. For Each MyItems → Completed → For Each PartnerItems → Loop Body:
   - CreateInventorySlotFromTradeItem(Array Element, Array Index)
   - Get PartnerSlotWidgets[Array Index] → Slot
   - Set SlotData (Target = Slot)
   - Cast to WBP_InventorySlot → Update Slot Visual
8. Set Text (contador e gold)
```

---

## 5. Correção do ItemTemplateID (C++/API)

Para poder usar `SetSlotData` (função) no Blueprint:

1. **API PHP** (`get_trade_state.php`): incluir `item_template_id` no array de cada item:

```php
$item = [
    'trade_item_id' => (int)$it['trade_item_id'],
    'inventory_id' => (int)$it['inventory_id'],
    'quantity' => (int)$it['quantity'],
    'item_template_id' => (int)$it['item_template_id'],  // ADICIONAR
    'item_name' => $it['item_name'],
    ...
];
```

2. **FUmbraTradeItem** (`UmbraDataStructures.h`): adicionar campo:

```cpp
UPROPERTY(BlueprintReadWrite, Category = "Trade")
int32 ItemTemplateID = 0;
```

3. **ParseTradeItems** (`UmbraGameInstance.cpp`): ler o campo:

```cpp
Item.ItemTemplateID = ItemObj->GetIntegerField(TEXT("item_template_id"));
```

4. **CreateInventorySlotFromTradeItem**: usar o valor:

```cpp
Slot.ItemTemplateID = TradeItem.ItemTemplateID;
```
