# Plano de implementação (nós literais) — loja pessoal UI C++

Plano **passo a passo no Unreal Editor** para alinhar os assets ao código nativo (`UUmbraPlayerStoreWidget`, `UUmbraSetItemPriceWidget`, `UUmbraStoreSlotWidget`). Cada linha indica **criar**, **adicionar** ou **modificar** o elemento nomeado.

**Pré-requisito (fora deste plano de nós):** o módulo C++ já compilado no Editor (**Toolbar** → **Compile**) para as classes aparecerem no picker de **Parent Class**.

**Documentação de apoio:** [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md) · [GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md](GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md) §2.4.

**Vários assets com o mesmo parent C++:** podes ter `WBP_PlayerStore`, `WBP_Personal_Player_Store`, etc., todos com **Parent Class** = `UmbraPlayerStoreWidget`, desde que repitam os **mesmos nomes** de widgets na hierarquia. O C++ não usa `BindWidgetOptional` no pai (evita ICE no compilador Blueprint do UE 5.x).

---

## A. `WBP_PlayerStore` *(ou `WBP_Personal_Player_Store` — ver também **§G** para o layout “personal”)*

### A.1 Class Settings — **modificar**

| # | Ação | Nó / campo literal |
|---|------|-------------------|
| A.1.1 | **modificar** | **Class Settings** (ícone ⚙ na barra do Blueprint) → painel **Details** → **Class Options** → **Parent Class** = `UmbraPlayerStoreWidget` (lista: digitar *UmbraPlayerStore* ou expandir hierarquia até `UmbraPlayerStoreWidget`). |
| A.1.2 | **modificar** | **Class Defaults** (botão **Class Defaults** na barra superior) → categoria **Personal Shop \| Classes** → **Store Slot Widget Class** = `WBP_StoreSlot` (referência de classe `User Widget`). |
| A.1.3 | **modificar** | **Class Defaults** → **Set Item Price Widget Class** = `WBP_SetItemPrice` (referência de classe; deve ser subclasse de `UmbraSetItemPriceWidget`). |

### A.2 Variáveis do Blueprint — **remover** (se existirem; estado passou para C++)

Para cada variável listada: **My Blueprint** → selecionar variável → **Delete** (ou desmarcar **Instance Editable** e esvaziar grafos que a usem, depois apagar).

| # | Ação | Nome literal da variável |
|---|------|--------------------------|
| A.2.1 | **remover** | `SelectedSlotIndex` (ou equivalente migrado) |
| A.2.2 | **remover** | `StoreSlotWidgets` |
| A.2.3 | **remover** | `ConfirmedInventoryID` |
| A.2.4 | **remover** | `ConfirmedPriceGold` |
| A.2.5 | **remover** | `DraftPriceGold` |
| A.2.6 | **remover** | `SlotHasOffer` |
| A.2.7 | **remover** | Qualquer variável de classe do modal / slot duplicada da lógica C++ (ex.: classe do modal só em BP, se já coberta por **Class Defaults**). |

*(Se o projeto nunca teve essas variáveis em BP, marcar N/A.)*

### A.3 Graph **Event Graph** — **remover nós** (grafos antigos da loja)

| # | Ação | Descrição literal |
|---|------|-------------------|
| A.3.1 | **remover** | Todas as cadeias ligadas a **Event Construct** que faziam: **Clear Children** em `Grid_StoreSlots`, **For Loop** 0–9, **Create Widget** (`WBP_StoreSlot`), **Add Child to Uniform Grid**, arrays de slots, etc. |
| A.3.2 | **remover** | Funções Blueprint personalizadas substituídas por C++ (ex.: nomes do tipo `Initialize Slot Price Grid`, `Register Item Dropped In Slot`, `Apply Price For Slot`, `Build Open Shop Listings`) — **My Blueprint** → **Functions** → botão direito → **Delete**. |
| A.3.3 | **remover** | **Custom Events** ligados só à lógica migrada (ex.: visibilidade Start/Close duplicada do GI) se o C++ já cobre. |

### A.4 Graph **Event Graph** — **vazio** (recomendado)

| # | Ação | Nota |
|---|------|------|
| A.4.1 | **modificar** | **Não** adicionar **Event Construct** só para “chamar o pai”: em muitos UE 5.x / idiomas **não** existe nó **Parent: Construct** / **Call to Parent**. O C++ (`NativeConstruct`) corre com o **Event Graph vazio**. |
| A.4.2 | **remover** | Se existir **Event Construct** sem utilidade, **apagar** o nó (evita confusão). Lógica visual extra = opcional com outros eventos, não obrigatória para a loja. |

### A.5 **Designer** (hierarquia / nomes) — **modificar** ou **criar**

Renomear cada widget na **Hierarchy** para o **Variable Name** / nome exato (painel **Details** → topo **Name**):

| # | Ação | Tipo de widget (sugerido) | Nome literal obrigatório |
|---|------|---------------------------|---------------------------|
| A.5.1 | **modificar** nome | `Uniform Grid Panel` | `Grid_StoreSlots` |
| A.5.2 | **modificar** nome | `Uniform Grid Panel` (ou painel que agrupe preços) | `Grid_SlotPrices` |
| A.5.3 | **modificar** nome | `Horizontal Box` | `HBox_SellItemOptions` |
| A.5.4 | **modificar** nome | `Horizontal Box` | `HBox_Start_Store` |
| A.5.5 | **modificar** nome | `Horizontal Box` | `HBox_Close_Store` |
| A.5.6 | **modificar** nome | `Button` | `BTN_Set_Price` |
| A.5.7 | **modificar** nome | `Button` | `BTN_Confirm_Offer` |
| A.5.8 | **modificar** nome | `Button` | `BTN_Cancel_Offer` |
| A.5.9 | **modificar** nome | `Button` | `BTN_Start_Store` |
| A.5.10 | **modificar** nome | `Button` | `BTN_Close_Store` |
| A.5.11 | **modificar** nome | `Button` | `Btn_Close` |
| A.5.12 | **modificar** nome | `Text` (Text Block) | `ShopNameText` |
| A.5.13 | **modificar** nome | `Text` (Text Block) | `Text_Vendor_Name` |
| A.5.14 | **criar** ou **modificar** nome | `Text` × 10 | `Text_Price_0`, `Text_Price_1`, …, `Text_Price_9` |

### A.6 **Compile** e **Save**

| # | Ação | Nó / comando literal |
|---|------|---------------------|
| A.6.1 | **modificar** | Barra superior do Blueprint → **Compile**. |
| A.6.2 | **modificar** | **Save**. |

---

## B. `WBP_SetItemPrice`

### B.1 Class Settings — **modificar**

| # | Ação | Nó / campo literal |
|---|------|-------------------|
| B.1.1 | **modificar** | **Class Settings** → **Parent Class** = `UmbraSetItemPriceWidget`. |

### B.2 Graph **Event Graph** — **remover**

| # | Ação | Descrição literal |
|---|------|-------------------|
| B.2.1 | **remover** | Cadeias de **Event Construct** que definiam dono da loja, índice, ou lia texto / confirmava preço em Blueprint. |
| B.2.2 | **remover** | **Functions** do tipo `On Confirm Clicked` / `Apply Price` implementadas em BP (substituídas por C++ `OnConfirmClicked` / `OnCancelClicked`). |

### B.3 Graph **Event Graph**

| # | Ação |
|---|------|
| B.3.1 | **Grafo vazio** (sem **Event Construct** obrigatório) — C++ do modal liga botões em `NativeConstruct`. |

### B.4 **Designer** — **modificar** nome

| # | Ação | Tipo | Nome literal |
|---|------|------|--------------|
| B.4.1 | **modificar** | `Editable Text Box` | `TextBox_GoldAmount` |
| B.4.2 | **modificar** | `Button` | `BTN_Confirm` |
| B.4.3 | **modificar** | `Button` | `BTN_Cancel` |

### B.5 **Compile** e **Save**

| # | Ação | Comando literal |
|---|------|-----------------|
| B.5.1 | **modificar** | **Compile** → **Save**. |

---

## C. `WBP_StoreSlot`

### C.1 Class Settings — **modificar**

| # | Ação | Nó / campo literal |
|---|------|-------------------|
| C.1.1 | **modificar** | **Class Settings** → **Parent Class** = `UmbraStoreSlotWidget`. |

### C.2 Graph — **remover** (se existir duplicação com C++)

| # | Ação | Descrição literal |
|---|------|-------------------|
| C.2.1 | **remover** | **Event On Drop** (ou **Override** → **On Drop**) e toda a cadeia: **Cast To Umbra Item Drag Drop Operation** → **Process Item Drop from Operation** → **Cast** para loja → **Register Item Dropped in Slot** — **não** manter em BP se `NativeOnDrop` em C++ já cobre o fluxo (evita duplo registo). |

### C.3 Graph — **vazio** + opcional visual

| # | Ação |
|---|------|
| C.3.1 | **Grafo vazio** (sem **Event Construct** obrigatório) — `NativeOnDrop` em C++. |
| C.3.2 | **Opcional:** **Event Refresh Slot Display** → **Get Slot Data** → atualizar **Image** / **Text** do item. |

### C.4 **Designer**

| # | Ação | Nota |
|---|------|------|
| C.4.1 | **modificar** | Sem obrigação de renomear nós para BindWidget do painel da loja; o slot usa a hierarquia herdada de `UmbraInventorySlotWidget` / BP existente. |

### C.5 **Compile** e **Save**

| # | Ação | Comando literal |
|---|------|-----------------|
| C.5.1 | **modificar** | **Compile** → **Save**. |

---

## D. Onde o `WBP_PlayerStore` é instanciado (fora dos três assets)

Se algum **HUD**, **Game Mode** ou **Widget** criar a loja por classe:

| # | Ação | Nó literal |
|---|------|------------|
| D.1 | **modificar** | Nó **Create Widget** → **Class** = `WBP_PlayerStore` **ou** `WBP_Personal_Player_Store` (não usar `UmbraPlayerStoreWidget` diretamente — classe abstract). |

---

## G. `WBP_Personal_Player_Store` (layout copiado, sem lógica Blueprint)

**Integração nó por nó (fazer funcionar — Class Defaults, Graph mínimo, Create Widget):** [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md).

Objetivo: o **mesmo** `UUmbraPlayerStoreWidget` C++ (`NativeConstruct`, slots, botões, GI) com **outro** asset só de Designer. Não precisas de **Functions** nem **Custom Events** para o fluxo da loja.

### G.0 Pré-requisito

| # | Ação | Nó / passo literal |
|---|------|-------------------|
| G.0.1 | **modificar** | Repositório com C++ atualizado (resolve `ShopNameText` como **Editable Text** / **Editable Text Box** / **Text**; resolve `HBox_StartStore` / `HBox_CloseStore` como alternativa a `HBox_Start_Store` / `HBox_Close_Store`). |
| G.0.2 | **modificar** | **Toolbar** → **Compile** (módulo C++), depois reabrir o Editor se necessário. |

### G.1 Class Settings (Blueprint `WBP_Personal_Player_Store`)

| # | Ação | Nó / campo literal |
|---|------|-------------------|
| G.1.1 | **modificar** | Abrir `WBP_Personal_Player_Store` → **Class Settings** (⚙) → **Parent Class** = `UmbraPlayerStoreWidget`. |
| G.1.2 | **modificar** | **Class Defaults** → **Personal Shop \| Classes** → **Store Slot Widget Class** = `WBP_StoreSlot` (subclasse de `UmbraStoreSlotWidget` ou `UmbraInventorySlotWidget`). |
| G.1.3 | **modificar** | **Class Defaults** → **Set Item Price Widget Class** = `WBP_SetItemPrice` (subclasse de `UmbraSetItemPriceWidget`). |

### G.2 Event Graph

| # | Ação |
|---|------|
| G.2.1 | **Event Graph vazio** (recomendado): **não** é preciso **Event Construct** nem nó “**Parent: Construct**” — em muitos UE 5.x / português esse menu **não existe**; o C++ `NativeConstruct` corre na mesma. |
| G.2.2 | Se existir **Event Construct** sem função: **apagar** o nó para evitar dúvidas. |

### G.3 Designer — nomes que o C++ procura (`GetWidgetFromName`)

A tabela cruza a **árvore** do teu export com o **nome** (coluna **Variable** no **Details** do widget selecionado na **Hierarchy**). O nome deve ser **exatamente** o da coluna “Nome obrigatório”.

| Área do layout (resumo) | Nome obrigatório no Editor | Tipo esperado pelo C++ |
|--------------------------|----------------------------|-------------------------|
| Grade de slots | `Grid_StoreSlots` | **Uniform Grid Panel** |
| Grade de preços | `Grid_SlotPrices` | **Uniform Grid Panel** |
| Rótulos preço | `Text_Price_0` … `Text_Price_9` | **Text** |
| Barra opções venda | `HBox_SellItemOptions` | **Horizontal Box** |
| Linha “abrir loja” | `HBox_Start_Store` **ou** `HBox_StartStore` | **Horizontal Box** |
| Linha “fechar loja” | `HBox_Close_Store` **ou** `HBox_CloseStore` | **Horizontal Box** |
| Botões barra | `BTN_Set_Price`, `BTN_Confirm_Offer`, `BTN_Cancel_Offer` | **Button** |
| Botões loja | `BTN_Start_Store`, `BTN_Close_Store` | **Button** |
| Fechar painel | `Btn_Close` | **Button** |
| Nome ao abrir loja (input) | `ShopNameText` | **Editable Text**, **Editable Text Box** ou **Text** |
| Nome exibido (título) | `Text_Vendor_Name` | **Text** |

**Como renomear no Editor (nós de UI, não Blueprint):**

1. **Hierarchy** → clicar no widget (ex. `HorizontalBox` que contém `BTN_Start_Store`).
2. **Details** → campo **Name** (ou **Variable** se “Is Variable” estiver ligado) → escrever exatamente `HBox_StartStore` ou `HBox_Start_Store`.
3. **Compile** (Blueprint) → **Save**.

O teu export já traz a maior parte dos nomes corretos (`Grid_StoreSlots`, `HBox_SellItemOptions`, `ShopNameText` como **Editable Text** — agora suportado em C++).

### G.4 Visibilidade inicial (opcional, só Designer)

| # | Ação | Passo literal |
|---|------|----------------|
| G.4.1 | **modificar** | Selecionar `HBox_SellItemOptions` → **Visibility** = **Collapsed** (já no export). O C++ mostra quando há drop. |
| G.4.2 | **modificar** | `HBox_StartStore` / `HBox_CloseStore` **Collapsed** no Designer está ok; `RefreshStartCloseVisibilityFromGameInstance` ajusta ao abrir o PIE. |

### G.5 Onde instanciar este asset

| # | Ação | Nó literal |
|---|------|------------|
| G.5.1 | **modificar** | No widget/HUD que abre a loja: nó **Create Widget** → **Class** = **`WBP_Personal_Player_Store`** (em vez de `WBP_PlayerStore`). |
| G.5.2 | **adicionar** | **Add to Viewport** (ou **Add to Player Screen**) após **Create Widget**, como já fazias para o store antigo. |

### G.6 Fluxo que corre **só em C++** (não criar estes nós no BP)

Após **Compile** do WBP com parent correto e **Class Defaults** preenchidos:

- **PopulateStoreSlotsModeA** preenche `Grid_StoreSlots` com 10× `WBP_StoreSlot`.
- **BindStoreButtons** liga **OnClicked** dos botões nomeados.
- **InitializeSlotPriceGrid** atualiza `Text_Price_0`…`9`.
- **Start Store** lê o texto de **`ShopNameText`** (Editable Text no teu layout) e chama **Open Personal Shop** no Game Instance.

**Não** adicionar ao **Event Graph**: loops de criação de slot, **Open Personal Shop**, **Assign** aos delegates do GI (duplicado).

---

## E. Ordem recomendada de execução do plano

1. **C** (`WBP_StoreSlot`) — parent + remover **On Drop** duplicado + **Refresh Slot Display**.  
2. **B** (`WBP_SetItemPrice`) — parent + nomes + grafo mínimo.  
3. **A** (`WBP_PlayerStore`) **ou** **G** (`WBP_Personal_Player_Store`) — parent + **Class Defaults** + renomes Designer + **Event Graph vazio** (sem depender de “Parent: Construct”).  
4. **D** — verificar **Create Widget** no resto do projeto.  
5. **PIE** — checklist do guia C++ (secção de validação).

---

## F. Nós que **não** devem ser criados (anti-padrão pós-migração)

| # | Não criar / não manter | Motivo |
|---|------------------------|--------|
| F.1 | **Get Game Instance** → **Open Personal Shop** dentro de `WBP_PlayerStore` | Já feito em C++ (`OnStartStoreClicked`). |
| F.2 | **Assign On Personal Shop Opened Local** dentro deste widget (duplicado) | C++ já subscreve em `SubscribePersonalShopDelegates`. |
| F.3 | **For Loop** + **Create Widget** (`WBP_StoreSlot`) no **Event Construct** do `WBP_PlayerStore` | C++ `PopulateStoreSlotsModeA` já faz **Clear Children** e recria os 10 filhos. |

---

*Plano literal para Editor; alterações de código C++ seguem repositório em `UmbraEternumUE/Source/UmbraEternumUE/UI/`.*
