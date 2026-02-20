# Procedimento Completo: Report e Block no Unreal Engine

Este documento descreve o sistema de **Denunciar (Report)** e **Bloquear (Block)** no cliente UE, incluindo APIs PHP, C++ e integração Blueprint.

---

## 1. Visão geral do sistema

### 1.1 Fluxo geral

```
Jogador clica direito em outro jogador
    → Menu de contexto abre (WBP com botões: Seguir, Trocar, Denunciar, Bloquear, etc.)
    → Jogador clica em "Denunciar" ou "Bloquear"
    → UmbraPlayerSelectionComponent.ExecuteContextAction(Action)
    → OnPlayerContextActionSelected.Broadcast(PlayerInfo, Action)
    → UmbraEternumUEPlayerController.OnPlayerContextActionSelected
    → UmbraGameInstance.HandlePlayerContextAction(PlayerInfo, Action)
```

- **Block:** `HandlePlayerContextAction` chama `BlockPlayer()` diretamente → API → sucesso/erro via delegates.
- **Report:** `HandlePlayerContextAction` deve abrir a UI de denúncia (motivo); o widget chama `ReportPlayer(TargetPlayerID, Reason)` e escuta os delegates.

### 1.2 APIs PHP (já existentes)

| Endpoint | Método | Body | Descrição |
|----------|--------|------|-----------|
| `/api/social/report_player.php` | POST | `token`, `reported_player_id`, `reason` (mín. 10 chars) | Registra denúncia |
| `/api/social/block_player.php` | POST | `token`, `blocked_player_id` | Bloqueia jogador |
| `/api/social/unblock_player.php` | POST | `token`, `blocked_player_id` | Desbloqueia jogador |
| `/api/social/get_blocked_players.php` | POST | `token` | Lista de bloqueados (carregada ao logar) |

### 1.3 C++ já implementado

| Item | Status |
|------|--------|
| `ReportPlayer(TargetPlayerID, Reason)` | ✅ |
| `OnReportPlayerComplete` / `OnReportPlayerFail` | ✅ |
| `OnPlayerReported` / `OnPlayerReportFailed` | ✅ |
| `BlockPlayer(TargetPlayerID, PlayerName)` | ✅ |
| `UnblockPlayer(TargetPlayerID)` | ✅ |
| `IsPlayerBlocked(PlayerID)` | ✅ |
| `LoadBlockedPlayers()` + callbacks | ✅ |
| Chamada de `LoadBlockedPlayers()` após seleção de personagem | ✅ |
| `HandlePlayerContextAction` → Block chama `BlockPlayer` | ✅ |
| `HandlePlayerContextAction` → Report dispara `OnReportPlayerRequested` | ✅ |

---

## 2. BLOCK – Procedimento completo

O Block não exige modal de confirmação: o menu de contexto já tem (ou terá) um botão **Bloquear** / **Desbloquear**. O C++ já está pronto; o procedimento abaixo detalha a parte Blueprint no **widget do menu de contexto** e o feedback opcional no HUD.

### 2.1 Passo 1: C++ (já implementado)

- `HandlePlayerContextAction(Block)` chama `BlockPlayer(PlayerInfo.PlayerID, PlayerInfo.CharacterName)`.
- `BlockPlayer` / `UnblockPlayer` / `IsPlayerBlocked` / `LoadBlockedPlayers` já existem.
- Após seleção de personagem, `LoadBlockedPlayers()` é chamado automaticamente.

Nenhuma alteração em C++ é necessária para o Block.

### 2.2 Passo 2: Menu de contexto – Bloquear / Desbloquear

O widget que exibe o **menu de contexto** ao clicar direito em um jogador (ex.: **WBP_PlayerContextMenu**, **WBP_ContextMenu** ou o widget que recebe `OnPlayerContextMenuRequested`) deve exibir um único botão que alterna entre **"Bloquear"** e **"Desbloquear"** conforme o jogador alvo já esteja ou não bloqueado.

#### 2.2.1 Identificar o widget do menu de contexto

1. Localize o Blueprint que é criado/exibido quando o jogador clica direito em outro jogador (geralmente o HUD ou o Player Controller faz **Create Widget** ao receber `OnPlayerContextMenuRequested` e passa **PlayerInfo** e **ScreenPosition**).
2. Esse widget deve ter acesso ao **PlayerID** (e ao **CharacterName**) do jogador sobre o qual o menu foi aberto — via variável definida ao abrir o menu (ex.: `SelectedPlayerID`, `SelectedPlayerName` ou uma struct `PlayerInfo`).

#### 2.2.2 Elementos no layout do menu de contexto

| Elemento | Tipo | Observação |
|----------|------|------------|
| **BTN_Block** | `Button` | Um único botão que mostrará o texto **"Bloquear"** ou **"Desbloquear"** conforme o estado. |

- Se no seu projeto já existir um botão separado "Bloquear" e você quiser adicionar "Desbloquear", pode usar dois botões no mesmo slot e alternar **Visibility** (um Visible, outro Collapsed) conforme abaixo. O procedimento abaixo considera **um botão** cujo texto e ação mudam.

#### 2.2.3 Variáveis do widget do menu de contexto

Garanta que, ao abrir o menu, o widget receba (ou leia de quem o criou) os dados do jogador alvo:

| Nome | Tipo | Descrição |
|------|------|-----------|
| `SelectedPlayerID` | `Integer` | ID do jogador sobre quem o menu foi aberto |
| `SelectedPlayerName` | `String` | Nome do jogador (usado ao chamar `BlockPlayer`) |

(Se o widget já usar uma struct de tipo `UmbraRemotePlayerInfo`, use `PlayerInfo.PlayerID` e `PlayerInfo.CharacterName` nos passos abaixo.)

#### 2.2.4 Função: Atualizar botão Block/Unblock

- **Nome:** `UpdateBlockButton` (ou `RefreshBlockState`)
- **Sem parâmetros** (usa as variáveis do widget).
- **Lógica:**
  1. **Get Game Instance** → **Cast to UmbraGameInstance**.
  2. Chamar **Is Player Blocked** com `Player ID` = `SelectedPlayerID` (variável do widget). Guardar o retorno (ex.: `bIsBlocked`).
  3. Se `bIsBlocked` == true:
     - Set Text do botão (ex.: **BTN_Block**) = **"Desbloquear"**.
     - (Se usar dois botões: Set Visibility do botão "Desbloquear" = Visible, do botão "Bloquear" = Collapsed.)
  4. Se `bIsBlocked` == false:
     - Set Text do botão = **"Bloquear"**.
     - (Se usar dois botões: Set Visibility do botão "Bloquear" = Visible, do "Desbloquear" = Collapsed.)

**Quando chamar essa função:** ao **abrir** o menu (ex.: na função que inicializa o widget com `SelectedPlayerID` / `SelectedPlayerName`, ou no **Event Construct** se as variáveis já estiverem setadas nesse momento). Assim, sempre que o menu aparece, o botão reflete o estado atual (bloqueado ou não).

#### 2.2.5 Botão "Bloquear" / "Desbloquear" – OnClicked

Use a **mesma** lógica para o único botão (ou para cada um dos dois, se forem botões separados):

**Se o jogador NÃO está bloqueado (botão mostra "Bloquear"):**

1. **Get Game Instance** → **Cast to UmbraGameInstance**.
2. Chamar **Block Player**:
   - `Target Player ID` = `SelectedPlayerID`
   - `Player Name` = `SelectedPlayerName`
3. Fechar o menu (ex.: **Remove From Parent** ou função que esconde o widget).

**Se o jogador JÁ está bloqueado (botão mostra "Desbloquear"):**

1. **Get Game Instance** → **Cast to UmbraGameInstance**.
2. Chamar **Unblock Player**:
   - `Target Player ID` = `SelectedPlayerID`
3. Fechar o menu (ex.: **Remove From Parent**).

**Implementação prática no Blueprint:** no **OnClicked** do botão, chame **Is Player Blocked** com `SelectedPlayerID`. Com um **Branch**:
- **False** (não bloqueado) → **Block Player** com `SelectedPlayerID` e `SelectedPlayerName`, depois Remove From Parent.
- **True** (bloqueado) → **Unblock Player** com `SelectedPlayerID`, depois Remove From Parent.

Assim um único botão serve para Bloquear e Desbloquear.

#### 2.2.6 Resumo visual (menu de contexto)

```
[Ao abrir o menu com SelectedPlayerID / SelectedPlayerName]
    │
    └──► UpdateBlockButton
              │
              ├──► Get Game Instance → Cast to UmbraGameInstance
              ├──► Is Player Blocked (SelectedPlayerID) → bIsBlocked
              └──► Branch bIsBlocked
                    ├── True  → Set Button Text = "Desbloquear"
                    └── False → Set Button Text = "Bloquear"

[BTN_Block OnClicked]
    │
    ├──► Is Player Blocked (SelectedPlayerID) → bIsBlocked
    └──► Branch bIsBlocked
          ├── False → Block Player (SelectedPlayerID, SelectedPlayerName) → Remove From Parent
          └── True  → Unblock Player (SelectedPlayerID) → Remove From Parent
```

### 2.3 Passo 3: Feedback no HUD (opcional)

Para mostrar uma mensagem de sucesso ou erro ao bloquear/desbloquear (toast ou notificação):

#### 2.3.1 Event Construct do HUD (ou widget raiz)

- **Bind Event to On Player Blocked** (Target = UmbraGameInstance):
  - No handler: exibir mensagem de sucesso (ex.: "Jogador bloqueado."). Pode usar o mesmo sistema de toast do Report ou do resto do jogo.
- **Bind Event to On Block Failed** (Target = UmbraGameInstance):
  - No handler: exibir a mensagem de erro recebida (ex.: "Jogador já está bloqueado.", "Não é possível bloquear a si mesmo.").
- **Bind Event to On Player Unblocked** (Target = UmbraGameInstance):
  - No handler: exibir mensagem de sucesso (ex.: "Jogador desbloqueado.").

Assim o jogador recebe feedback visual sem precisar de um modal extra para Block.

### 2.4 Resumo do fluxo Block

```
Usuário clica direito em jogador → Menu de contexto abre
    → Widget recebe SelectedPlayerID / SelectedPlayerName
    → UpdateBlockButton → Is Player Blocked → Botão mostra "Bloquear" ou "Desbloquear"

Usuário clica "Bloquear"
    → Block Player (SelectedPlayerID, SelectedPlayerName)
    → API block_player.php
    → OnPlayerBlocked.Broadcast → (opcional) toast "Jogador bloqueado."
    → Menu fecha (Remove From Parent)

Usuário clica "Desbloquear"
    → Unblock Player (SelectedPlayerID)
    → API unblock_player.php
    → OnPlayerUnblocked.Broadcast → (opcional) toast "Jogador desbloqueado."
    → Menu fecha (Remove From Parent)
```

---

## 3. REPORT – Procedimento completo

O Report exige **UI para o motivo**. O fluxo deve ser:

1. Usuário clica em **Denunciar** no menu de contexto.
2. C++ dispara um evento com `PlayerID` e `PlayerName` do jogador alvo.
3. O HUD cria o widget de denúncia e o exibe.
4. Usuário preenche o motivo (mín. 10 caracteres) e clica em **Enviar**.
5. Widget chama `ReportPlayer(TargetPlayerID, Reason)` e escuta sucesso/erro.
6. Widget mostra mensagem e fecha.

### 3.1 Passo 1: Delegate e uso em HandlePlayerContextAction (C++)

**Objetivo:** Quando o jogador escolhe "Denunciar", disparar um evento para o Blueprint abrir o widget com o alvo da denúncia.

#### 3.1.1 Adicionar o delegate no header

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h`

- Junto aos outros delegates de Report/Block (por volta da linha 118), adicione:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReportPlayerRequested, int32, PlayerID, const FString&, PlayerName);
```

- Na seção de delegates (por volta da linha 413), adicione:

```cpp
UPROPERTY(BlueprintAssignable, Category = "Events|Social")
FOnReportPlayerRequested OnReportPlayerRequested;
```

#### 3.1.2 Alterar HandlePlayerContextAction no .cpp

**Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`

- Localize o `case EUmbraPlayerContextAction::Report:` (por volta da linha 5238).
- Substitua o bloco atual por:

```cpp
case EUmbraPlayerContextAction::Report:
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🚨 Report solicitado para %s - Abrindo UI de denúncia"), *PlayerInfo.CharacterName);
    OnReportPlayerRequested.Broadcast(PlayerInfo.PlayerID, PlayerInfo.CharacterName);
    break;
```

- Recompile o projeto.

### 3.2 Passo 2: Criar o widget WBP_ReportPlayer

#### 3.2.1 Criar o Blueprint

1. **Content Browser** → pasta adequada (ex.: `Content/Widgets/UI/PlayerInteraction/`).
2. Botão direito → **User Interface** → **Widget Blueprint**.
3. Nome: **WBP_ReportPlayer**.
4. Abrir o widget.

#### 3.2.2 Layout sugerido

| Elemento | Tipo | Propriedades / Observação |
|----------|------|----------------------------|
| **Root** | `Canvas Panel` | — |
| **Container** | `Vertical Box` ou `Border` | Fundo semi-opaco, centralizado |
| **TX_Title** | `Text Block` | Ex.: "Denunciar jogador" |
| **TX_PlayerName** | `Text Block` | Nome do jogador sendo denunciado (preenchido por Blueprint) |
| **EDT_Reason** | `Multi-Line Editable Text Box` | Placeholder: "Descreva o motivo da denúncia (mínimo 10 caracteres)..." |
| **TX_Error** | `Text Block` | Mensagem de erro (ex.: motivo curto); Visibility = Collapsed por padrão |
| **BTN_Submit** | `Button` | Texto: "Enviar denúncia" |
| **BTN_Cancel** | `Button` | Texto: "Cancelar" |

Hierarquia de exemplo:

```
Canvas Panel (Root)
└── Vertical Box (ou Border)
    ├── Text Block (TX_Title) - "Denunciar jogador"
    ├── Text Block (TX_PlayerName) - "[Nome]"
    ├── Multi-Line Editable Text Box (EDT_Reason)
    ├── Text Block (TX_Error) - Visibility Collapsed
    └── Horizontal Box
        ├── Button (BTN_Submit) - "Enviar denúncia"
        └── Button (BTN_Cancel) - "Cancelar"
```

#### 3.2.3 Variáveis do widget

| Nome | Tipo | Editable | Descrição |
|------|------|----------|-----------|
| `TargetPlayerID` | `Integer` | Sim | ID do jogador denunciado |
| `TargetPlayerName` | `String` | Sim | Nome do jogador |

#### 3.2.4 Função: ShowReportDialog

- **Nome:** `ShowReportDialog`
- **Parâmetros:** `PlayerID` (Integer), `PlayerName` (String)
- **Lógica:**
  1. `Set TargetPlayerID = PlayerID`
  2. `Set TargetPlayerName = PlayerName`
  3. `Set Text (TX_PlayerName) = PlayerName`
  4. Limpar texto de `EDT_Reason`
  5. Esconder `TX_Error` (Set Visibility = Collapsed)
  6. Set Visibility do widget = Visible

#### 3.2.5 Botão Cancelar – OnClicked

- `Remove From Parent` (fecha o widget).

#### 3.2.6 Botão Enviar denúncia – OnClicked

1. Obter texto de `EDT_Reason` (trim se possível).
2. Se comprimento &lt; 10: mostrar mensagem em `TX_Error` (ex.: "O motivo deve ter pelo menos 10 caracteres."), Set Visibility = Visible, e **não** chamar API.
3. Se ≥ 10:
   - Obter **Game Instance** → **Cast to UmbraGameInstance**.
   - Chamar **Report Player** com:
     - `Target Player ID` = variável `TargetPlayerID` do widget
     - `Reason` = texto do `EDT_Reason`
   - (Opcional) desabilitar botão "Enviar" até receber resposta para evitar duplo envio.

Não fechar o widget aqui; fechar no evento de sucesso ou erro (abaixo).

#### 3.2.7 Event Construct do WBP_ReportPlayer

- **Bind Event to On Player Reported** (Target = UmbraGameInstance obtido do Get Game Instance):
  - No handler: mostrar mensagem de sucesso (ex.: "Denúncia registrada."), depois **Remove From Parent**.
- **Bind Event to On Player Report Failed** (Target = UmbraGameInstance):
  - No handler: definir texto de `TX_Error` com a mensagem recebida, Set Visibility = Visible; **não** remover o widget para o usuário poder corrigir e tentar de novo.

Assim, o widget só fecha em sucesso; em erro, o jogador vê a mensagem e pode alterar o motivo.

### 3.3 Passo 3: Integração no HUD (ou widget raiz)

O widget que gerencia a UI do jogador (ex.: **WBP_PlayerHUD**) deve abrir o WBP_ReportPlayer quando o C++ disparar `OnReportPlayerRequested`.

#### 3.3.1 Event Construct (ou BeginPlay)

- Obter **Game Instance** → **Cast to UmbraGameInstance** (guardar em variável local se quiser).
- **Bind Event to On Report Player Requested**:
  - Target = UmbraGameInstance
  - Event = **Custom Event** (criar um Custom Event, ex.: `OnReportPlayerRequested` com parâmetros `PlayerID` (Integer) e `PlayerName` (String)).

Importante: não criar o widget na cadeia de execução do Construct; criar **apenas** dentro do Custom Event que é chamado quando o evento C++ dispara.

#### 3.3.2 Custom Event OnReportPlayerRequested(PlayerID, PlayerName)

1. **Create Widget**
   - Class = `WBP_ReportPlayer`
   - Owning Player = **Get Owning Player** (do HUD)
   - Guardar o retorno (ex.: `ReportWidget`).
2. Chamar **Show Report Dialog** no widget criado:
   - Target = `ReportWidget`
   - `Player ID` = `PlayerID` (do evento)
   - `Player Name` = `PlayerName` (do evento)
3. **Add to Viewport** (Target = `ReportWidget`).

Ordem: Create Widget → ShowReportDialog → Add to Viewport.

### 3.4 Resumo do fluxo Report

```
Usuário clica "Denunciar" no menu de contexto
    → HandlePlayerContextAction(Report)
    → OnReportPlayerRequested.Broadcast(PlayerID, PlayerName)
    → HUD Custom Event OnReportPlayerRequested
    → Create WBP_ReportPlayer → ShowReportDialog → Add to Viewport
    → Usuário preenche motivo (≥10 chars) e clica "Enviar denúncia"
    → Widget chama ReportPlayer(TargetPlayerID, Reason)
    → API report_player.php
    → OnReportPlayerComplete → OnPlayerReported.Broadcast
    → Widget (bind OnPlayerReported) → mensagem de sucesso → Remove From Parent
    (Em erro: OnPlayerReportFailed → Widget mostra TX_Error, mantém aberto)
```

---

## 4. Lista de jogadores bloqueados (WBP_BlockedPlayersList)

Widget que exibe a lista de jogadores bloqueados (como a Friend List). Ao clicar em um nome, abre um widget de opções com a ação **Desbloquear**.

### 4.1 Dados e funções no C++ (GameInstance)

- **BlockedPlayerIDs** (TArray) e **BlockedPlayerNames** (TMap) são preenchidos por `LoadBlockedPlayers()` e ao bloquear; removidos ao desbloquear.
- **OnBlockedListLoaded** é disparado quando a lista é carregada ou alterada (bloquear/desbloquear).

**Funções para carregar e atualizar (reduzem nós no Blueprint):**

| Função | Uso no Blueprint |
|--------|-------------------|
| **Load Blocked List And Start Refresh** (IntervalSeconds = 30) | Chamar **uma vez** ao abrir o painel. Carrega a lista e inicia o refresh periódico (um único nó). |
| **Stop Blocked List Auto Refresh** | Chamar ao **fechar** o painel (ex.: no botão Fechar ou antes de Remove From Parent). |
| Load Blocked Players | Carregar só uma vez (sem timer). Use se não quiser auto-refresh. |
| Start Blocked List Auto Refresh (IntervalSeconds) | Iniciar só o timer (lista já carregada). |
| Stop Blocked List Auto Refresh | Parar o timer. |

**Funções para a UI (RefreshList):**

- **Get Blocked Players Count** → quantidade de itens.
- **Get Blocked Player ID At** (Index) → ID do jogador no índice (0-based).
- **Get Blocked Player Name** (PlayerID) → nome do jogador.

### 4.2 Passo 1: Widget de slot (WBP_BlockedPlayerSlot)

Classe pai: **UmbraBlockedPlayerSlotWidget** (C++).

#### 4.2.1 Criar o Blueprint

1. **Content Browser** → pasta (ex.: `Content/Widgets/UI/PlayerInteraction/Block/`).
2. Botão direito → **User Interface** → **Widget Blueprint**.
3. Nome: **WBP_BlockedPlayerSlot**.
4. **Class Settings** → Parent Class = **UmbraBlockedPlayerSlotWidget**.
5. Abrir o widget.

#### 4.2.2 Layout

| Widget | Nome (obrigatório) | Tipo | Observação |
|--------|--------------------|------|------------|
| Root | — | Canvas Panel ou qualquer | — |
| Nome do jogador | **Text_PlayerName** | Text Block | Bind obrigatório no C++. |
| Área clicável | **Button_Slot** | Button | Pode ser o próprio row ou um botão “transparente” sobre o texto. |

Hierarquia de exemplo:

```
Canvas Panel (Root)
└── Button_Slot (ou Horizontal Box + Button_Slot)
    └── Text_PlayerName - "[Nome do jogador]"
```

Não é necessário ícone de status (lista de bloqueados não tem online/offline).

#### 4.2.3 Lógica

- O C++ já faz: ao clicar em **Button_Slot**, dispara **OnBlockedPlayerSlotClicked** com `PlayerID` e `PlayerName`.
- O **WBP_BlockedPlayersList** (lista) vai criar esse slot, chamar **Set Blocked Player Data** e escutar **On Blocked Player Slot Clicked** para abrir o widget de opções.

### 4.3 Passo 2: Widget de opções (WBP_BlockedPlayerOptions)

Classe pai: **UmbraBlockedPlayerOptionsWidget** (C++).

#### 4.3.1 Criar o Blueprint

1. Mesma pasta (ex.: `Content/Widgets/UI/PlayerInteraction/Block/`).
2. Novo **Widget Blueprint** → Nome: **WBP_BlockedPlayerOptions**.
3. **Class Settings** → Parent Class = **UmbraBlockedPlayerOptionsWidget**.
4. Abrir o widget.

#### 4.3.2 Layout

| Widget | Nome (obrigatório) | Tipo | Observação |
|--------|--------------------|------|------------|
| Root | — | Canvas Panel | — |
| Nome do jogador | **Text_PlayerName** | Text Block | Ex.: "Jogador bloqueado: [Nome]" ou só o nome. |
| Desbloquear | **BTN_Unblock** | Button | Texto: "Desbloquear" |
| Fechar | **BTN_Close** | Button | Texto: "Fechar" |

Hierarquia de exemplo:

```
Canvas Panel (Root)
└── Vertical Box (ou Border)
    ├── Text_PlayerName - "NomeDoJogador"
    └── Horizontal Box
        ├── BTN_Unblock - "Desbloquear"
        └── BTN_Close - "Fechar"
```

#### 4.3.3 Lógica

- **Show Options** (PlayerID, PlayerName): já implementado no C++; preenche **Text_PlayerName** e torna o widget visível.
- **BTN_Unblock**: no C++, chama **Unblock Player** no Game Instance e **Close Options**.
- **BTN_Close**: no C++, chama **Close Options**.
- Nenhuma lógica extra é necessária no Blueprint além de garantir que os nomes dos widgets coincidam com os do C++ (**Text_PlayerName**, **BTN_Unblock**, **BTN_Close**).

### 4.4 Passo 3: Lista (WBP_BlockedPlayersList)

Widget que exibe todos os slots e abre as opções ao clicar.

#### 4.4.1 Criar o Blueprint

1. Mesma pasta.
2. Novo **Widget Blueprint** → Nome: **WBP_BlockedPlayersList**.
3. Parent Class = **User Widget** (não precisa ser C++).
4. Abrir o widget.

#### 4.4.2 Layout

| Elemento | Tipo | Observação |
|----------|------|------------|
| Root | Canvas Panel | — |
| Título | Text Block | Ex.: "Jogadores bloqueados" |
| Lista rolável | **Scroll Box** | Contêiner dos slots. |
| Contêiner dos slots | **Vertical Box** (dentro do Scroll Box) | Nome sugerido: **VB_Slots**. Aqui serão adicionados os **WBP_BlockedPlayerSlot** em runtime. |

Não crie os slots manualmente; eles serão criados por Blueprint em **Event Construct** e ao receber **OnBlockedListLoaded**.

#### 4.4.3 Variáveis

No **WBP_BlockedPlayersList**, use **Class Reference** para as classes de widget (necessário para **Create Widget**) e **Object Reference** para a instância do widget de opções:

| Nome | Tipo no Blueprint | Valor padrão / Uso | Descrição |
|------|-------------------|--------------------|-----------|
| **SlotWidgetClass** | **Class Reference** (Widget Class) | **WBP_BlockedPlayerSlot** | Referência à classe do slot. Tipo: Class → filtrar por **UmbraBlockedPlayerSlotWidget** ou selecionar o Blueprint **WBP_BlockedPlayerSlot**. Definir como padrão **WBP_BlockedPlayerSlot**. |
| **OptionsWidgetClass** | **Class Reference** (Widget Class) | **WBP_BlockedPlayerOptions** | Referência à classe do widget de opções. Tipo: Class → filtrar por **UmbraBlockedPlayerOptionsWidget** ou selecionar o Blueprint **WBP_BlockedPlayerOptions**. Definir como padrão **WBP_BlockedPlayerOptions**. |
| **OptionsWidgetRef** | **Object Reference** (User Widget) | (nenhum; preenchido em runtime) | Referência à instância criada do widget de opções (uma única instância reutilizada). Tipo: Object Reference → **User Widget** (ou **UmbraBlockedPlayerOptionsWidget**). |

Resumo: **SlotWidgetClass** e **OptionsWidgetClass** são **Class Reference** (não Object Reference); os valores padrão no detalhe da variável devem ser as classes **WBP_BlockedPlayerSlot** e **WBP_BlockedPlayerOptions** respectivamente. **OptionsWidgetRef** é **Object Reference** e guarda o widget criado em runtime.

#### 4.4.4 Event Construct

1. **Get Game Instance** → **Cast to UmbraGameInstance** (guardar em variável, ex.: **MyGI**).
2. **Bind Event to On Blocked List Loaded** (Target = MyGI, Event = **Custom Event** **OnBlockedListLoaded**).
3. **Load Blocked List And Start Refresh** (Target = MyGI, Interval Seconds = 30). Um único nó: carrega a lista e inicia o timer; quando a lista chegar, **OnBlockedListLoaded** dispara e chama **RefreshList**.
4. Chamar **RefreshList** (para preencher com dados já em memória, se houver).
5. **Create Widget** (Class = **OptionsWidgetClass**, Owning Player = Get Owning Player) → guardar em **OptionsWidgetRef**. **Add to Viewport** (ou como filho do painel) e **Set Visibility = Collapsed**.

**Ao fechar o painel:** chamar **Stop Blocked List Auto Refresh** (Target = MyGI).

Importante: **RefreshList** não deve ser chamada na cadeia “then” do Bind Event; deve ser chamada **apenas** dentro do Custom Event **OnBlockedListLoaded** e no **Event Construct** (uma vez).

#### 4.4.5 Custom Events no Event Graph (criar antes da função RefreshList)

No Blueprint **não é possível criar um Custom Event dentro de uma função**. Os Custom Events precisam existir no **Event Graph** (nível do widget). Crie-os **antes** de montar a função RefreshList.

**1) Custom Event: OnBlockedSlotClicked**

- No **Event Graph** do WBP_BlockedPlayersList: botão direito → **Add Event** → **Custom Event**.
- Nome: **OnBlockedSlotClicked**.
- Adicionar dois parâmetros: **PlayerID** (Integer) e **PlayerName** (String).
- No corpo do evento:
  - Obter **OptionsWidgetRef** (variável do widget).
  - Chamar **Show Options** (Target = OptionsWidgetRef, Player ID = PlayerID, Player Name = PlayerName).
  - **Set Visibility** (Target = OptionsWidgetRef, Visibility = Visible).

Esse evento será **referenciado** pelo nó Bind dentro da função RefreshList (você não cria evento dentro da função; só associa este que já existe).

**2) Custom Event: OnBlockedListLoaded**

- No **Event Graph**: outro **Custom Event** chamado **OnBlockedListLoaded** (sem parâmetros).
- No corpo: chamar **RefreshList**.

Esse evento é o que você associa ao **Bind Event to On Blocked List Loaded** no Event Construct (no pino "Event" do Bind, selecione **OnBlockedListLoaded**).

#### 4.4.6 Função: RefreshList

A função **RefreshList** só usa nós de lógica; o Bind vai **referenciar** o Custom Event já criado no Event Graph.

1. Obter **MyGI** (Game Instance como UmbraGameInstance). Se MyGI não for passada como parâmetro, use **Get Game Instance** → **Cast to UmbraGameInstance** dentro da função.
2. Obter **VB_Slots** (Vertical Box). **Clear Children** no VB_Slots.
3. **Get Blocked Players Count** (Target = MyGI) → **Count**.
4. **For Loop**: Index de 0 a Count - 1.
   - **Get Blocked Player ID At** (Index) → **PlayerID**.
   - **Get Blocked Player Name** (PlayerID) → **PlayerName**.
   - **Create Widget** (Class = **SlotWidgetClass**, Owning Player = Get Owning Player) → **SlotWidget**.
   - **Set Blocked Player Data** (Target = SlotWidget, Player ID = PlayerID, Player Name = PlayerName).
   - **Add Child to Vertical Box** (Parent = VB_Slots, Content = SlotWidget).
   - **Bind Event to On Blocked Player Slot Clicked** (Target = **SlotWidget**). No pino **"Event"** deste nó Bind: use o dropdown e **selecione o Custom Event OnBlockedSlotClicked** que você criou no Event Graph (não tente "Add Custom Event" dentro da função — o evento já deve existir).

Assim, quando o jogador clicar no slot, o engine chama o Custom Event **OnBlockedSlotClicked** (no Event Graph) com o PlayerID e PlayerName que o slot emitiu, e lá você abre o **OptionsWidgetRef** com **Show Options** e **Set Visibility = Visible**.

Se o widget de opções for filho do painel da lista, basta **Show Options** e **Set Visibility = Visible**. Se estiver no viewport, **Add to Viewport** já foi feito no Construct.

#### 4.4.7 Resumo: onde fica cada coisa

| Onde | O que fazer |
|------|-------------|
| **Event Graph** | Custom Event **OnBlockedSlotClicked** (PlayerID, PlayerName) → Show Options + Set Visibility no OptionsWidgetRef. |
| **Event Graph** | Custom Event **OnBlockedListLoaded** → chamar **RefreshList**. |
| **Event Construct** | Bind On Blocked List Loaded → **OnBlockedListLoaded**; **Load Blocked List And Start Refresh** (1 nó); RefreshList; Create Widget (OptionsWidgetRef); Add to Viewport; Visibility Collapsed. Ao fechar o painel: **Stop Blocked List Auto Refresh**. |
| **Função RefreshList** | Clear VB_Slots; For Loop com Create Slot, Set Blocked Player Data, Add Child, **Bind On Blocked Player Slot Clicked** escolhendo no dropdown o evento **OnBlockedSlotClicked** (já criado no Event Graph). |

### 4.5 Resumo do fluxo (lista de bloqueados)

```
Abrir WBP_BlockedPlayersList
    → Event Construct: Bind On Blocked List Loaded → RefreshList; Create OptionsWidget (collapsed)
    → RefreshList: Clear VB_Slots; para cada índice, Create WBP_BlockedPlayerSlot, Set Blocked Player Data, Add to VB_Slots, Bind On Blocked Player Slot Clicked

Usuário clica em um slot
    → On Blocked Player Slot Clicked (PlayerID, PlayerName)
    → OptionsWidgetRef → Show Options (PlayerID, PlayerName) → Set Visibility Visible

Usuário clica "Desbloquear" no widget de opções
    → C++ Unblock Player → API → On Blocked List Loaded.Broadcast
    → RefreshList (lista atualizada); OptionsWidget já foi fechado pelo C++ (Close Options)
```

### 4.6 Onde abrir a lista (WBP_BlockedPlayersList)

- Abrir como painel/floating (ex.: botão "Lista de bloqueados" no HUD ou no menu social) da mesma forma que a Friend List: **Create Widget** (WBP_BlockedPlayersList) → **Add to Viewport** (ou inserir em um container do HUD). Ao abrir, a lista já será preenchida por **RefreshList** no **Event Construct**; ao bloquear/desbloquear em qualquer lugar do jogo, **OnBlockedListLoaded** atualiza a lista se o painel estiver aberto.

---

## 5. Validações e regras

### 5.1 Report

- **Cliente (widget):** motivo com pelo menos 10 caracteres antes de chamar `ReportPlayer`.
- **API:** `reason` obrigatório; `strlen(trim($reason)) >= 10`; não pode denunciar a si mesmo.

### 5.2 Block

- **C++:** não bloquear se `TargetPlayerID == ActivePlayerID`; não bloquear se já estiver em `BlockedPlayerIDs`.
- **API:** não pode bloquear a si mesmo; jogador alvo deve existir.

---

## 6. Testes recomendados

### 6.1 Block

1. Entrar no jogo com um personagem; verificar no log que `LoadBlockedPlayers` foi chamado e que a quantidade de bloqueados está correta (se houver).
2. Clicar direito em outro jogador → Bloquear → verificar mensagem de sucesso e que o jogador aparece na lista de bloqueados (se houver UI).
3. Clicar direito no mesmo jogador → verificar que aparece "Desbloquear" (se a melhoria do menu tiver sido feita) ou que bloquear novamente retorna "Já bloqueado".
4. Desbloquear (pela UI ou pela função `UnblockPlayer`) e verificar que `IsPlayerBlocked` retorna false.
5. Sair e entrar de novo com o mesmo personagem; verificar que o bloqueado continua na lista (via `LoadBlockedPlayers`).

### 6.2 Report

1. Clicar direito em outro jogador → Denunciar → verificar que o WBP_ReportPlayer abre com o nome correto.
2. Enviar com motivo &lt; 10 caracteres → verificar que o widget mostra erro e não chama a API.
3. Enviar com motivo ≥ 10 caracteres → verificar mensagem de sucesso e que o widget fecha.
4. Verificar no backend (tabela `player_reports`) que a denúncia foi registrada.
5. Testar denunciar a si mesmo (se a UI permitir) e verificar que a API retorna erro.

---

## 7. Checklist final

### Block
- [x] API `block_player.php` / `unblock_player.php` / `get_blocked_players.php`
- [x] C++: BlockPlayer, UnblockPlayer, IsPlayerBlocked, LoadBlockedPlayers
- [x] HandlePlayerContextAction(Block) chama BlockPlayer
- [x] LoadBlockedPlayers chamado após seleção de personagem
- [ ] Menu de contexto: variáveis SelectedPlayerID / SelectedPlayerName ao abrir
- [ ] Menu de contexto: função UpdateBlockButton (Is Player Blocked → texto "Bloquear" ou "Desbloquear")
- [ ] Menu de contexto: OnClicked do botão → Branch Is Player Blocked → Block Player ou Unblock Player → Remove From Parent
- [ ] (Opcional) HUD: Bind On Player Blocked / On Block Failed / On Player Unblocked para toast
- [ ] WBP_BlockedPlayerSlot (parent UmbraBlockedPlayerSlotWidget): Text_PlayerName, Button_Slot
- [ ] WBP_BlockedPlayerOptions (parent UmbraBlockedPlayerOptionsWidget): Text_PlayerName, BTN_Unblock, BTN_Close
- [ ] WBP_BlockedPlayersList: Scroll Box + VB_Slots, Bind On Blocked List Loaded → RefreshList, Create slots + Bind On Blocked Player Slot Clicked → Show Options
- [ ] Abrir WBP_BlockedPlayersList a partir do HUD/menu (ex.: botão "Lista de bloqueados")

### Report
- [x] API `report_player.php`
- [x] C++: ReportPlayer, OnReportPlayerComplete/Fail, OnPlayerReported/OnPlayerReportFailed
- [x] C++: Delegate OnReportPlayerRequested e Broadcast em HandlePlayerContextAction(Report)
- [ ] WBP_ReportPlayer: layout, variáveis, ShowReportDialog
- [ ] WBP_ReportPlayer: validação de 10 caracteres, botões Enviar/Cancelar
- [ ] WBP_ReportPlayer: bind OnPlayerReported / OnPlayerReportFailed
- [ ] HUD: Bind OnReportPlayerRequested → Custom Event → Create Widget → ShowReportDialog → Add to Viewport

---

## 8. Arquivos de referência

| Sistema | Arquivos |
|--------|----------|
| C++ Report/Block | `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h/.cpp` |
| Context action enum | `UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h` (EUmbraPlayerContextAction) |
| Context menu / selection | `UmbraPlayerSelectionComponent.h/.cpp`, `UmbraEternumUEPlayerController.cpp` (OnPlayerContextActionSelected) |
| Blocked list widgets (C++) | `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraBlockedPlayerSlotWidget.h/.cpp`, `UmbraBlockedPlayerOptionsWidget.h/.cpp` |
| APIs PHP | `www/umbra_api/api/social/report_player.php`, `block_player.php`, `unblock_player.php`, `get_blocked_players.php` |
| Procedimento similar (widget + evento) | `UmbraServer/docs_main/PROCEDIMENTO_WBP_TRADE_REQUESTED.md` |
| Análise prévia | `UmbraServer/docs_main/ANALISE_REPORT_BLOCK_UE.md` |

---

**Versão:** 1.0  
**Data:** 2026-02-07
