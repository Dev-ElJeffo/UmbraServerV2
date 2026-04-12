# Integração nó por nó — loja pessoal a funcionar (WBP + onde abrir)

**Engine:** UE **5.6.1** — nameplate, comprador, bloqueio de movimento e encadeamento completo: [GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md](GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md).

Este documento é o **contrário** de um guia de montagem visual: assume que o **Designer** do `WBP_Personal_Player_Store` (ou equivalente) **já existe** com os **nomes** exigidos pelo C++. Aqui só entra **onde** ir no Editor e **que nós / campos** adicionar ou alterar para **ligar** o sistema.

Referência de nomes no UMG: [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md) §3 e [PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md) §G.3.

---

## Pré-requisito (fora de nós Blueprint)

| # | Onde | O que fazer |
|---|------|-------------|
| P.1 | Visual Studio / UE | Compilar o projeto C++ (**Compile** na toolbar do Editor ou build do `.sln`) para existir `UmbraPlayerStoreWidget`, `UmbraSetItemPriceWidget`, `UmbraStoreSlotWidget`. |
| P.2 | Designer do WBP da loja | Hierarquia com widgets nomeados para o `GetWidgetFromName` (lista na §G.3 do plano de nós). |

---

## A — `WBP_Personal_Player_Store` (ou qualquer WBP com o mesmo layout)

### A.1 Class Settings (não é Event Graph)

| # | Onde clicar | O que definir (literal) |
|---|-------------|-------------------------|
| A.1.1 | Abrir o asset `WBP_Personal_Player_Store` → barra superior → **Class Settings** (ícone de engrenagem / “Class Defaults” ao lado). | — |
| A.1.2 | Painel **Details** (Class Settings) → **Class Options** → **Parent Class**. | Clicar no dropdown → pesquisar **`UmbraPlayerStoreWidget`** → selecionar. **Não** deixar `User Widget` genérico. |

### A.2 Class Defaults (não é Event Graph)

| # | Onde | O que definir |
|---|------|----------------|
| A.2.1 | Barra superior do Blueprint → botão **Class Defaults**. | Abre o editor de defaults da classe. |
| A.2.2 | Painel **Details** → categoria **Personal Shop \| Classes** (ou nome próximo) → propriedade **Store Slot Widget Class**. | Clicar no seletor de classe → **User Widget** → escolher **`WBP_StoreSlot`** (o asset do slot). |
| A.2.3 | Mesma categoria → **Set Item Price Widget Class**. | Escolher **`WBP_SetItemPrice`**. |

### A.3 Event Graph — **não é obrigatório** ter nós aqui

O C++ (`UmbraPlayerStoreWidget::NativeConstruct`) corre **mesmo com o Event Graph vazio**. Muitas versões / idiomas do Editor **não** mostram nenhum nó “**Parent: Construct**” ou “**Call to Parent**” no menu — isso é normal.

| # | Onde | O que fazer |
|---|------|-------------|
| A.3.1 | **Graph** → **Event Graph**. | **Preferido:** deixar o grafo **completamente vazio** (sem **Event Construct**). **Compile** — a loja inicializa pelo C++. |
| A.3.2 | Se já existir um nó **Event Construct** (sobra de template). | **Apagar** o nó **Event Construct** e qualquer fio ligado a ele **ou** deixar o pin de saída **sem ligar** a nada (em geral o C++ continua a ser chamado; se algo falhar, apagar o **Event Construct**). |
| A.3.3 | **Não** adicionar lógica da loja no Graph (loops, **Open Personal Shop**, etc.). | — |

*(Se no teu Editor existir **“Add call to parent function”** / **“Chamar função pai”** ao clicar com o direito no pin de execução do **Event Construct**, podes usá-lo — é opcional, não obrigatório para este projeto.)*

### A.4 Guardar

| # | Onde | Ação |
|---|------|------|
| A.4.1 | Barra superior | **Compile** → **Save**. |

---

## B — `WBP_StoreSlot` (obrigatório para os 10 slots criados em C++)

### B.1 Class Settings

| # | Onde | O que definir |
|---|------|----------------|
| B.1.1 | Abrir `WBP_StoreSlot` → **Class Settings**. | **Parent Class** = **`UmbraStoreSlotWidget`**. |

### B.2 Event Graph

| # | O que fazer |
|---|-------------|
| B.2.1 | **Grafo vazio** (recomendado): sem **Event Construct** — o C++ do `UmbraStoreSlotWidget` / `NativeOnDrop` basta. |
| B.2.2 | **Opcional:** nó **Event Refresh Slot Display** (override no Graph, se existir na lista de eventos do widget) → cadeia só de **UI** (**Get Slot Data** → atualizar **Image** / **Text**). **Não** chamar **Open Personal Shop**. |

### B.3 Guardar

| # | Ação |
|---|------|
| B.3.1 | **Compile** → **Save**. |

---

## C — `WBP_SetItemPrice` (modal de preço)

### C.1 Class Settings

| # | Onde | O que definir |
|---|------|----------------|
| C.1.1 | Abrir `WBP_SetItemPrice` → **Class Settings**. | **Parent Class** = **`UmbraSetItemPriceWidget`**. |

### C.2 Event Graph

| # | O que fazer |
|---|-------------|
| C.2.1 | **Grafo vazio** (recomendado): sem **Event Construct** — `UUmbraSetItemPriceWidget::NativeConstruct` no C++ liga os botões. |

### C.3 Designer (nomes, se ainda não estiverem)

| # | Widget | **Name** exato |
|---|--------|----------------|
| C.3.1 | Caixa de texto do ouro | `TextBox_GoldAmount` |
| C.3.2 | Botão confirmar | `BTN_Confirm` |
| C.3.3 | Botão cancelar | `BTN_Cancel` |

### C.4 Guardar

| # | Ação |
|---|------|
| C.4.1 | **Compile** → **Save**. |

---

## D — Blueprint que **abre** a janela da loja (HUD, inventário, tecla, etc.)

Substitui `WBP_SeubMenu` pelo asset real onde queres o botão ou evento.

### D.1 Se for por **botão** (ex.: “Minha loja”)

| # | Onde | Nó / ação |
|---|------|-----------|
| D.1.1 | Abrir o WBP do menu (ex. inventário) → **Designer**. | Selecionar o **Button** que deve abrir a loja. |
| D.1.2 | **Details** do botão → secção **Events** → **OnClicked** → **+**. | O Editor muda para **Graph** com o nó **OnClicked (ButtonX)**. |
| D.1.3 | Graph: arrastar do pin **exec** de **OnClicked**. | Menu → **Create Widget**. |
| D.1.4 | Nó **Create Widget** → pin **Class**. | Dropdown → **`WBP_Personal_Player_Store`** (não `UmbraPlayerStoreWidget`). |
| D.1.5 | Nó **Create Widget** → pin **Owning Player**. | Arrastar de **Get Owning Player** (clicar direito → **Get Owning Player**) para o pin **Owning Player**. |
| D.1.6 | Arrastar do pin **Return Value** de **Create Widget**. | **Add to Viewport**. |
| D.1.7 | **OnClicked** (exec) | → **Create Widget** (exec) → **Add to Viewport** (exec). |

### D.2 Se for por **Custom Event** (chamado de outro Blueprint)

| # | Onde | Nó |
|---|------|-----|
| D.2.1 | **Graph** do WBP que tem contexto de jogo → botão direito. | **Add Custom Event** → nome ex.: `OpenPersonalPlayerStore`. |
| D.2.2 | **OpenPersonalPlayerStore** (exec saída) | → **Create Widget** (igual D.1.4–D.1.5) → **Add to Viewport** (D.1.6). |
| D.2.3 | No Blueprint que chama | Nó **OpenPersonalPlayerStore** (ou **Call OpenPersonalPlayerStore**) apontando para o widget correto (referência ou cast). |

### D.3 (Opcional) Rato e foco ao abrir UI

| # | Nós (em cadeia após **Add to Viewport** ou no mesmo **OnClicked**) | Função |
|---|---------------------------------------------------------------------|--------|
| D.3.1 | **Get Player Controller** (índice `0`) | Referência ao PC. |
| D.3.2 | **Set Show Mouse Cursor** (target = PC) | **Show Mouse Cursor** = ✓. |
| D.3.3 | **Set Input Mode UI Only** (opcional) | Target = PC; **In Widget to Focus** = return value do **Create Widget** se quiseres foco no painel. |

---

## E — Ordem de **Compile** (evita referências quebradas)

| Ordem | Asset | Ação |
|-------|-------|------|
| 1 | `WBP_SetItemPrice` | **Compile** |
| 2 | `WBP_StoreSlot` | **Compile** |
| 3 | `WBP_Personal_Player_Store` | **Compile** |
| 4 | Blueprint que tem **Create Widget** (§D) | **Compile** |

---

## F — Nós e secções **não** adicionar (C++ já faz)

| # | Não adicionar | Onde |
|---|----------------|------|
| F.1 | **For Loop** (0–9) + **Create Widget** (`WBP_StoreSlot`) + **Add Child to Uniform Grid** | `WBP_Personal_Player_Store` — feito em `PopulateStoreSlotsModeA`. |
| F.2 | **Get Game Instance** → **Open Personal Shop** | Qualquer WBP filho de `UmbraPlayerStoreWidget` — feito em **OnStartStoreClicked** no C++. |
| F.3 | **Assign On Personal Shop Opened Local** (ou Closed) ligado a este painel | O C++ já subscreve no `NativeConstruct`. |
| F.4 | **OnClicked** manual em **BTN_Set_Price**, **BTN_Confirm_Offer**, etc. | O C++ faz `BindStoreButtons` em `NativeConstruct`. |
| F.5 | **On Drop** no slot que chame lógica duplicada | Se o parent do slot for **`UmbraStoreSlotWidget`**, o drop é **NativeOnDrop** em C++; não duplicar grafo longo no BP. |

---

## G — Verificação rápida (PIE)

| # | Ação manual no jogo | Se falhar, verificar |
|---|---------------------|----------------------|
| G.1 | Abrir a loja pelo fluxo do §D. | **Create Widget** class; **Parent Class** da loja; **Compile** order. |
| G.2 | Arrastar item para um slot. | **Store Slot Widget Class**; `WBP_StoreSlot` parent `UmbraStoreSlotWidget`; **Refresh Slot Display** no slot se não houver ícone. |
| G.3 | **Set Price** → modal. | **Set Item Price Widget Class**; nomes §C.3 no modal. |
| G.4 | **Start Store** com nome + oferta. | Campo **`ShopNameText`** na hierarquia; texto não vazio; pelo menos uma oferta confirmada. |

---

*Lista de integração: ligação entre assets e abertura do widget; layout UMG não é descrito aqui.*
