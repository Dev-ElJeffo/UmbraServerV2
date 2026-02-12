 Procedimento: Sistema de Amizade (Friends) completo

Guia para implementar o sistema de amizade com mínimo de Blueprint e máximo de C++, alinhado ao padrão já usado em **Party** e **Trade**.

---

## Visão geral
| Componente | Responsabilidade |
|------------|------------------|
| **WBP_PlayerHUD** | BTN_FriendList abre WBP_FriendList; bind OnFriendRequestReceived → exibe WBP_FriendRequested |
| **WBP_FriendList** | Lista de amigos (ScrollBox) + “Pending Invites” (solicitações pendentes); LoadFriendList / LoadPendingInvites |
| **WBP_FriendRequested** | Pop-up “Add XXXXXXXX to friendlist?” com Accept / Decline |
| **UmbraGameInstance** | SendFriendRequest, AcceptFriendRequest, DeclineFriendRequest, RemoveFriend, LoadFriendList, LoadPendingInvites + getters para UI |
| **APIs PHP** | send_friend_request, accept_friend_request, get_friend_list, get_pending_invites?type=friend, remove_friend |
| **WebSocket** | msgType 31 = FriendRequest (notificação em tempo real) |

---

## Comparação com Trade e Party

| Aspecto | Trade | Party | Friend |
|---------|--------|--------|--------|
| Enviar | SendTradeRequest → API → SendTradeRequestViaWebSocket | SendPartyInvite → API → SendPartyInviteViaWebSocket | SendFriendRequest → API → SendFriendRequestViaWebSocket |
| Receber (tempo real) | msgType 21 → OnTradeRequestReceived | msgType 11 → OnPartyInviteReceived | msgType 31 → OnFriendRequestReceived |
| Aceitar | AcceptTradeRequest(FromPlayerID) | AcceptPartyInvite(FromPlayerID) | AcceptFriendRequest(FromPlayerID) |
| Recusar | DeclineTradeRequest(FromPlayerID) | DeclinePartyInvite(FromPlayerID) | DeclineFriendRequest(FromPlayerID) |
| Lista no cliente | — | CurrentPartyState, OnPartyStateLoaded | FriendPlayerIDs + FriendListNames, OnFriendListLoaded |
| Pendentes ao logar | — | LoadPendingInvites("all") | LoadPendingInvites("friend") ou "all" |

---

## Parte 1: APIs PHP (já existentes)

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `send_friend_request.php` | POST | Envia solicitação (body: `target_player_id`, header: Authorization) |
| `accept_friend_request.php` | POST | Aceita (body: `from_player_id` ou `request_id`, header: Authorization) |
| `get_friend_list.php` | GET | Retorna `success`, `friends[]` (friend_id, friend_name, friend_level, …), `count` |
| `get_pending_invites.php?type=friend` | GET | Retorna `data.friend_requests[]` (from_player_id, from_player_name) |
| `remove_friend.php` | POST | Remove amizade (body: `friend_player_id`) |

Schema: tabelas `friend_requests` e `friends` em `www/umbra_api/scripts/create_social_tables.sql`.

---

## Parte 2: C++ (UmbraGameInstance)

### Funções já existentes (BlueprintCallable)

- `SendFriendRequest(TargetPlayerID, TargetPlayerName)` — envia solicitação (HTTP + WebSocket após sucesso)
- `AcceptFriendRequest(FromPlayerID)` — aceita
- `DeclineFriendRequest(FromPlayerID)` — recusa (só local)
- `RemoveFriend(FriendPlayerID)` — remove da lista
- `LoadFriendList()` — GET get_friend_list.php, preenche listas e dispara OnFriendListLoaded
- `LoadPendingInvites(Type)` — GET get_pending_invites.php; para Type = `"friend"` só friend_requests

### Getters para a UI (BlueprintPure)

- `GetFriendCount()` — número de amigos (após LoadFriendList)
- `GetFriendIDAt(Index)` — ID do amigo no índice (0-based); 0 se inválido
- `GetFriendName(FriendID)` — nome do amigo; vazio se não encontrado
- `GetPendingFriendRequestCount()` — número de solicitações pendentes
- `GetPendingFriendRequestIDAt(Index)` — ID do solicitante no índice
- `GetPendingFriendRequestName(FromPlayerID)` — nome do solicitante

### Delegates

- `OnFriendRequestSent(TargetPlayerID, TargetPlayerName)`
- `OnFriendRequestReceived(FromPlayerID, FromPlayerName)` — ao receber WebSocket 31 ou ao processar LoadPendingInvites
- `OnFriendRequestFailed(ErrorMessage)`
- `OnFriendAdded(PlayerID, PlayerName)` — após aceitar
- `OnFriendRemoved(PlayerID, PlayerName)` — após remover
- `OnFriendListLoaded()` — após LoadFriendList concluir

### Dados internos

- `FriendPlayerIDs` (TArray) — ordem da API
- `FriendListNames` (TMap ID → Nome) — para exibir na lista
- `PendingFriendRequests` (TMap ID → Nome)
- `PendingFriendRequestIDs` (TArray) — ordem estável para índice na UI

---

## Parte 3: WBP_FriendRequested (pop-up “Add X to friendlist?”)

Objetivo: exibir quando o jogador **recebe** uma solicitação de amizade; botões Accept e Decline.

### 3.1 Estrutura (você já tem)

- **Text_PartyFrom** — texto tipo “Add XXXXXXXX to friendlist?”
- **BTN_Accept** — Aceitar
- **BTN_Decline** — Recusar
- **Btn_Close** — Fechar (equivalente a recusar)

### 3.2 Variáveis do widget

- `FromPlayerID` (Integer)
- `FromPlayerName` (String)

### 3.3 Função `ShowFriendRequest(FromPlayerID, FromPlayerName)`

1. Set `FromPlayerID` e `FromPlayerName`.
2. Atualizar **Text_PartyFrom**: “Add **{0}** to friendlist?” com `FromPlayerName` em {0}.
3. Set Visibility = Visible.

### 3.4 Botões

**BTN_Decline / Btn_Close**

- Get Game Instance → Cast to UmbraGameInstance → **Decline Friend Request** (From Player ID = variável `FromPlayerID` do widget).
- **Remove From Parent**.

**BTN_Accept**

- Get Game Instance → Cast to UmbraGameInstance → **Accept Friend Request** (From Player ID = `FromPlayerID`).
- **Remove From Parent**.

(Após aceitar, o GameInstance dispara `OnFriendAdded`; a lista de amigos pode ser atualizada ao reabrir WBP_FriendList ou ao fazer bind de OnFriendAdded.)

---

## Parte 4: WBP_PlayerHUD – BTN_FriendList e notificação de amizade

### 4.1 BTN_FriendList – On Clicked

- Get Game Instance → Cast to UmbraGameInstance.
- **Load Friend List** (para garantir dados atualizados).
- Create Widget **WBP_FriendList** (se ainda não tiver referência guardada), Owning Player = Get Owning Player.
- Add to Viewport (ou Add Child ao Canvas do HUD).
- Set Visibility = Visible.

Recomendação: guardar referência ao widget da friend list para reutilizar (evitar vários painéis abertos). Se já existir e estiver visível, pode apenas trazer ao foco ou não criar outro.

### 4.2 Bind de OnFriendRequestReceived (como em Trade)

No **Event Construct** (ou BeginPlay) do HUD:

- Get Game Instance → Cast to UmbraGameInstance.
- **Bind Event to On Friend Request Received** → Event = **Custom Event** (ex.: `OnFriendRequestReceived_Event` com parâmetros FromPlayerID, FromPlayerName).

Não coloque Create Widget no “then” do Bind; o “then” só segue para o próximo bind ou fica livre.

### 4.3 Custom Event OnFriendRequestReceived_Event(FromPlayerID, FromPlayerName)

- **Create Widget** (Class = WBP_FriendRequested, Owning Player = Get Owning Player).
- Chamar **Show Friend Request** no widget criado (FromPlayerID, FromPlayerName).
- **Add to Viewport** (widget criado).

Ordem: Create Widget → Show Friend Request → Add to Viewport.

Assim, ao receber WebSocket 31 ou ao processar friend_requests de LoadPendingInvites, o pop-up aparece com o nome correto.

---

## Parte 5: WBP_FriendList – lista de amigos e pending invites

### 5.1 Elementos que você já tem

- **Text_PartyFrom** — pode exibir “X friends” (count).
- **BTN_PendingInvites** — abre/atualiza solicitações pendentes.
- **BTN_RemoveFriend** — remove amigo (precisa de qual linha está selecionada; ver abaixo).
- **Btn_Close** — Remove From Parent.

### 5.2 Adicionar lista de amigos (Scroll Box + container)

No Unreal não existe nó "Add Child to Scroll Box"; existem **Add Child to Vertical Box**, **Add Child to Grid**, **Add Child to Uniform Grid**. Por isso: coloque um **Vertical Box** ou **Grid** *dentro* do Scroll Box e adicione os slots a esse container.

Dentro do layout (ex.: dentro do mesmo Vertical Box ou da área central):

- Adicionar um **Scroll Box** (nome sugerido: **ScrollBox_Friends**).
- Opção A: vários **Text Block** como filhos (um por amigo), criados em runtime.
- Opção B (recomendado): criar um **Widget Blueprint** de uma linha (ex.: **WBP_FriendSlot**) com um Text Block para o nome e, opcionalmente, botão “Remove”. Os slots são adicionados ao **container** (Vertical Box ou Grid) que fica *dentro* do Scroll Box — use **Add Child to Vertical Box** ou **Add Child to Grid**.

### 5.3 Preencher lista de amigos (após LoadFriendList)

Quando o painel for aberto, o HUD chama **Load Friend List**. No **WBP_FriendList**:

1. No **Event Construct** (ou quando o widget for exibido): fazer bind de **On Friend List Loaded** do GameInstance.
2. No **Custom Event** ligado a On Friend List Loaded:
   - Limpar filhos do **container** (Vertical Box ou Grid que está *dentro* do Scroll Box): Clear Children no container.
   - Get Game Instance → Cast to UmbraGameInstance.
   - **Get Friend Count** → usar em **For Loop** (0 a Count-1).
   - No loop: **Get Friend ID At** (Index) → **Get Friend Name** (esse ID).
   - Para cada índice: Create Widget **WBP_FriendSlot**, Set Text = nome no slot, **Add Child to Vertical Box** (ou **Add Child to Grid**) — Target = o container (VB_Friends / Grid_Friends), Content = widget criado.

Atualizar também **Text_PartyFrom** com “**X** friends” (X = Get Friend Count).

### 5.3.1 Passo a passo no Blueprint: Scroll Box + WBP_FriendSlot

Pré-requisito no **WBP_FriendSlot**:
- Um **Text Block** para o nome (ex.: `Text_FriendName`). Marque **Is Variable** se for definir o texto de fora.
- Opcional: variável **FriendID** (Integer) para o botão Remove usar depois.

No **WBP_FriendList**, no **Custom Event** que é chamado quando **On Friend List Loaded** dispara:

| Passo | Nó / ação |
|-------|-----------|
| 1 | **Clear Children** (Widget = **VB_Friends** ou **Grid_Friends**, o container *dentro* do Scroll Box). Se não existir “Clear Children”, use um **For Loop** de 0 a (Get Children Count do Scroll Box − 1) e **Remove Child At** (Index 0 sempre, pois a cada remoção o próximo passa a ser 0). |
| 2 | **Get Game Instance** → **Cast to Umbra Game Instance** → guardar (ex.: variável `MyGI`). |
| 3 | **Get Friend Count** (Target = MyGI) → **Last Index** = esse valor **− 1** (para o For Loop incluir 0 a Count-1). |
| 4 | **For Loop**: **First Index** = 0, **Last Index** = (Get Friend Count − 1). **Loop Body** recebe o **Index**. |
| 5 | Dentro do Loop Body: |
| 5a | **Get Friend ID At** (Target = MyGI, Index = **Index** do loop) → guardar em `FriendID`. |
| 5b | **Get Friend Name** (Target = MyGI, Friend ID = `FriendID`) → guardar em `FriendName`. |
| 5c | **Create Widget**: Class = **WBP_FriendSlot**, Owning Player = **Get Owning Player**. **Return Value** = o slot criado. |
| 5d | No **WBP_FriendSlot** criado: **Set Text** no Text Block do nome (ex.: `Text_FriendName`) = `FriendName`. Se o slot tiver função **Set Friend Data(FriendID, FriendName)**, use ela em vez de Set Text direto. |
| 5e | **Add Child to Vertical Box** (ou **Add Child to Grid**): **Target** = **VB_Friends** / **Grid_Friends**, **Content** = o widget criado. |
| 6 | Fora do loop: **Get Friend Count** → **Format Text** “{0} friends” com esse número → **Set Text** em **Text_PartyFrom**. |

Ordem dentro do loop: **Get Friend ID At** → **Get Friend Name** → **Create Widget (WBP_FriendSlot)** → Set nome no slot → **Add Child to Vertical Box** ou **Add Child to Grid** (Target = container, Content = slot criado).

**Importante:** Use **Add Child to Vertical Box** ou **Add Child to Grid** está em **Widget** (procure “Add Child” com Target sendo um Scroll Box). O **Content** é o widget que você criou (WBP_FriendSlot).

### 5.4 Botão “Pending Invites”

**BTN_PendingInvites** – On Clicked:

- Get Game Instance → Cast to UmbraGameInstance.
- **Load Pending Invites** (Type = **"friend"**).
- Opção A: preencher um segundo **Scroll Box** (ex.: **ScrollBox_PendingRequests**) com os pendentes:
  - **Get Pending Friend Request Count**.
  - For Loop de 0 a Count-1:
    - **Get Pending Friend Request ID At** (Index).
    - **Get Pending Friend Request Name** (esse ID).
    - Criar linha (Text ou WBP_FriendRequestSlot) com nome e botões Accept/Decline, Add Child ao ScrollBox_PendingRequests.
- Opção B: trocar a visibilidade de duas áreas (uma com lista de amigos, outra com lista de pendentes) e preencher a de pendentes quando o usuário clicar em Pending Invites.

Assim, “ao clicar abre as solicitações pendentes” fica coberto. **Implementação:** ao clicar, fazer **Load Pending Invites**("friend"); em seguida **For Loop** (0 a Get Pending Friend Request Count − 1); em cada iteração: **Get Pending Friend Request ID At**(Index) → FromPlayerID, **Get Pending Friend Request Name**(FromPlayerID) → FromPlayerName; **Create Widget** (WBP_FriendRequested); **Show Friend Request**(FromPlayerID, FromPlayerName); **Add to Viewport**. Assim abre um WBP_FriendRequested por solicitação; o jogador aceita ou recusa em cada pop-up.

### 5.5 Remove Friend (BTN_RemoveFriend)

Se **BTN_RemoveFriend** for global (remove um amigo selecionado):

- Manter uma variável no widget, ex.: `SelectedFriendID` (int32).
- Ao clicar numa linha da lista (ou num slot), Set `SelectedFriendID` = Get Friend ID At (índice da linha).
- BTN_RemoveFriend On Clicked: Get Game Instance → **Remove Friend** (`SelectedFriendID`), depois **Load Friend List** e re-preencher a lista (ou escutar **On Friend Removed** e atualizar a lista).

Se cada linha tiver seu próprio botão “Remove”, no WBP_FriendSlot passe o FriendID para o slot e no On Clicked do botão chame Remove Friend(FriendID) e atualize a lista.

### 5.6 Btn_Close

- **Remove From Parent** (fecha o painel).

---

## Parte 6: Context Menu – “Add Friend”

No menu de contexto do jogador (igual a Invite to Party / Trade):

- Botão “Add Friend” deve chamar **Execute Context Action** com ação do tipo **AddFriend** (ou o enum que você usar), passando o PlayerID/nome do jogador selecionado.
- No C++, essa ação chama **SendFriendRequest(TargetPlayerID, TargetPlayerName)** (já existe no GameInstance).

Verifique no seu **GUIA_WBP_CONTEXT_MENU_BOTOES** se já existe mapeamento para “Add Friend” e qual enum usar.

---

## Parte 7: WebSocket (msgType 31)

O Zone Server deve enviar ao destinatário a mensagem tipo **31** (FriendRequest) quando outro jogador envia solicitação de amizade (após o cliente do remetente chamar SendFriendRequestViaWebSocket). O formato já usado no cliente:

- Data[0] = 31
- Data[1..4] = FromPlayerID (little-endian)
- Data[5..8] = ToPlayerID (little-endian)

O **NetMovementClient** chama **ProcessSocialWebSocketMessage** para msgType 31; o GameInstance preenche **PendingFriendRequests** e **PendingFriendRequestIDs** e dispara **OnFriendRequestReceived**. Nada extra no Blueprint além do bind no HUD.

---

## Resumo de verificação

| Item | Status |
|------|--------|
| APIs PHP (send/accept/get_friend_list/get_pending_invites/remove_friend) | ✅ |
| C++ SendFriendRequest, Accept, Decline, RemoveFriend, LoadFriendList, LoadPendingInvites | ✅ |
| C++ GetFriendCount, GetFriendIDAt, GetFriendName, GetFriendIsOnline, GetPendingFriendRequest* | ✅ |
| C++ UmbraFriendSlotWidget, UmbraFriendOptionsWidget, UmbraFriendRemoveConfirmWidget | ✅ |
| API get_friend_list retorna is_online (por enquanto sempre false) | ✅ |
| WBP_FriendRequested: ShowFriendRequest, BTN_Accept/Decline/Close | ⬜ |
| WBP_PlayerHUD: BTN_FriendList abre WBP_FriendList e chama LoadFriendList | ✅ |
| WBP_PlayerHUD: Bind OnFriendRequestReceived → Create WBP_FriendRequested + Show + AddToViewport | ⬜ |
| WBP_FriendList: OnFriendListLoaded → preencher ScrollBox_Friends + “X friends” | ⬜ |
| WBP_FriendList: BTN_PendingInvites → LoadPendingInvites("friend") + preencher pendentes | ⬜ |
| WBP_FriendList: lista com nomes dos amigos (ScrollBox + itens dinâmicos) | ⬜ |
| WBP_FriendList: BTN_RemoveFriend (com amigo selecionado ou por linha) | ⬜ |
| Context menu: botão Add Friend → SendFriendRequest | ⬜ |

---

## Troubleshooting

- **Pop-up de amizade não aparece ao receber solicitação**  
  Verifique bind de **On Friend Request Received** no HUD e se Create Widget + Show Friend Request + Add to Viewport estão **no Custom Event** do delegate, não no “then” do Bind.

- **Lista de amigos vazia / "XXXXXXXX friends"**  
  O fluxo usa o **mesmo padrão** dos outros sistemas (Get Game Instance → Cast to Umbra Game Instance; Load Friend List ao abrir o widget). Confirme: (1) No fluxo que abre WBP_FriendList está sendo chamado **Load Friend List** no Game Instance. (2) No Output Log, ao abrir a lista, deve aparecer `[UmbraGameInstance] 📋 Carregando lista de amigos...` e depois `✅ Lista de amigos carregada: N amigos` e `Broadcast OnFriendListLoaded` — se esses logs aparecerem e a UI não atualizar, o problema está no Blueprint (evento/delegate); se não aparecerem, a requisição GET ou o parsing falharam.

- **Pending Invites vazia**  
  BTN_PendingInvites deve chamar **Load Pending Invites** com Type = `"friend"`. O servidor retorna `data.friend_requests`. O cliente preenche **PendingFriendRequestIDs** e **PendingFriendRequests**; use os getters **Get Pending Friend Request Count / ID At / Name** para montar a lista.

- **WebSocket 31 não dispara**  
  Confirme que o remetente está conectado ao Zone (WebSocket) e que, após sucesso da API send_friend_request, o cliente chama **SendFriendRequestViaWebSocket**. O servidor Zone deve repassar a mensagem ao jogador destino.

- **Accept/Decline não fazem nada**  
  Garanta que o widget guarda **FromPlayerID** e passa esse valor para **Accept Friend Request** e **Decline Friend Request**.

Este procedimento deixa o sistema de amizade alinhado às ações sociais já implementadas (Trade e Party), com uso mínimo de Blueprint e lógica centralizada no C++.
