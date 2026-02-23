# Procedimento Blueprint – WBP_Whisper

Alterações **somente no Editor** para o whisper funcionar (Enviar, Fechar e reabrir). Use o evento **On Clicked** pelo painel **Details** do botão — não é preciso arrastar nada do Designer para o Event Graph.

---

## 1. WBP_Whisper – Ligar os botões

### 1.1 Botão Enviar (BTN_Send)

1. Abra **WBP_Whisper**.
2. Fique na aba **Designer**.
3. Na **Hierarchy** (árvore à esquerda), clique em **BTN_Send** para selecionar o botão.
4. No painel **Details** (à direita), role até achar a seção **Events** (ou **Behavior**).
5. Ache a linha **On Clicked**.
6. Clique no **+** ao lado de **On Clicked** para adicionar o evento.
7. O Unreal cria sozinho um nó **On Clicked (BTN_Send)** no Event Graph e pode abrir o gráfico.
8. Vá na aba **Event Graph**. Do nó **On Clicked (BTN_Send)**, arraste o pin de execução (branco) e solte → procure **Send Current Message** → escolha o nó. O Target é este widget (ou deixe em branco se estiver no próprio WBP_Whisper).

**Resumo:** Selecionar BTN_Send → Details → On Clicked → + → no Event Graph, do evento criado chamar **Send Current Message**.

### 1.2 Botão Fechar (BTN_Close)

1. Volte à aba **Designer**.
2. Na **Hierarchy**, clique em **BTN_Close**.
3. No **Details**, em **Events**, ache **On Clicked**.
4. Clique no **+** ao lado de **On Clicked**.
5. Vá na aba **Event Graph**. Do nó **On Clicked (BTN_Close)** criado, ligue a execução ao nó **Close Whisper** (clique direito → procurar "Close Whisper"). Target = este widget se o nó pedir.

**Resumo:** Selecionar BTN_Close → Details → On Clicked → + → no Event Graph, do evento criado chamar **Close Whisper**.

### 1.3 O que não fazer

- No botão Fechar, **não** use **Remove From Parent**. Use **só** o nó **Close Whisper**.
- **Não** encadeie mais nada no Fechar (ex.: esconder outro painel). Só **Close Whisper**.

---

## 2. HUD (ex.: WBP_PlayerHUD) – Abrir e reabrir o whisper

O HUD cria o widget (na primeira vez), chama **Open Conversation** e garante que o widget volte ao viewport ao reabrir.

### 2.1 Variável

- Crie uma variável **WhisperWidgetRef**.
- Tipo: **Umbra Whisper Widget** (ou **User Widget**).
- Instance Editable = false (pode deixar).

### 2.2 Ligar ao evento do Game Instance

No **Event Construct** do HUD:

1. **Get Game Instance** → **Cast to Umbra Game Instance**.
2. Do cast, use **Bind Event to On Whisper Conversation Requested**.
3. No **Event** que aparece (parâmetros **PlayerID** e **PlayerName**), ligue a um **Custom Event** do HUD.

**Custom Event:** nome **OnWhisperConversationRequested_Event**, parâmetros **PlayerID** (Integer) e **PlayerName** (String).

### 2.3 Lógica do Custom Event (abrir / reabrir)

No **Custom Event OnWhisperConversationRequested_Event(PlayerID, PlayerName)**:

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | **Is Valid** | Target = **WhisperWidgetRef** |
| 2 | **Branch** | Condition = Return Value do Is Valid |
| 3 (False) | **Create Widget** | Class = **WBP_Whisper**; Owning Player = **Get Player Controller (0)** |
| 4 (False) | **Set WhisperWidgetRef** | Valor = Return Value do Create Widget |
| 5 (após o Branch) | **Open Conversation** | Target = **WhisperWidgetRef**; Other Player ID = **PlayerID**; Other Player Name = **PlayerName** |
| 6 | **Add to Viewport** | Target = **WhisperWidgetRef**; Z-Order = 0 |
| 7 | **Set Visibility** | Target = **WhisperWidgetRef**; Visibility = **Visible** |

**Ordem:** Open Conversation → Add to Viewport → Set Visibility Visible.

Sempre chamar **Add to Viewport** ao abrir para o whisper reaparecer ao usar de novo o context menu ou "Mensagem privada".

### 2.4 O que não fazer no HUD

- **Não** limpar **WhisperWidgetRef** ao fechar (não setar como null).
- **Não** usar **Remove From Parent** no whisper; o fechar é só **Close Whisper** no WBP_Whisper.

---

## 3. Checklist

- [ ] **WBP_Whisper:** BTN_Send → Details → On Clicked → + → Event Graph: evento **On Clicked (BTN_Send)** → **Send Current Message**.
- [ ] **WBP_Whisper:** BTN_Close → Details → On Clicked → + → Event Graph: evento **On Clicked (BTN_Close)** → **Close Whisper**.
- [ ] **WBP_Whisper:** No Fechar, **não** usar Remove From Parent.
- [ ] **HUD:** Variável **WhisperWidgetRef** existe.
- [ ] **HUD:** Event Construct → Bind Event to **On Whisper Conversation Requested** → Custom Event (PlayerID, PlayerName).
- [ ] **HUD:** No Custom Event: Is Valid → Branch → (False: Create Widget + Set WhisperWidgetRef) → **Open Conversation** → **Add to Viewport** → **Set Visibility Visible**.

---

## 4. Nomes obrigatórios no WBP_Whisper (Designer)

Para o C++ encontrar os controles, os nomes na Hierarchy devem ser **exatos**:

| Nome | Tipo |
|------|------|
| Text_Title | Text Block |
| Scroll_Messages | Scroll Box |
| VB_Messages | Vertical Box (filho de Scroll_Messages) |
| ET_Input | Editable Text |
| BTN_Send | Button |
| BTN_Close | Button |

Guia geral: **GUIA_WBP_WHISPER.md**.
