# GUIA — Fase 7: HUD, alvo, escala da interface e chat (UE 5.6.1)

Guia dos lotes da fase 7 ([plano](PLANO_UI_FASE7_HUD_UE561.md)). Cada lote acrescenta uma seção. O C++ resolve a lógica; o Blueprint só define parent class e layout.

## 0) Pré-requisitos

1. `zone_server` recompilado e reiniciado (o K1 mexe no protocolo).
2. Rebuild do módulo `UmbraEternumUE` (Live Coding não basta quando há header novo).
3. PIE com 2 clients (P1 e P2 no mesmo mapa).

---

## 1) K1 — Nomes nos pedidos e remover amigo

### Objetivo

- "Negociar com Player_32?", "Adicionar Player_32 à lista de amigos?" e o convite de grupo passam a mostrar o **nome do personagem**.
- "Remover Fulano da lista de amigos?" passa a mostrar o nome do amigo.
- Nada muda no Editor: não há WBP para editar neste lote.

### Protocolo

Campo novo **trailing** (frame antigo continua válido nos dois sentidos):

| Opcode | Nome | Frame |
|--------|------|-------|
| **11** | `PartyInviteReceived` | `[11][fromId:u32][toId:u32][partyId:u32][nameLen:u16][name:utf8]` |
| **21** | `TradeRequestReceived` | `[21][fromId:u32][toId:u32][nameLen:u16][name:utf8]` |
| **31** | `FriendRequestReceived` | `[31][fromId:u32][toId:u32][nameLen:u16][name:utf8]` |

- Little-endian; nome com até **32 bytes** UTF-8 (cortado sem partir caractere).
- Servidor: `appendSocialSenderName` em `src/zone/MovementProtocol.hpp`; nome vindo de `players_[id].characterName`.
- O `fromId` enviado agora é o **player da conexão** (`socialSenderUnlocked` em `MovementServer.hpp`). Se o cliente mandar outro id, o servidor usa o da conexão e grava um warning: "Pedido social com fromId X diferente do player da conexao Y".
- Cliente: `UUmbraGameInstance::ResolveSocialSenderName` lê o nome trailing; sem ele (servidor antigo), usa o cache de nomes próximos e, por último, "Jogador #id".

### O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Nome do remetente | `Core/UmbraGameInstance.cpp` (opcodes 11/21/31) | Preenche `PendingPartyInvites` / `PendingTradeRequests` / `PendingFriendRequests` e os delegates com o nome real |
| Remover amigo | `UI/UmbraFriendOptionsWidget.cpp` | **Remover** abre a confirmação vermelha padrão ("Remover X da lista de amigos?"); Confirmar/Enter chama `RemoveFriend`; Cancelar/ESC não faz nada |

> O delegate `OnRemoveFriendRequested` não é mais disparado, então o `WBP_FriendRemoveConfirm` antigo deixa de abrir. Não precisa apagar nada no BP.

### QA (PIE, 2 players)

| # | Teste | Esperado |
|---|-------|----------|
| 1 | P2 pede troca a P1 | P1 vê "Negociar com <nome do P2>?" |
| 2 | P2 pede amizade a P1 | "Adicionar <nome do P2> à lista de amigos?" |
| 3 | P2 convida P1 para o grupo | Convite com o nome do P2 |
| 4 | Aceitar a troca / a amizade / o grupo | Funciona como antes |
| 5 | Amigos → clicar no amigo → **Remover** | Confirmação vermelha "Remover <nome> da lista de amigos?"; **Cancelar** não remove; **Remover** remove e a lista atualiza |
| 6 | Servidor antigo (opcional) com cliente novo | Mostra o nome se o P2 estiver por perto; senão "Jogador #id" |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Ainda aparece "Player_32" | `zone_server` antigo ou cliente sem rebuild | Recompilar e reiniciar a zone; rebuild do UE |
| Aparece "Jogador #32" | Zone antiga e o P2 longe (sem nome no cache) | Atualizar a zone |
| Abrem dois diálogos ao remover amigo | BP ainda chama algo além do delegate | Avisar: verificar o Graph do `WBP_FriendList` |
| Warning "Pedido social com fromId..." no log | Cliente mandou id diferente do da conexão | Esperado só com cliente modificado; investigar se aparecer em jogo normal |
