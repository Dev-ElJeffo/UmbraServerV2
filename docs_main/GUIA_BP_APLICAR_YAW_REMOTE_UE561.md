# Guia UE 5.6.1 — Aplicar yaw remoto no Blueprint (pós-fix C++)

Após recompilar o módulo `UmbraEternumUE`, aplique estes passos nos assets. O C++ já removeu `360 - Yaw`, normaliza o wire e desliga `OrientRotationToMovement` em remotes; o BP ainda precisa trocar o `Lerp` de yaw.

**Assets:**

- `/Game/Maps/BP_NetMovementClient` (e espelho `/Game/Blueprints/BP_NetMovementClient2` se usado)
- `/Game/Blueprints/Player_BP/BP_RemotePlayer`

---

## 1) `BP_NetMovementClient` — SendMoveUpdate

Confirme (já costuma estar correto):

1. Pawn local → `Get Actor Rotation` → pino `Yaw`
2. Esse `Yaw` alimenta `Build Move Update Frame With Animation` → `Yaw Degrees`
3. **Não** use `Get Control Rotation`

Se houver qualquer `+ 90`, `+ 180` ou `360 - Yaw` no graph de envio, **apague**.

---

## 2) `BP_NetMovementClient` — Event Tick (obrigatório)

No `For Each` sobre `RemoteStates`, localize o `Lerp (Float)` que recebe:

- `A` = `StateA_Yaw`
- `B` = `StateB_Yaw`
- `Alpha` = `ClampedAlpha`
- saída → `InterpolatedYaw` (ou direto no `Make Rotator`)

### Opção A (recomendada) — um nó

1. Clique direito → **`Apply Interpolated Network Yaw To Actor`** (`Umbra | Net | WS | State`)
2. Conecte:
   - `Target Actor` = `RemoteActorRef` (após `Is Valid`)
   - `From Yaw` = `StateA_Yaw`
   - `To Yaw` = `StateB_Yaw`
   - `Alpha` = `ClampedAlpha`
3. Remova a cadeia antiga: `Lerp` → `Make Rotator` → `Set Actor Rotation` (se o helper já seta a rotação).
4. Opcional: grave `Out Applied Yaw` em `InterpolatedYaw` para debug.

### Opção B — só interpolação

1. Clique direito → **`Interpolate Network Yaw Degrees`**
2. Mesmas entradas do `Lerp` antigo
3. Saída → `Yaw` do `Make Rotator` (Roll=0, Pitch=0) → `Set Actor Rotation` em `RemoteActorRef`
4. Apague o `Lerp` antigo
5. Remova offsets `+90` / `+180` no `Make Rotator` se existirem

Compile e Salve o BP.

> A função só aparece após rebuild C++ do editor. Se não achar, `Build` → `Compile UmbraEternumUE`.

---

## 3) Spawn do remote

No `SpawnActorFromClass(BP_RemotePlayer)`:

- `Make Transform` → `Rotation` = `Make Rotator(Yaw = OutYawDegrees)` (não Rotation zerada)

---

## 4) `BP_RemotePlayer` — defaults CMC

1. Components → **Character Movement**:
   - `Orient Rotation to Movement` = **false**
   - `Use Controller Desired Rotation` = **false**
2. Pawn: `Use Controller Rotation Yaw/Pitch/Roll` = **false**
3. Compile + Save

Em runtime o C++ já força isso em `BeginPlay` (`!IsLocallyControlled`) e em `RegisterRemoteActorInGameInstance`.

---

## 5) QA rápido (PIE 2 clients)

Filtro Output Log: `YawSend` OR `YawRecv` OR `Remote pawn detectado`

| Esperado |
|----------|
| `YawReal ≈ WireYaw` (ex.: 90 → 90, **não** 270) |
| No observador: `OutYaw` ≈ `WireYaw` do owner |
| Remote olha na mesma direção do owner ao andar |
| Log `Remote pawn detectado` / `Remote pawn CMC locked` ao spawnar |

Docs antigas com `Yaw+180` / `±90` no wire ou no parse estão **obsoletas** — não reaplicar.

Próximo (animação): após yaw ok, aplicar [`GUIA_BP_REMOTE_LOCOMOTION_VELOCITY_UE561.md`](GUIA_BP_REMOTE_LOCOMOTION_VELOCITY_UE561.md) (velocity = Δposição/Δt).

### Mesh Relative Rotation (obrigatório igual no local e remote)

`BP_Player` e `BP_RemotePlayer` devem ter o **mesmo** Mesh Relative Rotation (padrão mannequin UE: **Yaw = -90**).  
Se o local estiver em 0 e o remote em -90, o mesmo `ActorYaw` da rede gera faces diferentes → numa tela parecem de frente, na outra de lado. Script: `UmbraEternumUE/scripts/align_remote_mesh_yaw.py`.
