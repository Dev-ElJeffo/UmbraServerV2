# Procedimento: WBP_Trade - Janela de Troca Completa

Este documento descreve o procedimento para implementar a janela de troca **WBP_Trade** de forma completa, incluindo troca de itens (até 10 por jogador), troca de gold, Accept Trade e Decline Trade.

---

## Visão Geral da Estrutura Atual

Com base no widget existente:

| Elemento | Descrição |
|----------|-----------|
| **Título** | "Trade" no topo |
| **Botão X** | Fecha/cancela a troca |
| **Slots de itens** | Grid 10 slots por jogador (5x2 ou similar) |
| **Slots: 0/10** | Contador de slots usados |
| **Accept Trade** | Confirma e executa a troca (quando ambos aceitam) |
| **Decline Trade** | Cancela a troca |
| **Gold 999999999** | Campo para troca de gold (a ser implementado como input) |

---

## Arquitetura de Dados

### Tabelas existentes

- **trade_sessions**: `trade_session_id`, `player1_id`, `player2_id`, `player1_ready`, `player2_ready`, `status`
- **trade_items**: `trade_item_id`, `trade_session_id`, `player_id`, `inventory_id`, `quantity`
- **player_inventory**: itens do jogador (slots 0-49 = inventário)
- **players.gold**: gold do personagem

### Alteração necessária no schema

Adicionar colunas de gold na sessão de troca:

```sql
-- Executar em: www/umbra_api/scripts/ ou via MySQL Workbench
ALTER TABLE trade_sessions 
ADD COLUMN player1_gold_offer BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Gold que player1 oferece',
ADD COLUMN player2_gold_offer BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Gold que player2 oferece';
```

---

## APIs Necessárias

### 1. add_trade_item.php
**POST** – Adiciona item à janela de troca

| Parâmetro | Tipo | Descrição |
|-----------|------|-----------|
| token | string | JWT |
| trade_session_id | int | ID da sessão |
| inventory_id | int | ID do item no inventário |
| quantity | int | Quantidade (padrão 1) |

**Validações:**
- Jogador pertence à sessão
- Item pertence ao jogador
- Item não equipado
- Sessão status = 'active'
- Máximo 10 itens por jogador na trade

**Resposta:** `{ success, message, trade_items (atualizado) }`

---

### 2. remove_trade_item.php
**POST** – Remove item da janela de troca

| Parâmetro | Tipo |
|-----------|------|
| token | string |
| trade_session_id | int |
| trade_item_id | int (ou inventory_id) |

---

### 3. set_trade_gold.php
**POST** – Define quantidade de gold oferecida

| Parâmetro | Tipo |
|-----------|------|
| token | string |
| trade_session_id | int |
| gold_amount | int |

**Validação:** gold_amount <= gold atual do jogador

---

### 4. get_trade_state.php
**POST** – Obtém estado atual da troca (itens + gold de ambos)

| Parâmetro | Tipo |
|-----------|------|
| token | string |
| trade_session_id | int |

**Resposta:**
```json
{
  "success": true,
  "player1_id": 1,
  "player2_id": 23,
  "player1_items": [...],
  "player2_items": [...],
  "player1_gold_offer": 100,
  "player2_gold_offer": 50,
  "player1_ready": false,
  "player2_ready": false,
  "my_gold": 999999
}
```

---

### 5. set_trade_ready.php
**POST** – Marca jogador como "pronto" (clicou Accept Trade)

| Parâmetro | Tipo |
|-----------|------|
| token | string |
| trade_session_id | int |
| ready | bool |

**Lógica:**
- Se `ready = true`: atualiza `player1_ready` ou `player2_ready`
- Se ambos `ready = true`: chama lógica de execução da troca

---

### 6. execute_trade.php (lógica interna)
Chamado automaticamente quando ambos estão ready, ou como endpoint separado.

**Fluxo:**
1. Validar ambos ready
2. Validar itens ainda existem no inventário
3. Validar gold suficiente
4. Em transação:
   - Trocar `player_id` dos itens em trade_items (ou mover entre inventários)
   - Debitar gold de cada jogador e creditar no outro
5. Atualizar `trade_sessions.status = 'completed'`
6. Retornar sucesso

---

### 7. cancel_trade.php (existente)
Já existe. Ao clicar Decline Trade ou X, chamar este endpoint e depois `OnTradeCancelled`.

---

## Integração no Cliente UE (UmbraGameInstance)

### Novas funções necessárias

| Função | Descrição |
|--------|-----------|
| `AddTradeItem(int32 TradeSessionID, int32 InventoryID, int32 Quantity)` | Chama add_trade_item.php |
| `RemoveTradeItem(int32 TradeSessionID, int32 TradeItemID)` | Chama remove_trade_item.php |
| `SetTradeGold(int32 TradeSessionID, int32 GoldAmount)` | Chama set_trade_gold.php |
| `SetTradeReady(int32 TradeSessionID, bool bReady)` | Chama set_trade_ready.php |
| `LoadTradeState(int32 TradeSessionID)` | Chama get_trade_state.php (polling ou on-demand) |

### Atualizar CancelTrade

Substituir o `// TODO` por chamada HTTP a `cancel_trade.php` com `trade_session_id` e, ao sucesso, limpar `CurrentTradeSessionID`, `TradePartnerID` e disparar `OnTradeCancelled`.

### Novos delegates (opcional)

| Delegate | Uso |
|----------|-----|
| `OnTradeStateUpdated` | Quando get_trade_state retorna (atualizar UI) |
| `OnTradeCompleted` | Quando execute_trade retorna sucesso |
| `OnTradeItemAdded` | Feedback ao adicionar item |

---

## Layout do WBP_Trade (Blueprint)

### Estrutura visual

```
┌─────────────────────────────────────────────────────────┐
│ Trade                                              [X]  │
├─────────────────────────────────────────────────────────┤
│  MEUS ITENS (esquerda)     │  ITENS DO PARCEIRO (direita)│
│  ┌──┬──┬──┬──┬──┐         │  ┌──┬──┬──┬──┬──┐          │
│  │ 1│ 2│ 3│ 4│ 5│         │  │ 1│ 2│ 3│ 4│ 5│          │
│  ├──┼──┼──┼──┼──┤         │  ├──┼──┼──┼──┼──┤          │
│  │ 6│ 7│ 8│ 9│10│         │  │ 6│ 7│ 8│ 9│10│          │
│  └──┴──┴──┴──┴──┘         │  └──┴──┴──┴──┴──┘          │
│  Slots: 0/10              │  Slots: 0/10                │
├─────────────────────────────────────────────────────────┤
│  MEU GOLD:    [_____]     │  GOLD DO PARCEIRO: [_____]   │
│  (EditableTextBox)        │  (TextBlock - só leitura)    │
├─────────────────────────────────────────────────────────┤
│  [Accept Trade]    [Decline Trade]                      │
└─────────────────────────────────────────────────────────┘
```

### Variáveis do widget

| Nome | Tipo | Descrição |
|------|------|-----------|
| `TradeSessionID` | int32 | ID da sessão |
| `TradePartnerID` | int32 | ID do parceiro |
| `MySlots` | TArray\<UWidget\> | 10 slots (Image ou Border) para meus itens |
| `PartnerSlots` | TArray\<UWidget\> | 10 slots para itens do parceiro |
| `TB_MyGold` | EditableTextBox | Input de gold que ofereço |
| `TB_PartnerGold` | TextBlock | Gold que o parceiro oferece |
| `BTN_Accept` | Button | Accept Trade |
| `BTN_Decline` | Button | Decline Trade |
| `BTN_Close` | Button | Botão X |

### Fluxo de dados

1. **OnTradeStarted(TradeSessionID)** → Abre widget, chama `LoadTradeState`
2. **LoadTradeState** → Preenche slots e gold de ambos
3. **Drag&Drop** (do inventário para slot) → `AddTradeItem`
4. **Clicar em item no slot** (remover) → `RemoveTradeItem`
5. **Alterar TB_MyGold** → `SetTradeGold` (on commit ou com debounce)
6. **BTN_Accept** → `SetTradeReady(true)`; se ambos ready, API executa e fecha widget
7. **BTN_Decline / BTN_Close** → `CancelTrade` (que chama API cancel_trade.php)

---

## Fluxo de Execução da Troca

```
Player A                    Server                      Player B
   │                          │                            │
   │  AddTradeItem(1,2,3)     │                            │
   │  SetTradeGold(100)       │                            │
   │  SetTradeReady(true)     │                            │
   │ ────────────────────────►│                            │
   │                          │  player1_ready = true       │
   │                          │  (player2_ready = false)   │
   │                          │  → Aguarda B               │
   │                          │                            │
   │                          │  AddTradeItem(...)          │
   │                          │  SetTradeGold(50)          │
   │                          │  SetTradeReady(true)       │
   │                          │◄────────────────────────────
   │                          │  player2_ready = true      │
   │                          │  → AMBOS READY             │
   │                          │  → execute_trade           │
   │                          │  → Troca itens + gold      │
   │                          │  → status = completed      │
   │  OnTradeCompleted        │  OnTradeCompleted          │
   │◄─────────────────────────│────────────────────────────►
   │  Fecha widget            │                            │  Fecha widget
```

---

## Polling vs WebSocket

- **Polling:** WBP_Trade chama `LoadTradeState` a cada 1–2 segundos enquanto aberto, para ver itens/gold do parceiro e se ele clicou Accept.
- **WebSocket (futuro):** Mensagens `TradeItemAdded`, `TradeGoldChanged`, `TradeReadyChanged` via Zone Server para atualização em tempo real sem polling.

Para a primeira versão, **polling** é suficiente.

---

## Checklist de Implementação

### Backend (PHP)

| Item | Arquivo | Status |
|------|---------|--------|
| ALTER TABLE trade_sessions (gold) | scripts/add_trade_gold_columns.sql | ✅ |
| add_trade_item.php | api/social/ | ✅ |
| remove_trade_item.php | api/social/ | ✅ |
| set_trade_gold.php | api/social/ | ✅ |
| get_trade_state.php | api/social/ | ✅ |
| set_trade_ready.php (+ execute_trade) | api/social/ | ✅ |
| CancelTrade → chamar cancel_trade.php | UmbraGameInstance | ✅ |

### Cliente UE

| Item | Descrição | Status |
|------|-----------|--------|
| AddTradeItem | UmbraGameInstance + HTTP | ✅ |
| RemoveTradeItem | UmbraGameInstance + HTTP | ✅ |
| SetTradeGold | UmbraGameInstance + HTTP | ✅ |
| SetTradeReady | UmbraGameInstance + HTTP | ✅ |
| LoadTradeState | UmbraGameInstance + HTTP | ✅ |
| CancelTrade → API | Chamar cancel_trade.php | ✅ |
| WBP_Trade: 10 slots meus | Grid de slots | ⬜ |
| WBP_Trade: 10 slots parceiro | Grid (read-only) | ⬜ |
| WBP_Trade: Campo Gold | EditableTextBox + TextBlock | ⬜ |
| WBP_Trade: Accept → SetTradeReady | Botão | ⬜ |
| WBP_Trade: Decline → CancelTrade | Botão | ⬜ |
| WBP_Trade: Drag item do inventário | Drag&Drop para slot | ⬜ |
| Timer de polling | LoadTradeState a cada 1–2s | ⬜ |

---

## Ordem de Desenvolvimento Sugerida

1. **SQL** – Adicionar colunas `player1_gold_offer`, `player2_gold_offer`
2. **APIs** – add_trade_item, remove_trade_item, set_trade_gold, get_trade_state, set_trade_ready (com execute_trade)
3. **UmbraGameInstance** – Novas funções HTTP + CancelTrade chamando API
4. **WBP_Trade** – Layout estático (slots, gold, botões)
5. **WBP_Trade** – Bind de LoadTradeState ao abrir
6. **WBP_Trade** – Accept/Decline conectados
7. **WBP_Trade** – Drag&Drop do inventário para slots (ou clique para adicionar)
8. **Polling** – Timer para LoadTradeState

---

## Referências

- `PROCEDIMENTO_WBP_TRADE_REQUESTED.md` – Fluxo de solicitação e aceite
- `www/umbra_api/scripts/create_social_tables.sql` – Schema trade_requests, trade_sessions, trade_items
- `www/umbra_api/api/social/cancel_trade.php` – Cancelamento
- `www/umbra_api/api/inventory/get_inventory.php` – Estrutura de itens
- `www/umbra_api/api/gold/get_gold.php` – Gold do jogador
