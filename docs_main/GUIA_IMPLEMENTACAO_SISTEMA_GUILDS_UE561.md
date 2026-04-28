# Guia de Implementação: Sistema de Guilds (UE 5.6.1)

## Objetivo
Este guia descreve a estrutura do `WBP_GuildPanel` com lógica centralizada em C++.
No Blueprint, você só conecta eventos de UI para chamadas C++.

## Arquitetura
- **Autoridade:** PHP API + MySQL.
- **Tempo real:** Zone WebSocket (`GuildInviteReceived`, `GuildStateRefresh`, `GuildMemberUpdated`, `GuildMemberKicked`).
- **Cliente UE:** `UUmbraGameInstance` + `UUmbraGuildWidget`.

## Passo a passo completo (implementacao)

### 1) Banco de dados (ordem dos scripts)

#### Ambiente novo (do zero)
1. `scripts_main/setup_database.sql`
2. `www/umbra_api/scripts/create_character_info_tables.sql`
3. `www/umbra_api/scripts/add_gold_column.sql`
4. `www/umbra_api/scripts/create_social_tables.sql`
5. `www/umbra_api/scripts/add_guild_system_v2.sql`

#### Ambiente existente (ja em uso)
1. `www/umbra_api/scripts/add_gold_column.sql` (se `players.gold` ainda nao existir)
2. `www/umbra_api/scripts/create_social_tables.sql` (garante tabelas sociais faltantes)
3. `www/umbra_api/scripts/add_guild_system_v2.sql` (migracao de guild v2)

### 2) Como executar scripts no MySQL
- Via MySQL Workbench: abrir cada arquivo e executar em ordem.
- Via CLI:
  - `mysql -u root -p umbra_eternum < caminho_do_script.sql`
- Ou com `SOURCE`:
  - `USE umbra_eternum;`
  - `SOURCE D:/UmbraServerV2/www/umbra_api/scripts/add_guild_system_v2.sql;`

### 3) Erro comum de sintaxe (1064) e causa
- Erro: `near 'IF NOT EXISTS guild_tag ...'`
- Causa: servidor MySQL sem suporte a `ADD COLUMN IF NOT EXISTS` em `ALTER TABLE`.
- Solucao aplicada no projeto: script `add_guild_system_v2.sql` reescrito com:
  - `INFORMATION_SCHEMA`
  - `PROCEDURE`
  - SQL dinamico (`PREPARE/EXECUTE`)
  - idempotencia real (pode rodar mais de uma vez).

### 4) Validacao obrigatoria apos migracao
Rode estes comandos:

```sql
SHOW CREATE TABLE guilds;
SHOW CREATE TABLE guild_members;
SHOW CREATE TABLE guild_invites;

SELECT COLUMN_NAME
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME = 'current_guild_id';
```

Checklist esperado:
- `guilds` com `guild_tag`, `owner_player_id`, `guild_xp`, `guild_level`, `ranking_score`, `member_limit`, `member_count`.
- `guild_members` com `member_rank` e `contribution_xp`.
- `guild_invites` criada.
- `players.current_guild_id` presente.

### 5) Backend PHP (endpoints de guild)
Implementados em `www/umbra_api/api/social/`:
- `create_guild.php`
- `invite_guild_member.php`
- `accept_guild_invite.php`
- `decline_guild_invite.php`
- `kick_guild_member.php`
- `leave_guild.php`
- `set_guild_member_rank.php`
- `transfer_guild_master.php`
- `get_guild_state.php`
- `add_guild_xp.php`
- `get_guild_ranking.php`

Helpers:
- `www/umbra_api/helpers/guild_helper.php` (regras de permissao/rank)
- `www/umbra_api/helpers/character_info_helper.php` (payload de guild para cliente)

### 6) Servidor C++ (Zone / realtime)
- `src/zone/MovementProtocol.hpp`:
  - mensagens de guild no protocolo binario.
  - `PlayerInfoUpdate` expandido para incluir `GuildName`.
- `src/zone/MovementServer.hpp`:
  - roteamento de notificacoes de guild.
  - encaminhamento de convite de guild para alvo online.
  - broadcast de refresh de guild.

### 7) Cliente UE C++ (dados e fluxo)
- `UUmbraGameInstance`:
  - funcoes de guild (`CreateGuild`, `InviteGuildMember`, `AcceptGuildInvite`, etc).
  - parse de estado de guild.
  - eventos `OnGuildStateLoaded`, `OnGuildInviteReceived`, `OnGuildActionFailed`.
- `WSBinaryBPFL` / `NetMovementClient`:
  - decode/encode de `PlayerInfoUpdate` com guild.
  - recebimento de mensagens realtime de guild.
- `FUmbraGuildState` / `FUmbraGuildMember` em `UmbraDataStructures.h`.

### 8) UI UE (Blueprint chama C++)
- Parent C++ do painel: `UUmbraGuildWidget`.
- Parent C++ da row de membro: `UUmbraGuildMemberRowWidget`.
- Blueprint `WBP_GuildPanel` deve:
  - apenas expor widgets com os nomes esperados (`BindWidgetOptional`).
  - configurar no Details a classe `GuildMemberRowWidgetClass` (ex.: `WBP_GuildMemberRow`).
  - deixar a logica de bind, refresh e visibilidade no C++.

## Arquivos C++ principais
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h`
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildWidget.h`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildWidget.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildMemberRowWidget.h`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildMemberRowWidget.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildInvitePopupWidget.h`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraGuildInvitePopupWidget.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h`
- `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.h`
- `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/Network/NetMovementClient.cpp`

## Estrutura recomendada do WBP_GuildPanel
- Root: `CanvasPanel`
- Container principal: `Border_GuildPanel`
- Cabeçalho:
  - `Text_GuildName`
  - `Text_GuildTag`
  - `Text_GuildLevel`
  - `Text_GuildXP`
  - `Text_GuildMembersCount`
- Área de criação (visível quando não estiver em guild):
  - `ETB_GuildName`
  - `ETB_GuildTag`
  - `MTB_GuildDescription`
  - `BTN_CreateGuild`
- Área de gerenciamento (visível quando estiver em guild):
  - `ETB_InviteTargetPlayerID`
  - `BTN_InviteMember`
  - `BTN_LeaveGuild`
  - `BTN_AddGuildXP`
- Lista de membros:
  - `ScrollBox_Members`
  - item row `WBP_GuildMemberRow` (parent `UUmbraGuildMemberRowWidget`) com:
    - `Text_PlayerName`
    - `Text_Level`
    - `Text_Class`
    - `Text_Rank`
    - `BTN_Promote`
    - `BTN_Demote`
    - `BTN_Kick`

## Parent C++ do Blueprint
- Defina parent do `WBP_GuildPanel` para `UUmbraGuildWidget`.
- Defina parent do `WBP_GuildMemberRow` para `UUmbraGuildMemberRowWidget`.
- `UUmbraGuildWidget` já possui:
  - `RequestCreateGuild`
  - `RequestInviteMember`
  - `RequestAcceptInvite`
  - `RequestDeclineInvite`
  - `RequestKickMember`
  - `RequestSetMemberRank`
  - `RequestTransferMaster`
  - `RequestLeaveGuild`
  - `RequestAddGuildXP`
  - `RequestLoadGuildState`
  - `GetCachedGuildState`

## Fluxo C++-first (Blueprint minimo)
- `NativeConstruct` do `UUmbraGuildWidget` faz:
  - bind de delegates do `UUmbraGameInstance` (`OnGuildStateLoaded`, `OnGuildActionFailed`, `OnGuildInviteReceived`);
  - bind de `OnClicked` dos botoes (`BTN_CreateGuild`, `BTN_InviteMember`, `BTN_LeaveGuild`, `BTN_AddGuildXP`);
  - chamada inicial de `LoadGuildState`.
- `HandleGuildStateLoaded` atualiza `CachedGuildState` e chama `RefreshGuildPanelFromCache`.
- `RefreshGuildPanelFromCache` resolve:
  - visibilidade de `Container_CreateGuild`/`Container_GuildManage`/`ScrollBox_Members`;
  - preenchimento de `Text_GuildName`, `Text_GuildTag`, `Text_GuildLevel`, `Text_GuildXP`, `Text_GuildMembersCount`;
  - rebuild da lista com `CreateWidget<UUmbraGuildMemberRowWidget>`.
- `UUmbraGuildMemberRowWidget::ApplyGuildMember` preenche campos da row e aplica permissao de botoes (promote/demote/kick) em C++.
- Cliques de `BTN_Promote`, `BTN_Demote` e `BTN_Kick` sobem via delegate C++ da row e o painel chama:
  - `RequestSetMemberRank` (promove/rebaixa entre rank 1..3)
  - `RequestKickMember`

## Blueprint: implementação nó a nó

### 1) Event Graph do WBP_GuildPanel
Blueprint minimo recomendado:
1. `Event Construct` sem logica de guild (pode ficar vazio).
2. Nao bindar delegates no Blueprint.
3. Nao bindar `OnClicked` de botoes no Blueprint.
4. Nao criar/limpar rows de membro no Blueprint.

Tudo isso agora ocorre no C++ (`NativeConstruct` + handlers).

### 2) Designer (nomes obrigatorios para BindWidgetOptional)
No `WBP_GuildPanel`, manter os nomes:
- `ETB_GuildName`
- `ETB_GuildTag`
- `MTB_GuildDescription`
- `ETB_InviteTargetPlayerID`
- `BTN_CreateGuild`
- `BTN_InviteMember`
- `BTN_LeaveGuild`
- `BTN_AddGuildXP`
- `Container_CreateGuild`
- `Container_GuildManage`
- `ScrollBox_Members`
- `Text_GuildName`
- `Text_GuildTag`
- `Text_GuildLevel`
- `Text_GuildXP`
- `Text_GuildMembersCount`

### 3) Designer da row (WBP_GuildMemberRow)
Parent: `UUmbraGuildMemberRowWidget`.

Nomes obrigatorios:
- `Text_PlayerName`
- `Text_Level`
- `Text_Class`
- `Text_Rank`
- `BTN_Promote`
- `BTN_Demote`
- `BTN_Kick`

Sem Event Graph obrigatorio para renderizacao e acoes basicas.

## Regra de cargos
- **Dono/Mestre:** privilégios totais.
- **Rank 3:** convida e remove.
- **Rank 2:** convida.
- **Rank 1:** sem privilégios administrativos.

## Nameplate com guild
- Pipeline atualizado para enviar `GuildName` junto com `CharacterName` e `Title`.
- Widget remoto lê:
  - `TB_CharacterName`
  - `TB_Title`
  - `TB_GuildName` (opcional; colapsa quando vazio)

### Implementação do nameplate (nó a nó)
Parent/fluxo:
- O C++ de `UmbraRemoteNameplateWidget` recebe `GuildName` e aplica no widget.
- No Blueprint `WBP_RemoteNameplate`, manter:
  - `TB_CharacterName`
  - `TB_Title`
  - `TB_GuildName`

Comportamento esperado:
1. `TB_CharacterName` recebe o nome do personagem.
2. `TB_Title` recebe o título.
3. `TB_GuildName`:
   - `Visible` quando `GuildName` vier preenchido.
   - `Collapsed` quando `GuildName` vier vazio.

## Player Context Menu: Invite To Guild

### Enum e rota central
- Adicionar `InviteToGuild` em `EUmbraPlayerContextAction`.
- O `WBP_PlayerContextMenu` chama apenas:
  - `UmbraPlayerSelectionComponent -> ExecuteContextAction(InviteToGuild)`.
- O `UUmbraGameInstance::HandlePlayerContextAction(...)` trata o caso:
  - valida se o player atual está em guild;
  - valida se tem permissão para convidar (`owner`, `master` ou `MyMemberRank >= 2`);
  - chama `InviteGuildMemberByName(PlayerInfo.CharacterName)`.

### Visibilidade do botão no Blueprint
No `WBP_PlayerContextMenu`, botão:
- `BTN_InviteToGuild`

Regra de exibição:
- Mostrar apenas quando:
  - o player atual está em guild;
  - tem permissão para convidar;
  - o alvo não é ele mesmo.

Implementação recomendada no Blueprint:
1. Criar função/binding `GetInviteToGuildVisibility`.
2. Obter `GameInstance` -> `Cast To UmbraGameInstance`.
3. Chamar `GetCurrentGuildState`.
4. Validar:
   - `bInGuild == true`
   - `MyMemberRank >= 2` ou player atual ser `owner/master`
5. Se válido -> `Visible`, senão -> `Collapsed`.

### Clique do botão
Nós exatos:
1. `OnClicked (BTN_InviteToGuild)`
2. `Get Owning Player`
3. `Get PlayerSelectionComponent`
4. `ExecuteContextAction`
5. `Action = InviteToGuild`

## Popup de convite de guild

### Parent C++ do popup
- Criado: `UUmbraGuildInvitePopupWidget`

Funções disponíveis:
- `SetupInviteData(InviteID, FromPlayerID, GuildName)`
- `RequestAcceptInvite()`
- `RequestDeclineInvite()`

O Blueprint só precisa chamar `SetupInviteData(...)` ao criar o popup.
Os botões `BTN_AcceptGuildInvite` e `BTN_DeclineGuildInvite` já são bindados em C++ no `NativeConstruct`.

### Estrutura recomendada do `WBP_GuildInvitePopup`
- Parent: `UUmbraGuildInvitePopupWidget`
- Widgets:
  - `TXT_GuildInviteTitle`
  - `TXT_GuildInviteBody`
  - `BTN_AcceptGuildInvite`
  - `BTN_DeclineGuildInvite`

### Como abrir o popup quando receber convite
Widget responsável: HUD/widget raiz sempre presente no cliente.

Passo a passo:
1. `Event Construct`
2. `Get Game Instance`
3. `Cast To UmbraGameInstance`
4. `Bind Event to OnGuildInviteReceived`

Custom Event bound:
1. Recebe `InviteID`, `FromPlayerID`, `GuildName`
2. `Create Widget (WBP_GuildInvitePopup)`
3. Chamar `SetupInviteData(InviteID, FromPlayerID, GuildName)`
4. `Add To Viewport`

### Aceitar / Recusar
Como os botões já estão bindados em C++, o Blueprint não precisa implementar lógica de clique.

Fluxo interno do C++:
- `BTN_AcceptGuildInvite` -> `RequestAcceptInvite()` -> `UUmbraGameInstance::AcceptGuildInvite(InviteID)`
- `BTN_DeclineGuildInvite` -> `RequestDeclineInvite()` -> `UUmbraGameInstance::DeclineGuildInvite(InviteID)`
- ambos removem o popup do viewport via `RemoveFromParent()`

## Testes recomendados
1. Criar guild com gold suficiente e insuficiente.
2. Convidar membro com rank 2, 3 e 1.
3. Aceitar/recusar convite.
4. Promover/rebaixar membro.
5. Remover membro por rank 3/mestre.
6. Abrir painel e validar lista: nome, nível, classe, rank.
7. Verificar `GuildName` no nameplate de players remotos.
8. Validar update em tempo real (refresh de guild via WebSocket).

## Fluxo rapido de teste (fim a fim)
1. Rodar scripts SQL na ordem indicada.
2. Reiniciar API PHP e servidor Zone.
3. Login com 2 jogadores.
4. Jogador A cria guild.
5. Jogador A convida jogador B.
6. Jogador B aceita convite.
7. Abrir painel em ambos e validar lista/ranks.
8. Validar `GuildName` no nameplate remoto.

