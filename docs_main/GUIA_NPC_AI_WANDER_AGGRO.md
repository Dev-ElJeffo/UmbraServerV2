# Guia: AI de mobs (wander, aggro, basic attack)

Complemento de [`GUIA_COMBATE_V2_DANO_REAL.md`](GUIA_COMBATE_V2_DANO_REAL.md).  
Servidor autoritativo no **zone**; cliente só renderiza posição/anim.

## Pré-requisito de schema

Rodar uma vez:

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_npc_ai_fields.sql
```

## Campos (template)

| Campo | Default | Significado |
|-------|---------|-------------|
| `roam_radius` | 0 | Wander em círculo no home; 0 = parado |
| `aggro_radius` | 0 | Detecta player; 0 = não inicia por proximidade |
| `leash_radius` | 0 | Distância máx. do home; 0 = `aggro*2` (mín. 2500 se só aggro por dano) |
| `attack_range` | 150 | Alcance do basic do mob |
| `attack_cooldown_ms` | 1500 | CD do basic |
| `move_speed` | 200 | uu/s (wander + chase) |
| `is_hostile` | 1 | Pode aggro/atacar (separado de `is_attackable`) |

Instância: `home_x/y/z` (centro do roam). Spawn/admin move redefine home = pos.

## Estados

`Idle → Wander → Combat/Chase → Return → Idle`

- **Wander**: destino aleatório em `home + roam_radius`
- **Aggro**: player vivo em `aggro_radius` **ou** dano recebido (`setAggroTarget`)
- **Chase**: move até `attack_range`
- **Attack**: `CombatCoreEngine::processNpcBasicAttack` (CombatCalculator V2)
- **Leash**: distância ao home > leash → Return (limpa alvo)
- **Respawn**: posição volta ao **home**

## Opcodes

| Opcode | Uso AI |
|--------|--------|
| **102** | HP + XY/Yaw (throttle ~8 Hz). Cliente aplica XY/Yaw e **preserva Z** (floor-snap) |
| **99** | Basic broadcast; byte final `sourceType` (1=player, 2=npc) |
| **92** | Dano/miss no player (`sourceId` = npcId) |

## Arquivos-chave

- `src/zone/NpcAiSystem.*` — tick AI
- `src/zone/NpcManager.*` — runtime + home/stats
- `src/zone/CombatCoreEngine::processNpcBasicAttack`
- `UmbraEternumUE/.../UmbraGameInstance.cpp` (102/99)
- `UmbraEternumUE/.../Actors/UmbraNpcCharacter` (`ApplyMovementXYYaw`)

## Animações por estado

Ver [`GUIA_NPC_ANIM_STATES_UE561.md`](GUIA_NPC_ANIM_STATES_UE561.md) (`anim_states_json`, death delay, `aiState` no 102).

## Teste rápido

1. Template: `roam_radius=800`, `aggro_radius=600`, `is_hostile=1`, `physical_attack` > 0  
2. Reload zone / spawn instância  
3. Mob anda no círculo; ao aproximar, persegue e causa dano (vitals 87 + event 92)  
4. Afastar além do leash → volta ao home e retoma wander  
