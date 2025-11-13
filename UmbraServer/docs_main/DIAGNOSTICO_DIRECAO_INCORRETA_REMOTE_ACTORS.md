# 🔍 **DIAGNÓSTICO: Direção Incorreta dos Remote Actors**

## 🎯 **PROBLEMA:**

**Quando um personagem se move em linha reta paralelo ao personagem do client 1, ele aparece correndo para a direita no client 1.**

**Isso indica um problema de rotação (Yaw) sendo aplicado incorretamente aos remote actors.**

---

## 🔍 **ANÁLISE:**

### **1. Como o Yaw é Obtido (Local Character):**

**No `WSBinaryBPFL.cpp`:**
```cpp
OutYaw = Pawn->GetActorRotation().Yaw;
```

**O `GetActorRotation().Yaw` retorna:**
- **0°** = Norte (eixo Y positivo no Unreal)
- **90°** = Leste (eixo X positivo)
- **180°** = Sul (eixo Y negativo)
- **270°** = Oeste (eixo X negativo)

### **2. Como o Yaw é Aplicado (Remote Actors):**

**No Blueprint, no `Event Tick`:**
```
Make Rotator (Roll: 0, Pitch: 0, Yaw: InterpolatedYaw)
  ↓
Set Actor Rotation (Target: RemoteActorRef, Rotation: NewRotation)
```

**O `Make Rotator` com Yaw deve funcionar corretamente, mas pode haver um problema de:**
- **Offset de 90 graus** necessário
- **Sistema de coordenadas invertido**
- **Yaw sendo interpretado como direção de movimento em vez de direção de rotação**

---

## 🔍 **POSSÍVEIS CAUSAS:**

### **CAUSA 1: Yaw Está Sendo Interpretado como Direção de Movimento**

**Se o personagem está se movendo para frente (Y positivo), mas o Yaw é 0°, o remote actor pode estar sendo rotacionado para a direita (90°).**

**Solução:** Verificar se o Yaw está sendo obtido corretamente do Character local.

### **CAUSA 2: Offset de 90 Graus Necessário**

**No Unreal Engine, às vezes há um offset de 90 graus entre a direção de movimento e a rotação do actor.**

**Solução:** Adicionar ou subtrair 90 graus do Yaw antes de aplicar.

### **CAUSA 3: Sistema de Coordenadas Invertido**

**O Unreal usa um sistema left-handed, e pode haver diferenças entre como o Yaw é enviado e como é recebido.**

**Solução:** Verificar se há inversão de eixos.

---

## ✅ **SOLUÇÕES PROPOSTAS:**

### **SOLUÇÃO 1: Verificar Como o Yaw é Obtido**

**Adicione logs para verificar o Yaw sendo enviado e recebido:**

**No código que envia (local character):**
```cpp
float Yaw = Pawn->GetActorRotation().Yaw;
UE_LOG(LogTemp, Warning, TEXT("[SendMoveUpdate] Yaw enviado: %f graus"), Yaw);
```

**No código que recebe (remote actor):**
```cpp
UE_LOG(LogTemp, Warning, TEXT("[ReceiveUpdate] Yaw recebido: %f graus"), OutYawDegrees);
```

**No Blueprint, antes de aplicar:**
```
[Print String: "[Tick] Yaw a ser aplicado: {InterpolatedYaw} graus"]
```

### **SOLUÇÃO 2: Adicionar Offset de 90 Graus**

**Se o problema for um offset de 90 graus, adicione no Blueprint:**

```
[InterpolatedYaw]
  ↓
[Float + Float] (A: InterpolatedYaw, B: 90.0)
  ↓
[Make Rotator (Yaw: resultado)]
```

**OU subtrair 90 graus:**
```
[InterpolatedYaw]
  ↓
[Float - Float] (A: InterpolatedYaw, B: 90.0)
  ↓
[Make Rotator (Yaw: resultado)]
```

### **SOLUÇÃO 3: Usar Direção de Movimento em Vez de Yaw**

**Se o problema for que o Yaw não representa a direção de movimento, calcule a direção a partir da velocidade:**

**No Blueprint:**
```
[Get Velocity] (do Character local)
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector2D (X: X, Y: Y)] → Velocity2D
  ↓
[Vector2D Length] → Speed
  ↓
[Branch: Speed > 0.1?]
  ├─ True:
  │    ↓
  │  [Vector2D Normalize] → Direction2D
  │    ↓
  │  [Atan2 (Y: Direction2D.Y, X: Direction2D.X)] → Yaw em radianos
  │    ↓
  │  [Radians to Degrees] → Yaw em graus
  └─ False:
       ↓
     [Get Actor Rotation] → [Break Rotator] → [Yaw] (usar rotação atual)
```

---

## 🧪 **TESTE:**

1. **Adicione logs** para verificar o Yaw sendo enviado e recebido
2. **Execute o jogo** e mova o personagem em linha reta
3. **Verifique os logs:**
   - Qual é o Yaw sendo enviado?
   - Qual é o Yaw sendo recebido?
   - Qual é o Yaw sendo aplicado?

4. **Teste com offset de 90 graus:**
   - Adicione +90 graus ao Yaw antes de aplicar
   - Teste novamente
   - Se melhorar, o problema é offset
   - Se piorar, tente -90 graus

---

## 🎯 **PRÓXIMOS PASSOS:**

1. ✅ Adicionar logs para diagnosticar
2. ✅ Testar com offset de 90 graus
3. ✅ Verificar se o problema é específico de um client ou todos
4. ✅ Verificar se há diferença entre movimento para frente vs para trás

**Com esses testes, podemos identificar a causa exata e aplicar a correção correta!**

