# WBP_Whisper – Janela de conversa privada (Whisper)

Guia para implementar a janela de whisper: dois jogadores trocam mensagens; o widget exibe os nomes dos personagens no título e, antes de cada mensagem, o nome do remetente. Lógica em C++ (UmbraWhisperWidget); Blueprint mínimo (layout e uma ligação no HUD).

**Guia só de Blueprint:** alterações necessárias no Editor (botões e HUD) estão em **PROCEDIMENTO_WBP_WHISPER_BLUEPRINT.md**.

---

## 1. Fluxo geral

```
[Abrir whisper]
  • Friend List: clicar em um amigo → WBP_FriendOptions → "Mensagem privada"
      → GameInstance.RequestWhisperConversation(FriendID, FriendName)
      → OnWhisperConversationRequested.Broadcast(PlayerID, PlayerName)
  • Context Menu: clicar em "Whisper" no menu de contexto do jogador
      → Execute Context Action (Whisper)
      → OnWhisperConversationRequested.Broadcast(PlayerID, PlayerName)

[HUD escuta OnWhisperConversationRequested]
  → Create Widget (WBP_Whisper) se ainda não existir
  → WhisperWidgetRef → Open Conversation (PlayerID, PlayerName)
  → Add to Viewport (se ainda não estiver)
  → Set Visibility Visible

[Enviar mensagem]
  → Usuário digita em ET_Input e clica Enviar
  → C++: SendWhisperViaWebSocket(OtherPlayerID, Text, nullptr)
  → Mensagem aparece como "[MeuNome]: texto"

[Receber mensagem]
  → Servidor envia WhisperReceived (tipo 41)
  → GameInstance → OnWhisperReceived.Broadcast(FromPlayerID, FromPlayerName, Message)
  → C++ do widget: se FromPlayerID == OtherPlayerID → AddMessage(OtherPlayerName, Message)
  → Aparece como "[OutroNome]: texto"
```

---

## 2. Pré-requisitos (já implementados em C++)

| Item | Onde | Uso |
|------|------|-----|
| UmbraWhisperWidget | UI/UmbraWhisperWidget.h .cpp | Widget com OpenConversation, AddMessage, Send, bind OnWhisperReceived/OnWhisperSent/OnWhisperFailed |
| OnWhisperConversationRequested | UmbraGameInstance | Bind no HUD para criar/exibir WBP_Whisper e chamar Open Conversation |
| RequestWhisperConversation(PlayerID, PlayerName) | UmbraGameInstance | Chamado por WBP_FriendOptions ao clicar "Mensagem privada" |
| Execute Context Action (Whisper) | Context menu | Já dispara OnWhisperConversationRequested |
| SendWhisperViaWebSocket(TargetID, Message, nullptr) | UmbraGameInstance | Envia via WebSocket; nullptr resolve cliente do ActivePlayerID |

---

## 3. Estrutura do widget WBP_Whisper

O Blueprint deve **herdar de UmbraWhisperWidget** (Parent Class = UmbraWhisperWidget) e expor os widgets com os **nomes exatos** abaixo para BindWidgetOptional.

### 3.1 Hierarquia recomendada

```
WBP_Whisper (UmbraWhisperWidget)
├── [Canvas / Border principal]
│   ├── Text_Title          (Text Block)     — Título: "Conversa: Nome1 ↔ Nome2"
│   ├── Scroll_Messages     (Scroll Box)     — Área rolável das mensagens
│   │   └── VB_Messages     (Vertical Box)  — Filho único do Scroll; C++ adiciona TextBlocks aqui
│   ├── ET_Input            (Editable Text)  — Campo para digitar a mensagem
│   ├── BTN_Send            (Button)         — Botão Enviar
│   └── BTN_Close           (Button)         — Botão Fechar
```

### 3.2 Nomes obrigatórios (Bind Widget Optional)

| Nome no Designer | Tipo | Descrição |
|------------------|------|-----------|
| Text_Title | Text Block | Nomes dos dois personagens (C++ preenche "Conversa: Nome1 ↔ Nome2") |
| Scroll_Messages | Scroll Box | Contém a lista de mensagens; C++ faz Scroll To End ao adicionar linha |
| VB_Messages | Vertical Box | Deve ser **filho** do Scroll_Messages; C++ adiciona uma linha (Text Block) por mensagem |
| ET_Input | Editable Text | Texto digitado pelo jogador; C++ lê em Send Current Message |
| BTN_Send | Button | Enviar; **Blueprint** deve ligar OnClicked → **Send Current Message** |
| BTN_Close | Button | Fechar; **Blueprint** deve ligar OnClicked → **Close Whisper** |

### 3.3 Formato de cada mensagem

C++ adiciona linhas no formato: **`[NomeDoRemetente]: texto da mensagem`**

- Mensagens enviadas: `[MeuNome]: ...` (MeuNome = personagem ativo do jogador local).
- Mensagens recebidas: `[OutroNome]: ...` (OutroNome = nome do jogador com quem se está conversando).

---

## 4. Passo a passo no Blueprint

### 4.1 Criar o Blueprint WBP_Whisper

1. Criar Widget Blueprint (ex.: `Content/Widgets/UI/Whisper/WBP_Whisper`).
2. **Parent Class:** **UmbraWhisperWidget** (procurar na lista de classes).
3. No Designer, adicionar os controles com os **nomes exatos** da tabela acima.
4. Colocar **VB_Messages** **dentro** de **Scroll_Messages** (VB_Messages como filho do Scroll Box).
5. Ajustar layout (tamanho do scroll, posição do input e dos botões).

### 4.1.1 Event Graph do WBP_Whisper (obrigatório)

O C++ **não** liga mais os botões automaticamente. No **Event Graph** do WBP_Whisper:

1. **Event Construct**
2. **BTN_Send:** arraste **BTN_Send** para o gráfico → **Bind Event to OnClicked** → no evento que dispara ao clicar, chame **Send Current Message** (Target = **self** / este widget).
3. **BTN_Close:** arraste **BTN_Close** para o gráfico → **Bind Event to OnClicked** → no evento que dispara ao clicar, chame **Close Whisper** (Target = **self** / este widget).

Assim o Enviar envia o texto do ET_Input e o Fechar esconde o widget (Set Visibility Collapsed). Para o whisper voltar a abrir, o HUD deve chamar **Add to Viewport** + **Set Visibility Visible** ao receber OnWhisperConversationRequested (ver 4.2).

### 4.2 Onde abrir o whisper (ex.: WBP_PlayerHUD)

O HUD (ou outro widget que possua a referência ao painel) deve:

1. Ter uma variável **WhisperWidgetRef** (tipo **Umbra Whisper Widget** ou **User Widget**).
2. No **Event Construct** (ou no primeiro uso):
   - **Bind Event to On Whisper Conversation Requested** (Target = **Get Game Instance** → **Cast to Umbra Game Instance**).
   - **Event** → ligar a um **Custom Event** (ex.: **OnWhisperConversationRequested_Event** com parâmetros **PlayerID** (Integer) e **PlayerName** (String)).

3. No **Custom Event OnWhisperConversationRequested_Event(PlayerID, PlayerName)**:
   - **Branch:** **Is Valid (WhisperWidgetRef)**?
   - **False:** **Create Widget** (Class = **WBP_Whisper**, Owning Player = **Get Player Controller (0)**) → **Set WhisperWidgetRef** (valor = Return Value).
   - **True:** (não criar de novo)
   - **Sempre (após o Branch):** **Open Conversation** (Target = **WhisperWidgetRef**; Other Player ID = **PlayerID**; Other Player Name = **PlayerName**) → **Add to Viewport** (Target = WhisperWidgetRef) → **Set Visibility** (Target = WhisperWidgetRef; **Visible**).
   - **Importante:** Chamar **Add to Viewport** sempre ao abrir. O C++ em **Open Conversation** apenas torna o widget visível; o widget **não** é removido do viewport ao fechar (só fica Collapsed). Assim, ao reabrir, ele continua no viewport e volta a aparecer.

4. **Botão Fechar (BTN_Close):** no WBP_Whisper, no Event Graph, o OnClicked de BTN_Close deve chamar **Close Whisper** (ver 4.1.1). **Não** use **Remove From Parent** no fechar — **Close Whisper** só faz Set Visibility Collapsed; o widget permanece no viewport. Para reabrir, o HUD chama **Add to Viewport** + **Set Visibility Visible** ao abrir de novo.

Resumo dos nós no Custom Event:

| Ordem | Nó | Conexões |
|-------|-----|----------|
| 1 | Is Valid | Target = WhisperWidgetRef |
| 2 | Branch | Condition = Return Value do Is Valid |
| 3 (ramo False) | Create Widget | Class = WBP_Whisper; Owning Player = Get Player Controller (0) |
| 4 | Set WhisperWidgetRef | Valor = Return Value do Create Widget |
| 5 (após Branch) | Open Conversation | Target = WhisperWidgetRef; Other Player ID = PlayerID; Other Player Name = PlayerName |
| 6 | **Add to Viewport** | Target = WhisperWidgetRef *(sempre, para reabrir após fechar)* |
| 7 | Set Visibility | Target = WhisperWidgetRef; In Visibility = Visible |

---

## 5. Friend List e Context Menu

- **WBP_FriendOptions** (botão "Mensagem privada"): em C++ já chama **RequestWhisperConversation(CurrentFriendID, CurrentFriendName)** e fecha as opções. Nada a fazer no Blueprint além de garantir que o botão existe (BTN_PrivateMessage).
- **WBP_PlayerContextMenu** (botão Whisper): deve chamar **Execute Context Action** com **Action = Whisper**; o C++ do GameInstance já faz **OnWhisperConversationRequested.Broadcast(PlayerID, PlayerName)**. Ver **GUIA_WBP_CONTEXT_MENU_BOTOES.md** se necessário.

---

## 6. Resumo

- **C++:** UmbraWhisperWidget (OpenConversation, AddMessage, envio via SendWhisperViaWebSocket, bind OnWhisperReceived/OnWhisperSent/OnWhisperFailed). GameInstance: OnWhisperConversationRequested, RequestWhisperConversation, SendWhisperViaWebSocket com WebSocket opcional (nullptr).
- **Blueprint:** (1) WBP_Whisper herda de UmbraWhisperWidget; layout com Text_Title, Scroll_Messages → VB_Messages, ET_Input, BTN_Send, BTN_Close. (2) HUD (ou equivalente) faz Bind to On Whisper Conversation Requested e no Custom Event cria WBP_Whisper (se não existir), chama Open Conversation e torna o widget visível.

Assim, ao clicar em um amigo (Mensagem privada) ou no botão Whisper do context menu, abre-se a mesma janela de whisper com o nome dos dois personagens no título e cada mensagem exibida como **[Remetente]: texto**.
