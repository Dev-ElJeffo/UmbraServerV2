> **OBSOLETO (2026-08):** não reaplicar offset ±90 no wire/parse. Ver [`GUIA_MOVIMENTO_REMOTE_YAW.md`](GUIA_MOVIMENTO_REMOTE_YAW.md).

# 🔧 **CORREÇÃO: Direção Incorreta - Offset de 90 Graus**

## 🎯 **PROBLEMA:**

**Quando um personagem se move em linha reta paralelo ao personagem do client 1, ele aparece correndo para a direita no client 1.**

**Isso indica um offset de 90 graus na rotação (Yaw) sendo aplicada aos remote actors.**

---

## 🔍 **CAUSA:**

**No Unreal Engine, há uma diferença entre:**
- **Direção de movimento** (Forward Vector)
- **Rotação do Actor** (Yaw)

**Quando o personagem se move para frente (Y positivo), o Yaw pode ser 0°, mas o remote actor precisa ser rotacionado para 90° (ou vice-versa) para aparecer correto.**

---

## ✅ **SOLUÇÃO: Adicionar Offset de 90 Graus no Blueprint**

### **PASSO 1: Localizar Onde o Yaw é Aplicado**

**No `BP_NetMovementClient`, no `Event Tick`:**

1. **Encontre** o node `Make Rotator` que cria a rotação para os remote actors
2. **Encontre** o `InterpolatedYaw` que é usado como input do `Make Rotator`

### **PASSO 2: Adicionar Offset de 90 Graus**

**ANTES (incorreto):**
```
[InterpolatedYaw]
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: InterpolatedYaw)]
  ↓
[Set Actor Rotation]
```

**DEPOIS (correto):**
```
[InterpolatedYaw]
  ↓
[Float + Float] (A: InterpolatedYaw, B: 90.0)
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: resultado)]
  ↓
[Set Actor Rotation]
```

**OU, se precisar subtrair 90 graus:**
```
[InterpolatedYaw]
  ↓
[Float - Float] (A: InterpolatedYaw, B: 90.0)
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: resultado)]
  ↓
[Set Actor Rotation]
```

---

## 📋 **IMPLEMENTAÇÃO DETALHADA:**

### **No Blueprint `BP_NetMovementClient`, no `Event Tick`:**

1. **Encontre** o node `Make Rotator` que está sendo usado para aplicar rotação aos remote actors
2. **Antes** do `Make Rotator`, adicione um node `Float + Float` (ou `Float - Float`)
3. **Conecte:**
   - **A:** `InterpolatedYaw` (do Lerp anterior)
   - **B:** `90.0` (constante Float)
4. **Conecte** o resultado ao input `Yaw` do `Make Rotator`

**Fluxo completo:**
```
[Lerp (Float): StateA_Yaw → StateB_Yaw] → InterpolatedYaw
  ↓
[Float + Float] (A: InterpolatedYaw, B: 90.0) → AdjustedYaw
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: AdjustedYaw)] → NewRotation
  ↓
[Set Actor Rotation (Target: RemoteActorRef, Rotation: NewRotation)]
```

---

## 🧪 **TESTE:**

1. **Compile** o Blueprint
2. **Execute** o jogo
3. **Mova** um personagem em linha reta
4. **Verifique** se o remote actor está apontando na direção correta

**Se ainda estiver incorreto:**
- **Tente subtrair** 90 graus em vez de adicionar
- **Tente** 180 graus (inversão completa)
- **Tente** -90 graus

---

## 🎯 **ALTERNATIVA: Corrigir no Spawn Inicial**

**Se o problema também ocorre quando o remote actor é spawnado, adicione o offset no spawn:**

**No `OnWSBinaryMessage`, quando spawna um novo remote actor:**

```
[ParseStateUpdateFrame] → OutYawDegrees
  ↓
[Float + Float] (A: OutYawDegrees, B: 90.0) → AdjustedYaw
  ↓
[Make Rotator (Roll: 0, Pitch: 0, Yaw: AdjustedYaw)] → SpawnRotation
  ↓
[Make Transform (Location: OutLocation, Rotation: SpawnRotation, Scale: 1,1,1)]
  ↓
[Spawn Actor from Class (Transform: resultado)]
```

---

## 📝 **NOTA:**

**Se o offset de 90 graus não resolver, o problema pode ser:**
- **Sistema de coordenadas invertido** (X e Y trocados)
- **Yaw sendo interpretado incorretamente** (radianos vs graus)
- **Direção de movimento vs rotação** (usar velocidade em vez de Yaw)

**Nesses casos, será necessário:**
1. Adicionar logs para verificar os valores
2. Testar com diferentes offsets
3. Verificar se há inversão de eixos

---

## ✅ **RESULTADO ESPERADO:**

**Após aplicar o offset de 90 graus:**
- ✅ Remote actors apontam na direção correta do movimento
- ✅ Personagens se movendo em linha reta aparecem corretos
- ✅ Rotação sincronizada entre clients

**Com isso, o problema de direção incorreta deve ser resolvido!**

