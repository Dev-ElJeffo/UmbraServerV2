# PLANO — Etapa 5b: chefes com fases (UE 5.6.1)

Aprofunda as dungeons da etapa 5: o chefe muda de comportamento ao perder vida (fases), pode chamar ajudantes (adds), fica furioso se a luta demorar (enrage) e se reinicia se o grupo morrer ou fugir (reset). Tudo é configurado no Manager. O servidor é dono da lógica; o cliente só mostra (barra do chefe, aviso de fase, animação).

Estado em 24/11/2026: **aprovado e implementado** (falta compilar o UE, montar o WBP_BossBar e o QA). Guia: `GUIA_CHEFES_FASES_UE561.md`. Mudanças em relação ao plano: sem flag de chefe no opcode 100 (a barra nasce do 140) e sem a coluna `adds_count_as_enemies` (adds nunca contam, decisão 5).

## 1) O que existe hoje (base)

| Peça | Onde | Como funciona |
|------|------|---------------|
| Skills de NPC | `npc_template_skills` → `NpcRuntimeInstance::boundSkills` | Lista fixa por template; a IA sorteia por peso entre as prontas e no alcance (`NpcAiSystem::pickBoundSkillId`) |
| Dano do NPC | `CharacterStateLoader::makeNpcAttackerState(inst)` | Usa `physicalAttack`/`magicAttack` do NPC (os clones da dungeon já vêm × dificuldade) |
| Imunidade a CC | `npc_templates.cc_immune` | Chefe ignora STUN/SILENCE/ROOT/SLOW |
| Papel na dungeon | `npc_instances.dungeon_role` | `boss`/`final_boss` contam para a conclusão (etapa 5) |
| Clones da cópia | `NpcManager::spawnInstanceClones` (combat worker, job `WorldOp`) | Só NPCs do molde; não há spawn de NPC "avulso" por template |
| Estado do NPC no cliente | opcodes 100 (spawn), 102 (estado/HP), 103 (combate) | Sem noção de fase/chefe |

## 2) Como fica para o jogador

1. **Barra do chefe:** ao entrar em combate com um chefe, aparece no topo da tela uma barra grande com:
   - nome, vida em %, nome da fase ("Fase 2 — Fúria") e marcas nos limiares de fase;
   - "Enfurece em 2:30" quando o chefe tem enrage.
2. **Troca de fase:** ao cruzar o limiar de vida (ex.: 70%, 40%), o chefe:
   - anuncia uma fala no centro da tela ("Vocês vão pagar por isso!");
   - toca a animação de transição;
   - pode ficar **invulnerável por N s** ("Imune" nos números);
   - passa a usar o **conjunto de skills da fase** e os **multiplicadores da fase** (dano, velocidade, intervalo de ataque);
   - pode **chamar adds** em volta dele.
3. **Enrage:** depois de N s em combate, o dano é multiplicado e aparece o aviso "Fulano se enfurece!".
4. **Reset (wipe ou fuga):** se o chefe perde o alvo e volta ao ponto de origem (leash), ou se todos morrem:
   - volta à fase 1 com vida cheia;
   - os adds somem;
   - o enrage zera.
5. **Adds:** são NPCs comuns da cópia. Contam em "Inimigos" do HUD da dungeon só se o molde marcar; por padrão **não** contam, para não travar a conclusão.

## 3) Decisões (recomendação; mudar antes de executar)

| # | Decisão | Recomendação |
|---|---------|--------------|
| D1 | Gatilho de fase | **% de vida** (limiar por fase). Tempo só no enrage |
| D2 | Onde valem as fases | Em qualquer lugar (mundo aberto também). **Adds só dentro de instância** nesta etapa (limpeza garantida ao fechar a cópia) |
| D3 | Pode voltar de fase? | Não: as fases só avançam. O reset volta tudo para a fase 1 |
| D4 | Fase pulada por dano alto | Aplica as fases intermediárias em sequência no mesmo instante: anuncia só a última e chama os adds de todas |
| D5 | Invulnerabilidade na transição | Opcional por fase (0 = não fica) |
| D6 | Configuração | Por **template** de NPC (todo NPC daquele template é chefe com essas fases) |

## 4) Dados (SQL `scripts/add_boss_phases.sql`, idempotente)

| Tabela | Campos | Uso |
|--------|--------|-----|
| `npc_boss_configs` | `npc_template_id` (PK), `enrage_seconds` (0 = sem), `enrage_damage_mult`, `enrage_text`, `reset_on_leash` (1), `show_boss_bar` (1), `adds_count_as_enemies` (0) | Regras gerais do chefe |
| `npc_boss_phases` | `phase_id`, `npc_template_id`, `phase_order`, `name`, `hp_threshold_pct` (entra quando a vida ≤ X%; a fase 1 é 100), `damage_mult`, `speed_mult`, `attack_cooldown_mult`, `invulnerable_ms`, `announce_text`, `transition_anim_path` (soft path), `transition_vfx_path` | Uma linha por fase |
| `npc_boss_phase_skills` | `phase_id`, `npc_skill_id`, `skill_rank`, `weight`, `cooldown_override_ms` | Conjunto de skills da fase. Sem linhas = usa as skills do template |
| `npc_boss_phase_adds` | `phase_id`, `npc_template_id`, `count`, `radius`, `once` (1) | Adds chamados ao entrar na fase |

## 5) Protocolo

| Op | Direção | Formato | Quando |
|----|---------|---------|--------|
| **140** `BossState` (novo) | S→C (camada do chefe) | `[140][npcId:u32][phase:u8][phaseCount:u8][flags:u8 bit0 enfurecido, bit1 invulnerável, bit2 em combate][enrageInSec:u16][invulnerableMs:u16][phaseName:u16+utf8≤64][announce:u16+utf8≤200][animPath:u16+utf8≤255][thresholds:u8 n]{pct:u8}` | Entrada em combate, troca de fase, enrage, reset e snapshot de quem entra na camada |
| **100** (trailing) | S→C | flags bit4 = chefe (o cliente cria a barra ao entrar em combate) | Spawn |

A vida continua chegando pelo **102**. O 140 só leva o que muda pouco.

## 6) Arquivos por camada

| Camada | Arquivos | O que muda |
|--------|----------|------------|
| SQL | `www/umbra_api/scripts/add_boss_phases.sql` | As 4 tabelas + seed de exemplo (template do chefe final do INST_TEST com 3 fases) |
| Zone | `src/zone/BossController.{hpp,cpp}` (novo) | Configs em memória (`reload_boss_configs`); estado por NPC (fase, início do combate, enrage, invulnerável até, adds vivos); `onDamaged` (limiares, D4), `tick` (enrage, reset), funções puras para teste |
| Zone | `src/zone/NpcManager.{hpp,cpp}` | Campos de runtime no NPC (`bossPhase`, multiplicadores, `invulnerableUntil`, skills base guardadas); troca do conjunto de skills; **protótipo por template** para os adds (SELECT do template com colunas de instância neutras, numa query à parte; o SELECT posicional atual fica igual); `spawnTemplateClones(templateId, layer, x, y, z, count, radius)` |
| Zone | `src/zone/CombatCoreEngine.{hpp,cpp}` | Depois do dano no NPC → `BossController::onDamaged` (combat worker); dano com `invulnerableUntil` → "Imune"; multiplicadores no atacante; flag de chefe no 100; snapshot do 140 na troca de camada |
| Zone | `src/zone/CharacterStateLoader.cpp` | `makeNpcAttackerState` aplica o `damage_mult` da fase e o enrage |
| Zone | `src/zone/NpcAiSystem.cpp` | `speed_mult` e `attack_cooldown_mult` da fase; ao voltar ao home (leash) → `BossController::onReset` |
| Zone | `src/zone/InstanceManager.cpp` | Adds não contam para a conclusão (a menos que `adds_count_as_enemies`); fechamento da cópia remove os adds (já remove os clones) |
| Zone | `MovementProtocol.hpp`, `ZoneServer.*`, `ServiceAdminRegister.cpp` (`reload_boss_configs`), `src/zone/CMakeLists.txt` | 140, flag no 100, ligação |
| Testes | `tests/test_boss_phases.cpp` | Limiares (inclui pular fases, D4), enrage, reset, protocolo 140 |
| PHP | `api/admin/get_boss_config.php`, `save_boss_config.php` (config + fases + skills + adds num JSON só, transação) | CRUD |
| Manager | `Models/BossPhaseRow.cs`, `BossPhaseSkillRow.cs`, `BossPhaseAddRow.cs`; `ViewModels/MainViewModel.BossPhases.cs`; nova sub-aba **Chefe** no editor de NPC (`Views/Npcs/NpcEditorView.xaml`); `PhpAdminClient.cs` | Grade de fases + skills e adds da fase selecionada + regras de enrage; **Salvar** manda o `reload_boss_configs` |
| UE | `Data/UmbraBossTypes.h` (novo), `Core/UmbraGameInstance.{h,cpp}` (parse do 140, cache por NPC, delegate), `Network/NetMovementClient.cpp` (forward) | Estado do chefe |
| UE | `UI/UmbraBossBarWidget.{h,cpp}` (novo) | Barra no topo: nome, vida (do 102), fase, marcas dos limiares, enrage; some fora de combate/morto |
| UE | `Actors/UmbraNpcCharacter.{h,cpp}` | Tocar a animação de transição (soft path) e o VFX; ver se já existe helper de montage por path antes de criar |
| UE | `UI` do aviso central | Reusar o aviso de tela que já existe (`ShowCombatFeedback`) ou um texto grande na própria barra |
| Editor (MCP, depois de compilar) | `WBP_BossBar` | Eu monto |
| Guia | `docs_main/GUIA_CHEFES_FASES_UE561.md` | Editor, Manager, SQL, protocolo, QA |

## 7) Riscos e cuidados

- **Thread:**
  - a troca de fase acontece no combat worker (onde o dano é aplicado), então os adds nascem ali mesmo, sem `WorldOp` extra;
  - o enrage e o reset rodam no tick da IA, sob o lock do `NpcManager`: só marcar flags ali e fazer os broadcasts **fora** do lock (o mesmo deadlock da etapa 4).
- **SELECT posicional:** o protótipo por template não pode mexer no SELECT existente. A query nova fica separada e é testada no dev antes.
- **Adds no mundo aberto:** ficam de fora (D2) até haver limpeza garantida.
- **Balanceamento:** os números do seed são só exemplo; o balanceamento geral continua para depois.

## 8) QA previsto (resumo; vai completo no guia)

- **B1:** a barra do chefe aparece ao entrar em combate, com o nome da fase e as marcas.
- **B2:** em 70%, a fase 2 anuncia, fica imune por 2 s, troca as skills e chama 2 adds.
- **B3:** um dano grande de 80% para 30% aplica as fases 2 e 3 de uma vez.
- **B4:** o enrage dispara no tempo e o dano sobe.
- **B5:** com wipe ou fuga, o chefe volta à fase 1 com vida cheia e os adds somem.
- **B6:** a conclusão da dungeon ignora os adds.
- **B7:** um segundo jogador entrando na cópia no meio da luta vê a fase atual (snapshot).
- **B8:** NPC sem config de chefe não muda nada.

## 9) Ordem de execução (amanhã)

1. *(Opcional, se você pedir)* Commit e push das etapas 4 e 5 na `feat/instancias`.
2. SQL no dev.
3. Zone + testes.
4. PHP e sync com o WAMP.
5. Manager.
6. UE C++.
7. Guia.
8. Você compila o UE e eu monto o `WBP_BossBar` via MCP.
9. QA B1–B8.
