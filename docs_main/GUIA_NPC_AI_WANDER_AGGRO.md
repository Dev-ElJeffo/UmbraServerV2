# Guia: AI de mobs (wander, aggro, basic attack)

Complemento de [`GUIA_COMBATE_V2_DANO_REAL.md`](GUIA_COMBATE_V2_DANO_REAL.md).  
Servidor autoritativo no **zone**; cliente só renderiza posição/anim.

## Pré-requisito de schema

Rodar uma vez (nessa ordem):

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_npc_ai_fields.sql
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_npc_combat_stop_chase_speed.sql
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_npc_kite_speed_mult.sql
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_npc_skills_cast_anim_path.sql
```

## Campos (template)

| Campo | Default | Significado |
|-------|---------|-------------|
| `roam_radius` | 0 | Wander em círculo no home; 0 = parado |
| `aggro_radius` | 0 | Detecta player; 0 = não inicia por proximidade |
| `leash_radius` | 0 | Distância máx. do home; 0 = `aggro*2` |
| `attack_range` | 150 | Alcance de **hit** do basic (validação 1.15×) |
| `combat_stop_range` | 0 | Distância para **parar o chase e atacar**; 0 = usa `attack_range`. O zone **capa** ao alcance efetivo do basic (`max(attack_range, bodyMinDist) × 1.15`) para não ficar parado fora do hit. |
| `damage_type` | PHYSICAL | School do auto-attack: `PHYSICAL` → phys ATK/DEF; `MAGIC` → mag ATK/DEF (`power_coef` via `basic_power_coef`) |
| `basic_power_coef` | 100 | Coef do basic do mob (`ATK * coef / 100`) |
| `attack_cooldown_ms` | 1500 | CD do basic |
| `move_speed` | 200 | uu/s no wander |
| `chase_speed_mult` | 1.5 | Multiplicador de `move_speed` no **Chase** (clamp 0.5–5) |
| `kite_speed_mult` | 1.0 | Multiplicador de `move_speed` no **recuo** (afastar do player); 1.0 = roam; clamp 0.25–5 |
| `is_hostile` | 1 | Pode aggro/atacar (separado de `is_attackable`) |

Instância pode sobrescrever: `home_*`, `roam/aggro/leash/move_speed`, `combat_stop_range`, `chase_speed_mult`, `kite_speed_mult` (NULL = herda).

## Estados

`Idle → Wander → Chase → Combat → Return → Idle`

- **Wander**: destino aleatório em `home + roam_radius` a `move_speed`
- **Aggro**: player vivo em `aggro_radius` **ou** dano (`setAggroTarget`)
- **Chase**: persegue a `move_speed * chase_speed_mult` até o ponto do anel de stop **na linha NPC→player** (fallback: fecha direto no player se o aim estagnar); cliente força anim de **run** (`aiState=3`)
- **Combat**: **para** no anel de stop, face o player, basic/skills; se o player entrar no stop, o NPC **recua** a `move_speed * kite_speed_mult` (sem órbita melee)
- Se o player sai do stop range (ainda em leash/deaggro) → volta a **Chase**
- Separação de cápsula vs qualquer player usa só `bodyMinDist()` — o anel de stop **não** teleporta o NPC; o recuo é limitado por `kite_speed_mult` (melee consegue fechar)
- **Leash / deaggro**: volta ao home → Wander
- **Respawn**: posição volta ao **home**
- Basic só enfileira / consome CD se `dist ≤ effectiveNpcMeleeReach2D()` (evita cooldown fantasma)
## Skills de mob (Niagara + anim)

| Campo (`npc_skills`) | Uso |
|----------------------|-----|
| `cast_anim_path` | Montage UE no opcode 97 (como player) |
| `vfx_path` / `hit_vfx_path` | Niagara caster / hit |
| `vfx_key` | Legado; **não** é path de anim preferencial |

Fallback de anim: `cast_anim_path` → `anim_states_json.casts[]` (round-robin) → `skill` → `attacks[]`.

Basic (`attacks[]`): servidor **rotaciona** paths no opcode 99 (`castAnimPath` + `animIndex`).

## Opcodes

| Opcode | Uso AI |
|--------|--------|
| **102** | HP + XY/Yaw + `aiState`. Cliente: Chase/Return → `bIsRunning` |
| **99** | Basic; `sourceType=2` + path/índice |
| **97** | Skill cast + Niagara (igual player) |
| **92** | Dano/miss no player (`sourceId` = npcId) |

## Arquivos-chave

- `src/zone/NpcAiSystem.*` — tick AI (stop + chase + kite)
- `src/zone/NpcManager.*` — load `combat_stop_range` / `chase_speed_mult` / `kite_speed_mult`
- `src/zone/CombatCoreEngine` — basic multi-anim + `cast_anim_path`
- `src/services/SkillService.cpp` — load `npc_skills.cast_anim_path`
- Manager: `NpcEditorView` / `MobSkillsEditorView`
- Cliente: `UmbraNpcAnimInstance` (Chase=run), `UmbraGameInstance` 97/99/102

## Teste rápido (PIE)

1. SQL acima + rebuild zone + Manager com template: `combat_stop_range=300`, `attack_range=350`, `chase_speed_mult=2`, `kite_speed_mult=1`, roam/aggro setados, 2+ Attacks CSV.
2. Mob: roam → ao entrar no aggro **corre** (chase) → para em ~300 e ataca sem orbitar.
3. Player melee aproxima: recuo **mais lento** (kite); basic melee consegue acertar (sem teleporte de cápsula para stop).
4. Afastar do stop → chase de novo; além do leash → return/wander.
5. Skill com `vfx_path` + `cast_anim_path`: Niagara + montage corretos.
6. Basics seguidos: animações alternam entre `attacks[]`.
