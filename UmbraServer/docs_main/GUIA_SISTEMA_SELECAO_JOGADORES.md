# Guia: Sistema de Seleção de Jogadores Remotos

Este guia explica como implementar o sistema completo de seleção de jogadores remotos, incluindo widgets de informação e menu de contexto.

## Índice
1. [Visão Geral](#visão-geral)
2. [Arquitetura do Sistema](#arquitetura-do-sistema)
3. [Configuração do Remote Actor](#configuração-do-remote-actor)
4. [Configuração do Input](#configuração-do-input)
5. [Widget: WBP_SelectedPlayerInfo](#widget-wbp_selectedplayerinfo)
6. [Widget: WBP_PlayerContextMenu](#widget-wbp_playercontextmenu)
7. [Integração com NetMovementClient](#integração-com-netmovementclient)
8. [Ações do Menu de Contexto](#ações-do-menu-de-contexto)

---

## Visão Geral

O sistema permite:
- **Clique Esquerdo**: Seleciona um jogador remoto e mostra widget com Nome/HP/MP
- **Clique Direito**: Abre menu de contexto com opções (Seguir, Grupo, Trade, Amigo)

### Componentes C++ Criados

| Arquivo | Descrição |
|---------|-----------|
| `UmbraDataStructures.h` | `FUmbraRemotePlayerInfo` - Dados do jogador remoto |
| `UmbraPlayerSelectionComponent.h/cpp` | Componente que gerencia seleção |
| `UmbraEternumUEPlayerController.h/cpp` | Atualizado com funções de seleção |

---

## Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                     PlayerController                             │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │           UmbraPlayerSelectionComponent                  │   │
│  │  - TrySelectPlayerUnderCursor()                         │   │
│  │  - RemotePlayersCache (TMap<PlayerID, Info>)            │   │
│  │  - OnPlayerSelected (Delegate)                          │   │
│  │  - OnPlayerContextMenuRequested (Delegate)              │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                        Widgets (Blueprint)                       │
│  ┌──────────────────────┐    ┌──────────────────────────────┐  │
│  │ WBP_SelectedPlayerInfo│    │   WBP_PlayerContextMenu      │  │
│  │ - Nome                │    │   - Seguir                   │  │
│  │ - HP Bar             │    │   - Convidar para Grupo      │  │
│  │ - MP Bar             │    │   - Trocar                   │  │
│  │ - Nível              │    │   - Adicionar Amigo          │  │
│  └──────────────────────┘    └──────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Configuração do Remote Actor

### 1. Adicionar Tag ao Blueprint do Remote Actor

No Blueprint do seu Remote Actor (o personagem que representa outros jogadores):

1. Abra o Blueprint do Remote Actor
2. No painel **Details** do root component ou do próprio Blueprint
3. Encontre **Actor > Tags**
4. Adicione a tag: `RemotePlayer`

```
Tags: [RemotePlayer]
```

### 2. Garantir Collision Ativa

O Remote Actor precisa ter collision para ser detectado pelo trace:

1. No **Capsule Component** (ou equivalente):
   - `Collision Presets` = **Pawn** ou **Custom**
   - `Generate Overlap Events` = ✅
   - `Collision Enabled` = **Query and Physics** ou **Query Only**

---

## Configuração do Input

### 1. Criar Input Action para Seleção

1. Vá em `Content/Input/` (ou crie a pasta)
2. Crie: **Right Click** > **Input** > **Input Action**
3. Nomeie: `IA_SelectPlayer`
4. Configure:
   - `Value Type` = **Digital (bool)**

5. Crie outro: `IA_ContextMenu`
   - `Value Type` = **Digital (bool)**

### 2. Adicionar ao Input Mapping Context

1. Abra seu `IMC_Default` ou crie um novo
2. Adicione os mappings:

```
IA_SelectPlayer:
  - Left Mouse Button

IA_ContextMenu:
  - Right Mouse Button
```

### 3. Configurar no Blueprint do PlayerController

No Blueprint que herda de `UmbraEternumUEPlayerController`:

#### Event Graph:

```
[Enhanced Input Action: IA_SelectPlayer] (Started)
    │
    └──► [Handle Player Selection Click]
              └── bRightClick = false

[Enhanced Input Action: IA_ContextMenu] (Started)
    │
    └──► [Handle Player Selection Click]
              └── bRightClick = true
```

---

## Widget: WBP_SelectedPlayerInfo

Widget que mostra informações do jogador selecionado (Nome, HP, MP).

### 1. Criar o Widget

1. **Right Click** em Content Browser
2. **User Interface** > **Widget Blueprint**
3. Nomeie: `WBP_SelectedPlayerInfo`

### 2. Design do Widget

```
┌────────────────────────────────────────┐
│  [Image: ClassIcon]  Nome do Jogador   │
│                      Lv. 50            │
├────────────────────────────────────────┤
│  HP: ████████████░░░░ 850/1000         │
│  MP: ██████░░░░░░░░░░ 200/500          │
└────────────────────────────────────────┘
```

#### Hierarchy:

```
Canvas Panel
└── VerticalBox (Anchor: Top Center)
    ├── HorizontalBox [Header]
    │   ├── Image (Image_ClassIcon) [32x32]
    │   └── VerticalBox
    │       ├── TextBlock (Text_PlayerName) [Bold, 16pt]
    │       └── TextBlock (Text_Level) [12pt]
    │
    ├── HorizontalBox [HP]
    │   ├── TextBlock "HP:"
    │   ├── ProgressBar (Progress_HP) [Fill: Red]
    │   └── TextBlock (Text_HP) "850/1000"
    │
    └── HorizontalBox [MP]
        ├── TextBlock "MP:"
        ├── ProgressBar (Progress_MP) [Fill: Blue]
        └── TextBlock (Text_MP) "200/500"
```

### 3. Variables

Adicione estas variáveis ao Widget:

| Nome | Tipo | Default |
|------|------|---------|
| `CachedPlayerInfo` | `FUmbraRemotePlayerInfo` | - |

### 4. Functions

#### `UpdateDisplay(PlayerInfo: FUmbraRemotePlayerInfo)`

```
[Function: UpdateDisplay]
    │
    ├──► Set CachedPlayerInfo = PlayerInfo
    │
    ├──► Text_PlayerName → Set Text = PlayerInfo.CharacterName
    │
    ├──► Text_Level → Set Text = "Lv. " + ToString(PlayerInfo.Level)
    │
    ├──► Progress_HP → Set Percent = PlayerInfo.GetHealthPercent()
    │
    ├──► Text_HP → Set Text = ToString(CurrentHealth) + "/" + ToString(MaxHealth)
    │
    ├──► Progress_MP → Set Percent = PlayerInfo.GetManaPercent()
    │
    └──► Text_MP → Set Text = ToString(CurrentMana) + "/" + ToString(MaxMana)
```

### 5. Event Graph - Conectar aos Delegates

No **Level Blueprint** ou no **HUD Blueprint**:

```
[Event BeginPlay]
    │
    ├──► Get Player Controller → Cast to UmbraEternumUEPlayerController
    │         │
    │         └──► Get Player Selection Component
    │                   │
    │                   ├──► Bind Event to OnPlayerSelected
    │                   │         │
    │                   │         └──► [CustomEvent: OnPlayerSelected]
    │                   │                   │
    │                   │                   ├──► WBP_SelectedPlayerInfo → Set Visibility (Visible)
    │                   │                   └──► WBP_SelectedPlayerInfo → UpdateDisplay(PlayerInfo)
    │                   │
    │                   ├──► Bind Event to OnPlayerDeselected
    │                   │         │
    │                   │         └──► [CustomEvent: OnPlayerDeselected]
    │                   │                   │
    │                   │                   └──► WBP_SelectedPlayerInfo → Set Visibility (Collapsed)
    │                   │
    │                   └──► Bind Event to OnSelectedPlayerInfoUpdated
    │                             │
    │                             └──► [CustomEvent: OnInfoUpdated]
    │                                       │
    │                                       └──► WBP_SelectedPlayerInfo → UpdateDisplay(PlayerInfo)
```

---

## Widget: WBP_PlayerContextMenu

Menu de contexto com opções de interação.

### 1. Criar o Widget

1. **Right Click** em Content Browser
2. **User Interface** > **Widget Blueprint**
3. Nomeie: `WBP_PlayerContextMenu`

### 2. Design do Widget

```
┌─────────────────────────┐
│  [Nome do Jogador]      │
├─────────────────────────┤
│  ▶ Seguir               │
│  ▶ Convidar para Grupo  │
│  ▶ Trocar               │
│  ▶ Adicionar Amigo      │
│  ─────────────────────  │
│  ▶ Sussurrar            │
│  ▶ Inspecionar          │
│  ─────────────────────  │
│  ▶ Duelar               │
│  ▶ Denunciar            │
│  ▶ Bloquear             │
└─────────────────────────┘
```

#### Hierarchy:

```
Canvas Panel
└── Border (Anchor: Custom - posicionado via código)
    └── VerticalBox
        ├── TextBlock (Text_PlayerName) [Header, Bold]
        ├── Spacer [Height: 4]
        │
        ├── Button (BTN_Follow)
        │   └── TextBlock "Seguir"
        │
        ├── Button (BTN_InviteParty)
        │   └── TextBlock "Convidar para Grupo"
        │
        ├── Button (BTN_Trade)
        │   └── TextBlock "Trocar"
        │
        ├── Button (BTN_AddFriend)
        │   └── TextBlock "Adicionar Amigo"
        │
        ├── Spacer [Height: 8]
        │
        ├── Button (BTN_Whisper)
        │   └── TextBlock "Sussurrar"
        │
        ├── Button (BTN_Inspect)
        │   └── TextBlock "Inspecionar"
        │
        ├── Spacer [Height: 8]
        │
        ├── Button (BTN_Duel)
        │   └── TextBlock "Duelar"
        │
        ├── Button (BTN_Report)
        │   └── TextBlock "Denunciar"
        │
        └── Button (BTN_Block)
            └── TextBlock "Bloquear"
```

### 3. Variables

| Nome | Tipo | Default |
|------|------|---------|
| `CachedPlayerInfo` | `FUmbraRemotePlayerInfo` | - |
| `SelectionComponent` | `UUmbraPlayerSelectionComponent*` | - |

### 4. Functions

#### `ShowAtPosition(PlayerInfo,  )`

```
[Function: ShowAtPosition]
    │
    ├──► Set CachedPlayerInfo = PlayerInfo
    │
    ├──► Text_PlayerName → Set Text = PlayerInfo.CharacterName
    │
    ├──► Border → Set Render Transform (Position = ScreenPosition)
    │
    └──► Set Visibility (Visible)
```

#### `Hide()`

```
[Function: Hide]
    │
    └──► Set Visibility (Collapsed)
```

### 5. Button Click Events

Para cada botão, crie um evento OnClicked:

```
[BTN_Follow OnClicked]
    │
    ├──► SelectionComponent → Execute Context Action (Follow)
    └──► Hide()

[BTN_InviteParty OnClicked]
    │
    ├──► SelectionComponent → Execute Context Action (InviteToParty)
    └──► Hide()

[BTN_Trade OnClicked]
    │
    ├──► SelectionComponent → Execute Context Action (Trade)
    └──► Hide()

[BTN_AddFriend OnClicked]
    │
    ├──► SelectionComponent → Execute Context Action (AddFriend)
    └──► Hide()

// ... similar para os outros botões
```

### 6. Conectar ao Delegate

No **HUD Blueprint** ou onde gerencia widgets:

```
[Bind Event to OnPlayerContextMenuRequested]
    │
    └──► [CustomEvent: OnContextMenuRequested(PlayerInfo, ScreenPosition)]
              │
              ├──► WBP_PlayerContextMenu → Set SelectionComponent (ref)
              └──► WBP_PlayerContextMenu → ShowAtPosition(PlayerInfo, ScreenPosition)
```

---

## Integração com NetMovementClient

### Registrar Jogadores Remotos

No Blueprint do `BP_NetMovementClient`, quando um jogador remoto é criado:

```
[After Spawning Remote Actor]
    │
    ├──► Create FUmbraRemotePlayerInfo struct
    │         │
    │         ├── PlayerID = ReceivedPlayerID
    │         ├── CharacterName = ReceivedName
    │         ├── Level = ReceivedLevel
    │         ├── CurrentHealth = ReceivedHP
    │         ├── MaxHealth = ReceivedMaxHP
    │         ├── CurrentMana = ReceivedMP
    │         ├── MaxMana = ReceivedMaxMP
    │         └── RemoteActor = SpawnedActor
    │
    └──► Get PlayerController → Get Player Selection Component
              │
              └──► Register Remote Player (PlayerInfo)
```

### Atualizar Informações

Quando receber updates de HP/MP via WebSocket:

```
[On Receive Player Update]
    │
    ├──► Create/Update FUmbraRemotePlayerInfo
    │
    └──► Get Player Selection Component
              │
              └──► Update Remote Player Info (PlayerInfo)
```

### Remover Jogador

Quando um jogador desconecta:

```
[On Player Disconnect]
    │
    └──► Get Player Selection Component
              │
              └──► Unregister Remote Player (PlayerID)
```

---

## Ações do Menu de Contexto

### Handler Principal (Já Implementado!)

O `UmbraGameInstance` agora possui todas as funções necessárias para o sistema social:

```cpp
// Handler principal - chamado automaticamente pelo PlayerSelectionComponent
void HandlePlayerContextAction(const FUmbraRemotePlayerInfo& PlayerInfo, EUmbraPlayerContextAction Action);
```

### Funções Disponíveis no GameInstance

#### Follow (Seguir)
```cpp
StartFollowingPlayer(int32 TargetPlayerID)   // Inicia seguir
StopFollowing()                               // Para de seguir
IsFollowingPlayer()                           // Verifica se está seguindo
GetFollowTargetID()                           // Obtém ID do alvo
```

#### Party (Grupo)
```cpp
SendPartyInvite(int32 TargetPlayerID, const FString& TargetPlayerName)
AcceptPartyInvite(int32 FromPlayerID)
DeclinePartyInvite(int32 FromPlayerID)
LeaveParty()
IsInParty()
```

#### Trade (Troca)
```cpp
RequestTrade(int32 TargetPlayerID, const FString& TargetPlayerName)
AcceptTradeRequest(int32 FromPlayerID)
DeclineTradeRequest(int32 FromPlayerID)
CancelTrade()
IsInTrade()
```

#### Friend (Amigo)
```cpp
SendFriendRequest(int32 TargetPlayerID, const FString& TargetPlayerName)
AcceptFriendRequest(int32 FromPlayerID)
DeclineFriendRequest(int32 FromPlayerID)
RemoveFriend(int32 FriendPlayerID)
LoadFriendList()
```

#### Whisper (Sussurrar)
```cpp
SendWhisper(int32 TargetPlayerID, const FString& Message)
```

#### Inspect (Inspecionar)
```cpp
InspectPlayer(int32 TargetPlayerID)
```

#### Duel (Duelo)
```cpp
SendDuelRequest(int32 TargetPlayerID, const FString& TargetPlayerName)
AcceptDuelRequest(int32 FromPlayerID)
DeclineDuelRequest(int32 FromPlayerID)
IsInDuel()
```

#### Report/Block (Denunciar/Bloquear)
```cpp
ReportPlayer(int32 TargetPlayerID, const FString& Reason)
BlockPlayer(int32 TargetPlayerID, const FString& PlayerName)
UnblockPlayer(int32 TargetPlayerID)
IsPlayerBlocked(int32 PlayerID)
```

### Delegates Disponíveis para Blueprint

Todos os eventos disparam delegates que você pode usar no Blueprint:

| Categoria | Delegates |
|-----------|-----------|
| **Follow** | `OnFollowStarted`, `OnFollowStopped`, `OnFollowFailed` |
| **Party** | `OnPartyInviteSent`, `OnPartyInviteReceived`, `OnPartyInviteFailed`, `OnPartyMemberJoined`, `OnPartyMemberLeft`, `OnPartyDisbanded` |
| **Trade** | `OnTradeRequestSent`, `OnTradeRequestReceived`, `OnTradeRequestFailed`, `OnTradeStarted`, `OnTradeCancelled`, `OnTradeCompleted` |
| **Friend** | `OnFriendRequestSent`, `OnFriendRequestReceived`, `OnFriendRequestFailed`, `OnFriendAdded`, `OnFriendRemoved`, `OnFriendListLoaded` |
| **Whisper** | `OnWhisperSent`, `OnWhisperReceived`, `OnWhisperFailed` |
| **Inspect** | `OnPlayerInspected`, `OnPlayerInspectFailed` |
| **Duel** | `OnDuelRequestSent`, `OnDuelRequestReceived`, `OnDuelRequestFailed`, `OnDuelStarted`, `OnDuelEnded` |
| **Report/Block** | `OnPlayerReported`, `OnPlayerReportFailed`, `OnPlayerBlocked`, `OnPlayerUnblocked`, `OnBlockFailed` |

### Via Blueprint

Conecte o delegate `OnPlayerContextActionSelected` ao handler:

```
[Bind Event to OnPlayerContextActionSelected]
    │
    └──► [CustomEvent: OnContextAction(PlayerInfo, Action)]
              │
              └──► Get Game Instance → Cast to UmbraGameInstance
                        │
                        └──► Handle Player Context Action (PlayerInfo, Action)
```

Ou use as funções individuais diretamente:

```
[BTN_Trade OnClicked]
    │
    ├──► Get Game Instance → Cast to UmbraGameInstance
    │         │
    │         └──► Request Trade (CachedPlayerInfo.PlayerID, CachedPlayerInfo.CharacterName)
    │
    └──► Hide Menu
```

---

## Resumo de Passos

1. ✅ Compilar o projeto C++ (estruturas e componentes já criados)
2. ✅ APIs PHP criadas (`/api/social/*`)
3. ✅ Handlers WebSocket implementados (C++)
4. ✅ GameInstance atualizado com funções HTTP e WebSocket
5. ⬜ Executar script SQL (`create_social_tables.sql`)
6. ⬜ Adicionar tag `RemotePlayer` ao Blueprint do Remote Actor
7. ⬜ Configurar Input Actions (IA_SelectPlayer, IA_ContextMenu)
8. ⬜ Criar `WBP_SelectedPlayerInfo`
9. ⬜ Criar `WBP_PlayerContextMenu`
10. ⬜ Conectar delegates no HUD/Level Blueprint
11. ⬜ Integrar WebSocket no `BP_NetMovementClient` (ver guia de integração)
12. ⬜ Integrar com NetMovementClient (Register/Update/Unregister)

---

## Troubleshooting

### Jogador não é selecionado ao clicar

1. Verifique se a tag `RemotePlayer` está no Actor
2. Verifique se o collision está ativo
3. No componente `UmbraPlayerSelectionComponent`, verifique `SelectionTraceChannel`
4. Adicione logs temporários no `TrySelectPlayerUnderCursor`

### Widget não aparece

1. Verifique se os delegates estão conectados
2. Verifique se o widget está adicionado ao viewport
3. Verifique a visibilidade inicial do widget

### Menu de contexto na posição errada

1. A posição recebida é em Screen Space
2. Use `Set Render Transform` ou `Set Position in Viewport`
3. Considere ajustar para não sair da tela

---

## Próximos Passos

Após implementar este sistema, você terá a base para:

1. ✅ **Sistema de Trade** - APIs e WebSocket implementados
2. ✅ **Sistema de Grupo** - APIs e WebSocket implementados
3. ✅ **Lista de Amigos** - APIs e WebSocket implementados
4. ✅ **Whisper/Chat** - WebSocket implementado
5. ✅ **Bloqueio/Denúncia** - APIs implementadas
6. ✅ **Inspeção** - API implementada
7. ⬜ **Sistema de Follow** - Lógica de movimento a implementar

## Integração WebSocket

Para notificações em tempo real, consulte:
📖 **`GUIA_INTEGRACAO_SISTEMA_SOCIAL.md`**

Este guia explica como:
- Conectar WebSocket ao GameInstance
- Processar mensagens sociais recebidas
- Enviar mensagens via WebSocket
- Carregar convites pendentes ao entrar no jogo
