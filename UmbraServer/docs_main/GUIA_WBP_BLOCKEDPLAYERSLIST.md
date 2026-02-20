# WBP_BlockedPlayersList – Lista de jogadores bloqueados

Seguir **exatamente** o mesmo padrão do **WBP_FriendList**: tudo no **Event Graph**, **Custom Events**, **Bind Event** ligado a Custom Event. Nenhuma função para preencher a lista; o preenchimento e o bind ficam no **Custom Event** que escuta On Blocked List Loaded (igual ao Friend List com On Friend List Loaded).

Referência: **PROCEDIMENTO_WBP_FRIENDLIST_MELHORIAS.md** — Parte 3.3 (Event Graph OnFriendListLoaded_Event) e 3.4 (Custom Event OnFriendSlotClicked_Event).

---

## 1. Fluxo (igual à Friend List)

```
[Botão "Lista de bloqueados"]
    → Create Widget (WBP_BlockedPlayersList) → Add to Viewport

[Event Construct da lista]
    → Get Game Instance → Cast → Set MyGI
    → Bind Event to On Blocked List Loaded → Custom Event OnBlockedListLoaded_Event
    → Load Blocked List And Start Refresh (MyGI, 30)
    → Create Widget (WBP_BlockedPlayerOptions) → Set OptionsWidgetRef → Add to Viewport → Set Visibility Collapsed

[Quando a API responde]
    → OnBlockedListLoaded.Broadcast()
    → Custom Event OnBlockedListLoaded_Event executa:
        Clear Children (VB_Slots)
        For Loop (0 to Get Blocked Players Count − 1):
            Get Blocked Player ID At, Get Blocked Player Name
            Create Widget (WBP_BlockedPlayerSlot)
            Set Blocked Player Data (slot, PlayerID, PlayerName)
            Bind Event to On Blocked Player Slot Clicked → Custom Event OnBlockedSlotClicked_Event
            Add Child to Vertical Box (VB_Slots, slot)

[Clique em um slot]
    → OnBlockedSlotClicked_Event(PlayerID, PlayerName) executa:
        OptionsWidgetRef → Show Options (PlayerID, PlayerName)
        OptionsWidgetRef → Set Visibility Visible
```

---

## 2. WBP_BlockedPlayersList – Event Graph

### 2.1 Event Construct – cadeia

| Ordem | Nó | Conexões |
|-------|-----|----------|
| 1 | Get Game Instance | GameplayStatics |
| 2 | Cast to UmbraGameInstance | Object = Return Value do Get Game Instance |
| 3 | Set MyGameInstance | Valor = As Umbra Game Instance do Cast |
| 4 | **Bind Event to On Blocked List Loaded** | **Target** = MyGameInstance (ou saída do Set). **Event** = **Custom Event OnBlockedListLoaded_Event** (criar o Custom Event e ligar o pino Event a ele). |
| 5 | Load Blocked List And Start Refresh | Target = MyGameInstance; Interval Seconds = 30 |
| 6 | Create Widget | Class = WBP_BlockedPlayerOptions; Owning Player = Get Player Controller (0) |
| 7 | Set OptionsWidgetRef | Valor = Return Value do Create Widget |
| 8 | Add to Viewport | Target = OptionsWidgetRef |
| 9 | Set Visibility | Target = OptionsWidgetRef; InVisibility = Collapsed |

### 2.2 Custom Event: OnBlockedListLoaded_Event (sem parâmetros)

**Onde:** Event Graph. **Quando dispara:** quando o GameInstance faz Broadcast de On Blocked List Loaded (resposta da API).

**Corpo (igual ao Friend List – Parte 3.3):**

| Ordem | Nó | Conexões |
|-------|-----|----------|
| 1 | Clear Children | Target = **VB_Slots** |
| 2 | Get Blocked Players Count | Target = **MyGameInstance** |
| 3 | For Loop | First Index = **0**; Last Index = **Get Blocked Players Count** (Target = MyGI) **− 1** |
| 4 | Get Blocked Player ID At | Target = MyGameInstance; Index = **Index** (saída do For Loop) |
| 5 | Get Blocked Player Name | Target = MyGameInstance; Player ID = saída do Get Blocked Player ID At |
| 6 | Create Widget | Class = **WBP_BlockedPlayerSlot**; Owning Player = Get Player Controller (0) |
| 7 | Set Blocked Player Data | **Target** = Return Value do Create Widget; PlayerID e PlayerName = saídas dos getters acima |
| 8 | **Bind Event to On Blocked Player Slot Clicked** | **Target** = Return Value do Create Widget (o slot). **Event** = **Custom Event OnBlockedSlotClicked_Event** (criar esse Custom Event com dois parâmetros: PlayerID Integer, PlayerName String; ligar o pino Event do Bind a esse Custom Event). |
| 9 | Add Child to Vertical Box | Target = **VB_Slots**; Content = Return Value do Create Widget |

Tudo no **Event Graph**, dentro deste Custom Event. Não usar função RefreshList.

### 2.3 Custom Event: OnBlockedSlotClicked_Event(PlayerID, PlayerName)

**Onde:** Event Graph. **Quando dispara:** quando o jogador clica num slot (o Bind Event acima chama este Custom Event).

**Corpo (igual ao Friend List – Parte 3.4, que abre WBP_FriendOptions):**

| Ordem | Nó | Conexões |
|-------|-----|----------|
| 1 | Show Options | **Target** = **OptionsWidgetRef**; PlayerID e PlayerName = parâmetros do Custom Event |
| 2 | Set Visibility | **Target** = **OptionsWidgetRef**; InVisibility = **Visible** |

O popup de opções já foi criado no Construct e está em OptionsWidgetRef; só mostramos e preenchemos com Show Options.

### 2.4 Botão Fechar do painel (ex.: Btn_Close)

| Ordem | Nó | Conexões |
|-------|-----|----------|
| 1 | Stop Blocked List Auto Refresh | Target = **MyGameInstance** |
| 2 | Remove From Parent | Target = **self** |

---

## 3. Onde cada coisa fica

| O quê | Onde |
|-------|------|
| Bind Event to On Blocked List Loaded | **Event Graph** — saída do Event Construct. Event = **OnBlockedListLoaded_Event** (Custom Event). |
| Lógica de preencher a lista (Clear, For Loop, Create Slot, Set Data, **Bind Event to On Blocked Player Slot Clicked**, Add Child) | **Dentro do Custom Event OnBlockedListLoaded_Event** (Event Graph). Não dentro de função. |
| Bind Event to On Blocked Player Slot Clicked | **Dentro do Custom Event OnBlockedListLoaded_Event**, no loop. Target = slot criado. Event = **OnBlockedSlotClicked_Event** (Custom Event com PlayerID e PlayerName). |
| Abrir popup ao clicar no slot | **Custom Event OnBlockedSlotClicked_Event** — Show Options (Target = OptionsWidgetRef), Set Visibility Visible. |

---

## 4. Variáveis e layout (WBP_BlockedPlayersList)

- **VB_Slots:** Vertical Box (dentro do Scroll Box), variável promovida; Clear Children e Add Child usam VB_Slots.
- **OptionsWidgetRef:** Object Reference (User Widget); no Construct recebe o Create Widget (WBP_BlockedPlayerOptions).
- **MyGameInstance:** Object Reference (UmbraGameInstance); no Construct recebe o Cast.

Create Widget: escolher a classe no dropdown (WBP_BlockedPlayerSlot, WBP_BlockedPlayerOptions), como no resto do projeto.

---

## 5. Resumo (espelho da Friend List)

- **Friend List:** Custom Event **OnFriendListLoaded_Event** faz Clear + For Loop + Create Slot + Set Friend Data + **Bind Event to On Friend Slot Clicked** → **OnFriendSlotClicked_Event** + Add Child. Tudo no Event Graph.
- **Blocked List:** Custom Event **OnBlockedListLoaded_Event** faz Clear + For Loop + Create Slot + Set Blocked Player Data + **Bind Event to On Blocked Player Slot Clicked** → **OnBlockedSlotClicked_Event** + Add Child. Tudo no Event Graph.

Nenhuma função; só Event Graph e Custom Events, como na Friend List.
