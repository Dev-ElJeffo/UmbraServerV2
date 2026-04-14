# Refinamento — loja pessoal: slot com ícone + preço, vendedor e inventário (UE 5.6.1)

Este guia descreve o layout **recomendado** (preço por baixo do ícone no mesmo slot) e o fluxo do vendedor (**WebSocket 65**, sem polling de 4 s). Código: `UmbraStoreSlotWidget`, `UmbraPlayerStoreWidget.cpp`, `UmbraPersonalShopBuyerWidget.cpp`, `UmbraGameInstance.cpp`, `MovementProtocol.hpp`.

---

## 1. Layout recomendado: uma grelha 2×5 (ícone + preço no `WBP_StoreSlot`)

### 1.1 Comportamento (C++)

- **`Grid_StoreSlots`** (`Uniform Grid Panel`): **10** widgets `StoreSlotWidgetClass` (ex. `WBP_StoreSlot` com parent **`UmbraStoreSlotWidget`**).
- Posição: **row = índice / 5**, **column = índice % 5** → duas filas de cinco (0–4 em cima, 5–9 em baixo).
- O preço é mostrado **dentro** de cada slot, num **`TextBlock`** com nome (por ordem de procura): **`Text_SlotPrice`**, **`TXT_SlotPrice`**, ou **`Text_Slot_Price`**.
- O C++ usa `SetSlotPriceGoldDisplay` em `UmbraStoreSlotWidget` quando o preço muda (equivalente ao antigo `Text_Price_N` no painel).

### 1.2 O que fazer no `WBP_StoreSlot` (Designer)

1. **Hierarchy** sugerida: `Vertical Box` (ou `Border`) → área do ícone / slot do item → **`Text`** por baixo com **Variable Name** = **`Text_SlotPrice`**.
2. **Parent Class** = **`UmbraStoreSlotWidget`** (drop da loja + preço integrado).
3. **Compile**.

Não é necessário alinhar duas grelhas (`Grid_StoreSlots` + `Grid_SlotPrices`) nem ajustar padding entre grelhas separadas.

### 1.3 Painel da loja (`WBP_Personal_Player_Store` / comprador)

- Vendedor: **Parent Class** = **`UmbraPlayerStoreWidget`**.
- Hierarquia mínima: **`Grid_StoreSlots`** (Uniform Grid). Podes **apagar** do Designer `Grid_SlotPrices` e os `Text_Price_0`…`9` — se não existirem `Text_Price_*`, o C++ **colapsa** `Grid_SlotPrices` quando esse painel existir mas sem textos legados.

### 1.4 Modo legado (opcional)

Se mantiveres **`Text_Price_0`…`9`** num segundo `Uniform Grid`, o C++ ainda actualiza esses textos **em paralelo** com `Text_SlotPrice` (útil para migração). O layout antigo em quatro linhas (itens / preços / itens / preços) já **não** é o da grelha de slots.

---

## 2. Vendedor: vendas, ouro, inventário (estado actual)

### 2.1 Push WebSocket (sem polling de 4 s)

- Após compra HTTP bem-sucedida, o cliente envia **tipo 64**; a Zone faz **broadcast tipo 65** (`PersonalShopListingsChanged`).
- No cliente, **vendedor** (`ActivePlayerID == seller`): encadeia **`LoadGold` → `LoadInventory`** (ouro actualizado antes de refrescar slots) e **`LoadPersonalShopBySeller`** se a loja local estiver aberta.
- **`SyncVendorSlotsFromServerState`** continua a detectar listagem removida (venda), mensagem em **`TXT_StoreInfo`**, **`SessionGoldFromShopSales`**, e pode chamar **`LoadInventory()`** nessa detecção.

### 2.2 Texto “ouro desta sessão”

1. No WBP da loja do vendedor, um **Text** (ex.: “Ouro desta sessão: …”).
2. **Variable Name** no Details: **`TXT_ShopSessionGold`** ou **`Text_ShopSessionGold`**.
3. O C++ preenche em `UpdateVendorSessionGoldLabel`.
4. Ao **fechar** a loja, o contador de sessão é **zerado**.

### 2.3 Inventário e texto de ouro (`Text_Gold_Info`)

- O `UmbraGameInstance` dispara **`OnGoldLoaded`** após `get_gold.php` (parse robusto int/string) e **também** após **`OnInventoryLoaded`** para realinhar a UI.
- O widget base **`UmbraInventoryWidget`** escuta **`OnGoldLoaded`** e **`OnInventoryLoaded`** e actualiza **`Text_Gold_Info`** (`UTextBlock` ou `URichTextBlock`).
- Se o **Blueprint do inventário** no `Event OnInventoryLoaded` escrever “0” ou limpar o ouro, o segundo broadcast / delegate corrige; confirma que o nome do widget é **`Text_Gold_Info`**.

---

## 3. Referências cruzadas

- Comprador: [GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md](GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md)
- Nameplate: [GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md](GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md)

---

## 4. Nota sobre `Spacer`

Não há `Spacer` em `UmbraPersonalShopBuyerWidget.cpp`; o modo lista só remove **`HorizontalBox`** dinâmicos dentro de `VBox_Listings`, preservando a grade do Designer.
