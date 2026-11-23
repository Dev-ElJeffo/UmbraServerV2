# GUIA — Revisão do sistema de habilidades (UE 5.6.1)

Revisão do motor de skills da zone: alvo de cada efeito, skills de aliado, área, multi-hit, passivas, execute, blink e stats novos. Tudo é resolvido no **servidor** (`CombatCoreEngine`). O cliente e os Blueprints não mudam nesta fase.

| Fase | Conteúdo | Status |
|------|----------|--------|
| **A** | Motor de efeitos (servidor C++ + ficha PHP) | Implementado, falta QA |
| **B** | Move cast + interrupt (opcode 122, UE) | Implementado, falta QA (ver §8) |
| **C** | Ajuste dos dados das skills (SQL) + UX do Manager | Implementado, falta QA (ver §9) |

## Objetivo

- Buff de **aliado** (Alchemist) funciona sem alvo: vai para o próprio caster.
- Golpe que dá dano **e** buff aplica o buff em **quem bate**, não no alvo.
- Skill de **área** dá dano em todos os inimigos do raio.
- **Passivas** sem condição passam a valer sempre.
- Opcodes: **nenhum novo nesta fase**. Os 97/103/104/105 continuam iguais.

---

## 0) Pré-requisitos

1. Parar a zone, recompilar e reiniciar:
   `cd D:\UmbraServerV2\build; cmake --build . --config Release --target zone_server`
2. PHP sincronizado com o WAMP (`/sync-wamp` ou `scripts_main\sync_umbra_api_to_wamp.bat`). Arquivo alterado: `helpers/character_info_helper.php`, que faz a ficha mostrar as passivas.
3. **Não** precisa recompilar o UE nesta fase.

---

## 1) Regras novas de alvo (fase A)

### 1.1 Alvo de cada efeito

Ordem de decisão, por efeito:

1. **`target_override`** preenchido no Manager → vale ele.
2. Sem override, numa skill **ENEMY** ou **AREA**, efeito **benéfico** → vai para o **caster**.
   Benéficos: `HEAL`, `HOT`, `SHIELD`, `BUFF_STAT`, `CLEANSE`, `STEALTH`, `INVULNERABLE`, `REFLECT`, `COOLDOWN_RESET` e `RESOURCE_RESTORE`.
3. Nos demais casos, o efeito herda o alvo da skill.

Exemplos: o `+double attack` da **Ruin Strike**, a velocidade da **Wild Charge** e o `stun_resist` dos ranks da **Holy Strike** agora caem em quem usou a skill.

> Se você **quer** um buff no inimigo (raro), use `target_override = ENEMY` no efeito.

### 1.2 Skill de aliado (`ALLY`)

| Situação no cast | Quem recebe |
|------------------|-------------|
| Player selecionado | O player (e o caster também, se **Incluir caster** estiver ligado) |
| Nada selecionado, NPC selecionado ou alvo offline | **O próprio caster** (self-cast) |

Antes, sem um player como alvo, o buff era descartado ou caía no NPC inimigo.

### 1.3 Grupo e área de aliados (`PARTY` / `AREA_ALLY`)

- O caster **sempre** entra: ele é membro do próprio grupo, e solo recebe sozinho.
- **Incluir caster** passa a valer só para `ALLY`.
- Em `AREA_ALLY`, os membros fora do `area_radius` (padrão 500uu) ficam de fora. O caster nunca fica.

### 1.4 Área inimiga (`AREA`)

- O dano atinge **todos** os inimigos no raio do ponto alvo, até **10** alvos, dos mais próximos para os mais distantes.
- Entram NPCs atacáveis vivos e players **fora do seu grupo**. Membros do grupo não levam nem dano nem debuff.
- O raio padrão é 300uu para o dano e 500uu para os efeitos, quando `area_radius = 0`.
- Cada alvo rola o acerto separadamente; um erro num alvo não impede os demais.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Alvo por efeito | `src/services/SkillTypes.hpp` (`resolveEffectTarget`) | Regra da §1.1 (players e NPCs) |
| Self-cast ALLY | `src/zone/CombatCoreEngine.cpp` (`processSkillCast`) | Sem player válido como alvo, o alvo vira o caster antes do range |
| Despacho dos efeitos | `CombatCoreEngine::applySkillEffects` | ALLY, PARTY e AREA_ALLY com o caster incluso; AREA via `collectAreaEnemies` |
| Dano | `CombatCoreEngine::resolveSkillDamageOnTarget` | **1 hit por efeito DAMAGE**, com o `value_percent` escalado pelo rank |
| Condições do DAMAGE | `damageEffectConditionsMet` | `health_below_percent` / `target_health_below_percent` / `health_above_percent` (alvo), `caster_health_below_percent`. Outra chave (ex.: `target_type`) = o hit **não** aplica |
| Crítico garantido | `CombatCalculator.hpp` (`SkillData::forceCrit`) | `conditions_json.guaranteed_crit = true` no efeito DAMAGE |
| EXECUTE | `applySkillEffects` | Alvo abaixo do limiar (padrão 20%): dano bônus = `value_percent`% do dano do golpe (ou `value_flat`). Instakill só com `conditions_json.instakill = true` |
| TELEPORT (blink) | `applySkillEffects` | Sem `x/y` nas condições: `value_flat` uu para frente, na direção do personagem (negativo = para trás, limite ±3000) |
| Bônus de rank | `SkillData::buildEffectsForRank` | Um extra com o mesmo tipo + `target_stat` de um rank anterior **substitui** o anterior, em vez de somar |
| Cura | `CombatCoreEngine::scaleHealAmount` | % do HP do alvo + MAG_ATK × `power_coef` da skill de cura, × (100 + `healing_bonus`)% do curador |
| Passivas | `src/zone/CharacterStateLoader.cpp`, `helpers/character_info_helper.php` | `BUFF_STAT`/`DEBUFF_STAT` sem condição valem sempre; com `health_below_percent` só abaixo do limiar; `LIFESTEAL` vira o stat `lifesteal` |
| Stats novos | `SkillTypes.hpp`, `StatKeyMapping.hpp` | Ver §3 |

## 3) Stats novos (`target_stat`)

| `target_stat` | Efeito | Valor |
|---------------|--------|-------|
| `ignore_dodge` | Ignora % da esquiva do alvo no teste de acerto | `value_percent` (0–100) |
| `buff_duration` | +% de duração nos buffs e escudos que **você** aplica | `value_percent` (até 200) |
| `lifesteal` | % do dano causado (skill e ataque básico) que volta como HP | `value_percent` (0–100). Passiva `LIFESTEAL` usa o mesmo stat |

Nesses três, `value_percent` **soma pontos**: +25 = +25%, e não 25% de um valor base.

---

## 4) Manager — o que conferir agora

Nada obrigatório na fase A. Para tirar proveito das regras:

1. **Skill de aliado:** o alvo da skill = **ALLY**. Para "aliado + eu", ligue **Incluir caster** na skill e deixe o check do efeito em **herdar** (estado intermediário).
2. **Efeito "Incluir caster" no editor de efeitos:** desmarcado = **força não**. Para herdar, deixe no estado intermediário. A fase C troca o check por um ComboBox.
3. **Cura escalável:** preencha o `power_coef` da skill de cura (ex.: 60 = +60% do MAG_ATK).

Depois de salvar, o Manager envia `reload_skills` para a zone.

---

## 5) QA (PIE)

PIE com **Number of Players ≥ 2** (Alchemist + outra classe), com e sem grupo.

| # | Cenário | Esperado |
|---|---------|----------|
| A1 | Alchemist **sem alvo** usa Strength Elixir / Precision Serum / Protective Coat | Buff aparece na **própria** BuffBar e a ficha mostra o stat aumentado |
| A2 | Alchemist com **NPC** selecionado usa um buff de aliado | Buff no Alchemist; **nenhum** buff no NPC |
| A3 | Alchemist com o **outro player** selecionado | Buff no outro player (e nele também, se **Incluir caster** estiver ligado) |
| A4 | Barbarian usa Ruin Strike / Wild Charge num NPC | O buff (double attack / velocidade) aparece no **Barbarian**; o NPC só leva dano |
| A5 | Templar Holy Strike rank ≥ 2 | `stun_resist` no Templar, e no valor do rank atual (não somado) |
| A6 | Assassin Double Attack | **2 números de dano** no alvo |
| A7 | Dark Mage Corruption Orb no meio de 3 NPCs | Os 3 levam dano |
| A8 | AREA perto de um membro do grupo | O membro não leva dano nem debuff |
| A9 | Restoring Mist / Survival Protocol **solo** | O caster é curado e recebe os buffs |
| A10 | Execution / Martial Execution em alvo com pouca vida | Número de dano extra (sem morte instantânea em PvP) |
| A11 | Rift Step | O Dark Mage avança ~5m na direção em que olha |
| A12 | Ficha de um personagem com passivas (ex.: Hardened Skin, Iron Bones) | Os stats incluem as passivas |

Logs esperados (`logs/zone_*.log`, nível debug):
- `"SkillCast AREA player=... alvos=N dano=..."`
- `"SkillCast dmg player=... hit=2 ..."` (multi-hit)
- `"DAMAGE da skill ... ignorado: condição não atendida"` (condição desconhecida, ex.: Ash Light `target_type`)

## 6) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Buff de aliado não sai em lugar nenhum | Efeito com "Incluir caster" desmarcado em ALLY + alvo inexistente | Deixar o check do efeito em herdar |
| Buff ainda cai no inimigo | Efeito com `target_override = ENEMY` | Limpar o override (herdar) |
| Strength Elixir buffa inimigos | `target_override = AREA` no efeito (dado antigo) | Limpar o override; a fase C corrige o seed |
| Área atinge só 1 alvo | Skill com alvo **ENEMY** e `area_radius` preenchido | Para área, o alvo da skill precisa ser **AREA** |
| Cura não escala | `power_coef = 0` na skill de cura | Preencher o `power_coef` |
| Rift Step não se move | `value_flat = 0` ou `x/y` nas condições | `value_flat = 500` e sem `x/y` |
| Ficha sem a passiva | PHP não sincronizado | `/sync-wamp` |

## 7) Referência de código

- Servidor: `src/zone/CombatCoreEngine.cpp` (`processSkillCast`, `applySkillEffects`, `resolveSkillDamageOnTarget`, `collectAreaEnemies`, `scaleHealAmount`, `applyLifesteal`, `effectiveBuffDurationMs`)
- Tipos: `src/services/SkillTypes.hpp` (`resolveEffectTarget`, `buildEffectsForRank`), `src/services/StatKeyMapping.hpp`, `src/services/CombatCalculator.hpp`
- Passivas: `src/zone/CharacterStateLoader.cpp`, `www/umbra_api/helpers/character_info_helper.php`

---

## 8) Fase B — Move cast e interrupt

### 8.0 Pré-requisitos

1. **SQL** (adiciona `INTERRUPT` ao ENUM `skill_effects.effect_type`, idempotente):
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skill_effects_interrupt.sql`
2. Recompilar a `zone_server` e reiniciar a zone.
3. PHP já sincronizado com o WAMP (`api/admin/skill_effect_schema_helpers.php`).
4. **Compilar o UE** (Live Coding ou rebuild): os arquivos estão na §8.6.
5. Recompilar o Manager com ele **fechado** (`dotnet build tools/UmbraManagerWpf/UmbraManager.sln`).

### 8.1 Regras do cast

Valem só para skills com **Cast time > 0**. Cast instantâneo não tem o que cancelar.

| Situação durante a barra | Resultado | Mana e CD |
|--------------------------|-----------|-----------|
| Skill **sem** "Mover ao conjurar", o caster anda mais de ~0,6m | Cancelado (motivo 1) | **Devolvidos** |
| Skill **com** "Mover ao conjurar" | Pode andar à vontade | — |
| Inimigo acerta um **INTERRUPT** e a chance passa | Interrompido (motivo 2) | Perdidos |
| **Stun**, **silence** ou **knockback** pegou no caster | Interrompido (motivo 3) | Perdidos |
| O caster morreu | Cancelado (motivo 4) | Perdidos |

- Há uma folga de **250 ms** no começo do cast, para o embalo da corrida e os frames em trânsito. Ao iniciar uma skill sem move cast, o cliente já para o personagem.
- Ao virar no lugar (girar a câmera) o cast não cancela; só o deslocamento conta.

### 8.2 Efeito `INTERRUPT`

| Campo | Uso |
|-------|-----|
| `chance_percent` | Chance base de interromper (padrão 100) |
| `value_percent` | Usado como chance só se `chance_percent` = 0 |
| `duration_ms` | > 0: após interromper, o alvo fica **silenciado** esse tempo (lockout) |
| `resist_penetration` | Reduz o `silence_resist` do alvo |

- A chance final usa o mesmo cálculo de CC: `silence_chance` do caster soma e `silence_resist` do alvo reduz.
- Só rola se o alvo **estiver conjurando**; contra alvo parado o efeito é ignorado.
- Funciona contra players e NPCs, e de NPC contra player.
- **Checkbox legado "Interrompe" (`is_interrupt`)**: numa skill ENEMY/AREA sem efeito INTERRUPT próprio, vira um INTERRUPT implícito de **50%**, sem lockout. Para outra chance, adicione um efeito INTERRUPT.

### 8.3 Cast de NPC

- Uma skill de NPC com `cast_time_ms > 0` agora tem **cast real**: o NPC para de andar e de atacar até o fim da barra, e o hit sai no fim.
- No fim do cast, o alvo precisa estar a até 1,5× o alcance; mais longe, o golpe é perdido.
- O NPC pode ser interrompido por INTERRUPT, stun, silence ou knockback (a montage dele para).

### 8.4 Protocolo — opcode 122 `SkillCastInterrupted` (Servidor → Clientes)

```
[0]    u8   msgType = 122
[1]    u8   sourceType   (1 = player, 2 = NPC)
[2..5] u32  sourceId     (LE)
[6..9] u32  skillId      (LE)
[10]   u8   reason       (1 = andou, 2 = interrompido, 3 = CC, 4 = morreu)
```

- 11 bytes fixos. É enviado para os clientes perto do caster (AOI do player ou do NPC).
- Os opcodes 97 e 105 não mudaram.

### 8.5 O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Cast pendente | `CombatCoreEngine::scheduleSkillHit` | Guarda a mana cobrada e o flag de move cast |
| Cancelar andando | `CombatCoreEngine::checkCastMovementCancels` | Verifica o deslocamento a cada tick, devolve mana e CD |
| Interrupt/CC | `CombatCoreEngine::applySkillEffects` | INTERRUPT com chance + lockout; STUN/SILENCE/KNOCKBACK cancelam |
| Cast de NPC | `processNpcSkillCast` → `resolveNpcSkillCast` | NPC parado (`canNpcMove`/`canNpcAct`) até o hit |
| Opcode 122 (cliente) | `UUmbraGameInstance::HandleSkillCastInterruptedMessage` | Libera o lock de cast, limpa o CD se andou, cancela o hit VFX adiado, para a montage do NPC, mostra o aviso |
| Montage do player | `UUmbraCombatComponent::HandleSkillCastInterrupted` | `Montage_Stop(0.2)` no personagem que conjurava |
| Barra de cast | `UUmbraSkillCastBarWidget` | Congela vermelha com **"Cancelado"** ou **"Interrompido"** por `InterruptedDisplaySeconds` (0,8s) |
| Para o embalo | `UUmbraGameInstance` (UseSkill) | `StopMovementImmediately` ao iniciar skill sem move cast |

### 8.6 Editor UE

- **Nenhum WBP novo.** O `WBP_SkillCastBar` continua com `ProgressBar_Cast`, `Text_SkillName` e `Text_Remaining` (`BindWidgetOptional`). O motivo aparece no `Text_Remaining`.
- Opcional, em **Class Defaults** do `WBP_SkillCastBar`: `InterruptedDisplaySeconds` e `InterruptedFillColor` (categoria `Umbra|CastBar`).
- Arquivos para compilar: `Core/UmbraGameInstance.{h,cpp}`, `Network/NetMovementClient.cpp`, `UI/UmbraSkillCastBarWidget.{h,cpp}`, `Components/UmbraCombatComponent.{h,cpp}`, `UI/UmbraBuffLabelHelper.cpp`.

### 8.7 Manager

- O tipo **INTERRUPT** aparece na lista de efeitos, com os campos de duração (lockout) e penetração.
- Os stats `ignore_dodge`, `buff_duration` e `lifesteal` da fase A entram na lista de `target_stat`.
- Para testar: numa skill de cast longo (ex.: 2000 ms) deixe **Mover ao conjurar** desligado; em outra, ligado.

### 8.8 QA (PIE, Number of Players ≥ 2)

| # | Cenário | Esperado |
|---|---------|----------|
| B1 | Skill com cast (sem move cast), parado | Barra completa, o hit sai |
| B2 | Mesma skill, andar no meio da barra | Barra vermelha "Cancelado", aviso "...cancelada: você se moveu", **mana volta e a skill sai do CD** |
| B3 | Skill **com** move cast, andando | Barra completa e o hit sai |
| B4 | Player 2 com uma skill INTERRUPT 100% acerta o Player 1 conjurando | Player 1: "Interrompido", mana e CD **perdidos**; o outro client vê a animação parar |
| B5 | INTERRUPT com `duration_ms = 3000` | Após B4, o Player 1 fica silenciado 3s (BuffBar) |
| B6 | INTERRUPT contra alvo parado (sem cast) | Nada acontece (log `INTERRUPT ... ` não aparece) |
| B7 | Stun/knockback no Player 1 conjurando | Cast interrompido (motivo 3) |
| B8 | Skill com o checkbox **Interrompe** (sem efeito INTERRUPT) | ~50% dos acertos interrompem |
| B9 | NPC com skill de `cast_time_ms > 0` | NPC para, anima, o hit sai no fim |
| B10 | Interromper o NPC de B9 (stun ou INTERRUPT) | Montage do NPC para, sem dano no player |

Logs (`logs/zone_*.log`):
- `"SkillCastInterrupted player=... skill=... motivo=1 refund_mana=..."`
- `"SkillCastInterrupted npc=... skill=... motivo=2"`
- `"INTERRUPT skill=... alvo=... chance=... roll=..."` (debug)
- No UE: `[UmbraGameInstance] SkillCastInterrupted (122): ...`

### 8.9 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Salvar efeito INTERRUPT dá erro no Manager | ENUM sem `INTERRUPT` | Rodar `add_skill_effects_interrupt.sql` |
| Manager não lista INTERRUPT | Manager antigo | Fechar e recompilar |
| Andar não cancela | Skill com "Mover ao conjurar" ou `cast_time_ms` < ~300 | Conferir flags/tempo |
| Cancela sozinho ao usar a skill correndo | Ainda segurando a tecla de movimento | Esperado: andar cancela |
| Barra some sem "Interrompido" | UE não recompilado | Compilar o UE |
| Hit VFX aparece mesmo interrompido | Cliente antigo | Compilar o UE |
| NPC continua andando no cast | Skill do NPC com `cast_time_ms = 0` | Preencher o cast time |

### 8.10 Referência de código (fase B)

- Protocolo: `src/zone/MovementProtocol.hpp` (`SkillCastInterrupted = 122`, `SkillCastInterruptReason`)
- Servidor: `src/zone/CombatCoreEngine.{hpp,cpp}` (`interruptPlayerCast`, `interruptNpcCast`, `checkCastMovementCancels`, `tickPendingNpcCasts`, `resolveNpcSkillCast`, `appendLegacyInterruptEffect`), `src/services/SkillTypes.hpp` e `SkillService.cpp` (`EffectType::INTERRUPT`)
- PHP/SQL: `api/admin/skill_effect_schema_helpers.php`, `scripts/add_skill_effects_interrupt.sql`
- Manager: `Models/Editors/SchemaCatalog.cs`

---

## 9) Fase C — Dados das skills e UX do Manager

### 9.0 Pré-requisitos

1. Rodar o SQL (idempotente; os efeitos ficam em `skills.effects_json`):
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\fix_skills_revisao_v1.sql`
   O `SELECT` no fim do script mostra as 16 skills alteradas.
2. Aplicar na zone: **reiniciar** a zone, ou salvar qualquer skill no Manager (envia `reload_skills`).
3. Recompilar o Manager com ele **fechado**.
4. Reiniciar o PIE: o cliente só baixa as skills no login.

### 9.1 O que o SQL muda

| Skill | Antes | Depois |
|-------|-------|--------|
| Alchemist — Tônico de Campo | BUFF em **AREA inimiga** com override ALLY | Cura em **AREA_ALLY** (raio 400, caster incluso), `power_coef` 40 |
| Alchemist — Elixir de Força | Override AREA + incluir caster = não (buffava inimigos) | Sem override; "Interrompe" desligado |
| Alchemist — Neutralizador | `redirect_to_self` (sem efeito fora de REACTION) | Limpo; sem alvo vai para o caster |
| Alchemist — Névoa Restauradora / Protocolo de Sobrevivência | Cura sem escala | `power_coef` 40 / 50 |
| Alchemist — Bomba Disruptiva | — | + INTERRUPT 50% |
| Templar — Oração de Campo | Override SELF (nunca curava o aliado) | Cura o aliado (sem alvo: o caster), `power_coef` 40 |
| Templar — Milagre das Cinzas | Cura só no caster | Cura o grupo na área, `power_coef` 60 |
| Templar — Aura do Guardião | Sem duração (5s) | 60s (= cooldown) |
| Templar — Chama do Voto | Overrides PARTY redundantes | Herda AREA_ALLY |
| Templar — Golpe Consagrado | — | + INTERRUPT 40% |
| Barbarian — Golpe da Ruína | `double_attack_chance` sem duração | `double_attack_rate` por 5s (no caster) |
| Barbarian — Rugido de Guerra | "Interrompe" ligado num buff | Desligado |
| Barbarian — Impacto Esmagador | — | + INTERRUPT 60% com silêncio de 1,5s |
| Assassin — Lâmina Precisa | Precisão sem duração | 5s, no caster |
| Assassin — Golpe nas Sombras | — | + INTERRUPT 50% com silêncio de 1s |
| Martial — Corte Técnico | Precisão sem duração | 5s + INTERRUPT 40% |
| Martial — Sequência Disciplinada | Double attack sem duração | 5s, no caster |
| Dark Mage — Torção da Realidade | — | + INTERRUPT 50% com silêncio de 2s |

Já estavam certos e funcionam com o motor da fase A: Rift Step (`value_flat` 500), Execução, Olho do Predador (`ignore_dodge`), Recuperação Sombria (`LIFESTEAL` passivo) e Preparação Antecipada (`buff_duration`).

Ficam com o "Interrompe" legado (50%): Golpe da Ruína, Investida Selvagem e Orbe da Corrupção. Para mudar, desligue o checkbox ou adicione um efeito INTERRUPT.

### 9.2 Manager — editor de efeitos

| Campo | Mudança |
|-------|---------|
| **incluir caster** (por efeito) | ComboBox **Herdar / Sim / Não** no lugar do check de 3 estados. Herdar = usa o da skill (recomendado) |
| **alvo do efeito** | Tooltip com a regra do INHERIT e dos efeitos benéficos |
| **Redirecionar para si** | Renomeado para "(só REACTION)", com tooltip |
| Checkboxes da skill (**Interrupt**, **Move cast**, **Incluir caster**) | Tooltips com o comportamento real |

### 9.3 QA (PIE, Number of Players ≥ 2)

| # | Cenário | Esperado |
|---|---------|----------|
| C1 | Tônico de Campo solo e com o grupo por perto | Cura o caster e os membros no raio; não afeta inimigos |
| C2 | Elixir de Força perto de NPCs | Buff só no caster/aliado; nenhum NPC ganha buff |
| C3 | Oração de Campo com o outro player selecionado | O outro player é curado |
| C4 | Milagre das Cinzas em grupo | O grupo no raio é curado e recebe as defesas |
| C5 | Aura do Guardião | Buff dura 60s na BuffBar |
| C6 | Golpe da Ruína / Corte Técnico | Buff no caster por 5s |
| C7 | Impacto Esmagador num player conjurando | ~60% interrompe e silencia 1,5s |
| C8 | Manager: abrir uma skill antiga com "incluir caster" vazio | ComboBox mostra **Herdar**; salvar não grava `include_caster` |

### 9.4 Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Nada mudou no jogo | Zone sem `reload_skills` ou PIE antigo | Reiniciar a zone e o PIE |
| Tooltip/ComboBox antigos no Manager | Manager não recompilado | Fechar, recompilar e abrir |
| Script não altera uma skill | `skill_key` diferente no seu banco | Conferir o `SELECT` final do script |

**Fim do guia.**
