# 📚 **GUIA DETALHADO: Calcular Yaw a Partir da Velocidade**

## 🎯 **OBJETIVO:**

**Calcular o Yaw (rotação) a partir da velocidade do movimento, em vez de usar o Yaw recebido do servidor.**

**Isso garante que a rotação corresponda à direção real do movimento!**

---

## 📋 **PASSO A PASSO COMPLETO:**

### **PASSO 1: Obter a Velocidade do Remote Actor**

**No Blueprint `BP_NetMovementClient`, no `ProcessNextFrame` ou `OnWSBinaryMessage`:**

1. **Após parsear** `ParseStateUpdateFrameWithAnimation`, você terá:
   - `OutSpeed` (Float) - velocidade horizontal
   - `OutYawDegrees` (Float) - Yaw recebido (vamos ignorar este)
   - `OutVelocityZ` (Float) - velocidade vertical

2. **Criar a velocidade a partir do Yaw e Speed:**
   - **Botão direito** → Procure por **`Make Rotator`**
   - **Inputs:**
     - **Roll:** `0.0` (constante Float)
     - **Pitch:** `0.0` (constante Float)
     - **Yaw:** `OutYawDegrees` (do ParseStateUpdateFrameWithAnimation)
   - **Output:** `Return Value` (Rotator)

3. **Obter o Forward Vector dessa rotação:**
   - **Botão direito** → Procure por **`Get Forward Vector`**
   - **Input `Target`:** Conecte ao `Return Value` do `Make Rotator`
   - **Output:** `Return Value` (Vector) - vetor normalizado apontando na direção do Yaw

4. **Multiplicar pelo Speed:**
   - **Botão direito** → Procure por **`Multiply (Vector * Float)`**
   - **Input A:** Conecte ao `Return Value` do `Get Forward Vector`
   - **Input B:** Conecte ao `OutSpeed` (do ParseStateUpdateFrameWithAnimation)
   - **Output:** `Return Value` (Vector) - velocidade horizontal (X, Y)

---

### **PASSO 2: Converter Vector para Vector2D**

**Agora vamos trabalhar apenas com X e Y (ignorar Z):**

1. **Break Vector:**
   - **Botão direito** → Procure por **`Break Vector`**
   - **Input:** Conecte ao `Return Value` do `Multiply (Vector * Float)`
   - **Outputs:** `X` (Float), `Y` (Float), `Z` (Float)

2. **Make Vector2D:**
   - **Botão direito** → Procure por **`Make Vector2D`**
   - **Input X:** Conecte ao `X` do `Break Vector`
   - **Input Y:** Conecte ao `Y` do `Break Vector`
   - **Output:** `Return Value` (Vector2D) - velocidade no plano horizontal (X, Y)

---

### **PASSO 3: Verificar se Está se Movendo**

**Antes de calcular a direção, precisamos verificar se há movimento:**

1. **Vector2D Length:**
   - **Botão direito** → Procure por **`Vector2D Length`**
   - **Input:** Conecte ao `Return Value` do `Make Vector2D`
   - **Output:** `Return Value` (Float) - magnitude da velocidade (Speed)

2. **Branch:**
   - **Botão direito** → Procure por **`Branch`**
   - **Input Condition:** Conecte ao `Return Value` do `Vector2D Length`
   - **Adicione um `Greater (Float > Float)`:**
     - **Botão direito** → Procure por **`Greater (Float > Float)`**
     - **Input A:** Conecte ao `Return Value` do `Vector2D Length`
     - **Input B:** `0.1` (constante Float) - velocidade mínima para considerar movimento
     - **Output:** Conecte ao `Condition` do `Branch`

---

### **PASSO 4: Calcular Direção (Se Está se Movendo)**

**No caminho `True` do `Branch` (está se movendo):**

1. **Vector2D Normalize:**
   - **Botão direito** → Procure por **`Vector2D Normalize`**
   - **Input:** Conecte ao `Return Value` do `Make Vector2D` (do PASSO 2)
   - **Output:** `Return Value` (Vector2D) - direção normalizada (comprimento = 1.0)

   **O que faz:** Converte o Vector2D em um vetor unitário (mantém a direção, mas comprimento = 1.0)
   
   **Exemplo:**
   - Se velocidade = (3.0, 4.0) → Normalize = (0.6, 0.8) - mesma direção, mas comprimento = 1.0

2. **Atan2:**
   - **Botão direito** → Procure por **`Atan2`** (ou **`Atan2 (Float, Float)`**)
   - **Input Y:** Conecte ao `Y` do `Return Value` do `Vector2D Normalize`
   - **Input X:** Conecte ao `X` do `Return Value` do `Vector2D Normalize`
   - **Output:** `Return Value` (Float) - ângulo em **radianos** (-π a +π)

   **O que faz:** Calcula o ângulo (em radianos) de um vetor (X, Y)
   
   **Fórmula:** `atan2(Y, X)` retorna o ângulo em radianos
   
   **Exemplos:**
   - Se (X=1.0, Y=0.0) → Atan2 = 0 radianos = 0 graus (leste)
   - Se (X=0.0, Y=1.0) → Atan2 = π/2 radianos = 90 graus (norte)
   - Se (X=-1.0, Y=0.0) → Atan2 = π radianos = 180 graus (oeste)
   - Se (X=0.0, Y=-1.0) → Atan2 = -π/2 radianos = -90 graus (sul)

3. **Radians to Degrees:**
   - **Botão direito** → Procure por **`Radians to Degrees`** (ou **`Degrees (Float)`**)
   - **Input:** Conecte ao `Return Value` do `Atan2`
   - **Output:** `Return Value` (Float) - ângulo em **graus** (-180 a +180)

   **O que faz:** Converte radianos para graus
   
   **Fórmula:** `graus = radianos * (180 / π)`
   
   **Exemplos:**
   - 0 radianos = 0 graus
   - π/2 radianos = 90 graus
   - π radianos = 180 graus
   - -π/2 radianos = -90 graus

4. **Normalizar para 0-360:**
   - **Botão direito** → Procure por **`Float + Float`**
   - **Input A:** Conecte ao `Return Value` do `Radians to Degrees`
   - **Input B:** `360.0` (constante Float)
   - **Botão direito** → Procure por **`Modulo (Float % Float)`** (ou **`% (Float)`**)
   - **Input A:** Conecte ao resultado do `Float + Float`
   - **Input B:** `360.0` (constante Float)
   - **Output:** `Return Value` (Float) - Yaw em graus (0 a 360)

   **O que faz:** Garante que o Yaw fique no range 0-360 graus
   
   **Exemplo:**
   - Se Atan2 retornar -90 graus → -90 + 360 = 270 → 270 % 360 = 270 graus ✅

---

### **PASSO 5: Usar Yaw Calculado ou Rotação Atual**

**No caminho `False` do `Branch` (não está se movendo):**

1. **Get Actor Rotation:**
   - **Botão direito** → Procure por **`Get Actor Rotation`**
   - **Input Target:** Conecte ao `RemoteActorRef`
   - **Output:** `Return Value` (Rotator)

2. **Break Rotator:**
   - **Botão direito** → Procure por **`Break Rotator`**
   - **Input:** Conecte ao `Return Value` do `Get Actor Rotation`
   - **Output:** `Yaw` (Float) - Yaw atual do Actor

---

### **PASSO 6: Aplicar o Yaw Calculado**

**Após calcular o Yaw (seja do movimento ou da rotação atual):**

1. **Make Rotator:**
   - **Botão direito** → Procure por **`Make Rotator`**
   - **Inputs:**
     - **Roll:** `0.0` (constante Float)
     - **Pitch:** `0.0` (constante Float)
     - **Yaw:** Conecte ao Yaw calculado (do PASSO 4 ou 5)
   - **Output:** `Return Value` (Rotator)

2. **Set Actor Rotation:**
   - **Botão direito** → Procure por **`Set Actor Rotation`**
   - **Input Target:** Conecte ao `RemoteActorRef`
   - **Input New Rotation:** Conecte ao `Return Value` do `Make Rotator`

---

## 📊 **FLUXO VISUAL COMPLETO:**

```
[ParseStateUpdateFrameWithAnimation] → OutSpeed, OutYawDegrees, OutVelocityZ
  ↓
[Make Rotator (Yaw: OutYawDegrees)] → ForwardDirection
  ↓
[Get Forward Vector] → ForwardVector
  ↓
[Multiply (Vector * Float)] (A: ForwardVector, B: OutSpeed) → VelocityXY
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector2D (X: X, Y: Y)] → Velocity2D
  ↓
[Vector2D Length] → Speed
  ↓
[Greater (Float > Float)] (A: Speed, B: 0.1) → IsMoving
  ↓
[Branch (Condition: IsMoving)]
  ├─ True (está se movendo):
  │    ↓
  │  [Vector2D Normalize] (Input: Velocity2D) → Direction2D
  │    ↓
  │  [Break Vector2D] → X, Y
  │    ↓
  │  [Atan2 (Y: Y, X: X)] → YawRadians
  │    ↓
  │  [Radians to Degrees] → YawDegrees (-180 a +180)
  │    ↓
  │  [Float + Float] (A: YawDegrees, B: 360.0) → YawPlus360
  │    ↓
  │  [Modulo (Float % Float)] (A: YawPlus360, B: 360.0) → CalculatedYaw (0 a 360)
  │    ↓
  │  [Make Rotator (Yaw: CalculatedYaw)] → NewRotation
  │
  └─ False (não está se movendo):
       ↓
     [Get Actor Rotation] (Target: RemoteActorRef) → CurrentRotation
       ↓
     [Break Rotator] → Yaw
       ↓
     [Make Rotator (Yaw: Yaw)] → NewRotation
       ↓
[Set Actor Rotation] (Target: RemoteActorRef, Rotation: NewRotation)
```

---

## 🎯 **EXPLICAÇÃO DETALHADA DOS NODES:**

### **Vector2D:**
- **O que é:** Um vetor 2D (apenas X e Y, sem Z)
- **Uso:** Para trabalhar com movimento horizontal (ignorar altura)
- **Exemplo:** (3.0, 4.0) = movimento 3 unidades para direita, 4 unidades para frente

### **Vector2D Normalize:**
- **O que faz:** Converte um Vector2D em um vetor unitário (comprimento = 1.0)
- **Uso:** Para obter apenas a direção, sem a magnitude
- **Exemplo:** (3.0, 4.0) → (0.6, 0.8) - mesma direção, mas comprimento = 1.0

### **Atan2:**
- **O que faz:** Calcula o ângulo (em radianos) de um vetor (X, Y)
- **Fórmula:** `atan2(Y, X)` retorna o ângulo em radianos
- **Range:** -π a +π radianos (-180 a +180 graus)
- **Uso:** Para converter direção (X, Y) em ângulo

### **Radians to Degrees:**
- **O que faz:** Converte radianos para graus
- **Fórmula:** `graus = radianos * (180 / π)`
- **Uso:** Para obter o ângulo em graus (mais fácil de trabalhar)

### **Modulo (%):**
- **O que faz:** Retorna o resto da divisão
- **Uso:** Para normalizar valores para um range específico (0-360)
- **Exemplo:** 450 % 360 = 90 (normaliza para 0-360)

---

## ✅ **RESULTADO ESPERADO:**

**Após implementar:**
- ✅ Yaw calculado a partir da direção real do movimento
- ✅ Rotação corresponde à direção de movimento
- ✅ Animação mostra personagem andando na direção correta

**Com isso, o problema de direção incorreta deve ser resolvido!**

