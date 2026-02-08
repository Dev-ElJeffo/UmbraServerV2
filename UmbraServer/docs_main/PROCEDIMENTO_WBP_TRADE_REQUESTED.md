# Procedimento: WBP_TradeRequested - Notificação de Solicitação de Troca

Este documento descreve o procedimento completo para criar e integrar o widget **WBP_TradeRequested**, que exibe a notificação quando um jogador recebe uma solicitação de troca de outro jogador.

## Objetivo

- Exibir na tela do jogador que **recebe** a solicitação de trade
- Mostrar: **"Trade requested from {0}"** onde {0} = nome do jogador
- Botões: **Accept** e **Decline**
- **Decline**: fecha o widget e remove a solicitação
- **Accept**: aceita a troca via API e abre o widget de troca (WBP_Trade)

## Fluxo de Dados (C++)

| Evento | Delegate | Parâmetros |
|--------|----------|-------------|
| Solicitação recebida | `OnTradeRequestReceived` | `FromPlayerID` (int32), `FromPlayerName` (FString) |

| Função GameInstance | Descrição |
|---------------------|-----------|
| `AcceptTradeRequest(int32 FromPlayerID)` | Aceita a troca (chama API) |
| `DeclineTradeRequest(int32 FromPlayerID)` | Recusa e remove da lista |

| Após Accept | Delegate | Parâmetros |
|-------------|----------|-------------|
| Troca iniciada | `OnTradeStarted` | `TradeSessionID` (int32) |

## Quando a Notificação é Disparada

**Trade é 100% tempo real via WebSocket.** Não há "pending invites" ao logar — o outro jogador pode estar offline ou longe.

1. Jogador A clica em Trade no menu de contexto do jogador B
2. API HTTP `send_trade_request.php` cria o registro
3. Cliente de A envia msgType 20 (TradeRequest) via WebSocket para o Zone Server
4. Zone Server repassa msgType 21 (TradeRequestReceived) para o cliente de B
5. `NetMovementClient` → `ProcessSocialWebSocketMessage` → `OnTradeRequestReceived.Broadcast`
6. HUD do jogador B mostra o widget WBP_TradeRequested

**Condição:** Ambos os jogadores devem estar online na **mesma zone** (WebSocket conectado).

---

## Passo 1: Criar o Widget WBP_TradeRequested

### 1.1 Criar o Blueprint

1. No **Content Browser**, navegue até `Content/Widgets/UI/PlayerInteraction/`
2. Botão direito → **User Interface** → **Widget Blueprint**
3. Nome: **WBP_TradeRequested**
4. Abra o widget

### 1.2 Layout do Widget

| Elemento | Tipo | Propriedades |
|----------|------|--------------|
| **Root** | `Canvas Panel` | — |
| **Container** | `Vertical Box` | Centralizado ou ancorado (ex.: centro-tela) |
| **TX_Message** | `Text Block` | Texto: "Trade requested from {0}" – use `Format Text` com variável |
| **BTN_Accept** | `Button` | Texto: "Accept" |
| **BTN_Decline** | `Button` | Texto: "Decline" |

**Hierarquia sugerida:**

```
Canvas Panel (Root)
└── Vertical Box (ou Border para fundo)
    ├── Text Block (TX_Message) - "Trade requested from {0}"
    └── Horizontal Box
        ├── Button (BTN_Accept) - "Accept"
        └── Button (BTN_Decline) - "Decline"
```

### 1.3 Variáveis do Widget

| Nome | Tipo | Editable | Descrição |
|------|------|----------|-----------|
| `FromPlayerID` | `Integer` | Sim | ID do jogador que enviou a solicitação |
| `FromPlayerName` | `String` | Sim | Nome do jogador |

---

## Passo 2: Função de Exibição

### 2.1 Criar Função `ShowTradeRequest`

No **WBP_TradeRequested**, crie uma função:

- **Nome:** `ShowTradeRequest`
- **Parâmetros:** `FromPlayerID` (int32), `FromPlayerName` (FString)
- **Lógica:**
  1. Atribuir `FromPlayerID` e `FromPlayerName` às variáveis do widget
  2. Atualizar `TX_Message` com `Format Text`: **"Trade requested from {0}"** usando `FromPlayerName` em {0}
  3. Definir `Visibility` = `Visible` (se estiver oculto por padrão)

**Exemplo no Blueprint:**
```
[ShowTradeRequest(FromPlayerID, FromPlayerName)]
    │
    ├──► Set FromPlayerID = FromPlayerID
    ├──► Set FromPlayerName = FromPlayerName
    ├──► Format Text: Format = "Trade requested from {0}", {0} = FromPlayerName
    │         └──► Set Text (TX_Message) = Result
    └──► Set Visibility = Visible
```

---

## Passo 3: Eventos dos Botões

### 3.1 BTN_Decline – OnClicked

O Decline fecha o widget e remove a solicitação da lista.

```
[BTN_Decline OnClicked]
    │
    ├──► Get Game Instance → Cast to UmbraGameInstance
    │         │
    │         └──► Decline Trade Request
    │                   From Player ID = [variável FromPlayerID do widget]
    │
    └──► Remove From Parent
```

### 3.2 BTN_Accept – OnClicked

O Accept chama a API e, ao sucesso, dispara `OnTradeStarted`. O widget fecha aqui; o WBP_Trade será aberto via bind de `OnTradeStarted`.

```
[BTN_Accept OnClicked]
    │
    ├──► Get Game Instance → Cast to UmbraGameInstance
    │         │
    │         └──► Accept Trade Request
    │                   From Player ID = [variável FromPlayerID do widget]
    │
    └──► Remove From Parent
```

---

## Passo 4: Integração no HUD (WBP_PlayerHUD)

O **WBP_PlayerHUD** (ou o widget raiz que fica sempre visível) deve:

1. Fazer bind de `OnTradeRequestReceived` do GameInstance
2. Ao receber o evento, criar o WBP_TradeRequested, adicionar ao viewport e chamar `ShowTradeRequest`

### ⚠️ ERRADO: Onde NÃO colocar Create Widget

O nó **"Bind Event to OnTradeRequestReceived"** tem um pin **"then"** que executa **imediatamente após o bind ser registrado** — **não** quando o evento dispara. Se você conectar **Create Widget** ou **AddToViewport** ao "then" do Bind Event:

- ❌ O widget aparece **assim que o HUD carrega** (se colocado após o último bind)
- ❌ Ou aparecerá no momento errado (ex.: ao abrir menu de contexto, se a cadeia de execução passar por ali)

**Nunca** coloque Create Widget na cadeia de execução do Event Construct/BeginPlay. O "then" do Bind Event deve ir apenas para o **próximo bind** (ex.: Bind OnTradeStarted), não para Create Widget.

### ✅ CORRETO: Onde colocar Create Widget

O **Create Widget** e **AddToViewport** devem estar **apenas** dentro do **Custom Event OnTradeRequestReceived** — ou seja, no fluxo que dispara **quando** o evento `OnTradeRequestReceived` é recebido.

### 4.1 Event Construct (ou BeginPlay) do HUD

```
[Event Construct]
    │
    └──► Get Game Instance → Cast to UmbraGameInstance [guardar em MyGameInstance]
              │
              └──► Bind Event to On Trade Request Received
                        Target = MyGameInstance
                        Event = [Custom Event: OnTradeRequestReceived]  ← apenas a REFERÊNCIA ao Custom Event
                        │
                        └──► (then) → próximo bind (ex.: Bind OnTradeStarted)
                                      ← NÃO conecte Create Widget aqui!
```

**Trade é tempo real via WebSocket.** Não chame Load Pending Invites para trade — o evento vem quando o jogador recebe a mensagem WebSocket (msgType 21).

### 4.2 Custom Event: OnTradeRequestReceived

Todo o fluxo de criar e exibir o widget deve estar **apenas** aqui:

```
[Custom Event: OnTradeRequestReceived(FromPlayerID, FromPlayerName)]
    │
    ├──► Create Widget
    │         Class = WBP_TradeRequested
    │         Owning Player = Get Owning Player  ← usar Owning Player do HUD (correto em PIE com 2 clientes)
    │         [guarde o Return Value]
    │
    ├──► Show Trade Request  (função do widget criado)
    │         Target = [widget criado acima]
    │         From Player ID = FromPlayerID (do Event)
    │         From Player Name = FromPlayerName (do Event)
    │
    └──► Add to Viewport
              Target = [widget criado]
```

**Ordem:** Create Widget → ShowTradeRequest → AddToViewport. O `ShowTradeRequest` deve ser chamado **antes** de AddToViewport para que o texto "Trade with X?" seja preenchido corretamente.

### 4.3 Resumo visual da conexão correta

```
Event Construct
    │
    └──► ... → Bind OnTradeRequestReceived (Event = OnTradeRequestReceived Custom Event)
                   │
                   └──► then → Bind OnTradeStarted (ou fim da cadeia)
                              
                              ← Create Widget NÃO vai aqui!

───────────────────────────────────────────────────────────────

Custom Event OnTradeRequestReceived  ← dispara quando trade é recebido
    │
    └──► then → Create Widget → ShowTradeRequest → AddToViewport
```

**Nota:** Se houver múltiplas solicitações simultâneas, cada uma criará um novo widget. Para uma fila, considere armazenar em TArray e exibir uma por vez.

---

## Passo 5: WBP_Trade (Janela de Troca) – Placeholder

Ao aceitar, `OnTradeStarted` é disparado com `TradeSessionID`. Para abrir o WBP_Trade:

### 5.1 Bind no HUD (obrigatório para abrir WBP_Trade ao aceitar)

O **bind de On Trade Started** deve estar no **WBP_PlayerHUD** (ou widget raiz), no **Event Construct** ou **Event BeginPlay**, junto com os outros binds:

```
[Event Construct] ou [Event BeginPlay]
    │
    └──► Get Game Instance → Cast to UmbraGameInstance
              │
              └──► Bind Event to On Trade Started
                        Target = MyGameInstance (UmbraGameInstance)
                        Event = [Custom Event: OnTradeStarted_Event]
                        │
                        └──► (then) → [Custom Event: OnTradeStarted_Event]
                                  Trade Session ID = [parâmetro do Event]
                                  │
                                  └──► Create Widget (WBP_Trade)
                                            Owning Player = Get Player Controller (0)
                                            │
                                  └──► Add to Viewport
```

**Crítico:** O Target do Bind deve ser o **UmbraGameInstance** (o mesmo usado para OnTradeRequestReceived). O delegate `OnTradeStarted` é disparado no GameInstance após sucesso da API `accept_trade_request.php`.

Por enquanto, o **WBP_Trade** pode ser um widget simples (ex.: "Trade session started" com botão fechar) até a implementação completa da janela de troca.

---

## Passo 6: Trade é Tempo Real (sem Pending Invites)

**Trade não usa LoadPendingInvites.** A notificação só chega via WebSocket quando o jogador está online na mesma zone.

- O HUD apenas precisa fazer o **bind** de `OnTradeRequestReceived` no Event Construct (Passo 4.1)
- Nada mais é necessário — o evento dispara quando a mensagem WebSocket (msgType 21) chega

---

## Resumo de Verificação

| Item | Status |
|------|--------|
| WBP_TradeRequested criado com TX_Message, BTN_Accept, BTN_Decline | ⬜ |
| ShowTradeRequest(FromPlayerID, FromPlayerName) implementada | ⬜ |
| BTN_Decline → DeclineTradeRequest + Remove From Parent | ⬜ |
| BTN_Accept → AcceptTradeRequest + Remove From Parent | ⬜ |
| HUD faz bind de OnTradeRequestReceived (trade é 100% tempo real via WebSocket) | ⬜ |
| OnTradeRequestReceived cria/exibe WBP_TradeRequested | ⬜ |
| (Opcional) OnTradeStarted → abrir WBP_Trade | ⬜ |

---

## Troubleshooting

**Widget aparece ao abrir menu de contexto (sem clicar em Trade):**
- O **Create Widget** está conectado ao pin **"then"** do nó **Bind Event to OnTradeRequestReceived**. Esse "then" executa assim que o bind é feito, não quando o evento dispara.
- **Correção:** Desconecte Create Widget do "then" do Bind Event. Conecte Create Widget **apenas** ao pin "then" do **Custom Event OnTradeRequestReceived**.

**Widget aparece assim que o HUD carrega:**
- Mesmo problema: Create Widget está na cadeia de Event Construct (por exemplo, após o último bind).
- **Correção:** Mova Create Widget + ShowTradeRequest + AddToViewport para **dentro** do Custom Event OnTradeRequestReceived. O "then" do Bind Event deve ir apenas ao próximo bind ou ficar desconectado.

**Widget não aparece ao receber solicitação:**
- Verifique se o HUD faz bind de `OnTradeRequestReceived`
- **Ambos os jogadores** devem estar online na mesma zone (WebSocket conectado)
- No Output Log (receptor): `[UmbraGameInstance] 🔄 Solicitação de troca recebida de PlayerID X` ou `[NetMovementClient] 📨 Mensagem social recebida (msgType=21)`
- No Output Log (remetente): `✅ Trade request enviado via WebSocket para PlayerID X` — se aparecer `⚠️ WebSocket não disponível`, o cliente não está conectado à zone

**API retorna "Invalid parameter number" (SQLSTATE[HY093]):**
- Corrigido em `send_trade_request.php`: placeholders PDO duplicados na query `check_trade`. Se o erro persistir, verifique se as tabelas `trade_requests` e `trade_sessions` existem executando `www/umbra_api/scripts/create_social_tables.sql`.

**API e WebSocket OK mas widget não aparece no receptor:**
- O `NetMovementClient` chama `ProcessSocialWebSocketMessage` automaticamente para msgType 21. Recompile o projeto C++ após alterações.
- **Servidor Zone:** Se o receptor não estiver na mesma zone, o log do servidor mostra `Player X not found online, cannot send message`. Ambos precisam estar conectados ao mesmo Zone Server.

**Widget aparece no cliente que enviou o trade (não no que recebe):**
- O `OnTradeRequestReceived` só dispara no cliente **destino** (o que recebe a solicitação). Se aparecer no sender, pode haver outro bind ou fluxo incorreto. Verifique se não há múltiplos binds ou chamadas diretas.

**Accept/Decline não funcionam:**
- Verifique se `FromPlayerID` está sendo passado corretamente para o widget
- Verifique se o GameInstance é `UmbraGameInstance` (Cast)

**Accept clicado mas WBP_Trade não abre:**
- O bind de **On Trade Started** deve estar no **HUD** (WBP_PlayerHUD), no Event Construct ou BeginPlay
- O **Target** do Bind deve ser o **UmbraGameInstance** (Get Game Instance → Cast to UmbraGameInstance)
- No Output Log deve aparecer `✅ Troca iniciada - Sessão: X, Parceiro: Y` após clicar Accept — se não aparecer, a API pode ter falhado (verifique os logs de erro)

**Múltiplas solicitações:**
- Se houver vários trades pendentes, o `OnTradeRequestReceived` dispara uma vez por solicitação. Cada uma cria um widget. Considere uma fila ou lista se quiser exibir várias ao mesmo tempo.
