# 🔍 **DIAGNÓSTICO: Direção Incorreta - Personagem Virado para Direita**

## 📊 **PROBLEMA:**

Quando um personagem está virado totalmente para a frente, outro cliente o vê virado para a direita (90 graus de diferença).

---

## 🔍 **ANÁLISE:**

### **Sistema de Coordenadas do Unreal Engine:**

No Unreal Engine:
- **X** = Forward/Backward (vermelho) - Norte/Sul
- **Y** = Right/Left (verde) - Leste/Oeste  
- **Z** = Up/Down (azul)

### **Rotator no Unreal Engine:**

`Make Rotator` usa `(Roll, Pitch, Yaw)`:
- **Roll** = Rotação em torno do eixo X (Forward)
- **Pitch** = Rotação em torno do eixo Y (Right)
- **Yaw** = Rotação em torno do eixo Z (Up)

### **Get Forward Vector:**

`Get Forward Vector` de um `Rotator` retorna o vetor Forward baseado no sistema de coordenadas do Unreal:
- **Yaw = 0°** → Forward Vector = **(1.0, 0.0, 0.0)** (Norte/X+)
- **Yaw = 90°** → Forward Vector = **(0.0, 1.0, 0.0)** (Leste/Y+)
- **Yaw = 180°** → Forward Vector = **(-1.0, 0.0, 0.0)** (Sul/X-)
- **Yaw = 270°** → Forward Vector = **(0.0, -1.0, 0.0)** (Oeste/Y-)

---

## ❌ **PROBLEMA IDENTIFICADO:**

### **Causa Raiz:**

O problema pode estar em **dois lugares**:

1. **Cálculo da `Velocity` no `ProcessNextFrame`:**
   - Quando calculamos `New Velocity` usando `Get Forward Vector` de um `Rotator` criado com `Make Rotator(0, OutYawDegrees, 0)`, estamos usando o sistema de coordenadas do Unreal corretamente.
   - **MAS** se o `OutYawDegrees` está sendo recebido/enviado incorretamente, a direção será errada.

2. **Uso de `Get Forward Vector` vs `Get Right Vector`:**
   - Se estamos usando `Get Forward Vector` quando deveríamos usar `Get Right Vector` (ou vice-versa), teremos uma rotação de 90 graus.

3. **`CalculateDirection` no Animation Blueprint:**
   - O `CalculateDirection` usa `Velocity` e `BaseRotation`.
   - Se a `Velocity` está sendo calculada usando o eixo errado, a direção calculada será incorreta.

---

## 🔧 **SOLUÇÃO:**

### **SOLUÇÃO 1: Verificar Cálculo de `New Velocity`**

No `ProcessNextFrame`, verificar se estamos usando `Get Forward Vector` corretamente:

**Atual (pode estar errado):**
```
OutYawDegrees → Make Rotator (Roll=0, Pitch=0, Yaw=OutYawDegrees)
  ↓
Get Forward Vector
  ↓
Multiply (Vector * OutSpeed)
  ↓
Break Vector → X, Y
  ↓
Make Vector (X, Y, OutVelocityZ)
```

**Verificação:**
- `Make Rotator` deve usar `(Roll, Pitch, Yaw)` = `(0, 0, OutYawDegrees)`
- `Get Forward Vector` deve retornar o vetor correto baseado no Yaw

### **SOLUÇÃO 2: Verificar `GetActorRotation` no `SendMoveUpdate`**

Verificar se o `Yaw` está sendo extraído corretamente:

**Atual (deve estar correto):**
```
Get Player Pawn
  ↓
Get Actor Rotation
  ↓
Return Value.Yaw → YawDegrees
```

**Verificação:**
- O `Yaw` extraído de `Get Actor Rotation` deve estar em graus (0-360)
- Deve representar a rotação em torno do eixo Z (Up)

### **SOLUÇÃO 3: Verificar `Set Actor Rotation` no `ProcessNextFrame`**

Verificar se estamos usando `Make Rotator` corretamente:

**Atual (pode estar errado):**
```
InterpolatedYaw → Make Rotator (Roll=0, Pitch=0, Yaw=InterpolatedYaw)
  ↓
Set Actor Rotation
```

**Verificação:**
- `Make Rotator` deve usar `(Roll, Pitch, Yaw)` = `(0, 0, InterpolatedYaw)`
- O `InterpolatedYaw` deve estar em graus (0-360)

---

## 🎯 **SOLUÇÃO RECOMENDADA:**

### **Teste 1: Adicionar Logs**

Adicionar logs para verificar os valores:

**No `SendMoveUpdate`:**
```
Get Actor Rotation → Yaw
  ↓
Print String: "SendMoveUpdate - Yaw: {Yaw}"
```

**No `ProcessNextFrame`:**
```
ParseStateUpdateFrameWithAnimation → OutYawDegrees
  ↓
Print String: "ProcessNextFrame - OutYawDegrees: {OutYawDegrees}"
  ↓
Make Rotator (0, 0, OutYawDegrees)
  ↓
Get Forward Vector
  ↓
Print String: "Forward Vector: X={X}, Y={Y}, Z={Z}"
```

### **Teste 2: Verificar se `Get Forward Vector` está correto**

Se o problema for que o personagem está virado 90 graus para a direita, pode ser que precisemos usar `Get Right Vector` em vez de `Get Forward Vector`:

**Teste alternativo:**
```
OutYawDegrees → Make Rotator (Roll=0, Pitch=0, Yaw=OutYawDegrees)
  ↓
Get Right Vector  ← TESTE: Usar Right em vez de Forward
  ↓
Multiply (Vector * OutSpeed)
  ↓
Break Vector → X, Y
  ↓
Make Vector (X, Y, OutVelocityZ)
```

**OU** pode ser que precisemos ajustar o Yaw:

```
OutYawDegrees → Subtract (OutYawDegrees - 90.0)  ← TESTE: Ajustar Yaw
  ↓
Make Rotator (Roll=0, Pitch=0, Yaw=AdjustedYaw)
  ↓
Get Forward Vector
  ↓
Multiply (Vector * OutSpeed)
```

### **Teste 3: Verificar `CalculateDirection` no Animation Blueprint**

O `CalculateDirection` usa `Velocity` e `BaseRotation`. Se a `Velocity` está sendo calculada usando o eixo errado, a direção calculada será incorreta.

**Verificação:**
- A `Velocity` deve estar alinhada com a direção do movimento
- A `BaseRotation` deve estar alinhada com a rotação do actor

---

## 📝 **IMPLEMENTAÇÃO:**

### **PASSO 1: Adicionar Logs**

1. **No `SendMoveUpdate`:**
   - Adicionar log após `Get Actor Rotation` para verificar o `Yaw` enviado

2. **No `ProcessNextFrame`:**
   - Adicionar log após `ParseStateUpdateFrameWithAnimation` para verificar o `OutYawDegrees` recebido
   - Adicionar log após `Get Forward Vector` para verificar o vetor calculado

### **PASSO 2: Testar Ajuste de Yaw**

Se os logs mostrarem que o `Yaw` está correto, mas a direção ainda está errada, testar:

1. **Ajustar Yaw em -90 graus:**
   ```
   OutYawDegrees → Subtract (OutYawDegrees - 90.0) → AdjustedYaw
   AdjustedYaw → Make Rotator (0, 0, AdjustedYaw)
   ```

2. **OU usar `Get Right Vector` em vez de `Get Forward Vector`:**
   ```
   Make Rotator (0, 0, OutYawDegrees) → Get Right Vector
   ```

### **PASSO 3: Verificar `Set Actor Rotation`**

Verificar se `Set Actor Rotation` está usando `Make Rotator` corretamente:

```
InterpolatedYaw → Make Rotator (Roll=0, Pitch=0, Yaw=InterpolatedYaw)
  ↓
Set Actor Rotation
```

---

## ✅ **VERIFICAÇÃO:**

Após aplicar as correções:

1. **Logs:**
   - [ ] `Yaw` enviado no `SendMoveUpdate` está correto?
   - [ ] `OutYawDegrees` recebido no `ProcessNextFrame` está correto?
   - [ ] `Forward Vector` calculado está correto?

2. **Direção:**
   - [ ] Personagem remoto está virado na direção correta?
   - [ ] Animações estão na direção correta?

---

## 🎯 **RESUMO:**

**Causa provável:** Problema de conversão de coordenadas ou uso incorreto de `Get Forward Vector` vs `Get Right Vector`.

**Solução:**
1. Adicionar logs para verificar valores
2. Testar ajuste de Yaw (-90 graus)
3. Testar usar `Get Right Vector` em vez de `Get Forward Vector`
4. Verificar `Set Actor Rotation` está usando `Make Rotator` corretamente

**Próximo passo:** Adicionar logs e testar ajuste de Yaw ou uso de `Get Right Vector`.

