# Guia completo: Sessão única por conta + retorno ao login após disconnect WS

**Engine:** Unreal Engine **5.6.1**  
**Projeto:** UmbraEternumUE + UmbraServerV2  
**Regra:** lógica de rede e fluxo em **C++**; Blueprint só para **UI** (`WBP_ServerDisconnect` + evento no `BP_UmbraGameInstance`).

---

## Índice

1. [O que a feature faz](#1-o-que-a-feature-faz)
2. [Pré-requisitos e ordem de deploy](#2-pré-requisitos-e-ordem-de-deploy)
3. [Passo a passo — Backend (MySQL, PHP, C++)](#3-passo-a-passo--backend-mysql-php-c)
4. [Passo a passo — Compilar o cliente UE](#4-passo-a-passo--compilar-o-cliente-ue)
5. [Passo a passo — Unreal: WBP_ServerDisconnect](#5-passo-a-passo--unreal-wbp_serverdisconnect)
6. [Passo a passo — Unreal: BP_UmbraGameInstance](#6-passo-a-passo--unreal-bp_umbragameinstance)
7. [Passo a passo — Parar timers no personagem (opcional)](#7-passo-a-passo--parar-timers-no-personagem-opcional)
8. [Testes integrados](#8-testes-integrados)
9. [Troubleshooting](#9-troubleshooting)
10. [Referência técnica (opcodes e arquivos)](#10-referência-técnica-opcodes-e-arquivos)

---

## 1) O que a feature faz

### Sessão única por conta

- Cada **login HTTP** incrementa `accounts.session_version` no MySQL.
- O JWT passa a carregar o claim `session_version`.
- Ao conectar o **WebSocket da zone**, o cliente envia o opcode **109** (`SessionAuthNotify`) com o JWT.
- A zone valida assinatura JWT + `session_version` no banco.
- Se a **mesma conta** já estiver conectada em outro client:
  - o client **antigo** recebe opcode **110** (`SessionRevokedNotify`) com a mensagem *"Sua conta entrou em outro cliente."*;
  - o client **antigo** é desconectado;
  - o client **novo** continua jogando normalmente.

### UX de desconexão

- Queda de WS, kick ou login duplicado disparam `NotifyZoneDisconnected` no `UUmbraGameInstance`.
- O Blueprint implementa `ShowServerDisconnectDialog` → modal com a mensagem.
- Ao clicar **OK**, chama `ReturnToLoginLevel()` → desconecta Gateway TCP, faz `Logout()` se necessário e abre o mapa **`Lvl_TestAuth`**.

### Fluxo resumido

```text
[Novo client]  POST login.php  →  session_version++
[Novo client]  WS connect  →  opcode 109 (JWT)
[Zone]         valida JWT + session_version
[Zone]         envia 110 ao client antigo  →  disconnect antigo
[Novo client]  opcode PlayerInfoUpdate  →  jogo normal

[Client antigo] recebe 110  →  NotifyZoneDisconnected  →  diálogo  →  ReturnToLoginLevel
```

---

## 2) Pré-requisitos e ordem de deploy

| Ordem | Componente | Obrigatório |
|------:|------------|-------------|
| 1 | MySQL: coluna `session_version` | Sim |
| 2 | PHP: `login.php` + `jwt_helper.php` (api/common) | Sim |
| 3 | `config/server.json`: `auth.jwt_secret` alinhado com PHP | Sim |
| 4 | Rebuild `zone_server.exe` | Sim |
| 5 | Rebuild cliente UE (C++ Source) | Sim |
| 6 | Blueprint: `WBP_ServerDisconnect` + `BP_UmbraGameInstance` | Sim (senão não há diálogo nem retorno ao login) |

**Breaking change:** clientes UE **sem** opcode 109 são desconectados após ~10 s. Deploy **servidor + UE juntos**.

**Game Instance configurado em** `UmbraEternumUE/Config/DefaultEngine.ini`:

```ini
GameInstanceClass=/Game/Blueprints/BP_UmbraGameInstance.BP_UmbraGameInstance_C
GameDefaultMap=/Game/Maps/Lvl_TestAuth.Lvl_TestAuth
```

---

## 3) Passo a passo — Backend (MySQL, PHP, C++)

### 3.1 MySQL — migration

1. Abra **MySQL Workbench** (ou cliente SQL).
2. Selecione o banco `umbra_eternum`.
3. Execute o script:

   `www/umbra_api/scripts/add_account_session_version.sql`

4. Confirme:

```sql
SHOW COLUMNS FROM accounts LIKE 'session_version';
```

   Deve aparecer: `session_version INT UNSIGNED NOT NULL DEFAULT 0`.

> Se der erro de sintaxe com versões antigas do MySQL, use o script atualizado (usa `INFORMATION_SCHEMA`) ou rode direto:
>
> ```sql
> ALTER TABLE accounts ADD COLUMN session_version INT UNSIGNED NOT NULL DEFAULT 0;
> ```

### 3.2 JWT — chave secreta alinhada

A chave deve ser **a mesma** em:

| Arquivo | Campo |
|---------|--------|
| `config/server.json` | `auth.jwt_secret` |
| `www/umbra_api/api/common/jwt_helper.php` | fallback / `JWT_SECRET` |
| `www/umbra_api/helpers/jwt_helper.php` | fallback (validação nas APIs) |

Valor de desenvolvimento atual do projeto:

```text
umbra_eternum_secret_key_2024_very_secure
```

Após alterar `server.json`, **reinicie** `zone_server` (e Auth/Gateway se usar separados).

### 3.3 PHP — o que já está implementado

| Arquivo | Comportamento |
|---------|---------------|
| `api/login.php` | `session_version + 1` no login; JWT com claim |
| `api/common/jwt_helper.php` | `generateJWT(..., $sessionVersion)` |
| `api/character/select_character.php` | Regenera token **sem** incrementar `session_version` |
| `helpers/jwt_helper.php` | Só **valida** tokens (80+ endpoints); compatível com claim no payload |

### 3.4 Servidor C++ — rebuild zone

No Windows (PowerShell):

```powershell
cd D:\UmbraServerV2\build
cmake --build . --config Release --target zone_server
```

Binário: `build\bin\Release\zone_server.exe`

Reinicie o processo da zone após o build.

---

## 4) Passo a passo — Compilar o cliente UE

1. Feche o editor Unreal (se estiver aberto).
2. Compile o módulo C++ (Visual Studio ou botão **Compile** no editor).
3. Arquivos relevantes já alterados no Source:
   - `UmbraGameInstance.h/.cpp` — auth WS, disconnect UX
   - `NetMovementClient.cpp` — envia 109, parse 110
   - `WSBinaryBPFL.h/.cpp` — encode/decode 109/110
4. Abra o projeto `UmbraEternumUE.uproject` na **5.6.1**.
5. Se pedir rebuild, confirme.

**Verificação rápida no editor:** em **Project Settings → Maps & Modes**, confirme `Game Instance Class` = `BP_UmbraGameInstance`.

---

## 5) Passo a passo — Unreal: WBP_ServerDisconnect

Widget modal exibido quando o servidor encerra ou perde a sessão WS.

### 5.1 Criar o asset

1. **Content Browser** → pasta sugerida: `Content/UI/` (ou `Content/Widgets/`).
2. Clique direito → **User Interface → Widget Blueprint**.
3. Nome: **`WBP_ServerDisconnect`**.
4. **Parent Class:** `User Widget` (padrão).
5. Abra o designer.

### 5.2 Hierarquia visual (Designer)

Estrutura recomendada (nomes livres, mas use estes para facilitar o Graph):

```text
[Canvas Panel]  (root — preenche tela)
└── [Overlay]  ou [Canvas Panel] centralizado
    └── [Border]  Border_Backdrop
        └── [Vertical Box]  VB_Main
            ├── [Text Block]  TXT_Title
            ├── [Text Block]  TXT_Message
            └── [Button]      BTN_OK
```

#### Propriedades sugeridas

| Widget | Nome | Ajustes |
|--------|------|---------|
| Canvas root | — | Anchors: full screen (0,0 → 1,1) |
| Border_Backdrop | fundo escuro semi-transparente | Brush Color: preto ~70% alpha; Padding: 24 |
| TXT_Title | título fixo | Text: `Conexão encerrada`; Font maior; Justification: Center |
| TXT_Message | mensagem dinâmica | Text: `(placeholder)`; Wrap Text; Justification: Center; min height ~80 |
| BTN_OK | botão confirmar | Text: `OK`; min width 120, height 40 |

#### Centralizar o painel

- Selecione `Border_Backdrop` (ou `VB_Main`).
- No **Canvas Panel Slot**: Anchor no centro; Alignment 0.5, 0.5; Position 0,0.
- Tamanho fixo sugerido: **480 × 220** (ajuste ao seu estilo).

#### Input mode (importante)

No **Graph** do widget (ver 5.3), ao aparecer na tela:

- `Set Input Mode UI Only` (Player Controller)
- `Set Show Mouse Cursor` = **true**

Ao fechar (botão OK), o `ReturnToLoginLevel` no GameInstance troca de mapa — não precisa restaurar input manualmente.

### 5.3 Variables no WBP_ServerDisconnect

1. Aba **Graph** → **My Blueprint → Variables**.
2. Crie:

| Nome | Tipo | Expose on Spawn | Descrição |
|------|------|-----------------|-----------|
| `MessageText` | String | **Sim** | Texto vindo do servidor/C++ |

### 5.4 Event Graph — WBP_ServerDisconnect

#### Event Construct

```text
Event Construct
  → Branch (MessageText Is Empty?)
       False → TXT_Message → Set Text (MessageText)
       True  → TXT_Message → Set Text ("Conexão com o servidor perdida.")
  → Get Player Controller (index 0)
  → Set Input Mode UI Only
  → Set Show Mouse Cursor (true)
```

#### BTN_OK — OnClicked

O botão **não** chama `OpenLevel` diretamente. Ele avisa o GameInstance e se remove:

```text
BTN_OK → OnClicked
  → Get Game Instance → Cast to UmbraGameInstance
  → Return to Login Level   (função C++ BlueprintCallable)
  → Remove from Parent       (self)
```

> **Por quê?** `ReturnToLoginLevel()` já faz `DisconnectFromGatewayTCP`, `Logout()` quando aplicável e `OpenLevel("Lvl_TestAuth")`.

#### (Opcional) Função SetupMessage

Se preferir configurar a mensagem depois do Create Widget:

1. Crie função **`SetupMessage`** (Input: `Message` String).
2. Corpo: `TXT_Message → Set Text (Message)`.

---

## 6) Passo a passo — Unreal: BP_UmbraGameInstance

O C++ expõe um **Blueprint Implementable Event**. Você **precisa** implementá-lo no filho Blueprint.

**Asset:** `Content/Blueprints/BP_UmbraGameInstance`  
**Parent Class:** `UmbraGameInstance` (C++)

### 6.1 Abrir o evento

1. Abra **`BP_UmbraGameInstance`**.
2. Aba **Graph**.
3. No painel **My Blueprint → Functions**, procure **`ShowServerDisconnectDialog`** (evento implementável, ícone roxo).
4. Se não aparecer: clique direito no Graph → **Add Event for UmbraGameInstance → Show Server Disconnect Dialog**.

Parâmetro de entrada: **`Message`** (String).

### 6.2 Implementação nó a nó

```text
Event ShowServerDisconnectDialog (Message)
  │
  ├─ Create Widget
  │    Class: WBP_ServerDisconnect
  │    Owning Player: Get Player Controller (0)
  │    Message Text: Message          ← pin "Expose on Spawn" se criou a variable
  │
  ├─ (alternativa se usou SetupMessage)
  │    → Call SetupMessage on Return Value (Message)
  │
  ├─ Add to Viewport
  │    ZOrder: 9999                   ← acima de HUD/chat
  │
  └─ (opcional) Set Input Mode UI Only + Show Mouse Cursor
       se não fez isso dentro do WBP
```

#### Detalhe do Create Widget (UE 5.6.1)

1. Arraste **Create Widget** para o Graph.
2. **Class** = `WBP_ServerDisconnect`.
3. Se marcou **Expose on Spawn** em `MessageText`:
   - O pin **Message Text** aparece no nó Create Widget.
   - Ligue **Message** (do evento) nesse pin.
4. **Return Value** → **Add to Viewport**.

### 6.3 (Opcional) Delegate OnZoneDisconnected

Se quiser lógica extra (log, som, analytics):

```text
Event BeginPlay (ou Init customizado)
  → Get Game Instance (self)
  → Bind Event to OnZoneDisconnected
       → Custom Event On Zone Disconnected (Reason, Message)
            → (opcional) mesma lógica do diálogo, ou só Print String
```

> **Não duplique** o diálogo se já implementou `ShowServerDisconnectDialog` — o C++ já chama os dois (`Broadcast` + `ShowServerDisconnectDialog`).

### 6.4 Funções C++ disponíveis no Blueprint

| Função | Quando usar |
|--------|-------------|
| `ShowServerDisconnectDialog` | **Implementar** — exibe UI |
| `ReturnToLoginLevel` | Botão OK do modal |
| `NotifyZoneDisconnected` | Só debug manual; o C++ já chama |
| `ResetZoneDisconnectNotification` | Automático no reconnect WS |
| `SendZoneSessionAuth` | Automático no `NetMovementClient` |

### 6.5 Motivos de desconexão (`EUmbraZoneDisconnectReason`)

| Valor | Enum | Quando ocorre |
|------:|------|---------------|
| 0 | Generic | Motivo genérico |
| 1 | DuplicateLogin | Opcode 110 reason=1 — outro client logou |
| 2 | ConnectionLost | WS fechou ou erro de rede |
| 3 | AuthFailed | Token inválido / session_version / timeout auth |
| 4 | ServerKick | Outros kicks do servidor |

`ReturnToLoginLevel` faz **`Logout()`** automaticamente para: DuplicateLogin, AuthFailed, ServerKick.

---

## 7) Passo a passo — Parar timers no personagem (opcional)

Se o personagem continua enviando movimento/skills após disconnect:

1. No Blueprint do personagem ou do **NetMovementClient** (onde já existe **`OnWSClosed`**):
2. No evento **`OnWSClosed`** (custom event do `ANetMovementClient`):
   - **Clear Timer by Handle** (timer de movimento)
   - **Clear Timer by Handle** (timer de skill/heartbeat local)
   - Parar animações de combate se necessário

Referência: `docs_main/PROCEDIMENTO_MOVIMENTO_WEBSOCKET_BINARIO.md` (seção OnWSClosed).

> O C++ do `NetMovementClient` já limpa party, sessão social e remote actors no close. Timers **locais do BP** ainda precisam ser parados manualmente se existirem.

---

## 8) Testes integrados

### Checklist

| # | Cenário | Passos | Resultado esperado |
|---|---------|--------|-------------------|
| 1 | Auth OK | Login → select char → entrar na zone | Log: `SessionAuthNotify (109) enviado`; jogo normal |
| 2 | Login duplicado | Client 1 na zone; Client 2 mesma conta login + zone | Client 1: modal *"Sua conta entrou em outro cliente."* → OK → `Lvl_TestAuth`; Client 2 joga |
| 3 | Queda de rede | Matar `zone_server` com client na zone | Modal *"Conexão com o servidor perdida."* → OK → login map |
| 4 | Token antigo | Copiar JWT antigo (session_version menor) e tentar reconectar | Opcode 110 reason=3; modal auth; logout ao OK |
| 5 | Select character | Client 1 na zone; Client 1 faz select char (mesma conta) | **Não** derruba sessão (session_version preservado) |
| 6 | Client sem BP | Remover implementação de `ShowServerDisconnectDialog` | Disconnect ocorre mas **sem UI** (só log) — confirma necessidade do WBP |

### Logs úteis (Output Log UE)

```text
[UmbraGameInstance] SessionAuthNotify (109) enviado
[UmbraGameInstance] Zone desconectada: reason=1 msg=Sua conta entrou em outro cliente.
[UmbraGameInstance] Abrindo mapa de login Lvl_TestAuth
```

### Logs zone (servidor)

```text
WS client X autenticado account=Y session_version=Z
WS client X auth rejeitada: ...
```

---

## 9) Troubleshooting

| Problema | Causa provável | Solução |
|----------|----------------|---------|
| Desconecta após ~10 s na zone | Cliente não envia opcode 109 | Recompile UE; confirme `SendZoneSessionAuth` no connect |
| Auth falha sempre | JWT secret diferente PHP vs C++ | Alinhar `server.json` e `jwt_helper.php`; reiniciar zone |
| Auth falha após login | Coluna `session_version` ausente | Rodar migration SQL |
| Modal não aparece | `ShowServerDisconnectDialog` não implementado no BP | Seguir seção 6 |
| OK não volta ao login | Botão não chama `ReturnToLoginLevel` | Corrigir Graph do WBP |
| Login duplicado não kicka | `session_version` não incrementa | Verificar `login.php` + coluna no DB |
| Select char derruba sessão | Token regenerado sem preservar sv | Verificar `select_character.php` usa `api/common/jwt_helper.php` |

---

## 10) Referência técnica (opcodes e arquivos)

### Opcodes WebSocket binários

| Opcode | Nome | Direção | Payload |
|-------:|------|---------|---------|
| 109 | SessionAuthNotify | Cliente → Zone | `[type:1][tokenLen:2 LE][token:utf8]` |
| 110 | SessionRevokedNotify | Zone → Cliente | `[type:1][reason:1][msgLen:2 LE][msg:utf8]` |

**Reason (110):**

| reason | Significado |
|-------:|-------------|
| 0 | Genérico |
| 1 | DuplicateLogin |
| 2 | AuthTimeout |
| 3 | InvalidToken |

### Ordem no connect WS (cliente)

```text
1. WS Connected
2. SendZoneSessionAuth (109)
3. TrySendPlayerInfoUpdateOnConnect
4. party / invites / ReportSessionOnline / heartbeat
```

### Arquivos principais

| Camada | Arquivos |
|--------|----------|
| DB/PHP | `scripts/add_account_session_version.sql`, `api/login.php`, `api/character/select_character.php`, `api/common/jwt_helper.php` |
| Servidor | `MovementProtocol.hpp`, `MovementSessionAuth.hpp`, `MovementServer.hpp`, `ZoneServer.cpp`, `JWTManager.hpp/cpp` |
| Cliente UE | `WSBinaryBPFL.*`, `NetMovementClient.cpp`, `UmbraGameInstance.*` |
| Blueprint | `BP_UmbraGameInstance`, `WBP_ServerDisconnect` |
| Config | `config/server.json`, `Config/DefaultEngine.ini` |

---

## Resumo executivo (checklist final)

- [ ] SQL `session_version` aplicado no MySQL
- [ ] `auth.jwt_secret` alinhado PHP + `server.json`
- [ ] `zone_server` recompilado e reiniciado
- [ ] Cliente UE C++ recompilado
- [ ] `WBP_ServerDisconnect` criado (layout + BTN_OK → `ReturnToLoginLevel`)
- [ ] `BP_UmbraGameInstance`: evento `ShowServerDisconnectDialog` implementado
- [ ] Teste login duplicado (2 clients)
- [ ] Teste queda de zone (kill server)
- [ ] Timers locais parados no `OnWSClosed` (se aplicável)
