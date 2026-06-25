# Guia de implementação: WBP_ChatMain (Chat Local + Global + Grupo) — UE 5.6.1

**Engine:** Unreal Engine **5.6.1**  
**Regra do projeto:** lógica em **C++**, Blueprint só para **layout + chamadas de função**.

---

## 1) Pré-requisitos

| Passo | Ação |
|---|---|
| C++ cliente | Compilar `UmbraEternumUE` após puxar as classes `UUmbraChatMainWidget` e mudanças no `UUmbraGameInstance`. |
| Zone server | Rodar `zone_server` atualizado com suporte aos msgTypes de chat (66-72). |
| WebSocket | Garantir que o cliente está conectado ao Zone (`NetMovementClient` ativo). |
| Teste | Usar PIE com 2+ clientes para validar Local (proximidade), Global (todos) e Grupo (somente party). |

---

## 2) Parent class e asset

1. No Content Browser: **User Interface -> Widget Blueprint**.
2. Nome sugerido: `WBP_ChatMain`.
3. **Class Settings -> Parent Class:** `UmbraChatMainWidget`.
4. Compile e Save.

---

## 3) Hierarquia obrigatória (nomes exatos)

Crie os widgets com estes nomes (o C++ espera estes IDs):

- `BTN_TabLocal` (`Button`)
- `BTN_TabGlobal` (`Button`)
- `BTN_TabGroup` (`Button`)
- `Scroll_ChatFeed` (`ScrollBox`)
- `VB_ChatLines` (`VerticalBox`) como filho de `Scroll_ChatFeed`
- `Border_ChatInput` (`Border`) envolvendo o input
- `ET_ChatInput` (`EditableText`)
- `BTN_SendChat` (`Button`)
- `TXT_ChatInfo` (`TextBlock`)
- `BTN_CloseChat` (`Button`) opcional para modo painel

Estrutura mínima sugerida:

```text
Canvas
└── VerticalBox_Main
    ├── HorizontalBox_Tabs (BTN_TabLocal, BTN_TabGlobal, BTN_TabGroup)
    ├── Scroll_ChatFeed
    │   └── VB_ChatLines
    ├── HorizontalBox_Input
    │   ├── Border_ChatInput
    │   │   └── ET_ChatInput
    │   └── BTN_SendChat
    ├── TXT_ChatInfo
    └── BTN_CloseChat
```

---

## 4) Event Graph (nó a nó, mínimo)

> O C++ já faz bind de botões no `NativeConstruct`, inclusive o fluxo de Enter.
> O Graph pode ficar vazio se você usar os nomes corretos.

### 4.1 BTN_TabLocal
- `OnClicked(BTN_TabLocal)` -> chamar função `Set Active Channel Local`.

### 4.2 BTN_TabGlobal
- `OnClicked(BTN_TabGlobal)` -> chamar função `Set Active Channel Global`.

### 4.3 BTN_TabGroup
- `OnClicked(BTN_TabGroup)` -> chamar função `Set Active Channel Group`.

### 4.4 BTN_SendChat
- `OnClicked(BTN_SendChat)` -> chamar função `Send Current Chat Message`.

### 4.5 Enter (abre/envia/fecha)
- Sem input visível: `Enter` abre `Border_ChatInput` + `ET_ChatInput` + `BTN_SendChat`.
- Input visível e com texto: `Enter` envia mensagem.
- Input visível e vazio: `Enter` fecha o input.

### 4.6 BTN_CloseChat (opcional)
- `OnClicked(BTN_CloseChat)` -> chamar função `Close Chat`.

---

## 5) Como o fluxo funciona (C++ -> Blueprint)

1. `WBP_ChatMain` chama `SendCurrentChatMessage()` (C++).
2. C++ escolhe canal ativo:
   - Local -> `SendLocalChatViaWebSocket`.
   - Global -> `SendGlobalChatViaWebSocket`.
   - Grupo -> `SendGroupChatViaWebSocket` (requer `IsInParty()`).
3. Zone valida/modera e retorna:
   - `ChatLocalReceived (69)` para próximos.
   - `ChatGlobalReceived (70)` para todos.
   - `ChatGroupReceived (71)` para membros do grupo.
   - `ChatServerError (72)` quando rejeita.
4. `GameInstance` recebe e dispara delegates:
   - `OnLocalChatReceived`
   - `OnGlobalChatReceived`
   - `OnGroupChatReceived`
   - `OnChatSendFailed`
   - `OnChatSystemInfo`
5. `UUmbraChatMainWidget` atualiza feed e `TXT_ChatInfo`.

---

## 6) Boas práticas de layout

- Deixe `Scroll_ChatFeed` com **Fill** no `VerticalBox` para o chat ocupar o espaço principal.
- `VB_ChatLines` deve estar dentro do `Scroll_ChatFeed`.
- `Border_ChatInput` deve envolver o `ET_ChatInput` e começar **Collapsed**.
- `ET_ChatInput` com largura maior que `BTN_SendChat`.
- O C++ aplica quebra de linha automática no feed e limite de caracteres por mensagem.
- Se for HUD fixo, prefira esconder com `CloseChat` em vez de remover widget toda hora.

---

## 7) Checklist de validação (PIE)

1. Cliente A e B conectados no Zone.
2. A envia em **Local**:
   - A vê mensagem.
   - B só vê se estiver próximo.
3. A envia em **Global**:
   - todos os clientes conectados veem.
4. A envia em **Grupo**:
   - apenas membros do mesmo grupo recebem.
   - se não estiver em grupo, recebe erro amigável.
5. Spam acima do limite:
   - cliente recebe erro via `OnChatSendFailed`.
6. Mensagem vazia:
   - bloqueada no cliente/servidor com feedback.

---

## 8) Troubleshooting rápido

| Sintoma | Causa provável | Correção |
|---|---|---|
| Não envia nada | `GetActiveMovementWSClient` nulo | Conferir conexão WebSocket e `NetMovementClient` ativo. |
| Mensagem não aparece | Nome de widget errado no Designer | Validar nomes exatos da seção 3. |
| `ET_ChatInput` não aparece no Graph | Widget não marcado como variável ou nome divergente | Marcar **Is Variable** e manter nome `ET_ChatInput`; o C++ também tenta fallback por nome em runtime. |
| Enter não abre input | `Border_ChatInput` ausente ou nome diferente | Criar `Border_ChatInput` e manter nome exato. |
| Chat de grupo não entrega | jogador fora de party ou party desatualizada | validar `LoadPartyState(0)` após conectar e checar `IsInParty()`. |
| Local chega para todo mundo | Canal errado no envio ou handler server incorreto | Verificar uso de `broadcastToNearby` para Local. |
| Erro constante de limite | rate-limit baixo em `config/server.json` | Ajustar `chat.rate_limit_per_minute` e reiniciar Zone. |
| Texto estranho/rompido | payload UTF-8 inválido | revisar montagem de bytes no envio C++. |

