# GUIA — Limpeza de log/tela e correção de party (UE 5.6.1)

Rodada 1 do `docs_main/PLANO_RODADAS_UI_UE561.md`. O que esta rodada mudou:

| Área | Mudança |
|---|---|
| Log de movimento | O log de yaw de cada frame virou `VeryVerbose` |
| Tela | Os `Print String` de debug saíram dos BPs de gameplay |
| Party | O cliente ignora avisos de grupo que não são dele; o PHP desfaz o grupo que fica com 1 membro |
| Conteúdo | A pasta `Content/Necropolis` foi restaurada só no disco |

Nenhum WBP ganhou lógica nova.

## Objetivo

- O Output Log para de receber ~110 mil linhas de `[YawSend]`/`[YawRecv]` por sessão.
- Nenhum texto de debug azul na tela (`SavePositionTimer - ...`, `WBP_STORAGE - ...`, `TRADE REQUEST CALLED ...`).
- Party:
  - quem sai não recebe mais `403 Você não pertence a este grupo`;
  - jogadores fora do grupo não carregam o grupo dos outros;
  - quem fica sozinho sai do grupo e o painel some.
- **Protocolo:** nenhum opcode novo ou alterado.
  - Os opcodes **13**, **14** e **27** continuam com o mesmo formato. Só mudou como o cliente reage a eles.

---

## 0) Pré-requisitos

1. **Fechar o editor** e fazer **rebuild** de `UmbraEternumUEEditor` (Development Editor): `UmbraGameInstance.cpp` e `WSBinaryBPFL.cpp` mudaram.
2. PHP sincronizado com o WAMP: `scripts_main\sync_umbra_api_to_wamp.bat` (ou `/sync-wamp`), por causa do `leave_party.php`.
3. Reabrir o editor. O `Lvl_Tutorial_` deve carregar os LevelInstances da Necropolis sem `Failed to load LevelInstance`.

## 1) Assets alterados via MCP (`Print String` removidos)

O fluxo de execução foi religado (o que entrava no `Print String` passa direto para o nó seguinte). Os nós que só montavam o texto (`To String`, `Append`, `Format Text`) também saíram.

| Asset | Removidos | Mantidos (feedback ao jogador) |
|---|---|---|
| `/Game/Blueprints/BP_NetMovementClient2` | 34 | — |
| `/Game/Blueprints/Player_BP/BP_ThirdPersonCharacter` | 22 | — |
| `/Game/Widgets/UI/PlayerInteraction/PlayerSelection/WBP_PlayerContextMenu` | 7 | — |
| `/Game/Widgets/UI/Storage/WBP_Storage` | 7 | 2 (falha ao carregar storage, falha de ouro) |
| `/Game/Widgets/UI/CharacterInfo/WBP_CharacterInfo1` | 6 | — |
| `/Game/Widgets/UI/CharacterInfo/WBP_EquipmentSlot` | 3 | — |
| `/Game/Widgets/UI/Inventory/WBP_Inventory` | 1 | 2 (falha de stack, falha de ouro) |
| `/Game/Widgets/UI/PlayerInteraction/Trade/WBP_Trade` | 1 | 1 (falha de ação do trade) |
| `WBP_Levelup`, `WBP_SplitInput`, `WBP_SkillSlot`, `WBP_SkillSlot1`, `WBP_SkillBookEntry` | 1 cada | — |
| `WBP_InventorySlot` | — | 1 ("Item não pode ser dividido") |
| `WBP_SkillBook`, `WBP_SkillDetails` | — | 1 cada (falha ao carregar skills) |

**Mantidos de propósito:** são a única mensagem de erro que o jogador vê. Viram texto de UI quando cada janela for para C++ (Rodadas 2 e 4).

**Fora desta rodada:** telas de login, cadastro, seleção e criação de personagem (Rodada 4).

**Não usado:** `BP_Player` não tem referenciadores. Fica para a limpeza de assets mortos (R4.3).

## 2) O que o C++ e o PHP já fazem

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Log de yaw | `Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp` | `[YawSend]`/`[YawRecv]` em `VeryVerbose`. Para ver, use o console: `log LogTemp VeryVerbose` |
| Aviso 13 (`PartyMemberJoined`) | `Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` | Sem grupo local: chama `LoadPartyState(0)`, e o servidor resolve "meu grupo" pelo token (sem 403). Mesmo grupo: recarrega. Outro grupo: ignora |
| Avisos 14 (`PartyMemberLeft`) e 27 (`PartyStatsRefresh`) | idem | Ignora quando `PartyID != CurrentPartyID` (grupo alheio ou grupo que acabou de deixar) |
| Resposta do `get_party_state` | idem (`OnLoadPartyStateComplete`) | `CurrentPartyID` vem só da resposta do servidor; com `in_party=true`, garante o painel (`EnsurePartyPanelToViewport`) |
| `leave_party.php` | `www/umbra_api/api/social/leave_party.php` | Se sobrar 1 membro (ou menos), apaga o membro restante e o grupo. Erro 500 responde "Erro interno" (sem a mensagem da exceção) |

> Por que o cliente filtra: a zone faz **broadcast para todos os clientes** dos avisos de party (`MovementServer.hpp`). Mandar só para os membros é a R2.5.

> **Não** adicione lógica no Event Graph desses BPs.

## 3) Conteúdo — Necropolis

- `Content/Necropolis` (950 arquivos, 2,19 GB) foi restaurada do commit `11ef18f` (branch local `backup/mega-commit-11ef18f`) **só no disco**: `git restore --source=11ef18f --worktree -- Content/Necropolis`.
- **Não é versionada**:
  - passa do limite de 2 GB por push do GitHub e o repo não usa LFS;
  - segue o mesmo padrão dos outros pacotes pesados (Synty, Polyart etc.).
- **Se sumir de novo:** com o editor fechado, rode o mesmo comando no submódulo.

---

## 6) QA (PIE)

PIE com **Number of Players = 3** para os cenários de party.

| Cenário | Esperado |
|---------|----------|
| Andar 1 minuto | Output Log sem `[YawSend]`/`[YawRecv]` |
| Jogar normalmente (inventário, storage, menu de contexto, ficha) | Nenhum texto de debug na tela |
| Erro real (ex.: dividir item que não divide) | A mensagem de erro continua aparecendo |
| P1 convida P2, P2 aceita; P3 fora do grupo | P3 sem painel de party e sem 403 no log |
| P2 sai do grupo (grupo de 2) | P2 sem 403; P1 fica sem grupo e o painel some |
| Grupo de 3, P3 sai | P1 e P2 continuam no grupo; o painel atualiza para 2 |
| Grupo de 2, P2 desconecta | P1 recarrega o grupo e o painel some |
| Abrir o `Lvl_Tutorial_` | Sem `Failed to load LevelInstance` no log |

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Texto de debug ainda na tela | Asset fora desta rodada (login/criação de personagem) ou BP novo | Conferir na tabela da seção 1; os de login entram na R4.1 |
| Painel de party não aparece para quem entrou | `get_party_state.php` não sincronizado ou resposta com `in_party=false` | Rodar `/sync-wamp`; conferir a tabela `party_members` |
| Quem ficou sozinho continua no grupo | `leave_party.php` antigo no WAMP | Rodar `/sync-wamp` |
| `Failed to load LevelInstance ... /Game/Necropolis/...` | Pasta Necropolis ausente | Restaurar com o comando da seção 3 (editor fechado) |
| Aviso `'SavePlayerPosition' ... timer never runs` no `BP_NetMovementClient2` | Já existia: o timer aponta para uma função que esse BP não tem | Sem efeito no jogo; o salvamento de posição roda no `BP_ThirdPersonCharacter` |

## 8) Referência de código

- Cliente:
  - `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` (opcodes 13/14/27, `OnLoadPartyStateComplete`);
  - `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`.
- PHP: `www/umbra_api/api/social/leave_party.php`.
- Servidor (sem mudança): `src/zone/MovementServer.hpp` (broadcast dos avisos de party).

**Fim do guia.**
