# GUIA — Missões de guilda (UE 5.6.1)

Esta etapa (3b) dá a cada guilda **3 missões diárias e 2 semanais**, sorteadas de uma lista do Manager conforme o nível da guilda. Todos os membros contribuem e o progresso é da guilda inteira. Ao concluir:
- a guilda ganha **EXP de guilda**, concedida uma vez só, mesmo com várias zones;
- cada membro que contribuiu o mínimo **resgata** ouro e/ou item na janela de missões.

O **C++ e o PHP já fazem tudo**. No Editor você só cria `WBP_GuildMissions`, `WBP_GuildMissionRow` e o botão `Btn_GuildMissions` na janela da guilda. Se você pedir, eu monto pelo MCP depois de compilar.

## Objetivo

- **Como os eventos chegam:**
  - os eventos de jogo que a zone já vê nas quests (kill, kill_any, ouro saqueado, skill, consumível) também avançam as missões da guilda do jogador;
  - craft e refino avançam pelo PHP.
- O progresso aparece ao vivo para os membros online (**opcode 133**, no máximo 1 a cada 2 s por missão).
- Ao concluir, todos os membros online recebem o aviso *"Missão de guilda concluída: Caçada da guilda (+300 EXP de guilda)…"* pelo **73**.
- **Sorteio:** procedure `sp_guild_missions_ensure` no MySQL, usada pelo PHP e pela zone (uma regra só). Ela roda no primeiro acesso do dia ou da semana, ao abrir a janela ou no primeiro evento na zone.
- **Segurança:** `add_guild_xp.php` agora é **só admin**. O botão e a caixa de "EXP de teste" da janela da guilda ficam escondidos pelo C++.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `guild_mission_templates.*` | Manager → **Missões de guilda** | Título, descrição, período (daily/weekly), tipo e parâmetros (mesmo formato dos objetivos de quest), meta da guilda, nível mínimo da guilda, peso no sorteio |
| Recompensas | mesma aba | EXP de guilda (guilda), ouro e item (por membro), contribuição mínima |
| `guild_missions` | (jogo) | Instância por guilda e período: progresso, status, `xp_awarded` |
| `guild_mission_contributions` | (jogo) | Contribuição por membro + resgate |

**Parâmetros por tipo (JSON):**

| Tipo | Exemplo |
|---|---|
| `kill` | `{"npc_template_id":6}` ou `{"npc_template_ids":[6,9]}` |
| `kill_any` | `{"min_level":1,"max_level":10,"zone_id":0}` |
| `earn_gold` | `{}` (a meta é o total de ouro) |
| `use_skill` | `{"skill_id":0}` (0 = qualquer) |
| `use_consumable` | `{"item_subtype":"health_potion"}` ou `{"item_template_id":7}` |
| `craft_item` | `{"item_template_id":0}` (0 = qualquer) |
| `refine_item` | `{"min_refine_level":3}` |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\create_guild_missions.sql
```

O script cria as 3 tabelas, a procedure e 6 missões de exemplo (4 diárias e 2 semanais).

---

## 0) Pré-requisitos

1. `zone_server` recompilado (já feito) e **reiniciado**.
2. Manager recompilado (já feito): abrir de novo.
3. Rebuild do módulo `UmbraEternumUE`.
4. PHP já sincronizado com o WAMP.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_GuildMissions` | `/Game/Widgets/UI/Guild/WBP_GuildMissions` | `UmbraGuildMissionsWidget` |
| `WBP_GuildMissionRow` | `/Game/Widgets/UI/Guild/WBP_GuildMissionRow` | `UmbraGuildMissionRowWidget` |
| `WBP_GuildPanel` (existente) | `/Game/Widgets/UI/Guild/WBP_GuildPanel` | + botão `Btn_GuildMissions` |

## 1) Designer

### `WBP_GuildMissions` (janela, mesmo esqueleto das outras)

| Bind | Tipo | Função |
|---|---|---|
| `VB_MissionList` | VerticalBox ou ScrollBox | Recebe uma linha por missão |
| `Text_MissionsEmpty` | TextBlock | "Nenhuma missão disponível…" (sem guilda) |
| `Text_GuildMissionStatus` | TextBlock | Resgate ok (verde) ou erro (vermelho), por 4 s |

Class Defaults: **Mission Row Class** = `WBP_GuildMissionRow` (opcional).

### `WBP_GuildMissionRow` (linha)

| Bind | Tipo | Função |
|---|---|---|
| `Text_MissionTitle` | TextBlock | Título |
| `Text_MissionPeriod` | TextBlock | "Diária · termina em 5 h" / "Semanal · concluída" |
| `Text_MissionDescription` | TextBlock | Descrição |
| `ProgressBar_Mission` | ProgressBar | Progresso da guilda |
| `Text_MissionProgress` | TextBlock | "40 / 60" |
| `Text_MissionContribution` | TextBlock | "Sua contribuição: 12 (mínimo 5)" |
| `Text_MissionReward` | TextBlock | "+300 EXP de guilda · 100 de ouro · Item" |
| `Btn_Claim` + `Text_ClaimLabel` | Button + TextBlock | Aparece com a missão concluída: **Resgatar**, **Resgatada** ou **Sem direito** |

### `WBP_GuildPanel`
- Adicionar o botão `Btn_GuildMissions` ("Missões") onde fizer sentido, por exemplo ao lado das abas.
- O C++ esconde `BTN_AddGuildXP` e `ETB_AddGuildXPAmount` (o EXP de teste).
- Class Defaults (opcional): **Guild Missions Window Class** = `WBP_GuildMissions`.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `GuildMissionService` | `src/zone/GuildMissionService.cpp` | Na thread do `QuestProgressService`: guilda do jogador (cache 60 s), missões e membros da guilda (cache 30 s; chama a procedure), crédito + contribuição, conclusão atômica (status, EXP uma vez, nível pela curva do PHP), 133 e aviso 73 aos membros online |
| `QuestProgressService` | `processEvent` | Repassa cada evento para as missões |
| Conector MySQL | `src/database/MySQLConnector.cpp` | Drena os resultados extras do `CALL` (evita "Commands out of sync" no pool) |
| PHP | `helpers/guild_mission_helper.php` | `gmList` (sorteio + lista + contribuição), `gmClaim` (transação, uma vez, contribuição mínima, bolsa cheia = erro), `gmCreditEvent` (craft/refino) |
| UE | `Core/UmbraGameInstanceGuildMissions.cpp` | HTTP da lista e do resgate, 133 atualiza o cache (conclusão recarrega a lista), delegates |
| UE | `UI/UmbraGuildMissionsWidget.cpp`, `UI/UmbraGuildMissionRowWidget.cpp`, `UI/UmbraGuildWidget.cpp` | Janela, linhas, botão Missões, esconder o EXP de teste |

## 4) Protocolo

### 133 `GuildMissionProgress` (zone → membros online da guilda)
`[guildMissionId:u32][progress:u32][required:u32][status:u8]`, 14 bytes. `status` 1 = concluída. O aviso de conclusão vai também pelo 73 (`SystemBroadcast`).

## 5) Manager
1. **Missões de guilda** → **Atualizar** → editar ou criar (**Nova missão**) → **Salvar**. O Manager manda o `reload_guild_missions`.
2. Missões já sorteadas mantêm a meta do sorteio; a mudança vale do próximo período em diante.

---

## 6) QA (PIE)

PIE com **2 jogadores na mesma guilda** (A e B), zone reiniciada, UE recompilado.

| # | Cenário | Esperado |
|---|---------|----------|
| G1 | A abre a guilda → **Missões** | Lista com 3 diárias e 2 semanais, progresso 0, "termina em …" |
| G2 | A mata mobs (missão `kill_any` sorteada) | Barra sobe ao vivo para A **e** B (133); contribuição de A sobe |
| G3 | B saqueia ouro (missão `earn_gold`) | Progresso soma para a guilda; contribuição de B sobe |
| G4 | Completar uma missão (no Manager, meta baixa numa missão nova + reload, ou SQL de teste) | Aviso de conclusão para A e B; EXP da guilda sobe **uma vez**; botão **Resgatar** |
| G5 | A resgata | Ouro/item entram; botão vira **Resgatada**; resgatar de novo dá erro |
| G6 | Membro que não contribuiu o mínimo | Botão **Sem direito** (desabilitado) |
| G7 | Craft com missão `craft_item` ativa | Progresso sobe (PHP) |
| G8 | Jogador sem guilda abre a janela | Mensagem "Nenhuma missão disponível…" |
| G9 | Chamar `add_guild_xp.php` com token de jogador | 401/403 (só admin); botão de EXP de teste não aparece na janela da guilda |
| G10 | Virar o dia (ou apagar as missões do período em `guild_missions` no dev) | Novo sorteio no próximo acesso |

**SQL de teste para G4** (dev): completar na hora a missão de id X.

```sql
UPDATE guild_missions SET progress = required_total - 1 WHERE guild_mission_id = X;
```

Depois é só fazer 1 evento do tipo da missão no jogo.

Log esperado na zone: `"[GuildMissionService] guilda G concluiu a missão M (Caçada da guilda)"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Lista vazia com guilda | Sem missões ativas do nível da guilda | Conferir `is_active` e `min_guild_level` no Manager |
| Progresso não sobe | Zone antiga ou o tipo da missão não é creditado pela zone (craft/refino = PHP) | Reiniciar a zone nova; para craft/refino conferir o PHP sincronizado |
| Erro "Commands out of sync" no log | Zone sem o dreno do conector | Recompilar a zone |
| Resgate "Inventário cheio" | Item de recompensa sem espaço | Liberar espaço e resgatar de novo (nada é perdido) |

## 8) Referência de código

- Servidor:
  - `src/zone/GuildMissionService.{hpp,cpp}`;
  - `src/zone/QuestProgressService.cpp`;
  - `src/zone/MovementProtocol.hpp` (133);
  - `src/zone/ZoneServer.{hpp,cpp}`;
  - `src/admin/ServiceAdminRegister.cpp` (`reload_guild_missions`);
  - `src/database/MySQLConnector.cpp`.
- PHP/SQL:
  - `scripts/create_guild_missions.sql`;
  - `helpers/guild_mission_helper.php`;
  - `api/social/get_guild_missions.php`, `claim_guild_mission_reward.php`, `add_guild_xp.php` (admin);
  - `api/admin/*guild_mission_template*.php`;
  - `api/crafting/craft.php`, `api/refinement/refine_item.php`.
- Manager: `Models/GuildMissionTemplateRow.cs`, `ViewModels/MainViewModel.GuildMissions.cs`, `Views/GuildMissionsEditorView.xaml`.
- UE:
  - `Data/UmbraGuildMissionTypes.h`;
  - `Core/UmbraGameInstanceGuildMissions.cpp`;
  - `UI/UmbraGuildMissionsWidget.*`, `UI/UmbraGuildMissionRowWidget.*`, `UI/UmbraGuildWidget.*`.

**Fim do guia.**
