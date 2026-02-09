# Procedimento: Abrir Inventário com Trade e Recarregar ao Concluir

Ao abrir a janela de trade, o inventário também deve abrir (para arrastar itens). Ao efetuar o trade (ambos clicam Accept), o inventário deve ser recarregado para refletir o estado atual.

---

## Visão geral

| Momento | Ação |
|---------|------|
| **Trade abre** (OnTradeStarted) | Mostrar WBP_Trade + **mostrar WBP_Inventory** |
| **Trade completa** (OnTradeCompleted) | **LoadInventory** (recarrega do servidor) |

---

## Onde implementar

O bind de **OnTradeStarted** e **OnTradeCompleted** geralmente fica no **HUD** ou no **Player Controller**. Onde você já cria/add o WBP_Trade ao viewport, adicione também a lógica do inventário.

---

## Parte 1: Ao abrir o trade – também abrir o inventário

### 1.1 Bind de OnTradeStarted

Quando **OnTradeStarted** dispara (aceitou a troca ou recebeu via WebSocket):

1. **Create Widget** (WBP_Trade) e **Add to Viewport** (como já faz hoje).
2. **Create Widget** (WBP_Inventory) e **Add to Viewport** — ou, se o inventário já existir como variável, use **Add to Viewport** nele.

**Exemplo de fluxo:**

```
[Event] OnTradeStarted (TradeSessionID)  ← bind do GameInstance
    │
    ├─► Create Widget (WBP_Trade)
    ├─► Add to Viewport (Target = WBP_Trade)
    │
    ├─► Branch: Inventário já criado?
    │   ├─► TRUE:  Add to Viewport (Target = InventoryWidgetRef)
    │   └─► FALSE: Create Widget (WBP_Inventory)
    │              Add to Viewport (Target = novo widget)
    │              (Opcional) Set InventoryWidgetRef = widget (para reutilizar)
    │
    └─► Iniciar polling LoadTradeState (como já faz)
```

### 1.2 Se o inventário for toggle (tecla I)

Se o inventário é controlado por tecla (ex.: "I" para abrir/fechar):

- Ao abrir o trade, chame a mesma lógica que abre o inventário (ex.: um Custom Event `OpenInventory`).
- Ou force **Add to Viewport** no widget de inventário quando OnTradeStarted disparar.

### 1.3 Variável de referência (opcional)

Para não criar vários inventários:

| Variável | Tipo | Descrição |
|----------|------|-----------|
| InventoryWidgetRef | WBP_Inventory (object ref) | Referência ao widget de inventário |

No **Event Construct** do HUD (ou onde fizer sentido): `InventoryWidgetRef = null`.  
Na primeira vez que abrir o trade: Create Widget → Add to Viewport → Set InventoryWidgetRef.  
Nas próximas: `Add to Viewport (InventoryWidgetRef)` se ele tiver sido removido.

---

## Parte 2: Ao concluir o trade – recarregar inventário

### 2.1 Bind de OnTradeCompleted

Quando **OnTradeCompleted** dispara (ambos clicaram Accept e a troca foi executada):

1. **Load Inventory** (GameInstance) — busca inventário atualizado no servidor.
2. O `OnInventoryLoaded` que o WBP_Inventory já usa vai atualizar os slots.

**Exemplo de fluxo:**

```
[Event] OnTradeCompleted  ← bind do GameInstance
    │
    ├─► Get Game Instance → Cast to UmbraGameInstance
    ├─► Load Inventory
    │
    └─► (O widget de trade geralmente fecha em UpdateTradeUIFromState
         quando status = completed, ou no bind de OnTradeCompleted)
```

### 2.2 Onde o inventário é atualizado

O **LoadInventory** do GameInstance:
- Chama a API de inventário.
- Em **OnLoadInventoryRequestComplete**, faz `OnInventoryLoaded.Broadcast()`.
- O **WBP_Inventory** (ou storage) deve estar com bind em **On Inventory Loaded** para atualizar os slots.

Se o inventário ainda não tiver esse bind, adicione no **Event Construct**:

```
Bind Event to On Inventory Loaded (Target = GameInstance)
    → Event: Custom Event que refresha os slots do inventário
```

---

## Parte 3: Cancelamento (OnTradeCancelled)

Ao cancelar o trade, o inventário não precisa ser recarregado, pois nenhum item foi trocado.  
Opcionalmente, pode manter o inventário aberto ou fechá-lo conforme sua UX.

---

## Checklist

| Item | Verificação |
|------|-------------|
| OnTradeStarted: Add WBP_Trade ao viewport | ⬜ |
| OnTradeStarted: Add WBP_Inventory ao viewport (ou abrir inventário) | ⬜ |
| OnTradeCompleted: Load Inventory (GameInstance) | ⬜ |
| WBP_Inventory com bind em On Inventory Loaded para refresh dos slots | ⬜ |

---

## Resumo

| Evento | Ação |
|--------|------|
| **OnTradeStarted** | Create/Add WBP_Trade + Create/Add WBP_Inventory |
| **OnTradeCompleted** | Load Inventory |

O `LoadInventory` busca os dados da API e dispara `OnInventoryLoaded`; o widget de inventário atualiza os slots automaticamente se estiver vinculado a esse evento.
