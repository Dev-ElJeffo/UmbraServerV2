# 🔧 **GUIA PASSO A PASSO: Modificar Blueprint para Transmissão de Animações**

## 📋 **OBJETIVO:**

Modificar `SendMoveUpdate` e `ProcessNextFrame` no `BP_NetMovementClient` para transmitir e receber dados de animação.

---

## 🔧 **ETAPA 1: Modificar `SendMoveUpdate`**

### **OBJETIVO:**
Obter dados de animação do player local e enviar via WebSocket usando `BuildMoveUpdateFrameWithAnimation`.

---

### **PASSO 1.1: Localizar a Função `SendMoveUpdate`**

1. Abra `BP_NetMovementClient` no Blueprint Editor
2. No painel esquerdo, encontre a função `SendMoveUpdate`
3. Clique para abrir o grafo da função

---

### **PASSO 1.2: Obter o Pawn do Player Local**

**ANTES do `BuildMoveUpdateFrame` (ou onde quer que ele esteja):**

1. **Adicione um nó `Get First Player Controller`:**
   - Clique direito → Busque: `Get First Player Controller`
   - Selecione: `Get First Player Controller` (da categoria `Game`)
   - **World Context Object:** Conecte ao `Self` (ou deixe desconectado se auto-referenciar)

2. **Adicione um nó `Get Pawn`:**
   - Clique direito → Busque: `Get Pawn`
   - Selecione: `Get Pawn` (função do Player Controller)
   - **Target:** Conecte ao `Return Value` do `Get First Player Controller`
   - **Return Value:** Pawn do player local

---

### **PASSO 1.3: Obter Movement Base Actor**

**Após `Get Pawn`:**

1. **Adicione um nó `Get Movement Base Actor`:**
   - Clique direito → Busque: `Get Movement Base Actor`
   - Selecione: `Get Movement Base Actor` (função do Pawn)
   - **Target:** Conecte ao `Return Value` do `Get Pawn`
   - **Return Value:** Movement Base Actor (compatível com Character Movement Component)

**⚠️ NOTA:** `Get Movement Base Actor` funciona com qualquer Pawn que tenha movimento, não precisa fazer `Cast to Character`.

---

### **PASSO 1.4: Calcular Speed (Velocidade Horizontal)**

**Após `Get Movement Base Actor`:**

1. **Adicione um nó `Get Velocity`:**
   - Clique direito → Busque: `Get Velocity`
   - Selecione: `Get Velocity` (função do Movement Base Actor)
   - **Target:** Conecte ao `Return Value` do `Get Movement Base Actor`
   - **Return Value:** FVector (velocidade completa)

2. **Adicione um nó `Break Vector`:**
   - Clique direito → Busque: `Break Vector`
   - Selecione: `Break Vector`
   - **Input:** Conecte ao `Return Value` do `Get Velocity`
   - **Outputs:** `X`, `Y`, `Z`

3. **Adicione um nó `Make Vector`:**
   - Clique direito → Busque: `Make Vector`
   - Selecione: `Make Vector`
   - **X:** Conecte ao `X` do `Break Vector`
   - **Y:** Conecte ao `Y` do `Break Vector`
   - **Z:** Conecte a uma constante `0.0` (remove componente vertical)

4. **Adicione um nó `Vector Length`:**
   - Clique direito → Busque: `Vector Length`
   - Selecione: `Vector Length`
   - **Input:** Conecte ao `Return Value` do `Make Vector`
   - **Return Value:** Speed (magnitude da velocidade horizontal)

---

### **PASSO 1.5: Obter VelocityZ (Velocidade Vertical)**

**Após `Break Vector` (do PASSO 1.4):**

1. **Use o `Z` do `Break Vector`:**
   - O `Z` já está disponível do `Break Vector` feito anteriormente
   - Conecte diretamente ao `VelocityZ` do `BuildMoveUpdateFrameWithAnimation`

---

### **PASSO 1.6: Obter IsInAir (Estado no Ar)**

**Após `Get Movement Base Actor`:**

**⚠️ IMPORTANTE:** `Is Falling` não está disponível diretamente no `Character Movement Component` (requer `Nav Movement Interface`). Vamos usar `VelocityZ` para detectar se está no ar.

**OPÇÃO RECOMENDADA: Usar VelocityZ para detectar pulo/caindo:**

1. **Use o `Z` do `Break Vector` (do PASSO 1.4):**
   - Se `Z > 0.1` (velocidade vertical positiva) → está pulando/subindo
   - Se `Z < -0.1` (velocidade vertical negativa) → está caindo
   - Se `Z` está próximo de `0` → está no chão

2. **Adicione um nó `Abs` (valor absoluto):**
   - Clique direito → Busque: `Abs`
   - Selecione: `Abs (Float)`
   - **Input:** Conecte ao `Z` do `Break Vector`
   - **Return Value:** Valor absoluto de Z

3. **Adicione um nó `Greater`:**
   - Clique direito → Busque: `Greater`
   - Selecione: `Greater (Float)`
   - **A:** Conecte ao `Return Value` do `Abs`
   - **B:** Conecte a uma constante `0.1` (threshold)
   - **Return Value:** Boolean (true se velocidade vertical significativa = no ar)

**OPÇÃO ALTERNATIVA: Usar `Is Falling` do Nav Movement Interface (se disponível):**

Se o seu Character implementar `Nav Movement Interface`, você pode usar:

1. **Após `Cast to Character`:**
   - Busque por `Is Falling` e selecione a função do `Nav Movement Interface`
   - Conecte ao Character
   - Retorna Boolean

**Recomendação:** Usar OPÇÃO RECOMENDADA (`Abs(VelocityZ) > 0.1`) - mais simples e sempre disponível.

---

### **PASSO 1.7: Substituir `BuildMoveUpdateFrame` por `BuildMoveUpdateFrameWithAnimation`**

**Localize o nó `BuildMoveUpdateFrame` existente:**

1. **Delete o nó `BuildMoveUpdateFrame`** (ou desconecte todos os pins)
2. **Adicione um nó `BuildMoveUpdateFrameWithAnimation`:**
   - Clique direito → Busque: `BuildMoveUpdateFrameWithAnimation`
   - Selecione: `BuildMoveUpdateFrameWithAnimation`
   - **Pins de entrada:**
     - **PlayerId:** Conecte ao mesmo valor que estava conectado antes (geralmente `Get Active Player ID`)
     - **Location:** Conecte ao mesmo valor que estava conectado antes (geralmente `Get Actor Location`)
     - **YawDegrees:** Conecte ao mesmo valor que estava conectado antes (geralmente `Get Actor Rotation` → `Yaw`)
     - **Speed:** Conecte ao `Return Value` do `Vector Length` (PASSO 1.4)
     - **VelocityZ:** Conecte ao `Z` do `Break Vector` (PASSO 1.5)
     - **IsInAir:** Conecte ao `Return Value` do `Greater (Abs(VelocityZ) > 0.1)` (PASSO 1.6)
     - **TimestampMs:** Conecte ao mesmo valor que estava conectado antes

3. **Conecte o `Return Value` ao mesmo lugar que `BuildMoveUpdateFrame` estava conectado** (geralmente `Send Bytes` do WebSocket)

---

### **📊 ESTRUTURA FINAL DO `SendMoveUpdate`:**

```
SendMoveUpdate (Custom Event)
  ↓
[Lógica existente para obter Location, Yaw, Timestamp]
  ↓
Get First Player Controller
  ↓
Get Pawn
  ↓
Get Movement Base Actor
  ↓
[PARALELO:]
  ├─ Get Velocity
  │   ↓
  │   Break Vector → X, Y, Z
  │   ↓
  │   [PARALELO:]
  │   ├─ Make Vector (X, Y, 0) → Vector Length → Speed
  │   └─ Z → VelocityZ
  │
  └─ Abs (VelocityZ) → Greater (0.1) → IsInAir
  ↓
BuildMoveUpdateFrameWithAnimation
  ├─ PlayerId: [Valor existente]
  ├─ Location: [Valor existente]
  ├─ YawDegrees: [Valor existente]
  ├─ Speed: [Do Vector Length]
  ├─ VelocityZ: [Do Break Vector Z]
  ├─ IsInAir: [Do Greater (Abs(VelocityZ) > 0.1)]
  └─ TimestampMs: [Valor existente]
  ↓
Send Bytes (WebSocket)
```

---

## 🔧 **ETAPA 2: Modificar `ProcessNextFrame`**

### **OBJETIVO:**
Tentar parsear frame com animação primeiro, e se falhar (frame antigo), usar parse antigo. Aplicar dados de animação ao remote actor.

---

### **PASSO 2.1: Localizar a Função `ProcessNextFrame`**

1. Abra `BP_NetMovementClient` no Blueprint Editor
2. No painel esquerdo, encontre a função `ProcessNextFrame`
3. Clique para abrir o grafo da função

---

### **PASSO 2.2: Adicionar Tentativa de Parse com Animação**

**ANTES do `ParseStateUpdateFrame` atual:**

1. **Adicione um nó `ParseStateUpdateFrameWithAnimation`:**
   - Clique direito → Busque: `ParseStateUpdateFrameWithAnimation`
   - Selecione: `ParseStateUpdateFrameWithAnimation`
   - **Data:** Conecte ao mesmo array `Data` que `ParseStateUpdateFrame` recebe
   - **Outputs:** 
     - `OutPlayerId`
     - `OutLocation`
     - `OutYawDegrees`
     - `OutSpeed` ← NOVO
     - `OutVelocityZ` ← NOVO
     - `OutIsInAir` ← NOVO
     - `OutTimestampMs`
     - `ReturnValue` (Boolean)

2. **Adicione um nó `Branch`:**
   - Clique direito → Busque: `Branch`
   - Selecione: `Branch`
   - **Condition:** Conecte ao `ReturnValue` do `ParseStateUpdateFrameWithAnimation`
   - **True:** Frame novo (34 bytes) com animação → Continuar com animação
   - **False:** Frame antigo (25 bytes) → Tentar parse antigo

---

### **PASSO 2.3: Caminho True (Frame com Animação)**

**Após o `Branch` - Pin `True`:**

1. **As variáveis de saída já estão disponíveis:**
   - `OutPlayerId`, `OutLocation`, `OutYawDegrees`, `OutTimestampMs` (mesmas do parse antigo)
   - `OutSpeed`, `OutVelocityZ`, `OutIsInAir` (novas)

2. **Continue com a lógica existente:**
   - Filtro de `OutPlayerId != Active Player ID`
   - `Array_Find` para verificar se actor existe
   - Spawnar ou atualizar actor existente

3. **Após obter `RemoteActorRef` (seja spawnado ou existente), adicione:**

   **a) Obter Character Movement Component do Remote Actor:**
   ```
   RemoteActorRef
     ↓
   Cast to Character
     ↓ (Branch True)
   Get Character Movement
   ```

   **⚠️ IMPORTANTE:** `RemoteActorRef` é um Actor Reference, precisamos fazer `Cast to Character` primeiro para obter `Get Character Movement`.

   **b) Calcular `New Velocity` (Vetor de Velocidade 3D):**
   
   **OBJETIVO:** Converter `OutSpeed` (magnitude horizontal), `OutYawDegrees` (direção horizontal) e `OutVelocityZ` (velocidade vertical) em um vetor de velocidade 3D `(X, Y, Z)` que será usado no `Set Velocity`.
   
   **MÉTODO RECOMENDADO: Passo a Passo Detalhado**
   
   ```
   PASSO 1: Criar um Rotator com o Yaw recebido
   OutYawDegrees (exemplo: 90.0 graus = Leste)
     ↓
   Make Rotator
     ├─ Pitch: 0.0 (não inclinamos para cima/baixo - movimento horizontal)
     ├─ Yaw: OutYawDegrees (direção horizontal - já está em graus)
     └─ Roll: 0.0 (não rotacionamos em torno do eixo Z)
     ↓
   Return Value: Rotator (representa a direção do movimento no plano horizontal)
   
   PASSO 2: Obter o vetor Forward dessa direção
   Make Rotator → Return Value
     ↓
   Get Forward Vector
     ├─ Target: Make Rotator → Return Value
     └─ Return Value: FVector (vetor normalizado de comprimento 1.0)
     
     Exemplos:
     - Se Yaw = 0°   → Forward Vector ≈ (1.0, 0.0, 0.0)   [Norte/X+]
     - Se Yaw = 90°  → Forward Vector ≈ (0.0, 1.0, 0.0)   [Leste/Y+]
     - Se Yaw = 180° → Forward Vector ≈ (-1.0, 0.0, 0.0)  [Sul/X-]
     - Se Yaw = 45°  → Forward Vector ≈ (0.707, 0.707, 0.0) [Nordeste]
   
   PASSO 3: Multiplicar pelo Speed para obter a magnitude correta
   Get Forward Vector → Return Value
     ↓
   Multiply (Vector * Float)
     ├─ A: Get Forward Vector → Return Value (vetor direção normalizado, comprimento 1.0)
     └─ B: OutSpeed (magnitude da velocidade horizontal em unidades/segundo)
     ↓
   Return Value: FVector (velocidade horizontal com magnitude correta)
     
     Exemplo prático:
     - Forward Vector = (0.707, 0.707, 0.0) [45 graus]
     - OutSpeed = 500 unidades/segundo
     - Resultado = (353.5, 353.5, 0.0)
   
   PASSO 4: Separar componentes X e Y para adicionar Z separadamente
   Multiply → Return Value
     ↓
   Break Vector
     ├─ Input: Multiply → Return Value
     └─ Outputs: X, Y, Z
     
     Nota: O Z do Multiply será sempre 0 ou próximo, pois Forward Vector não tem componente vertical
   
   PASSO 5: Combinar tudo em um vetor final 3D
   Break Vector → X, Y (componentes horizontais)
   OutVelocityZ (valor separado recebido do frame binário)
     ↓
   Make Vector
     ├─ X: Break Vector → X (velocidade horizontal X em unidades/segundo)
     ├─ Y: Break Vector → Y (velocidade horizontal Y em unidades/segundo)
     └─ Z: OutVelocityZ (velocidade vertical em unidades/segundo)
     ↓
   Return Value: FVector (velocidade completa 3D)
     
     Exemplos práticos:
     
     CASO 1: Player correndo no chão (Yaw=45°, Speed=500, VelocityZ=0)
     - Forward Vector (45°) = (0.707, 0.707, 0.0)
     - Multiplicado por 500 = (353.5, 353.5, 0.0)
     - Com VelocityZ = 0 → Resultado Final = (353.5, 353.5, 0.0)
     
     CASO 2: Player pulando para frente (Yaw=90°, Speed=500, VelocityZ=200)
     - Forward Vector (90°) = (0.0, 1.0, 0.0)
     - Multiplicado por 500 = (0.0, 500.0, 0.0)
     - Com VelocityZ = 200 → Resultado Final = (0.0, 500.0, 200.0)
     
     CASO 3: Player caindo enquanto se move (Yaw=0°, Speed=300, VelocityZ=-400)
     - Forward Vector (0°) = (1.0, 0.0, 0.0)
     - Multiplicado por 300 = (300.0, 0.0, 0.0)
     - Com VelocityZ = -400 → Resultado Final = (300.0, 0.0, -400.0)
   ```
   
   **ENTENDENDO OS VALORES:**
   
   - **`OutSpeed`**: 
     - É a magnitude da velocidade horizontal (ex: 500 unidades/segundo)
     - Representa o quão rápido o player se move no plano horizontal (X, Y)
     - Não inclui movimento vertical
     - Valores típicos: 0 (parado) até 600-800 (correndo rápido)
   
   - **`OutYawDegrees`**: 
     - É o ângulo de rotação horizontal em graus
     - 0° = Norte (direção X positiva)
     - 90° = Leste (direção Y positiva)
     - 180° = Sul (direção X negativa)
     - 270° = Oeste (direção Y negativa)
     - Usado para determinar a direção do movimento horizontal
   
   - **`OutVelocityZ`**: 
     - É a velocidade vertical em unidades/segundo
     - `> 0` = subindo/pulando (ex: 200 = pulando alto)
     - `< 0` = caindo (ex: -400 = caindo rápido)
     - `≈ 0` = no chão (valores entre -0.1 e 0.1)
     - Usado para animações de pulo/queda
   
   **CONEXÃO NO BLUEPRINT:**
   
   O `Return Value` do último `Make Vector` (PASSO 5) é conectado diretamente ao pin `New Velocity` do nó `Set Velocity`:
   
   ```
   Make Vector (X, Y, OutVelocityZ)
     ↓
   Set Velocity
     ├─ Target: Get Character Movement
     ├─ New Velocity: [Return Value do Make Vector acima]
     └─ then
   ```

   **c) Usar `OutIsInAir` para atualizar Character Movement Component:**
   
   **OBJETIVO:** O `OutIsInAir` (Boolean) indica se o remote player está no ar (pulando/caindo) ou no chão. Use isso para atualizar o Character Movement Component corretamente.
   
   **FLUXO COMPLETO:**
   
   ```
   PASSO 1: Conectar OutIsInAir a um Branch
   OutIsInAir (Boolean do ParseStateUpdateFrameWithAnimation)
     ↓
   Branch
     ├─ Condition: OutIsInAir
     ├─ True: Player está no ar (pulando/caindo)
     └─ False: Player está no chão
   
   PASSO 2: Após Set Velocity, conectar ao Branch
   [Cálculo de Velocity do passo b acima]
     ↓
   Set Velocity
     ├─ Target: Get Character Movement
     ├─ New Velocity: [Do passo b]
     └─ then
     ↓
   Branch (OutIsInAir)
     ├─ True → [OPCIONAL: Set Movement Mode: Falling]
     └─ False → [OPCIONAL: Set Movement Mode: Walking]
   ```
   
   **EXPLICAÇÃO DETALHADA:**
   
   O `OutIsInAir` é uma saída Boolean do `ParseStateUpdateFrameWithAnimation`:
   - **`true`**: Player está no ar (pulando ou caindo)
   - **`false`**: Player está no chão
   
   **Onde conectar:**
   
   1. **Após `Set Velocity`**: 
      - Conecte o pin `then` do `Set Velocity` ao `Branch` que usa `OutIsInAir` como Condition
      - Isso garante que a velocidade seja definida primeiro, depois o modo de movimento (se aplicável)
   
   2. **No Branch:**
      - **Pin True**: Usado quando `OutIsInAir = true` (player no ar)
      - **Pin False**: Usado quando `OutIsInAir = false` (player no chão)
   
   3. **OPCIONAL - Set Movement Mode** (se o enum estiver disponível):
      ```
      Branch (OutIsInAir)
        ├─ True → Set Movement Mode
        │   ├─ Target: Get Character Movement
        │   ├─ New Movement Mode: MOVE_Falling (ou valor equivalente)
        │   └─ then → [Continuar fluxo]
        │
        └─ False → Set Movement Mode
            ├─ Target: Get Character Movement
            ├─ New Movement Mode: MOVE_Walking (ou valor equivalente)
            └─ then → [Continuar fluxo]
      ```
   
   **⚠️ NOTA IMPORTANTE:** 
   - `Set Movement Mode` é **OPCIONAL**. O Character Movement Component pode inferir o modo automaticamente baseado na `Velocity` definida.
   - Se os valores do enum `Movement Mode` (`Falling`, `Walking`) não estiverem disponíveis, você pode **omitir completamente** essa parte do `Set Movement Mode`.
   - O `OutIsInAir` ainda é útil para outras lógicas, como:
     - Atualizar variáveis do Animation Blueprint
     - Controlar efeitos visuais (partículas de pulo, etc.)
     - Lógica de gameplay específica
   
   **FLUXO SIMPLIFICADO (SEM Set Movement Mode):**
   
   Se você não quiser usar `Set Movement Mode`, o fluxo fica assim:
   
   ```
   [Cálculo de Velocity]
     ↓
   Set Velocity
     ├─ Target: Get Character Movement
     ├─ New Velocity: [Do passo b]
     └─ then
     ↓
   [Continuar com Set Actor Location / Rotation]
   ```
   
   O `OutIsInAir` ainda pode ser usado para outras lógicas se necessário, mas não é obrigatório conectá-lo diretamente ao Character Movement Component.

---

### **PASSO 2.4: Caminho False (Frame Antigo - Compatibilidade)**

**Após o `Branch` - Pin `False`:**

1. **Adicione o `ParseStateUpdateFrame` antigo:**
   - Este já deve existir no código
   - Se não existir, adicione normalmente
   - Conecte ao pin `False` do `Branch`

2. **Continue com a lógica existente:**
   - Esta parte permanece igual ao código atual
   - Não precisa de animação (valores padrão serão usados)

---

### **PASSO 2.5: Resumo Visual da Cálculo de Velocity**

**Fluxo completo para calcular `New Velocity`:**

```
OutYawDegrees + OutSpeed + OutVelocityZ
  ↓
[PARALELO]
  ├─ OutYawDegrees
  │   ↓
  │   Make Rotator (0, OutYawDegrees, 0)
  │   ↓
  │   Get Forward Vector
  │   ↓
  │   Multiply (Vector * Float)
  │   ├─ A: Forward Vector
  │   └─ B: OutSpeed
  │   ↓
  │   Break Vector → X, Y
  │
  └─ OutVelocityZ
  ↓
Make Vector
  ├─ X: [Do Break Vector X]
  ├─ Y: [Do Break Vector Y]
  └─ Z: OutVelocityZ
  ↓
Set Velocity → New Velocity
```

**⚠️ NOTA:** Veja o PASSO 2.3-b acima para explicação detalhada passo a passo com exemplos práticos.

**OU ainda mais simples - usar valores padrão:**

Se o Animation Blueprint já lê do `CharacterMovementComponent`, você pode apenas:
- Definir `Velocity` aproximada baseada em `Speed` e `Yaw`
- Definir `Movement Mode` baseado em `IsInAir`
- O Animation Blueprint fará o resto automaticamente

---

## 🔧 **ETAPA 3: Implementação Simplificada (RECOMENDADA)**

### **Para `SendMoveUpdate`:**

Mantenha a estrutura existente, apenas substitua `BuildMoveUpdateFrame` por `BuildMoveUpdateFrameWithAnimation` e adicione os cálculos de Speed, VelocityZ e IsInAir.

### **Para `ProcessNextFrame`:**

**Estrutura simplificada:**

```
ProcessNextFrame
  ↓
ProcessBinaryBuffer → OutFrame
  ↓
Break BinaryFrame → Data
  ↓
Get Array Item: Data[0] == 2?
  ↓
ParseStateUpdateFrameWithAnimation (tentar primeiro)
  ↓
Branch (ReturnValue)
  ├─ True: [Frame novo com animação]
  │   ├─ [Filtro, Array_Find, etc - lógica existente]
  │   ├─ [Obter RemoteActorRef]
  │   ├─ Cast to Character → Get Character Movement
  │   ├─ [Calcular Velocity baseado em OutSpeed e OutYawDegrees]
  │   ├─ Set Velocity
  │   ├─ [OPCIONAL: Branch (OutIsInAir) → Set Movement Mode]
  │   └─ Set Actor Location / Rotation (existente)
  │
  └─ False: [Tentar frame antigo]
      ├─ ParseStateUpdateFrame (fallback)
      └─ [Lógica existente sem animação]
```

---

## ⚠️ **NOTAS IMPORTANTES:**

1. **Manter Compatibilidade:**
   - Sempre tentar frame novo primeiro
   - Se falhar, usar frame antigo
   - Isso garante que clientes antigos continuem funcionando

2. **Velocity Calculation:**
   - O cálculo exato de Velocity pode ser simplificado
   - O importante é que `Speed` e `IsInAir` sejam corretos
   - O Animation Blueprint lerá do `CharacterMovementComponent` automaticamente

3. **Performance:**
   - Tentar parse novo primeiro tem custo mínimo
   - Se falhar, tenta parse antigo (compatibilidade)

---

## ✅ **CHECKLIST:**

### **SendMoveUpdate:**
- [ ] `Get First Player Controller` adicionado
- [ ] `Get Pawn` adicionado
- [ ] `Get Movement Base Actor` adicionado
- [ ] `Get Velocity` → `Break Vector` → `Make Vector` → `Vector Length` para Speed
- [ ] `Break Vector Z` para VelocityZ
- [ ] `Abs (VelocityZ)` → `Greater (0.1)` para IsInAir
- [ ] `BuildMoveUpdateFrame` substituído por `BuildMoveUpdateFrameWithAnimation`
- [ ] Todos os pins conectados corretamente

### **ProcessNextFrame:**
- [ ] `ParseStateUpdateFrameWithAnimation` adicionado ANTES do parse antigo
- [ ] `Branch` adicionado para escolher entre frame novo e antigo
- [ ] Caminho True (frame novo) atualiza `CharacterMovementComponent` (usando `Cast to Character`)
- [ ] Caminho False (frame antigo) usa parse antigo
- [ ] `Set Velocity` adicionado no caminho True (OPCIONAL: `Set Movement Mode` se enum disponível)

---

**Fim do Guia**

