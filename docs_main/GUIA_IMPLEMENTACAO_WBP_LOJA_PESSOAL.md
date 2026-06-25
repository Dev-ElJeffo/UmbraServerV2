# Guia de Implementação: Loja pessoal (Player Shop) — Blueprint, nós exatos

**Engine:** Unreal Engine 5.x · **Código:** `UmbraEternumUE/`  
**Backend:** `www/umbra_api/api/shop/*.php` · **Zona WS:** tipos binários **60–63** (`MovementProtocol.hpp`)

Este documento descreve **nós Blueprint exatos** (nomes como aparecem no editor) e **cadeias** `→` entre eles. Não substitui o plano de arquitetura; foca na **camada UE (WBP + personagem + HUD)**.

---

## Índice

1. [Pré-requisitos](#1-pré-requisitos)
2. [Variáveis e tipos no Blueprint](#2-variáveis-e-tipos-no-blueprint) · [**2.4 Variante C++ (nativo)**](#24-variante-c-nativo) · [Guia C++ (UI nativa)](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md) · [Plano de nós Editor](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md)
3. [WBP_PlayerStore — hierarquia real e fluxo (vendedor)](#3-wbp_playerstore--hierarquia-real-e-fluxo-vendedor) — **comece por [§3.0 sequências de nós](#30-sequências-de-nós-referência-única)** (logo abaixo) · **`WBP_StoreSlot`** (§3.4), grade §3.5, drop §3.6
4. [Nameplate / placa world-space — UpdatePersonalShopSign](#4-nameplate--placa-world-space--updatepersonalshopsign)
5. [WBP_LojaComprador — listar e comprar](#5-wbp_lojacomprador--listar-e-comprar)
6. [Abrir loja ao clicar na placa](#6-abrir-loja-ao-clicar-na-placa)
7. [Bloqueio de movimento (vendedor local)](#7-bloqueio-de-movimento-vendedor-local)
8. [Delegates — ligações recomendadas](#8-delegates--ligações-recomendadas)
9. [Checklist final](#9-checklist-final)

---

## 1. Pré-requisitos

| Passo | Ação |
|--------|------|
| BD | Executar `www/umbra_api/scripts/create_personal_shop_tables.sql` no MySQL (`umbra_eternum`). |
| API | Endpoints em `/api/shop/` (open, close, get, purchase) acessíveis pelo mesmo `ServerURL` do VaRest. |
| Personagem | `WidgetComponent` no mesh (já usado para nameplate); **mesmo** User Widget pode implementar `UpdateNameplate` **e** `UpdatePersonalShopSign`. |
| Compilar | Projeto C++ compilado após a feature (GameInstance + `NetMovementClient` + opcional `UmbraPersonalShopSignWidget`). |

---

## 2. Variáveis e tipos no Blueprint

### 2.1 Structs (aparecem como pins)

| Struct | Uso |
|--------|-----|
| **Personal Shop Listing Input** (`FUmbraPersonalShopListingInput`) | `Slot Index`, `Inventory ID`, `Price Gold` — um por linha ao chamar **Open Personal Shop**. |
| **Personal Shop Listing Entry** (`FUmbraPersonalShopListingEntry`) | Resposta do servidor: inclui **Listing ID**, preço, item, etc. |
| **Personal Shop State** (`FUmbraPersonalShopState`) | **Shop ID**, **Seller Player ID**, **Shop Name**, **Listings** (array). |

### 2.2 Funções no `UmbraGameInstance` (categoria **Social \| Personal Shop**)

| Nó (nome amigável no BP) | Exec / Pure | Parâmetros principais |
|--------------------------|-------------|----------------------|
| **Open Personal Shop** | Exec | `Shop Name` (String), `Listings` (Array de Personal Shop Listing Input) |
| **Close Personal Shop** | Exec | `Shop ID` (int; **0** = fecha loja aberta do personagem atual) |
| **Load Personal Shop By Seller** | Exec | `Seller Player ID` (int32) |
| **Purchase Personal Shop Listing** | Exec | `Listing ID` (int32) |
| **Is Local Personal Shop Open** | Pure | — → bool |
| **Get Local Personal Shop ID** | Pure | → int32 |
| **Get Local Personal Shop Name** | Pure | → String |
| **Get Cached Personal Shop State** | Pure | → Personal Shop State |
| **Create Inventory Slot From Personal Shop Listing** | Pure (static) | `Entry`, `Display Slot Index`, `Game Instance` → **Umbra Inventory Slot** |
| **Update Personal Shop Buyer UI From State** | Exec (static) | `World Context Object`, `Shop State`, `Listing Slot Widgets` (array de **Umbra Inventory Slot Widget**) |
| **Update Remote Player Personal Shop Sign** | Exec | `Player ID`, `Shop ID`, `Shop Name`, `b Visible` |

### 2.3 Eventos (delegates) — **Events** no painel do Game Instance

| Evento (assign no Blueprint) | Parâmetros |
|--------------------------------|------------|
| **On Personal Shop Action Failed** | `Error Message` (String) |
| **On Personal Shop Opened Local** | `Shop ID`, `Shop Name` |
| **On Personal Shop Closed Local** | (sem parâmetros) |
| **On Personal Shop State Loaded** | `Shop State` (Personal Shop State) |
| **On Remote Personal Shop Visual Updated** | `Seller Player ID`, `Shop ID`, `Shop Name`, `b Shop Open` |
| **On Personal Shop Purchase Completed** | `Listing ID`, `New Buyer Gold`, `Price Gold` |

**Como ligar:** no **Level Blueprint**, **HUD**, **Game Mode** ou widget raiz que tenha acesso ao mundo:

1. **Get Game Instance** → **Cast to Umbra Game Instance** (falha → Return).
2. Arrastar do return pin → **Assign On Personal Shop …** (menu de contexto do delegate).
3. Criar **Custom Event** com os mesmos parâmetros que o delegate mostrar.

### 2.4 Variante C++ (nativo)

A lógica do vendedor (estado de 10 slots, preços, modal, **Start/Close** loja, delegates do `UmbraGameInstance`) pode viver em C++ com Blueprint só para **layout** (Designer) e **Parent Class** nas WBP.

| Blueprint / guia (legado) | C++ |
|---------------------------|-----|
| Variáveis §3.3 (`StoreSlotWidgets`, arrays 10, `SelectedSlotIndex`) | `UUmbraPlayerStoreWidget` — `UPROPERTY` no header |
| **Event Construct** Modo A (grade + slots) | `NativeConstruct` → `PopulateStoreSlotsModeA` |
| **Initialize Slot Price Grid** | `InitializeSlotPriceGrid` (`Text_Price_0` … `Text_Price_9` ou fallback por nome) |
| **Register Item Dropped In Slot** | `UUmbraPlayerStoreWidget::RegisterItemDroppedInSlot` |
| **Apply Price For Slot** | `UUmbraPlayerStoreWidget::ApplyPriceForSlot` |
| Modal §3.7 | `UUmbraSetItemPriceWidget` + `OpenSetPriceModal` / `SetupModal` |
| **Confirm / Cancel** oferta | `TryConfirmOfferForSelectedSlot`, `TryCancelOfferUI` |
| **Start Store** (montar listings + `Open Personal Shop`) | `OnStartStoreClicked` → `BuildOpenShopListings` → `UUmbraGameInstance::OpenPersonalShop` |
| **Close Store** | `OnCloseStoreClicked` → `ClosePersonalShop(0)` |
| Visibilidade Start/Close | `RefreshStartCloseVisibilityFromGameInstance` + `OnPersonalShopOpenedLocal` / `OnPersonalShopClosedLocal` |
| Drop no slot da loja | Preferir **`UUmbraStoreSlotWidget`** (`NativeOnDrop` → `ProcessItemDropFromOperation` + `RegisterItemDroppedInSlot`) em vez de grafo **OnDrop** no BP |
| Índice 0–9 vs inventário | `ProcessItemDrop` / `ProcessItemDropFromOperation` (ramo loja): após `SetSlotData` restaura-se `SlotData.SlotIndex` para o índice da grelha da loja |

**Ficheiros:** `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraPlayerStoreWidget.h/.cpp`, `UmbraSetItemPriceWidget.h/.cpp`, `UmbraStoreSlotWidget.h/.cpp`.

**Nomes no Designer (resolvidos em C++ por nome, sem `BindWidgetOptional` na classe pai):** evita ICE ao compilar vários WBP filhos de `UmbraPlayerStoreWidget` / `UmbraSetItemPriceWidget` no UE 5.x. Use os mesmos nomes: `Grid_StoreSlots`, `Grid_SlotPrices`, `HBox_SellItemOptions`, `HBox_Start_Store`, `HBox_Close_Store`, `BTN_Set_Price`, `BTN_Confirm_Offer`, `BTN_Cancel_Offer`, `BTN_Start_Store`, `BTN_Close_Store`, `Btn_Close`, `ShopNameText`, `Text_Vendor_Name`. Modal: `TextBox_GoldAmount`, `BTN_Confirm`, `BTN_Cancel`. O C++ preenche ponteiros em `TryResolveOptionalWidgetsByName` / `NativeConstruct` com `GetWidgetFromName` se necessário.

**Migração no Editor (obrigatório após compilar C++):**

1. **`WBP_PlayerStore`**: *Class Settings* → **Parent Class** = `UmbraPlayerStoreWidget`. Apagar funções/Construct duplicados da lógica migrada (deixar só layout; **Event Graph** pode ficar **vazio** — o C++ `NativeConstruct` corre sem nó “Parent: Construct” no Blueprint).
2. **`WBP_SetItemPrice`**: **Parent Class** = `UmbraSetItemPriceWidget`.
3. **`WBP_StoreSlot`**: **Parent Class** = `UmbraStoreSlotWidget` (recomendado) *ou* manter `UmbraInventorySlotWidget` com **OnDrop** fino que chame `RegisterItemDroppedInSlot` — com `UmbraStoreSlotWidget` o drop fica todo em C++.
4. **Class Defaults** de `WBP_PlayerStore`: **Store Slot Widget Class** = `WBP_StoreSlot`, **Set Item Price Widget Class** = `WBP_SetItemPrice`.

**PIE / regressão:** arrastar item para slot 0–9; **Set Price** → modal → confirmar preço; **Confirm Offer**; **Start Store** com nome + ≥1 oferta; **Close Store**; inventário normal (fora da loja) sem colisão de índice/preço.

**Plano literal (Editor):** [PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md) — cada **Parent Class**, **Class Defaults**, renome no **Designer**, nós a **apagar**; **Event Graph** pode ficar **vazio** (C++ `NativeConstruct` não exige “Parent: Construct”).

---

## 3. WBP_PlayerStore — hierarquia real e fluxo (vendedor)

Guia alinhado ao asset **`WBP_PlayerStore`**: grade **`Grid_StoreSlots`**, barra **`HBox_SellItemOptions`** (Set Price / Confirm / Cancel) e **`HBox_Start_Store`** / **`HBox_Close_Store`** com visibilidade conforme a loja está ou não aberta no servidor.

### 3.0 Sequências de nós (referência única)

Só ordem de nós e pins essenciais. Detalhes extras ficam nas seções §3.1 em diante.

### A) `WBP_PlayerStore` — Class Defaults (antes de qualquer grafo)

- `SelectedSlotIndex` = **-1**
- `ConfirmedInventoryID`, `ConfirmedPriceGold`, `DraftPriceGold`: array **10** elementos (valor **0**)
- `SlotHasOffer`: array **10** elementos **false**
- `StoreSlotWidgets`: tipo **Array** de **Umbra Inventory Slot Widget** (pode começar vazio; o Construct enche)

### B) `WBP_PlayerStore` — **Event Construct** — **Modo A** (cria 10 slots em runtime)

```
Event Construct
  → Clear Children (Target = Grid_StoreSlots)
  → Clear (Target = StoreSlotWidgets)
  → For Loop (0 … 9) Index = i
        → Create Widget (Class = WBP_StoreSlot, Owning Player = Get Owning Player)
        → Set Owning Player Store (Target = Return, Owning Player Store = Self)
        → Set Parent Personal Shop Widget (Target = Return, = Self)
        → Set Store Slot Index (Target = Return, = i)
        → Add Child to Uniform Grid (Grid_StoreSlots, Content = Return, Row = i/5, Column = i%5)
        → ADD (StoreSlotWidgets, Item = Return)
  → (visibilidade Start/Close conforme Game Instance)
  → Initialize Slot Price Grid (Target = Self)   [se existir §K]
```

### C) `WBP_PlayerStore` — **Event Construct** — **Modo B** (10 filhos já no Designer em `Grid_StoreSlots`)

```
Event Construct
  → Clear (StoreSlotWidgets)     [NÃO Clear Children em Grid_StoreSlots]
  → For Loop (0 … 9) Index = i
        → Get Child at Index (Target = Grid_StoreSlots, Index = i)
        → Cast to WBP_StoreSlot
        Cast Failed → (Print opcional) → (próximo i ou Return)
        Cast Success
              → Set Owning Player Store (Target = As WBP_StoreSlot, Owning Player Store = Self)
              → Set Parent Personal Shop Widget (Target = As WBP_StoreSlot, Parent Personal Shop Widget = Self)
              → Set Store Slot Index (Target = As WBP_StoreSlot, Store Slot Index = i)
              → ADD (StoreSlotWidgets, Item = As WBP_StoreSlot)
  → (visibilidade Start/Close)
  → Initialize Slot Price Grid (Target = Self)
```

### D) `WBP_PlayerStore` — **Function `RegisterItemDroppedInSlot`** (inputs: `Slot Index`, `Slot Widget`)

```
Function Entry
  → Set Selected Slot Index (= pin Slot Index de entrada)
  → Set Visibility (Target = HBox_SellItemOptions, Visibility = Visible)
  → Return
```

### E) `WBP_StoreSlot` — fim do fluxo de drop válido (onde já chama `Process Item Drop…`)

```
… exec após drop OK …
  → Get Owning Player Store
  → Is Valid
  → Branch (Condition = válido)
        False → Return
        True  → Register Item Dropped In Slot
                    Target     = Owning Player Store
                    Slot Index = Get Store Slot Index (Target = Self)
                    Slot Widget= Self
        → Return (OnDrop true se for o caso)
```

### F) `WBP_PlayerStore` — **BTN_Set_Price** → **OnClicked**

```
OnClicked (BTN_Set_Price)
  → Branch
        Condition ← Integer >= Integer   (A = Get SelectedSlotIndex, B = 0)
        False → Print (opcional) → Return
        True  → Get Owning Player
              → Create Widget (Class = WBP_SetItemPrice, Owning Player = …)
              → Set Owning Player Store (Target = Return CreateWidget, Owning Player Store = Self)
              → Set Target Slot Index     (Target = Return CreateWidget, Target Slot Index = Get SelectedSlotIndex)
              → Add to Viewport (Target = Return CreateWidget, ZOrder = 100)
              → (opcional) Set Keyboard Focus … TextBox_GoldAmount do modal
```

**Pré-requisito no modal:** variável **Integer** `TargetSlotIndex` em `WBP_SetItemPrice` (Compile no modal antes de achar o setter).

### G) `WBP_SetItemPrice` — **BTN_Confirm** → **OnClicked** (preço → pai)

```
OnClicked (BTN_Confirm)
  → Branch   Condition ← Integer > Integer (A = ParsedPrice ou Conv String→Int, B = 0)
        False → Print → Return
        True  → Apply Price For Slot
                    Target     = Get OwningPlayerStore
                    Slot Index = Get TargetSlotIndex
                    Price Gold = ParsedPrice (ou Int64)
        → Remove from Parent (Target = Self)
```

### H) `WBP_PlayerStore` — **Function `ApplyPriceForSlot`** (`Slot Index`, `Price Gold` inputs)

```
Function Entry
  → Branch (Slot Index < 0) → Print → Return
  → Branch (Slot Index > 9) → Print → Return
  → Set Array Elem (Target = DraftPriceGold, Index = Slot Index, Item = Price Gold)
  → Get Slot Price Texts → GET copy (Index = Slot Index) → Set Text (Target = Text, In Text = formatado)
  → Return
```

### I) `WBP_PlayerStore` — **BTN_Confirm_Offer** → **OnClicked**

```
OnClicked
  → Branch (SelectedSlotIndex >= 0)  False → Print → Return
  → Get StoreSlotWidgets → GET copy (Index = SelectedSlotIndex)
  → (Cast to Umbra Inventory Slot Widget se precisar) → Get Slot Data → Inventory ID
  → Branch (Inventory ID > 0)  False → Print → Return
  → Get DraftPriceGold → GET copy (Index = SelectedSlotIndex)
  → Branch (preço > 0)  False → Print → Return
  → Set Array Elem (ConfirmedInventoryID, Index = SelectedSlotIndex, Item = Inventory ID)
  → Set Array Elem (ConfirmedPriceGold,   Index = SelectedSlotIndex, Item = GET DraftPriceGold)
  → Set Array Elem (SlotHasOffer,          Index = SelectedSlotIndex, Item = true)
  → Set Visibility (HBox_SellItemOptions, Collapsed)
  → Set SelectedSlotIndex (= -1)
  → Return
```

### J) `WBP_PlayerStore` — **Function `InitializeSlotPriceGrid`**

```
Function Entry
  → Make Array [0]…[9] = Get Text_Price_0 … Text_Price_9 (ou mapeamento §3.7.7.1)
  → Set Slot Price Texts (= Return Make Array)
  → For 0…9
        → Get Slot Price Texts → GET (i) → Set Text (In Text = "—")
  → Return
```

**`Event Construct` (trecho):** `… Create Store Slots ou B/C …` → **`Initialize Slot Price Grid` (Self)`**.

---

### 3.1 Designer — hierarquia (referência)

```
[WBP_PlayerStore]
  Canvas Panel
    Border_InventoryPanel
      Vertical Box
        Border_TitleBar
          Horizontal Box
            ShopNameText          ← nome da loja (texto editável / título)
            Btn_Close             ← fecha o painel (UI); ver §3.13
        Border
          Vertical Box
            Text_Vendor_Name      ← ex.: mesmo nome ou sufixo "Store"
            Vertical Box
              (opcional) Uniform Grid ou overlay alinhado à grade:
                Grid_StoreSlots     ← Uniform Grid Panel: 10× WBP_StoreSlot
                Grid_SlotPrices     ← 10× Text — preço por slot (criação §3.7.7)
              Spacer
              Horizontal Box
              Vertical Box
                HBox_SellItemOptions   ← Collapsed por defeito; Visible após drop no slot
                  BTN_Set_Price         ← abre WBP_SetItemPrice (§3.7)
                  BTN_Confirm_Offer
                  BTN_Cancel_Offer
                HBox_Start_Store
                  BTN_Start_Store
                HBox_Close_Store
                  BTN_Close_Store
```

Marque **`Grid_StoreSlots`**, **`Grid_SlotPrices`**, **`HBox_SellItemOptions`**, **`HBox_Start_Store`**, **`HBox_Close_Store`** como **Is Variable** (a função **`PopulateGridSlotPrices`** dinâmica — §3.7.7.4 — usa **`Grid_SlotPrices`** no **Clear Children**).

### 3.2 Estados de UI

| Situação | `HBox_Start_Store` | `HBox_Close_Store` | `HBox_SellItemOptions` |
|----------|--------------------|--------------------|-------------------------|
| Montagem (loja **fechada** no servidor) | **Visible** | **Collapsed** | **Collapsed** até haver drop num slot |
| Loja **aberta** no servidor | **Collapsed** | **Visible** | **Collapsed** (recom.: não editar ofertas com loja aberta; novo `open` substitui no API) |

- **Ao abrir o widget:** popular **`Grid_StoreSlots`** com **`WBP_StoreSlot`** (§3.5) e aplicar a linha da tabela usando **`Is Local Personal Shop Open`** (Game Instance).
- **Após sucesso de abertura:** delegate **`On Personal Shop Opened Local`** → **Start** colapsado, **Close** visível (§3.11).
- **Após fecho no servidor:** **`On Personal Shop Closed Local`** → **Start** visível, **Close** colapsado (§3.12).

### 3.3 Variáveis (WBP_PlayerStore)

| Nome | Tipo | Uso |
|------|------|-----|
| `StoreSlotWidgets` | Array **Umbra Inventory Slot Widget** | Os 10 slots da grade |
| `ConfirmedInventoryID` | Array **Integer** (10, default 0) | Após **Confirm Offer** |
| `ConfirmedPriceGold` | Array **Integer** ou **Int64** (10) | Preço confirmado por índice |
| `SlotHasOffer` | Array **Boolean** (10) | Slot entra no payload do **Start Store** |
| `SelectedSlotIndex` | **Integer** (−1 = nenhum) | Slot em edição após drop / ao definir preço |
| `SlotPriceTexts` | **Array** de referências a **Text** (widget UMG), tamanho 10 | Preenchido por **`InitializeSlotPriceGrid`** (§3.7.7.3); atualizar no modal §3.7.5 |
| `DraftPriceGold` | **Array** **Int64** (10, default 0) | Rascunho do preço por índice **antes** do **Confirm Offer**; preenchido pelo **`WBP_SetItemPrice`** |
| `SetItemPriceWidgetClass` | **Class** → **`WBP_SetItemPrice`** (opcional) | Usado em **Create Widget** no clique de **BTN_Set_Price** |

**Obrigatório — tamanho 10 nos Class Defaults:** em **`WBP_PlayerStore`**, abra **Class Defaults** (botão **Class Defaults** no toolbar do Blueprint) e, para **`ConfirmedInventoryID`**, **`ConfirmedPriceGold`**, **`SlotHasOffer`** e **`DraftPriceGold`**, expanda cada array e use **+** até ficarem **dez elementos** (ex.: inteiros **0**, booleanos **false**, **Int64** **0**). Se o tamanho for **0**, o **`Set Array Elem`** no **Confirm Offer** dispara erro do tipo **`[0/0]`** / *invalid index* — o Blueprint **não** cria sozinho 10 posições só porque você usa índice **0…9** no nó.

**Nota:** o guia antigo usava um único **`Pending Price Editable`** na barra. O fluxo recomendado agora é **modal** (`WBP_SetItemPrice`), no mesmo espírito de telas de depósito/saque de gold: valor isolado, Confirm/Cancel, depois o pai atualiza arrays + texto sob o slot.

### 3.4 `WBP_StoreSlot` — asset dedicado (sem alterar o inventário)

**Objetivo:** a loja usa **sempre** **`WBP_StoreSlot`**. O **`WBP_InventorySlot`** permanece só para inventário / trade / outras telas — **sem** `Branch` “se estou na loja” e **sem** risco de regressões no que já funciona.

**Parent Class (C++):** **`Umbra Inventory Slot Widget`** — o mesmo tipo base do slot de inventário (drag & drop, `Set Slot Data`, `Get Slot Index`, etc.).

#### 3.4.1 Criar o asset no Content Browser

1. Pasta desejada (ex.: `UI/Shop/`) → clique direito → **User Interface** → **Widget Blueprint**.
2. **Pick Parent Class** → **`Umbra Inventory Slot Widget`**.
3. Nome sugerido: **`WBP_StoreSlot`**.

**Alternativa (recomendada se o inventário já tem Designer polido):** duplicar **`WBP_InventorySlot`** → renomear para **`WBP_StoreSlot`**. Confirmar em **Class Settings** que **Parent Class** = **`Umbra Inventory Slot Widget`**. No **Graph** do novo asset, **apagar** nós e variáveis usadas **só** para trade (`Parent Trade Widget`, `Trade Session ID`, …) ou armazém (`Parent Storage Widget`); ver §3.4.4.

#### 3.4.2 Designer

- Reutilizar a **mesma hierarquia visual** que o slot de inventário (tamanhos, ancoragens, ícone, stack count) para o **hit-test** e o **drag** comportarem-se igual.
- Em **Class Defaults** (`WBP_StoreSlot` → **Class Defaults**):
  - **`b Is Personal Shop Listing Slot`** = **true** (C++: `UmbraInventorySlotWidget::bIsPersonalShopListingSlot`, exposto com **`EditAnywhere`** para aparecer em **Class Defaults** e em **Details**). **Obrigatório.** Sem isso, **`Process Item Drop From Operation`** chama **`Move Item`** no `GameInstance` usando **`Get Slot Index`** como destino — os índices **0–9** da grade da loja são os **mesmos** que os primeiros slots do inventário, por isso o item **parece** “pular” só dentro do inventário em vez de ficar na loja.
  - Se **não** vires o campo em **Class Defaults** após alterar o C++: fecha o editor, **recompila** o projeto, reabre o **`.uproject`**, abre **`WBP_StoreSlot`** → **Class Defaults** → secção **Personal Shop** (ou usa a caixa de pesquisa *Personal* no painel).
  - Alternativa sempre válida: no **Event Construct** do **`WBP_StoreSlot`**, **Set** `b Is Personal Shop Listing Slot` = **true** no **Self** (equivalente ao default da classe).
  - **Obrigatório no C++ (recomendado mesmo com o bool):** ao criar cada slot no **`WBP_PlayerStore`**, além do Blueprint **`Owning Player Store`**, chama **`Set Parent Personal Shop Widget`** (variável C++ no slot, tipo **Object**) = **Self** — é a referência que o código nativo usa para saber que o drop é da loja. Só o bool no Construct às vezes **não** é aplicado ao objeto C++ antes do primeiro drop; o ponteiro **Parent Personal Shop Widget** evita esse caso.
  - Alinhar **`b Can Accept Items`** e **`b Can Drag From`** ao inventário (normalmente **true** / **true**).
- **Não** usar **`Parent Trade Widget`**, **`Parent Storage Widget`**, **`Trade Item ID`**, **`Trade Session ID`** para a loja: deixar **não definidos** / **0**. A loja liga-se ao painel só via variáveis Blueprint da §3.4.3.

#### 3.4.3 Variáveis Blueprint (`WBP_StoreSlot` → **My Blueprint** → **Variables**)

| Nome | Tipo | Uso |
|------|------|-----|
| `OwningPlayerStore` | **Object Reference** → **`WBP_PlayerStore`** (o seu User Widget da loja) | Preenchido pelo pai com **Self** ao criar o slot. Permite chamar **`Register Item Dropped In Slot`** sem **Custom Event** cruzado. |
| `StoreSlotIndex` | **Integer** | Índice **0–9** na grade da loja (payload **Start Store** / UI). |

Se **`WBP_PlayerStore`** ainda não existir como tipo, criar primeiro o asset da loja, **compilar**, e voltar a definir o tipo da variável (ou usar **User Widget** + **Cast** temporário — menos ideal).

#### 3.4.4 Graph do `WBP_StoreSlot` — só drop + notificação ao pai

1. Copiar **apenas** a parte do **`WBP_InventorySlot`** que trata **drop** até **`Process Item Drop From Operation`** (ou a cadeia **`Native On Drop`** / **`On Drop`** que o projeto usa), **sem** lógica de trade ou armazém.
2. Com **`b Is Personal Shop Listing Slot** = **true** (§3.4.2), **`Process Item Drop From Operation`** copia o item para o slot **sem** mover no servidor e tenta chamar **`Update Slot Visual`** no Blueprint (se existir). Depois, no mesmo ramo **true**:
   - **Get Owning Player Store** → **Is Valid** → **Branch**
   - **True:** → **Register Item Dropped In Slot**  
     - **Slot Index** = **Get Store Slot Index**  
     - **Slot Widget** = **Self**

**Não** use **`Set Slot Index`** (C++) nos slots da loja para representar o índice **0–9** da grade **a menos** que você tenha **`b Is Personal Shop Listing Slot`** ativo — caso contrário o C++ trata o destino como slot de inventário real.

#### 3.4.5 Resumo

| Item | Valor |
|------|--------|
| Asset | **`WBP_StoreSlot`** |
| Parent C++ | **`Umbra Inventory Slot Widget`** |
| **`WBP_InventorySlot`** | **Não** editar para a loja |
| **Class Defaults** | **`b Is Personal Shop Listing Slot** = **true** |
| Ligação ao painel | **`OwningPlayerStore`** + **`StoreSlotIndex`** no spawn; **`Register Item Dropped In Slot`** após drop válido |

#### 3.4.6 Sintoma: “soltar na loja move o item no inventário”

Causa: **`ProcessItemDropFromOperation`** em `UmbraInventorySlotWidget.cpp` chama **`RequestMoveItemByID`**, que usa **`GetSlotIndex()`** como **slot de destino no inventário**. Índices **0–9** na loja coincidem com os primeiros slots da mochila.

**Correção:** **`b Is Personal Shop Listing Slot** = **true** e/ou **`Parent Personal Shop Widget** = painel da loja (**Self**) no spawn (§3.4.2, §3.5). O C++ passa a **espelhar** dados (`SetSlotData` + **`Update Slot Visual`** ou **`Refresh Slot Display`** no Blueprint se existirem) **sem** `MoveItem`.

#### 3.4.7 Sintoma: “já não move no inventário mas o slot da loja fica vazio”

Causas comuns: (1) o modo loja **não** estava ativo no C++ (faltava **`Parent Personal Shop Widget`** ou o bool não chegava ao objeto); (2) **`Update Slot Visual`** / **`Refresh Slot Display`** não existem no **`WBP_StoreSlot`** — copiar do **`WBP_InventorySlot`** o grafo que atualiza ícone/quantidade após **`Set Slot Data`**.

---

### 3.5 Ao abrir o widget — `Create Store Slots` / `Grid_StoreSlots`

**Onde:** **`WBP_PlayerStore`** — **Event Construct** (ou função **`Create Store Slots`** chamada no **Construct**).

**Classe dos slots:** **`WBP_StoreSlot`** (§3.4).

**Escolha só um destes dois modos** na mesma **`Grid_StoreSlots`** — não misture **Clear Children** + **Create Widget** com dez filhos **já** desenhados no Designer (você apaga os do Designer no runtime ou duplica lógica).

---

**Modo A — Slots criados em runtime (sem filhos da loja no Designer na grade)**

- **`Clear Children`** em **`Grid_StoreSlots`**: **sim** (limpa recriações antigas).
- **`Clear`** no array **`StoreSlotWidgets`**: **sim**.
- Loop **0…9**: **Create Widget** → **Set Owning Player Store** / **Set Parent Personal Shop Widget** / **Set Store Slot Index** = **i** → **Add Child to Uniform Grid** → **ADD** em **`StoreSlotWidgets`**.

**Modo B — Dez `WBP_StoreSlot` já são filhos de `Grid_StoreSlots` no Designer**

- **`Clear Children`** em **`Grid_StoreSlots`**: **não** — apaga os dez widgets do Designer.
- **`Clear`** no array **`StoreSlotWidgets`**: **sim** — só esvazia a lista de referências; depois você enche de novo.
- **Não** use **Create Widget** + **Add Child** para os mesmos buracos (já estão ocupados na tela).
- Loop **0…9**: **`Get Child at Index`** (**Target** = **`Grid_StoreSlots`**, **Index** = **i**) → **Cast to WBP_StoreSlot** → **Set Owning Player Store** / **Set Parent Personal Shop Widget** / **Set Store Slot Index** = **i** → **ADD** em **`StoreSlotWidgets`**.
- Ordem dos filhos na **Hierarchy** deve corresponder a **0…9** (primeiro filho = slot **0**); senão o **`Store Slot Index`** não bate com o lugar visual.

---

**Uniform Grid (Modo A):** **`NumColumns`** = **5**; para **i** = **0…9**: **`Row`** = **i / 5**, **`Column`** = **i % 5**.

**Cadeia Modo A (referência):**

```
Event Construct (ou Create Store Slots)
  → Clear Children (Grid_StoreSlots)
  → Clear (StoreSlotWidgets)
  → For Loop (0 to 9) [Index = i]
        → Create Widget (Class = WBP_StoreSlot, Owning Player = self default)
        → Set Owning Player Store (target = return do Create Widget) = Self
        → Set Parent Personal Shop Widget (target = mesmo widget) = Self
        → Set Store Slot Index (target = mesmo widget) = i
        → Add Child to Uniform Grid (Grid_StoreSlots, Content = widget, Row, Column)
        → ADD (StoreSlotWidgets) = widget criado
  → Set Visibility (HBox_SellItemOptions) = Collapsed
  → Get Game Instance → Cast to Umbra Game Instance → Is Local Personal Shop Open?
        true  → HBox_Start_Store Collapsed, HBox_Close_Store Visible
        false → HBox_Start_Store Visible, HBox_Close_Store Collapsed
```

**Cadeia Modo B (referência):** igual à caixa do parágrafo **Modo B** acima (sem **Clear Children**, sem **Create Widget** na grade); depois o mesmo bloco de visibilidade **Start/Close** e **`InitializeSlotPriceGrid`** que você já tiver no **Event Construct**.

---

### 3.6 Drop no slot → `HBox_SellItemOptions` visível

#### 3.6.1 Resposta direta: Function vs Custom Event

| Pergunta | Resposta |
|----------|----------|
| **Onde chamar `RegisterItemDroppedInSlot`?** | **Só** no **`WBP_StoreSlot`**, **depois** de um drop **válido** (ex.: ramo **true** de **`Process Item Drop From Operation`** e **`Owning Player Store` válido**). O alvo do nó é a referência **`Owning Player Store`** → **Register Item Dropped In Slot** (`Slot Index`, `Slot Widget`). |
| **Onde chamar o Custom Event `OnItemDroppedInStoreSlot`?** | **Nunca** a partir do **`WBP_StoreSlot`**. O Unreal **não** permite arrastar um fio do slot e “chamar” um Custom Event em outro User Widget pelo nome. Só pode ser chamado **dentro do gráfico do próprio `WBP_PlayerStore`** (por outro nó do mesmo asset). |
| **Se existir o Custom Event, a Function deve ser chamada por ele?** | **Não** — é o **contrário** do que costuma ser útil: o fluxo recomendado é **`RegisterItemDroppedInSlot`** (Function) conter **toda** a lógica de visibilidade (`SelectedSlotIndex`, **`HBox_SellItemOptions`** visível, foco no preço). O Custom Event é **opcional** e, se você usar, deve ser **só** um extra (som, analytics) **chamado a partir do fim** da Function (**Call** → **`On Item Dropped In Store Slot`**), **ou** você pode **não criar** o Custom Event e ficar **só** com a Function. **Evite** colocar `Set Visibility` **duas vezes** (na Function e no Event) com os mesmos pins — escolha **um** lugar para a UI. |

**Resumo em uma frase:** o **`WBP_StoreSlot`** chama **apenas** a **Function** **`RegisterItemDroppedInSlot`** no pai; o **Custom Event** é interno ao **`WBP_PlayerStore`** e **opcional**; se existir, quem o dispara é a **Function** (ou outro evento **dentro** da loja), **não** o slot.

#### 3.6.2 Fluxo recomendado (maioria dos projetos)

```
[WBP_StoreSlot — OnDrop, após drop aceite]
  → … Process Item Drop From Operation …
  → Get Owning Player Store → Is Valid → Branch True
  → Register Item Dropped In Slot (Target = Owning Player Store)
        Slot Index = Store Slot Index
        Slot Widget = Self
  → Return true no OnDrop
```

```
[WBP_PlayerStore — Function RegisterItemDroppedInSlot]
  → Set Selected Slot Index = Slot Index
  → Set Visibility (HBox_SellItemOptions) = Visible
  → (opcional) foco no **BTN_Set_Price** ou no primeiro controle da barra
```

**Não** é obrigatório ter **`On Item Dropped In Store Slot`** neste fluxo.

#### 3.6.3 Custom Event opcional (`OnItemDroppedInStoreSlot`)

Use **só** se quiser um segundo gancho **dentro** da loja (ex.: som, log), **sem** duplicar a lógica de botões:

1. Cria o **Custom Event** **`OnItemDroppedInStoreSlot`** no **`WBP_PlayerStore`** com os mesmos inputs (`SlotIndex`, `SlotWidget`) **se** precisares deles no gancho extra.
2. No **fim** da **Function** **`RegisterItemDroppedInSlot`**, adiciona **Call** **`On Item Dropped In Store Slot`** (passa os pins).
3. No corpo do Custom Event: **apenas** o que for “extra” — **não** voltes a fazer **`Set Visibility`** da barra se já o fizeste na Function.

**Alternativa errada:** colocar **`Set Visibility`** só no Custom Event e **nada** na Function, **e** chamar o Event a partir do slot — **isso não funciona** do slot para o pai sem delegates/interfaces.

#### 3.6.4 Onde fica o gráfico do drop

- **Drop (On Drop / Process Item Drop):** no **`WBP_StoreSlot`**.
- **Visibilidade / estado da loja (`HBox_SellItemOptions`, `SelectedSlotIndex`):** na **Function** **`RegisterItemDroppedInSlot`** do **`WBP_PlayerStore`**, invocada pelo slot como acima.

#### Checklist rápido

- [ ] **`Register Item Dropped In Slot`** definida no **`WBP_PlayerStore`** (corpo com **Selected Slot Index** + **`HBox_SellItemOptions`** visível).  
- [ ] **`WBP_StoreSlot`** chama **só** essa Function no pai, após drop válido.  
- [ ] Custom Event **`On Item Dropped In Store Slot`**: **omitido** **ou** chamado **só** no fim da Function para extras — **não** duplicar UI.  
- [ ] **`Create Store Slots`** / **Construct** segue **§3.5 Modo A ou B** e preenche **`Owning Player Store`**, **`Store Slot Index`**, **`Parent Personal Shop Widget`**, **`StoreSlotWidgets`**.

#### 3.6.5 `SelectedSlotIndex` em **`-1`** — o que fazer, passo a passo

**O que `-1` significa:** no **`WBP_PlayerStore`**, ninguém gravou um slot da loja como “selecionado” desde o último reset. O **`BTN_Set_Price`** com teste **`SelectedSlotIndex >= 0`** vai cair no **False** se o valor for **`-1`**.

**Quem grava `0`…`9`:** só a **Function** **`RegisterItemDroppedInSlot`** na **loja**, com **`Set Selected Slot Index` =** entrada **`Slot Index`** (§3.6.2).

**O que você tem de conferir no Unreal (ordem):**

1. **`WBP_PlayerStore` → Functions → `RegisterItemDroppedInSlot`**  
   - Tem **`Set Selected Slot Index`**? O pin de dados vem do parâmetro **`Slot Index`** da própria function (não de uma constante).  
   - **Compile**.

2. **`WBP_StoreSlot` →** grafo onde o drop é aceito (ex.: depois de **`Process Item Drop From Operation`** com sucesso):  
   - Nó **`Register Item Dropped In Slot`**.  
   - **Target** = saída de **`Get Owning Player Store`** (variável do slot; tem de estar preenchida no **Construct** da loja — §3.5).  
   - **`Slot Index`** = **`Get Store Slot Index`** executado no **Self** do slot (é o índice **0–9** da **grade da loja**).  
   - **`Slot Widget`** = **Self**.  
   - O fio **exec** tem de passar por esse **Call** quando o drop for válido.

3. **`WBP_PlayerStore` → Class Defaults:** **`SelectedSlotIndex`** pode começar em **`-1`**. Isso é normal. **Depois de cada drop válido** no passo 2, o valor passa a ser **0…9**. Se você **nunca** largar item na loja após abrir a UI, continua **`-1`** — aí o **Set Price** **não** deve abrir o modal (é o **Branch** **False**).

4. **Se largou no segundo buraco e ainda grava `0`:** no **Construct** da loja (§3.5), cada **`WBP_StoreSlot`** precisa de **`Set Store Slot Index` = i** no loop. Sem isso, o passo 2 manda **`Slot Index` 0** para os dois buracos.

5. **Depois de Confirm Offer / Cancel (§3.8 / §3.9):** o guia zera a seleção com **`Selected Slot Index = -1`**. Se você clicar **Set Price** **sem** largar de novo na loja, volta a falhar — largue de novo ou mude a regra se quiser outro comportamento.

### 3.7 `BTN_Set_Price` + **`WBP_SetItemPrice`** (modal de valor)

**Objetivo:** em vez de editar o preço só na barra (`HBox_SellItemOptions`), abrir um widget de modal com caixa de texto (**`TextBox_GoldAmount`**) e botões **Confirm** / **Cancel**, no mesmo estilo de outras telas de ouro (valor isolado, confirmar ou cancelar).

#### 3.7.1 Designer do `WBP_SetItemPrice`

- **Raiz:** `Border` → `Vertical Box` → título (**Text Block** “Set Item Price”) → **`TextBox_GoldAmount`** (Editable Text) → `Horizontal Box` com **`BTN_Confirm`** e **`BTN_Cancel`**.
- Marcar **`TextBox_GoldAmount`**, **`BTN_Confirm`**, **`BTN_Cancel`** como **Is Variable**.
- **Hint Text** do `TextBox_GoldAmount`: pode ser vazio ou um número exemplo; o valor válido vem do usuário.

#### 3.7.2 Variáveis no `WBP_SetItemPrice` (Blueprint)

| Nome | Tipo | Uso |
|------|------|-----|
| `OwningPlayerStore` | **WBP Player Store** (referência) | Preencher depois do **Create Widget** do modal: **Target** = widget criado; valor = **Self** (a loja). |
| `TargetSlotIndex` | **Integer** (recomendado) | Ao abrir o modal, gravar o **Selected Slot Index** da loja. No **Confirm**, passar esse valor para **Apply Price For Slot** → **Slot Index**. |

#### 3.7.3 `WBP_PlayerStore` — **OnClicked** (`BTN_Set_Price`) — nós exatos

**Onde:** Blueprint **`WBP_PlayerStore`**, evento do botão **`BTN_Set_Price`**.

```
[Custom Event] OnClicked (BTN_Set_Price)
  → [Branch]
        Condition = [Integer >= Integer]
                      A = [Get] SelectedSlotIndex
                      B = [Literal Integer] 0
        True  → (continua)
        False → [Print String] In String = "Selecione um slot (drop) antes de definir preço."
                → (fim do fluxo)
  → [Get Owning Player]   (no contexto do User Widget; devolve o Player Controller)
  → [Create Widget]
        Class = WBP_SetItemPrice (ou variável **Set Item Price Widget Class**)
        Owning Player = saída do Get Owning Player
  → (opcional) [Promote to Variable] Return Value → variável local **Modal** (tipo WBP_SetItemPrice)
  → [Set OwningPlayerStore]
        Target = Return Value do Create Widget (ou **Modal**)
        Owning Player Store = [Self]
  → [Set TargetSlotIndex]
        Target = mesmo widget criado
        Target Slot Index = [Get] SelectedSlotIndex
  → [Add to Viewport]
        Target = Return Value / **Modal**
        ZOrder = 100
  → (opcional) [Set Keyboard Focus]
        In Widget to Focus = **TextBox_GoldAmount** do widget criado (arrasta a variável do modal)
```

**Regra:** a condição do slot precisa ser **`SelectedSlotIndex >= 0`**, não **`> 0`**, senão o **slot 0** nunca abre o modal.

#### 3.7.3.1 Ramo **False** no `BTN_Set_Price` com **`>= 0`**

Se o **Branch** usa **`GreaterEqual`** (**`>=`**) e mesmo assim cai no **else**:

- **`SelectedSlotIndex` está `-1` (ou outro valor menor que 0)** — veja **§3.6.5**. O teste **`>= 0`** fica **falso** para **`-1`**.
- O texto do **Print String** pode ser **fixo** (digitado no campo **In String** do nó). Ele **não** prova que o operador seja **`>`**; pode estar escrito “`> 0 = FALSE`” à mão mesmo com **`>=`** no grafo. Troque o texto ou use **Build String** + **Get Selected Slot Index** para depurar.

#### 3.7.3.2 `TargetSlotIndex` no **`WBP_SetItemPrice`** (criar e ligar no `WBP_PlayerStore`)

O **`Set Target Slot Index`** **só aparece** depois que a variável existir **no Blueprint do modal**.

1. Abra **`WBP_SetItemPrice`** → **My Blueprint** → **Variables** → **+** → nome **`TargetSlotIndex`**, tipo **Integer** → **Compile** → **Save**.
2. Abra **`WBP_PlayerStore`** → evento **`BTN_Set_Price` → OnClicked** (ramo **True** do **Branch**).
3. No fio que sai do **Create Widget** (**Return Value** = instância do modal), encadeie **nesta ordem** (exec):
   - **`Set Owning Player Store`** — **Target** = **Return Value** do **Create Widget**, **Owning Player Store** = **Self** (loja).
   - **`Set Target Slot Index`** — busque na paleta ou arraste do pin azul do **Return Value** do modal: a função/setter da variável **`TargetSlotIndex`**. **Target** = return do **Create Widget**. **Target Slot Index** (ou nome parecido) = **Get Selected Slot Index** na **loja** (**Self**).
   - **`Add to Viewport`** — **Target** = return do **Create Widget**.
4. No **`WBP_SetItemPrice`**, no **Confirm** (**§3.7.4**), o **Apply Price For Slot** → pin **Slot Index** deve usar **`Get Target Slot Index`** no **Self** do modal (valor gravado no passo 3), **não** um literal.

Se o **Passo 3** não mostrar **Set Target Slot Index**, falta **Compile** no **`WBP_SetItemPrice`** ou o **Target** do setter não está no widget criado (tem de ser o **Return Value** do **Create Widget**).

---

#### 3.7.4 `WBP_SetItemPrice` — **OnClicked** (`BTN_Confirm`) — pins e variável **`ParsedPrice`**

**Criar o evento:** no **Designer**, selecione **`BTN_Confirm`** → **Details** → **On Clicked** → **+**. No **Graph** aparece o evento do botão. A saída branca de execução costuma se chamar **`then`**.

**Variáveis usadas:** `TextBox_GoldAmount`, `OwningPlayerStore`, `TargetSlotIndex` (tabela §3.7.2). O exemplo abaixo usa preço como **Integer**. Se a função no pai usar **Int64**, troque **`Conv String to Integer`** por **`Conv String to Int64`**, a comparação por **`int64 > int64`**, e ligue o tipo certo no pin **Price Gold**.

##### O que é **`ParsedPrice`**

- **Significado:** o valor numérico do preço depois de ler a caixa e converter texto → número.
- **Tipo:** **Integer** (ou **Int64**, se for o caso do projeto).
- **Como obter:** `Get Text` na caixa → `Conv Text to String` → (opcional) `Trim` → `Conv String to Integer` → o pin **Return Value** desse último nó é o número.
- **Como guardar:** clique direito no **Return Value** do `Conv String to Integer` → **Promote to Variable** → nome **`ParsedPrice`**. Use **`Get ParsedPrice`** (ou o nome que o Unreal mostrar) na comparação **> 0** e de novo no pin **Price Gold** do **`Apply Price For Slot`**, para não converter duas vezes.

##### Fio branco (exec) x fio de dados

- **`Get Text`**, conversões e **`>`** não usam fio branco: são só **dados**.
- O primeiro fio **branco** depois do clique costuma ir para um **`Branch`** que testa se o preço é válido (**> 0**). O Unreal calcula a condição quando o **`Branch`** roda.

##### Fluxo com nomes de pins (resumo)

```
[OnClicked (BTN_Confirm)]
  then (exec) → [Branch]
        Condition ← [Integer > Integer] Return Value   (A = ParsedPrice, B = 0)
        False (exec) → [Print String] In String = "Digite um preço maior que zero."
        True (exec) → [Cast To WBP_PlayerStore]   (pule o Cast se OwningPlayerStore já for WBP_PlayerStore)
              Object ← [Get OwningPlayerStore] (pin da variável)
              Cast Failed (exec) → mensagem de erro / Return
              Cast Success (exec) → [Apply Price For Slot]
                    Target ← As WBP Player Store
                    Slot Index ← [Get TargetSlotIndex] (pin Target Slot Index)
                    Price Gold ← ParsedPrice
              then (exec) → [Remove from Parent]   Target ← Self
```

##### Cadeia só de dados (liga no **Condition** do Branch e no **Price Gold**)

```
[Get Text]  Target = TextBox_GoldAmount
  Return Value → [Conv Text to String]  In Text
  Return Value → [Trim] opcional  Source String
  Return Value → [Conv String to Integer]  String
  Return Value → variável ParsedPrice (Set ou Promote to Variable)
```

**Sem Trim:** ligue **Conv Text to String → Return Value** direto no pin **String** do **Conv String to Integer**.

**Sem Cast:** ramo **True** do **Branch** liga direto no **exec** do **Apply Price For Slot**; **Target** = **Get OwningPlayerStore** (já tipado como loja).

##### Nomes que podem mudar no editor

- **Branch:** às vezes as saídas aparecem como **True / False** ou **then / else**; o pin da condição é **Condition**.
- **Maior que:** na paleta pode aparecer como **int > int**; a saída é **Return Value** (bool).
- **Apply Price For Slot:** os pins seguem a função criada no **`WBP_PlayerStore`** (**Target**, **Slot Index**, **Price Gold**).
- Se **Price Gold** for **Int64**, coloque **Conv Integer to Int64** entre **ParsedPrice** e o pin.

---

#### 3.7.5 `WBP_PlayerStore` — função **`ApplyPriceForSlot`** (detalhamento completo)

**O que faz:** grava o preço em rascunho no array **`DraftPriceGold`** e atualiza o rótulo na grade (**`SlotPriceTexts`** ou **`Text_Price_*`**). **Não** chama API, **não** abre loja no servidor, **não** altera **`ConfirmedPriceGold`** / **`SlotHasOffer`** (isso é o **§3.8**).

**Pré-requisitos no mesmo Blueprint:** variáveis **`DraftPriceGold`** (array de **Integer** ou **Int64**, tamanho 10), **`SlotPriceTexts`** (array de referências ao widget **Text**, tamanho 10, preenchido em **§3.7.7.3**), **`StoreSlotWidgets`** (só se usar o bloco opcional com **Inventory ID** — tipo do array: **Umbra Inventory Slot Widget** Object Reference, **não** “User Widget” genérico; ver **§3.7.5.0.3**).

---

##### 3.7.5.0 Criar a função e os parâmetros

1. Abra o Blueprint **`WBP_PlayerStore`** e vá ao painel **Graph** (não o **Designer**).
2. **My Blueprint** (esquerda) → **Functions** → botão **+** ao lado de *Functions*.
3. Nome sugerido: **`ApplyPriceForSlot`**.
4. Com a função selecionada, no painel **Details** (à direita), em **Inputs**, clique em **+** e adicione:
   - **`SlotIndex`** — tipo **Integer**.
   - **`PriceGold`** — tipo **Integer** (ou **Int64**, igual ao restante do projeto e ao array **`DraftPriceGold`**).
5. **Compile** e **Save**.

Os pins **`Slot Index`** e **`Price Gold`** que o modal liga na chamada (**§3.7.4**) correspondem a esses dois parâmetros (o Unreal pode mostrar nomes com espaço: **Slot Index**, **Price Gold**).

---

##### 3.7.5.0.1 Onde achar cada nó no editor (busca na paleta / clique direito no gráfico)

No gráfico da função, **clique direito** em área vazia e use a caixa de busca (ou arraste da **Palette** à esquerda). Termos úteis (português do editor / inglês):

| O que você precisa | Como buscar (exemplos) |
|--------------------|-------------------------|
| Entrada da função | Já existe: nó **Function Entry** com os pins dos parâmetros. |
| Comparar inteiros | **“integer <”** / **Less (integer)**; **“integer >”** / **Greater (integer)**. |
| Decidir caminho | **“Branch”** / **Ramificação**. |
| Mensagem no log | **“Print String”** / **Imprimir sequência de caracteres**. |
| Sair da função | **“Return”** / **Retornar nó** (função void ainda pode ter **Return** só de execução). |
| Ler variável da loja | Arraste **`DraftPriceGold`**, **`SlotPriceTexts`**, **`StoreSlotWidgets`** da lista **My Blueprint → Variables** para o gráfico → aparece **Get**. |
| Escrever num elemento do array | **“Set Array Elem”** / **Definir elemento de matriz**. |
| Ler índice do array | Do **Get** do array, arraste o pin azul → **“Get (a copy)”** / obter cópia do elemento (índice = **Slot Index**). |
| Dados do slot (widget) | O pin do **GET (a copy)** precisa ser **Umbra Inventory Slot Widget** (ou WBP filha). Arraste do pin do widget → **“Get Slot Data”**. Se **não aparecer**, a variável **`StoreSlotWidgets`** está genérica (**User Widget**, etc.): altere o tipo do array em **Details** para **Umbra Inventory Slot Widget**, ou use **Cast to Umbra Inventory Slot Widget** (ou **Cast to** sua **WBP** do slot) no **Return Value** e chame **Get Slot Data** na saída do cast (**§3.7.5.0.3**). |
| Montar texto com número | **“Format Text”** / **Formatar texto**; para número → **“Integer to Text”** ou **“Conv Integer to Text”** / **“Int64 to Text”** conforme o tipo de **`PriceGold`**. |
| Mudar texto na tela | Arraste do widget **Text** → **“Set Text”** / **Definir texto**; ou busque **Set Text** e ligue o **Target**. |
| Escolher widget por índice (sem array) | **“Switch on Int”** / **Interruptor em inteiro**. |

Se a interface estiver em inglês, use os nomes em inglês na busca; em português, os nomes acima costumam aparecer traduzidos de forma parecida.

---

##### 3.7.5.0.2 Fluxo de execução (fio branco) — ordem obrigatória

A função valida o índice em **cascata**: só segue em frente no ramo **False** de cada **Branch** de erro.

```
[Function Entry]  (saída de execução “then”)
  → [Branch]  “Índice negativo?”
        Condition  ←  [Integer < Integer]:  A = Slot Index,  B = 0
        True  (exec)  →  [Print String]  “ApplyPriceForSlot: índice < 0”  →  [Return Node]
        False (exec)  →  [Branch]  “Índice maior que 9?”
              Condition  ←  [Integer > Integer]:  A = Slot Index,  B = 9
              True  (exec)  →  [Print String]  “ApplyPriceForSlot: índice > 9”  →  [Return Node]
              False (exec)  →  (opcional: ver §3.7.5.0.3) OU direto para o próximo passo
              →  [Set Array Elem]  …  →  [Set Text]  …  →  [Return Node]
```

**Ligação dos dados (pins coloridos, sem fio branco):** os nós **Integer <**, **Integer >**, **Get**, **Set Array Elem** (pins de dados), **Format Text**, etc. ligam entre si até alimentar **Condition** e os pins **Index** / **Item** / **In Text**.

---

##### 3.7.5.0.3 Bloco opcional — “só aplicar preço se o slot tiver item”

**Só monte isso se fizer sentido para o seu jogo.** Ele pode impedir o **Set Text** se **`Get Slot Data`** ainda devolver **`Inventory ID = 0`** (slot vazio no espelho), o que confunde na hora de testar o modal (**§3.7.5.1**).

Se quiser manter:

1. **Get** **`StoreSlotWidgets`**.
2. Do pin do array → **GET (a copy)** — **Array Index** = pin **Slot Index** (parâmetro da função).
3. **Get Slot Data** no **Return Value** do **GET** só funciona se o tipo do elemento do array for **Umbra Inventory Slot Widget** (C++) ou uma **WBP** que herde dela. Se o pin aparecer como **User Widget** / **Widget** e o menu **não** oferecer **Get Slot Data**:
   - **Preferível:** em **My Blueprint → Variables → StoreSlotWidgets → Details**, mude **Variable Type** para **Array** de **Umbra Inventory Slot Widget** (Object Reference). Recompile; ao **ADD** no **Create Store Slots**, o tipo continua compatível se os slots forem instâncias dessa classe/WBP filha.
   - **Alternativa sem mudar a variável:** do **Return Value** do **GET** → nó **Cast to Umbra Inventory Slot Widget** (ou **Cast to WBP_…** do seu slot, se for subclasse). Use a saída de objeto **“As Umbra Inventory Slot Widget”** (ou **As WBP_…**) como **Target** implícito ao arrastar e buscar **Get Slot Data**. O fio **exec** do ramo que segue para **Set Array Elem** deve sair de **Cast Succeeded** (e pode ligar **Cast Failed** em **Return** ou no mesmo fluxo de erro).
4. Do struct / pin **Inventory ID** → **Integer > Integer**: **A** = Inventory ID, **B** = 0.
5. **Branch**: **Condition** = resultado “maior que zero”.  
   - **False** (exec) → **Print** opcional → **Return**.  
   - **True** (exec) → ligue ao **exec** de entrada do próximo **Set Array Elem** (mesmo fio que usaria se não houvesse este bloco).

---

##### 3.7.5.0.4 Gravar o preço em **`DraftPriceGold`** (**Set Array Elem**)

1. Clique direito → busque **Set Array Elem** / **Definir elemento de matriz**.
2. **Target Array:** arraste **Get Draft Price Gold** (ou o nome da sua variável **`DraftPriceGold`**).
3. **Index:** ligue ao pin **Slot Index** da **Function Entry**.
4. **Item:** ligue ao pin **Price Gold** da **Function Entry**.  
   - O tipo do pin **Item** precisa ser o **mesmo** tipo dos elementos do array (**Integer** ou **Int64**). Se misturar, o Blueprint não compila ou converte errado.
5. **Size to Fit:** em geral **desligado**; o array já deve ter tamanho 10 desde o **Construct** / **Class Defaults**.
6. Entrada **exec** do **Set Array Elem** vem do **False** do último **Branch** válido (ou do **True** do bloco opcional do §3.7.5.0.3).
7. Saída **exec** do **Set Array Elem** → próximo nó (**Set Text** ou **Format Text** antes do **Set Text**).

---

##### 3.7.5.0.5 Atualizar o texto na grade (**Get** + **Set Text**)

**Com array `SlotPriceTexts`:**

1. Arraste **Get Slot Price Texts** para o gráfico.
2. Do pin do array → **GET (a copy)** — **Array Index** = **Slot Index** (mesmo pin da **Function Entry**).
3. **Return Value** desse **GET** é o **Target** do nó **Set Text** (arraste o fio para o pin **Target** do **Set Text**).
4. **In Text:**
   - **Opção A:** **Format Text** — texto literal tipo **`{0} gold`** — no pin **{0}** ligue **Conv Integer to Text** (ou **Conv Int64 to Text**) com entrada = **Price Gold**.
   - **Opção B:** só **Conv Integer to Text** em **Price Gold** ligado direto em **In Text** (sem “gold” no texto).
5. Entrada **exec** do **Set Text** = saída **exec** do **Set Array Elem** (ou da cadeia anterior).
6. Saída **exec** do **Set Text** → **Return Node**.

**Sem array (só `Text_Price_0` … `Text_Price_9`):** após validar índice, use **Switch on Int** com entrada **Slot Index** e dez saídas **0**…**9**; em cada saída, **Set Text** no **`Text_Price_N`** correspondente, com o mesmo texto que na opção A/B acima.

---

##### 3.7.5.0.6 Resumo visual (todos os nós em sequência lógica)

```
Function Entry  (Slot Index, Price Gold)
  exec → Branch₁  [Condition ← (Slot Index < 0)]
       True  → Print → Return
       False → Branch₂  [Condition ← (Slot Index > 9)]
              True  → Print → Return
              False → [opcional: validação Inventory ID → Branch₃ → Return se vazio]
              → Set Array Elem (DraftPriceGold, Index=Slot Index, Item=Price Gold)
              exec → Set Text (Target = SlotPriceTexts[Slot Index], In Text = formatado)
              exec → Return
```

---

##### 3.7.5.1 Preço certo no Print, mas o texto da grade não muda

**Erro muito comum (visto em Blueprint):** depois do **`Set Array Elem`** **não existe fio branco** ligado à saída **exec** do nó. Só atualizar um array **não redesenha** o **Text** na tela — é obrigatório encadear **`Set Text`** (ou equivalente) **depois** do **`Set Array Elem`**, no mesmo fio de execução. Confira também se o array que você altera é o de **rascunho** (**`DraftPriceGold`**, §3.3) e não só **`ConfirmedPriceGold`** / outro nome, se o fluxo do guia for “modal → rascunho → Confirm Offer”.

O modal (**`WBP_SetItemPrice`**) está lendo e convertendo o valor certo. O que falha é quase sempre **depois**: a função **`ApplyPriceForSlot`** na loja (**`WBP_PlayerStore`**) não chega no **`Set Text`**, ou atualiza **outro** widget.

**Confira nesta ordem:**

1. **Target do `Apply Price For Slot`**  
   Tem que ser a **mesma** janela de loja que está na tela. Se o modal guardar referência a **outra** instância da loja, o texto muda num widget que você não está vendo.

2. **Pin Slot Index**  
   Use o **`TargetSlotIndex`** gravado ao **abrir** o modal (§3.7.3). Não leia de novo o **`SelectedSlotIndex`** no Confirm se ele puder ter mudado. Coloque **Print** no começo de **`ApplyPriceForSlot`** com **`SlotIndex`** e **`PriceGold`**.

3. **Ramificação opcional com `Inventory ID`**  
   Se existir um **Branch** que só continua quando **`Inventory ID > 0`** e o espelho do slot ainda estiver **0**, a função **termina antes** de **`Set Text`**. Para testar, **desligue temporariamente** esse trecho ou garanta que o slot já tem item quando você confirma o preço.

4. **Array `SlotPriceTexts`**  
   Confira se o array tem **10** itens válidos (**Print Length**, **Is Valid** no elemento no índice **`SlotIndex`**). Tipo do array: referência ao widget **Text**, não lista de **String**. Chame **`InitializeSlotPriceGrid`** no **Construct** se ainda não chama.

5. **`Set Text`**  
   **Target** = resultado do **GET** em **`SlotPriceTexts`** no índice **`SlotIndex`**. **In Text** = texto formatado a partir de **`PriceGold`** (**Conv Integer to Text** se for Integer).

6. **Menos de 10 labels na tela**  
   Se só existirem 4 textos (ou 4 entradas no **Make Array**), índice 4 em diante não tem onde desenhar. Alinhe a quantidade de slots e de labels.

**Teste rápido:** no início de **`ApplyPriceForSlot`**, **Print** `SlotIndex` e **`Is Valid`** no widget retornado por **`SlotPriceTexts[SlotIndex]`**. Se for inválido, o problema é array ou índice.

---

#### 3.7.6 `WBP_SetItemPrice` — **OnClicked** (`BTN_Cancel`)

```
OnClicked (BTN_Cancel) → [Remove from Parent] Target = Self
```

---

#### 3.7.7 Textos de preço na grade (`SlotPriceTexts`)

**`SlotPriceTexts`** é uma variável **só no Blueprint** da loja: uma lista com **10 referências** aos widgets **Text** (rótulos de preço). O C++ não cria isso. Você coloca os **Text** no **Designer**, dentro de **`Grid_SlotPrices`**, e grava as referências nessa lista (por exemplo na função **`InitializeSlotPriceGrid`**).

##### 3.7.7.1 Designer — `Grid_SlotPrices` e 10 textos

1. Abra **`WBP_PlayerStore`** e localize **`Grid_SlotPrices`** na hierarquia (§3.1). Pode ser **Uniform Grid Panel** ou **Grid Panel**.
2. Para cada um dos 10 preços:
   - Arraste da paleta um widget **Text** (Common → **Text**) para **dentro** de **`Grid_SlotPrices`**.
   - Ajuste **Row** e **Column** de cada um — veja o passo a passo em **§3.7.7.1.2** (é no **Details** de **cada** texto filho, não nas propriedades do painel vazio).
3. Renomeie na hierarquia. O guia usa **`Text_Price_0`** … **`Text_Price_9`** (nomes = **índice lógico do slot**, 0 a 9). Se você preferir **`Text_Price_1`** … **`Text_Price_10`**, tudo bem — continuam **filhos diretos** de **`Grid_SlotPrices`**. No **`Make Array`** de **`InitializeSlotPriceGrid`**, o pin **`[0]`** precisa apontar para o texto do **slot 0** (no seu caso seria **`Text_Price_1`**), o **`[1]`** para o slot 1 (**`Text_Price_2`**), …, o **`[9]`** para o slot 9 (**`Text_Price_10`**). O que não pode é embaralhar a ordem: **`DraftPriceGold[i]`** e **`SlotPriceTexts[i]`** são sempre o **slot índice `i`**.
4. Em cada **Text**: marque **Is Variable**; texto inicial pode ser **"—"** ou vazio; alinhe ao centro se quiser.

**Dica:** a grade de slots e a grade de preços devem usar a **mesma** lógica de linhas/colunas (§3.5 e tabela abaixo), para o índice **i** do item bater com o índice **i** do preço.

##### 3.7.7.1.1 Grade 2 × 5 (igual aos seus prints)

A loja usa **10 slots** em **2 linhas × 5 colunas**, como no **§3.5** (use **5** como número de colunas no loop).

**Na tela:** no **Vertical Box**, coloque **`Grid_StoreSlots`** (itens) **acima** e **`Grid_SlotPrices`** (preços) **logo abaixo**, para cada coluna ficar alinhada (item em cima, valor embaixo).

**Fórmula** para o índice do slot **`i`** de **0** a **9**:

- **Linha (Row)** = divisão inteira de **`i` por 5** → linha **0** para `i` de 0 a 4; linha **1** para `i` de 5 a 9.  
- **Coluna (Column)** = resto de **`i` dividido por 5** → sempre entre **0** e **4**.

**Tabela — nome do widget, linha, coluna e posição no `Make Array`:**

Use a mesma **Row** / **Column** no **`Create Store Slots`** (§3.5) e aqui no **Designer**. O índice **`i`** é o mesmo de **`Store Slot Index`** / **`StoreSlotWidgets[i]`** e do pin **`[i]`** do **Make Array** em **`InitializeSlotPriceGrid`**.

| Ordem na tela | Índice `i` | Nome sugerido | Row | Column | Pin do Make Array |
|---------------|------------|---------------|-----|--------|-------------------|
| 1º (canto superior esquerdo) | 0 | `Text_Price_0` | 0 | 0 | `[0]` |
| 2º | 1 | `Text_Price_1` | 0 | 1 | `[1]` |
| 3º | 2 | `Text_Price_2` | 0 | 2 | `[2]` |
| 4º | 3 | `Text_Price_3` | 0 | 3 | `[3]` |
| 5º | 4 | `Text_Price_4` | 0 | 4 | `[4]` |
| 6º | 5 | `Text_Price_5` | 1 | 0 | `[5]` |
| 7º | 6 | `Text_Price_6` | 1 | 1 | `[6]` |
| 8º | 7 | `Text_Price_7` | 1 | 2 | `[7]` |
| 9º | 8 | `Text_Price_8` | 1 | 3 | `[8]` |
| 10º (canto inferior direito) | 9 | `Text_Price_9` | 1 | 4 | `[9]` |

**Largura das colunas:** nos dois **Uniform Grid Panel** (slots e preços), use o **mesmo** valor de **Min Desired Slot Width** quando fizer sentido, para as colunas ficarem uma em cima da outra.

**Opcional:** no **Vertical Box**, o slot do **`Grid_SlotPrices`** pode ser **Fill** e **Centralizado** na horizontal. O painel **`Grid_SlotPrices`** pode estar **Not Hit-Testable (Self Only)** para o clique ir para o slot de item, não ficar preso no texto.

##### 3.7.7.1.2 Onde mudar **Row** e **Column** (Unreal 5, modo Design)

A palavra **“Slot”** aqui é o **bloco de opções no painel Details** que aparece quando você seleciona **um texto filho** da grade. **Não** aparece quando só a grade **`Grid_SlotPrices`** está selecionada.

1. Abra **`WBP_PlayerStore`** no modo **Design** (aba **Design** no editor do widget — **não** o **Graph**).
2. Na **Hierarchy**, expanda **`Grid_SlotPrices`**.
3. Clique em **um** dos filhos **Text** (ex.: **`Text_Price_0`**). Tem que ser o **Text**, não o painel da grade.
4. No **Details** (menu **Window → Details** se não estiver visível), procure a seção **Slot** (pode aparecer como **Uniform Grid Slot**).
5. Ajuste **Row** e **Column** conforme a tabela acima.
6. Repita para os outros nove textos.

**Se não achar “Slot”:** confira se o **Text** é filho direto de **`Grid_SlotPrices`**. Use a caixa de busca do **Details** e digite **Row**. Se o texto estiver com **pai errado**, arraste na **Hierarchy** para dentro de **`Grid_SlotPrices`**.

**Resumo:** **Row** e **Column** ficam no **Details** de **cada** widget **Text** filho, na seção **Slot**.

##### 3.7.7.2 Variável **`SlotPriceTexts`** no Blueprint

1. **My Blueprint → Variables → +**
2. Nome: **`SlotPriceTexts`**
3. Tipo: **Array** → elemento = **Object Reference** → classe **Text** (widget de interface, **não** o tipo “texto” de variável comum). Se o editor mostrar **Text Block**, pode usar também.
4. **Compile** e **Save**. A lista começa vazia até você preencher (por exemplo na função abaixo).

##### 3.7.7.3 Função **`InitializeSlotPriceGrid`**

**Para que serve:** guardar na variável **`SlotPriceTexts`** as dez referências aos **Text** que você já colocou no **Designer** e colocar **"—"** (ou outro placeholder) em cada um. Isso **não** cria os slots de item; isso continua no **§3.5** (**`Create Store Slots`**).

**Onde criar:** **`WBP_PlayerStore`** → **Functions** → **+** → nome **`InitializeSlotPriceGrid`** (sem parâmetros).

**Quando chamar:** no **Event Construct**, de preferência **depois** de **`Create Store Slots`**, com os dez **Text** já na hierarquia.

```
[Function Entry] InitializeSlotPriceGrid
  → [Make Array]  pins [0]…[9] ligados a [Get] Text_Price_0 … Text_Price_9  (ou Text_Price_1 … Text_Price_10 alinhados ao slot 0…9; ver §3.7.7.1)
  → [Set SlotPriceTexts]  Target = Self; Slot Price Texts = saída do Make Array
  → [For Loop] 0 até 9 (Index = i)
        → [Get] SlotPriceTexts → [GET] (a copy) com Index = i
        → [Set Text]  Target = saída do GET; In Text = "—"
  → [Return Node]
```

**No `Event Construct`:**

```
[Event Construct] → [Create Store Slots] → [Initialize Slot Price Grid] (Target = Self) → …
```

Se os widgets tiverem **outros nomes**, mantenha a **ordem** [0] = primeiro slot, …, [9] = décimo, igual ao **`Store Slot Index`**.

##### 3.7.7.3.1 Um “—” só ou textos um em cima do outro

- **Make Array** só monta a lista no Blueprint; **não** posiciona nada na tela. Posição = **Row** / **Column** no **Designer** (§3.7.7.1.2 e tabela §3.7.7.1.1).
- Se todos os **Text** ficarem com **Row = 0** e **Column = 0**, os dez ficam **no mesmo lugar** e você só vê **um** traço. Corrija **Row** e **Column** na tabela.
- Se o pin **`[0]`** do **Make Array** **não** for o texto do **primeiro slot** (confusão entre nome `Text_Price_1` e pin `[0]`, ou ordem trocada), o preço aparece na coluna errada. **Ou** alinhe **`[0]`…`[9]`** com **`Text_Price_0`…`Text_Price_9`**, **ou** use **`Text_Price_1`…`Text_Price_10`** com **`[0]`→`Text_Price_1`**, …, **`[9]`→`Text_Price_10`** (§3.7.7.1).

**Se criar filhos no runtime** com **Add Child to Uniform Grid**, ligue **Row** e **Column** a **`i / 5`** e **`i % 5`**. Se deixar sempre **0** e **0**, o problema é o mesmo.

##### 3.7.7.4 Criar preços só em tempo de execução (opcional)

Só precisa se **não** quiser **Text** fixos no **Designer**. No Blueprint costuma-se usar um widget pequeno (**`WBP_PriceUnderSlot`**) com um **Text** dentro: **Clear Children** em **`Grid_SlotPrices`**, loop **0…9**, **Create Widget**, calcular **Row** = `i / 5` e **Column** = `i % 5`, **Add Child to Uniform Grid**, pegar o **Text** de dentro do widget criado e guardar em **`SlotPriceTexts[i]`**.

Na prática, **dez Text no Designer** (§3.7.7.3) é o mais simples.

##### 3.7.7.5 `Event Construct` sem função separada

Você pode copiar o corpo de **`InitializeSlotPriceGrid`** direto no **Construct**, mas chamar a **função** deixa o gráfico mais limpo.

##### 3.7.7.6 **`Set Text` dentro de `ApplyPriceForSlot`**

Use o elemento **`SlotPriceTexts[SlotIndex]`** como **Target** do **Set Text**. O **Set Array Elem** é **só** para o array numérico **`DraftPriceGold`**, não para trocar o texto na tela.

##### 3.7.7.7 Sem lista: só dez variáveis **`Text_Price_0`…`9`**

Use **`Switch on Int`** no **`SlotIndex`** e em cada saída um **Set Text** no **`Text_Price_N`** correspondente.

##### 3.7.7.8 Quando o preço volta a zero

Depois de cancelar oferta ou limpar slot, você pode voltar o rótulo para **"—"** ou esconder o **Text**. O valor numérico em rascunho continua em **`DraftPriceGold`** até o fluxo do **§3.8** (**Confirm Offer** / loja no servidor).

### 3.8 `BTN_Confirm_Offer`

**Onde:** **`WBP_PlayerStore`** → **Designer** → botão **`BTN_Confirm_Offer`** → **On Clicked** → **+** (abre o evento no **Graph**).

**Variáveis usadas (§3.3):** **`SelectedSlotIndex`**, **`StoreSlotWidgets`**, **`DraftPriceGold`**, **`ConfirmedInventoryID`**, **`ConfirmedPriceGold`**, **`SlotHasOffer`**, hierarquia **`HBox_SellItemOptions`**.

Se você preferir manter **`PendingPriceEditable`** para debug, pode ignorar; o fluxo oficial do guia passa a ser **DraftPriceGold** + **`SlotPriceTexts`**.

#### 3.8.1 Cadeia completa de nós (exec = fio branco)

Nomes entre **aspas** são os que costumam aparecer no editor (inglês ou PT); o pin de execução de saída do **OnClicked** costuma chamar-se **`then`**.

```
[OnClicked (BTN_Confirm_Offer)]  (saída exec "then")
  → [Branch]  “Índice selecionado válido?”
        Condition  ←  [Integer Greater or Equal to Integer]  (ou ≥ int)
              A  ←  [Get] SelectedSlotIndex
              B  ←  literal Integer  0
        False (exec)  →  [Print String]  "BTN_Confirm_Offer: SelectedSlotIndex < 0"
                          →  [Return Node]
        True (exec)   →  [Branch]  “Slot tem item?”
              Condition  ←  [Integer Greater than Integer]  (int > int)
                    A  ←  pin Inventory ID  (ver §3.8.2 abaixo)
                    B  ←  literal Integer  0
              False (exec)  →  [Print String]  "BTN_Confirm_Offer: slot sem item (Inventory ID)"
                                  →  [Return Node]
              True (exec)   →  [Branch]  “Preço em rascunho > 0?”
                    Condition  ←  [Integer Greater than Integer]  OU  [Int64 Greater than Int64]
                          A  ←  pin “preço rascunho”  (ver §3.8.2)
                          B  ←  literal 0 (mesmo tipo que A)
                    False (exec)  →  [Print String]  "BTN_Confirm_Offer: defina preço > 0 (modal §3.7)"
                                        →  [Return Node]
                    True (exec)   →  [Set Array Elem]  Target = ConfirmedInventoryID
                                          Index  ←  [Get] SelectedSlotIndex
                                          Item   ←  mesmo Inventory ID do §3.8.2
                                          (exec out)  →
                                    [Set Array Elem]  Target = ConfirmedPriceGold
                                          Index  ←  [Get] SelectedSlotIndex
                                          Item   ←  mesmo valor de DraftPriceGold do §3.8.2
                                          (exec out)  →
                                    [Set Array Elem]  Target = SlotHasOffer
                                          Index  ←  [Get] SelectedSlotIndex
                                          Item   ←  literal Boolean  true
                                          (exec out)  →
                                    [Set Visibility]  Target = HBox_SellItemOptions
                                          Visibility = Collapsed  (ou “Recolhido”)
                                          (exec out)  →
                                    [Set] SelectedSlotIndex  =  literal Integer  -1
                                          (exec out)  →  [Return Node]  (opcional)
```

**Observações de execução**

- Os três **`Set Array Elem`** devem estar **em sequência** no **mesmo** fio branco (**exec** do primeiro → **exec** do segundo → …). Não deixe só pins de dados sem encadear **exec**.
- **`Return`** nos ramos de erro evita confirmar com **índice** ou **preço** inválidos.

#### 3.8.2 Pins de dados (fios coloridos) — Inventory ID e preço rascunho

Ligue **uma vez** (ou replique os **Get** com o mesmo índice) para alimentar os **Branch** e os **Set Array Elem**:

1. **Inventory ID (para o Branch “slot tem item?” e para `ConfirmedInventoryID`)**  
   - **[Get] StoreSlotWidgets** → do pin do array → **[GET (a copy)]** — **Array Index** ← **[Get] SelectedSlotIndex**.  
   - No **Return Value** do **GET** → se o tipo permitir, **[Get Slot Data]** (função do **Umbra Inventory Slot Widget**). Se **não** aparecer, **Cast to Umbra Inventory Slot Widget** (ou à sua **WBP** do slot) e **Get Slot Data** na saída **As …**.  
   - Do **Return Value** de **Get Slot Data** (struct **Umbra Inventory Slot**) → arraste o pin **Inventory ID** (ou **Break** no struct e use **Inventory ID**).

2. **Preço rascunho (para o Branch “> 0?” e para `ConfirmedPriceGold`)**  
   - **[Get] DraftPriceGold** → **[GET (a copy)]** — **Array Index** ← **[Get] SelectedSlotIndex**.  
   - O **Return Value** desse **GET** liga no **A** do comparador **> 0** e no pin **Item** do **Set Array Elem** de **`ConfirmedPriceGold`** (mesmo valor, mesmo tipo **Integer** ou **Int64** que o array).

**Reutilizar pins:** o **Inventory ID** pode sair de **um** único **Get Slot Data** para o **Branch** e para o **Item** do primeiro **Set Array Elem**; o preço, de **um** único **GET** em **DraftPriceGold** para o **Branch** e para o segundo **Set Array Elem**.

#### 3.8.3 Resumo em uma linha (referência rápida)

**OnClicked** → **Branch** (SelectedSlotIndex ≥ 0) → **Get Slot Data** no slot **[SelectedSlotIndex]** → **Branch** (Inventory ID > 0) → **GET DraftPriceGold[SelectedSlotIndex]** → **Branch** (preço > 0) → três **Set Array Elem** (**ConfirmedInventoryID**, **ConfirmedPriceGold**, **SlotHasOffer**) → **Set Visibility** (**HBox_SellItemOptions** = **Collapsed**) → **Set SelectedSlotIndex** = **-1**.

#### 3.8.4 Troubleshooting — `StoreSlotWidgets` com **length 0** e `CallFunc_Array_Get_Item` **None**

**Sintomas no log (PIE):**  
`Attempted to access index 0 from array 'StoreSlotWidgets' of length 0`  
`Acessado "None" … CallFunc_Array_Get_Item`  
e o **Print** `BTN_Confirm_Offer: slot sem item (Inventory ID)` (o **Branch** de Inventory ID recebe **0** porque o **GET** no array falhou ou devolveu referência inválida — **não** significa necessariamente que o slot visual está vazio).

**Causa:** o array **`StoreSlotWidgets`** **não** está sendo preenchido. Os **WBP_StoreSlot** podem existir na **`Grid_StoreSlots`** (Designer ou **Add Child**), mas **cada** instância precisa ser guardada com **`ADD`** (ou **Set** com índice) em **`StoreSlotWidgets`**, como no **§3.5** (passo **`ADD (StoreSlotWidgets) = widget criado`** dentro do **For Loop**).

**O que fazer no Blueprint**

1. Abra **`Create Store Slots`** (ou o **Event Construct** onde você monta a grade). Verifique se **depois** de **`Create Widget`** / **`Add Child to Uniform Grid`** existe **`ADD`** na variável **`StoreSlotWidgets`** com o **mesmo** widget criado (return do **Create Widget**).
2. Se você usa **slots fixos no Designer**: **§3.5 Modo B** — **sem** **Clear Children** na grade; **Clear** no array; loop **Get Child at Index** + **Cast** + **Set Store Slot Index** + **ADD**.
3. Garanta que **não** há um **`Clear (StoreSlotWidgets)`** em outro lugar (ex.: ao abrir a loja) **sem** voltar a encher o array.
4. **Teste rápido:** no **Construct**, após montar a grade, **Print String** com **`Length`** de **`StoreSlotWidgets`** — tem que sair **10** antes de clicar em **Confirm Offer**.

**Defesa opcional no `BTN_Confirm_Offer`:** antes do **GET** em **`StoreSlotWidgets`**, **Branch** com **Condition** = **`Length (StoreSlotWidgets) > SelectedSlotIndex`** **e** `SelectedSlotIndex >= 0`; se **False**, **Print** “StoreSlotWidgets não preenchido” e **Return**. Isso evita o erro de execução, mas **só** o passo 1–2 acima corrige o comportamento.

#### 3.8.5 Troubleshooting — `Attempted to set an invalid index` em **`ConfirmedInventoryID [0/0]`** (e iguais)

**Significado do `[0/0]`:** o array tem **comprimento 0**. O **Confirm Offer** chama **`Set Array Elem`** com **Index** **0** (ou outro), mas **não existe posição** — daí o aviso em **`ConfirmedPriceGold`**, **`SlotHasOffer`**, etc.

**Correção:** no **`WBP_PlayerStore`**, **Class Defaults** → para **`ConfirmedInventoryID`**, **`ConfirmedPriceGold`**, **`SlotHasOffer`** e (recomendado) **`DraftPriceGold`**, defina **10 entradas** cada (valores iniciais **0** / **false**). Alternativa menos boa: no **`Set Array Elem`**, marque **Size to Fit** para o motor expandir o array — ainda assim é melhor começar com **10** fixos para bater com os **10** slots.

**Sobre o log C++ `CreateItemDragOperation - Slot: 0`:** esse número é **`SlotData.SlotIndex`** do **item** (ex.: posição na mochila), **não** o **`Store Slot Index`** da grade da loja. Arrastar do **primeiro** quadrado do inventário costuma mostrar **0** mesmo que o destino seja o **segundo** slot visual da loja. Para depurar a loja, use **`Print`** com **`SelectedSlotIndex`** e com **`Store Slot Index`** do **`WBP_StoreSlot`** depois do drop.

### 3.9 `BTN_Cancel_Offer`

```
OnClicked (BTN_Cancel_Offer)
  → Limpar slot selecionado (ClearSlot) e arrays naquele índice se aplicável
  → Collapsed (HBox_SellItemOptions)
  → SelectedSlotIndex = -1
```

### 3.10 `BTN_Start_Store` — `Open Personal Shop`

```
OnClicked (BTN_Start_Store)
  → ShopNameText → Get Text → String (trim, não vazio)
  → Set Text (Text_Vendor_Name) coerente com o nome
  → Montar ListingsToSend: For 0..9, se SlotHasOffer[i] → Make Personal Shop Listing Input (i, ConfirmedInventoryID[i], ConfirmedPriceGold[i])
  → Length >= 1 senão Print "Confirme ao menos uma oferta"
  → Cast Umbra Game Instance → Open Personal Shop (Shop Name, ListingsToSend)
```

A mudança **Start/Close** faz-se no delegate **`On Personal Shop Opened Local`** (assíncrono).

### 3.11 `On Personal Shop Opened Local`

```
Custom Event (Shop ID, Shop Name)
  → Set Visibility (HBox_Start_Store) = Collapsed
  → Set Visibility (HBox_Close_Store) = Visible
  → Set Visibility (HBox_SellItemOptions) = Collapsed
```

### 3.12 `BTN_Close_Store` — `Close Personal Shop`

```
OnClicked (BTN_Close_Store)
  → Cast Umbra Game Instance → Close Personal Shop (Shop ID = 0)
```

**`On Personal Shop Closed Local`:**

```
  → HBox_Start_Store Visible, HBox_Close_Store Collapsed
  → (opcional) reset grade / arrays e recarregar slots
```

### 3.13 `Btn_Close` (X)

**Feche** só a UI (Remove from Parent / ocultar). **Não** encerra a loja no servidor, a menos que você queira essa regra — nesse caso chame também **Close Personal Shop**.

---

**Notas:** **Open Personal Shop** já dispara WS **60** no C++. **`Grid_StoreSlots`** pode ser *Not Hit-Testable* se os filhos **`WBP_StoreSlot`** receberem o drop. **Inventory ID** no slot: igual ao **WBP_Trade** se necessário.

---

## 4. Nameplate / placa world-space — UpdatePersonalShopSign

O **UmbraGameInstance** chama por reflexão a função Blueprint:

- Nome exato: **`UpdatePersonalShopSign`**
- **UFUNCTION(BlueprintCallable)** no User Widget do `WidgetComponent`
- Parâmetros (nomes ajudam o preenchimento automático no `ProcessEvent` C++):
  - **`ShopName`** (String)
  - **`ShopId`** ou **`ShopID`** (Integer)
  - **`bVisible`** ou **`Visible`** (Boolean)

### 4.1 Implementação mínima no WBP do nameplate

```
Function UpdatePersonal Shop Sign (Shop Name, Shop Id, b Visible)
  → Set Text (TextBlock_ShopName) = Shop Name
  → Set Visibility (Overlay_PlacaLoja)
        Visible se b Visible == true
        Collapsed (ou Hidden) se false
```

Opcional: **`Set Is Enabled`** no botão da placa = `b Visible`.

### 4.2 Classe pai opcional

Pode criar `WBP_ShopSign` com parent class **Umbra Personal Shop Sign Widget** (`UUmbraPersonalShopSignWidget`) e implementar o evento **Update Personal Shop Sign** no grafo (override do Native Event).

---

## 5. WBP_LojaComprador — listar e comprar

### 5.1 Variáveis

| Nome | Tipo |
|------|------|
| `ListingSlots` | Array **Umbra Inventory Slot Widget** (10) |
| `PriceTexts` | Array **Text** (10) — mostrar `Price Gold` |
| `BuyButtons` | Array **Button** (10) |
| `CurrentSellerPlayerID` | Integer |

### 5.2 Ao abrir para um vendedor (ex.: após clique na placa)

```
(Custom Event) Open Shop For Seller (Seller Player ID : int)
  → Set CurrentSellerPlayerID = Seller Player ID
  → Get Game Instance → Cast to Umbra Game Instance
  → Load Personal Shop By Seller
        Seller Player ID = CurrentSellerPlayerID
  → (Widget Add to Viewport / Set Visibility Visible)
```

### 5.3 Quando **On Personal Shop State Loaded** disparar

Ligar no **Assign** do Game Instance:

```
Custom Event OnShopStateLoaded (Shop State : Personal Shop State)
  → Set Text (Title) = Shop State → Shop Name   [Break Personal Shop State]
  → Self (WBP_LojaComprador) → Update Personal Shop Buyer UI From State
        World Context Object = self
        Shop State = (pin)
        Listing Slot Widgets = ListingSlots
  → For Each Loop (Shop State → Listings) com índice i
        → Set Text PriceTexts[i] = Format Text "{0} gold" | Listing → Price Gold
        → Set Visibility BuyButtons[i] = Visible
  → (Para slots sem listing) Clear text / Collapsed button — loop 0..9 comparar com Listings.Num()
```

**Compra por slot:** ao preparar os botões, o jeito mais simples é guardar em **variável de array** `CurrentListingIDs` (10x int), preenchida no mesmo **For Each** com `Listing ID` de cada **Personal Shop Listing Entry**.

```
BuyButtons[i] → On Clicked
  → Get CurrentListingIDs[i]
  → (Branch) > 0?
        → Get Game Instance → Cast to Umbra Game Instance
        → Purchase Personal Shop Listing
              Listing ID = CurrentListingIDs[i]
```

### 5.4 Após compra bem-sucedida

**Assign On Personal Shop Purchase Completed:**

```
→ Get Game Instance → Load Inventory
→ (opcional) Load Personal Shop By Seller (mesmo Seller Player ID) para atualizar lista
→ Play Sound / Print "Comprado"
```

---

## 6. Abrir loja ao clicar na placa

Duas abordagens comuns (escolha uma):

### A) Botão dentro do widget world-space

No **WBP** do nameplate, o **Button** sobre a placa:

```
OnClicked (Button_Loja)
  → Get Owning Player Pawn   [ou Player Controller → Get Pawn]
  → (se precisar do Seller ID: usar Player State / replicated ID — o projeto pode ter **Get Active Player ID** só no local)
```

Para **personagem remoto**, o clique ocorre no **cliente que compra**: o **GameInstance** costuma saber o **Player ID** selecionado no **Player Selection** / **target**. Cadeia típica (se já existir no projeto para trade/inspect):

```
→ Get Game Instance → Cast to Umbra Game Instance
→ (função existente) Get Selected Player ID / Inspect Target ID
→ (Branch) > 0?
      → Create Widget (WBP_LojaComprador)
      → Open Shop For Seller (Selected ID)
```

Ajuste ao sistema real de **alvo** (raycast, `UmbraPlayerSelectionComponent`, etc.).

### B) Line trace + hit no Widget Component

1. **Get Player Controller** → **Get Hit Result Under Cursor by Channel** (ou Line Trace).  
2. Se **Hit Component** == **Widget Component**, obter **Widget** e **Cast** ao WBP da placa.  
3. Chamar **Custom Event** no WBP que execute a mesma lógica de **§6A**.

---

## 7. Bloqueio de movimento (vendedor local)

**Servidor** já rejeita **MoveUpdate** com loja aberta. No **cliente**, melhor UX:

**Assign On Personal Shop Opened Local:**

```
→ Get Player Character (0)
→ Cast to (seu Character BP, ex. Umbra Character)
→ Set Personal Shop Open (variável bool no Character) = true
→ Get Character Movement → Set Movement Mode = NONE
  [ou Disable Input no Player Controller]
```

**Assign On Personal Shop Closed Local:**

```
→ Set Personal Shop Open = false
→ Set Movement Mode = Walking (ou o padrão do jogo)
→ Enable Input
```

**Alternativa:** no **Character BP**, **Event Tick** ou **Enhanced Input**: **Branch** `Is Local Personal Shop Open` (chamar no Game Instance) → não processar movimento.

---

## 8. Delegates — ligações recomendadas

| Delegate | Ação sugerida |
|----------|----------------|
| **On Personal Shop Action Failed** | `Print String` / toast com `Error Message` |
| **On Personal Shop Opened Local** | Bloqueio movimento + **WBP_PlayerStore:** `HBox_Start_Store` Collapsed, `HBox_Close_Store` Visible |
| **On Personal Shop Closed Local** | Liberar movimento + **WBP_PlayerStore:** `HBox_Start_Store` Visible, `HBox_Close_Store` Collapsed |
| **On Remote Personal Shop Visual Updated** | Opcional: ícone no minimapa; o **C++** já chama **Update Remote Player Personal Shop Sign** |
| **On Personal Shop State Loaded** | Atualizar **WBP_LojaComprador** (**§5.3**) |
| **On Personal Shop Purchase Completed** | **Load Inventory**, atualizar gold na UI |

---

## 9. Checklist final

- [ ] SQL aplicado; `open` / `get` / `purchase` / `close` testados (curl ou Postman).  
- [ ] **`WBP_StoreSlot`** na grade; **`Parent Personal Shop Widget** = **Self** em cada slot criado pelo **`WBP_PlayerStore`**; opcional **`b Is Personal Shop Listing Slot** = **true** (§3.4–§3.5).
- [ ] **WBP_PlayerStore:** `Grid_StoreSlots` com 10 slots (§3.5); **`Grid_SlotPrices`** 2×5 alinhado (§3.7.7.1.1); **`InitializeSlotPriceGrid`** + **`SlotPriceTexts`** (§3.7.7.3); **Class Defaults:** arrays **`ConfirmedInventoryID`**, **`ConfirmedPriceGold`**, **`SlotHasOffer`**, **`DraftPriceGold`** com **10 elementos** cada (§3.3); drop → **`HBox_SellItemOptions`** visível; **BTN_Set_Price** abre **`WBP_SetItemPrice`** (§3.7); **Confirm Offer / Cancel** conforme §3.  
- [ ] **BTN_Start_Store** → **Open Personal Shop**; **`On Personal Shop Opened Local`** alterna **Start/Close** (§3.11).
- [ ] **BTN_Close_Store** → **Close Personal Shop**; **`On Personal Shop Closed Local`** repõe **Start/Close** (§3.12).
- [ ] Nameplate (widget do `WidgetComponent`) implementa **`UpdatePersonalShopSign`**.  
- [ ] **WBP_LojaComprador** usa **Update Personal Shop Buyer UI From State** + **Purchase Personal Shop Listing**.  
- [ ] Clique na placa resolve **Seller Player ID** e chama **Load Personal Shop By Seller**.  
- [ ] **On Personal Shop Opened/Closed Local** ligado ao **Character Movement** (ou input) e à UI da loja.  
- [ ] Todos os **Assign …** feitos sobre **Cast to Umbra Game Instance** válido (após login + personagem selecionado).

---

## Referências de código

- `UmbraGameInstance.h` / `.cpp` — categoria **Social \| Personal Shop**  
- `UmbraDataStructures.h` — structs **Personal Shop**  
- `NetMovementClient.cpp` — mensagens sociais **61**, **63**  
- `docs_main` — plano arquitetural original (fases API / zona / UE) se existir cópia local do plano Cursor  

---

*Documento gerado para implementação Blueprint da loja pessoal; manter alinhado às funções C++ após upgrades do módulo.*
