# Guia — Animações de NPC por estado (UE 5.6.1)

## Visão geral

Idle/Walk: **AnimBP + Velocity** (opcode **102**) quando existe um ABP válido. Packs **sem AnimBP** (só `ANIM_*` Sequences) usam `PlayAnimation` no puppet.

Attack / Hit / Death / Skill: Montage **ou** AnimSequence (`anim_states_json`).

Um ABP **global para todos os NPCs não funciona**: o AnimBP é preso a um **Skeleton**. Griffon e humanoide precisam de ABPs diferentes.

| Camada | Papel |
|--------|--------|
| 1 ABP por família de skeleton | Ex. `ABP_NpcGriffon` (parent C++ `UUmbraNpcAnimInstance`) |
| Sequences do template | Idle/Walk/Attack no JSON; spawn injeta no AnimInstance |
| Sem ABP (pasta / vazio) | Fallback Sequence: Idle/Walk loop, combat oneshot |

## JSON do template

Coluna `anim_states_json`:

```json
{
  "attacks": ["/Game/QuadrapedCreatures/Griffon/Animations/ANIM_Griffon_BiteAttack.ANIM_Griffon_BiteAttack"],
  "hits": ["/Game/QuadrapedCreatures/Griffon/Animations/ANIM_Griffon_GetHit.ANIM_Griffon_GetHit"],
  "death": "/Game/QuadrapedCreatures/Griffon/Animations/ANIM_Griffon_Death.ANIM_Griffon_Death",
  "idle": "/Game/QuadrapedCreatures/Griffon/Animations/ANIM_Griffon_Idle.ANIM_Griffon_Idle",
  "walk": "/Game/QuadrapedCreatures/Griffon/Animations/ANIM_Griffon_FlyForward.ANIM_Griffon_FlyForward",
  "death_ms": 1500
}
```

Compat: `"attack"` / `"hit"` (string) viram array de 1.

Campo **Anim Blueprint** no Manager: classe `ABP_*` (termina em `_C`) ou **vazio**. Pasta (`/Game/.../Animations`) é ignorada.

## Cliente (`AUmbraNpcCharacter`)

- Mesh do servidor **não** recebe mais o AnimBP do player.
- `PlayNpcAnimFromPath`: tenta Montage; senão Sequence (`PlayAnimation`).
- Sem ABP: loco Idle/Walk por Sequence; após attack/hit, retoma loco (timer 0,4–3s).
- Com ABP `UUmbraNpcAnimInstance`: `IdleSequence`, `WalkSequence`, `GroundSpeed` preenchidos no spawn. No Editor, blend Idle/Walk por `GroundSpeed`; DefaultSlot para montages.

## Como criar ABP do Griffon (opcional)

1. Animation Blueprint no skeleton do Griffon.
2. Parent Class = `UUmbraNpcAnimInstance`.
3. Grafo: blend `IdleSequence` / `WalkSequence` por `GroundSpeed` (> ~20 = walk).
4. Slot `DefaultSlot` no grafo.
5. Path no Manager: `/Game/.../ABP_Griffon.ABP_Griffon_C`

Packs novos: duplicar ABP **só** se o skeleton for outro.

## UmbraManager (Griffon hoje)

- AnimBP **vazio**.
- Preencher Idle + Walk + attacks/hits/death (`ANIM_*`).

## Opcodes

| Op | Uso |
|----|-----|
| 100 | mesh + ABP + arrays attacks/hits + death/skill/idle/walk + `death_ms` |
| 102 | HP + pos + `aiState` |
| 99 | basic NPC + `animIndex` |
| 103 | hit + `animIndex` (255 = random) |
| 101 | despawn após `death_ms` |
