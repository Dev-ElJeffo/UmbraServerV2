> **OBSOLETO (2026-08):** não reaplicar `Yaw ±180` no parse/wire. Causa raiz e fix: remover `360 - Yaw` + CMC remote — ver [`GUIA_MOVIMENTO_REMOTE_YAW.md`](GUIA_MOVIMENTO_REMOTE_YAW.md) e [`docs_main/GUIA_BP_APLICAR_YAW_REMOTE_UE561.md`](../../docs_main/GUIA_BP_APLICAR_YAW_REMOTE_UE561.md).

# 🔧 **CORREÇÃO: Yaw e Animação de Direção**

## 🎯 **PROBLEMAS:**

1. **Rotação ainda invertida** (mesmo com +180 graus)
2. **Animação mostra personagem andando para o lado** quando se move em linha reta

---

## ✅ **SOLUÇÃO 1: Tentar Subtrair 180 Graus**

**Mudei de adicionar para subtrair 180 graus ao Yaw:**

**ANTES:**
```cpp
OutYawDegrees = Yaw + 180.0f;
```

**DEPOIS:**
```cpp
OutYawDegrees = Yaw - 180.0f;
```

**Teste isso primeiro!**

---

## ✅ **SOLUÇÃO 2: Corrigir Animação Usando Velocidade**

**O problema da animação é que ela está usando a rotação do Actor em vez da direção de movimento (velocidade).**

### **PROBLEMA:**

**A animação está calculando a direção baseada no Yaw (rotação), mas deveria usar a velocidade (direção real do movimento).**

### **SOLUÇÃO:**

**No Blueprint, quando aplicar velocidade aos remote actors, use a velocidade calculada a partir do Yaw e Speed:**

**No `ProcessNextFrame` ou `OnWSBinaryMessage`, após parsear:**

```
[ParseStateUpdateFrameWithAnimation] → OutYawDegrees, OutSpeed, OutVelocityZ
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: OutYawDegrees)] → MovementRotation
  ↓
[Get Forward Vector] (do MovementRotation) → ForwardDirection
  ↓
[Multiply (Vector * Float)] (A: ForwardDirection, B: OutSpeed) → HorizontalVelocity
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector (X: X, Y: Y, Z: OutVelocityZ)] → FinalVelocity
  ↓
[Set Velocity] (do Character Movement Component)
```

**Isso garante que a velocidade está na direção correta do movimento!**

---

## 🎯 **SOLUÇÃO ALTERNATIVA: Calcular Yaw a Partir da Velocidade**

**Se o problema persistir, calcule o Yaw a partir da velocidade em vez de usar o Yaw recebido:**

**No Blueprint, após parsear:**

```
[ParseStateUpdateFrameWithAnimation] → OutSpeed, OutVelocityZ
  ↓
[Branch: OutSpeed > 0.1?]
  ├─ True (está se movendo):
  │    ↓
  │  [Make Rotator (Yaw: OutYawDegrees)] → [Get Forward Vector] → [Multiply * OutSpeed]
  │    ↓
  │  [Break Vector] → X, Y
  │    ↓
  │  [Make Vector2D (X: X, Y: Y)] → Velocity2D
  │    ↓
  │  [Vector2D Normalize] → Direction2D
  │    ↓
  │  [Atan2 (Y: Direction2D.Y, X: Direction2D.X)] → CalculatedYaw (radianos)
  │    ↓
  │  [Radians to Degrees] → CalculatedYaw (graus)
  │    ↓
  │  [Make Rotator (Roll: 0, Pitch: 0, Yaw: CalculatedYaw)] → CorrectedRotation
  │    ↓
  │  [Set Actor Rotation (Rotation: CorrectedRotation)]
  └─ False (não está se movendo):
       ↓
     [Make Rotator (Yaw: OutYawDegrees)] → [Set Actor Rotation]
```

---

## 🧪 **TESTE:**

1. **Compile** o projeto (C++ foi modificado - agora subtrai 180 graus)
2. **Execute** o jogo
3. **Verifique** se a rotação está correta
4. **Verifique** se a animação está correta quando se move em linha reta

**Se a rotação ainda estiver incorreta:**
- Tente **sem offset** (usar Yaw diretamente)
- Tente **+90 graus** ou **-90 graus**
- Tente **+270 graus** ou **-270 graus**

**Se a animação ainda estiver incorreta:**
- Verifique se a velocidade está sendo aplicada corretamente
- Verifique se o Animation Blueprint está usando a velocidade ou a rotação

---

## 📝 **NOTA IMPORTANTE:**

**A animação deve usar a direção de movimento (velocidade), não a rotação do Actor!**

**Se o Animation Blueprint está usando `GetActorRotation()` ou `GetActorForwardVector()`, ele pode estar usando a rotação errada.**

**O correto é usar `GetVelocity()` do Character Movement Component para calcular a direção da animação.**

---

## ✅ **RESULTADO ESPERADO:**

**Após aplicar as correções:**
- ✅ Remote actors virados na mesma direção que o local character
- ✅ Animação mostra personagem andando na direção correta do movimento
- ✅ Rotação e animação sincronizadas corretamente

**Com isso, ambos os problemas devem ser resolvidos!**

