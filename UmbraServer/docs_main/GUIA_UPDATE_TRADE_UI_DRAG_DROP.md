# Guia: UpdateTradeUI com Drag & Drop

Este guia detalha a implementação da **UpdateTradeUI** usando **Grid_TradeSlot1** e **Grid_TradeSlot2**, integrando o sistema de drag and drop existente (inventário e armazém).

---

## Visão Geral

- **Grid_TradeSlot1**: itens do jogador local (permite adicionar via drag do inventário e remover via drag de volta)
- **Grid_TradeSlot2**: itens do parceiro (somente leitura)
- Slots usam **WBP_ItemSlot** (ou similar) que herda de **UmbraInventorySlotWidget**
- **Text_ItemCount**: "Slots: X/10"
- **Text_Gold_Info**: Gold (meu e do parceiro)

---

## Pré-requisitos

1. **WBP_ItemSlot** (ou WBP_InventorySlot) herda de **UmbraInventorySlotWidget**
2. **UmbraInventorySlotWidget** com `ParentTradeWidget` e `TradeItemID` (ver alterações C++ abaixo)
3. Helper **CreateInventorySlotFromTradeItem** no GameInstance (ver alterações C++ abaixo)

---

## Passo 1: Função CreateTradeSlots (como CreateInventorySlots)

Crie uma função **CreateTradeSlots** no WBP_Trade que cria os 10 slots dinamicamente em cada grid, igual ao **CreateInventorySlots** do WBP_Inventory.

### 1.1 Variáveis no WBP_Trade

| Variável | Tipo | Descrição |
|----------|------|-----------|
| MySlotWidgets | Array of UmbraInventorySlotWidget | Referências aos 10 slots de Grid_TradeSlot1 |
| PartnerSlotWidgets | Array of UmbraInventorySlotWidget | Referências aos 10 slots de Grid_TradeSlot2 |
| Grid_TradeSlot1 | Uniform Grid Panel | Container dos meus itens |
| Grid_TradeSlot2 | Uniform Grid Panel | Container dos itens do parceiro |

### 1.2 Lógica da CreateTradeSlots

**Layout:** 10 slots = 5 colunas × 2 linhas (Row = Index/5, Column = Index%5).

```
[CreateTradeSlots]
    │
    ├──► MySlotWidgets.Clear() e PartnerSlotWidgets.Clear()
    │
    ├──► ForLoop: FirstIndex=0, LastIndex=9  (Index)
    │         │
    │         ├──► Row = Index / 5
    │         ├──► Column = Index % 5
    │         │
    │         ├──► [MEUS SLOTS]
    │         │     Create Widget (Class = WBP_InventorySlot, OwningPlayer = GetPlayerController)
    │         │     Set Slot Index (Target = widget, NewSlotIndex = Index)
    │         │     Set Parent Trade Widget (Target = widget) = self
    │         │     Add Child to Uniform Grid (Target = Grid_TradeSlot1, Content = widget, InRow = Row, InColumn = Column)
    │         │     MySlotWidgets.Add(widget)
    │         │
    │         └──► [PARCEIRO SLOTS]
    │               Create Widget (Class = WBP_InventorySlot, OwningPlayer = GetPlayerController)
    │               Set Slot Index (Target = widget, NewSlotIndex = Index)
    │               (NÃO setar Parent Trade Widget – slots do parceiro são só leitura)
    │               Add Child to Uniform Grid (Target = Grid_TradeSlot2, Content = widget, InRow = Row, InColumn = Column)
    │               PartnerSlotWidgets.Add(widget)
```

**Importante:** Use **dois** Create Widget + Add Child no mesmo loop (um para cada grid) ou faça **dois loops** separados: um para Grid_TradeSlot1 e outro para Grid_TradeSlot2.

### 1.3 Versão com dois loops (mais simples)

**Loop 1 – Meus slots (0 a 9):**
```
ForLoop 0 to 9 → Index
    Row = Index / 5
    Column = Index % 5
    Create Widget (WBP_InventorySlot)
    Set Slot Index = Index
    Set Parent Trade Widget = self
    Add Child to Uniform Grid (Grid_TradeSlot1, Row, Column)
    MySlotWidgets.Add(widget)
```

**Loop 2 – Slots do parceiro (0 a 9):**
```
ForLoop 0 to 9 → Index
    Row = Index / 5
    Column = Index % 5
    Create Widget (WBP_InventorySlot)
    Set Slot Index = Index
    Add Child to Uniform Grid (Grid_TradeSlot2, Row, Column)
    PartnerSlotWidgets.Add(widget)
```

### 1.4 Set Parent Trade Widget (Blueprint) – PASSO A PASSO

`ParentTradeWidget` é variável BlueprintReadWrite no **UmbraInventorySlotWidget**. Você precisa de um nó **Set** (não Get) para atribuir o valor.

**Onde inserir:** Entre **Set Slot Index** e **Add Child to Uniform Grid**, no fluxo de execução.

**Método 1 – Arrastar do pin do widget:**
1. No grafo, localize o **Return Value** do nó **Create Widget** (o widget criado).
2. Arraste esse pin e solte na área vazia.
3. No menu de contexto, pesquise por **"Set Parent Trade Widget"** ou **"Parent Trade Widget"**.
4. Se aparecer **"Set Parent Trade Widget"**, selecione. O nó será criado com **Target** = widget.
5. Conecte o pin **self** (do WBP_Trade) ao pin de **valor** do Set (o pin que recebe o novo valor).

**Método 2 – Criar o nó manualmente:**
1. Clique com botão direito no grafo → pesquise **"Set Parent Trade Widget"**.
2. Se não aparecer, pesquise **"Set"** e depois escolha a variável **Parent Trade Widget** (dentro de UmbraInventorySlotWidget).
3. No nó Set:
   - **Target** (ou "self"): conecte o **Return Value** do **Create Widget** (o slot criado).
   - **Parent Trade Widget** (valor): conecte **self** (referência ao WBP_Trade).

**Método 3 – Nó “Set (by ref)” de variável:**
1. Arraste o **Return Value** do Create Widget para o grafo.
2. No menu, procure por **"Set"** e depois selecione a variável **Parent Trade Widget**.
3. O nó terá um pin **Target** – conecte o widget (Return Value do Create Widget).
4. O pin de valor – conecte **self** (drag do pin "self" do evento/função).

**Conexões resumidas:**

| Pin do nó Set | Conectar a |
|---------------|------------|
| **Target** (objeto a modificar) | Return Value do **Create Widget** |
| **Parent Trade Widget** (valor) | **self** (WBP_Trade) |

**Importante:** O Blueprint atual (WBP_Trade) **não é** UmbraInventorySlotWidget. Por isso o pin **Target** **precisa** de conexão explícita — o Target é o slot criado (Return Value do Create Widget), não o "self" do WBP_Trade.

**Fluxo visual esperado:**
```
Create Widget (Return Value) ──┬──► Target do Set Parent Trade Widget (OBRIGATÓRIO)
                               ├──► Target do Set Slot Index
                               └──► Content do Add Child to Uniform Grid

self (WBP_Trade) ──────────────► Valor do Set Parent Trade Widget
```

**Atenção:** Use **Set**, não **Get**. "Get Parent Trade Widget" apenas lê o valor; é o **Set** que define o valor.

**Se só aparecer "Get Parent Trade Widget":** No nó Get, clique com o botão direito no nó ou no pin de saída e procure por **"Convert to Set"** / **"Promote to Set"**. Isso cria o nó Set correspondente. Depois:
- Conecte o **Return Value** do Create Widget ao pin **Target** do Set.
- Conecte **self** ao pin de entrada do valor (que antes era saída no Get).

### 1.5 Onde chamar CreateTradeSlots

- **Event Construct:** chame `CreateTradeSlots` no início (antes dos binds).
- Ou em **InitializeTrade:** se quiser recriar os slots a cada troca (útil para resetar estado).
  - Nesse caso, limpe os grids e arrays antes: `MySlotWidgets.Empty()`, `PartnerSlotWidgets.Empty()`, e remova os filhos dos grids (Remove Child em cada filho).

---

## Passo 2: Configurar Slots para Trade

Para cada slot em **MySlotWidgets**:

1. **Set Slot Index** = 0 a 9 (índice no grid)
2. **Set Parent Trade Widget** = self (WBP_Trade)
3. **bCanAcceptItems** = true
4. **bCanDragFrom** = true (quando tiver item)

Para **PartnerSlotWidgets**:

1. **Set Slot Index** = 0 a 9
2. **bCanAcceptItems** = false
3. **bCanDragFrom** = false (somente leitura)

---

## Passo 3: Lógica da UpdateTradeUI (DETALHADO)

Crie a função **UpdateTradeUI** com um parâmetro **TradeState** (tipo **FUmbraTradeState**).

---

### 3.1 Verificar Status (Fechar se troca acabou)

**Passo a passo:**

1. Rode **Break FUmbraTradeState** no parâmetro TradeState para obter os campos.
2. Pegue o pin **Status** (String).
3. Adicione um **Branch**.
4. Na condição do Branch, use **Equal (String)**:
   - A = **Status**
   - B = `"completed"` (literal)
5. Ou use **OR** entre dois Equal: `Status == "completed"` **OR** `Status == "cancelled"`.

**No pin True do Branch:**
- **Clear Timer by Handle** (Handle = variável PollingTimerHandle)
- **Remove from Parent** (Target = self)
- **Return** (interrompe a função)

**No pin False do Branch:** continue o fluxo abaixo.

---

### 3.2 Determinar Meus Itens e Parceiro (Quem sou eu?)

A API retorna `player1_items` e `player2_items`. Você precisa saber se você é o Player1 ou o Player2 para usar os arrays corretos.

---

#### 3.2.1 Criar as variáveis locais

Antes de montar o Branch, crie **4 variáveis locais** na função UpdateTradeUI:

| Nome da variável | Tipo | Descrição |
|------------------|------|-----------|
| **MeusItems** | Array of FUmbraTradeItem | Itens que você colocou na troca |
| **MeuGold** | Integer | Seu gold oferecido |
| **ParceiroItems** | Array of FUmbraTradeItem | Itens que o parceiro colocou |
| **ParceiroGold** | Integer | Gold oferecido pelo parceiro |

**Como criar:**
1. No grafo da função UpdateTradeUI, clique com botão direito.
2. Pesquise **"Add Local Variable"** ou vá em **Local Variables** no painel à esquerda.
3. Crie cada variável com o nome e tipo acima. Não precisa definir valor inicial.

---

#### 3.2.2 Montar o Branch (MyPlayerID == Player1ID?)

**Passo a passo:**

1. Coloque o **Break FUmbraTradeState** no parâmetro TradeState (se ainda não tiver).
2. Adicione um **Equal (Integer)**:
   - **A** = pin **MyPlayerID** do Break
   - **B** = pin **Player1ID** do Break
3. Adicione um **Branch**.
4. Conecte a saída **Return Value** (bool) do Equal ao pin **Condition** do Branch.
5. O pin **exec** do Branch deve vir do fluxo anterior (ex.: do pin **False** do Branch de Status, quando a troca está ativa).

---

#### 3.2.3 Ramo True (você é o Player1)

Quando **MyPlayerID == Player1ID**, seus itens estão em Player1Items e os do parceiro em Player2Items.

**Passo a passo no pin True do Branch:**

1. No **Loop Body** (ou sequência) do pin **True**:
   - **Set MeusItems** (Target = variável local MeusItems) ← conecte **Player1Items** (do Break) ao valor
   - **Set MeuGold** ← conecte **Player1GoldOffer** (do Break) ao valor
   - **Set ParceiroItems** ← conecte **Player2Items** (do Break) ao valor
   - **Set ParceiroGold** ← conecte **Player2GoldOffer** (do Break) ao valor

2. **Ordem dos nós:** Conecte em sequência (exec pin):
   - True → Set MeusItems → Set MeuGold → Set ParceiroItems → Set ParceiroGold → **[continua para o próximo passo]**

3. **Conexões de valor:**
   - Set MeusItems: pin de valor ← **Player1Items** (do Break FUmbraTradeState)
   - Set MeuGold: pin de valor ← **Player1GoldOffer**
   - Set ParceiroItems: pin de valor ← **Player2Items**
   - Set ParceiroGold: pin de valor ← **Player2GoldOffer**

---

#### 3.2.4 Ramo False (você é o Player2)

Quando **MyPlayerID != Player1ID**, você é o Player2. Seus itens estão em Player2Items e os do parceiro em Player1Items.

**Passo a passo no pin False do Branch:**

1. No pin **False**:
   - **Set MeusItems** ← **Player2Items** (do Break)
   - **Set MeuGold** ← **Player2GoldOffer**
   - **Set ParceiroItems** ← **Player1Items**
   - **Set ParceiroGold** ← **Player1GoldOffer**

2. **Ordem:** False → Set MeusItems → Set MeuGold → Set ParceiroItems → Set ParceiroGold → **[continua para o próximo passo]**

---

#### 3.2.5 Unir os dois ramos

Os ramos **True** e **False** devem **convergir** para o mesmo próximo passo (Clear Slot). Ou seja:

- O último **Set ParceiroGold** do ramo True e do ramo False devem ambos conectar ao mesmo nó seguinte (ex.: For Each MySlotWidgets → Clear Slot).

**Fluxo visual:**
```
                    ┌→ True  → Set MeusItems → Set MeuGold → Set ParceiroItems → Set ParceiroGold ─┐
Break → Equal ──► Branch                                                                           ├──► For Each (Clear Slot)
                    └→ False → Set MeusItems → Set MeuGold → Set ParceiroItems → Set ParceiroGold ─┘
```

Depois dessa etapa, **MeusItems**, **MeuGold**, **ParceiroItems** e **ParceiroGold** estarão preenchidos corretamente e poderão ser usados nos próximos passos (Clear Slot, Preencher Meus Items, Preencher Parceiro Items, Textos).

---

### 3.3 Limpar Todos os Slots (Deixar tudo vazio antes de preencher)

**Passo a passo – Meus slots:**

1. **For Each** (Array = **MySlotWidgets**).
2. O pin **Array Element** retorna cada slot (tipo UmbraInventorySlotWidget).
3. No **Loop Body** do For Each:
   - **Clear Slot** (Target = **Array Element**)
   - O nó Clear Slot fica em: pesquise "Clear Slot" → UmbraInventorySlotWidget.

**Passo a passo – Slots do parceiro:**

1. **For Each** (Array = **PartnerSlotWidgets**).
2. No **Loop Body**:
   - **Clear Slot** (Target = Array Element)
   - (Opcional) **Set Parent Trade Widget** = null no slot, se quiser garantir que não aceita drop. Para slots do parceiro normalmente não é necessário.

---

### 3.4 Configurar Meus Slots para Aceitar Trade (Antes de preencher)

**Passo a passo:**

1. **For Each** (Array = **MySlotWidgets**).
2. No **Loop Body**:
   - **Set Parent Trade Widget** (Target = Array Element, Valor = **self**)
   - **Set Trade Session ID** (Target = Array Element, Valor = variável **TradeSessionID** do WBP_Trade)

Isso garante que, ao fazer drag do inventário, o slot reconheça que é de trade.

---

### 3.5 Preencher Meus Slots com os Itens (MeusItems)

**Passo a passo – nó a nó:**

1. **For Each** (Array = **MeusItems**).
   - O For Each tem: **Array Element** (cada FUmbraTradeItem) e **Array Index** (0, 1, 2, …).

2. No **Loop Body**, em sequência:

   **a) Create Inventory Slot From Trade Item**
   - Pesquise: "Create Inventory Slot From Trade Item"
   - Target: **Get Game Instance** → **Cast to UmbraGameInstance**
   - **Trade Item** = **Array Element** (do For Each)
   - **Slot Index** = **Array Index** (do For Each)
   - Saída: **Return Value** (FUmbraInventorySlot) → guarde em variável ou use direto

   **b) Get slot do array**
   - Pegue a variável **MySlotWidgets** (Array).
   - Arraste ou use o nó **Get** em arrays: em MySlotWidgets, há um pin que permite indexar. Use **Array Index** (do For Each) como índice.
   - Alternativa: pesquise "Get" → escolha "Get (a copy)" do array com **Index** = Array Index.
   - Resultado: o slot (UmbraInventorySlotWidget) onde o item será exibido, na posição i.

   **c) Set Slot Data**
   - **Set Slot Data** (Target = slot do passo b, New Slot Data = Return Value do passo a)

   **d) Set Trade Item ID**
   - **Break FUmbraTradeItem** no Array Element para obter **TradeItemID**.
   - Use **Set** no slot: variável **Trade Item ID** = TradeItemID (do Break).
   - O slot tem a variável TradeItemID (BlueprintReadWrite). Target = slot do passo b.
   - Objetivo: guardar o trade_item_id para poder chamar RemoveTradeItem ao arrastar de volta.

   **e) Update Slot Visual**
   - **Update Slot Visual** (Target = slot)
   - Função do WBP_InventorySlot. **Sem isso o ícone não aparece.**

**Fluxo visual da cadeia:**
```
For Each (MeusItems)
    Array Element ──► Create Inventory Slot From Trade Item (Trade Item, Slot Index = Array Index)
    Array Index  ──► Get MySlotWidgets[Array Index]  →  Slot
    Return Value ──► Set Slot Data (Target = Slot, New Slot Data = Return Value)
    Array Element.TradeItemID ──► Set Trade Item ID (Target = Slot)
    Slot ──► Update Slot Visual (Target = Slot)
```

---

### 3.6 Preencher Slots do Parceiro (ParceiroItems)

**Passo a passo – igual ao 3.5, mas com ParceiroItems e PartnerSlotWidgets:**

1. **For Each** (Array = **ParceiroItems**).
2. No **Loop Body**:
   - **Create Inventory Slot From Trade Item** (Trade Item = Array Element, Slot Index = Array Index)
   - **Get** PartnerSlotWidgets[**Array Index**] → Slot
   - **Set Slot Data** (Target = Slot, New Slot Data = Return Value)
   - **Update Slot Visual** (Target = Slot)
   - **Não** precisa Set Trade Item ID (parceiro é só leitura).

---

### 3.7 Atualizar Textos (Contador e Gold)

**Contador de slots:**
- **Length** (MeusItems) → int
- **Append** strings: `"Slots: "` + **ToString**(Length) + `"/10"`
- **Set Text** (Target = Text_ItemCount, In Text = resultado)

**Gold:**
- **Append**: `"Meu Gold: "` + **ToString**(MeuGold) + `"  |  Parceiro: "` + **ToString**(ParceiroGold)
- **Set Text** (Target = Text_Gold_Info, In Text = resultado)

Se tiver **TB_MyGold** e **TB_PartnerGold** separados:
- **Set Text** (TB_MyGold) = **ToString**(MeuGold)
- **Set Text** (TB_PartnerGold) = **ToString**(ParceiroGold)

---

### 3.8 Ordem Completa da UpdateTradeUI (Checklist)

| # | Ação |
|---|------|
| 1 | Break FUmbraTradeState (TradeState) |
| 2 | Branch: Status == "completed" ou "cancelled"? → True: Clear Timer, Remove from Parent, Return |
| 3 | Branch: MyPlayerID == Player1ID? → Definir MeusItems, MeuGold, ParceiroItems, ParceiroGold |
| 4 | For Each MySlotWidgets → Clear Slot |
| 5 | For Each PartnerSlotWidgets → Clear Slot |
| 6 | For Each MySlotWidgets → Set Parent Trade Widget, Set Trade Session ID |
| 7 | For Each MeusItems → CreateInventorySlotFromTradeItem, Get MySlotWidgets[i], SetSlotData, Set TradeItemID, UpdateSlotVisual |
| 8 | For Each ParceiroItems → CreateInventorySlotFromTradeItem, Get PartnerSlotWidgets[i], SetSlotData, UpdateSlotVisual |
| 9 | Set Text (contador e gold) |

---

## Passo 4: Drag & Drop – Inventário → Trade

O **UmbraInventorySlotWidget** (após alterações C++) passa a tratar:

- **Destino = slot de trade** (`ParentTradeWidget != null`) e **origem = inventário** → chama **AddTradeItem** em vez de **RequestMoveItem**.

**IMPORTANTE – O que o slot de trade precisa para aceitar drops:**

| Requisito | Onde definir | Por quê |
|-----------|-------------|---------|
| **ParentTradeWidget** = self | CreateTradeSlots | Sem isso, o C++ não reconhece o slot como destino de trade |
| **TradeSessionID** ou sessão ativa | UpdateTradeUI OU C++ fallback | Se TradeSessionID=0, o C++ usa `GetCurrentTradeSessionID()` do GameInstance |
| **bCanAcceptItems** = true | CreateTradeSlots (ou default) | CanAcceptItem retorna false se for false |

**Correção C++ (já implementada):** Se o slot tem `ParentTradeWidget` mas `TradeSessionID=0`, o C++ usa `GetCurrentTradeSessionID()` do GameInstance. Assim funciona mesmo sem UpdateTradeUI setar TradeSessionID nos slots.

**Troca deve estar ativa:** O drag inventário→trade só funciona quando há uma sessão de troca ativa (ex.: aceitar uma solicitação de trade). Sem isso, o item continua sendo tratado como movimento dentro do inventário.

O WBP_InventorySlot já tem OnDrop ligado ao ProcessItemDropFromOperation. Nenhuma alteração extra no Blueprint do slot é necessária — basta configurar ParentTradeWidget nos slots de trade.

---

## Passo 5: Drag & Drop – Trade → Inventário (Remover)

Quando o jogador arrasta um item do grid de trade e solta no inventário:

- **Origem = slot de trade** (`ParentTradeWidget != null`) e **destino = inventário** → chama **RemoveTradeItem(TradeItemID)**.

O **TradeItemID** vem de **Get Trade Item ID** no slot de origem (definido em UpdateTradeUI).

---

## Passo 6: Variáveis e Funções no C++

### 6.1 UmbraInventorySlotWidget – Novas Propriedades

```cpp
/** Quando != null, este slot pertence à janela de trade */
UPROPERTY(BlueprintReadWrite, Category = "Trade")
UObject* ParentTradeWidget;

/** ID do item na troca (trade_item_id). Usado para RemoveTradeItem. 0 = não é item de trade. */
UPROPERTY(BlueprintReadWrite, Category = "Trade")
int32 TradeItemID;
```

### 6.2 UmbraGameInstance – Helper

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Social|Trade")
static FUmbraInventorySlot CreateInventorySlotFromTradeItem(const FUmbraTradeItem& TradeItem, int32 SlotIndex);
```

### 6.3 ProcessItemDropFromOperation – Lógica de Trade

Antes de chamar `RequestMoveItemByID`:

- Se **destino** tem `ParentTradeWidget != null` e **origem** tem `ParentStorageWidget == null` e `ParentTradeWidget == null` → **AddTradeItem**.
- Se **origem** tem `ParentTradeWidget != null` e **destino** tem `ParentStorageWidget == null` → **RemoveTradeItem**.

---

## Resumo do Fluxo

1. **UpdateTradeUI** limpa os slots, preenche com `CreateInventorySlotFromTradeItem` + `SetSlotData` + `SetTradeItemID`.
2. **Drag do inventário para trade**: o slot de trade recebe o drop e o C++ chama **AddTradeItem**.
3. **Drag do trade para inventário**: o slot de inventário recebe o drop e o C++ chama **RemoveTradeItem**.
4. O **polling** chama **LoadTradeState** a cada 1,5 s, que dispara **OnTradeStateLoaded** e **UpdateTradeUI** atualiza a UI.

---

## Checklist

| Item | Onde | Status |
|------|------|--------|
| 10 slots em Grid_TradeSlot1 | Designer | ⬜ |
| 10 slots em Grid_TradeSlot2 | Designer | ⬜ |
| MySlotWidgets e PartnerSlotWidgets | Event Construct | ⬜ |
| ParentTradeWidget nos meus slots | Event Construct | ⬜ |
| UpdateTradeUI: Branch Status | Função | ⬜ |
| UpdateTradeUI: MeusItems/ParceiroItems | Função | ⬜ |
| UpdateTradeUI: Clear + Preencher | Função | ⬜ |
| UpdateTradeUI: Textos | Função | ⬜ |
| C++: ParentTradeWidget, TradeItemID | UmbraInventorySlotWidget | ⬜ |
| C++: CreateInventorySlotFromTradeItem | UmbraGameInstance | ⬜ |
| C++: Lógica de trade em ProcessItemDrop | UmbraInventorySlotWidget | ⬜ |
| UpdateSlotVisual após SetSlotData | UpdateTradeUI | ⬜ |
| Bind OnTradeStateLoaded → UpdateTradeUI | Event Construct WBP_Trade | ⬜ |

---

## Troubleshooting: Itens não aparecem na janela de trade

Se os logs mostram **AddTradeItem** e **Trade state carregado** com `player1_items`/`player2_items` preenchidos, mas os itens não aparecem na UI:

### 1. Verificar bind de OnTradeStateLoaded

No **Event Construct** do WBP_Trade: **Get Game Instance** → **Cast to UmbraGameInstance** → **Bind Event to On Trade State Loaded** → Event = Custom Event `OnTradeStateLoaded_Custom`. O Custom Event deve ter parâmetro **TradeState** (FUmbraTradeState) e chamar **UpdateTradeUI(TradeState)**.

### 2. Implementar UpdateTradeUI

A função **UpdateTradeUI(TradeState)** precisa: Limpar todos os slots; Determinar MeusItems e ParceiroItems; Preencher MeusItems e ParceiroItems com SetSlotData + **UpdateSlotVisual**.

### 3. Chamar UpdateSlotVisual

Após **Set Slot Data**, **sempre** chame **Update Slot Visual** no slot. Sem isso, o ícone não aparece.

### 4. PartnerSlotWidgets

Crie os slots do parceiro em **CreateTradeSlots** (segundo loop) e adicione a **PartnerSlotWidgets**.

---

## FIX: O que falta na UpdateTradeUI atual

**Causa raiz:** A `UpdateTradeUI` atual **apenas limpa** os slots (`ClearSlot`) e **não popula** com os dados de `MyItems` e `PartnerItems`. Por isso os itens nunca aparecem na UI.

### O que a UpdateTradeUI atual faz (parcialmente):
- ✅ Break TradeState, Branch Status, atribuição MyItems/PartnerItems
- ✅ For Each MySlotWidgets → ClearSlot
- ✅ For Each PartnerSlotWidgets → ClearSlot
- ❌ **Falta:** Loops que preenchem os slots com os itens

### O que adicionar (após os ClearSlot):

#### Passo A: For Each MyItems (MeusItems)

1. Adicione **For Each** com Array = **MyItems** (ou MeusItems).
2. No Loop Body:
   - **Create Inventory Slot From Trade Item** (Target = Get Game Instance → Cast to UmbraGameInstance)
     - Trade Item = **Array Element** (do For Each)
     - Slot Index = **Array Index** (do For Each)
   - **Get** MySlotWidgets[**Array Index**] → Slot (use “Get (a copy)” ou “Array Get” com Index)
   - **Set Slot Data** (Target = Slot, New Slot Data = Return Value do CreateInventorySlotFromTradeItem)
   - **Break FUmbraTradeItem** no Array Element → pegue **TradeItemID**
   - **Set Trade Item ID** (Target = Slot, valor = TradeItemID) — variável BlueprintReadWrite do slot
   - **Cast to WBP_InventorySlot** (Object = Slot)
   - **Update Slot Visual** (Target = saída "As WBP Inventory Slot" do Cast)

   **Target do Update Slot Visual:** O Target espera `WBP_InventorySlot`, mas a saída de Get MySlotWidgets[Array Index] é `UmbraInventorySlotWidget`. Use **Cast to WBP_InventorySlot** entre o Array Get e o Update Slot Visual: `Array Get (MySlotWidgets, Array Index) → Cast to WBP_InventorySlot → Target do Update Slot Visual`. A saída "As WBP Inventory Slot" é válida porque os slots são criados com WBP_InventorySlot.

#### Passo B: For Each PartnerItems (ParceiroItems)

1. Adicione **For Each** com Array = **PartnerItems** (ou ParceiroItems).
2. No Loop Body:
   - **Create Inventory Slot From Trade Item** (Trade Item = Array Element, Slot Index = Array Index)
   - **Get** PartnerSlotWidgets[**Array Index**] → Slot
   - **Set Slot Data** (Target = Slot, New Slot Data = Return Value)
   - **Cast to WBP_InventorySlot** (Object = Slot)
   - **Update Slot Visual** (Target = saída "As WBP Inventory Slot" do Cast)
   - (Não precisa Set Trade Item ID nos slots do parceiro)

#### Ordem no grafo

```
Set PartnerGoldOffer (último Set antes dos loops)
    │
    ├──► For Each MySlotWidgets → Clear Slot
    │
    ├──► For Each PartnerSlotWidgets → Clear Slot
    │
    ├──► [NOVO] For Each MyItems → CreateInventorySlotFromTradeItem, Get MySlotWidgets[i], SetSlotData, Set TradeItemID, UpdateSlotVisual
    │
    ├──► [NOVO] For Each PartnerItems → CreateInventorySlotFromTradeItem, Get PartnerSlotWidgets[i], SetSlotData, UpdateSlotVisual
    │
    └──► Set Text (contador e gold)
```

### ParentTradeWidget

- **ParentTradeWidget** deve ser setado nos **meus slots** (MySlotWidgets), não nos do parceiro.
- Nos **meus slots**: `ParentTradeWidget = self` (WBP_Trade) — para aceitar drag do inventário.
- Nos **slots do parceiro**: **não** setar ParentTradeWidget (são somente leitura).

Se estiver setando ParentTradeWidget apenas nos PartnerSlotWidgets, inverta para setar nos **MySlotWidgets**.

### Se UpdateSlotVisual não existir no WBP_InventorySlot

Se o WBP_InventorySlot não tiver a função **Update Slot Visual**, crie-a no Blueprint do slot. Ela deve atualizar o ícone e o texto de quantidade a partir de `SlotData`. O C++ não chama mais UpdateSlotVisual automaticamente — o Blueprint deve chamar manualmente após SetSlotData.
