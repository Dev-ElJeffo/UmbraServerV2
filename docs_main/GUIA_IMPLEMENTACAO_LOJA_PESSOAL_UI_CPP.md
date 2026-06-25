# Guia de implementação: loja pessoal (vendedor) — UI nativa em C++ (UE **5.6.1**)

Este documento descreve **o que foi implementado em C++** para a loja pessoal do vendedor (`UUmbraPlayerStoreWidget`, modal de preço, slot com drop nativo, correção de índice na listagem), **como validar no Editor**, e uma **reanálise de lacunas** face ao plano original. Para nós Blueprint e renomes **literais**, ver [PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md). Para **só integração** (Class Defaults, **Event Graph** vazio ou só visual, **Create Widget** onde abrir a loja), ver [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md).

**Nameplate + comprador + movimento:** [GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md](GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md).

**Relação com outros guias**

- Fluxo Blueprint nó a nó (legado / referência): [GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md](GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md) — secção **§2.4** resume o mapeamento BP → C++.
- Este arquivo aprofunda **arquitetura C++**, **Editor** e **itens em aberto**.

---

## 1. Ficheiros e responsabilidades

| Ficheiro | Classe | Função |
|----------|--------|--------|
| `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraPlayerStoreWidget.h` / `.cpp` | `UUmbraPlayerStoreWidget` | Estado (10 slots, preços rascunho/confirmados, oferta ativa), grade Modo A, botões, delegates `OnPersonalShopOpenedLocal` / `ClosedLocal`, `OpenPersonalShop` via `BuildOpenShopListings`. |
| `UmbraEternumUE/.../UI/UmbraSetItemPriceWidget.h` / `.cpp` | `UUmbraSetItemPriceWidget` | Modal: `TextBox_GoldAmount`, confirmar (`ParseInt64`, `> 0`) → `ApplyPriceForSlot`, cancelar. |
| `UmbraEternumUE/.../UI/UmbraStoreSlotWidget.h` / `.cpp` | `UUmbraStoreSlotWidget` | `NativeOnDrop`: `ProcessItemDropFromOperation` + `RegisterItemDroppedInSlot` na loja. |
| `UmbraEternumUE/.../UI/UmbraInventorySlotWidget.cpp` (+ `.h`) | `UUmbraInventorySlotWidget` | Nos ramos **loja pessoal** de `ProcessItemDrop` e `ProcessItemDropFromOperation`: após `SetSlotData(ShopDragged)` repõe-se `SlotData.SlotIndex` para o índice da **grade 0–9**, evitindo confundir com o slot de inventário de origem. |

**Dados enviados ao servidor**

- Struct `FUmbraPersonalShopListingInput` em `UmbraDataStructures.h`: `SlotIndex`, `InventoryID`, `PriceGold` (`int64`).
- `UUmbraPlayerStoreWidget::BuildOpenShopListings` só inclui slots com `SlotHasOffer[i]` e preço confirmado `> 0`.

**GameInstance (já existente)**

- `OpenPersonalShop`, `ClosePersonalShop`, delegates — **não** duplicados nesta feature; o widget chama a API já exposta em `UUmbraGameInstance`.

---

## 2. Ciclo de vida e fluxo (resumo)

1. **`NativeConstruct`** (`UUmbraPlayerStoreWidget`): resolve widgets opcionais por nome se necessário, `PopulateStoreSlotsModeA` (10 filhos em `Grid_StoreSlots`, `SetSlotIndex(i)`, `bIsPersonalShopListingSlot = true`, `ParentPersonalShopWidget = this`), `InitializeSlotPriceGrid` (procura `Text_Price_0` … `9`), liga botões, subscreve delegates, alinha visibilidade Start/Close com `IsLocalPersonalShopOpen()`.
2. **Drop** (ideal): `WBP_StoreSlot` com parent `UUmbraStoreSlotWidget` → `NativeOnDrop` → espelho de item + `RegisterItemDroppedInSlot` → barra **Set Price / Confirm / Cancel**.
3. **Preço**: `OpenSetPriceModal` → `CreateWidget` com `SetItemPriceWidgetClass` → `SetupModal(Store, SelectedSlotIndex)` → utilizador confirma → `ApplyPriceForSlot` atualiza `DraftPriceGold` e o rótulo.
4. **Confirmar oferta**: `TryConfirmOfferForSelectedSlot` valida item + preço rascunho, grava em `Confirmed*` / `SlotHasOffer`, colapsa barra.
5. **Abrir loja**: `OnStartStoreClicked` lê nome em `ShopNameText`, monta array com `BuildOpenShopListings`, chama `OpenPersonalShop`.
6. **`NativeDestruct`**: remove subscrições aos delegates.

**Classes `Abstract`**

- `UUmbraPlayerStoreWidget`, `UUmbraSetItemPriceWidget` e `UUmbraStoreSlotWidget` estão marcadas como **`UCLASS(Abstract)`**. No Editor, as WBP **têm de** usar essas classes como **Parent Class** dos Blueprints concretos (`WBP_PlayerStore`, etc.); não se instancia o `.generated.h` “puro” sem filho BP.

---

## 3. Designer: nomes esperados (resolução em C++)

Os **nomes dos widgets** na hierarquia UMG (ex. `ShopNameText`, `Grid_StoreSlots`) devem coincidir com as strings em `TryResolveOptionalWidgetsByName` / `GetWidgetFromName`. As variáveis C++ que guardam os ponteiros usam **prefixo `Cached*`** (`CachedTextShopName`, `CachedGridStoreSlots`, …) para **não** repetir o identificador do widget: no UE 5.x, o SKEL do Blueprint filho gera `ObjectProperty` com o nome do widget na hierarquia; se o pai C++ tivesse `UPROPERTY TObjectPtr ... ShopNameText`, ocorre **ICE** (“property already exists”).

**`WBP_PlayerStore` (parent `UmbraPlayerStoreWidget`)**

- Painéis / caixas: `Grid_StoreSlots`, `Grid_SlotPrices`, `HBox_SellItemOptions`, `HBox_Start_Store`, `HBox_Close_Store`
- Botões: `BTN_Set_Price`, `BTN_Confirm_Offer`, `BTN_Cancel_Offer`, `BTN_Start_Store`, `BTN_Close_Store`, `Btn_Close`
- Texto: `ShopNameText` (nome ao abrir loja), `Text_Vendor_Name` (atualizado ao abrir loja no cliente e após Start com o nome usado)
- Preços por slot: widgets nomeados `Text_Price_0` … `Text_Price_9` (filhos nomeados na hierarquia; não é obrigatório estarem sob `Grid_SlotPrices` para o código atual, desde que o nome exista no widget tree)

**`WBP_SetItemPrice` (parent `UmbraSetItemPriceWidget`)**

- `TextBox_GoldAmount`, `BTN_Confirm`, `BTN_Cancel`

**Class Defaults do `WBP_PlayerStore`**

- **Store Slot Widget Class** → `WBP_StoreSlot` (deve herdar `UUmbraStoreSlotWidget` ou, no mínimo, `UUmbraInventorySlotWidget` com drop BP que chame a loja).
- **Set Item Price Widget Class** → `WBP_SetItemPrice`.

---

## 4. Checklist de validação (após compilar)

1. **Reparent** e **Class Defaults** conforme secção 3.
2. PIE: arrastar item do inventário para cada um dos 10 slots; ícone/texto atualizam (depende do **Blueprint** do slot implementar `RefreshSlotDisplay` / visual — ver lacunas abaixo).
3. **Set Price** → valor `> 0` → rótulo `Text_Price_N` atualizado.
4. **Confirm Offer** → barra de opções fecha; **Start Store** com nome não vazio e ≥1 oferta → chamada `OpenPersonalShop` (observar `OnPersonalShopActionFailed` se validação falhar).
5. **Close Store** e visibilidade Start/Close após abrir/fechar.
6. Inventário normal: drag entre slots **fora** da loja sem regressão.

---

## 5. Reanálise: o que está coberto e o que falta / opcional

### 5.1 Alinhado com o plano

- Estado de 10 slots, arrays `Confirmed*` / `Draft*` / `SlotHasOffer`, `SelectedSlotIndex`.
- `PopulateStoreSlotsModeA` com `SetSlotIndex(i)`, flags loja, `ParentPersonalShopWidget`.
- Modal C++, botões e integração `OpenPersonalShop` / `ClosePersonalShop`.
- `UUmbraStoreSlotWidget` com `NativeOnDrop`.
- Correção **SlotIndex** no ramo loja (`ProcessItemDrop` **e** `ProcessItemDropFromOperation`).
- Resolução por nome (`TryResolveOptionalWidgetsByName`) no painel da loja (sem `BindWidgetOptional` no pai — múltiplos WBP filhos).
- Documentação resumida em §2.4 do guia WBP.

### 5.2 Lacunas ou diferenciações conscientes

| Item | Estado |
|------|--------|
| **Modo B (§3.5 guia)** — 10 filhos já no Designer em `Grid_StoreSlots` sem `ClearChildren` | **Não implementado** em C++; só **Modo A** (runtime). Se precisar de Modo B, ou mantém lógica no BP do filho, ou acrescenta-se `PopulateStoreSlotsDesignerMode()` que percorra filhos existentes. |
| **`.uasset`** (reparent, apagar grafos antigos) | **Manual no Editor**; não versionável de forma fiável só a partir deste repositório sem o UE. |
| **Visual do slot após drop** | `TryCallBlueprintUpdateSlotVisual()` chama evento implementável no BP do **slot**. Se `WBP_StoreSlot` não implementar o visual, o estado C++ está correto mas o ícone pode não atualizar até haver ligação ao `RefreshSlotDisplay` / lógica visual herdada. |
| **`ShopNameText` vs `Text_Vendor_Name`** | O nome para **abrir** a loja vem de `ShopNameText`. Quando o servidor confirma abertura, `HandleShopOpenedLocal` atualiza **`Text_Vendor_Name`**. Se quiser o mesmo texto nos dois sítios, pode alinhar-se no BP ou estender-se o C++ com uma linha extra. |
| **Fase 2 do plano** (comprador C++, placa `UpdatePersonalShopSign` nativa) | **Fora do âmbito** da implementação atual; continuar a usar o guia WBP §4–§5. |

### 5.3 Segurança e rede

- Esta feature **não** altera `MovementProtocol`, TCP nem PHP; validação de preço no servidor continua responsabilidade da API.

---

## 6. Resolução de problemas

| Sintoma | Causa provável |
|---------|----------------|
| Widgets não ligam / null em runtime | Nome no Designer diferente do esperado; alinhar ao guia ou acrescentar o nome em `TryResolveOptionalWidgetsByName`. |
| ICE “property ShopNameText already exists” ao compilar WBP filho | Garantir C++ com membros **`Cached*`** (não repetir o nome do widget na `UPROPERTY`); recompilar C++ e voltar a **Compile** no WBP. |
| “Abstract class” ao criar widget | Está a instanciar a classe C++ abstrata; use sempre o **Blueprint** filho como classe de widget. |
| Drop não mostra barra de preço | `ParentPersonalShopWidget` / `RegisterItemDroppedInSlot` não disparados — confirme parent `UUmbraStoreSlotWidget` ou OnDrop BP. |
| Preço aplicado ao slot errado | Regressão de `SlotIndex`: verificar que o binário inclui o patch em `UmbraInventorySlotWidget.cpp` (restauro após `SetSlotData`). |
| Start Store não faz nada | Nome vazio, sem ofertas confirmadas, ou `OnPersonalShopActionFailed` no GI — ver Output Log. |

---

## 7. Referência rápida de API (vendedor)

| Método | Descrição |
|--------|-----------|
| `RegisterItemDroppedInSlot(SlotIndex, SlotWidget)` | Define slot selecionado e mostra `HBox_SellItemOptions`. |
| `ApplyPriceForSlot(SlotIndex, PriceGold)` | Rascunho de preço + atualização do `Text_Price_*`. |
| `TryConfirmOfferForSelectedSlot()` | Promove rascunho a oferta confirmada no índice selecionado. |
| `TryCancelOfferUI()` | Limpa slot selecionado (oferta + preço) e esconde a barra. |
| `BuildOpenShopListings(Out)` | Monta o array para `OpenPersonalShop`. |
| `OpenSetPriceModal()` | Abre o widget classe `SetItemPriceWidgetClass`. |

---

*Última revisão alinhada ao código em `UmbraEternumUE/Source/UmbraEternumUE/UI/` (loja pessoal vendedor, UI nativa).*
