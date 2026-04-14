# Guia de implementação: mercado global (leilão / buyout) — UE 5.6.1

**Engine:** Unreal Engine **5.6.1** · **Código:** `UmbraEternumUE/`  
**Backend:** `www/umbra_api/api/auction/*.php` + `get_item_template_filters.php` · **BD:** `auction_listings`

A UI fica em **C++** (`UUmbraAuctionHouseWidget`, `UUmbraAuctionListingRowWidget`). O Blueprint só **parent class**, **hierarquia de widgets** e **chamadas** a funções `BlueprintCallable` (abas, OnDrop no slot de registro).

---

## 1) Pré-requisitos

| Passo | Ação |
|--------|------|
| MySQL | Executar [`www/umbra_api/scripts/create_auction_listings_table.sql`](www/umbra_api/scripts/create_auction_listings_table.sql) no banco `umbra_eternum` (após `players` e `player_inventory`). |
| API | Endpoints acessíveis pelo mesmo `ServerURL` / VaRest do login (Apache + symlink `umbra_api`). |
| JWT | Personagem selecionado no token (`player_id`), como na loja pessoal. |
| C++ | Recompilar o módulo `UmbraEternumUE` após adicionar os `.h/.cpp`. |

**Regras de negócio (servidor):**

- Anúncio **ativo** dura **72 horas** (`expires_at`); após isso, a API marca como `expired` (lazy cleanup em cada listagem).
- Máximo **30** anúncios ativos por personagem (`AUCTION_MAX_ACTIVE_LISTINGS_PER_PLAYER` em `helpers/auction_helper.php`).
- Item **não pode** estar na loja pessoal aberta ao criar anúncio.
- Compra: mesmo modelo da loja pessoal (ouro + inventário + `FOR UPDATE`).

---

## 2) Endpoints HTTP

| Arquivo | Método | Descrição |
|---------|--------|-----------|
| `api/auction/create_auction_listing.php` | POST JSON | `token`, `inventory_id`, `price_gold` |
| `api/auction/list_auction_listings.php` | GET | `token`, `page`, `page_size`, opcional `item_type`, `item_subtype`, `rarity`, `search` |
| `api/auction/my_auction_listings.php` | GET | `token`, `page`, `page_size` |
| `api/auction/purchase_auction_listing.php` | POST JSON | `token`, `listing_id` |
| `api/auction/cancel_auction_listing.php` | POST JSON | `token`, `listing_id` |
| `api/inventory/get_item_template_filters.php` | GET | `token` — retorna `item_types`, `item_subtypes`, `rarities` |

**Exemplo POST criar anúncio:**

```json
{ "token": "<jwt>", "inventory_id": 12345, "price_gold": 5000 }
```

**Campos úteis nas listas:** cada linha inclui `expires_at_unix` (epoch segundos) para o cliente calcular tempo restante.

---

## 3) Cliente UE — classes C++

| Classe | Arquivo |
|--------|---------|
| Dados | [`UmbraDataStructures.h`](UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h) — `FUmbraAuctionListingEntry`, `FUmbraAuctionListingsPageResult`, `FUmbraAuctionTemplateFilters` |
| HTTP / delegates | [`UmbraGameInstance.h/.cpp`](UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h) — `CreateAuctionListing`, `RequestAuctionListings`, `RequestMyAuctionListings`, `PurchaseAuctionListing`, `CancelAuctionListing`, `RequestAuctionTemplateFilters`, `EnsureInventoryVisibleForAuctionHouse`, `CreateInventorySlotFromAuctionListing` |
| Painel | [`UmbraAuctionHouseWidget.h/.cpp`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraAuctionHouseWidget.h) |
| Linha | [`UmbraAuctionListingRowWidget.h/.cpp`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraAuctionListingRowWidget.h) |

**Delegates (Eventos|Auction):** `OnAuctionActionFailed`, `OnAuctionListingsPageLoaded`, `OnAuctionListingCreated`, `OnAuctionPurchaseCompleted`, `OnAuctionListingCancelled`, `OnAuctionTemplateFiltersLoaded`.

---

## 4) WBP_AuctionHouse (painel principal)

**Parent Class:** `UmbraAuctionHouseWidget`

**Class Defaults → Auction|Classes:** `Listing Row Widget Class` = `WBP_AuctionListingRow` (subclasse de `UmbraAuctionListingRowWidget`).

### 4.1 Hierarquia sugerida (visão geral)

- `Canvas` / `Overlay` raiz  
  - `VerticalBox` principal  
    - **Barra de abas:** `HorizontalBox` com botões (nomes reconhecidos pelo C++):  
      - `BTN_TabBrowse` · `BTN_TabRegister` · `BTN_TabMyListings` (fallback: `Btn_TabBrowse`, etc.)  
    - **`Switcher_Main`** — ver **§4.2** (é aqui que a maior parte do layout por aba vive).  
    - **Feedback global:** `TXT_AuctionInfo` ou `Text_AuctionInfo` (`UTextBlock`) — fora do switcher, sempre visível em todas as abas.  
    - `BTN_Close` / `Btn_Close` — fecha o painel (`RemoveFromParent`).

Os nomes acima são os que `TryResolveWidgetsByName` procura; se usar outros nomes, ajuste o C++ ou crie aliases no Designer com esses nomes.

### 4.2 `Switcher_Main` — Widget Switcher (detalhamento)

No Unreal, o **`Widget Switcher`** mostra **apenas um filho por vez**. O C++ usa o nome **`Switcher_Main`** e chama `SetActiveWidgetIndex(0|1|2)`. Por isso a **ordem dos filhos no painel** precisa ser exatamente a abaixo.

#### 4.2.1 Criar o switcher no UMG

1. No `WBP_AuctionHouse`, adicione um **`Widget Switcher`** (paleta *Panel* / busca por “Widget Switcher” — UE 5.6.x).  
2. Renomeie o widget para **`Switcher_Main`** (painel *Details* → *Name* — deve ser o nome da variável, não só o rótulo na árvore).  
3. Coloque-o no `VerticalBox` principal **abaixo** da barra de abas e **acima** de `TXT_AuctionInfo` (assim o texto de status fica sempre visível; se preferir o info no topo, inverta, mas mantenha o nome `TXT_AuctionInfo`).  
4. No slot do `VerticalBox` que contém o switcher: **Size** → *Fill* (ou peso 1) para o switcher ocupar a altura útil; use **Padding** mínimo para não colidir com a barra de abas.

#### 4.2.2 Três filhos = três abas (ordem = índice)

O switcher deve ter **exatamente 3 widgets filhos**, nesta ordem:

| Índice | Nome sugerido do filho (árvore) | Conteúdo |
|--------|---------------------------------|----------|
| **0** | `Panel_Browse` | Aba **Explorar** |
| **1** | `Panel_Register` | Aba **Registrar** |
| **2** | `Panel_MyListings` | Aba **Meus anúncios** |

**Regra:** o **primeiro** filho do `Widget Switcher` na hierarquia = índice **0**, o segundo = **1**, o terceiro = **2**. No editor, cada filho ocupa um **slot** do switcher; reordenar arrastando na árvore altera o índice. Se os índices ficarem errados, as abas abrirão o painel errado — realinhe ou ajuste o C++.

**Boas práticas:** cada índice deve ser **um único root** por slot (recomendado: `Border` ou `VerticalBox` nomeado `Panel_Browse`, etc.), e **dentro** dele você monta o layout. Evite colocar dezenas de widgets soltos diretamente como filhos do switcher.

**Índice ativo ao abrir:** em *Details* do `Widget Switcher`, **Active Widget Index** pode ficar **0** (Explorar). O `NativeConstruct` do C++ também chama `ShowBrowseTab()`, que força o índice 0.

#### 4.2.3 Conteúdo de cada painel filho

**Índice 0 — `Panel_Browse` (Explorar)**

- `VerticalBox` (ou similar) contendo:  
  - `HorizontalBox` de filtros: `Combo_ItemType`, `Combo_ItemSubtype`, `Combo_Rarity` (`Combo Box String`).  
  - `EditableText_Search` ou `EditableTextBox_Search`.  
  - `HorizontalBox` com `BTN_ApplyFilters` e `BTN_RefreshBrowse`.  
  - `Scroll_BrowseListings` **ou** `Scroll_Box_Browse` — **deve preencher o espaço vertical restante** (slot *Fill* no `VerticalBox` pai do scroll).

**Índice 1 — `Panel_Register` (Registrar)**

- Layout com `Slot_RegisterItem` ou `Slot_Register` (parent `UmbraInventorySlotWidget`), campo de preço (`EditableText_Price` ou `EditableTextBox_Price`), `BTN_ConfirmRegister`, `BTN_ClearRegister`. Texto de ajuda opcional (`TextBlock` estático).

**Índice 2 — `Panel_MyListings` (Meus anúncios)**

- `BTN_RefreshMy` + `Scroll_MyListings` ou `Scroll_Box_MyListings` com slot *Fill* como na aba Explorar.

#### 4.2.4 Ligação com os botões de aba

Os botões **não** trocam o switcher pelo Blueprint se você já usa os nomes `BTN_Tab*`: o C++ faz `OnClicked` → `ShowBrowseTab` / `ShowRegisterTab` / `ShowMyListingsTab` → `CachedSwitcherMain->SetActiveWidgetIndex(...)`.

Se quiser **destaque visual** na aba ativa (cor/opacidade), faça no **Blueprint** com base no índice: por exemplo, ao clicar em cada aba, além da chamada nativa (se expuser um evento) ou repetindo a lógica, atualize o estilo dos três botões. O código C++ atual **não** altera o visual dos botões — só o índice do switcher.

#### 4.2.5 Resumo índice ↔ C++

| Índice | Conteúdo | Função C++ |
|--------|----------|------------|
| 0 | Explorar | `ShowBrowseTab()` → `SetActiveWidgetIndex(0)` |
| 1 | Registrar | `ShowRegisterTab()` → `SetActiveWidgetIndex(1)` |
| 2 | Meus anúncios | `ShowMyListingsTab()` → `SetActiveWidgetIndex(2)` + `RequestMyAuctionListings` |

#### 4.2.6 Árvore mínima (referência)

```
VerticalBox_Root
├── HBox_Tabs (BTN_TabBrowse, BTN_TabRegister, BTN_TabMyListings)
├── Switcher_Main                    ← UWidgetSwitcher
│   ├── Panel_Browse                 ← índice 0
│   │   └── … filtros + Scroll_BrowseListings
│   ├── Panel_Register               ← índice 1
│   │   └── … Slot_RegisterItem + preço + botões
│   └── Panel_MyListings             ← índice 2
│       └── … BTN_RefreshMy + Scroll_MyListings
├── TXT_AuctionInfo
└── BTN_Close
```

**Nota de layout:** as abas podem ficar **no rodapé** (como no seu `WBP_AuctionHouse`) ou no topo; o C++ só exige os **nomes** `BTN_TabBrowse`, `BTN_TabRegister`, `BTN_TabMyListings`. O `TXT_AuctionInfo` pode estar em um `HBox_AuctionInfo` — o importante é o nome do `TextBlock` ser `TXT_AuctionInfo` (ou `Text_AuctionInfo`).

#### 4.2.7 Combo Box String — nomes, dados e uso

O `UmbraAuctionHouseWidget` **não** usa `BindWidget`; ele localiza widgets com `GetWidgetFromName`. Por isso cada **Combo Box String** precisa ter **exatamente** este **nome de variável** na hierarquia (clique com o botão direito → *Rename*):

| Nome obrigatório no editor | Função |
|----------------------------|--------|
| **`Combo_ItemType`** | Filtro `item_type` na API |
| **`Combo_ItemSubtype`** | Filtro `item_subtype` |
| **`Combo_Rarity`** | Filtro `rarity` (valor como no banco: número ou texto, conforme seus templates) |

Se o nome for genérico (`ComboBoxString_111`), o C++ **não encontra** o controle: os filtros ficam sempre “vazios” na requisição (a listagem ainda funciona, mas sem filtro por combo).

**Detalhes no Designer**

1. Arraste **Combo Box String** da paleta (*Input* ou busca por “Combo Box String”).  
2. Renomeie para um dos três nomes da tabela (recomenda-se três combos lado a lado num `Horizontal Box`, dentro de `Panel_Browse`).  
3. Em *Details*, marque **Is Variable** se for manipular também no Blueprint (opcional para o fluxo atual — o C++ acha pelo nome na árvore).  
4. **Não** precisa preencher opções manualmente no editor: ao abrir o painel, o `GameInstance` chama `RequestAuctionTemplateFilters()` e, ao responder, o C++ executa `PopulateFilterComboBoxes()`, que:  
   - limpa cada combo encontrado;  
   - adiciona a primeira opção **`Todos`**;  
   - adiciona as strings vindas de `get_item_template_filters.php` (`item_types`, `item_subtypes`, `rarities`).  
5. **“Todos”** significa “não enviar esse filtro na URL” — `GetComboSelectionOrEmpty` trata `Todos` ou seleção vazia como filtro omitido.  
6. O jogador escolhe tipo/subtipo/raridade e pressiona **`BTN_ApplyFilters`** (ou **Atualizar** em `BTN_RefreshBrowse`, que também reaplica os mesmos combos + texto de busca). Não há `OnSelectionChanged` obrigatório no C++: a lista só atualiza quando um desses botões dispara `RequestAuctionListings`.

**Fluxo resumido**

```
Abrir WBP → NativeConstruct → RequestAuctionTemplateFilters + RequestAuctionListings
         → API devolve listas distintas → combos preenchidos
Usuário altera combos (+ opcional busca) → BTN_ApplyFilters → GET list_auction_listings com query params
```

#### 4.2.8 Checklist — hierarquia tipo `Canvas` → `Border_StoragePanel` → `VBox`

Estruturas como **título no topo** (`Border_TitleBar` + `Btn_Close`), **`VBox_Switcher`** envolvendo o `Switcher_Main`, **`HBox_AuctionInfo`** + `TXT_AuctionInfo`, e **abas no rodapé** (`BTN_Tab*`) estão **alinhadas ao planejado**, desde que:

| Item | Status típico |
|------|----------------|
| `Switcher_Main` com filhos `Panel_Browse` → `Panel_Register` → `Panel_MyListings` | Correto (índices 0, 1, 2). |
| `BTN_TabBrowse` / `Register` / `MyListings` | Nomes corretos para o C++. |
| `Btn_Close` | Resolvido como `BTN_Close` ou `Btn_Close`. |
| `TXT_AuctionInfo` | Nome correto. |
| Um único `ComboBoxString_111` | **Ajustar:** renomear para `Combo_ItemType` e **adicionar** mais dois `Combo Box String`: `Combo_ItemSubtype` e `Combo_Rarity`. |
| `Panel_Browse` só com combo | **Completar:** busca (`EditableTextBox_Search`), `BTN_ApplyFilters`, `BTN_RefreshBrowse`, `Scroll_BrowseListings` (Fill). |
| `Panel_Register` / `Panel_MyListings` | Conferir slot de registro, preço, botões e scroll conforme §4.2.3. |

### 4.3 Drag-and-drop no slot de registro

No **WBP** do slot (`WBP_StoreSlot` reutilizado ou cópia):

- No evento **On Drop** (ou cadeia que a loja pessoal já usa), após o slot receber o item, chame no **painel** `WBP_AuctionHouse`:

  **`Register Item Dropped in Register Slot`** → passar referência ao `UmbraInventorySlotWidget` do slot de registro.

Isso atualiza a mensagem em `TXT_AuctionInfo` (o C++ não substitui a lógica de drag; só reage ao callback).

### 4.4 Abrir o mercado (Blueprint)

- **Create Widget** `WBP_AuctionHouse` → **Add to Viewport**  
- Opcional: tecla/HUD chama só isso; toda a lógica HTTP está no `GameInstance`.

---

## 5) WBP_AuctionListingRow (linha da lista)

Cada linha é **instanciada em tempo de execução** pelo `WBP_AuctionHouse` (`RepopulateListingScroll`): o Designer define **um** asset `WBP_AuctionListingRow`; o C++ cria uma instância por anúncio e chama `SetListingData`. Não coloque linhas manualmente dentro do `ScrollBox` do painel — só o **modelo** da linha.

### 5.1 Configuração do asset

| Campo | Valor |
|--------|--------|
| **Parent Class** | `UmbraAuctionListingRowWidget` |
| **Nome do asset** | `WBP_AuctionListingRow` (ou outro; lembre de apontar no **Class Defaults** do `WBP_AuctionHouse` → *Listing Row Widget Class*) |

**Tamanho do root:** no `WBP_AuctionListingRow`, defina no **root** (ex. `Border_Row` ou `SizeBox_Row`) uma **altura** confortável para o slot — tipicamente **72–96 px** (ou o mesmo tamanho do `WBP_StoreSlot` / slot de inventário). Largura: **Fill** no `ScrollBox` pai (o scroll distribui a largura horizontal).

### 5.2 Hierarquia completa sugerida (Designer)

Use um **único eixo horizontal** para leitura esquerda → direita: ícone, textos empilhados, botão de ação.

```
Border_Row (ou Canvas / SizeBox como root)
└── HorizontalBox_HRow
    ├── Slot_Item                    ← User Widget, classe = mesmo WBP de slot do inventário
    ├── Spacer_8 (opcional, Size 8)
    ├── VerticalBox_Texts
    │   ├── Text_ItemName            ← UTextBlock (nome do item)
    │   ├── Text_SellerOrMeta        ← UTextBlock (vendedor + tipo / subtipo / R)
    │   ├── HorizontalBox_PriceTime (opcional)
    │   │   ├── Text_Price           ← ouro formatado
    │   │   ├── Spacer
    │   │   └── Text_TimeLeft        ← "02h 15m 30s" ou "Expirado"
    │   └── (opcional) SizeBox reservado
    ├── Spacer_Flex (Size: Fill, prioridade 1)   ← empurra o botão para a direita
    ├── BTN_Buy                      ← UButton ("Comprar")
    └── BTN_Cancel                   ← UButton ("Cancelar")
```

**Variação simples:** se preferir **uma coluna** só de textos sem `HorizontalBox_PriceTime`, deixe `Text_Price` e `Text_TimeLeft` como irmãos diretos de `VerticalBox_Texts` (nomeie-os igual — o C++ só exige os nomes na árvore).

### 5.3 Nomes obrigatórios (resolução no C++)

O código usa `GetWidgetFromName` em `TryResolveWidgetsByName` ([`UmbraAuctionListingRowWidget.cpp`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraAuctionListingRowWidget.cpp)). Use **exatamente** estes nomes na coluna *Name* da hierarquia (ou aliases abaixo):

| Nome no Designer | Tipo | Obrigatório | Função |
|------------------|------|-------------|--------|
| **`Slot_Item`** | `User Widget` com parent **`UmbraInventorySlotWidget`** | Recomendado | Ícone, quantidade, borda de raridade (mesmo WBP usado na loja/comprador). |
| **`Text_ItemName`** | `Text Block` | Recomendado | `Entry.ItemName` |
| **`Text_Price`** | `Text Block` | Recomendado | Ouro (`FormatGoldThousands`) |
| **`Text_TimeLeft`** | `Text Block` | Recomendado | Tempo até `expires_at_unix` ou "Expirado" |
| **`Text_SellerOrMeta`** | `Text Block` | Recomendado | Explorar: "Vendedor: … · tipo · subtipo · R#". Meus: só meta (tipo/subtipo/R). |
| **`BTN_Buy`** ou **`Btn_Buy`** | `Button` | Recomendado | Comprar; **Collapsed** na aba Meus ou se o vendedor for o jogador local. |
| **`BTN_Cancel`** ou **`Btn_CancelListing`** | `Button` | Recomendado | Cancelar; **Collapsed** na aba Explorar. |

Se um controle não existir ou o nome estiver errado, essa parte simplesmente não atualiza (sem crash); o slot e os botões são os mais importantes para a UX.

### 5.4 Slots UMG (alinhamento sugerido)

| Widget | Pai | Slot (Horizontal Box) | Size | Alinhamento vertical |
|--------|-----|------------------------|------|----------------------|
| `Slot_Item` | `HorizontalBox_HRow` | Auto | Largura/altura fixas iguais ao slot (ex. 64) | Center |
| `VerticalBox_Texts` | `HorizontalBox_HRow` | Fill (peso 1) | — | Fill / Top |
| `Spacer_Flex` | `HorizontalBox_HRow` | Fill | peso 1 | — |
| `BTN_Buy` / `BTN_Cancel` | `HorizontalBox_HRow` | Auto | Padding 4–8 | Center |

**Dica:** `BTN_Buy` e `BTN_Cancel` podem ocupar a **mesma** posição visual (direita): um fica *Visible* e o outro *Collapsed* conforme o modo — não é necessário dois slots flex distintos.

### 5.5 Comportamento em tempo de execução (sem Blueprint extra)

| Modo | Origem | `BTN_Buy` | `BTN_Cancel` |
|------|--------|-----------|--------------|
| Explorar | `RepopulateListingScroll(..., bMyListingsMode=false)` | Visível se `SellerPlayerID != LocalPlayerId` | Collapsed |
| Meus anúncios | `bMyListingsMode=true` | Collapsed | Visível |

**Cliques:** o C++ registra `OnClicked` em `NativeConstruct` → `PurchaseAuctionListing` / `CancelAuctionListing` no `GameInstance`. Não precisa ligar eventos no Graph do `WBP_AuctionListingRow`, desde que os nomes dos botões batam.

### 5.6 `Slot_Item` — classe do User Widget

1. **Palette** → **User Widget** dentro de `HorizontalBox_HRow`.  
2. **Widget Class** = o mesmo asset usado nas listas da loja (ex. `WBP_StoreSlot` / slot de inventário com parent `UmbraInventorySlotWidget`).  
3. Renomeie para **`Slot_Item`**.  
4. Em `RefreshDisplay`, o C++ monta um `FUmbraInventorySlot` via `UUmbraGameInstance::CreateInventorySlotFromAuctionListing` e chama `SetSlotData` + `RefreshSlotDisplay` (se existir no Blueprint do slot).

### 5.7 Estilo visual (opcional)

- **Raridade:** use cores de texto ou borda no `Text_ItemName` / slot (pode ser Blueprint no `WBP` do slot, não na row).  
- **Preço:** prefixo opcional "Ouro: " pode ser texto estático irmão ou prefixo no próprio `Text_Price` via fonte maior.  
- **Botões:** textos "Comprar" / "Cancelar" no *Content* do `Button` no Designer.

### 5.8 Ligação com `WBP_AuctionHouse`

No **Class Defaults** de `WBP_AuctionHouse` → **Auction | Classes** → **Listing Row Widget Class** = `WBP_AuctionListingRow`. O `Scroll_BrowseListings` / `Scroll_MyListings` permanece vazio no Designer; em jogo, o C++ adiciona filhos `WBP_AuctionListingRow` dinamicamente.

### 5.9 Árvore mínima (copiar/colar referência)

```
Border_Row
└── HorizontalBox_HRow
    ├── Slot_Item
    ├── VerticalBox_Texts
    │   ├── Text_ItemName
    │   ├── Text_SellerOrMeta
    │   ├── Text_Price
    │   └── Text_TimeLeft
    ├── Spacer_Flex
    ├── BTN_Buy
    └── BTN_Cancel
```

---

## 6) Fluxo de teste (checklist)

1. Login + personagem ativo; abrir `WBP_AuctionHouse`.  
2. Aba **Explorar:** lista carrega; filtros + **Aplicar** alteram a query.  
3. Aba **Registrar:** inventário visível (`EnsureInventoryVisibleForAuctionHouse`); arrastar item; preço; **Confirmar** → mensagem de sucesso + lista atualizada.  
4. Aba **Meus anúncios:** vê o anúncio; **Cancelar** remove da lista ativa.  
5. Outro personagem: **Comprar** → ouro e inventário atualizados (`LoadInventory` no `GameInstance`).  
6. Simular expiração: no MySQL, `UPDATE auction_listings SET expires_at = NOW() - INTERVAL 1 HOUR WHERE listing_id = ?` e chamar listagem → status `expired` após lazy cleanup.  
7. Item na loja pessoal: criar anúncio deve falhar com mensagem da API.

---

## 7) Manutenção

- **Cron (opcional):** além do lazy cleanup, um evento agendado pode executar o mesmo `UPDATE` de expiração para manter a tabela limpa.  
- **Limite de anúncios:** constante em `www/umbra_api/helpers/auction_helper.php`.  
- **Zona / WebSocket:** não obrigatório para o mercado global na fase atual (somente HTTP).

---

## 8) Referência rápida — funções Blueprint

| Função | Onde |
|--------|------|
| `Show Browse Tab` | Painel |
| `Show Register Tab` | Painel |
| `Show My Listings Tab` | Painel |
| `Register Item Dropped in Register Slot` | OnDrop → Painel |
| `Try Confirm Register Listing` | Botão confirmar (ou já ligado no C++) |
| `Clear Register Slot` | Botão limpar |
| `Request Refresh Browse` | Botão atualizar explorar |
| `Request Refresh My Listings` | Botão atualizar meus |

O painel já liga os botões padrão no `NativeConstruct` se os nomes `BTN_*` acima existirem.

**Nota:** o carregamento dos filtros (`Combo_*`) é silencioso; o texto de `TXT_AuctionInfo` após abrir o painel reflete sobretudo o resultado da listagem (explorar) ou das suas ações.

---

## 9) Solução de problemas (slot invisível, ComboBox “não faz nada”)

### 9.1 Não aparece área do slot de registro (`Slot_RegisterItem`)

1. **Aba correta** — O slot fica em **`Panel_Register`** (índice **1** do `Switcher_Main`). Só aparece depois de clicar em **Register Item** / `BTN_TabRegister`. Na aba **Browse** o `Switcher_Main` mostra `Panel_Browse`, não o registro.

2. **Tamanho zero no layout** — `UmbraInventorySlotWidget` com root em **Canvas** sem tamanho fixo pode reportar **Desired Size 0×0** dentro de um `Vertical Box` com slot **Auto**: o slot “some”.  
   - **Designer:** envolva `Slot_RegisterItem` num **`Size Box`**, marque **Width Override** e **Height Override** (ex.: **72** ou **80**), ou defina tamanho mínimo no próprio WBP do slot (root com tamanho explícito).  
   - **Slot no `Vertical Box`:** use **Auto** no filho `Size Box` (recomendado), não **Fill** com peso 0 em cadeias estranhas.

3. **Classe do User Widget** — O nó deve ser um **User Widget** cuja **Parent Class** é **`UmbraInventorySlotWidget`** (ex.: o mesmo `WBP_StoreSlot` da loja). Se for outra classe, o C++ não preenche `CachedRegisterSlot` e o registro não funciona (veja **Output Log** por aviso `[UmbraAuctionHouseWidget] Slot_RegisterItem...`).

4. **`Accepted Item Types` vazio** — No código, lista vazia **aceita todos** os tipos (`ValidateItemType`). Não é a causa de bloqueio de drop.

5. **C++ (build recente)** — O painel força slot **Visible**, `InvalidateLayout` em `Panel_Register` e, se o slot estiver num `VerticalBoxSlot`, tende a usar regra **Automatic** para evitar colapso.

6. **Espelho + visual vazio (drop no leilão)** — O slot de registro precisa do mesmo “modo espelho” da loja pessoal (`ConfigurePersonalShopListingSlot`), senão o `ProcessItemDrop` tenta **mover** o item pela API em vez de só mostrar no widget. O painel agora configura isso ao resolver `Slot_RegisterItem`, chama `ClearSlot` + `TryCallBlueprintUpdateSlotVisual` para o **WBP do slot** desenhar o estado vazio (borda/ícone). No drop, o `UmbraInventorySlotWidget` notifica o `UmbraAuctionHouseWidget` via `Register Item Dropped in Register Slot`. **Recompile** o C++ após puxar as alterações.

7. **Size Box “Not Hit-Testable (Self Only)”** — Em geral o filho recebe o drop; se não receber, teste o **Size Box** como **Visible** (hit-test visível).

### 9.2 Combo Box String: clique não abre lista ou “nada acontece”

1. **Comportamento esperado** — Clicar no combo **só abre a lista** para escolher **Tipo / Subtipo / Raridade**. A listagem do mercado **não** atualiza sozinha ao mudar a seleção: use **`BTN_ApplyFilters`** ou **`BTN_RefreshBrowse`** para disparar o HTTP com os filtros atuais.

2. **Lista vazia (só “Todos”)** — Se a API `get_item_template_filters.php` falhar (token, URL, Apache) ou retornar arrays vazios, cada combo fica só com **“Todos”**. Abrir o combo mostra uma linha só — pode parecer que “nada acontece”. Confira **Output Log** (VaRest / mensagem em `TXT_AuctionInfo` se `OnAuctionActionFailed` for usado) e teste a URL no navegador/curl com o mesmo token.

3. **Widget cobrindo os combos** — Muito comum: uma **imagem de fundo** ou `Border` em **Panel_Browse** com **Visibility = Visible** e recebendo **hit-test** cobre os combos. Ajuste o fundo para **`Self Hit Test Invisible`** ou **`Hit Test Invisible`** (o desenho continua, mas o clique “passa” para os filhos / combos).

4. **Nomes (recomendado) + aliases (fallback)** — O ideal continua sendo **`Combo_ItemType`**, **`Combo_ItemSubtype`**, **`Combo_Rarity`**.  
   O C++ agora também tenta aliases (`ComboBox_*`, `CB_*`, etc.) e fallback por ordem dos `ComboBoxString` no `WidgetTree`, mas padronizar os nomes evita binding errado.

5. **Aba Browse** — Os combos estão em `Panel_Browse`. Se o switcher estiver na aba **Registrar**, os combos não aparecem — volte em **Browse Items** para testá-los.

### 9.3 `WBP_AuctionListingRow` — linha aparece, mas textos/slot não atualizam

- O C++ da row procura primeiro nomes recomendados (`Slot_Item`, `Text_ItemName`, `Text_Price`, `Text_TimeLeft`, `Text_SellerOrMeta`, `BTN_Buy`, `BTN_Cancel`) e depois aliases/fallback por tipo.
- Se sua linha mostra placeholders (ex.: “Bloco de Texto”), o binding de textos falhou parcialmente.  
  Verifique o Output Log para:
  - `[UmbraAuctionListingRowWidget] Text refs incompletos...`
  - `[UmbraAuctionListingRowWidget] Binding refs: ...`
- O refresh do slot da row agora usa `TryCallBlueprintUpdateSlotVisual()` (compatível com `UpdateSlotVisual`, `RefreshSlotDisplay`, `UpdateStoreSlot`).

Se no **Horizontal Box** o `Slot_Item` estiver com slot **Fill** competindo com outros **Fill**, a largura útil pode ir a zero. O C++ da linha força **Automatic** no `UHorizontalBoxSlot` do `Slot_Item` após o construct; ainda assim, prefira **Size Box** ao redor do slot ou slot **Auto** no Designer.

### 9.4 Checklist rápido de runtime (Browse / Register / My Listings)

1. **Bindings do painel**
   - Procure no log:  
     - `[UmbraAuctionHouseWidget] Binding refs: BrowseScroll=... MyScroll=... RegisterSlot=... Combos=...`
     - avisos de scroll/combos ausentes.
2. **Rows adicionadas no scroll correto**
   - Procure no log:  
     - `[UmbraAuctionHouseWidget] RepopulateListingScroll(BROWSE|MY): listings=X rowsAdded=Y ...`
   - Se `listings > 0` e `rowsAdded = 0`, normalmente é classe de row não configurada ou `CreateWidget` falhando.
3. **Filtros carregados**
   - Procure no log:  
     - `[UmbraAuctionHouseWidget] Filtros recebidos: types=... subtypes=... rarities=...`
     - `[UmbraAuctionHouseWidget] Combo '...' carregado com ... opções.`
4. **Aplicação de filtros**
   - Ao clicar em `BTN_ApplyFilters`/`BTN_RefreshBrowse`, procure:
     - `[UmbraAuctionHouseWidget] RequestRefreshBrowse filtros: type='...' subtype='...' rarity='...' search='...'`
   - Isso confirma que o clique foi capturado e os valores selecionados foram enviados para a API.

---
