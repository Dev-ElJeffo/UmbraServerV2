# GUIA — Progressão de skills por rank (UE 5.6.1)

Fase **R1** do `PLANO_SKILLS_PROGRESSAO.md`: o rank passa a fortalecer **todos** os efeitos da skill (dano, cura, buff, debuff, escudo, DOT/HOT e controle), as passivas e as durações. Inclui a correção do **TELEPORT** (Rift Step), que falhou no QA da revisão.

| Item | Status |
|------|--------|
| Fórmula de crescimento (servidor + ficha PHP) | Implementado, falta QA |
| TELEPORT / blink (opcode 123) | Implementado, falta QA |
| Manager: campo `rank_growth_percent` | Fica para a R5 (por enquanto, via SQL) |
| **R2** — custo escalonado, nível por rank, ultimates com 3 ranks, reset por gold | QA ok |
| **R3** — retorno decrescente de CC, chefe imune, teto de esquiva | Implementado; QA junto com a R4 (§7.6) |
| **R4** — kits revisados (90 skills) + furtividade, puxar, área ao redor | Implementado, falta QA (§8) |

---

## 0) Pré-requisitos

1. **SQL** (idempotente):
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skills_rank_growth.sql`
   - Cria `skills.rank_growth_percent` (NULL = padrão).
   - Zera `skill_rank_scaling.power_coef_bonus` (era contagem dupla com o +10% implícito).
2. Parar a zone, recompilar e reiniciar:
   `cd D:\UmbraServerV2\build; cmake --build . --config Release --target zone_server`
3. PHP já sincronizado com o WAMP (`helpers/character_info_helper.php`).
4. **Compilar o UE**: `Core/UmbraGameInstance.{h,cpp}` e `Network/NetMovementClient.cpp`.
5. Reiniciar o PIE.

---

## 1) Fórmula

```
valor(rank) = base × (1 + crescimento × (rank − 1))
```

| Tipo de efeito | Escala | Crescimento padrão | Rank 5 vs rank 1 |
|----------------|--------|--------------------|------------------|
| DAMAGE, HEAL, SHIELD, DOT, HOT, RESOURCE_RESTORE, EXECUTE, LIFESTEAL, MANASTEAL | `value_percent` e `value_flat` | 12,5%/rank | +50% |
| BUFF_STAT, DEBUFF_STAT, SLOW, REFLECT, passivas | `value_percent` e `value_flat` | 10%/rank | +40% |
| STUN, ROOT, SILENCE, KNOCKBACK, TAUNT, INTERRUPT | `duration_ms`; `chance_percent` (se < 100) +5 pontos/rank | 10%/rank | +40% duração, +20 de chance |
| TELEPORT, CLEANSE, SUMMON, STEALTH, INVULNERABLE... | Não escalam | — | — |

- **Debuff negativo** cresce em módulo: −25% vira −35% no rank 5.
- **Duração de buff** não cresce pela fórmula. Para isso use o `duration_bonus_ms` da linha do rank (Manager → skill → ranks), que agora soma na duração de **cada efeito** com duração (antes não valia).
- **Bônus de rank** (`extra_effects_json`) **não** escalam: já são o valor daquele rank (marcos dos ranks 3 e 5).
- `power_coef` (parte de MAG_ATK das curas e coeficiente do NPC) cresce igual ao DAMAGE. O `power_coef_bonus` da tabela continua existindo como ajuste fino **aditivo**.

### Exemplos

| Skill | Rank 1 | Rank 3 | Rank 5 |
|-------|--------|--------|--------|
| Golpe da Ruína (DAMAGE 180%) | 180% | 225% | 270% |
| Rugido de Guerra (+20% ATK) | +20% | +24% | +28% |
| Marca Dimensional (−25% MDEF) | −25% | −30% | −35% |
| Golpe Quebrador (STUN 1,5s) | 1,5s | 1,8s | 2,1s |
| Impacto Esmagador (INTERRUPT 60%, silêncio 1,5s) | 60% / 1,5s | 70% / 1,8s | 80% / 2,1s |
| Pele Endurecida (passiva +50 DEF, +15%) | +50 / +15% | +60 / +18% | +70 / +21% |

### Ajustar o crescimento

| Onde | Campo | Efeito |
|------|-------|--------|
| Skill inteira | `skills.rank_growth_percent` (ex.: `15`, `7.5`) | Substitui o padrão em todos os efeitos da skill. `0` = não escala |
| Um efeito | `"rank_growth": 20` no JSON do efeito | Substitui a skill e o padrão só naquele efeito |
| Padrões globais | `defaultRankGrowthTenths` em `src/services/SkillTypes.hpp` | Exige recompilar a zone |

Exemplo (SQL, até a R5 trazer o campo no Manager):
`UPDATE skills SET rank_growth_percent = 15 WHERE skill_key = 'BARB_RUIN_STRIKE';` e depois `reload_skills` (salvar uma skill no Manager) ou reiniciar a zone.

---

## 2) TELEPORT / blink (Rift Step)

- **Causa:** o servidor movia o player e mandava um `StateUpdate` (opcode 2), mas o cliente **descarta** os frames do próprio player. O pawn nunca se movia e a trava de posição do servidor descartava os moves até expirar.
- **Correção:** novo opcode **123 `PlayerPositionCorrection`**, enviado só ao dono. O cliente ajusta o Z pelo chão no destino e usa `TeleportTo`, que procura um ponto livre se cair dentro de geometria. Os outros clients continuam recebendo o `StateUpdate`.
- O blink vai `value_flat` uu na direção para onde o personagem olha (Rift Step = 500 ≈ 5m).
- **Paredes (mode 1):** blink e knockback em player mandam `mode = 1`. O cliente varre o trajeto em trechos de 100uu com a cápsula (subindo `MaxStepHeight` para passar degraus e rampas), acompanha o chão e **para antes da primeira parede** ou de um vão sem chão. O servidor aceita qualquer ponto do trajeto (a trava de posição usa raio = distância + 150uu).
- Teleporte de admin (Manager) usa `mode = 0`: vai direto ao ponto, com `TeleportTo`.

### Protocolo — opcode 123 (Servidor → dono)

```
[0]     u8   msgType = 123
[1..4]  u32  playerId (LE)
[5..8]  f32  x
[9..12] f32  y
[13..16] f32 z
[17..20] f32 yaw
[21]    u8   mode (trailing): 0 = teleporte direto, 1 = varrer o trajeto (blink/knockback)
```

22 bytes. Um cliente antigo lê só os 21 primeiros (mode = 0).

---

## 3) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Fórmula | `src/services/SkillTypes.hpp` (`applyRankToEffect`, `scaleByRankTenths`, `defaultRankGrowthTenths`) | Escala os efeitos base no `buildEffectsForRank` |
| Coeficiente | `SkillData::getEffectivePowerCoef` / `getRankBonusFactor` | Sem contagem dupla; o bônus da tabela é aditivo |
| Carga | `src/services/SkillService.cpp` | Lê `rank_growth_percent` (coluna trailing [49]) e `rank_growth` do efeito |
| Passivas | `src/zone/CharacterStateLoader.cpp`, `helpers/character_info_helper.php` | Usam `player_skills.current_rank` |
| Blink | `MovementServer::teleportPlayer` + `UUmbraGameInstance::HandlePlayerPositionCorrectionMessage` | Opcode 123 só para o dono |
| Testes | `tests/test_combat_effects.cpp` (`SkillRankProgressionTests`) | Dano, buff, debuff, controle, overrides e `duration_bonus_ms` |

---

## 4) QA (PIE, Number of Players ≥ 2)

Dica para testar rápido: suba o rank direto no banco
`UPDATE player_skills SET current_rank = 5 WHERE player_id = <id> AND skill_id = <skill>;`
e relogue (a zone recarrega o rank no login).

| # | Cenário | Esperado |
|---|---------|----------|
| R1 | Mesma skill de dano no rank 1 e no rank 5, mesmo alvo | Dano ~50% maior no rank 5 |
| R2 | Buff (ex.: Rugido de Guerra) no rank 5 | Ficha mostra +28% em vez de +20% |
| R3 | Debuff (Marca Dimensional) rank 5 no NPC | −35% na BuffBar do NPC |
| R4 | Golpe Quebrador rank 5 | Stun de ~2,1s |
| R5 | Passiva (Pele Endurecida) rank 5 | Ficha mostra +70 DEF e +21% |
| R6 | Cura (Tônico de Campo) rank 5 | Cura ~50% maior |
| R7 | Rift Step | O Dark Mage avança ~5m para a frente; o outro client vê o deslocamento |
| R8 | Rift Step de frente para uma parede | Não fica preso dentro da geometria |
| R9 | Skill com `rank_growth_percent = 0` | Mesmo valor em todos os ranks |

Logs: zone `SkillCast dmg player=... hit=...` (o dano já vem escalado); UE `[UmbraGameInstance] PlayerPositionCorrection (123): -> X=...`.

## 5) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Rank 5 igual ao rank 1 | Zone antiga ou `rank_growth_percent = 0` | Reiniciar a zone; conferir a coluna |
| Dano dobrado no rank alto | `power_coef_bonus` não zerado | Rodar `add_skills_rank_growth.sql` |
| Ficha não muda a passiva | PHP não sincronizado | `/sync-wamp` |
| Rift Step não move | UE não recompilado | Compilar o UE |
| Rift Step volta para trás logo depois | Zone antiga (sem o opcode 123) | Recompilar e reiniciar a zone |

---

## 6) R2 — Economia de pontos

### 6.0 Pré-requisitos

1. SQL (já aplicado no banco de dev; idempotente):
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skills_rank_economy.sql`
2. PHP sincronizado com o WAMP (`helpers/skill_rank_helper.php`, `api/skills/upgrade_skill.php`, `get_available_skills.php`, `get_player_skills.php`, `reset_skill_points.php`).
3. A zone **não** muda nesta fase.
4. Compilar o UE: `Core/UmbraGameInstance.{h,cpp}`, `Data/UmbraSkillDataStructures.h`, `UI/UmbraSkillBookWidget.{h,cpp}`, `UI/UmbraSkillBookEntryWidget.cpp`.

### 6.1 Regras

| Regra | Fórmula | Padrão |
|-------|---------|--------|
| Custo do rank N | `skill_cost + rank_cost_step × (N − 1)` | 1, 2, 3, 4, 5 (15 por skill) |
| Nível do rank N | `required_level + rank_level_step × (N − 1)`, limitado ao nível máximo (50) | +4 níveis por rank |
| Ultimates | `skill_cost = 3`, `max_rank = 3` | 3, 4, 5 (12 no total) |
| Reset | `nível × 100` gold | Desaprende tudo menos o ataque básico; devolve todos os pontos |

- Maximizar uma classe inteira custa 14 × 15 + 12 = **222 pontos**, contra **150** no nível 50: dá para maximizar cerca de 2/3 das skills.
- Exemplo: skill de nível 10 → ranks nos níveis 10, 14, 18, 22, 26. Skill de nível 45 → 45, 49, 50, 50, 50.
- Aprender (rank 1) continua custando `skill_cost` no nível `required_level`.
- Personagens existentes: os ranks ficam como estão (ultimates acima de 3 foram para 3). Use o reset para redistribuir pelas regras novas.
- Depois de aprender, melhorar ou resetar, o cliente avisa a zone (opcode 27, `PartyStatsRefresh`), que recarrega ranks e passivas **sem relogar**. Antes o rank novo só valia depois de relogar.
- Correção: o cliente lia `new_rank` / `skill_points_remaining` na resposta do upgrade (campos que a API não devolve) e os pontos zeravam até recarregar.

Ajuste por skill (SQL, até a R5): `UPDATE skills SET rank_cost_step = 2, rank_level_step = 5 WHERE skill_key = '...';`

### 6.2 Editor UE — `WBP_SkillBook`

Parent Class continua `UmbraSkillBookWidget`. Adicionar (opcional; sem ele o reset só não aparece):

| Widget | Nome exato | Tipo |
|--------|-----------|------|
| Botão de reset | `Reset_Button` | Button |
| Texto do botão | `Text_Reset_Button` | TextBlock (filho do botão) |

- Sugestão de lugar: perto do `SkillPoints_Text` (rodapé do livro).
- O texto vem do C++: **"Redistribuir pontos"** → **"Confirmar (N gold)"**.
- Class Defaults: `ResetConfirmSeconds` (6s) em `Umbra|SkillBook`.
- O `Action_Info_1` (já existente) mostra custo, erros e confirmação.

### 6.3 O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Custo/nível do próximo rank | `UUmbraSkillBookWidget::UpdateActionButton` | Botão "Melhorar (N pt)" com o custo real; aviso "Rank N requer nível X" |
| Tag na lista | `UUmbraSkillBookEntryWidget` | "Rank N: Nv X" quando falta nível para o próximo rank |
| Reset em 2 cliques | `HandleResetButtonClicked` / `HandleSkillResetPreview` / `HandleSkillPointsReset` | 1º clique mostra o custo; 2º (em até 6s) confirma; recarrega livro, skillbar, gold e pontos |
| Requisições | `UUmbraGameInstance::PreviewSkillPointsReset` / `ResetSkillPoints` | `reset_skill_points.php` (`preview: true` só consulta) |

### 6.4 QA

| # | Cenário | Esperado |
|---|---------|----------|
| E1 | Melhorar uma skill do rank 1 para o 2 | Botão mostra "Melhorar (2 pt)"; gasta 2 pontos |
| E2 | Rank 3 → 4 | Gasta 4 pontos |
| E3 | Personagem nível 20 com skill de nível 10 no rank 3 | Rank 4 (Nv 22) bloqueado: "Rank 4 requer nível 22"; tag "Rank 4: Nv 22" |
| E4 | Ultimate | Rank máximo 3; custos 3, 4, 5 |
| E5 | Reset: 1º clique | Mostra "Confirmar (nível × 100 gold)" e a info com os pontos |
| E6 | Reset: 2º clique em até 6s | Skills desaprendidas (menos o ataque básico), skillbar sem elas, gold debitado, pontos = total |
| E7 | Reset sem gold suficiente | "Gold insuficiente: o reset custa X" |
| E8 | Melhorar uma passiva e abrir a ficha sem relogar | Stat novo já aparece; na zone, o rank novo vale no próximo cast |
| E9 | Pontos na tela após melhorar | Contador correto (não zera) |

### 6.5 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Botão ainda "(1 pt)" em todo rank | PHP não sincronizado ou UE antigo | `/sync-wamp`; compilar o UE |
| Upgrade 500 "Erro interno" | Migration R2 não rodou | Rodar `add_skills_rank_economy.sql` |
| Botão de reset não aparece | WBP sem `Reset_Button` | Adicionar o bind (§6.2) |
| Rank novo só vale após relogar | UE antigo (sem o `PartyStatsRefresh` pós-upgrade) | Compilar o UE |

---

## 7) R3 — Controle de combate

### 7.0 Pré-requisitos

1. SQL (já aplicado no banco de dev; idempotente):
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_npc_templates_cc_immune.sql`
2. Recompilar a `zone_server` e reiniciar a zone.
3. Compilar o UE: `Core/UmbraGameInstance.{h,cpp}`, `Components/UmbraCombatFloatingTextComponent.{h,cpp}`, `UI/UmbraDamageNumberWidget.cpp`.

### 7.1 Retorno decrescente (players e NPCs)

| Categoria | Efeitos |
|-----------|---------|
| Atordoamento | STUN |
| Silêncio | SILENCE (inclui o silêncio do INTERRUPT com `duration_ms`) |
| Prisão | ROOT e SLOW |

| Aplicação da mesma categoria no mesmo alvo | Resultado |
|--------------------------------------------|-----------|
| 1ª | Duração cheia |
| 2ª dentro de 15s da anterior | **50%** da duração |
| 3ª dentro de 15s | **Não aplica**: texto **"Imune"** e o alvo fica imune àquela categoria por 15s |

- A janela de 15s recomeça a cada CC aplicado; 15s sem CC da categoria zera a contagem.
- Categorias são independentes: um stun não reduz um root seguinte.
- O **cancelamento de cast** (INTERRUPT, knockback) não entra no retorno decrescente; só o silêncio que vem depois dele.
- Knockback e taunt não têm retorno decrescente.

### 7.2 Chefe imune (`npc_templates.cc_immune`)

- `1` = o NPC ignora STUN, SILENCE, ROOT e SLOW (texto "Imune"). INTERRUPT ainda cancela o cast dele; knockback e taunt ainda valem.
- Marcar (SQL, até a R5 trazer o campo no Manager):
  `UPDATE npc_templates SET cc_immune = 1 WHERE npc_template_id = <id>;`
- A zone lê no spawn: reiniciar a zone ou **Recarregar instâncias** no Manager.

### 7.3 Teto de esquiva

| Regra | Valor |
|-------|-------|
| Esquiva somada por **buffs temporários** (Véu de Fumaça, Passo Invisível, Defesa Rotacional...) | No máximo **+50** sobre a esquiva base (atributos, equipamento e passivas) |
| Esquiva efetiva no teste de acerto | Tira no máximo **60** pontos: acerto mínimo = 80 − 60 + precisão do atacante (e nunca abaixo de 5%) |

A ficha (PHP) ainda mostra a esquiva sem o teto dos buffs.

### 7.4 Protocolo

- `CombatReason::Immune = 7` nos opcodes **92** (alvo player) e **103** (alvo NPC), com `delta = 0`. Cliente antigo trata como evento sem número (nada aparece).

### 7.5 O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Retorno decrescente | `CombatCoreEngine::consumeCcDiminishing` | Por alvo (player/NPC) e categoria; aplicado antes do interrupt de cast |
| Chefe imune | `NpcRuntimeInstance::ccImmune` (`NpcManager.cpp`, consulta separada) + `isNpcCcImmune` | Bloqueia o CC antes de rolar o retorno decrescente |
| "Imune" | `broadcastCcImmune` → `UUmbraCombatFloatingTextComponent::ShowImmune` | Mesmo estilo do "Errou" |
| Teto de esquiva | `CombatEffectRuntime::recompose` (`kMaxDodgeFromEffects`), `CombatCalculator::calculateHitChance` (`MAX_DODGE_PENALTY`) | Valem para players e NPCs |
| ROOT / KNOCKBACK / TAUNT | já existentes | ROOT trava o movimento (opcode 118); knockback em player para na parede (opcode 123 mode 1); taunt força o alvo do NPC |
| Teste | `tests/test_combat_effects.cpp` (`DodgeFromEffectsIsCapped`) | Teto de esquiva |

### 7.6 QA (PIE, Number of Players ≥ 2)

| # | Cenário | Esperado |
|---|---------|----------|
| C1 | Golpe Quebrador (stun) 3 vezes seguidas no mesmo NPC (< 15s entre eles) | 1º stun cheio, 2º com metade do tempo, 3º "Imune" |
| C2 | Esperar 15s e repetir | Stun cheio de novo |
| C3 | Stun e logo depois slow/root no mesmo alvo | Ambos cheios (categorias diferentes) |
| C4 | PvP: dois players se atordoando | Mesma regra do C1 |
| C5 | NPC com `cc_immune = 1` | Todo stun/silence/root/slow mostra "Imune"; dano normal |
| C6 | INTERRUPT com silêncio no chefe imune | Cast dele é cancelado; silêncio mostra "Imune" |
| C7 | Assassin com Véu de Fumaça + Passo Invisível | Ainda leva parte dos golpes (esquiva extra limitada a +50) |
| C8 | ROOT em player | Não anda até acabar; skills ainda saem |
| C9 | Knockback em player perto de parede | Para na parede |

Logs: `CC STUN imune (retorno decrescente) alvo=...` (debug).

### 7.7 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| 3º stun ainda pega | Zone antiga | Recompilar e reiniciar |
| "Imune" não aparece, mas o CC não pega | UE antigo | Compilar o UE |
| Chefe ainda atordoa | `cc_immune` marcado com a zone rodando | Reiniciar a zone ou recarregar instâncias |

---

## 8) R4 — Kits revisados

Kits aprovados em `docs_main/R4_PROPOSTA_KITS.md` (valores, marcos dos ranks 3/5 e skills novas por classe).

### 8.0 Pré-requisitos

1. SQL (já aplicado no banco de dev; idempotente). Backup antes da aplicação: tabelas `skills`, `skill_rank_scaling`, `player_skills`, `player_skillbar`.
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\fix_skills_r4_kits.sql`
2. Recompilar a `zone_server` e reiniciar a zone.
3. Reiniciar o PIE (o cliente baixa as skills no login).
4. UE: nenhum arquivo novo nesta fase (usa o que veio da R1–R3).

### 8.1 Motor novo

| Item | Como funciona |
|------|---------------|
| **Furtividade quebra ao agir** | Ataque básico ou skill ENEMY/AREA remove o STEALTH do caster (opcode 104 + 118: o personagem reaparece). Buffs em si (SELF) não quebram |
| **Crítico saindo da furtividade** | `conditions_json.crit_if_stealthed = true` no DAMAGE: crítico garantido se o caster estava furtivo ao resolver o golpe |
| **Puxar** | KNOCKBACK com `value_flat` negativo puxa o alvo até 1,5m do caster (NPC e player; player para na parede) |
| **Área ao redor** | Skill AREA com `range_max = 0` é centrada no caster, mesmo com alvo selecionado (Pisotear, Desafio, Onda do Vazio, Bomba de Fumaça, Chute Giratório) |
| Skills novas | Reaproveitam a linha da skill antiga (ID, nível, ícone, animação). Quem já tinha a antiga aprendida fica com a nova no mesmo rank |

### 8.2 Editor / Manager (sua parte)

Trocar ícone, animação e VFX das skills novas (hoje herdam os da skill substituída): Pisotear, Arremesso de Corrente, Desafio, Correntes Sombrias, Onda do Vazio, Frasco Ácido, Elixir Ofensivo, Elixir Defensivo, Gás Paralisante, Bomba Incendiária, Bomba de Fumaça, Chute Giratório, Varredura. O Tônico de Campo levou os visuais da linha antiga (Nv 1) para o Nv 7.

### 8.3 QA (PIE, Number of Players ≥ 2) — junto com o da R3

Use o personagem com tudo no rank 5 e um de rank baixo para comparar. Por classe, cada skill nova/alterada pelo menos uma vez, conferindo dano, efeito na BuffBar do alvo certo e o marco do rank.

| # | Cenário | Esperado |
|---|---------|----------|
| K1 | Pisotear / Chute Giratório com um NPC selecionado longe | Área acontece **ao redor de você**, não no NPC |
| K2 | Arremesso de Corrente num NPC a 8m | NPC puxado até perto; slow; no rank 5, stun |
| K3 | Arremesso de Corrente em player, com parede entre vocês | Player puxado até a parede |
| K4 | Desafio perto de 3 NPCs atacando um aliado | Os NPCs passam a atacar o Templar; ele recebe redução de dano |
| K5 | Correntes Sombrias | Root 2s no alvo (rank 5: + silêncio 1s) |
| K6 | Onda do Vazio com NPCs colados | Dano e empurrão para longe de você |
| K7 | Passo Invisível → andar | Some para o outro client; NPCs não o atacam |
| K8 | Passo Invisível → Golpe nas Sombras | Reaparece e o golpe é **crítico** |
| K9 | Passo Invisível → ataque básico | Reaparece no primeiro ataque |
| K10 | Bomba de Fumaça | Silêncio 2s nos inimigos ao redor; rank 5: você fica furtivo 2s |
| K11 | Alchemist nível 1: Frasco Ácido | Dano + DOT (agora há skill de dano no início) |
| K12 | Elixir Ofensivo / Defensivo sem alvo | Buff no próprio Alchemist |
| K13 | Gás Paralisante / Bomba Incendiária no meio de NPCs | Todos na área levam dano e slow / DOT |
| K14 | Barreira Rúnica | Escudo de 400 + 25% da vida (não mais 5000) |
| K15 | Retorno decrescente: Pisotear 3× seguidas no mesmo NPC | 2ª com slow pela metade; 3ª "Imune" (o dano sempre entra) |
| K16 | QA da R3 | Cenários C1–C9 (§7.6) |

### 8.4 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Skill nova com nome antigo no livro | PIE antigo | Reiniciar o PIE |
| Área ao redor cai no alvo selecionado | Zone antiga ou `range_max` ≠ 0 | Reiniciar a zone; conferir a skill |
| Furtividade não quebra | Zone antiga | Recompilar e reiniciar |
| Corrente empurra em vez de puxar | Zone antiga ou `value_flat` positivo | Conferir o efeito KNOCKBACK (-500) |
| Quer desfazer a R4 | — | Restaurar o backup das 4 tabelas |

### 8.5 Correções do QA da R4 (furtividade)

| Problema | Correção |
|----------|----------|
| Ao voltar da furtividade, o outro client via o personagem sem nome e título (só voltava ao apertar **C**) | O UE guarda o último nome, título e guilda de cada jogador remoto e reaplica o nameplate 0,15s depois que o opcode 118 tira o STEALTH (`UUmbraGameInstance::ReapplyCachedRemoteNameplateNextTick`) |
| NPC continuava perseguindo quem estava furtivo (só errava os golpes) | A IA da zone ignora jogadores furtivos no aggro por proximidade e perde o alvo quando ele fica furtivo, voltando ao spawn (`CombatCoreEngine::snapshotStealthedPlayers` + `NpcAiSystem::tick`) |

QA extra:

| # | Passo | Esperado |
|---|-------|----------|
| K17 | Client B vê A usar Passo Invisível e depois atacar | A reaparece **com** nome, título e guilda, sem apertar C |
| K18 | A puxa um NPC e usa Passo Invisível | O NPC para de perseguir e volta ao spawn |
| K19 | A furtivo passa perto de um NPC hostil | O NPC não puxa aggro; ao sair da furtividade, puxa normalmente |

---

## 9) R5 — Manager: progressão por rank e CC de NPC

### 9.0 Pré-requisitos

- SQL das fases anteriores já aplicado (`add_skills_rank_growth.sql`, `add_skills_rank_economy.sql`, `add_npc_templates_cc_immune.sql`). Nada novo no banco.
- PHP sincronizado com o WAMP (`scripts_main\sync_umbra_api_to_wamp.bat`): endpoint novo `api/admin/preview_skill_ranks.php`.
- Manager recompilado (`dotnet build tools/UmbraManagerWpf/UmbraManager.sln`, com o Manager fechado).

### 9.1 Editor de skills (aba **Skills**)

**Progressão por rank** (abaixo dos números da skill):

| Campo | Coluna | Regra |
|-------|--------|-------|
| crescimento %/rank | `skills.rank_growth_percent` | Vazio = padrão por tipo (dano/cura/escudo/DOT/HOT 12,5%; buff/debuff/slow 10%; controle +10% de duração e +5 de chance). 0 = não escala. Teto 100 |
| pontos extras/rank | `skills.rank_cost_step` | Custo do rank N = `skill_cost` + step × (N − 1). Padrão 1 |
| níveis extras/rank | `skills.rank_level_step` | Nível do rank N = `req_level` + step × (N − 1), limitado ao nível máximo. Padrão 4 |

**Pré-visualização por rank** (abaixo dos efeitos extras):

- Abre sozinha ao **Editar** uma skill. Depois de mudar qualquer campo, clique em **Calcular prévia**.
- Calcula com o formulário **atual, sem salvar**, usando a mesma fórmula da zone.
- Colunas:
  - **Pts** (custo do rank) e **Total** (acumulado);
  - **Nível**, **Power**, **Mana** e **CD**;
  - **Efeitos**: resumo de cada efeito com valor, duração e chance já escalados.
- **★** marca os ranks com efeito extra (marco).
- Abaixo da grade aparecem os alertas, ou "Sem alertas.".

**Validação ao salvar:** se houver alerta, o **Salvar skill** pergunta "Salvar mesmo assim?". O alerta não bloqueia o save.

| Alerta | Quando |
|--------|--------|
| stat desconhecido | `target_stat` de BUFF_STAT/DEBUFF_STAT fora da lista da zone (`StatKeyMapping.hpp`), que ignora o efeito |
| sem target_stat | BUFF_STAT/DEBUFF_STAT sem stat |
| sem duração | Buff, debuff, CC, DOT/HOT, escudo ou furtividade sem `duration_ms` no efeito e na skill (passivas não entram) |
| crescimento alto | `rank_growth_percent` ou `rank_growth` do efeito acima de 50% por rank |
| Power+ alto | `power_coef_bonus` acima de 100 (o crescimento já escala o power) |
| CD- zera o cooldown | `cooldown_reduction_ms` ≥ cooldown base |
| sem progressão | O último rank sai igual ao rank 1 |
| rank 3/5 sem marco | Skill com 5 ranks sem efeito extra no 3 ou no 5 (meta da R4) |
| ultimate com max_rank > 3 | Padrão da R2 é 3 |
| vários ranks no nível máximo | `req_level` + step estoura o nível máximo |

Depois de salvar: **Recarregar no Zone** (o PHP sozinho não recarrega o C++).

### 9.2 Editor de NPC (seção de atributos de combate)

Novo bloco **Controle de combate (CC)**:

| Campo | Coluna | Efeito |
|-------|--------|--------|
| Resist. stun / silence/interrupt / root / slow | `npc_templates.stun_resist`, `silence_resist`, `root_resist`, `slow_resist` | 0–100: chance de anular o CC daquele tipo |
| **Imune a CC (chefe)** | `npc_templates.cc_immune` | STUN, SILENCE, ROOT e SLOW não pegam (texto "Imune"); dano, debuff de stat e DOT continuam |

A zone lê esses campos ao carregar as instâncias. Ao salvar o template, o Manager já manda `reload_npc_instances` para as zones conectadas no AdminHub; sem conexão, reinicie a zone.

### 9.3 O que já está pronto (C++ e PHP)

- A fórmula de rank fica num só lugar no PHP (`helpers/skill_rank_helper.php`, espelho de `SkillTypes.hpp`). A pré-visualização, o livro de skills e a zone dão o mesmo número.
- `skill_admin_helpers.php` grava os três campos de rank, com crescimento vazio = NULL e passos entre 0 e 20.
- `create/update/list_npc_template.php` gravam e devolvem as resistências (0–100) e o `cc_immune`.

### 9.4 QA

| # | Passo | Esperado |
|---|-------|----------|
| M1 | **Editar** o Golpe nas Sombras | Grade com 5 ranks; Power 140 → 158 → 175 → 193 → 210 (12,5%/rank); ★ nos ranks com marco |
| M2 | Mudar crescimento para 0 e **Calcular prévia** | Valores iguais em todos os ranks + alerta "Sem progressão" (se não houver extras) |
| M3 | Escrever `atackk` num BUFF_STAT e salvar | Alerta "stat desconhecido"; **Não** cancela, **Sim** salva |
| M4 | Pontos extras/rank = 2 | Pts 1/3/5/7/9; Total 25 |
| M5 | Salvar, **Recarregar no Zone**, subir o rank no PIE | Custo e nível do livro iguais à grade |
| M6 | NPC com **Imune a CC** (salvar com a zone conectada); usar Pisotear | Dano entra; o slow mostra "Imune" |
| M7 | NPC com Resist. stun 100 | Stun nunca pega |

### 9.5 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| "Pré-visualização falhou" | PHP sem sync ou sessão admin vencida | Rodar o sync do WAMP; logar de novo no Manager |
| Grade vazia ao editar | Mesmo motivo acima | Idem; clicar em **Calcular prévia** |
| Save "Erro ao atualizar skill" | Coluna de rank ausente no banco | Rodar `add_skills_rank_growth.sql` e `add_skills_rank_economy.sql` |
| Resistência do NPC não muda em jogo | Zone não conectada no AdminHub na hora do save | Conectar o admin e salvar de novo, ou reiniciar a zone |

---

## 10) R6 — Tooltip e livro: "rank atual → próximo"

### 10.0 Pré-requisitos

- PHP sincronizado com o WAMP (`get_available_skills.php` manda `next_rank_preview`).
- Compilar o UE. Arquivos alterados:
  - `Data/UmbraSkillDataStructures.h`;
  - `Core/UmbraGameInstance.{h,cpp}`;
  - `UI/UmbraSkillTooltipWidget.{h,cpp}`;
  - `UI/UmbraSkillBookWidget.{h,cpp}`.
- A zone não muda.

### 10.1 Editor UE (sua parte)

| Widget | O que fazer |
|--------|-------------|
| `/Game/Widgets/UI/Skills/WBP_SkillTooltip` | Nada, se ele usa o layout novo (`VB_Content`): a seção é montada em C++. No layout antigo, adicionar um **TextBlock** chamado **`Tooltip_NextRank`** (Auto Wrap ligado) |
| `WBP_SkillBook` | Adicionar um **TextBlock** chamado **`NextRank_Text_1`** logo abaixo do `CurrentRank_Text_1` (Auto Wrap ligado). É opcional: sem ele o livro funciona como antes, só sem a prévia |

Não precisa de Graph. Cor, fonte e visibilidade são definidas em C++.

### 10.2 O que aparece

Só para skill **aprendida** e **fora do rank máximo**:

```
Próximo: Rank 3/5 ★ marco
Requer 3 pontos · nível 18
Poder 158% → 175%
Recarga 12,0 s → 11,0 s
Atordoa por 2,2→2,4 s
★ Novo: Silencia por 1,0 s
```

- **Cabeçalho:** o próximo rank; "★ marco" quando ele tem efeito extra.
- **Requer:** pontos e nível do próximo rank (os mesmos do botão do livro).
- **Poder, custo, recarga e duração:** só aparecem os que mudam.
- **Efeitos:**
  - a mesma linha nos dois ranks mostra só os números que mudam (`2,2→2,4`);
  - efeito que só existe no próximo rank aparece como **★ Novo:**, em dourado.

Os valores vêm do PHP (`helpers/skill_rank_helper.php`), com a mesma fórmula da zone e da pré-visualização do Manager.

### 10.3 O que o C++ já faz

- `ParseSkillData` lê `next_rank_preview` e preenche os campos `Next*` do `FUmbraSkillData`.
- `UUmbraGameInstance::BuildSkillNextRankLines` / `FormatSkillNextRankText`:
  - formata os efeitos do próximo rank com o mesmo formatador do tooltip;
  - casa as linhas pelo formato (ignorando números);
  - marca como novas as que não existem no rank atual.
- **Tooltip:** no layout novo, a seção entra antes do rodapé, com o marco em dourado; no layout antigo, vai para `Tooltip_NextRank`.
- **Livro:** `FillDetail` preenche o `NextRank_Text_1`, em dourado se o próximo rank for marco, e o limpa quando nenhuma skill está selecionada.

### 10.4 QA

| # | Passo | Esperado |
|---|-------|----------|
| T1 | Hover numa skill rank 1 na barra | Seção "Próximo: Rank 2/5" com Poder e efeitos subindo |
| T2 | Skill rank 2 com marco no 3 | "★ marco" no cabeçalho e linha **★ Novo:** em dourado |
| T3 | Subir o rank no livro | O tooltip e o `NextRank_Text_1` passam a mostrar o rank seguinte |
| T4 | Skill no rank máximo | Seção não aparece |
| T5 | Skill não aprendida | Seção não aparece (o livro mostra "Não aprendida · requer nível X") |
| T6 | Comparar com o Manager (§9.1) | Valores do próximo rank iguais aos da grade de pré-visualização |

### 10.5 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Seção nunca aparece | PHP sem sync ou UE não compilado | Rodar o sync do WAMP; compilar o UE; reabrir o livro (recarrega as skills) |
| Livro sem a prévia, tooltip com | `NextRank_Text_1` não existe no `WBP_SkillBook` | Adicionar o TextBlock com o nome exato |
| Linha inteira "a → b" em vez de só o número | O texto do efeito mudou de formato entre os ranks (ex.: extra substituindo outro) | Esperado; só cosmético |

**Fim do guia.**
