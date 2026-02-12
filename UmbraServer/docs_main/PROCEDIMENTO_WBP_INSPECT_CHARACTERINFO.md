# 📋 PROCEDIMENTO: Implementar Inspect no WBP_PlayerContextMenu

## 🎯 **OBJETIVO:**

Ao clicar no botão **Inspect** no `WBP_PlayerContextMenu`, abrir o `WBP_CharacterInfo` mostrando as informações do jogador selecionado (permitindo inspecionar outros jogadores).

---

## ✅ **PRÉ-REQUISITOS:**

1. ✅ Sistema C++ já implementado (`InspectPlayer`, `OnPlayerInspected`, `OnPlayerInspectedManual`, `OnPlayerInspectFailed`)
2. ✅ `WBP_CharacterInfo` já existe e funciona para mostrar o próprio personagem
3. ✅ `WBP_PlayerContextMenu` já tem o botão Inspect configurado

---

## 🔧 **MUDANÇAS IMPLEMENTADAS NO C++:**

### **1. Novo Delegate `OnPlayerInspectedManual`:**
- Criado delegate `FOnPlayerInspectedManual` que só dispara quando `InspectPlayer` é chamado manualmente (via botão Inspect)
- Use este delegate no Blueprint em vez de `OnPlayerInspected`

### **2. `InspectPlayer` agora aceita `bIsManualInspect`:**
- `InspectPlayer(int32 TargetPlayerID, bool bIsManualInspect = false)`
- Quando `bIsManualInspect = true` → dispara `OnPlayerInspectedManual`
- Quando `bIsManualInspect = false` → dispara apenas `OnPlayerInspected` (para processamento interno)

### **3. Chamadas atualizadas:**
- `HandlePlayerContextAction` (botão Inspect) → `InspectPlayer(PlayerID, true)` ✅
- `SelectPlayer` (automático) → `InspectPlayer(FoundPlayerID, false)` ✅
- `OnEquipItemRequestComplete` (automático) → `InspectPlayer(ActivePlayerID, false)` ✅

**Isso garante que o widget só seja criado quando o usuário clicar explicitamente em Inspect.**

---

## 🔧 **PARTE 1: Verificar Botão Inspect no Context Menu**

### **1.1 Verificar se o botão já está conectado**

**No `WBP_PlayerContextMenu`:**

1. Abra o widget no Editor
2. Verifique se existe o botão `BTN_Inspect` (ou nome similar)
3. Verifique se o **OnClicked** do botão já chama `Execute Context Action` com `Action = Inspect`

**Se NÃO estiver conectado:**

```
[BTN_Inspect OnClicked]
    │
    ├──► Get Player Controller
    │         │
    │         └──► Get Player Selection Component
    │                   │
    │                   └──► Execute Context Action
    │                             Action = Inspect
    │
    └──► Remove From Parent  (ou Set Visibility Collapsed, para fechar o menu)
```

---

## 🔧 **PARTE 2: Adaptar WBP_CharacterInfo para Inspeção**

### **2.1 Criar Variável para Armazenar Dados de Inspeção**

**No `WBP_CharacterInfo`:**

1. Vá para a aba **Variables**
2. Crie uma nova variável:
   - **Nome:** `InspectedPlayerInfo`
   - **Tipo:** `Umbra Character Info` (Structure)
   - **Instance Editable:** False
   - **Expose on Spawn:** False

### **2.2 Criar Função para Atualizar com Dados de Inspeção**

**No `WBP_CharacterInfo`:**

1. Vá para a aba **Functions**
2. Crie uma nova função:
   - **Nome:** `Update Inspected Player Info`
   - **Input:** `Player Info` (tipo `Umbra Character Info`)

**Implementação da função:**

```
[Update Inspected Player Info]
    Input: Player Info (Umbra Character Info)
    │
    ├──► Set InspectedPlayerInfo = Player Info
    │
    └──► Call Update Character Info Display
              Input: Player Info
```

**Nota:** A função `Update Character Info Display` já existe e funciona com `FUmbraCharacterInfo`, então pode ser reutilizada diretamente.

---

### **2.3 Equipped Items e Tooltips – Usar sempre o CharacterInfo que está sendo exibido**

Para que os **slots de itens equipados** apareçam preenchidos e os **tooltips** mostrem os itens do **jogador inspecionado** (e não do jogador local), o widget deve usar sempre o **mesmo** `CharacterInfo` que está na tela.

**Problema:** Se a grade de equipamentos usar `Get Current Character Info` ou buscar itens do jogador local, os slots ficam errados no modo inspeção. Se o **tooltip** chamar **Set Tooltip Data** com um struct obtido do Game Instance (em vez do InventorySlot que o slot já guarda), o tooltip mostrará o item do jogador local em vez do inspecionado.

---

#### **2.3.1 Detalhamento: função `Update Character Info Display`**

- A função recebe **um parâmetro**: `CharacterInfo` (tipo **Umbra Character Info** / `FUmbraCharacterInfo`).
- Esse parâmetro é o **único** que deve ser usado para preencher **todos** os dados na tela, incluindo a seção de itens equipados.

**O que verificar:**

1. No **Event Graph** da função `Update Character Info Display`:
   - O nó de entrada da função tem um pin de saída **CharacterInfo** (struct).
   - Esse pin já está ligado a um **Break Umbra Character Info** (ou similar) para preencher nome, nível, stats, etc. — mantenha isso.
   - **Importante:** a chamada que preenche os **slots de equipamento** deve usar **esse mesmo** `CharacterInfo` do parâmetro, e não o retorno de **Get Current Character Info**.

2. Se existir uma função auxiliar **`UpdateEquipmentSlots`**:
   - Ela também recebe **CharacterInfo** (Umbra Character Info).
   - **Update Character Info Display** deve chamar **UpdateEquipmentSlots** passando **o mesmo CharacterInfo** que ela recebeu como parâmetro.
   - Fluxo correto:
     ```
     [Update Character Info Display]
         Entrada: CharacterInfo
         │
         ├── (outros sets: nome, nível, stats, etc., a partir de CharacterInfo)
         │
         └── Call UpdateEquipmentSlots
                   CharacterInfo = (pin CharacterInfo da entrada de Update Character Info Display)
     ```
   - Ou seja: o pin **CharacterInfo** que entra em **UpdateEquipmentSlots** deve vir **diretamente** do pin **CharacterInfo** de saída do nó de entrada de **Update Character Info Display** (pode usar um “Knot” no meio se precisar ligar em vários lugares). **Não** ligue em **Get Current Character Info** nesse fluxo.

---

#### **2.3.2 Detalhamento: função `UpdateEquipmentSlots`**

- A função recebe **CharacterInfo** (Umbra Character Info).
- Ela é responsável por preencher a **grade/lista visual** dos slots de equipamento (ícones, etc.).

**O que fazer dentro de `UpdateEquipmentSlots`:**

1. Onde a **lista de itens equipados** é obtida para preencher os slots:
   - Use **Get Equipped Items Array** (função do **Umbra Game Instance**).
   - **Target:** Get Game Instance → Cast to Umbra Game Instance.
   - **Parâmetro CharacterInfo:** use **o pin de saída CharacterInfo do nó de entrada da própria função UpdateEquipmentSlots** (o parâmetro que a função recebeu).
   - **Não** use **Get Current Character Info** como entrada de **Get Equipped Items Array**.

2. Resumo do nó:
   ```
   [UpdateEquipmentSlots]
       Entrada: CharacterInfo
       │
       └── Get Game Instance
                 │
                 Cast to Umbra Game Instance
                 │
                 Get Equipped Items Array
                     CharacterInfo = (pin CharacterInfo da entrada de UpdateEquipmentSlots)
                     Return Value → (usar para iterar e preencher cada slot da UI)
   ```

3. Se hoje o grafo usa algo como **Get Current Character Info** → **Get Equipped Items Array**:
   - **Remova** a ligação de **Get Current Character Info** para **Get Equipped Items Array**.
   - Ligue o **pin CharacterInfo** do nó de **entrada** de **UpdateEquipmentSlots** ao pin **CharacterInfo** de **Get Equipped Items Array**.

Assim, tanto ao abrir com **C** (próprio personagem) quanto ao abrir por **Inspect**, quem chama **Update Character Info Display** passa o `CharacterInfo` correto; **UpdateEquipmentSlots** recebe esse mesmo struct e a lista de itens equipados virá sempre do personagem que está sendo exibido.

---

#### **2.3.2a O que verificar DENTRO da função UpdateEquipmentSlots (obrigatório)**

Quem chama **UpdateEquipmentSlots** já está passando **CharacterInfo** certo (OnCharacterInfoLoaded envia o do evento; Update Inspected Player Info envia InspectedPlayerInfo). Se os slots continuam vazios ou mostram o jogador errado, o erro está **dentro** do grafo da função **UpdateEquipmentSlots**.

**Passos no Editor:**

1. Abra o **WBP_CharacterInfo** e vá na aba **Functions**. Selecione a função **UpdateEquipmentSlots** (ou abra o grafo dela pelo Event Graph).
2. No grafo de **UpdateEquipmentSlots**, localize o **nó de entrada da função** (o primeiro nó à esquerda). Ele tem um pin de saída **CharacterInfo** (tipo Umbra Character Info). Esse é o **parâmetro** que a função recebe.
3. Procure **qualquer** nó que chame **Get Equipped Items Array** (função do **Umbra Game Instance**). Esse nó tem:
   - **Target** = referência ao Game Instance (Get Game Instance → Cast to Umbra Game Instance),
   - **CharacterInfo** = entrada que define **de qual personagem** virá a lista.
4. **Correção obrigatória:** o pin **CharacterInfo** de **Get Equipped Items Array** **tem que** estar ligado ao **CharacterInfo** do **nó de entrada da função UpdateEquipmentSlots** (o parâmetro).  
   - Se estiver ligado a **Get Current Character Info**, **CurrentCharacterInfo** do Game Instance, ou a qualquer variável que venha do Game Instance, **desconecte** e ligue ao pin **CharacterInfo** do nó de entrada de **UpdateEquipmentSlots**.
5. Se a função **não** tiver um nó **Get Equipped Items Array**: a lista de itens está vinda de outro lugar (por exemplo um **For Each** em cima de outra fonte). Nesse caso:
   - Adicione: **Get Game Instance** → **Cast to Umbra Game Instance** → **Get Equipped Items Array**.
   - Ligue o **CharacterInfo** de **Get Equipped Items Array** ao pin **CharacterInfo** do nó de entrada de **UpdateEquipmentSlots**.
   - Use o **Return Value** (array de equipamentos) para o loop que preenche os slots (ícone, etc.). Assim os ícones passam a vir do personagem cujo **CharacterInfo** foi passado para a função.

6. Se existir **mais de um** uso de “lista de equipamentos” no grafo (por exemplo um para ícones e outro para tooltip), **todos** devem usar a lista obtida de **Get Equipped Items Array(CharacterInfo)** com o **CharacterInfo** da entrada da função, nunca **Get Current Character Info**.

**Resumo:** Dentro de **UpdateEquipmentSlots**, a **única** fonte da lista de itens equipados deve ser **Get Equipped Items Array**, e o argumento **CharacterInfo** desse nó **obrigatoriamente** deve ser o **parâmetro CharacterInfo** da própria função **UpdateEquipmentSlots**.

**Se você já verificou e o grafo está assim (CharacterInfo do parâmetro → Knot → Get Equipped Items Array):** então a função **UpdateEquipmentSlots** está correta. Nesse caso, o problema costuma ser um dos dois abaixo.

---

#### **2.3.2b Verificar: Update Character Info Display chama UpdateEquipmentSlots?**

Quando a janela abre por **Inspect**, o fluxo é: **Update Inspected Player Info(Player Info)** → deve chamar **Update Character Info Display(Player Info)**. Para os slots de equipamento serem preenchidos, **Update Character Info Display** **precisa** chamar **UpdateEquipmentSlots** com o **mesmo CharacterInfo** que ela recebeu.

**O que verificar no grafo de Update Character Info Display:**

1. Além de preencher textos (nome, nível, stats), existe uma chamada a **UpdateEquipmentSlots**?
2. Se existir, o pin **CharacterInfo** dessa chamada está ligado ao **CharacterInfo** do **nó de entrada** de **Update Character Info Display** (ou a um Knot que vem desse nó)?
3. Se **não** existir chamada a **UpdateEquipmentSlots** dentro de **Update Character Info Display**, **adicione**: ao final do fluxo (depois de setar os textos), chame **UpdateEquipmentSlots** e ligue o pin **CharacterInfo** ao **CharacterInfo** de entrada da função.

Assim, ao abrir por Inspect, **Update Inspected Player Info** → **Update Character Info Display(InspectedPlayerInfo)** → **UpdateEquipmentSlots(InspectedPlayerInfo)** e os slots passam a usar os dados do inspecionado.

---

#### **2.3.2c Ícones dos slots: usar o InventorySlot recebido (não o Game Instance)**

**Problema:** Os tooltips estão corretos (dados do inspecionado), mas os **ícones** dos itens equipados mostram os itens do **jogador que inspeciona** em vez do **jogador inspecionado**. Isso acontece quando o widget do slot desenha o ícone consultando o Game Instance (ex.: **Get Equipped Item Value(Slot)**), que sempre retorna o personagem **local**.

**UpdateEquipmentSlots** já chama **Set Head**, **Set Chest**, etc. passando o **InventorySlot** correto (do CharacterInfo do inspecionado). O slot precisa usar **esse mesmo** dado para o **ícone**, não o Game Instance.

**Onde corrigir (widget de cada slot de equipamento):**

1. **Identificar o widget do slot**  
   Pode ser um widget filho do WBP_CharacterInfo (ex.: um widget por slot Head, Chest, Feet, etc.) ou um widget reutilizável (ex.: WBP_EquipmentSlot) que recebe **Set Head(InventorySlot)** (ou Set Chest, Set Feet, etc.).

2. **Onde o ícone é setado**  
   Procure no Blueprint do slot:
   - Uma função do tipo **Set Head(InventorySlot)** (ou Set Chest, etc.) que **guarda** o InventorySlot em uma variável (ex.: `SlotData`, `CurrentInventorySlot`).
   - Em seguida, onde a **imagem/brush do ícone** é definida:
     - Se houver **Set Brush from Texture** ou **Set Brush** em um **Image** do slot, a **textura/ícone** deve vir do **InventorySlot que foi passado**, por exemplo:
       - **Break Umbra Inventory Slot** (ou a variável onde você guardou o slot) → **Item Template** → **Item Icon** (Texture2D).
       - Use esse **Item Icon** no **Set Brush from Texture** (ou **Set Brush**) do **Image** do slot.
     - **Não** use **Get Equipped Item Value(Equipment Slot)** ou **Get Equipped Item Value From Character Info(Get Current Character Info(), Slot)** para definir o ícone — isso traz o item do jogador local.

3. **Fluxo correto no slot (ex.: Set Head)**  
   - Entrada: **InventorySlot** (struct).
   - **Set** uma variável do widget (ex.: `CurrentSlotData`) = **InventorySlot**.
   - **Break Umbra Inventory Slot** → pegar **Item Template** → **Item Icon**.
   - Se **Item Icon** for válido: **Set Brush from Texture** no **Image** do slot, usando **Item Icon**.
   - Se o slot estiver vazio (InventorySlot vazio ou ItemTemplateID 0), **Set Brush** com brush vazio/default para não mostrar ícone do jogador local.

4. **Se o ícone for definido por Binding no Designer**  
   Se no **Designer** o **Image** do slot tem um **Binding** (ex.: uma função que retorna o brush):
   - Essa função **não** deve chamar **Get Equipped Item Value** no Game Instance.
   - Ela deve retornar o brush a partir da **variável do widget** que guarda o InventorySlot atual (ex.: `CurrentSlotData.ItemTemplate.ItemIcon`). Assim o ícone reflete sempre o dado que foi passado em **Set Head** / **Set Chest** / etc.

**Resumo:** O ícone do slot deve ser preenchido **apenas** com **InventorySlot.ItemTemplate.ItemIcon** do struct que o slot recebe em **Set Head** (ou Set Chest, etc.). Qualquer uso de **Get Equipped Item Value(Slot)** ou de **Get Current Character Info** para definir o ícone faz o slot mostrar o equipamento do jogador local.

---

#### **2.3.2d Resumo: widgets de slot (ícone e tooltip do dado recebido)**

**UpdateEquipmentSlots** chama **Set Head**, **Set Chest**, **Set Feet**, etc. passando o **InventorySlot** do loop (dado que veio de **Get Equipped Items Array(CharacterInfo)**). Ou seja, o **dado correto** está sendo passado para cada slot.

- **Ícone:** o slot usa o **InventorySlot** recebido em **Set Head** / **Set Chest** (ex.: **ItemTemplate.ItemIcon**) para **Set Brush** / **Set Brush from Texture**. Não usar **Get Equipped Item Value(Slot)** do Game Instance para o ícone.
- **Tooltip:** o tooltip é montado por **Set Tooltip Data(InSlotData)**. O **InSlotData** deve ser o **mesmo InventorySlot** que o slot guarda (o recebido em Set Head/Set Chest). Não passar como InSlotData um struct obtido do Game Instance — ver 2.3.3.

---

#### **2.3.3 Tooltip: Set Tooltip Data(InSlotData) deve receber o InventorySlot do slot (WBP_EquipmentSlot)**

**Como o tooltip funciona no seu código:**  
A função **Set Tooltip Data** tem entrada **InSlotData** (tipo **Umbra Inventory Slot**). Dentro dela: Break **InSlotData** → Template → Break template; do template, Break **Stats**; do Stats, Break **Combat Stats**, **Base Stats**, **Bonus Stats**. Esses valores são usados em **Format Text** para definir os textos do tooltip. Ou seja, o tooltip é montado **a partir do struct que é passado em InSlotData**.

**Problema:** Ao inspecionar, o tooltip mostra os itens do jogador que inspeciona em vez dos itens do jogador inspecionado. Isso só pode acontecer se **Set Tooltip Data** estiver sendo chamada com um **InSlotData** que vem do personagem errado — por exemplo, um InventorySlot obtido do jogador local em vez do InventorySlot que o slot está exibindo (o que veio de **Set Head** / **Set Chest** no **UpdateEquipmentSlots**).

**Solução (única necessária):**  
Quem **chama** **Set Tooltip Data** (por exemplo no hover do slot) deve passar em **InSlotData** o **mesmo InventorySlot** que esse slot já guarda — ou seja, a variável do widget que foi preenchida quando **Set Head(InventorySlot)** ou **Set Chest(InventorySlot)** (etc.) foi chamado pelo **UpdateEquipmentSlots**. Esse struct já é o do personagem que está na tela (próprio ou inspecionado), porque o **UpdateEquipmentSlots** recebe o **CharacterInfo** correto e passa os **InventorySlot** desse CharacterInfo para cada slot.

**Passos no WBP_EquipmentSlot:**

1. **Onde o slot guarda o InventorySlot**  
   Quando **Set Head**, **Set Chest**, **Set Feet**, etc. são chamados, o slot recebe um **InventorySlot** e deve guardá‑lo em uma variável (ex.: `SlotData`, `CurrentSlotData`, `StoredInventorySlot`). Essa mesma variável já é usada para o ícone. Anote o nome dessa variável.

2. **Onde Set Tooltip Data é chamada**  
   Procure no **WBP_EquipmentSlot** (e, se aplicável, no parent que chama no slot) **quem chama Set Tooltip Data** e com **que valor** no pin **InSlotData** (ex.: no **OnHovered** ou no evento que abre o tooltip).

3. **Correção**  
   Garanta que essa chamada use **sempre** a variável do passo 1 como **InSlotData**:
   - **Chamada:** **Set Tooltip Data** → pin **InSlotData** = **Get Variable** → [variável que guarda o InventorySlot do slot, a mesma usada para o ícone].
   - **Não** use como InSlotData nenhum struct obtido do Game Instance (ex.: Get Equipped Item Value, Get Current Character Info, etc.) nesse momento. O slot já tem o struct correto guardado desde o **Set Head** / **Set Chest**.

4. **Se Set Tooltip Data for chamada no parent (WBP_CharacterInfo)**  
   Se quem chama **Set Tooltip Data** é o **parent** e ele passa um struct que buscou do Game Instance para o slot, mude para: o **slot** deve informar ao parent o **seu** struct guardado (ex.: função **Get Slot Data** no slot que retorna essa variável), e o parent chama **Set Tooltip Data(InSlotData = retorno de Get Slot Data do slot)**. Ou então: o evento de hover ocorre no slot, e o **próprio slot** chama **Set Tooltip Data** passando a **sua** variável de InventorySlot — assim não depende do parent buscar dado errado.

**Resumo:** O tooltip já é montado corretamente a partir de **InSlotData** (Break → Template → Stats → Format Text). Basta garantir que **InSlotData** seja sempre o **InventorySlot guardado no slot** (o que veio de Set Head/Set Chest), e não um struct vindo do jogador local. Assim, ao inspecionar, o tooltip mostrará os itens do jogador inspecionado.

---

#### **2.3.4 Evento OnEquipmentUpdated (bind de OnItemMoved) – Único uso de CurrentCharacterInfo**

O único lugar que usa **Get Current Character Info** (variável **CurrentCharacterInfo** do Game Instance) no WBP_CharacterInfo é o **evento OnEquipmentUpdated**, ligado ao delegate **OnItemMoved** do Game Instance. Esse evento dispara quando o **jogador local** move um item (equipar/desequipar/mover no inventário).

**Problema:** Hoje o grafo faz:
1. **OnEquipmentUpdated** (then) → PrintString → **Update Character Info Display**(**CurrentCharacterInfo**) → **UpdateEquipmentSlots**(**CurrentCharacterInfo**).

Assim, sempre que o jogador local move um item, a tela é atualizada com **CurrentCharacterInfo**. Se a janela estiver aberta em **modo inspeção** (mostrando outro jogador), essa atualização **troca** o que está na tela para os dados do personagem local e os slots de equipamento passam a mostrar o equipamento do local em vez do inspecionado.

**Solução:** Só atualizar com **CurrentCharacterInfo** quando o widget **não** estiver em modo inspeção. Quando estiver mostrando um jogador inspecionado, **não** chamar Update Character Info Display / UpdateEquipmentSlots nesse evento (o inspecionado não está movendo itens na sua tela).

**Passos no Blueprint:**

1. No **Event Graph**, localize o nó do evento **OnEquipmentUpdated** (Custom Event) e a sequência que termina em **Update Character Info Display** e **UpdateEquipmentSlots**.
2. **Antes** de chamar **Update Character Info Display**, insira um **Branch**:
   - **Condition:** verificar se estamos em modo inspeção.
     - **Get Variable** → **Inspected Player Info** (do WBP_CharacterInfo).
     - **Break Umbra Character Info** (ou acessar **Player ID** do struct).
     - **Comparação:** **Player ID > 0** (ou **Greater (Integer)** com 0).
     - Use o resultado como **Condition** do Branch.
   - **False (estamos mostrando o próprio personagem):** executa a lógica atual:
     - PrintString (opcional),
     - **Get Variable** → **My Game Instance** → **Get Current Character Info** (ou variável **CurrentCharacterInfo** do Game Instance),
     - **Update Character Info Display**(**CurrentCharacterInfo**),
     - **UpdateEquipmentSlots**(**CurrentCharacterInfo**).
   - **True (estamos em modo inspeção):** não faça nada (não chame Update Character Info Display nem UpdateEquipmentSlots).

3. Fluxo final desejado:

   ```
   [OnEquipmentUpdated]
       then
       │
       Get Variable → Inspected Player Info
       │
       Break Umbra Character Info → Player ID
       │
       Branch (Condition: Player ID > 0)
       │
       ├─ True (modo inspeção)  → (nada; não atualizar com dados do local)
       │
       └─ False (próprio personagem)
             │
             PrintString (opcional)
             │
             Get Variable → My Game Instance
             │
             Get Current Character Info  (ou CurrentCharacterInfo do Game Instance)
             │
             Update Character Info Display (CharacterInfo = CurrentCharacterInfo)
             │
             UpdateEquipmentSlots (CharacterInfo = CurrentCharacterInfo)
   ```

Assim, o único uso de **CurrentCharacterInfo** fica restrito ao caso “janela mostrando o próprio personagem e jogador moveu um item”; em modo inspeção a janela não é sobrescrita pelos dados do personagem local.

---

#### **2.3.5 Checklist rápido**

- [ ] **Update Character Info Display:** quando chamada por **Update Inspected Player Info** ou **OnCharacterInfoLoaded**, recebe o **CharacterInfo** correto (InspectedPlayerInfo ou do evento) e chama **UpdateEquipmentSlots** com esse parâmetro.
- [ ] **UpdateEquipmentSlots:** **Get Equipped Items Array** usa o **CharacterInfo** do parâmetro da função (pin de entrada), com Target = Game Instance.
- [ ] **OnEquipmentUpdated:** só chama **Update Character Info Display** e **UpdateEquipmentSlots** com **CurrentCharacterInfo** quando **não** está em modo inspeção (**Branch** em **InspectedPlayerInfo.Player ID > 0**; executar a atualização apenas no ramo **False**).
- [ ] **Tooltip dos slots:** onde **Set Tooltip Data** é chamada (ex.: OnHovered no slot), passar em **InSlotData** a variável do slot que guarda o **InventorySlot** (a mesma usada para o ícone). Não usar struct vindo do Game Instance — ver 2.3.3.

Com isso, a parte 2.3 fica aplicada: equipamentos e tooltips usam sempre o personagem que está sendo exibido (o struct que o slot recebeu em Set Head/Set Chest), e o evento de “item movido” não sobrescreve a tela quando a janela está em modo inspeção.

---

## 🔧 **PARTE 3: Escutar OnPlayerInspectedManual no Blueprint**

**⚠️ IMPORTANTE:** Use `OnPlayerInspectedManual` em vez de `OnPlayerInspected`! O `OnPlayerInspectedManual` só dispara quando o usuário clica explicitamente no botão Inspect, evitando que o widget seja criado quando `InspectPlayer` é chamado automaticamente.

### **3.1 Onde Escutar o Delegate**

**Opção A: No Player Controller (Recomendado)**

**No Blueprint do Player Controller:**

1. **Event BeginPlay:**
   ```
   [Event BeginPlay]
       │
       └──► Get Game Instance
                 │
                 └──► Cast to Umbra Game Instance
                           │
                           └──► Bind Event to OnPlayerInspectedManual  ← USAR ESTE!
                                     │
                                     └──► [Custom Event: OnPlayerInspectedManual_Event]
                                               Input: Player ID (int32)
                                               Input: Player Info (Umbra Character Info)
   ```

2. **Criar Variável para Armazenar o Widget:**
   
   **No Player Controller (ou WBP_PlayerHUD):**
   
   - Vá para a aba **Variables**
   - Clique em **+ Variable**
   - Configure:
     - **Nome:** `InspectedCharacterInfoWidget`
     - **Tipo:** `WBP Character Info` (Object Reference) - selecione o tipo do widget `WBP_CharacterInfo`
     - **Instance Editable:** False (não precisa ser editável)
     - **Expose on Spawn:** False
     - **Category:** (opcional) "UI" ou "Widgets"
   
   **Como encontrar o tipo correto:**
   - Ao criar a variável, no dropdown de tipo, procure por `WBP Character Info` ou `WBP_CharacterInfo`
   - Se não aparecer, digite "WBP" e procure na lista
   - O tipo deve ser uma **Object Reference** (não Structure ou Enum)

3. **Custom Event: OnPlayerInspected_Event**
   ```
   [OnPlayerInspected_Event]
       Input: Player ID (int32)
       Input: Player Info (Umbra Character Info)
       │
       ├──► Get Variable → InspectedCharacterInfoWidget
       │        │
       │        └──► Is Valid?
       │                 │
       │                 ├──► [True] → Remove From Parent (InspectedCharacterInfoWidget)
       │                 │        │
       │                 │        └──► Set InspectedCharacterInfoWidget = null (opcional, limpar referência)
       │                 │
       │                 └──► [False] → (continuar)
       │
       ├──► Create Widget
       │        Class: WBP_CharacterInfo
       │        OwningPlayer: Get Player Controller (0) ou Get Owning Player
       │
       ├──► Add to Viewport
       │        Target: Return Value (Create Widget)
       │
       ├──► Set InspectedCharacterInfoWidget = Return Value (Create Widget)
       │
       └──► Call Update Inspected Player Info
                 Target: InspectedCharacterInfoWidget (Get Variable)
                 Input: Player Info
   ```

   **Passo a passo detalhado no Blueprint:**
   
   1. **No Custom Event `OnPlayerInspected_Event`:**
      - **Right Click** → **Get Variable** → Selecione `InspectedCharacterInfoWidget`
      - **Right Click** → **Is Valid?**
      - Conecte `InspectedCharacterInfoWidget` ao input do `Is Valid?`
   
   2. **Se Is Valid retornar True:**
      - **Right Click** → **Remove from Parent**
      - Conecte `InspectedCharacterInfoWidget` ao input `Target` do `Remove from Parent`
      - (Opcional) **Right Click** → **Set** → Selecione `InspectedCharacterInfoWidget`
      - Conecte `null` (ou deixe vazio) ao input do `Set`
   
   3. **Criar o Widget:**
      - **Right Click** → **Create Widget**
      - **Class:** Selecione `WBP_CharacterInfo` no dropdown
      - **OwningPlayer:** **Get Player Controller (0)** ou **Get Owning Player**
   
   4. **Adicionar ao Viewport:**
      - **Right Click** → **Add to Viewport**
      - Conecte **Return Value** (do Create Widget) ao input `Target` do `Add to Viewport`
   
   5. **Armazenar Referência:**
      - **Right Click** → **Set** → Selecione `InspectedCharacterInfoWidget`
      - Conecte **Return Value** (do Create Widget) ao input do `Set`
   
   6. **Atualizar Widget:**
      - **Right Click** → **Call Function** → Procure por `Update Inspected Player Info`
      - **Target:** Conecte `InspectedCharacterInfoWidget` (Get Variable)
      - **Input:** Conecte `Player Info` (do input do Custom Event)

**Opção B: No WBP_PlayerHUD (Recomendado - Já está implementado)**

O `WBP_PlayerHUD` já escuta outros eventos de seleção, então faz sentido escutar `OnPlayerInspectedManual` aqui também:

**⚠️ IMPORTANTE:** Use `OnPlayerInspectedManual` em vez de `OnPlayerInspected`! O `OnPlayerInspectedManual` só dispara quando o usuário clica explicitamente no botão Inspect, evitando que o widget seja criado quando `InspectPlayer` é chamado automaticamente pelo `SelectPlayer`.

1. **Event Construct (adicionar ao existente):**
   ```
   [Event Construct]
       │
       ├──► (bindings existentes: OnPlayerSelected, OnPlayerContextMenuRequested, etc.)
       │
       └──► Get Game Instance
                 │
                 └──► Cast to Umbra Game Instance
                           │
                           └──► Bind Event to OnPlayerInspectedManual  ← USAR ESTE!
                                     │
                                     └──► [Custom Event: OnPlayerInspectedManual_Event]
                                               Input: Player ID (int32)
                                               Input: Player Info (Umbra Character Info)
   ```

2. **Custom Event: OnPlayerInspected_Event (no WBP_PlayerHUD):**
   ```
   [OnPlayerInspected_Event]
       Input: Player ID (int32)
       Input: Player Info (Umbra Character Info)
       │
       ├──► Get Variable → InspectedCharacterInfoWidget
       │        │
       │        └──► Is Valid?
       │                 │
       │                 ├──► [True] → Remove From Parent (InspectedCharacterInfoWidget)
       │                 │
       │                 └──► [False] → (continuar)
       │
       ├──► Create Widget
       │        Class: WBP_CharacterInfo
       │        OwningPlayer: Get Player Controller (0)
       │
       ├──► Add to Viewport
       │        Target: Return Value (Create Widget)
       │
       ├──► Set InspectedCharacterInfoWidget = Return Value (Create Widget)
       │
       └──► Call Update Inspected Player Info
                 Target: InspectedCharacterInfoWidget (Get Variable)
                 Input: Player Info
   ```

3. **Variável no WBP_PlayerHUD:**
   - **Nome:** `InspectedCharacterInfoWidget`
   - **Tipo:** `WBP Character Info` (Object Reference)
   - **Instance Editable:** False

**IMPORTANTE:** 
- O `WBP_CharacterInfo` criado aqui **NÃO** precisa escutar `OnPlayerInspected` no próprio widget
- Os dados já vêm prontos do `WBP_PlayerHUD`, então apenas chame `Update Inspected Player Info` diretamente

---

## 🔧 **PARTE 4: Tratar Erros de Inspeção (Opcional)**

### **4.1 Escutar OnPlayerInspectFailed**

**No mesmo lugar onde escutou `OnPlayerInspectedManual`:**

```
[Event BeginPlay] ou [Event Construct]
    │
    └──► Get Game Instance
              │
              └──► Cast to Umbra Game Instance
                        │
                        └──► Bind Event to OnPlayerInspectFailed
                                  │
                                  └──► [Custom Event: OnPlayerInspectFailed_Event]
                                            Input: Error Message (String)
```

**Custom Event: OnPlayerInspectFailed_Event**

```
[OnPlayerInspectFailed_Event]
    Input: Error Message (String)
    │
    └──► Print String
              In String: Error Message
              Print to Screen: True
              Text Color: Red
```

**Ou criar um widget de erro personalizado.**

---

## 🔧 **PARTE 5: Diferenciar Inspeção de Próprio Personagem**

### **5.1 Fluxos Separados (Solução Simples)**

**IMPORTANTE:** Não é necessário usar `bIsInspectingOtherPlayer` porque os dois fluxos são completamente separados:

1. **Fluxo 1 - Próprio Personagem (WBP_CharacterInfo):**
   - `Event Construct` → Bind `OnCharacterInfoLoaded` → `Load Character Info`
   - `OnCharacterInfoLoaded_Event` → `Update Character Info Display`
   - **Usado quando:** Jogador pressiona tecla **C** ou abre o próprio personagem

2. **Fluxo 2 - Inspeção de Outro Jogador (WBP_PlayerHUD → WBP_CharacterInfo):**
   - `WBP_PlayerHUD` escuta `OnPlayerInspected` → Cria `WBP_CharacterInfo` → Chama `Update Inspected Player Info`
   - `Update Inspected Player Info` → `Update Character Info Display`
   - **Usado quando:** Jogador clica em "Inspect" no menu de contexto

**Os dois fluxos não se misturam**, então não precisa de flag para diferenciá-los.

### **5.2 Estrutura Correta do WBP_CharacterInfo**

**No `WBP_CharacterInfo`:**

1. **Event Construct (mantém como está - para próprio personagem):**
```
[Event Construct]
    │
    ├──► Get Game Instance
    │         │
    │         └──► Cast to Umbra Game Instance
    │                   │
    │                   ├──► Bind Event to OnCharacterInfoLoaded
    │                   │         │
    │                   │         └──► [Custom Event: OnCharacterInfoLoaded_Event]
    │                   │                   Input: Character Info
    │                   │
    │                   └──► Bind Event to OnCharacterInfoFailed
    │                             │
    │                             └──► [Custom Event: OnCharacterInfoFailed_Event]
    │                                       Input: Error Message
    │
    └──► Load Character Info (só se for próprio personagem)
              Target: Cast to Umbra Game Instance
```

**Nota:** O `Load Character Info` só deve ser chamado quando o widget é criado para mostrar o próprio personagem. Quando é criado para inspeção, não deve chamar `Load Character Info`.

2. **OnCharacterInfoLoaded_Event (mantém como está):**
```
[OnCharacterInfoLoaded_Event]
    Input: Character Info
    │
    └──► Call Update Character Info Display
              Input: Character Info
```

3. **Update Inspected Player Info (nova função - para inspeção):**
```
[Update Inspected Player Info]
    Input: Player Info (Umbra Character Info)
    │
    └──► Call Update Character Info Display
              Input: Player Info
```

**Como funciona:**
- Quando `WBP_PlayerHUD` cria o widget para inspeção, ele **não** chama `Load Character Info`
- Ele apenas chama `Update Inspected Player Info` diretamente com os dados já recebidos de `OnPlayerInspected`
- A função `Update Inspected Player Info` simplesmente repassa os dados para `Update Character Info Display`

### **5.3 Estrutura Correta do WBP_PlayerHUD**

**No `WBP_PlayerHUD`:**

1. **Event Construct (adicionar bind de OnPlayerInspected):**
```
[Event Construct]
    │
    ├──► (bindings existentes de OnPlayerSelected, OnPlayerContextMenuRequested, etc.)
    │
    └──► Get Game Instance
              │
              └──► Cast to Umbra Game Instance
                        │
                        └──► Bind Event to OnPlayerInspected
                                  │
                                  └──► [Custom Event: OnPlayerInspected_Event]
                                            Input: Player ID (int32)
                                            Input: Player Info (Umbra Character Info)
```

2. **OnPlayerInspectedManual_Event (no WBP_PlayerHUD):**
   ```
   [OnPlayerInspectedManual_Event]
       Input: Player ID (int32)
       Input: Player Info (Umbra Character Info)
       │
       ├──► Get Variable → InspectedCharacterInfoWidget
       │        │
       │        └──► Is Valid?
       │                 │
       │                 ├──► [True] → Remove From Parent (InspectedCharacterInfoWidget)
       │                 │
       │                 └──► [False] → (continuar)
       │
       ├──► Make Literal Bool
       │        Value: True
       │        │
       │        └──► Create Widget
       │                 Class: WBP_CharacterInfo
       │                 OwningPlayer: Get Player Controller (0)
       │                 bIsInspectingMode: True  ← CONECTAR AQUI!
       │
       ├──► Add to Viewport
       │        Target: Return Value (Create Widget)
       │
       ├──► Set InspectedCharacterInfoWidget = Return Value (Create Widget)
       │
       └──► Call Update Inspected Player Info
                 Target: InspectedCharacterInfoWidget (Get Variable)
                 Input: Player Info
   ```

**IMPORTANTE:** 
- Use `OnPlayerInspectedManual` em vez de `OnPlayerInspected` - isso garante que o widget só seja criado quando o usuário clicar explicitamente em Inspect
- O widget criado para inspeção **NÃO** deve chamar `Load Character Info` no `Event Construct` (por isso usamos `bIsInspectingMode = True`)
- Os dados já vêm prontos em `OnPlayerInspectedManual`, então apenas chame `Update Inspected Player Info`

**⚠️ ATENÇÃO:** O `Event Construct` do `WBP_CharacterInfo` sempre executa quando o widget é criado. Se ele chamar `Load Character Info`, isso vai tentar carregar dados do próprio jogador mesmo quando o widget é criado para inspeção.

**Soluções:**

**Solução 1 (Recomendada - Mais Simples):**
- Deixe o `Event Construct` chamar `Load Character Info` normalmente
- Quando `WBP_PlayerHUD` cria o widget para inspeção, chame `Update Inspected Player Info` **imediatamente após criar**
- Isso sobrescreve os dados antes que `Load Character Info` complete (é assíncrono)
- Quando `OnCharacterInfoLoaded` disparar depois, pode simplesmente ignorar (os dados de inspeção já estão visíveis)

**Solução 2 (Mais Limpa - Usar Flag) - RECOMENDADA:**

### **PASSO 1: Criar Variável bIsInspectingMode no WBP_CharacterInfo**

1. **Abra o `WBP_CharacterInfo` no Editor**
2. **Vá para a aba Variables** (ícone de variável no topo)
3. **Clique em + Variable** (botão verde no canto superior direito)
4. **Configure a variável:**
   - **Nome:** `bIsInspectingMode`
   - **Tipo:** Boolean
   - **Default Value:** False
   - **Instance Editable:** False
   - **Expose on Spawn:** True ← **CRÍTICO: Deve estar marcado!**
   - **Category:** (opcional) "UI" ou "Inspect"

### **PASSO 2: Modificar Event Construct do WBP_CharacterInfo**

**No `WBP_CharacterInfo` → Event Graph → Event Construct:**

1. **Encontre o nó que chama `Load Character Info`**
2. **Antes de chamar `Load Character Info`, adicione uma verificação:**

```
[Event Construct]
    │
    ├──► Get Game Instance
    │         │
    │         └──► Cast to Umbra Game Instance
    │                   │
    │                   ├──► Bind Event to OnCharacterInfoLoaded
    │                   │         │
    │                   │         └──► OnCharacterInfoLoaded_Event
    │                   │
    │                   └──► Bind Event to OnCharacterInfoFailed
    │                             │
    │                             └──► OnCharacterInfoFailed_Event
    │
    ├──► Get Variable → bIsInspectingMode
    │        │
    │        └──► Branch (ou If Then Else)
    │                 │
    │                 ├──► [False] → Load Character Info
    │                 │        Target: Cast to Umbra Game Instance
    │                 │
    │                 └──► [True] → (NÃO fazer nada - não chamar Load Character Info)
```

**Passo a passo no Blueprint:**

1. **No Event Construct, encontre onde `Load Character Info` é chamado**
2. **Right Click** → **Get Variable** → Selecione `bIsInspectingMode`
3. **Right Click** → **Branch** (ou **If Then Else**)
4. **Conecte `bIsInspectingMode` ao input `Condition` do Branch**
5. **Conecte o `then` (False) do Branch ao nó que chama `Load Character Info`**
6. **Remova a conexão direta de `Bind Event to OnCharacterInfoFailed` para `Load Character Info`**
7. **O `else` (True) do Branch não precisa conectar nada** (não faz nada quando é inspeção)

### **PASSO 3: Modificar Create Widget no WBP_PlayerHUD**

**No `WBP_PlayerHUD` → Event Graph → OnPlayerInspected_Event:**

1. **Encontre o nó `Create Widget`**
2. **No nó `Create Widget`, você verá um pino `bIsInspectingMode`** (porque está Expose on Spawn)
3. **Conecte um nó `Make Literal Bool` com valor `True` ao pino `bIsInspectingMode`**

**Passo a passo:**

1. **No `Create Widget` (que cria WBP_CharacterInfo), você verá um pino extra chamado `bIsInspectingMode`**
2. **Right Click** → **Make Literal Bool**
3. **No `Make Literal Bool`, defina o valor como `True`**
4. **Conecte o output do `Make Literal Bool` ao pino `bIsInspectingMode` do `Create Widget`**

**Se o pino não aparecer:**
- Verifique se `bIsInspectingMode` tem **Expose on Spawn: True** no `WBP_CharacterInfo`
- Recompile o `WBP_CharacterInfo`
- Feche e reabra o `WBP_PlayerHUD`

### **PASSO 4: Verificar que Update Inspected Player Info está sendo chamado**

**No `WBP_PlayerHUD` → OnPlayerInspected_Event:**

Certifique-se de que após criar o widget, você está chamando `Update Inspected Player Info`:

```
Create Widget (bIsInspectingMode = True)
    ↓
Add to Viewport
    ↓
Set InspectedCharacterInfoWidget = (widget criado)
    ↓
Update Inspected Player Info
    Target: InspectedCharacterInfoWidget
    Input: Player Info (do evento)
```

---

## 🔍 **PROBLEMA 2: Widget Abre ao Clicar no Actor**

**Possível causa:** Algum evento está sendo disparado incorretamente ao clicar no actor.

### **Verificações:**

1. **Verifique se `OnPlayerSelected` está criando o widget:**
   - No `WBP_PlayerHUD`, verifique o `OnPlayerSelected_Event`
   - Ele **NÃO deve** criar `WBP_CharacterInfo`
   - Ele deve apenas criar `WBP_SelectedPlayerInfo`

2. **Verifique se há múltiplos binds de `OnPlayerInspected`:**
   - No `WBP_PlayerHUD`, verifique o `Event Construct`
   - Deve haver apenas **UM** bind de `OnPlayerInspected`
   - Se houver múltiplos, remova os duplicados

3. **Verifique se o botão Inspect está conectado corretamente:**
   - No `WBP_PlayerContextMenu`, verifique se `BTN_Inspect` está chamando `Execute Context Action` com `Action = Inspect`
   - Não deve chamar diretamente `InspectPlayer` ou criar o widget

4. **Adicione logs para debug:**
   - No `OnPlayerInspected_Event`, adicione um `Print String` no início:
     - "🔍 OnPlayerInspected_Event chamado - PlayerID: " + ToString(PlayerID)
   - Isso ajuda a identificar quando e por que o evento está sendo disparado

### **5.4 Solução Alternativa: Criar Widget Sem Load Character Info**

Se você quiser evitar que o `Event Construct` do `WBP_CharacterInfo` chame `Load Character Info` quando for criado para inspeção, pode:

**Opção A:** Criar uma variável `bIsInspectingMode` no `WBP_CharacterInfo` e passar via **Expose on Spawn**:

1. **No WBP_CharacterInfo:**
   - Criar variável `bIsInspectingMode` (Boolean, **Expose on Spawn: True**)

2. **No Event Construct:**
```
[Event Construct]
    │
    ├──► Get bIsInspectingMode
    │        │
    │        ├──► [False] → (chamar Load Character Info normalmente)
    │        │
    │        └──► [True] → (NÃO chamar Load Character Info)
```

3. **No WBP_PlayerHUD ao criar widget:**
```
Create Widget
    Class: WBP_CharacterInfo
    bIsInspectingMode: True  ← PASSAR TRUE AQUI
```

**Opção B (Mais Simples - Recomendada):** Não usar flag. O `WBP_CharacterInfo` sempre chama `Load Character Info` no `Event Construct`, mas quando é criado para inspeção, o `WBP_PlayerHUD` imediatamente chama `Update Inspected Player Info` que sobrescreve os dados. Isso funciona porque:
- `Load Character Info` é assíncrono (leva tempo)
- `Update Inspected Player Info` é síncrono (atualiza imediatamente)
- Quando `OnCharacterInfoLoaded` disparar depois, pode simplesmente ignorar se já tiver dados de inspeção

**Recomendação:** Use a **Opção B** (mais simples) ou a **Opção A** se quiser evitar carregamento desnecessário.

---

## 🔧 **PARTE 6: Botão Fechar**

### **6.1 Verificar se já existe**

**No `WBP_CharacterInfo`:**

O botão `Button_Close` já deve estar configurado para fechar o widget:

```
[Button_Close OnClicked]
    │
    └──► Remove From Parent (self)
```

Se não existir, adicione um botão de fechar e configure o OnClicked acima.

---

## 📋 **CHECKLIST FINAL:**

### **WBP_PlayerContextMenu:**
- [ ] Botão `BTN_Inspect` existe
- [ ] `BTN_Inspect OnClicked` → `Execute Context Action` com `Action = Inspect`
- [ ] Menu fecha após clicar (Remove From Parent ou Set Visibility Collapsed)

### **WBP_CharacterInfo:**
- [ ] Função `Update Inspected Player Info` criada
- [ ] Função `Update Inspected Player Info` recebe `Player Info` (Umbra Character Info) e chama `Update Character Info Display`
- [ ] `Event Construct` mantém bind de `OnCharacterInfoLoaded` (para próprio personagem)
- [ ] (Opcional) Variável `bIsInspectingMode` criada com **Expose on Spawn: True** se usar Solução 2

### **WBP_PlayerHUD:**
- [ ] Escuta `OnPlayerInspectedManual` no `Event Construct` (adicionar bind - **NÃO usar OnPlayerInspected!**)
- [ ] Custom Event `OnPlayerInspectedManual_Event` criado com inputs: `Player ID` (int32) e `Player Info` (Umbra Character Info)
- [ ] `OnPlayerInspectedManual_Event` verifica se `InspectedCharacterInfoWidget` já existe (Is Valid?)
- [ ] Se existe, remove do viewport antes de criar novo
- [ ] `OnPlayerInspectedManual_Event` cria widget `WBP_CharacterInfo` com `bIsInspectingMode = True` e adiciona ao viewport
- [ ] `OnPlayerInspectedManual_Event` chama `Update Inspected Player Info` no widget criado **imediatamente após criar**
- [ ] Variável `InspectedCharacterInfoWidget` criada no `WBP_PlayerHUD` para armazenar referência

**⚠️ IMPORTANTE:** Use `OnPlayerInspectedManual` em vez de `OnPlayerInspected`! O `OnPlayerInspectedManual` só dispara quando o usuário clica explicitamente no botão Inspect, evitando que o widget seja criado quando `InspectPlayer` é chamado automaticamente.

### **Tratamento de Erros (Opcional):**
- [ ] Escuta `OnPlayerInspectFailed`
- [ ] Mostra mensagem de erro ao usuário

---

## 🎯 **FLUXO COMPLETO:**

### **Fluxo 1: Inspeção de Outro Jogador**

```
1. Jogador clica com botão direito em outro jogador
   ↓
2. WBP_PlayerContextMenu abre
   ↓
3. Jogador clica em "Inspect"
   ↓
4. BTN_Inspect OnClicked → Execute Context Action (Inspect)
   ↓
5. PlayerSelectionComponent → GameInstance.HandlePlayerContextAction
   ↓
6. GameInstance.InspectPlayer(TargetPlayerID)
   ↓
7. Requisição HTTP para /api/character/get_public_info.php
   ↓
8. OnInspectPlayerComplete → OnPlayerInspected.Broadcast(PlayerID, PlayerInfo)
   ↓
9. WBP_PlayerHUD escuta OnPlayerInspectedManual (Event Construct já fez bind)
   ↓
10. OnPlayerInspectedManual_Event (no WBP_PlayerHUD) cria WBP_CharacterInfo com bIsInspectingMode = True
    ↓
11. WBP_PlayerHUD chama Update Inspected Player Info no widget criado
    ↓
12. Update Inspected Player Info → Update Character Info Display
    ↓
13. Widget exibe informações do jogador inspecionado
```

### **Fluxo 2: Próprio Personagem (Tecla C)**

```
1. Jogador pressiona tecla C
   ↓
2. Player Controller → Create Widget WBP_CharacterInfo
   ↓
3. WBP_CharacterInfo Event Construct executa
   ↓
4. Bind OnCharacterInfoLoaded → OnCharacterInfoLoaded_Event
   ↓
5. Load Character Info (chama API)
   ↓
6. OnCharacterInfoLoaded dispara
   ↓
7. OnCharacterInfoLoaded_Event → Update Character Info Display
   ↓
8. Widget exibe informações do próprio personagem
```

### **Diferenças Importantes:**

| Aspecto | Próprio Personagem | Inspeção |
|---------|-------------------|----------|
| **Onde é criado** | Player Controller (tecla C) | WBP_PlayerHUD (OnPlayerInspected) |
| **Dados vêm de** | `Load Character Info` → `OnCharacterInfoLoaded` | `OnPlayerInspected` (já tem dados) |
| **Função chamada** | `Update Character Info Display` (via evento) | `Update Inspected Player Info` (direto) |
| **Event Construct** | Chama `Load Character Info` | Não precisa chamar (dados já vêm prontos) |

---

## 🔍 **TESTES:**

1. **Teste Básico:**
   - Clique direito em outro jogador → Menu abre
   - Clique em "Inspect" → Widget `WBP_CharacterInfo` abre com dados do jogador

2. **Teste de Erro:**
   - Tente inspecionar um jogador offline/inválido → Mensagem de erro aparece

3. **Teste de Múltiplas Inspeções:**
   - Inspecione jogador A → Widget abre
   - Inspecione jogador B → Widget atualiza ou fecha e abre novo (dependendo da implementação)

4. **Teste de Fechar:**
   - Abra o widget de inspeção → Clique no botão fechar → Widget fecha

---

## 📝 **NOTAS:**

- O sistema C++ já está completo (`InspectPlayer`, `OnPlayerInspected`, `OnPlayerInspectFailed`)
- A API `/api/character/get_public_info.php` já deve existir e retornar dados públicos do jogador
- O `WBP_CharacterInfo` pode ser reutilizado para inspeção, apenas passando dados diferentes
- Se preferir, pode criar um widget separado `WBP_InspectedCharacterInfo` apenas para inspeção

---

## 🐛 **TROUBLESHOOTING:**

### **Widget não abre ao clicar em Inspect:**
- Verificar se `BTN_Inspect OnClicked` está conectado corretamente
- Verificar se `Execute Context Action` está sendo chamado com `Action = Inspect`
- Verificar se está escutando `OnPlayerInspectedManual` (não `OnPlayerInspected`)
- Verificar Output Log para erros de `InspectPlayer`
- Verificar se o log mostra "Chamada manual detectada - Disparando OnPlayerInspectedManual"

### **Widget abre mas está vazio ou mostra dados do próprio jogador:**
- Verificar se `Update Inspected Player Info` está sendo chamado **imediatamente após** criar o widget
- Verificar se `bIsInspectingMode` está sendo passado como `True` no `Create Widget`
- Verificar se o `Event Construct` do `WBP_CharacterInfo` está verificando `bIsInspectingMode` antes de chamar `Load Character Info`
- Adicionar logs em `Update Inspected Player Info` para verificar se está sendo chamado com os dados corretos

### **Widget abre ao clicar no actor (sem clicar em Inspect):**

**CAUSA IDENTIFICADA:** O código C++ em `UmbraPlayerSelectionComponent.cpp` (linha ~185) chama `InspectPlayer` automaticamente quando:
- Um jogador é selecionado (`SelectPlayer`)
- O jogador **não está no cache local** (`RemotePlayersCache`)
- Mas o `PlayerID` **é encontrado no Map do GameInstance** (`GetPlayerIDFromActor`)

**Código problemático:**
```cpp
// Se encontrou PlayerID, chamar InspectPlayer automaticamente
if (FoundPlayerID > 0)
{
    GameInstance->InspectPlayer(FoundPlayerID);  // ← CHAMADA AUTOMÁTICA!
}
```

**Isso causa:**
- Ao clicar em um actor que não está no cache do `UmbraPlayerSelectionComponent`
- O código chama `InspectPlayer` automaticamente
- Isso dispara `OnPlayerInspected`
- Que abre o widget `WBP_CharacterInfo` mesmo sem clicar em Inspect

**Por que acontece:**
- O cache local (`RemotePlayersCache`) pode estar vazio ou não ter sido atualizado
- Mas o `GameInstance` tem o `PlayerID` no Map (`RemotePlayerActorsMap`)
- Então o código assume que deve chamar `InspectPlayer` para obter os dados completos

**Soluções:**

1. **SOLUÇÃO RECOMENDADA: Verificar Active Player ID no Blueprint (Mais Simples):**
   
   Adicione verificação no `OnPlayerInspected_Event` para **não criar o widget se for chamado automaticamente**:
   
   - Verificar se `Player ID` é diferente de `Active Player ID` antes de criar o widget
   - Se for o próprio player ou se for chamado automaticamente, não criar o widget
   - Isso evita que o widget seja criado quando `InspectPlayer` é chamado automaticamente pelo `SelectPlayer`
   
   **Instruções detalhadas estão na seção "Solução 2" abaixo.**

2. **Modificar o código C++ (Mais Complexo):**
   
   **Opção A:** Remover a chamada automática de `InspectPlayer` em `SelectPlayer`:
   - Em `UmbraPlayerSelectionComponent.cpp`, linha ~185, **remover ou comentar** a chamada `GameInstance->InspectPlayer(FoundPlayerID)`
   - Isso evita que `InspectPlayer` seja chamado automaticamente
   - **Problema:** Pode quebrar funcionalidades que dependem dessa chamada automática
   
   **Opção B:** Adicionar um parâmetro para indicar se é chamada automática:
   - Modificar `InspectPlayer` para aceitar um parâmetro `bIsAutomaticCall`
   - Quando `bIsAutomaticCall = true`, não disparar `OnPlayerInspected` (ou disparar um evento diferente)
   - Modificar `SelectPlayer` para passar `bIsAutomaticCall = true`
   - **Problema:** Requer mudanças no código C++ e pode afetar outras partes do sistema

3. **Adicionar verificação no Blueprint (SOLUÇÃO RECOMENDADA - Resolve o Problema):**
   
   **No `WBP_PlayerHUD` → `OnPlayerInspected_Event`:**
   
   Adicione uma verificação no início para **não criar o widget se for chamado automaticamente**:
   
   **IMPORTANTE:** Como `InspectPlayer` é chamado automaticamente quando um jogador é selecionado (mesmo que não seja o próprio player), você precisa verificar se o evento está vindo de uma ação manual do usuário (botão Inspect) ou automática.
   
   **Solução:** Criar o widget **apenas quando o usuário clicar explicitamente em Inspect**. Como não temos essa informação no delegate, a melhor solução é:
   
   - **Opção 1:** Verificar se é o próprio player (mais simples, mas pode não cobrir todos os casos)
   - **Opção 2:** Adicionar uma flag no GameInstance que indica se `InspectPlayer` foi chamado manualmente
   - **Opção 3:** Criar um delegate separado para inspeção manual vs automática
   
   **SOLUÇÃO PRÁTICA (Opção 1 - Mais Simples):**
   
   Adicione verificação para **não criar o widget se for o próprio player**:
   
   ```
   [OnPlayerInspected_Event]
       Input: Player ID
       Input: Player Info
       │
       ├──► Get Game Instance
       │         │
       │         └──► Cast to Umbra Game Instance
       │                   │
       │                   └──► Get Active Player ID
       │                             │
       │                             └──► Equal (int)
       │                                   A: Player ID (do evento)
       │                                   B: Active Player ID
       │                                   │
       │                                   ├──► [True] → (RETORNAR - não fazer nada, é o próprio player)
       │                                   │
       │                                   └──► [False] → (continuar - criar widget normalmente)
       │
       └──► (resto do código - criar widget, etc.)
   ```
   
   **Passo a passo no Blueprint:**
   
   1. No início do `OnPlayerInspected_Event`, após receber os inputs
   2. **Right Click** → **Get Game Instance**
   3. **Right Click** → **Cast to Umbra Game Instance**
   4. **Right Click** → **Get Active Player ID** (procure por "Active Player ID" ou "Get Active Player ID")
   5. **Right Click** → **Equal (int)**
   6. **Conecte:**
      - `Player ID` (do input do evento) → `A` (input do Equal)
      - `Active Player ID` (do Get Active Player ID) → `B` (input do Equal)
   7. **Right Click** → **Branch**
   8. **Conecte:**
      - `Equal` (output) → `Condition` (input do Branch)
      - `OnPlayerInspected_Event` (`then`) → `execute` (input do Branch)
      - `Branch` (`then` - True) → **NÃO conectar nada** (retorna, não cria widget)
      - `Branch` (`else` - False) → **Conecte ao resto do código** (cria widget normalmente)
   
   **NOTA:** Esta solução resolve o caso do próprio player, mas se `InspectPlayer` for chamado automaticamente para outro jogador (não o próprio), o widget ainda será criado. 
   
   **SOLUÇÃO IDEAL (Requer mudança no C++):**
   
   Para resolver completamente, seria necessário:
   
   1. **Adicionar um parâmetro `bIsManualInspect` em `InspectPlayer`:**
      ```cpp
      void InspectPlayer(int32 TargetPlayerID, bool bIsManualInspect = false);
      ```
   
   2. **Modificar `SelectPlayer` para passar `bIsManualInspect = false`:**
      ```cpp
      GameInstance->InspectPlayer(FoundPlayerID, false);  // false = chamada automática
      ```
   
   3. **Modificar `HandlePlayerContextAction` para passar `bIsManualInspect = true`:**
      ```cpp
      InspectPlayer(PlayerInfo.PlayerID, true);  // true = chamada manual (botão Inspect)
      ```
   
   4. **Adicionar um novo delegate `OnPlayerInspectedManual` que só dispara quando `bIsManualInspect = true`**
   
   5. **No Blueprint, escutar apenas `OnPlayerInspectedManual` em vez de `OnPlayerInspected`**
   
   **Isso garantiria que o widget só seja criado quando o usuário clicar explicitamente em Inspect.**

4. **Verificar se há outros lugares que chamam InspectPlayer:**
   - Procure por todas as chamadas de `InspectPlayer` no código C++
   - Verifique se alguma delas está sendo chamada quando não deveria

### **Erro ao inspecionar:**
- Verificar Output Log para mensagens de erro da API
- Verificar se `/api/character/get_public_info.php` existe e funciona
- Verificar se o token está sendo enviado corretamente
- Verificar se o `PlayerID` passado para `InspectPlayer` é válido

---

## ✅ **RESUMO:**

1. **Context Menu:** Botão Inspect já deve estar conectado (verificar)
2. **WBP_CharacterInfo:** Criar função `Update Inspected Player Info` que chama `Update Character Info Display`. Para evitar que o widget carregue o próprio personagem na inspeção: usar a **solução alternativa sem pino** (remover `Load Character Info` do Event Construct e criar `Load Self Character Info`; quem abre para “próprio personagem” chama essa função depois de criar o widget). A variável `bIsInspectingMode` com Expose on Spawn é opcional (se o pino não aparecer no Create Widget, use a solução alternativa).
3. **Player Controller/HUD:** Escutar `OnPlayerInspectedManual` (não `OnPlayerInspected`) e criar/atualizar widget quando disparar
4. **C++:** Já implementado - `InspectPlayer` agora aceita `bIsManualInspect` e dispara `OnPlayerInspectedManual` apenas quando manual
5. **Testar:** Clicar em Inspect e verificar se widget abre com dados corretos (não deve abrir ao clicar no actor)

---

## 🔧 **SOLUÇÃO ALTERNATIVA (SEM bIsInspectingMode / SEM PINO NO CREATE WIDGET)**

Se o pino **bIsInspectingMode** não aparecer no **Create Widget** (mesmo com Expose on Spawn marcado), use esta solução. Ela **não usa** variável nem pino.

### **Ideia**
- No **Event Construct** do `WBP_CharacterInfo` **não** chame `Load Character Info`.
- Quem abrir o widget para o **próprio personagem** (tecla C) chama `Load Character Info` **depois** de criar e adicionar o widget.
- Quem abrir para **inspeção** só chama `Update Inspected Player Info` (como já faz).

### **Passos**

**1. No WBP_CharacterInfo – Event Construct**
- Deixe apenas: **Bind** `OnCharacterInfoLoaded` e `OnCharacterInfoFailed`.
- **Remova** a chamada a `Load Character Info` (e qualquer Branch que use `bIsInspectingMode` para isso).

**2. No WBP_CharacterInfo – Nova função**
- Crie a função **Load Self Character Info** (sem parâmetros).
- Dentro: **Get Variable** `MyGameInstance` → **Load Character Info** (Target = MyGameInstance).

**3. Fluxo “próprio personagem” (tecla C)**
- Onde hoje você faz: Create Widget (WBP_CharacterInfo) → Add to Viewport:
  - Depois de **Add to Viewport**, chame **Load Self Character Info** no widget criado (Target = retorno do Create Widget).
- Assim o próprio personagem continua sendo carregado ao abrir com C.

**4. Fluxo “inspeção” (OnPlayerInspectedManual_Event)**
- Create Widget (WBP_CharacterInfo) → Add to Viewport → **Update Inspected Player Info** (como já está).
- **Não** chame `Load Character Info` nem `Load Self Character Info` aqui.

Com isso, o widget de inspeção não chama `Load Character Info` e continua mostrando só os dados recebidos em **Update Inspected Player Info**. Não é necessário **bIsInspectingMode** nem pino no Create Widget.

---

## 🔧 **INSTRUÇÕES EXATAS: Implementar bIsInspectingMode (Solução para Problema 1)**

### **PROBLEMA:** Widget mostra informações do próprio jogador em vez do jogador inspecionado

**CAUSA:** O `Event Construct` do `WBP_CharacterInfo` sempre chama `Load Character Info`, que carrega dados do próprio jogador, sobrescrevendo os dados de inspeção.

**SOLUÇÃO (escolha uma):**
- **Opção A:** Usar a **Solução alternativa (sem bIsInspectingMode)** descrita acima.
- **Opção B:** Usar a flag `bIsInspectingMode` e o Branch no Event Construct (abaixo), se o pino aparecer no Create Widget.

---

### **📍 PASSO 1: Criar Variável no WBP_CharacterInfo**

**Localização:** `Content/Widgets/UI/CharacterInfo/WBP_CharacterInfo`

1. Abra o `WBP_CharacterInfo` no Editor
2. Clique na aba **Variables** (ícone de variável no topo)
3. Clique no botão **+ Variable** (canto superior direito)
4. No painel **Details** à direita, configure:
   - **Variable Name:** `bIsInspectingMode`
   - **Variable Type:** Boolean (procure por "bool" ou "Boolean")
   - **Default Value:** False (deixe desmarcado)
   - **Instance Editable:** False (desmarcado)
   - **Expose on Spawn:** True ← **MARQUE ESTA OPÇÃO!**
   - **Category:** (opcional) "Inspect" ou "UI"
5. Clique em **Compile** (ícone de compilação no topo)
6. Salve o widget (Ctrl+S)

---

### **📍 PASSO 2: Modificar Event Construct do WBP_CharacterInfo**

**Localização:** `WBP_CharacterInfo` → **Event Graph** → **Event Construct**

**O que fazer:**

1. Encontre o nó que chama `Load Character Info` (geralmente após `Bind Event to OnCharacterInfoFailed`)
2. **Desconecte** a conexão de execução que vai direto para `Load Character Info`
3. Adicione um **Branch** antes de `Load Character Info`:

**Passo a passo visual:**

```
ANTES (estrutura atual):
[Bind Event to OnCharacterInfoFailed]
    ↓ (exec)
[Load Character Info]  ← REMOVER ESTA CONEXÃO DIRETA

DEPOIS (estrutura correta):
[Bind Event to OnCharacterInfoFailed]
    ↓ (exec)
[Get Variable → bIsInspectingMode]
    ↓ (exec)
[Branch]
    Condition: bIsInspectingMode
    ↓
    ├─ then (False) → [Load Character Info]  ← Só chama se False
    └─ else (True) → (não fazer nada)
```

**Como fazer no Blueprint:**

1. **Right Click** no espaço vazio do Event Graph → **Get Variable** → Selecione `bIsInspectingMode`
2. **Right Click** → **Branch**
3. **Conecte:**
   - `bIsInspectingMode` (output) → `Condition` (input do Branch)
   - `Bind Event to OnCharacterInfoFailed` (then) → `execute` (input do Branch)
   - `Branch` (`then` - False) → `execute` (input de `Load Character Info`)
4. **Remova** a conexão direta de `Bind Event to OnCharacterInfoFailed` para `Load Character Info`
5. **O `else` (True) do Branch não precisa conectar nada** - quando é inspeção, não chama `Load Character Info`
6. Compile e salve

---

### **📍 PASSO 3: Modificar Create Widget no WBP_PlayerHUD**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → **OnPlayerInspected_Event**

**O que fazer:**

1. Encontre o nó `Create Widget` que cria `WBP_CharacterInfo`
2. Você verá um **novo pino** chamado `bIsInspectingMode` no `Create Widget` (porque está Expose on Spawn)
3. Conecte um valor `True` a esse pino

**Passo a passo visual:**

```
ANTES:
[Create Widget]
    Class: WBP_CharacterInfo
    OwningPlayer: Get Player Controller

DEPOIS:
[Make Literal Bool]
    Value: True
    ↓ (output)
[Create Widget]
    Class: WBP_CharacterInfo
    OwningPlayer: Get Player Controller
    bIsInspectingMode: True  ← NOVO PINO!
```

**Como fazer no Blueprint:**

1. **Right Click** próximo ao `Create Widget` → **Make Literal Bool**
2. No `Make Literal Bool`, no painel **Details**, marque **Value** como `True`
3. **Conecte** o output do `Make Literal Bool` ao pino `bIsInspectingMode` do `Create Widget`
4. Compile e salve

**Se o pino `bIsInspectingMode` não aparecer:**
- Verifique se `bIsInspectingMode` tem **Expose on Spawn: True** no `WBP_CharacterInfo`
- Recompile o `WBP_CharacterInfo` e salve
- Feche completamente o `WBP_PlayerHUD` e reabra
- Se ainda não aparecer, verifique se a variável foi salva corretamente

---

### **📍 PASSO 4: Verificar Update Inspected Player Info**

**Localização:** `WBP_PlayerHUD` → **OnPlayerInspected_Event**

Certifique-se de que após criar o widget, você está chamando `Update Inspected Player Info`:

**Estrutura correta:**

```
[OnPlayerInspected_Event]
    Input: Player ID
    Input: Player Info
    ↓
[Is Valid?] → InspectedCharacterInfoWidget
    ↓
[Create Widget] (bIsInspectingMode = True)
    ↓
[Add to Viewport]
    ↓
[Set] → InspectedCharacterInfoWidget = (widget criado)
    ↓
[Update Inspected Player Info]
    Target: InspectedCharacterInfoWidget
    Input: Player Info
```

**Verifique:**
- O `Update Inspected Player Info` está sendo chamado **após** criar o widget?
- O `Target` está conectado ao widget criado (`InspectedCharacterInfoWidget`)?
- O `Input` está conectado ao `Player Info` do evento?

---

## 🔍 **PROBLEMA 2: Widget Abre ao Clicar no Actor (Sem Clicar em Inspect)**

**Possível causa:** O evento `OnPlayerInspected` está sendo disparado incorretamente ou o widget está sendo criado múltiplas vezes.

### **Verificações:**

1. **Adicione logs para identificar quando OnPlayerInspected é disparado:**
   - No `OnPlayerInspected_Event` do `WBP_PlayerHUD`, adicione no início:
     - **Right Click** → **Print String**
     - **In String:** "🔍 OnPlayerInspected_Event chamado - PlayerID: " + ToString(PlayerID) + " - CharacterName: " + PlayerInfo.CharacterName
     - **Print to Screen:** True
     - **Text Color:** Amarelo ou Verde (para destacar)
   - Isso ajuda a identificar quando e por que o evento está sendo disparado
   - Se aparecer ao clicar no actor sem clicar em Inspect, há algo disparando `InspectPlayer` incorretamente

2. **Verifique se há múltiplas criações do widget:**
   - No `OnPlayerInspected_Event`, verifique se o `Is Valid?` está funcionando corretamente
   - Se o widget já existe, deve ser removido antes de criar novo
   - Adicione um log antes de criar o widget:
     - **Print String:** "🔍 Criando WBP_CharacterInfo para inspeção - PlayerID: " + ToString(PlayerID)

3. **Verifique se o C++ está disparando OnPlayerInspected incorretamente:**
   - No código C++, verifique `HandlePlayerInspectedInternal` em `UmbraGameInstance.cpp`
   - Verifique se há algum lugar que chama `InspectPlayer` automaticamente
   - Verifique se `OnPlayerInspected.Broadcast` está sendo chamado em algum lugar além de `OnInspectPlayerComplete`

4. **Verifique se há algum Input Action ou tecla que chama Inspect:**
   - Verifique se há alguma tecla (ex.: I, F, etc.) que chama `InspectPlayer`
   - Verifique se há algum Input Action mapeado para Inspect
   - Isso pode estar causando inspeção acidental ao clicar

5. **Verifique se o widget está sendo criado no Event Construct:**
   - No `WBP_PlayerHUD`, verifique o `Event Construct`
   - Ele **NÃO deve** criar `WBP_CharacterInfo` automaticamente
   - Deve apenas fazer binds de eventos

### **Possíveis Causas:**

1. **Algum código C++ está chamando `InspectPlayer` automaticamente:**
   - Verifique `HandlePlayerInspectedInternal` em `UmbraGameInstance.cpp`
   - Verifique se há algum lugar que chama `InspectPlayer` quando um jogador é selecionado

2. **O evento está sendo disparado múltiplas vezes:**
   - Adicione um contador ou flag para evitar múltiplas criações
   - Verifique se o bind está sendo feito múltiplas vezes (mesmo que só apareça uma vez no Blueprint)

3. **Race condition:**
   - O widget pode estar sendo criado antes de `Update Inspected Player Info` ser chamado
   - Certifique-se de que `Update Inspected Player Info` é chamado **imediatamente após** criar o widget

### **Solução Temporária (Debug):**

Adicione uma verificação no início do `OnPlayerInspected_Event`:

```
[OnPlayerInspected_Event]
    Input: Player ID
    Input: Player Info
    │
    ├──► Print String: "🔍 OnPlayerInspected_Event - PlayerID: " + ToString(PlayerID)
    │
    ├──► Delay (0.1 segundos)  ← Adicionar delay para ver se há múltiplas chamadas
    │
    └──► (resto do código)
```

Se aparecerem múltiplas mensagens de log, o evento está sendo disparado múltiplas vezes.
