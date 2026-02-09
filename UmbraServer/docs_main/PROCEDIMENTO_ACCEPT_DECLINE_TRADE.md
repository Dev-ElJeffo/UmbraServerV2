# Procedimento: Accept Trade e Decline Trade (WBP_Trade)

Implementação completa dos botões **Accept Trade** e **Decline Trade** no WBP_Trade.

---

## Resumo do fluxo

| Botão | Ação | API |
|-------|------|-----|
| **Accept Trade** | Marca o jogador como pronto | `set_trade_ready.php` (ready=true) |
| **Decline Trade** | Cancela a sessão de troca | `cancel_trade.php` |

**Quando ambos clicam Accept:** a API executa a troca automaticamente e retorna `trade_completed: true`. O GameInstance dispara `OnTradeCompleted`.

---

## Funções C++ já disponíveis (UmbraGameInstance)

| Função | Descrição |
|--------|-----------|
| `SetTradeReady(int32 TradeSessionID, bool bReady)` | Marca pronto (true) ou não pronto (false). Chama set_trade_ready.php |
| `CancelTrade()` | Cancela a sessão ativa. Chama cancel_trade.php e dispara OnTradeCancelled |
| `GetCurrentTradeSessionID()` | Retorna o ID da sessão atual (0 se não está em troca) |
| `IsInTrade()` | Retorna true se está em uma sessão de troca |

**Delegates:**
- `OnTradeCompleted` – disparado quando a troca é concluída (ambos clicaram Accept)
- `OnTradeCancelled` – disparado quando a troca é cancelada

---

## Passo 1: BTN_Accept (Accept Trade)

### 1.1 Identificar o botão

No **Designer** do WBP_Trade, localize o botão **Accept Trade** (ex.: BTN_Accept).

### 1.2 Criar o evento OnClicked

1. Selecione o botão **Accept Trade**.
2. No **Details**, em **Events**, clique em **On Clicked** (ou arraste o pin para o EventGraph).
3. Isso criará o evento **On Clicked (BTN_Accept)**.

### 1.3 Conectar a lógica

No **EventGraph**, construa a seguinte cadeia:

```
[On Clicked (BTN_Accept)]
    │
    ├─► Get Game Instance
    │       │
    │       └─► Cast to UmbraGameInstance  (Target = Return Value)
    │               │
    │               ├─► Get Current Trade Session ID  (Target = As Umbra Game Instance)
    │               │       │
    │               │       └─► Return Value (int32) = TradeSessionID
    │               │
    │               └─► Branch  (Condition: TradeSessionID > 0)
    │                       │
    │                       ├─► TRUE: Set Trade Ready
    │                       │       • Target = As Umbra Game Instance
    │                       │       • Trade Session ID = TradeSessionID (do Get)
    │                       │       • Ready = true
    │                       │
    │                       └─► FALSE: (opcional) Print String "Nenhuma troca ativa"
```

### 1.4 Nós necessários

| Nó | Categoria | Parâmetros |
|----|-----------|------------|
| Get Game Instance | Game | — |
| Cast to UmbraGameInstance | UmbraGameInstance | Target = Get Game Instance (Return Value) |
| Get Current Trade Session ID | UmbraGameInstance | Target = Cast (As Umbra Game Instance) |
| Branch | Flow Control | Condition = Get Current Trade Session ID (Return Value) > 0 |
| Set Trade Ready | UmbraGameInstance | Target = Cast, Trade Session ID = valor do Get, Ready = true |

### 1.5 Resultado esperado

- Ao clicar **Accept Trade**, o jogador fica marcado como "pronto".
- Quando **ambos** clicam Accept, a API executa a troca e o GameInstance dispara `OnTradeCompleted`.
- O widget será fechado pelo bind de OnTradeCompleted (Passo 3).

---

## Passo 2: BTN_Decline (Decline Trade)

### 2.1 Identificar o botão

No **Designer** do WBP_Trade, localize o botão **Decline Trade** (ex.: BTN_Decline).

### 2.2 Criar o evento OnClicked

1. Selecione o botão **Decline Trade**.
2. Em **Events**, clique em **On Clicked**.
3. Isso criará o evento **On Clicked (BTN_Decline)**.

### 2.3 Conectar a lógica

No **EventGraph**, construa a seguinte cadeia:

```
[On Clicked (BTN_Decline)]
    │
    ├─► Get Game Instance
    │       │
    │       └─► Cast to UmbraGameInstance
    │               │
    │               └─► Cancel Trade  (Target = As Umbra Game Instance)
    │                       │
    │                       └─► (then) → Clear Timer by Handle
    │                                       • Handle = PollingTimerHandle (variável do WBP_Trade)
    │                                       │
    │                                       └─► (then) → Remove from Parent  (Target = self)
```

### 2.4 Nós necessários

| Nó | Categoria | Parâmetros |
|----|-----------|------------|
| Get Game Instance | Game | — |
| Cast to UmbraGameInstance | UmbraGameInstance | Target = Get Game Instance (Return Value) |
| Cancel Trade | UmbraGameInstance | Target = Cast (As Umbra Game Instance) |
| Clear Timer by Handle | Engine | Handle = **PollingTimerHandle** (variável) |
| Remove from Parent | User Widget | Target = **self** |

### 2.5 Importante

- **PollingTimerHandle** deve ser a variável onde você guarda o retorno do `Set Timer by Event` (polling de LoadTradeState).
- Se o WBP_Trade não tiver essa variável, crie: `PollingTimerHandle` (tipo **Timer Handle**).

### 2.6 Resultado esperado

- Ao clicar **Decline Trade**, a API cancela a sessão e o widget fecha imediatamente.
- O parceiro receberá status `cancelled` no próximo polling e o widget dele também fechará (via UpdateTradeUIFromState ou OnTradeCancelled).

---

## Passo 3: Bind OnTradeCompleted e OnTradeCancelled

Para fechar o widget quando a troca é concluída ou cancelada pelo outro jogador (ou pela API).

### 3.1 No Event Construct do WBP_Trade

Após **CreateTradeSlots** e **Bind Event to On Trade State Loaded**, adicione:

1. **Get Game Instance** → **Cast to UmbraGameInstance**
2. **Bind Event to On Trade Completed** (Target = Cast)
   - Event = **Custom Event** ou função que faz: Clear Timer by Handle (PollingTimerHandle) → Remove from Parent
3. **Bind Event to On Trade Cancelled** (Target = mesmo Cast)
   - Event = **Custom Event** ou função que faz: Clear Timer by Handle → Remove from Parent

### 3.2 Criar Custom Events (opcional)

Crie dois Custom Events no WBP_Trade:

**OnTradeCompleted_Close:**
```
[Event] OnTradeCompleted_Close
    │
    ├─► Clear Timer by Handle  (Handle = PollingTimerHandle)
    │       │
    │       └─► (then) → Remove from Parent  (Target = self)
```

**OnTradeCancelled_Close:**
```
[Event] OnTradeCancelled_Close
    │
    ├─► Clear Timer by Handle  (Handle = PollingTimerHandle)
    │       │
    │       └─► (then) → Remove from Parent  (Target = self)
```

### 3.3 Conectar os binds

```
Event Construct
    │
    ├─► CreateTradeSlots
    ├─► ... (resto do setup)
    │
    └─► Get Game Instance → Cast to UmbraGameInstance
            │
            ├─► Bind Event to On Trade State Loaded  → (Event = UpdateTradeUI)
            ├─► Bind Event to On Trade Completed     → (Event = OnTradeCompleted_Close)
            └─► Bind Event to On Trade Cancelled    → (Event = OnTradeCancelled_Close)
```

### 3.4 Resultado esperado

- Quando **ambos** clicam Accept → API executa → OnTradeCompleted dispara → widget fecha.
- Quando **qualquer um** cancela → OnTradeCancelled dispara no outro → widget fecha.
- Quando o polling recebe status `completed` ou `cancelled` → UpdateTradeUIFromState já remove o widget (se herdar de UmbraTradeWidget).

---

## Passo 4: Botão X (fechar)

O botão **X** no canto do widget deve ter o mesmo comportamento do **Decline Trade**:

```
[On Clicked (BTN_Close ou X)]
    │
    └─► (mesma cadeia do BTN_Decline)
        Cancel Trade → Clear Timer → Remove from Parent
```

Ou reuse: crie uma função **CloseTradeWidget** que faz Clear Timer + Remove from Parent, e chame de ambos (Decline e X) após Cancel Trade.

---

## Checklist final

| Item | Verificação |
|------|-------------|
| BTN_Accept → SetTradeReady(GetCurrentTradeSessionID, true) | ⬜ |
| BTN_Decline → CancelTrade → Clear Timer → Remove from Parent | ⬜ |
| BTN_Close (X) → CancelTrade → Clear Timer → Remove from Parent | ⬜ |
| Bind OnTradeCompleted → Clear Timer → Remove from Parent | ⬜ |
| Bind OnTradeCancelled → Clear Timer → Remove from Parent | ⬜ |
| Variável PollingTimerHandle existe e está preenchida pelo Set Timer | ⬜ |

---

## Diagrama de fluxo

```
Jogador A                          Jogador B                    API
    │                                  │                          │
    │  Clica Accept Trade              │                          │
    │  → SetTradeReady(session, true)  │                          │
    │  ──────────────────────────────────────────────────────────►│
    │                                  │                    player1_ready=1
    │                                  │  Clica Accept Trade      │
    │                                  │  → SetTradeReady(...)    │
    │                                  │  ──────────────────────►│
    │                                  │                    player2_ready=1
    │                                  │                    executeTrade()
    │                                  │                    status=completed
    │  ◄──────────────────────────────────────────────────────────│
    │  OnTradeCompleted.Broadcast()    │  OnTradeCompleted         │
    │  → Clear Timer, Remove from Parent   (via polling ou callback)
    │                                  │  → Clear Timer, Remove   │
```

---

## Tratamento de erros (opcional)

- **SetTradeReady** pode falhar (ex.: gold insuficiente). O GameInstance dispara `OnTradeActionFailed` com a mensagem.
- Para mostrar erro ao usuário: faça bind de **On Trade Action Failed** e exiba um **Print String** ou widget de erro.
- **CancelTrade** em caso de falha de rede: o `OnTradeActionFail` é chamado, mas o widget já terá fechado (Remove from Parent). Considere fechar apenas após sucesso, ou fechar de qualquer forma (UX).

---

## Referência rápida – nós do Blueprint

**Accept Trade:**
```
On Clicked → Get Game Instance → Cast to UmbraGameInstance
  → Get Current Trade Session ID
  → Branch (Return Value > 0)
    TRUE → Set Trade Ready (Trade Session ID = Return Value, Ready = true)
```

**Decline Trade:**
```
On Clicked → Get Game Instance → Cast to UmbraGameInstance
  → Cancel Trade
  → Clear Timer by Handle (PollingTimerHandle)
  → Remove from Parent (self)
```
