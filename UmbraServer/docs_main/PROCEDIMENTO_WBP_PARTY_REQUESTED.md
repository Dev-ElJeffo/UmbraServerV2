# Procedimento: WBP_PartyRequested - Notificação de Convite de Grupo

Este documento descreve o procedimento completo para criar e integrar o widget **WBP_PartyRequested**, que exibe a notificação quando um jogador recebe um convite de grupo de outro jogador.

## Objetivo

- Exibir na tela do jogador que **recebe** o convite de grupo
- Mostrar: **"{0} convidou você para o grupo"** onde {0} = nome do jogador
- Botões: **Accept** e **Decline**
- **Decline**: fecha o widget e remove o convite da lista
- **Accept**: aceita o convite via API e entra no grupo (WBP_PartyPanel aparece via OnPartyStateLoaded)

## Fluxo de Dados (C++)

| Evento | Delegate | Parâmetros |
|--------|----------|------------|
| Convite recebido | `OnPartyInviteReceived` | `FromPlayerID` (int32), `FromPlayerName` (FString) |

| Função GameInstance | Descrição |
|---------------------|-----------|
| `AcceptPartyInvite(int32 FromPlayerID)` | Aceita o convite (chama API) |
| `DeclinePartyInvite(int32 FromPlayerID)` | Recusa e remove da lista |

| Após Accept | Delegate | Parâmetros |
|-------------|----------|------------|
| Estado do grupo carregado | `OnPartyStateLoaded` | `PartyState` (FUmbraPartyState) |

## Quando a Notificação é Disparada

**Duas fontes:** LoadPendingInvites (ao logar) e WebSocket (tempo real).

1. **LoadPendingInvites:** Ao logar, o GameInstance chama `LoadPendingInvitesOnLogin()`. Se houver `party_invites` pendentes, para cada um dispara `OnPartyInviteReceived`.
2. **WebSocket (tempo real):** Quando o jogador A envia convite (SendPartyInvite HTTP → sucesso → SendPartyInviteViaWebSocket), o servidor Zone recebe msgType 10 (PartyInvite) e repassa msgType 11 (PartyInviteReceived) para o jogador B. O `NetMovementClient` processa e dispara `OnPartyInviteReceived`.

**Condição WebSocket:** Ambos os jogadores devem estar online na **mesma zone** (WebSocket conectado). Caso contrário, o receptor verá o convite ao **logar** ou ao **carregar convites pendentes**.

---

## Passo 1: Criar o Widget WBP_PartyRequested

### 1.1 Criar o Blueprint

1. No **Content Browser**, navegue até `Content/Widgets/UI/PlayerInteraction/Party/`
2. Botão direito → **User Interface** → **Widget Blueprint**
3. Nome: **WBP_PartyRequested**
4. Abra o widget

### 1.2 Layout do Widget

| Elemento | Tipo | Propriedades |
|----------|------|--------------|
| **Root** | `Canvas Panel` | — |
| **Container** | `Vertical Box` | Centralizado ou ancorado (ex.: centro-tela) |
| **TX_Message** | `Text Block` | Texto: "{0} convidou você para o grupo" – use `Format Text` com variável |
| **BTN_Accept** | `Button` | Texto: "Accept" |
| **BTN_Decline** | `Button` | Texto: "Decline" |

**Hierarquia sugerida:**

```
Canvas Panel (Root)
└── Vertical Box (ou Border para fundo)
    ├── Text Block (TX_Message) - "{0} convidou você para o grupo"
    └── Horizontal Box
        ├── Button (BTN_Accept) - "Accept"
        └── Button (BTN_Decline) - "Decline"
```

### 1.3 Variáveis do Widget

| Nome | Tipo | Editable | Descrição |
|------|------|----------|-----------|
| `FromPlayerID` | `Integer` | Sim | ID do jogador que enviou o convite |
| `FromPlayerName` | `String` | Sim | Nome do jogador |

---

## Passo 2: Função de Exibição

### 2.1 Criar Função `ShowPartyRequest`

No **WBP_PartyRequested**, crie uma função:

- **Nome:** `ShowPartyRequest`
- **Parâmetros:** `FromPlayerID` (int32), `FromPlayerName` (FString)
- **Lógica:**
  1. Atribuir `FromPlayerID` e `FromPlayerName` às variáveis do widget
  2. Atualizar `TX_Message` com `Format Text`: **"{0} convidou você para o grupo"** usando `FromPlayerName` em {0}
  3. Definir `Visibility` = `Visible` (se estiver oculto por padrão)

**Passo a passo no Blueprint:**

1. **Event Graph** → Adicione **Function** (nome: `ShowPartyRequest`).
2. Adicione **Inputs**: `FromPlayerID` (Integer), `FromPlayerName` (String).
3. Dentro da função:
   - **Set FromPlayerID** = `FromPlayerID` (do input)
   - **Set FromPlayerName** = `FromPlayerName` (do input)
   - **Format Text**:
     - **Format** = `"{0} convidou você para o grupo"`
     - **{0}** = `FromPlayerName`
   - **Set Text** (Target = TX_Message, In Text = Result do Format Text)
   - **Set Visibility** (Target = Self, In Visibility = Visible)

---

## Passo 3: Eventos dos Botões

### 3.1 BTN_Decline – OnClicked

O Decline fecha o widget e remove o convite da lista.

**Passo a passo no Blueprint:**

1. Selecione **BTN_Decline** no Designer.
2. No **Details**, em **Events**, clique em **On Clicked** (ou arraste para o Event Graph).
3. No Event Graph, construa:

```
[BTN_Decline OnClicked]
    │
    ├─► Get Game Instance
    │       └─► Cast to UmbraGameInstance
    │               └─► Decline Party Invite
    │                       From Player ID = [variável FromPlayerID do widget]
    │
    └─► Remove From Parent
            Target = Self
```

**Nodos necessários:**

| Nodo | Onde encontrar | Entrada |
|------|----------------|---------|
| Get Game Instance | GameplayStatics | WorldContextObject (opcional) |
| Cast to UmbraGameInstance | UmbraGameInstance | Object = Get Game Instance (Return Value) |
| Decline Party Invite | UmbraGameInstance | Target = Cast (As Umbra Game Instance), From Player ID = Get FromPlayerID |
| Remove From Parent | Widget | Target = Self |

### 3.2 BTN_Accept – OnClicked

O Accept chama a API e, ao sucesso, dispara `OnPartyMemberJoined` e `LoadPartyState` → `OnPartyStateLoaded`. O widget fecha aqui; o WBP_PartyPanel será mostrado via bind de `OnPartyStateLoaded`.

**Passo a passo no Blueprint:**

1. Selecione **BTN_Accept** no Designer.
2. No **Details**, em **Events**, clique em **On Clicked**.
3. No Event Graph, construa:

```
[BTN_Accept OnClicked]
    │
    ├─► Get Game Instance
    │       └─► Cast to UmbraGameInstance
    │               └─► Accept Party Invite
    │                       From Player ID = [variável FromPlayerID do widget]
    │
    └─► Remove From Parent
            Target = Self
```

**Nodos necessários:**

| Nodo | Onde encontrar | Entrada |
|------|----------------|---------|
| Get Game Instance | GameplayStatics | — |
| Cast to UmbraGameInstance | UmbraGameInstance | Object = Get Game Instance (Return Value) |
| Accept Party Invite | UmbraGameInstance | Target = Cast (As Umbra Game Instance), From Player ID = Get FromPlayerID |
| Remove From Parent | Widget | Target = Self |

---

## Passo 4: Integração no HUD (WBP_PlayerHUD)

O **WBP_PlayerHUD** (ou o widget raiz que fica sempre visível) deve:

1. Fazer bind de `OnPartyInviteReceived` do GameInstance
2. Ao receber o evento, criar o WBP_PartyRequested, chamar `ShowPartyRequest` e adicionar ao viewport

### ⚠️ ERRADO: Onde NÃO colocar Create Widget

O nó **"Bind Event to On Party Invite Received"** tem um pin **"then"** que executa **imediatamente após o bind ser registrado** — **não** quando o evento dispara. Se você conectar **Create Widget** ou **AddToViewport** ao "then" do Bind Event:

- ❌ O widget aparece **assim que o HUD carrega**
- ❌ Ou aparecerá no momento errado

**Nunca** coloque Create Widget na cadeia de execução do Event Construct/BeginPlay ligada ao "then" do Bind Event.

### ✅ CORRETO: Onde colocar Create Widget

O **Create Widget** e **AddToViewport** devem estar **apenas** dentro do **Custom Event OnPartyInviteReceived** — ou seja, no fluxo que dispara **quando** o evento `OnPartyInviteReceived` é recebido.

### 4.1 Event Construct (ou BeginPlay) do HUD

```
[Event Construct]
    │
    └─► Get Game Instance → Cast to UmbraGameInstance [guardar em MyGameInstance]
              │
              └─► Bind Event to On Party Invite Received
                        Target = MyGameInstance
                        Event = [Custom Event: OnPartyInviteReceived]  ← apenas a REFERÊNCIA ao Custom Event
                        │
                        └─► (then) → próximo bind (ex.: Bind OnTradeRequestReceived)
                                      ← NÃO conecte Create Widget aqui!
```

### 4.2 Custom Event: OnPartyInviteReceived

Todo o fluxo de criar e exibir o widget deve estar **apenas** aqui:

```
[Custom Event: OnPartyInviteReceived(FromPlayerID, FromPlayerName)]
    │
    ├─► Create Widget
    │       Class = WBP_PartyRequested
    │       Owning Player = Get Owning Player
    │       [guarde o Return Value]
    │
    ├─► Show Party Request  (função do widget criado)
    │       Target = [widget criado acima]
    │       From Player ID = FromPlayerID (do Event)
    │       From Player Name = FromPlayerName (do Event)
    │
    └─► Add to Viewport
              Target = [widget criado]
```

**Ordem:** Create Widget → ShowPartyRequest → AddToViewport.

### 4.3 Resumo visual da conexão correta

```
Event Construct
    │
    └─► ... → Bind OnPartyInviteReceived (Event = OnPartyInviteReceived Custom Event)
                   │
                   └─► then → próximo bind (ou fim da cadeia)
                              
                              ← Create Widget NÃO vai aqui!

───────────────────────────────────────────────────────────────

Custom Event OnPartyInviteReceived  ← dispara quando convite é recebido
    │
    └─► then → Create Widget → ShowPartyRequest → AddToViewport
```

**Nota:** Se houver múltiplos convites simultâneos, cada um criará um novo widget. Para uma fila, considere armazenar em TArray e exibir um por vez.

---

## Passo 5: WebSocket - Notificação em Tempo Real

Após o convite ser enviado com sucesso pela API HTTP, o cliente que enviou chama `SendPartyInviteViaWebSocket`. O servidor Zone repassa msgType 11 (PartyInviteReceived) para o jogador alvo.

- **Requisito:** Ambos os jogadores devem estar na **mesma zone** (conectados ao Zone Server via WebSocket).
- **Fallback:** Se o receptor não estiver online ou em outra zone, o convite entra em `party_invites` no banco. Ao logar, `LoadPendingInvitesOnLogin` carrega e dispara `OnPartyInviteReceived` para cada convite pendente.

---

## Resumo de Verificação

| Item | Status |
|------|--------|
| WBP_PartyRequested criado com TX_Message, BTN_Accept, BTN_Decline | ⬜ |
| ShowPartyRequest(FromPlayerID, FromPlayerName) implementada | ⬜ |
| BTN_Decline → DeclinePartyInvite + Remove From Parent | ⬜ |
| BTN_Accept → AcceptPartyInvite + Remove From Parent | ⬜ |
| HUD faz bind de OnPartyInviteReceived | ⬜ |
| OnPartyInviteReceived cria/exibe WBP_PartyRequested | ⬜ |
| OnSendPartyInviteComplete chama SendPartyInviteViaWebSocket (C++) | ⬜ |

---

## Troubleshooting

**Widget aparece ao carregar o HUD:**
- O **Create Widget** está conectado ao pin **"then"** do nó **Bind Event to OnPartyInviteReceived**. Esse "then" executa assim que o bind é feito, não quando o evento dispara.
- **Correção:** Desconecte Create Widget do "then" do Bind Event. Conecte Create Widget **apenas** ao pin "then" do **Custom Event OnPartyInviteReceived**.

**Widget não aparece ao receber convite (WebSocket):**
- Verifique se o HUD faz bind de `OnPartyInviteReceived`
- **Ambos os jogadores** devem estar online na mesma zone (WebSocket conectado)
- No Output Log (receptor): `[UmbraGameInstance] 📨 Convite de grupo recebido de PlayerID X` ou `[NetMovementClient] 📨 Mensagem social recebida (msgType=11)`
- No Output Log (remetente): `✅ Convite de grupo enviado via WebSocket para PlayerID X` — se aparecer `⚠️ WebSocket não disponível`, o cliente não está conectado à zone

**Widget não aparece ao logar (LoadPendingInvites):**
- O GameInstance chama `LoadPendingInvitesOnLogin` ao logar
- Verifique se `get_pending_invites.php` retorna `party_invites` corretamente
- No Output Log: `[UmbraGameInstance] Loaded X party invites, Y trade requests...`

**Accept/Decline não funcionam:**
- Verifique se `FromPlayerID` está sendo passado corretamente para o widget
- Verifique se o GameInstance é `UmbraGameInstance` (Cast)
