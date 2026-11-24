# GUIA — Dungeons (UE 5.6.1)

A etapa 5 transforma uma **instância** (etapa 4) em **dungeon**: chefes, condição de vitória, dificuldades, recompensa de conclusão e lockout. Tudo é cadastrado no Manager e reaproveita a sala de espera e as cópias da etapa 4.

O **C++ e o PHP já fazem tudo**. No Editor você só:
- ajusta `WBP_InstanceLobby` e `WBP_InstanceHud`;
- cria `WBP_DungeonComplete`.

Se você pedir, eu monto pelo MCP depois de compilar. Chefes com fases ficam para a próxima etapa.

## Objetivo

- **Sala de espera:**
  - quem abriu escolhe a **dificuldade** (botões montados pelo C++);
  - todos veem a escolha, e trocar zera os "Pronto" (**134** ação 5, **137** trailing);
  - a linha de quem já pegou a recompensa no período mostra "· sem recompensa". Essa pessoa entra, mas não recebe a recompensa de conclusão.
- **Dentro:**
  - o HUD mostra "Heroica · Chefes 1/3 · Inimigos 12/40" ao vivo (**138**);
  - a dificuldade multiplica a **vida** e o **ataque físico/mágico** dos NPCs da cópia.
- **Vitória:** depende do modo do molde.
  - `final_boss`: matar o(s) chefe(s) final(is). Sem chefe final, valem todos os chefes; sem chefes, todos os inimigos.
  - `all_bosses`: matar todos os chefes.
- **Conclusão:** cada jogador **dentro da cópia** recebe EXP e ouro (× multiplicador da dificuldade) e os itens sorteados da tabela.
  - Abre a janela **"Dungeon concluída"** (**139**).
  - Bolsa cheia: o item vai para o **correio** (remetente "Sistema").
  - A cópia fecha em `completion_close_seconds` (padrão 300 s), com o HUD mostrando "Concluída · fecha em 4:51".
- **Lockout:**
  - recompensa de conclusão 1x por período (`daily`/`weekly`/`none`), por jogador, dungeon e dificuldade;
  - fica registrado em `dungeon_completions`;
  - o loot dos mobs continua normal.
- **Derrota:** tempo limite esgotado → "Falhou: tempo esgotado" e todos voltam ao portal.
- **Quests/guilda:** novo objetivo **`complete_dungeon`** (`instance_template_id` 0 = qualquer; `difficulty_key` vazio = qualquer).
- **O que não muda:** a sala, as cópias e a entrada da etapa 4. Uma instância que não é dungeon continua igual.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `instance_templates.is_dungeon` | Manager → **Instâncias** → "É dungeon" | Liga as regras abaixo |
| `completion_mode` | mesma tela → "Conclui com" | `final_boss` / `all_bosses` |
| `reward_exp`, `reward_gold` | "EXP/Ouro de conclusão" | × `reward_mult` da dificuldade |
| `lockout_reset` | "Lockout" | `daily`, `weekly` ou `none` |
| `completion_close_seconds` | "Fecha após concluir (s)" | Tempo até a cópia fechar |
| `dungeon_difficulties.*` | grade **Dificuldades** | Chave, nome, ordem, nível mínimo, x Vida, x Dano, x Recompensa |
| `dungeon_reward_items.*` | grade **Recompensas de conclusão** | Dificuldade (vazio = todas), item, chance 0..1, qtd mín/máx |
| `npc_instances.dungeon_role` | **NPCs e mundo** → **Instâncias no mundo** → "Papel na dungeon" | vazio = comum, `trash`, `boss`, `final_boss` (só NPC de molde) |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_dungeons.sql
```

O seed transforma o **INST_TEST** em dungeon com **Normal** (×1) e **Heroica** (vida ×2,5, dano ×1,8, recompensa ×2), 500 EXP e 200 de ouro, com lockout diário.

---

## 0) Pré-requisitos

1. `zone_server` recompilado (já feito) e **reiniciado**.
2. Manager recompilado (já feito): abrir de novo.
3. Rebuild do módulo `UmbraEternumUE` (classe nova `UmbraDungeonCompleteWidget`; binds novos na sala e no HUD).
4. PHP já sincronizado com o WAMP.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_InstanceLobby` (existente) | `/Game/Widgets/UI/Instance/WBP_InstanceLobby` | `UmbraInstanceLobbyWidget` + 2 binds |
| `WBP_InstanceHud` (existente) | `/Game/Widgets/UI/Instance/WBP_InstanceHud` | `UmbraInstanceHudWidget` + 1 bind |
| `WBP_DungeonComplete` | `/Game/Widgets/UI/Instance/WBP_DungeonComplete` | `UmbraDungeonCompleteWidget` |

## 1) Designer

### `WBP_InstanceLobby` (acrescentar acima de "GRUPO")

| Bind | Tipo | Função |
|---|---|---|
| `Text_DifficultyHeader` | TextBlock | "DIFICULDADE" (some se não é dungeon) |
| `HB_Difficulties` | HorizontalBox | Recebe um botão por dificuldade (C++ usa o estilo do `Btn_Ready`; a escolhida fica dourada; só quem abriu pode clicar) |

### `WBP_InstanceHud` (acrescentar abaixo do tempo)

| Bind | Tipo | Função |
|---|---|---|
| `Text_DungeonProgress` | TextBlock | "Heroica · Chefes 1/3 · Inimigos 12/40" / "Concluída · fecha em 4:51" (verde) / "Falhou" (vermelho) |

### `WBP_DungeonComplete` (janela, mesmo esqueleto das outras)

| Bind | Tipo | Função |
|---|---|---|
| `Text_DungeonResultTitle` | TextBlock | "Cripta de teste concluída!" |
| `Text_DungeonResultInfo` | TextBlock | "Heroica · Tempo 12:34" |
| `Text_DungeonResultExp` | TextBlock | "+1000 EXP" |
| `Text_DungeonResultGold` | TextBlock | "+400 de ouro" |
| `VB_DungeonRewardItems` | VerticalBox | Um texto por item ("Espada x1", "(correio)") montado em C++ |
| `Text_DungeonResultNote` | TextBlock (Auto Wrap) | Lockout ou aviso de correio |
| `Btn_DungeonResultOk` + `Text_DungeonResultOkLabel` | Button + TextBlock | **Continuar** (fecha) |

No `BP_UmbraGameInstance` (opcional): **Dungeon Complete Window Class** = `WBP_DungeonComplete`. Vazio = path padrão.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `InstanceManager` | `src/zone/InstanceManager.*` | Regras de dungeon do molde; dificuldade na sala (só quem abriu; zera prontos); lockout por membro (flag no 137); totais da cópia; progresso a cada morte (138); vitória; fechamento após concluir; falha por tempo |
| `DungeonService` | `src/zone/DungeonService.*` | Thread própria: lockout no período, registro em `dungeon_completions`, EXP/ouro, sorteio de itens, bolsa ou correio, 139 e evento `complete_dungeon` |
| `NpcManager` | `src/zone/NpcManager.*` | `dungeon_role` dos moldes (query própria, sem mexer no SELECT posicional); vida e ataques dos clones × dificuldade; contagem de inimigos/chefes |
| `CombatCoreEngine` | `src/zone/CombatCoreEngine.*` | Morte de NPC de cópia → `InstanceManager` (combat worker) |
| `LootService` | `src/zone/LootService.*` | EXP com aviso (106/107), ouro e item usados pela conclusão |
| Quests/guilda | `src/zone/QuestProgressService.*` | Tipo `CompleteDungeon` (quests v2 e missões de guilda) |
| UE | `Core/UmbraGameInstanceInstances.cpp` | 134 ação 5; trailing do 137; 138 → `OnDungeonProgressUpdated`; 139 → ouro/bolsa + `WBP_DungeonComplete` |
| UE | `UI/UmbraInstanceLobbyWidget.*`, `UI/UmbraInstanceLobbyRowWidget.*`, `UI/UmbraInstanceHudWidget.*`, `UI/UmbraDungeonCompleteWidget.*` | Botões de dificuldade, "sem recompensa", progresso no HUD, janela de conclusão |

## 3) Protocolo

| Op | Direção | Formato |
|----|---------|---------|
| **134** | C→S | `[134][portalNpcId:u32][action:u8][value:u8]`. Ação **5** = escolher dificuldade (`value` = índice da lista do 137). `value` é trailing (0 se ausente) |
| **137** (trailing) | S→C | Depois dos membros: `[isDungeon:u8][selected:u8][leaderPlayerId:u32][n:u8]{key:u16+utf8≤32, name:u16+utf8≤64, minLevel:u16}`. Flags do membro: bit1 = lockout. Frame antigo termina nos membros |
| **138** `DungeonProgress` | S→C (membros da cópia) | `[138][instanceId:u32][bossesKilled:u16][bossesTotal:u16][enemiesKilled:u16][enemiesTotal:u16][state:u8 0=andando,1=concluída,2=falhou][closesInSec:u16][difficultyName:u16+utf8≤64]`. Enviado na entrada, a cada morte, na conclusão e na falha |
| **139** `DungeonCompleted` | S→C (cada jogador) | `[139][templateId:u32][duration:u32][rewarded:u8][exp:u32][gold:u32][playerGold:i64][invChanged:u8][dungeonName:u16+utf8≤120][difficultyName:u16+utf8≤64][n:u8]{itemId:u32, qty:u16, mailed:u8, name:u16+utf8≤64}` |

## 4) Manager

1. **Instâncias** → editar o molde:
   - marcar **É dungeon**;
   - escolher **Conclui com** e **Lockout**;
   - preencher EXP e ouro de conclusão.
2. **Dificuldades** → **+ Dificuldade** (a 1ª vira `normal`, a 2ª `heroic`). Editar direto na grade.
3. **Recompensas** → **+ Item**: Item ID, chance (1 = sempre), quantidade. Dificuldade vazia = todas.
4. **Salvar**. Grava molde, dificuldades e recompensas e manda o `reload_instances`.
5. Chefes:
   - **NPCs e mundo** → **Instâncias no mundo** → NPC de molde → **Papel na dungeon** = `boss` ou `final_boss` → **Salvar vínculo**;
   - para o chefe ignorar CC, use o `cc_immune` do template.
6. Quests/missões: tipo **`complete_dungeon`**. Na quest: **Alvo** = id da dungeon (0 = qualquer) e **Subtipo** = chave da dificuldade (vazio = qualquer).

---

## 5) QA (PIE)

PIE com **2 jogadores no mesmo grupo** (A abre a sala), zone reiniciada, UE recompilado, INST_TEST com 1 `boss` e 1 `final_boss` e 1 item de recompensa.

| # | Cenário | Esperado |
|---|---------|----------|
| D1 | A abre o portal | Sala com "DIFICULDADE": Normal (dourado) e Heroica; B vê os botões desabilitados |
| D2 | A escolhe Heroica com A pronto | Os dois "Aguardando" de novo; aviso "Dificuldade: Heroica." |
| D3 | Ambos prontos | HUD "Heroica · Chefes 0/2 · Inimigos 0/N"; NPCs com mais vida que no Normal |
| D4 | Matar trash e o `boss` | HUD sobe ao vivo para A e B; "Chefes 1/2" |
| D5 | Matar o `final_boss` | HUD "Concluída · fecha em 5:00"; janela de conclusão para A e B com tempo, EXP (×2), ouro (×2) e itens; ouro e bolsa atualizam |
| D6 | Bolsa de A cheia na conclusão | Item com "(correio)" e aviso; correio do sistema chega para A |
| D7 | A refaz Heroica no mesmo dia | Na sala, A aparece "· sem recompensa"; na conclusão, "Você já recebeu a recompensa…" sem EXP/ouro/itens |
| D8 | A faz Normal no mesmo dia | Recebe normalmente (lockout por dificuldade) |
| D9 | Tempo limite 1 min sem matar o chefe final | HUD "Falhou"; aviso "Tempo esgotado"; todos voltam ao portal; sem recompensa |
| D10 | Quest `complete_dungeon` ativa | Progresso sobe na conclusão |
| D11 | Missão de guilda `complete_dungeon` | Progresso da guilda sobe |
| D12 | C (do grupo) tenta entrar depois da conclusão | Recusa "A dungeon do seu grupo já foi concluída." |
| D13 | Instância que não é dungeon | Sem botões de dificuldade, sem progresso no HUD, sem janela de conclusão (etapa 4 igual) |

SQL de apoio (dev): zerar o lockout de um jogador.

```sql
DELETE FROM dungeon_completions WHERE player_id = X;
```

Logs esperados:
- `[InstanceManager] dungeon INST_TEST concluída na instância N (…s, 2 jogador(es))`;
- `[DungeonService] player P concluiu … (heroic) em …s: recompensado (exp=… ouro=… itens=…)`.

## 6) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Sala sem botões de dificuldade | Molde sem "É dungeon" ou sem dificuldades ativas | Manager → Instâncias; conferir a grade e **Salvar** |
| HUD sem "Chefes" | Nenhum NPC do molde com papel `boss`/`final_boss` | Marcar o papel e **Salvar vínculo** (recarrega a zone) |
| Dungeon conclui no primeiro mob | Molde sem chefes e só 1 inimigo atacável | Marcar o chefe final |
| Nunca conclui | Chefe final não atacável, ou cópia criada antes do papel ser salvo | Conferir `is_attackable`; abrir uma cópia nova |
| Janela de conclusão não abre | `WBP_DungeonComplete` ausente | Criar o asset ou definir **Dungeon Complete Window Class** |
| Recompensa sem itens | Chance 0, item inativo ou dificuldade diferente | Conferir a grade de recompensas |

## 7) Referência de código

- **Servidor:**
  - `src/zone/DungeonService.{hpp,cpp}`;
  - `src/zone/InstanceManager.{hpp,cpp}`;
  - `src/zone/NpcManager.{hpp,cpp}`;
  - `src/zone/CombatCoreEngine.{hpp,cpp}`;
  - `src/zone/LootService.{hpp,cpp}`;
  - `src/zone/QuestProgressService.{hpp,cpp}`;
  - `src/zone/MovementProtocol.hpp` (134/137 trailing, 138, 139);
  - `src/zone/MovementServer.hpp`;
  - `src/zone/ZoneServer.{hpp,cpp}`;
  - `tests/test_dungeons.cpp`.
- **PHP/SQL:**
  - `scripts/add_dungeons.sql`;
  - `api/admin/instance_admin_helpers.php`;
  - `api/admin/list_instance_templates.php`;
  - `api/admin/{list,save}_dungeon_{difficulties,rewards}.php`;
  - `api/admin/spawn_npc.php`, `update_npc_instance.php`, `list_npc_instances.php`;
  - `api/admin/quest_schema_helpers.php`, `quest_admin_helpers.php`, `guild_mission_admin_helpers.php`.
- **Manager:**
  - `Models/InstanceTemplateRow.cs`, `DungeonDifficultyRow.cs`, `DungeonRewardRow.cs`, `NpcInstanceRow.cs`, `QuestObjectiveRow.cs`;
  - `ViewModels/MainViewModel.Instances.cs`, `MainViewModel.NpcVendorQuest.cs`, `MainViewModel.GuildMissions.cs`;
  - `Views/InstancesEditorView.xaml`, `Views/Npcs/NpcEditorView.xaml`, `Views/GuildMissionsEditorView.xaml`;
  - `Services/PhpAdminClient.cs`.
- **UE:**
  - `Data/UmbraInstanceTypes.h`, `Data/UmbraDataStructures.h`;
  - `Core/UmbraGameInstanceInstances.cpp`, `Core/UmbraGameInstance.{h,cpp}`;
  - `Network/NetMovementClient.cpp`;
  - `UI/UmbraInstanceLobbyWidget.*`, `UI/UmbraInstanceLobbyRowWidget.*`, `UI/UmbraInstanceHudWidget.*`, `UI/UmbraDungeonCompleteWidget.*`, `UI/UmbraQuestHudTrackerWidget.cpp`.

**Fim do guia.**
