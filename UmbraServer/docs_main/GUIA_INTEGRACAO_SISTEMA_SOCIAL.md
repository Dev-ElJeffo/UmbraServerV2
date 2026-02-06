# Guia: Integração do Sistema Social - APIs e WebSocket

Este guia explica como integrar o sistema social completo, incluindo APIs PHP e handlers WebSocket para eventos em tempo real.

## Índice
1. [Visão Geral](#visão-geral)
2. [Setup do Banco de Dados](#setup-do-banco-de-dados)
3. [APIs PHP Criadas](#apis-php-criadas)
4. [Handlers WebSocket (C++)](#handlers-websocket-c)
5. [Integração no Cliente (C++/Blueprint)](#integração-no-cliente-cblueprint)
6. [Fluxo Completo de Exemplo](#fluxo-completo-de-exemplo)

---

## Visão Geral

O sistema social funciona em **duas camadas**:

1. **APIs HTTP (PHP)**: Persistem dados no banco (convites, solicitações, etc.)
2. **WebSocket (C++)**: Notificações em tempo real entre jogadores

### Arquitetura

```
┌─────────────────────────────────────────────────────────────┐
│                    Cliente (UE5)                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │         UmbraGameInstance                             │  │
│  │  - SendPartyInvite() → HTTP API                      │  │
│  │  - SendPartyInviteViaWebSocket() → WebSocket         │  │
│  │  - ProcessSocialWebSocketMessage()                    │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┴─────────────────┐
        │                                     │
        ▼                                     ▼
┌──────────────────┐              ┌──────────────────────┐
│  PHP APIs        │              │  Zone Server (C++)    │
│  - send_party_   │              │  - MovementServer     │
│    invite.php    │              │  - Processa mensagens │
│  - accept_party_ │              │    sociais WebSocket │
│    invite.php    │              │  - Encaminha para    │
│  - etc.          │              │    jogadores online  │
└──────────────────┘              └──────────────────────┘
        │                                     │
        └──────────────┬──────────────────────┘
                       ▼
              ┌─────────────────┐
              │  MySQL Database │
              │  - party_invites │
              │  - trade_requests│
              │  - friend_requests│
              │  - etc.          │
              └─────────────────┘
```

---

## Setup do Banco de Dados

### 1. Executar Script SQL

Execute o script para criar todas as tabelas necessárias:

```sql
-- No MySQL Workbench ou phpMyAdmin
SOURCE d:/UmbraServerV2/www/umbra_api/scripts/create_social_tables.sql;
```

Ou copie e cole o conteúdo do arquivo `create_social_tables.sql`.

### 2. Verificar Tabelas Criadas

```sql
SHOW TABLES LIKE '%party%';
SHOW TABLES LIKE '%trade%';
SHOW TABLES LIKE '%friend%';
SHOW TABLES LIKE '%block%';
SHOW TABLES LIKE '%report%';
SHOW TABLES LIKE '%duel%';
```

Deve retornar:
- `party_invites`
- `parties`
- `party_members`
- `trade_requests`
- `trade_sessions`
- `trade_items`
- `friend_requests`
- `friends`
- `blocked_players`
- `player_reports`
- `duel_requests`

### 3. Configurar Limpeza Automática (Opcional)

Configure um evento MySQL para limpar convites expirados:

```sql
CREATE EVENT IF NOT EXISTS CleanExpiredSocialRequests
ON SCHEDULE EVERY 1 HOUR
DO
  CALL CleanExpiredSocialRequests();
```

---

## APIs PHP Criadas

### Endpoints Disponíveis

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/api/social/send_party_invite.php` | POST | Envia convite de grupo |
| `/api/social/accept_party_invite.php` | POST | Aceita convite de grupo |
| `/api/social/send_trade_request.php` | POST | Solicita troca |
| `/api/social/accept_trade_request.php` | POST | Aceita troca |
| `/api/social/send_friend_request.php` | POST | Solicita amizade |
| `/api/social/accept_friend_request.php` | POST | Aceita amizade |
| `/api/social/get_pending_invites.php` | GET | Lista convites pendentes |
| `/api/social/get_friend_list.php` | GET | Lista de amigos |
| `/api/social/remove_friend.php` | POST | Remove amigo |
| `/api/social/block_player.php` | POST | Bloqueia jogador |
| `/api/social/unblock_player.php` | POST | Desbloqueia jogador |
| `/api/social/report_player.php` | POST | Denuncia jogador |
| `/api/character/get_public_info.php` | GET | Informações públicas (Inspect) |

### Exemplo de Uso (HTTP)

```cpp
// No GameInstance, já implementado:
SendPartyInvite(123, TEXT("PlayerName"));
// → Chama /api/social/send_party_invite.php
// → Persiste no banco
// → Dispara OnPartyInviteSent
```

---

## Handlers WebSocket (C++)

### Tipos de Mensagem Adicionados

No `MovementProtocol.hpp`, foram adicionados:

```cpp
enum class MovementMsgType : uint8_t {
  // ... tipos existentes ...
  PartyInvite = 10,           // Cliente -> Servidor
  PartyInviteReceived = 11,   // Servidor -> Cliente
  TradeRequest = 20,
  TradeRequestReceived = 21,
  FriendRequest = 30,
  FriendRequestReceived = 31,
  WhisperMessage = 40,
  WhisperReceived = 41,
  // ... etc
};
```

### Processamento no MovementServer

O `MovementServer` agora processa mensagens sociais no callback `setBinaryCallback`:

```cpp
// Party Invite
if (msgType == MovementMsgType::PartyInvite) {
  // Decodifica mensagem
  // Encontra cliente do jogador alvo
  // Envia PartyInviteReceived
}
```

---

## Integração no Cliente (C++/Blueprint)

### 1. Conectar WebSocket ao GameInstance

No Blueprint do `BP_NetMovementClient`, no evento `OnWSBinaryMessage`:

#### Opção A: Processar Todas as Mensagens

```
[OnWSBinaryMessage Event]
    │
    ├──► Get Game Instance → Cast to UmbraGameInstance
    │         │
    │         └──► Process Social WebSocket Message (Data)
    │                   │
    │                   └──► [Is Social Message?] → Branch
    │                             │
    │                             ├── TRUE → [Já processado pelo GameInstance]
    │                             └── FALSE → [Processar como mensagem de movimento]
```

#### Opção B: Filtrar por Tipo (Mais Eficiente)

```
[OnWSBinaryMessage Event]
    │
    ├──► [Get Array Element] → Data[0] (msgType)
    │         │
    │         └──► [Switch on Integer]
    │                   │
    │                   ├── 10, 11, 12, 13, 14, 15 → [Social - Party]
    │                   │         │
    │                   │         └──► Get Game Instance → Process Social WebSocket Message
    │                   │
    │                   ├── 20, 21, 22, 23, 24 → [Social - Trade]
    │                   │         │
    │                   │         └──► Get Game Instance → Process Social WebSocket Message
    │                   │
    │                   ├── 30, 31, 32, 33 → [Social - Friend]
    │                   │         │
    │                   │         └──► Get Game Instance → Process Social WebSocket Message
    │                   │
    │                   ├── 40, 41 → [Social - Whisper]
    │                   │         │
    │                   │         └──► Get Game Instance → Process Social WebSocket Message
    │                   │
    │                   └── 1, 2, 3, 4 → [Movement Messages]
    │                             │
    │                             └──► [Processar normalmente]
```

### 2. Enviar Mensagens WebSocket

Quando uma API HTTP retornar sucesso, você pode também enviar via WebSocket para notificação imediata:

#### Exemplo: Após Enviar Convite de Grupo

```
[OnSendPartyInviteComplete Event]
    │
    ├──► Get All Actors of Class (NetMovementClient)
    │         │
    │         └──► [For Each Loop]
    │                   │
    │                   ├──► Cast to NetMovementClient
    │                   │         │
    │                   │         └──► Get WebSocketRef
    │                   │                   │
    │                   │                   └──► [Is Valid?] → Branch
    │                   │                             │
    │                   │                             └──► TRUE
    │                   │                                       │
    │                   │                                       └──► Get Game Instance
    │                   │                                                 │
    │                   │                                                 └──► Send Party Invite Via WebSocket
    │                   │                                                       (TargetPlayerID, PartyID, WebSocketRef)
    │                   │
    │                   └──► [Break Loop]
```

**NOTA**: Por enquanto, as APIs HTTP já funcionam. O WebSocket é opcional para notificações em tempo real. Se o jogador alvo não estiver online, ele receberá o convite quando carregar seus convites pendentes via `LoadPendingInvitesOnLogin()`.

### 3. Carregar Convites ao Entrar no Jogo

No `Event BeginPlay` do seu HUD ou Level Blueprint:

```
[Event BeginPlay]
    │
    └──► Get Game Instance → Cast to UmbraGameInstance
              │
              └──► Load Pending Invites On Login
```

Isso carregará todos os convites pendentes e disparará os delegates apropriados.

### 4. Conectar Delegates para UI

```
[Bind Event to OnPartyInviteReceived]
    │
    └──► [CustomEvent: OnPartyInviteReceived(FromPlayerID, FromPlayerName)]
              │
              └──► [Mostrar notificação/UI de convite]

[Bind Event to OnTradeRequestReceived]
    │
    └──► [CustomEvent: OnTradeRequestReceived(FromPlayerID, FromPlayerName)]
              │
              └──► [Mostrar notificação/UI de solicitação de troca]

// ... similar para FriendRequest, WhisperReceived, etc.
```

---

## Fluxo Completo de Exemplo

### Exemplo: Enviar Convite de Grupo

#### 1. Cliente (Blueprint/C++)

```
[BTN_InviteParty OnClicked]
    │
    └──► Get Game Instance
              │
              └──► Send Party Invite (TargetPlayerID, TargetPlayerName)
```

#### 2. GameInstance (C++)

```cpp
SendPartyInvite(TargetPlayerID, TargetPlayerName)
  → Cria requisição HTTP
  → Chama /api/social/send_party_invite.php
  → OnSendPartyInviteComplete()
    → Dispara OnPartyInviteSent (para UI)
    → (Opcional) Envia também via WebSocket
```

#### 3. API PHP

```php
send_party_invite.php
  → Valida JWT
  → Verifica se jogador existe
  → Insere em party_invites
  → Retorna sucesso
```

#### 4. WebSocket (Opcional - Notificação Imediata)

```
[Após sucesso da API]
    │
    └──► SendPartyInviteViaWebSocket(TargetPlayerID, PartyID, WebSocketClient)
          → Codifica mensagem binária
          → Envia via WebSocket
```

#### 5. Zone Server (C++)

```
MovementServer recebe mensagem
  → Identifica tipo: PartyInvite (10)
  → Decodifica: fromPlayerId, toPlayerId, partyId
  → Encontra cliente do toPlayerId
  → Envia PartyInviteReceived (11) para o cliente alvo
```

#### 6. Cliente Alvo Recebe

```
[OnWSBinaryMessage no NetMovementClient]
    │
    └──► ProcessSocialWebSocketMessage(Data)
          → Identifica tipo: PartyInviteReceived (11)
          → Decodifica: fromPlayerId, partyId
          → Adiciona a PendingPartyInvites
          → Dispara OnPartyInviteReceived
```

#### 7. UI do Cliente Alvo

```
[OnPartyInviteReceived Event]
    │
    └──► [Mostrar notificação]
              │
              └──► [Botão Aceitar] → AcceptPartyInvite(FromPlayerID)
```

---

## Formato das Mensagens WebSocket

### Party Invite (Tipo 10)

```
[msgType:uint8=10][fromPlayerId:uint32][toPlayerId:uint32][partyId:uint32]
Total: 13 bytes
```

### Trade Request (Tipo 20)

```
[msgType:uint8=20][fromPlayerId:uint32][toPlayerId:uint32]
Total: 9 bytes
```

### Friend Request (Tipo 30)

```
[msgType:uint8=30][fromPlayerId:uint32][toPlayerId:uint32]
Total: 9 bytes
```

### Whisper Message (Tipo 40)

```
[msgType:uint8=40][fromPlayerId:uint32][toPlayerId:uint32][msgLen:uint16][message:bytes]
Total: 11 + msgLen bytes
```

---

## Resumo de Passos de Integração

1. ✅ **Executar SQL**: `create_social_tables.sql`
2. ✅ **Compilar C++**: Recompilar projeto (MovementProtocol, MovementServer, GameInstance)
3. ⬜ **Conectar WebSocket no Blueprint**: `BP_NetMovementClient` → `OnWSBinaryMessage` → `ProcessSocialWebSocketMessage`
4. ⬜ **Carregar Convites ao Login**: Chamar `LoadPendingInvitesOnLogin()` no `BeginPlay` do HUD/Level
5. ⬜ **Conectar Delegates**: Bind `OnPartyInviteReceived`, `OnTradeRequestReceived`, etc. para UI
6. ⬜ **Testar APIs HTTP**: Testar envio de convites via menu de contexto
7. ⬜ **Testar WebSocket**: Verificar se notificações chegam em tempo real

## Checklist de Implementação

### Backend (Servidor)
- [x] Script SQL criado
- [x] APIs PHP criadas (12 endpoints)
- [x] MovementProtocol atualizado (tipos de mensagem)
- [x] MovementServer atualizado (handlers WebSocket)

### Frontend (Cliente)
- [x] GameInstance atualizado (funções HTTP + WebSocket)
- [x] Handlers implementados (callbacks HTTP)
- [x] Processamento WebSocket implementado
- [ ] Integração Blueprint (NetMovementClient)
- [ ] UI de notificações (convites recebidos)
- [ ] UI de lista de amigos
- [ ] UI de troca (quando implementar sistema completo)

---

## Troubleshooting

### Convites não chegam em tempo real

1. Verifique se `ProcessSocialWebSocketMessage` está sendo chamado no `OnWSBinaryMessage`
2. Verifique logs do Zone Server para ver se a mensagem está sendo recebida
3. Verifique se o PlayerID do alvo está online (no `clientIdToPlayerId_`)

### APIs retornam erro 401

1. Verifique se o token JWT está sendo enviado no header `Authorization: Bearer {token}`
2. Verifique se o token não expirou

### Mensagens WebSocket não são processadas

1. Verifique se o tipo de mensagem está correto (10, 20, 30, 40, etc.)
2. Verifique se o tamanho da mensagem está correto
3. Adicione logs temporários em `ProcessSocialWebSocketMessage`

---

## Próximos Passos

Após integrar este sistema, você terá:

1. ✅ **Sistema de Grupo** completo (convites, membros, dissolução)
2. ✅ **Sistema de Troca** (solicitações, sessões, itens)
3. ✅ **Sistema de Amigos** (solicitações, lista, remoção)
4. ✅ **Whisper/Chat Privado** (mensagens em tempo real)
5. ✅ **Bloqueio/Denúncia** (moderação básica)
6. ✅ **Inspeção de Jogadores** (informações públicas)

Tudo funcionando com **persistência no banco** (APIs HTTP) e **notificações em tempo real** (WebSocket)!
