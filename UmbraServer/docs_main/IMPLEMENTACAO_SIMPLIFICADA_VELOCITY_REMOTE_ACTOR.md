> **OBSOLETO para velocity remota (2026-08):** não usar mais `Forward(Yaw) * OutSpeed` no remote — causa moonwalk em ré/strafe. Usar `ComputeRemoteLocomotionVelocity` / `ApplyRemoteLocomotionVelocity` (Δposição/Δt). Ver [`docs_main/GUIA_BP_REMOTE_LOCOMOTION_VELOCITY_UE561.md`](../../docs_main/GUIA_BP_REMOTE_LOCOMOTION_VELOCITY_UE561.md).

# 🎯 **IMPLEMENTAÇÃO SIMPLIFICADA: Atualização de Velocity no Remote Actor**

## 📋 **OBJETIVO:**

Simplificar a atualização de Velocity no remote actor para que o Animation Blueprint funcione automaticamente.

---

## 🔍 **ANÁLISE:**

O Animation Blueprint do Unreal Engine lê automaticamente do `CharacterMovementComponent`:
- `Velocity` → Para calcular `Speed` e `Direction`
- `Movement Mode` → Para determinar `IsInAir`

Portanto, não precisamos calcular Velocity complexa. Podemos usar uma aproximação simples baseada em `Speed` e `Yaw`.

---

## 🔧 **SOLUÇÃO SIMPLIFICADA:**

### **Opção 1: Usar Rotator e Forward Vector (RECOMENDADA)**

```
OutSpeed
  ↓
Make Rotator
  ├─ Pitch: 0.0
  ├─ Yaw: OutYawDegrees
  └─ Roll: 0.0
  ↓
Get Forward Vector
  ↓
Multiply (Vector * Float)
  ├─ A: Forward Vector
  └─ B: OutSpeed
  ↓
Break Vector → X, Y
  ↓
Make Vector
  ├─ X: [Do Multiply X]
  ├─ Y: [Do Multiply Y]
  └─ Z: OutVelocityZ
  ↓
Set Velocity (Character Movement Component)
```

---

### **Opção 2: Usar Funções Matemáticas (Alternativa)**

```
OutSpeed
  ↓
Multiply (Float * Float)
  ├─ A: OutSpeed
  └─ B: cos(OutYawDegrees em radianos)
  ↓
[Repetir para sin]
  ↓
Make Vector
  ├─ X: Speed * cos(Yaw)
  ├─ Y: Speed * sin(Yaw)
  └─ Z: OutVelocityZ
  ↓
Set Velocity
```

---

### **Opção 3: Usar Velocity Existente e Ajustar (MAIS SIMPLES)**

Se o remote actor já tem uma Velocity (mesmo que errada), podemos apenas:
1. Obter Velocity atual
2. Calcular magnitude XY
3. Normalizar e multiplicar por `OutSpeed`
4. Manter Z como `OutVelocityZ`

```
Get Character Movement
  ↓
Get Velocity
  ↓
Break Vector → X, Y, Z
  ↓
Make Vector (X, Y, 0)
  ↓
Vector Length → CurrentSpeed
  ↓
[Se CurrentSpeed > 0:]
  Normalize (X, Y, 0) → Normalized
  ↓
Multiply (Vector * Float)
  ├─ A: Normalized
  └─ B: OutSpeed
  ↓
Break Vector → X, Y
  ↓
Make Vector (X, Y, OutVelocityZ)
  ↓
Set Velocity
```

---

## 🎯 **IMPLEMENTAÇÃO RECOMENDADA (Opção 1):**

### **No Blueprint `ProcessNextFrame` - Após obter `RemoteActorRef`:**

```
[Após Set Variable: RemoteActorRef]
  ↓
Cast to Character
  ├─ Object: RemoteActorRef
  ↓ (Branch True)
Get Character Movement
  ├─ Target: Cast to Character → Return Value
  ↓
[PARALELO - Calcular Velocity:]
  ├─ Make Rotator
  │   ├─ Pitch: 0.0
  │   ├─ Yaw: OutYawDegrees
  │   └─ Roll: 0.0
  │   ↓
  │   Get Forward Vector
  │   ↓
  │   Multiply (Vector * Float)
  │   ├─ A: Forward Vector
  │   └─ B: OutSpeed
  │   ↓
  │   Break Vector → X, Y
  │   ↓
  │   Make Vector
  │   ├─ X: [Do Multiply X]
  │   ├─ Y: [Do Multiply Y]
  │   └─ Z: OutVelocityZ
  │
  └─ [OPCIONAL: Se Movement Mode enum estiver disponível]
     Branch (OutIsInAir)
       ├─ True → Set Movement Mode: MOVE_Falling (ou valor equivalente)
       └─ False → Set Movement Mode: MOVE_Walking (ou valor equivalente)
  ↓
Set Velocity
  ├─ Target: Get Character Movement
  ├─ New Velocity: [Do Make Vector acima]
  └─ then
  ↓
[OPCIONAL: Conectar ao Set Movement Mode do Branch acima]
  ↓
Set Actor Location / Rotation (lógica existente)
```

---

## ⚠️ **NOTA IMPORTANTE:**

**`Set Movement Mode` é OPCIONAL:**

O `CharacterMovementComponent` pode inferir o modo automaticamente baseado na `Velocity` definida. Se o enum `Movement Mode` não tiver os valores `Falling` ou `Walking` disponíveis, você pode omitir essa parte e apenas definir `Velocity`. O Animation Blueprint ainda funcionará corretamente lendo a `Velocity` diretamente.

**Se o Animation Blueprint não estiver atualizando automaticamente:**

Você pode precisar expor variáveis no Animation Blueprint e atualizá-las manualmente:

1. **No Animation Blueprint:**
   - Expor variáveis: `Speed`, `IsInAir`, `VelocityZ`
   
2. **No Blueprint `ProcessNextFrame`:**
   ```
   RemoteActorRef
     ↓
   Get Mesh
     ↓
   Get Anim Instance
     ↓
   Cast to [Seu Animation Blueprint Class]
     ↓
   Set Speed: OutSpeed
   Set IsInAir: OutIsInAir
   Set VelocityZ: OutVelocityZ
   ```

Mas isso só é necessário se o Animation Blueprint não ler automaticamente do `CharacterMovementComponent`.

---

**Fim do Documento**

