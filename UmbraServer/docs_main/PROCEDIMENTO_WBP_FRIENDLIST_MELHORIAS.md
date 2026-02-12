# Procedimento: Melhorias no WBP_FriendList

## Objetivo
Adicionar círculo de status online/offline no WBP_FriendSlot e sistema de opções ao clicar no slot (WBP_FriendOptions e WBP_FriendRemoveConfirm).

**Se “Remove Friend” não faz nada:** veja **Passos concretos 1** (BTN_RemoveFriend).  
**Se não sabe onde ficam report_online/offline:** veja **Passos concretos 2** (já estão no C++; basta compilar).

---

## Parte 1: C++ - Widgets Criados

### 1.1 UmbraFriendSlotWidget
- **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraFriendSlotWidget.h/cpp`
- **Funcionalidade:** Slot de amigo com nome e círculo de status online/offline
- **Delegates:** `OnFriendSlotClicked(FriendID, FriendName)` — disparado ao clicar no Button_Slot
- **Métodos:**
  - `SetFriendData(FriendID, FriendName, bIsOnline)` — preenche dados do amigo
  - `ClearSlot()` — limpa o slot

### 1.2 UmbraFriendOptionsWidget
- **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraFriendOptionsWidget.h/cpp`
- **Funcionalidade:** Widget de opções (Invite to Party, Private Message, Remove Friend)
- **Métodos:**
  - `ShowOptions(FriendID, FriendName)` — exibe opções para um amigo
  - `CloseOptions()` — fecha o widget
- **Delegates:** `OnRemoveFriendRequested(FriendID, FriendName)` — disparado ao clicar em Remove Friend

### 1.3 UmbraFriendRemoveConfirmWidget
- **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraFriendRemoveConfirmWidget.h/cpp`
- **Funcionalidade:** Widget de confirmação para remover amigo
- **Métodos:**
  - `ShowConfirm(FriendID, FriendName)` — exibe confirmação
  - `CloseConfirm()` — fecha o widget
- **Ações:** Confirm chama `UmbraGameInstance::RemoveFriend()`, Cancel fecha

### 1.4 UmbraGameInstance - Novos Getters
- `GetFriendIsOnline(int32 FriendID) const` — retorna `true` se online, `false` se offline
- `FriendOnlineStatus` (TMap<int32, bool>) — armazena status online dos amigos

---

## Parte 2: API PHP e tabela - Status Online

### 2.1 Tabela player_sessions
- **Script:** `www/umbra_api/scripts/create_player_sessions_table.sql`
- **Estrutura:** `player_id` (PK), `last_seen` (timestamp). Um registro por jogador; atualizado ao reportar online.
- **Regra:** `last_seen` nos últimos **2 minutos** = jogador considerado online.

### 2.2 report_online.php (POST)
- **Uso:** Cliente chama ao **conectar ao WebSocket do Zone** (após ter `MyPlayerId`).
- **Auth:** JWT (player_id do token).
- **Ação:** `INSERT INTO player_sessions ... ON DUPLICATE KEY UPDATE last_seen = NOW()`.

### 2.3 report_offline.php (POST)
- **Uso:** Cliente chama ao **desconectar do WebSocket do Zone**.
- **Auth:** JWT.
- **Ação:** `DELETE FROM player_sessions WHERE player_id = :player_id`.

### 2.4 get_friend_list.php
- **Campo:** `is_online` (bool) em cada amigo.
- **Lógica:** Consulta `player_sessions` onde `last_seen >= NOW() - INTERVAL 2 MINUTE`; se `friend_id` está nessa lista, `is_online = true`.

### 2.5 Onde inserir report_online e report_offline (cliente C++)
- **report_online:** Chamar quando o cliente **conectar** ao WebSocket do Zone e **periodicamente** enquanto estiver conectado (heartbeat).
  - **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/NetMovementClient.cpp`
  - **Função:** `HandleWebSocketConnected()` — após `LoadPartyState(0)` chamar `MyGameInstance->ReportSessionOnline()` e iniciar timer de **heartbeat** (a cada 90 s) que chama novamente `ReportSessionOnline()`. Assim `last_seen` não ultrapassa a janela de 2 min e o jogador continua aparecendo online na Friend List.
  - **GameInstance:** `UUmbraGameInstance::ReportSessionOnline()` faz POST para `/api/social/report_online.php` com token (CreateRequest já envia o header).
- **report_offline:** Chamar quando o cliente **desconectar** do WebSocket do Zone.
  - **Arquivo:** mesmo `NetMovementClient.cpp`
  - **Função:** `HandleWebSocketClosed()` — cancelar o timer de heartbeat, e junto com `LeavePartyOnDisconnect()` chamar `MyGameInstance->ReportSessionOffline()`.
  - **GameInstance:** `UUmbraGameInstance::ReportSessionOffline()` faz POST para `/api/social/report_offline.php` com token.
- Não é necessário bind em Blueprint; o fluxo é automático ao conectar/desconectar do Zone.
- Se a tabela não existir, `get_friend_list` trata exceção e retorna todos `is_online = false`.

---

## Parte 3: Blueprint - WBP_FriendSlot

### 3.1 Estrutura do Widget
1. **Root:** Border ou Canvas Panel
2. **Text_FriendName:** TextBlock com nome do amigo (Bind Widget)
3. **Image_OnlineStatus:** Image (círculo pequeno) — Bind Widget
4. **Button_Slot:** Button transparente cobrindo toda a área — Bind Widget

### 3.2 Ao abrir / exibir o painel (obrigatório para status online correto)

Se o painel for **só escondido** (visibility) em vez de removido, ao reabrir o **Event Construct** não roda de novo e a lista fica com dados antigos (ex.: todos offline). Por isso:

- No **Event Construct**: chamar **Load Friend List** e **Start Friend List Auto Refresh** (ex.: 30 s).
- **Quando o painel for exibido de novo** (mesmo widget só ficando visível): chamar **Load Friend List** de novo.
  - Se o painel for um **tab** ou **subpainel** que só muda visibilidade: usar **Event On Visibility Changed** → quando **Visibility** for **Visible**, chamar **Load Friend List** (e opcionalmente **Start Friend List Auto Refresh**).
  - Assim, ao fechar e abrir de novo (ou trocar de tab e voltar), a lista é atualizada e o **Image_OnlineStatus** fica correto.

### 3.3 Event Graph - OnFriendListLoaded_Event
**Sempre limpar o VBox antes de preencher** (senão slots antigos ficam e o status não atualiza):

```
- Clear Children (no VBox_Friends / Vertical Box da lista)
- For Loop (0 to GetFriendCount-1):
  - GetFriendIDAt(Index) → FriendID
  - GetFriendName(FriendID) → FriendName
  - GetFriendIsOnline(FriendID) → bIsOnline
  
  - Create Widget (Class = UmbraFriendSlotWidget)
  - Set Friend Data (FriendID, FriendName, bIsOnline)
  - Bind Event to On Friend Slot Clicked → Custom Event (OnFriendSlotClicked_Event)
  - Add Child to Vertical Box (VBox_Friends)
```

### 3.4 Custom Event OnFriendSlotClicked_Event(FriendID, FriendName)
```
- Create Widget (Class = WBP_FriendOptions)
- Show Options (FriendID, FriendName)
- Add to Viewport
```

---

## Parte 4: Blueprint - WBP_FriendOptions

### 4.1 Estrutura do Widget
- **Text_FriendName:** TextBlock com nome do amigo
- **BTN_InviteToParty:** Button "Invite to Party"
- **BTN_PrivateMessage:** Button "Private Message" (desabilitado por enquanto)
- **BTN_RemoveFriend:** Button "Remove Friend"
- **BTN_Close:** Button "X" ou "Close"

### 4.2 Event Graph - Construct
- **Bind Event to On Remove Friend Requested** → Custom Event (OnRemoveFriendRequested_Event)

### 4.3 On Clicked BTN_RemoveFriend (procedimento)
- **No C++:** O botão **BTN_RemoveFriend** está ligado a **OnRemoveFriendClicked()** em `UmbraFriendOptionsWidget.cpp`. Esse método faz:
  - **OnRemoveFriendRequested.Broadcast(CurrentFriendID, CurrentFriendName)**
  - **CloseOptions()**
- **No Blueprint:** Não é necessário criar outro OnClicked para BTN_RemoveFriend. O fluxo é:
  1. Usuário clica em "Remove Friend" → C++ dispara o delegate **OnRemoveFriendRequested**.
  2. No **Construct** do WBP_FriendOptions você já fez **Bind Event to On Remove Friend Requested** → **OnRemoveFriendRequested_Event**.
  3. O **OnRemoveFriendRequested_Event** deve criar o WBP_FriendRemoveConfirm e mostrá-lo (ver 4.4).

### 4.4 Custom Event OnRemoveFriendRequested_Event(FriendID, FriendName)
```
- Create Widget (Class = WBP_FriendRemoveConfirm)
- Show Confirm (FriendID, FriendName)
- Add to Viewport
```

### 4.5 BTN_InviteToParty (OnClicked)
- **Get Game Instance** → **Cast to Umbra Game Instance** → **Send Party Invite**(FriendID, FriendName)
- **Close Options**

### 4.6 BTN_PrivateMessage (OnClicked)
- Por enquanto: log ou mensagem "Em breve"
- **Close Options**

---

## Parte 5: Blueprint - WBP_FriendRemoveConfirm

### 5.1 Estrutura do Widget
- **Text_Message:** TextBlock com mensagem "Tem certeza que deseja remover [Nome] da sua lista de amigos?"
- **BTN_Confirm** ou **BTN_ACCEPT:** botão que confirma a remoção (pode usar um ou outro; o C++ trata os dois).
- **BTN_Cancel:** Button "Cancel"

### 5.2 Procedimento para BTN_ACCEPT (ou BTN_Confirm) – On Clicked

O comportamento do botão **Accept** (confirmar remoção) já está implementado no C++. Você só precisa garantir **nome** e **Parent Class** corretos.

**No Designer (WBP_FriendRemoveConfirm):**
1. **Parent Class** do widget deve ser **UmbraFriendRemoveConfirmWidget**.
2. O botão que confirma a remoção deve ter **nome** exatamente:
   - **BTN_Confirm**, ou
   - **BTN_ACCEPT**  
   (o C++ liga o clique aos dois; use um deles).
3. O texto visível do botão pode ser "Accept", "Confirmar", "Confirm", etc. — o que importa é o **Name** na Hierarchy (BTN_ACCEPT ou BTN_Confirm).

**No Blueprint (Event Graph):**  
Não é necessário criar **OnClicked** para esse botão no Blueprint. O C++ faz:
- Ao clicar em **BTN_ACCEPT** ou **BTN_Confirm** → **OnConfirmClicked()** → **Get Game Instance** → **RemoveFriend(CurrentFriendID)** → **CloseConfirm()** (que chama **RemoveFromParent()**).

Se quiser fazer no Blueprint em vez do C++ (widget sem parent C++):
- **On Clicked** do BTN_ACCEPT (ou BTN_Confirm): **Get Game Instance** → **Cast to Umbra Game Instance** → **Remove Friend**(CurrentFriendID) → **Close Confirm**.

### 5.3 BTN_Cancel (OnClicked)
- **Close Confirm** (no C++ já está ligado a **BTN_Cancel**; mesmo critério de nome exato no Designer).

### 5.4 World leak (PIE)
- **CloseConfirm()** em C++ chama **RemoveFromParent()** para o widget sair da hierarquia; caso contrário o widget continua referenciando o World e ao encerrar PIE ocorre "Fatal world leaks detected". Em Blueprint puro, ao fechar o confirm chamar também **Remove from Parent** no widget.

---

## Passos concretos 1: BTN_RemoveFriend (clique não faz nada)

Se o menu de opções (WBP_FriendOptions) abre, mas ao clicar em **Remove Friend** nada acontece, siga estes passos na ordem.

### A. Verificar que o widget é filho da classe C++

1. Abra o **Content Browser** do Unreal e localize **WBP_FriendOptions**.
2. Clique com o botão direito no asset → **Asset Actions** → **Reference Viewer** (opcional) ou abra o widget (duplo clique).
3. Com o **WBP_FriendOptions** aberto no editor de Widget Blueprint, olhe o **Details** quando nenhum elemento está selecionado (ou selecione o **Canvas Panel** raiz).
4. Em **Details**, procure **Parent Class**.
5. **Deve estar:** `UmbraFriendOptionsWidget` (ou `Umbra Friend Options Widget`).
6. Se estiver **User Widget** ou outro pai: no dropdown **Parent Class** mude para **UmbraFriendOptionsWidget**. Salve. (Pode ser necessário recriar o layout se o pai for diferente.)

Sem isso, o C++ que liga o botão ao delegate **nunca roda**, e o clique não dispara nada.

### B. Nome exato do botão no Designer

1. Dentro do **WBP_FriendOptions**, na árvore **Hierarchy**, localize o botão de remover amigo.
2. Selecione-o e no **Details** veja o campo **Name** (nome do widget).
3. O nome **tem que ser exatamente:** `BTN_RemoveFriend` (igual ao C++).
4. Se for outro (ex.: `Button_RemoveFriend`, `RemoveFriend_Button`): renomeie para `BTN_RemoveFriend`.
5. Salve o widget.

O C++ usa `meta = (BindWidgetOptional)` e procura um widget com nome `BTN_RemoveFriend`. Nome diferente = variável fica `nullptr` e o clique não é ligado.

### C. Event Graph – Bind e Custom Event

1. Abra **WBP_FriendOptions** e vá na aba **Graph** (Event Graph).
2. **No Event Construct:**
   - Arraste um fio a partir do pino **exec** (branco) do **Event Construct**.
   - No menu de busca digite: **Bind Event to On Remove Friend Requested**.
   - Escolha o nó **Bind Event to On Remove Friend Requested** (ele tem **Target** e **Event**).
   - **Target:** deixe como **self** (referência ao próprio widget). Se não estiver, arraste do **self** do Event Construct para o pino **Target**.
   - O pino **Event** (delegate) deve ligar a um **Custom Event**.
3. **Criar o Custom Event que recebe o clique:**
   - Clique com o botão direito no grid → **Add Custom Event**.
   - Nome sugerido: **OnRemoveFriendRequested_Event**.
   - Nesse Custom Event, **adicione dois parâmetros de entrada:** um **Integer** (nome: `FriendID`) e um **String** (nome: `FriendName`). No Unreal: selecione o nó do Custom Event → no **Details** → **Inputs** → **+** e adicione um **Integer** e um **String**.
   - Ligue o pino **Event** do **Bind Event to On Remove Friend Requested** ao **input exec** desse Custom Event.
4. **Dentro do Custom Event** (quando Remove Friend for clicado):
   - **Create Widget:** classe **WBP_FriendRemoveConfirm** (ou a classe C++ `UmbraFriendRemoveConfirmWidget` se o Blueprint tiver outro nome). Guarde o **Return Value** (o widget criado).
   - No **Return Value**, chame **Show Confirm**. Nos parâmetros: **Friend ID** = parâmetro `FriendID` do evento; **Friend Name** = parâmetro `FriendName` do evento.
   - Ainda no **Return Value**, chame **Add to Viewport** (sem parâmetros).
5. Fique assim em texto:
   - `Event Construct` → `Bind Event to On Remove Friend Requested` (Target = self) → `OnRemoveFriendRequested_Event` (FriendID, FriendName).
   - `OnRemoveFriendRequested_Event` → `Create Widget (WBP_FriendRemoveConfirm)` → `Show Confirm(FriendID, FriendName)` → `Add to Viewport`.
6. **Compile** e **Save** o Blueprint.

Resumo: o C++ já trata o clique em **BTN_RemoveFriend** e dispara **OnRemoveFriendRequested**. No Blueprint você só precisa **ligar** esse evento ao Custom Event e, no Custom Event, **criar** o WBP_FriendRemoveConfirm, **Show Confirm** e **Add to Viewport**.

### D. Nota: Assign e Bind dão no mesmo (Assign já cria o evento). Sobre “Bind Event to …”, não “Assign Delegate” _(Assign e Bind dão no mesmo)_

Se no Event Graph você usou um nó **“Assign Delegate”** (arrastando um delegate genérico e escolhendo On Remove Friend Requested), o clique pode não disparar o Custom Event. Para delegate **BlueprintAssignable** (multicast), o correto é o nó que **adiciona** o listener.

**O que fazer:**

1. **Remova** o nó **“Assign Delegate”** (e a ligação do Event Construct para ele).
2. **Crie o bind do jeito certo:**
   - Arraste um fio do **exec** do **Event Construct**.
   - No menu de busca digite: **Bind Event to On Remove Friend Requested** (ou só **On Remove Friend Requested**).
   - Escolha a opção que aparece como **“Bind Event to On Remove Friend Requested”** (não “Assign Delegate”).
   - Esse nó deve ter: **Target** (object) e **Event** (delegate).
   - **Target:** puxe do **self** (ou use o nó “Get a reference to self” e ligue ao Target).
   - **Event:** ligue ao **Custom Event** **OnRemoveFriendRequested_Event** (com parâmetros FriendID e FriendName).
3. Mantenha o resto igual: dentro do Custom Event, Create Widget (WBP_FriendRemoveConfirm) → Show Confirm → Add to Viewport.
4. **Compile** e **Save**.

**Assign** e **Bind** dão no mesmo; a única diferença é que Assign já cria o evento. “Bind Event to On Remove Friend Requested” é o nó específico. Pode usar qualquer um dos dois. “Assign Delegate” _(Assign e Bind dão no mesmo)_ às vezes Assign e Bind dão no mesmo; use o que preferir.

### E. Conferir no jogo (Output Log)

Depois de **recompilar** o projeto C++ (para ter os logs abaixo):

1. Abra **Window → Developer Tools → Output Log** (ou **Output Log** no editor).
2. Entre no jogo, abra a Friend List, clique num amigo (abre o menu de opções) e clique em **Remove Friend**.
3. Olhe o log:
   - **Ao abrir o menu de opções** deve aparecer:  
     `[UmbraFriendOptionsWidget] BTN_RemoveFriend ligado ao clique (NativeConstruct)`  
     Se aparecer **“BTN_RemoveFriend é null”**, o botão no Designer não tem o nome exato **BTN_RemoveFriend**.
   - **Ao clicar em Remove Friend** deve aparecer:  
     `[UmbraFriendOptionsWidget] Remover amigo solicitado: NomeDoAmigo (ID: 123) - Broadcast OnRemoveFriendRequested (...)`  
     - Se **não** aparecer: o clique não está chegando no C++ (botão null ou Parent Class errada).
     - Se **aparecer** e o confirm **não** abrir: o Blueprint não está recebendo o delegate → refaça o bind com **“Bind Event to On Remove Friend Requested”** (passo D).

---

## Passos concretos 2: Onde estão report_online e report_offline

Não é preciso **adicionar** nada no Editor. As chamadas já estão no código C++; você só precisa **compilar** o projeto.

### Onde está no código

| O quê | Arquivo | Onde |
|-------|--------|------|
| **ReportSessionOnline()** (marca jogador online) | `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` | Função `ReportSessionOnline()` (faz POST para `/api/social/report_online.php`). |
| **ReportSessionOffline()** (marca jogador offline) | Mesmo arquivo | Função `ReportSessionOffline()` (faz POST para `/api/social/report_offline.php`). |
| **Chamada ao conectar ao Zone** | `UmbraEternumUE/Source/UmbraEternumUE/Network/NetMovementClient.cpp` | Dentro de **HandleWebSocketConnected()**, após `LoadPartyState(0)`: linha com `MyGameInstance->ReportSessionOnline();`. |
| **Chamada ao desconectar do Zone** | Mesmo arquivo | Dentro de **HandleWebSocketClosed()**, após `LeavePartyOnDisconnect()`: linha com `MyGameInstance->ReportSessionOffline();`. |

### O que você precisa fazer

1. **Compilar o projeto** (Visual Studio ou “Build” no Unreal) para que essas alterações entrem no jogo.
2. **Não** é necessário criar nós no Blueprint, nem chamar report_online/report_offline manualmente: ao **entrar na zone** (WebSocket conecta) o cliente chama **ReportSessionOnline**; ao **sair/desconectar** chama **ReportSessionOffline**.

Se quiser conferir no código:

- Abra **NetMovementClient.cpp** e pesquise por **ReportSessionOnline** ou **HandleWebSocketConnected** — verá a chamada ao conectar.
- Pesquise por **ReportSessionOffline** ou **HandleWebSocketClosed** — verá a chamada ao desconectar.

---

## Parte 6: WBP_FriendList - Remover Botão Remove Friend

### 6.1 Remover do Designer
- Remover **BTN_RemoveFriend** do **HorizontalBox_788** (parte inferior do widget)
- O botão agora só existe em **WBP_FriendOptions**

---

## Resumo de Verificação

| Item | Status |
|------|--------|
| C++ UmbraFriendSlotWidget criado | ✅ |
| C++ UmbraFriendOptionsWidget criado | ✅ |
| C++ UmbraFriendRemoveConfirmWidget criado | ✅ |
| Tabela player_sessions + report_online/offline | ✅ |
| API get_friend_list retorna is_online (via player_sessions) | ✅ |
| C++ GetFriendIsOnline implementado | ✅ |
| WBP_FriendSlot: círculo Image_OnlineStatus | ⬜ |
| WBP_FriendSlot: Button_Slot para clique | ⬜ |
| WBP_FriendList: usar UmbraFriendSlotWidget | ⬜ |
| WBP_FriendOptions criado e vinculado | ⬜ |
| WBP_FriendRemoveConfirm criado e vinculado | ⬜ |
| BTN_RemoveFriend removido de WBP_FriendList | ⬜ |

---

## Notas

- **Lista atualiza após remover amigo:** Em **OnRemoveFriendComplete** o C++ chama **LoadFriendList()**; o delegate **OnFriendListLoaded** dispara e o WBP_FriendList deve reconstruir a lista (no evento que escuta OnFriendListLoaded: limpar o VBox e recriar os slots com **GetFriendIDAt**, **GetFriendName**, **GetFriendIsOnline**). Assim o amigo removido some da UI sem fechar o painel.
- **Status Online:** Implementado com tabela `player_sessions` e APIs `report_online` / `report_offline`. O cliente C++ chama **ReportSessionOnline()** em `NetMovementClient::HandleWebSocketConnected()` e **a cada 90 segundos** (heartbeat) enquanto o WebSocket estiver conectado, para que `last_seen` não saia da janela de 2 min; em **HandleWebSocketClosed()** o timer de heartbeat é cancelado e **ReportSessionOffline()** é chamado. O token é enviado no **body** do POST (além do header) para garantir que o PHP aceite. **Obrigatório:** rodar o script `www/umbra_api/scripts/create_player_sessions_table.sql` no MySQL; sem a tabela, todos aparecem offline. Ao **abrir** a Friend List, chamar **LoadFriendList()** (ex.: no Event Construct ou quando o painel fica visível) para obter dados frescos com `is_online`. No Output Log, ao carregar a lista, aparecerá `Friend Nome (ID X) is_online=true/false`; se sempre false, conferir se a tabela existe e se `report_online` está sendo chamado (log `ReportSessionOnline enviado` ao conectar ao Zone).
- **Refresh em tempo real (online/offline):** Enquanto a Friend List estiver aberta, chamar **Start Friend List Auto Refresh** (ex.: 30 segundos) no **Event Construct** ou ao exibir o painel; ao **fechar** o painel, chamar **Stop Friend List Auto Refresh**. Assim a lista é recarregada periodicamente e o círculo verde/vermelho atualiza quando um amigo entra ou sai do jogo.
- **Private Message:** Sistema de Whisper ainda não implementado — botão pode ficar desabilitado ou mostrar "Em breve".
- **Padrão:** Segue o mesmo padrão de **WBP_PartyPanel** → **WBP_PartyMemberSlot** → clique abre menu de contexto.

### Círculo sempre vermelho (todos offline) – tabela existe mas está vazia

Se `player_sessions` existe mas não tem linhas (SELECT * FROM player_sessions vazio), o **report_online** não está inserindo. Conferir:

1. **Cliente (Output Log), ao conectar na zone:**  
   - Deve aparecer `[UmbraGameInstance] ReportSessionOnline POST enviado` e depois **ou** `ReportSessionOnline OK: Online` **ou** `ReportSessionOnline falhou: ...` **ou** `ReportSessionOnline request fail (rede/URL/timeout)`.  
   - Se aparecer **falhou** ou **request fail**: URL da API errada, token inválido, ou rede (CORS/timeout).  
   - Se aparecer **OK** e a tabela continua vazia: conferir no PHP se está usando o mesmo banco/schema (ex.: `umbra_eternum`).

2. **Servidor (PHP error_log), ao conectar um jogador na zone:**  
   - Deve aparecer `[report_online] Request recebido. Body tem token: sim` e depois `[report_online] OK player_id=X inserido/atualizado em player_sessions`.  
   - Se aparecer `Body tem token: nao` ou `JWT inválido`: o cliente não está enviando o token no body ou o JWT está expirado/inválido.  
   - Se não aparecer nenhuma linha `[report_online]`: o POST não está chegando no PHP (URL, virtual host ou proxy).

3. **Abrir a Friend List** chamando **LoadFriendList()** ao abrir o painel; no log do cliente deve aparecer `Friend Nome (ID X) is_online=true/false`. Se continuar tudo `false`, a tabela ainda está vazia ou a consulta em `get_friend_list` está usando outro schema.
