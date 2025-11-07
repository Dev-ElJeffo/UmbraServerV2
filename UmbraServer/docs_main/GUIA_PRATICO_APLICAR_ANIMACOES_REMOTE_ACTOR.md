# 🔧 **GUIA PRÁTICO: Aplicar Dados de Animação ao Remote Actor**

## 📋 **PROBLEMA:**

Os frames de 34 bytes estão sendo recebidos e parseados corretamente, mas os dados de animação (`Speed`, `VelocityZ`, `IsInAir`) não estão sendo aplicados ao remote actor, então as animações não aparecem.

---

## ✅ **SOLUÇÃO: Adicionar Lógica de Animação no `ProcessNextFrame`**

### **LOCALIZAÇÃO:**

No `ProcessNextFrame`, após obter `RemoteActorRef` (seja spawnado ou existente), ANTES de `Set Actor Location` e `Set Actor Rotation`, adicione a lógica de animação.

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Verificar Estrutura Atual**

Seu `ProcessNextFrame` deve estar assim:

```
ParseStateUpdateFrameWithAnimation
  ↓
Branch (ReturnValue) → True
  ↓
[Filtro, Array_Find, Spawn/Update Actor]
  ↓
RemoteActorRef (obtido ou spawnado)
  ↓
Set Actor Location
Set Actor Rotation
```

---

### **PASSO 2: Adicionar Cast to Character**

**ANTES de `Set Actor Location`:**

1. **Adicione um nó `Cast to Character`:**
   - Clique direito → Busque: `Cast to Character`
   - Selecione: `Cast to Character`
   - **Object:** Conecte ao `RemoteActorRef` (variável ou output do `SpawnActorFromClass` ou `Get Array Item`)
   - **Outputs:** `As Character` (Character Reference)

2. **Adicione um nó `Branch`:**
   - Clique direito → Busque: `Branch`
   - **Condition:** Conecte ao `Cast Succeeded` (Boolean) do `Cast to Character`
   - **True:** É um Character → Continuar com animação
   - **False:** Não é Character → Pular animação e ir direto para `Set Actor Location`

---

### **PASSO 3: Obter Character Movement Component**

**No caminho `True` do `Branch` (Cast Successful):**

1. **Adicione um nó `Get Character Movement`:**
   - Clique direito → Busque: `Get Character Movement`
   - Selecione: `Get Character Movement` (função do Character)
   - **Target:** Conecte ao `As Character` do `Cast to Character`
   - **Return Value:** Character Movement Component Reference

---

### **PASSO 4: Calcular Velocity (Componente Horizontal)**

**Criar um Rotator com Yaw:**

1. **Adicione um nó `Make Rotator`:**
   - Clique direito → Busque: `Make Rotator`
   - **Pitch:** Conecte a uma constante `0.0`
   - **Yaw:** Conecte ao `OutYawDegrees` do `ParseStateUpdateFrameWithAnimation`
   - **Roll:** Conecte a uma constante `0.0`
   - **Return Value:** Rotator

2. **Adicione um nó `Get Forward Vector`:**
   - Clique direito → Busque: `Get Forward Vector`
   - Selecione: `Get Forward Vector` (função do Rotator)
   - **Target:** Conecte ao `Return Value` do `Make Rotator`
   - **Return Value:** FVector (direção normalizada)

3. **Adicione um nó `Multiply (Vector * Float)`:**
   - Clique direito → Busque: `Multiply`
   - Selecione: `Multiply (Vector * Float)`
   - **A:** Conecte ao `Return Value` do `Get Forward Vector`
   - **B:** Conecte ao `OutSpeed` do `ParseStateUpdateFrameWithAnimation`
   - **Return Value:** FVector (velocidade horizontal com magnitude correta)

4. **Adicione um nó `Break Vector`:**
   - Clique direito → Busque: `Break Vector`
   - **Input:** Conecte ao `Return Value` do `Multiply`
   - **Outputs:** `X`, `Y`, `Z`

---

### **PASSO 5: Combinar Velocity Horizontal e Vertical**

1. **Adicione um nó `Make Vector`:**
   - Clique direito → Busque: `Make Vector`
   - **X:** Conecte ao `X` do `Break Vector` (velocidade horizontal X)
   - **Y:** Conecte ao `Y` do `Break Vector` (velocidade horizontal Y)
   - **Z:** Conecte ao `OutVelocityZ` do `ParseStateUpdateFrameWithAnimation` (velocidade vertical)
   - **Return Value:** FVector (velocidade completa 3D)

---

### **PASSO 6: Aplicar Velocity ao Character Movement Component**

1. **Adicione um nó `Set Velocity`:**
   - Clique direito → Busque: `Set Velocity`
   - Selecione: `Set Velocity` (função do Character Movement Component)
   - **Target:** Conecte ao `Return Value` do `Get Character Movement`
   - **New Velocity:** Conecte ao `Return Value` do `Make Vector` (do PASSO 5)
   - **then:** Conecte ao próximo passo (ou a `Set Actor Location`)

---

### **PASSO 7: Aplicar Movement Mode (OPCIONAL - Mas Recomendado)**

**Após `Set Velocity`:**

1. **Adicione um nó `Branch`:**
   - Clique direito → Busque: `Branch`
   - **Condition:** Conecte ao `OutIsInAir` do `ParseStateUpdateFrameWithAnimation`
   - **True:** Player no ar → Set Movement Mode: Falling
   - **False:** Player no chão → Set Movement Mode: Walking

2. **No caminho `True` (IsInAir = true):**
   - **Adicione um nó `Set Movement Mode`:**
     - Clique direito → Busque: `Set Movement Mode`
     - Selecione: `Set Movement Mode` (função do Character Movement Component)
     - **Target:** Conecte ao `Return Value` do `Get Character Movement`
     - **New Movement Mode:** Selecione `MOVE_Falling` (ou valor equivalente no enum)
     - **then:** Conecte ao próximo passo

3. **No caminho `False` (IsInAir = false):**
   - **Adicione um nó `Set Movement Mode`:**
     - Clique direito → Busque: `Set Movement Mode`
     - Selecione: `Set Movement Mode` (função do Character Movement Component)
     - **Target:** Conecte ao `Return Value` do `Get Character Movement`
     - **New Movement Mode:** Selecione `MOVE_Walking` (ou valor equivalente no enum)
     - **then:** Conecte ao próximo passo

**⚠️ NOTA:** Se `Set Movement Mode` não estiver disponível ou os valores do enum não estiverem corretos, você pode **omitir completamente** esta parte. O Character Movement Component pode inferir o modo automaticamente baseado na `Velocity`.

---

### **PASSO 8: Continuar com Set Actor Location/Rotation**

**Após `Set Velocity` (e `Set Movement Mode` se aplicável):**

Conecte ao `Set Actor Location` e `Set Actor Rotation` existentes.

---

## 📊 **ESTRUTURA FINAL:**

```
ParseStateUpdateFrameWithAnimation (ReturnValue = True)
  ↓
Branch (ReturnValue) → True
  ↓
[Filtro: OutPlayerId != Active Player ID]
  ↓
Array_Find (RemoteActorIds, OutPlayerId)
  ↓
Branch (FoundIndex >= 0?)
  ├─ True: [Actor existe]
  │   ↓
  │   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │
  └─ False: [Actor não existe]
      ↓
      SpawnActorFromClass → RemoteActorRef
      ↓
      Array_Add (RemoteActorIds, OutPlayerId)
      ↓
      Array_Add (RemoteActors, RemoteActorRef)
  ↓
[APLICAR ANIMAÇÃO - NOVO]
  ↓
Cast to Character (RemoteActorRef)
  ↓
Branch (Cast Succeeded?)
  ├─ True: [É Character]
  │   ↓
  │   Get Character Movement
  │   ↓
  │   [CALCULAR VELOCITY]
  │   │
  │   OutYawDegrees → Make Rotator (0, OutYawDegrees, 0)
  │     ↓
  │   Get Forward Vector
  │     ↓
  │   Multiply (Vector * OutSpeed)
  │     ↓
  │   Break Vector → X, Y
  │   │
  │   OutVelocityZ
  │   ↓
  │   Make Vector (X, Y, OutVelocityZ) → New Velocity
  │   ↓
  │   Set Velocity
  │     ├─ Target: Get Character Movement
  │     ├─ New Velocity: [Do Make Vector]
  │     └─ then
  │   ↓
  │   Branch (OutIsInAir)
  │   ├─ True → Set Movement Mode: Falling (OPCIONAL)
  │   └─ False → Set Movement Mode: Walking (OPCIONAL)
  │
  └─ False: [Não é Character - pular animação]
  ↓
[CONTINUAR COM LÓGICA EXISTENTE]
Set Actor Location (OutLocation)
Set Actor Rotation (OutYawDegrees)
```

---

## 🎯 **PONTOS CRÍTICOS:**

1. **`Cast to Character` é OBRIGATÓRIO:** `RemoteActorRef` é um Actor Reference genérico, você precisa fazer `Cast to Character` para acessar `Get Character Movement`.

2. **Ordem é IMPORTANTE:** `Set Velocity` deve ser ANTES de `Set Actor Location` para que o Character Movement Component processe a velocidade corretamente.

3. **Speed = 0 Quando Player Está Parado:** Se o player está parado, `Speed`, `VelocityZ` e `IsInAir` serão zerados. Isso é esperado. Para testar animações, o player precisa estar se movendo.

---

## 🔍 **TESTE:**

1. **Mova o player** e verifique os logs:
   - Se `Speed > 0` quando se move → **CORRETO**
   - Se `Speed = 0` quando se move → **PROBLEMA:** Cálculo de Speed está incorreto

2. **Verifique se `Set Velocity` está sendo executado:**
   - Adicione um log após `Set Velocity`
   - Se o log aparecer → **CORRETO:** `Set Velocity` está sendo executado
   - Se o log não aparecer → **PROBLEMA:** `Cast to Character` pode estar falhando ou há um erro na conexão

---

**Fim do Guia**

