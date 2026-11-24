# GUIA — Instâncias (UE 5.6.1)

Esta etapa (4) cria **instâncias lógicas dentro da própria zone**: um NPC portal leva o jogador (ou o grupo) a uma cópia privada de um mapa, com cópia própria dos NPCs. É a base das dungeons (etapa 5) e da arena (etapa 6).

O **C++ e o PHP já fazem tudo**. No Editor você só:
- cria o mapa da instância;
- cria o HUD `WBP_InstanceHud`. Se você pedir, eu monto pelo MCP depois de compilar.

## Objetivo

- **Sala de espera (ready check):** interagir com o **NPC portal** abre a janela **WBP_InstanceLobby** (opcodes **134** e **137**), sem teleporte imediato:
  - mostra o nome, a descrição e as regras da dungeon (nível mínimo, jogadores, grupo, tempo limite) e uma linha por membro (nome, nível, **Pronto** ou **Aguardando**; nível baixo em vermelho);
  - em grupo, a janela abre sozinha para **todos os membros do grupo na mesma zone**, no mundo aberto, com o aviso "Fulano abriu a sala de …";
  - cada um clica **Pronto**. Quando **todos** estão prontos, a zone revalida (online, mundo aberto, nível, vagas) e leva todos para a **mesma cópia**;
  - **Sair** ou o X da janela cancela a sala para todos ("Fulano cancelou a entrada."). Desconectar também cancela ("Fulano desconectou."). Sem todos prontos em `instance.lobby_timeout_seconds` (padrão 120 s, na config da zone), ela fecha sozinha;
  - se o grupo **já está numa cópia**, quem chega abre uma sala só dele com o botão **Entrar** e cai na cópia do grupo;
  - membros offline ou em outra zone não entram na sala nem contam no "todos prontos".
- **Entrar:** com todos prontos, a zone:
  - cria a cópia ou reaproveita a do grupo/do próprio jogador;
  - clona os NPCs do molde;
  - manda o **135** (entrou) e teleporta cada um.
- **No cliente:** carrega o level da instância como **level instance** na origem da cópia, sem `OpenLevel`, então o WebSocket continua vivo. O pawn fica travado até o level aparecer.
- **Isolamento:**
  - cada cópia fica num ponto distante do mundo: origem + nº da cópia × espaçamento, no eixo Y;
  - combate, IA, loot, pet e AOI se separam pela distância;
  - spawn, despawn e estado de NPC e o snapshot de jogadores vão só para a **camada** (instância) certa.
- **Sair:** botão **Sair** no HUD (**136**), tempo esgotado ou fechamento pelo admin. O jogador volta para a frente do portal.
- **Cópia vazia:** fecha depois de *N* segundos (configurável).
- **Morrer lá dentro:** renasce na entrada da cópia.
- **Cair ou relogar:** o autosave grava a frente do portal, então ele volta ao mundo aberto.
- **Fica para a etapa 5 (dungeons):** chefes, condição de vitória, recompensa de conclusão, lockout e dificuldade. Os NPCs da cópia **não respawnam**.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `instance_templates.*` | Manager → **Instâncias** | Código, nome, **level** (soft path), origem e espaçamento das cópias, entrada (local), nível mínimo, máx. de jogadores, exige grupo, tempo limite, fechamento vazia, cópias simultâneas |
| `npc_instances.instance_template_id` | Manager → **NPCs e mundo** → **Instâncias no mundo** → "Pertence à instância" | NPC faz parte do molde (X/Y/Z **locais** do level); não aparece no mundo aberto |
| `npc_instances.portal_instance_template_id` | mesma tela → "Portal para a instância" | NPC do mundo aberto vira portal |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\create_instances.sql
```

O script cria `instance_templates`, as 2 colunas em `npc_instances` e o molde de teste **INST_TEST** ("Cripta de teste"):
- level `/Game/Maps/Instances/Lvl_Inst_Test.Lvl_Inst_Test`;
- entrada local (0, 0, 150);
- 30 min de limite.

### Coordenadas

- **Origem e espaçamento:** posição de **mundo** das cópias. Padrão: origem (600 000, −900 000, 0), espaçamento 150 000 no Y, até 12 cópias.
  - O mundo aberto atual fica dentro de ±50 000.
  - Mantenha todas as cópias dentro de **±1 000 000**. O Manager avisa quando a última passa disso.
  - O espaçamento precisa ser **maior que o tamanho do level + 30 000** (o AOI cobre 3×3 células de 10 000).
- **Entrada e NPCs do molde:** coordenadas **locais** do level, ou seja, exatamente as que aparecem no Editor com o mapa da instância aberto. A zone soma a origem da cópia.

---

## 0) Pré-requisitos

1. `zone_server` recompilado (já feito) e **reiniciado**.
2. Manager recompilado: **fechar o Manager aberto** e rodar `dotnet build tools/UmbraManagerWpf/UmbraManager.sln`.
3. Rebuild do módulo `UmbraEternumUE` (classes novas: `UmbraInstanceHudWidget`; `UmbraGameInstanceInstances.cpp`).
4. PHP já sincronizado com o WAMP.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| Mapa da instância | `/Game/Maps/Instances/Lvl_Inst_Test` | — (level) |
| `WBP_InstanceHud` | `/Game/Widgets/UI/Instance/WBP_InstanceHud` | `UmbraInstanceHudWidget` |
| `WBP_InstanceLobby` | `/Game/Widgets/UI/Instance/WBP_InstanceLobby` | `UmbraInstanceLobbyWidget` |
| `WBP_InstanceLobbyRow` | `/Game/Widgets/UI/Instance/WBP_InstanceLobbyRow` | `UmbraInstanceLobbyRowWidget` |

---

## 1) Editor — mapa da instância

1. **File → New Level → Empty Level**. Salvar como `/Game/Maps/Instances/`.
2. Adicionar o chão:
   - um **Cube** ou **Plane** grande (ex.: escala 40×40×1) em (0, 0, 0);
   - paredes, se quiser.
3. Luz:
   - o mundo aberto já tem sol e céu, que continuam valendo, porque o level entra **junto** com o persistente;
   - para uma cripta fechada, use luzes locais (**Point Light**/**Rect Light**);
   - **Não** adicionar Directional Light, Sky Atmosphere, Sky Light nem Exponential Height Fog no level da instância, senão eles duplicam com os do mundo aberto.
4. **Não** adicionar Player Start nem GameMode próprio (World Settings → GameMode Override = **None**).
5. Ponto de entrada:
   - o jogador nasce na **entrada local** do molde (padrão 0, 0, 150);
   - o C++ assenta o pawn no chão com um trace para baixo a partir de entrada.Z + 500;
   - deixe chão abaixo desse ponto.
6. **Save All**.

> O level pode ser montado com assets normais. Ele vira level instance em runtime; não precisa ser sublevel de nenhum mapa.

## 2) Designer — `WBP_InstanceHud`

| Bind | Tipo | Função |
|---|---|---|
| `Text_InstanceName` | TextBlock | Nome da instância |
| `Text_InstanceTimer` | TextBlock | "Tempo restante 12:34" (vermelho no último minuto; some sem limite) |
| `Btn_LeaveInstance` + `Text_LeaveLabel` | Button + TextBlock | **Sair** (manda o 136) |

- Sugestão: faixa pequena no topo centro (Anchor topo-centro), sem bloquear cliques fora dela.

### `WBP_InstanceLobby` (janela, mesmo esqueleto das outras)

| Bind | Tipo | Função |
|---|---|---|
| `Text_DungeonName` | TextBlock | Nome da dungeon |
| `Text_DungeonDescription` | TextBlock (Auto Wrap) | Descrição do molde (some se vazia) |
| `Text_DungeonInfo` | TextBlock | "Nível mínimo 10 · Até 5 jogadores · Exige grupo · Tempo limite 30 min" |
| `VB_LobbyMembers` | VerticalBox ou ScrollBox | Uma linha por membro |
| `Text_LobbyStatus` | TextBlock | "Prontos 1/3 · fecha em 1:45" ou o aviso do momento |
| `Btn_Ready` + `Text_ReadyLabel` | Button + TextBlock | **Pronto** / **Cancelar pronto** / **Entrar** (desabilitado com nível baixo) |
| `Btn_LeaveLobby` + `Text_LeaveLobbyLabel` | Button + TextBlock | **Sair** (cancela a sala para todos) |

Class Defaults (opcional): **Member Row Class** = `WBP_InstanceLobbyRow`. O X da janela (`Btn_Close`/`Btn_WindowClose`) também cancela.

### `WBP_InstanceLobbyRow` (linha)

| Bind | Tipo | Função |
|---|---|---|
| `Text_MemberName` | TextBlock | Nome ("(você)" no jogador local) |
| `Text_MemberLevel` | TextBlock | "Nv 12" (vermelho abaixo do mínimo) |
| `Text_MemberReady` | TextBlock | **Pronto** (verde), **Aguardando** ou "Nível 10 necessário" (vermelho) |

No `BP_UmbraGameInstance` (opcional): **Instance Lobby Window Class** = `WBP_InstanceLobby`. Vazio = path padrão.
- Class Defaults (opcional): **Warning Seconds** = 60.
- No `BP_UmbraGameInstance` (opcional): **Instance Hud Widget Class** = `WBP_InstanceHud`. Vazio = path padrão acima.

---

## 3) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `InstanceManager` | `src/zone/InstanceManager.{hpp,cpp}` | Moldes da zone; **sala de espera** (abrir pelo portal, membros do grupo na zone, pronto/desmarcar/cancelar, timeout, desconexão, revalidação e entrada em conjunto); criação/reuso da cópia (dono ou grupo), slots, entrada/saída, fechamento por vazio ou tempo, admin `list/close/reload_instances` |
| `NpcManager` | `src/zone/NpcManager.*` | Moldes fora do mundo aberto (filtro no WHERE, sem mexer no SELECT posicional); clones com IDs a partir de `0x40000000`, sem DB e sem respawn; portal no spawn (100) |
| `CombatCoreEngine` | `src/zone/CombatCoreEngine.*` | Spawn/despawn/estado de NPC por camada; criar/remover clones no **combat worker** (job `WorldOp`, nunca descartado); despawn/snapshot ao trocar de camada |
| `MovementServer` | `src/zone/MovementServer.hpp` | Camada por jogador, `broadcastToLayer`, `movePlayerToLayer` (somem os atores da camada antiga e chegam os da nova), snapshot por camada, respawn na entrada da cópia |
| `NpcAiSystem` | `src/zone/NpcAiSystem.cpp` | Aggro e alvo só na mesma camada |
| `ZoneServer` | `src/zone/ZoneServer.cpp` | Liga tudo; tick de 1 s; autosave grava a frente do portal |
| UE | `Core/UmbraGameInstanceInstances.cpp` | 134/136; 135: trava o pawn, `ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr` na origem, `OnLevelShown` → assenta e destrava, HUD; timeout de carga (20 s) → sai; saída descarrega o level |
| UE | `Core/UmbraGameInstance.cpp` | Portal no spawn (100) → `OpenNpcDialog` manda o 134; save de posição usa a frente do portal |
| UE | `UI/UmbraInstanceHudWidget.*` | Nome, tempo restante e botão Sair |
| UE | `Core/UmbraGameInstanceInstances.cpp` | 134 com ação; parse do 137 → `OnInstanceLobbyUpdated`; abre/fecha `WBP_InstanceLobby` pelo window manager; aviso na tela ao fechar com motivo |
| UE | `UI/UmbraInstanceLobbyWidget.*`, `UI/UmbraInstanceLobbyRowWidget.*` | Janela da sala (dados, linhas, Pronto/Entrar, Sair, contagem regressiva); X/ESC cancela |

## 4) Protocolo

| Op | Direção | Formato |
|----|---------|---------|
| **134** `InstanceEnterRequest` | C→S | `[134][portalNpcInstanceId:u32][action:u8]` (6 B). `action`: 1 = abrir a sala (portal obrigatório; frame de 5 B = 1), 2 = pronto, 3 = desmarcar, 4 = cancelar |
| **137** `InstanceLobbyState` | S→C (membros da sala) | `[137][state:u8 1=aberta/atualizada, 2=fechada][lobbyId:u32][templateId:u32][minLevel:u16][maxPlayers:u8][partyRequired:u8][timeLimitMin:u16][expiresInSec:u16][joinExisting:u8][name:u16+utf8≤120][description:u16+utf8≤500][reason:u16+utf8≤200][count:u8]{playerId:u32, level:u16, ready:u8, flags:u8 (bit0 = nível baixo), name:u16+utf8≤64}`. Na fechada, `reason` vazio = todos entraram |
| **135** `InstanceState` | S→C (dono) | `[135][state:u8][instanceId:u32][templateId:u32][originX,Y,Z:f32][posX,Y,Z:f32][yaw:f32][expiresAtUnix:u32][mapPath:u16+utf8≤255][name:u16+utf8≤120][reason:u16+utf8≤200]`. `state`: 1 = entrou (pos = entrada), 2 = saiu (pos = frente do portal), 3 = recusado (reason) |
| **136** `InstanceLeaveRequest` | C→S | `[136]` |
| **100** `NpcSpawnNotify` | S→C | campo **trailing** `portalInstanceTemplateId:u32`; `flags` bit3 (0x08) = portal. Decoder antigo ignora; frame antigo = 0 |

Ordem na entrada: **137** fechada (painel some) → **135** (o cliente trava o pawn e começa a carregar) → **123** (teleporte) → despawn dos NPCs da camada antiga + spawn dos da cópia (100/101 só para ele).

## 5) Manager

1. **Instâncias** → **Atualizar** → editar o **INST_TEST** ou criar uma (**Nova instância**):
   - **Level do UE** = `/Game/Maps/Instances/Lvl_Inst_Test.Lvl_Inst_Test`;
   - **Entrada** = coordenadas locais do Editor;
   - **Salvar**. O Manager manda o `reload_instances`.
2. NPCs do molde:
   - **NPCs e mundo** → **Instâncias no mundo** → **Criar instância**: template hostil, Zone = a da zone, X/Y/Z **locais** do level de instância;
   - selecione a linha nova → **Pertence à instância** = id do molde → **Salvar vínculo**;
   - o NPC some do mundo aberto e passa a nascer em cada cópia.
3. Portal:
   - crie no mundo aberto um NPC de template **não atacável** (`is_attackable = 0`), perto de onde os jogadores passam;
   - selecione → **Portal para a instância** = id do molde → **Salvar vínculo**.
4. **Instâncias** → **Consultar zones** lista as cópias abertas (nº, slot, jogadores, idade). **Fechar cópia** encerra uma delas.

---

## 6) QA (PIE)

PIE com **2 jogadores** (A e B), zone reiniciada, UE recompilado, mapa e WBP criados, portal configurado.

| # | Cenário | Esperado |
|---|---------|----------|
| I1 | A (sem grupo) interage com o portal → **Pronto** | Abre a sala com uma linha (A); ao clicar Pronto, carrega o level; A aparece na entrada; HUD com nome e "Tempo restante 29:5x"; NPCs do molde na cópia |
| I2 | B (fora do grupo) olha para onde A estava | A some para B (PlayerDisconnected); B continua vendo o mundo aberto normal |
| I3 | B entra pelo mesmo portal | B vai para **outra** cópia (slot 1): não vê A nem os NPCs dele |
| I4 | A e B em grupo; A abre a sala | A janela abre para A **e** B ("A abriu a sala de …"), duas linhas "Aguardando" |
| I4b | A marca Pronto | Linha de A fica verde; "Prontos 1/2"; ninguém é teleportado |
| I4c | B marca Pronto | Janela some para os dois; ambos entram na **mesma** cópia, se veem e lutam juntos |
| I5 | A mata os NPCs da cópia | Loot/EXP normais; NPCs não respawnam; o mundo aberto não recebe nada (log sem spawn/despawn para quem está fora) |
| I6 | A clica **Sair** | Volta para a frente do portal; level descarregado; HUD some; NPCs do mundo aberto reaparecem |
| I7 | Todos saem; esperar `empty_close_seconds` | Log `[InstanceManager] instância N fechada (vazia)`; **Consultar zones** não lista mais |
| I8 | A morre na instância | Renasce na entrada da cópia (não no spawn da zone) |
| I9 | A fecha o client dentro da instância e reloga | Volta na frente do portal (autosave) |
| I10 | Nível abaixo do mínimo / molde exige grupo / longe do portal | Mensagem de recusa na tela; sem teleporte |
| I11 | Molde com tempo limite 1 min | Ao zerar: "O tempo da instância acabou." e volta ao portal |
| I12 | Manager → **Fechar cópia** com jogador dentro | Jogador volta ao portal com "Instância encerrada pelo administrador." |
| I13 | `map_path` errado no molde | "Não foi possível carregar o mapa da instância." e volta ao portal (ou timeout de 20 s) |
| I14 | Sala aberta em grupo; B clica **Sair** (ou o X) | Janela some para os dois; A vê "B cancelou a entrada." |
| I15 | Sala aberta; A desmarca Pronto | Volta "Aguardando"; botão volta a "Pronto" |
| I16 | Sala aberta e ninguém confirma por 120 s | Fecha com "O tempo para confirmar acabou." |
| I17 | Sala aberta; B fecha o client | Fecha para A com "B desconectou." |
| I18 | B abaixo do nível mínimo no grupo | Linha de B em vermelho "Nível N necessário"; botão Pronto de B desabilitado; a sala não parte |
| I19 | A e B dentro da cópia; C (do grupo) chega e abre o portal | Sala só de C com "Seu grupo já está na instância" e botão **Entrar** → C cai na cópia de A e B |
| I20 | Grupo de 3 num molde com máx. 2 | Recusa "Seu grupo tem mais jogadores que o máximo desta instância (2)." |

Logs esperados na zone:
- `[InstanceManager] criada instância N (molde M 'INST_TEST', slot S, ...)`;
- `[CombatCoreEngine] instância N (molde M): K NPC(s) clonados`;
- `[CombatCoreEngine] player P camada 0→N: despawn=X spawn=Y`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Interagir com o portal abre o diálogo normal | Spawn antigo sem o campo trailing, ou vínculo não salvo | Reiniciar a zone; conferir a coluna **Portal** na lista de NPCs; reentrar no mapa |
| "Instância indisponível no momento." | Molde inativo, sem `map_path` ou de outra zone | Manager → Instâncias: **Ativa**, level e zone (0 = qualquer) |
| Pawn cai no vazio ao entrar | Sem chão abaixo de entrada.Z + 500 no level | Ajustar a entrada local ou o chão do mapa |
| Pawn fica travado e volta depois de 20 s | Level não carregou (path errado ou mapa não salvo) | Conferir o soft path `/Game/...Lvl_X.Lvl_X` e o asset salvo |
| Sol/céu duplicados ou muito claros | Level da instância tem Directional Light/Sky | Remover do level da instância |
| Skills recusadas "Alvo muito distante" com o alvo do lado; log `teleport distance too high` | Cliente e servidor dessincronizados (antes: queda no vazio enquanto o level carregava) | Corrigido: o 123 espera o level aparecer, a trava de entrada dura 30 s e o servidor corrige o cliente depois de ~1 s de recusas (`Player N dessincronizado: correção autoritativa`) |
| "Não foi encontrado chão na entrada da instância" | Entrada (local) sem chão abaixo em até 10 s | Ajustar a entrada no Manager ou o chão/landscape do level |
| NPC do molde aparece no mundo aberto | Vínculo salvo sem reload | **Salvar vínculo** de novo (manda `reload_npc_instances`) ou reiniciar a zone |
| Cópias se enxergando | Espaçamento menor que o level + 30 000 | Aumentar o espaçamento no molde |
| `tabela instance_templates ausente` no log | SQL não aplicado | Rodar `create_instances.sql` e reiniciar a zone |

## 8) Referência de código

- **Servidor:**
  - `src/zone/InstanceManager.{hpp,cpp}`;
  - `src/zone/NpcManager.{hpp,cpp}`;
  - `src/zone/CombatCoreEngine.{hpp,cpp}`;
  - `src/zone/MovementServer.hpp`;
  - `src/zone/MovementProtocol.hpp` (134–136, trailing no 100);
  - `src/zone/NpcAiSystem.cpp`;
  - `src/zone/ZoneServer.{hpp,cpp}`;
  - `src/admin/ServiceAdminRegister.cpp`;
  - `tests/test_instances.cpp`.
- **PHP/SQL:**
  - `scripts/create_instances.sql`;
  - `api/admin/instance_admin_helpers.php`;
  - `api/admin/{list,create,update}_instance_template(s).php`;
  - `api/admin/spawn_npc.php`, `update_npc_instance.php`, `list_npc_instances.php`.
- **Manager:**
  - `Models/InstanceTemplateRow.cs`;
  - `ViewModels/MainViewModel.Instances.cs`;
  - `Views/InstancesEditorView.xaml`;
  - `Views/Npcs/NpcEditorView.xaml` (vínculo);
  - `Models/NpcInstanceRow.cs`.
- **UE:**
  - `Data/UmbraInstanceTypes.h`;
  - `Core/UmbraGameInstanceInstances.cpp`;
  - `Core/UmbraGameInstance.{h,cpp}`;
  - `Network/NetMovementClient.cpp` (forward do 135);
  - `UI/UmbraInstanceHudWidget.*`.

**Fim do guia.**
