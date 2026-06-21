# Guia Combat V2 — Dano autoritativo, ataque básico e NPC



> Complementa [`GUIA_SISTEMA_COMBATE.md`](GUIA_SISTEMA_COMBATE.md) (V1, opcodes 86–95).  

> **Princípio:** cliente envia intenção; Zone Server C++ calcula e faz broadcast; PHP persiste/admin.



---



## 1. O que já existia (não alterado)



| Componente | Função |

|------------|--------|

| `UUmbraCombatFloatingTextComponent` | DoT/HoT (**93**), dano/cura (**92**), poções (**95**) |

| `apply_vitals.php` / opcode **88** | Fluxo legado cliente→PHP→rebroadcast |

| `ZoneCombatService` | DoT DB + respawn |



**Combat V2 adiciona** opcodes **96–103** em paralelo — handlers **92/93/95** permanecem iguais.



---



## 2. Arquitetura



```

Cliente UE --96/98--> Zone (CombatCoreEngine) --97/99/103--> Cliente UE

                              |

                              +--> MySQL (players, npc_instances)

Admin PHP --spawn_npc--> MySQL npc_instances --> Zone reload --> opcode 100

```



---



## 3. Banco de dados



Script: [`www/umbra_api/scripts/combat_v2.sql`](../../www/umbra_api/scripts/combat_v2.sql)



```bat

mysql -u root -p umbra_eternum < D:\UmbraServerV2\www\umbra_api\scripts\combat_v2.sql

```



Tabelas: `basic_attacks`, `npc_templates`, `npc_instances`, ALTER em `skills`.



**Verificação rápida:**



```sql

SELECT COUNT(*) FROM basic_attacks;                    -- esperado: 6

SELECT npc_template_id, npc_name, skeletal_mesh_path FROM npc_templates;

SELECT * FROM npc_instances WHERE zone_id = 1;

```



Template padrão `dummy_treino` usa mesh Mannequin (`SKM_Manny`) — fallback também no C++ se o path falhar.



**Skills de ataque básico (91–96) + backfill:**



Ordem de execução no MySQL:



```bat

mysql -u root -p umbra_eternum < D:\UmbraServerV2\www\umbra_api\scripts\combat_v2.sql

mysql -u root -p umbra_eternum < D:\UmbraServerV2\www\umbra_api\scripts\add_basic_attack_skills.sql

mysql -u root -p umbra_eternum < D:\UmbraServerV2\www\umbra_api\scripts\grant_basic_attack_existing_players.sql

```



| Script | Função |

|--------|--------|

| `add_basic_attack_skills.sql` | Coluna `is_basic_attack` + INSERT skills **91–96** (espelham `basic_attacks`) |

| `grant_basic_attack_existing_players.sql` | Concede skill `is_basic_attack=1` da classe em `player_skills` (idempotente) |



**Importante:** Golpe da Ruína (`skill_id=1`, `skill_order=1`) **não** é ataque básico — é skill da árvore, aprendida via pontos. Ataque básico na skillbar = **skill_id 91–96** por classe:



| class_id | skill_id | skill_key |

|----------|----------|-----------|

| 1 Barbarian | 91 | BARB_BASIC_ATTACK |

| 2 Templar | 92 | TEMP_BASIC_ATTACK |

| 3 Dark Mage | 93 | DMAGE_BASIC_ATTACK |

| 4 Cleric | 94 | CLERIC_BASIC_ATTACK |

| 5 Assassin | 95 | ASSN_BASIC_ATTACK |

| 6 Monk | 96 | MONK_BASIC_ATTACK |



**Migração:** jogadores que receberam Golpe da Ruína só pelo grant antigo mantêm a row em `player_skills` — remoção manual se indesejado. Re-arrastar **Ataque Básico** (91–96) na skillbar.



---



## 4. PHP (WAMP)



Copiar para `C:\wamp64\www\umbra_api\`:



| Repo | WAMP |

|------|------|

| `api/npc/spawn_npc.php` | idem (corrigido: lê `npc_template_id`, `zone_id`, `pos_x/y`) |

| `api/npc/get_npc_templates.php` | idem |

| `api/npc/update_npc_template.php` | idem |

| `api/admin/list_npc_templates.php` | idem (UmbraManager) |

| `api/admin/create_npc_template.php` | idem |

| `api/admin/update_npc_template.php` | idem |

| `api/admin/spawn_npc.php` | idem (auth `admin_username`) |

| `api/admin/list_npc_instances.php` | idem (UmbraManager aba Instâncias) |

| `api/admin/delete_npc_instance.php` | idem (excluir dummy de teste) |

| `api/combat/log_damage.php` | idem |



**Spawn dummy (admin JWT):**



```bat

curl -X POST http://localhost/umbra_api/api/npc/spawn_npc.php ^

  -H "Content-Type: application/json" ^

  -d "{\"token\":\"<JWT_ADMIN>\",\"npc_template_id\":1,\"zone_id\":1,\"pos_x\":1000,\"pos_y\":0,\"pos_z\":200,\"yaw\":0}"

```



**Hot spawn:** após INSERT no MySQL, o UmbraManager chama `spawn_npc_instance` no `zone_{id}` via Admin Channel — opcode 100 broadcast a todos os clientes conectados. Reiniciar o zone só é necessário se o serviço estiver offline.



---



## 5. Servidor C++ Zone



### Build



```bat

cd D:\UmbraServerV2\build

cmake --build . --config Release --target zone_server

```



### Arquivos novos



- `src/zone/CombatCoreEngine.{hpp,cpp}`

- `src/zone/NpcManager.{hpp,cpp}`

- `src/services/SkillService.cpp`



### Zone ID vs porta



| Comando | zone_id | Porta WS |

|---------|---------|----------|

| `zone_server.exe` | 0 | 8082 |

| `zone_server.exe 1` | 1 | 8083 |



O `zone_id` do spawn **deve coincidir** com o argumento do executável.



### Logs esperados



```

[SkillService] N skills carregadas do DB    (N > 0)

[CombatCoreEngine] basic_attacks carregadas: 6

[NpcManager] carregadas N instancias de NPC para zone_id=1

```



### Opcodes 96–103



| Opcode | Nome | Direção |

|--------|------|---------|

| 96 | SkillCastNotify | C→S |

| 97 | SkillCastBroadcast | S→C |

| 98 | BasicAttackNotify | C→S |

| 99 | BasicAttackBroadcast | S→C |

| 100 | NpcSpawnNotify | S→C |

| 101 | NpcDespawnNotify | S→C |

| 102 | NpcStateUpdate | S→C |

| 103 | NpcCombatEvent | S→C |



**Fase atual:** dano placeholder (`-50` skill, `-powerCoef/2` basic). Próximo passo: cabear `CombatCalculator` completo.



---



## 6. Cliente UE 5.6.1



### Arquivos C++ (Combat V2 + NPC dummy)



| Arquivo | Função |

|---------|--------|

| `Components/UmbraCombatComponent` | Animação 97/99 |

| `Components/UmbraBasicAttackComponent` | LMB → 98 |

| `Actors/UmbraNpcCharacter` | Spawn opcode 100, mesh, floating text, barra HP |

| `UI/UmbraNpcOverheadWidget` | Nameplate + barra de vida (100% C++) |



### Alterações existentes



- `UmbraGameInstance` — send 96/98, handlers 97/99/100–103, passa mesh paths do opcode 100

- `UseSkill` — `bUseAuthoritativeSkillCast=true` → opcode **96**

- `NetMovementClient` — roteia 97/99/100–103

- `UmbraEternumUECharacter` — `BasicAttackAction` + componentes

- `UmbraPlayerSelectionComponent` — **clique direito** em NPC → `SetFollowTargetNpcId`



### Blueprint / Input (único passo BP obrigatório)



1. Criar `IA_BasicAttack` em `/Game/Input/Actions/` (se ainda não existir)

2. Mapear LMB em `IMC_Default` no `BP_ThirdPersonCharacter`

3. Assign `BasicAttackAction` no personagem



**Não é necessário** criar `BP_UmbraNpc` para lógica — tudo está em C++.



### Classe NPC (`AUmbraNpcCharacter`)



Criada automaticamente pelo `UmbraGameInstance` ao receber opcode **100**. Inclui:



- Mesh do template SQL (fallback: `SKM_Manny`)

- `UUmbraCombatFloatingTextComponent` (opcode **103**)

- `UUmbraNpcOverheadWidget` (nome, level, barra HP)



### Targeting NPC



1. **Clique direito** no dummy no mundo → `SetFollowTargetNpcId` via `UmbraPlayerSelectionComponent`

2. LMB (ataque básico) → opcode **98** com `target_type=2`

3. Alternativa programática:



```cpp

GameInstance->SetFollowTargetNpcId(NpcInstanceId);  // tipo 2

```



---



## 7. Checklist E2E (executar na ordem)



### 7.1 Infraestrutura



- [ ] MySQL rodando, tabelas Combat V2 aplicadas

- [ ] WAMP + APIs PHP copiadas

- [ ] `zone_server.exe <zone_id>` com mesmo id do spawn

- [ ] Cliente UE compilado (Release/Debug)



### 7.2 Spawn do dummy



- [ ] `curl spawn_npc.php` retorna `npc_instance_id`

- [ ] Reiniciar zone server

- [ ] Log: `[NpcManager] carregadas 1 instancias ... zone_id=<id>`



### 7.3 Cliente in-game



- [ ] Login → personagem → WS conectado

- [ ] Log: `[UmbraGameInstance] NpcSpawnNotify: id=...`

- [ ] Dummy visível (Mannequin) com nameplate acima



### 7.4 Combate



- [ ] **Clique direito** no dummy → log `NPC alvo de combate selecionado`

- [ ] LMB → log `BasicAttackNotify: type=2 target=<npc_id>`

- [ ] Floating text de dano no dummy (opcode **103**)

- [ ] Skill hotkey → opcode **96** → **97** + **103**



### 7.5 Regressão V1



- [ ] DoT/HoT ainda disparam via opcode **93** (sem mudança)

- [ ] Poções/consumíveis via **95** intactos



---



## 8. Teste E2E resumido



1. MySQL + WAMP + `zone_server.exe 1` (porta 8083)

2. Spawn dummy via curl admin (`zone_id=1`)

3. Reiniciar zone server

4. PIE: login → conectar WS → ver NPC (opcode 100)

5. Clique direito no dummy → LMB → floating text **103**

6. Skill hotkey → **96** → **97** + dano



---



## 9. Troubleshooting



| Problema | Causa |

|----------|--------|

| spawn_npc 500 | Variáveis PHP não lidas — usar versão corrigida do repo |

| NPC não aparece | Reiniciar zone após INSERT; conferir `zone_id` |

| NPC invisível | Path mesh inválido — fallback C++ usa Mannequin |

| LMB não envia 98 | `BasicAttackAction` não assignado ou sem alvo NPC |

| Sem floating text | NPC sem `UUmbraCombatFloatingTextComponent` (já no C++) |

| Clique não seleciona NPC | Usar **clique direito** (mesmo fluxo de seleção de player) |

| Dano sempre -50 / ~-40 | Placeholder intencional até cabear `CombatCalculator` |

| 0 skills no zone | Colunas `skills` desalinhadas — ver `SkillService.cpp` |

| Skillbar mostra Golpe da Ruína | Arrastar **Ataque Básico** (skill_id 91–96), não skill_order=1 da árvore |

| Drag ataque básico falha | Rodar `add_basic_attack_skills.sql` + `grant_basic_attack_existing_players.sql` |



---



## 10. Unreal — Input Basic Attack (LMB → opcode 98)



Referência de padrão Input Action: [`GUIA_HUD_SKILLBOOK_BOTAO_INPUT.md`](GUIA_HUD_SKILLBOOK_BOTAO_INPUT.md)



### 10.1 Criar `IA_BasicAttack`



1. Content Browser → **Input** → **Input Action**

2. Nome: `IA_BasicAttack`

3. Value Type: **Digital (bool)**



### 10.2 Mapear no `IMC_Default`



1. Abrir `IMC_Default` (ou o Mapping Context padrão do personagem)

2. Adicionar mapping: **Input Action** = `IA_BasicAttack`, **Key** = **Left Mouse Button**



### 10.3 Assign em `BP_ThirdPersonCharacter`



1. Abrir `BP_ThirdPersonCharacter`

2. No **Class Defaults**, campo **Basic Attack Action** → selecionar `IA_BasicAttack`

3. Compilar e salvar



O C++ (`UUmbraBasicAttackComponent`) escuta essa action e envia opcode **98** quando há alvo NPC (`target_type=2`).



### 10.4 Verificação



PIE → Output Log deve mostrar:



```

BasicAttackAction vinculado

```



Clique direito no dummy → LMB → log `BasicAttackNotify: type=2 target=<npc_id>`.



---



## 11. Unreal — WBP_SkillBook1 + skillbar (Ataque Básico 91–96 → opcode 96)



Guias relacionados:



- [`PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md`](PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md) — estrutura SkillBook

- [`GUIA_WBP_SKILLBAR.md`](GUIA_WBP_SKILLBAR.md) — drag SkillBook → SkillBar



**Dois caminhos paralelos:** LMB → opcode **98** (`basic_attacks`); skillbar com **Ataque Básico** (91–96) → opcode **96**. Dano unificado virá na fase 14 (`CombatCalculator`).



### 11.1 Pré-requisitos (SQL + API + C++)



1. Rodar `add_basic_attack_skills.sql` + `grant_basic_attack_existing_players.sql` (seção 3)

2. `get_available_skills.php` força `is_basic_attack=1` como **aprendida** (`is_learned=true`, `current_rank>=1`)

3. Cliente C++ reforça em `OnLoadAvailableSkillsComplete` (`bIsBasicAttack` → learned)



Sem isso, o drag do ataque básico no SkillBook falha (C++ exige `bIsLearned || CurrentRank > 0`).



### 11.2 WBP_SkillBook1



1. **Reparent** → parent class `UmbraSkillBookWidget`

2. **Viewport Z Order When Shown** = `10`

3. Em `BP_PlayerController` → **Player Skill Book Widget Class** = `WBP_SkillBook1`



### 11.3 PopulateSkillList (Event Graph)



Bind em `OnAvailableSkillsLoaded` (GameInstance):



1. `Get Available Skills` do GameInstance

2. Filtrar: `status.is_learned == true` (inclui `is_basic_attack`; **não** inclui Golpe da Ruína até aprender com pontos)

3. Para cada skill:

   - Create `WBP_SkillBookEntry`

   - `SetSkillData`

   - `SetSkillBookRef(self)`

   - AddChild em `Skills_ScrollBox`



### 11.4 WBP_SkillBookEntry + WBP_SkillBar



- **WBP_SkillBookEntry**: parent `UmbraSkillBookEntryWidget`, botão `BTN_SkillSelect`; drag é C++

- **WBP_SkillBar**: slots com parent `UmbraSkillSlotWidget`; drop chama `SetSkillbarSlot` via C++



### 11.5 Teste



1. Abrir SkillBook → filtro **Aprendidas** → arrastar **Ataque Básico** (ex. skill_id **91** Barbarian) para slot 1 da skillbar

2. Hotkey do slot → log opcode **96** (`UseSkill`)

3. Com NPC selecionado (clique direito) → **97** + floating text **103**



---



## 12. Unreal — Teste NPC dummy in-game



### 12.1 Spawn



Via UmbraManager (seção 13) ou curl admin:



```bat

curl -X POST http://localhost/umbra_api/api/admin/spawn_npc.php ^

  -H "Content-Type: application/json" ^

  -d "{\"admin_username\":\"<ADMIN>\",\"npc_template_id\":1,\"zone_id\":1,\"pos_x\":1000,\"pos_y\":0,\"pos_z\":200,\"yaw\":0}"

```



### 12.2 Reiniciar zone



```bat

zone_server.exe 1

```



Log esperado: `[NpcManager] carregadas N instancias ... zone_id=1`



### 12.3 PIE



1. Login → personagem → WS conectado

2. Log: `[UmbraGameInstance] NpcSpawnNotify: id=...` (opcode **100**)

3. Dummy visível (Mannequin) com nameplate

4. **Clique direito** no dummy → LMB (98) ou skillbar slot 1 (96)

5. Floating text de dano (opcode **103**)



---



## 13. UmbraManager — aba NPCs



Executável (após publish): `dist/UmbraManager/UmbraManager.exe` — ver [`GUIA_UMBRA_MANAGER.md`](GUIA_UMBRA_MANAGER.md) (build vs publish).



Código-fonte: `tools/UmbraManagerWpf/UmbraManager/`



### 13.1 Login



Mesmo fluxo da aba **Items**: autenticação via `admin_username` em `verify_admin.php`.



### 13.2 Aba NPCs (Templates | Instâncias no mundo)

A aba **NPCs** tem duas sub-abas:

#### Templates

| Ação | Descrição |
|------|-----------|
| **Atualizar** | Lista templates via `api/admin/list_npc_templates.php` |
| **Novo template** | Limpa formulário para INSERT |
| Seleção na lista / **Editar** | Carrega template no formulário (`SelectedNpcTemplate`) |
| **Salvar template** | CREATE ou UPDATE conforme ID |
| **Spawn na zone** | INSERT em `npc_instances` via `api/admin/spawn_npc.php` (usa template selecionado ou em edição) |
| **Copiar posição do player** | Preenche X/Y/Z do spawn a partir do primeiro player online na mesma zone |

Campos spawn: `zone_id` (default 1), `pos_x/y/z` (z default 200), `yaw`.

Campos template: stats completos + `skeletal_mesh_path` + `anim_blueprint_path`.

#### Instâncias no mundo

Grid estilo aba **Players** (dados em `npc_instances` no MySQL):

| Coluna | Campo |
|--------|--------|
| Instance ID | `npc_instance_id` |
| Template | `npc_name` + `#template_id` |
| Zone | `zone_id` |
| X / Y / Z | `pos_x`, `pos_y`, `pos_z` |
| Yaw, HP, Morto, Criado | … |

Toolbar: **Atualizar instâncias**, filtro **Zone ID** (0 = todas), contador de linhas.

Menu de contexto: **Preencher spawn** (volta coords para aba Templates), **Excluir instância** (`delete_npc_instance.php`).

Após spawn bem-sucedido, a lista de instâncias é atualizada automaticamente.

### 13.3 Fluxo operacional

1. Rodar `scripts\sync_umbra_api_admin_npc.bat` no WAMP
2. Recompilar `zone_server` e publicar UmbraManager (`scripts\package_umbra_manager.bat`)
3. UmbraManager → **Servers** → garantir **`zone_1`** online (mesmo `Zone ID` do spawn)
4. **NPCs → Templates** → selecionar template → **Spawn na zone**
5. PIE conectado → dummy aparece **imediatamente** (sem restart)
6. **NPCs → Instâncias no mundo** → confirmar linha com X/Y/Z
7. Workbench: `SELECT * FROM npc_instances`
8. Combate (seções 10–12); morte → opcode 101; respawn ~10s → opcode 100

**Comandos admin zone (GM Console):**

- `spawn_npc_instance npc_instance_id=N` — hot spawn de instância já no MySQL
- `reload_npc_instances` — sincroniza novas instâncias do DB
- `list_npcs` — NPCs carregados em runtime (posição live)



---



## 14. Próximas fases



1. `CombatCalculator` + cache `CharacterState` do DB

2. `respawn_seconds` por template (hoje fixo 10s em `NpcManager::kDefaultRespawnSeconds`)

3. Batch `log_damage.php` a partir do zone

4. VFX/SFX Niagara nos paths SQL

5. Range check + PvP modifier


