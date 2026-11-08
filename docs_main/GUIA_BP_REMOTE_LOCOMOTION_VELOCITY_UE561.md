# Guia UE 5.6.1 — Velocity remota a partir do deslocamento

Após rebuild do módulo `UmbraEternumUE`, aplique nos BPs de movimento. O C++ passa a expor:

- `Update Player State Buffer With Animation`
- `Compute Remote Locomotion Velocity`
- `Apply Remote Locomotion Velocity`

**Assets:** `/Game/Maps/BP_NetMovementClient` e `/Game/Blueprints/BP_NetMovementClient2`

---

## Por quê

`Velocity = Forward(Yaw) * Speed` força animação sempre “pra frente”. Em ré/strafe o corpo desliza num sentido e os pés no outro (moonwalk). A correção usa **Δposição / Δt** do buffer A→B.

---

## 1) ProcessNextFrame — buffer com animação

Localize `Update Player State Buffer` (só Location/Yaw/Timestamp).

**Trocar por** `Update Player State Buffer With Animation`:

| Pino | Fonte |
|------|--------|
| Entry | entry do remote (como antes) |
| New Location | `OutLocation` |
| New Yaw | `OutYawDegrees` |
| New Speed | `OutSpeed` |
| New Velocity Z | `OutVelocityZ` |
| New Is In Air | `OutIsInAir` |
| New Timestamp Ms | `OutTimestampMs` |

Compile o BP após o rebuild C++ para os pinos novos (`StateB_Speed`, etc.) aparecerem no Break da struct.

---

## 2) Event Tick — remover Forward × Speed

No loop dos remotes (`HasStateA` + `HasStateB`), **apague** a cadeia:

```
Make Rotator(Yaw) → Get Forward Vector → Multiply × OutSpeed → Make Vector (+VelZ) → Set Velocity
```

**Substitua por:**

1. `Compute Remote Locomotion Velocity`
   - From Loc = `StateA_Location`
   - To Loc = `StateB_Location`
   - From Ts Ms = `StateA_TimestampMs`
   - To Ts Ms = `StateB_TimestampMs`
   - Fallback Speed = `StateB_Speed`
   - Velocity Z = `StateB_VelocityZ`
2. `Apply Remote Locomotion Velocity`
   - Target = `RemoteActorRef` (após Is Valid)
   - Velocity = saída do Compute
   - Is In Air = `StateB_IsInAir`

Manter VLerp de location e yaw (`Interpolate Network Yaw Degrees` / `Apply Interpolated Network Yaw To Actor`).

---

## 3) Compile + Save

`Compile` + `Save` nos dois NetMovementClient BPs.

---

## 4) QA rápido

| Ação do owner | Observador |
|---------------|------------|
| W | walk forward, sem moonwalk |
| S | anim de costas / Direction ~180 |
| A/D | strafe coerente |
| Parado | idle |
| Pulo | falling / VelocityZ |

Filtro log Verbose: `RemoteLocomotion`

Também: [`GUIA_BP_APLICAR_YAW_REMOTE_UE561.md`](GUIA_BP_APLICAR_YAW_REMOTE_UE561.md) (yaw) e [`GUIA_BP_ABP_SHOULD_MOVE_REMOTE_UE561.md`](GUIA_BP_ABP_SHOULD_MOVE_REMOTE_UE561.md) (Should Move / Acceleration).
