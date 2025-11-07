# 🎬 **IMPLEMENTAÇÃO: Transmissão de Animações para Remote Players**

## 📋 **OBJETIVO:**

Adicionar transmissão de dados de animação via WebSocket para que os `BP_RemotePlayer` reproduzam as animações corretas (idle, walk, run, jump, etc.) baseadas no estado do movimento do player remoto.

---

## 🔍 **ANÁLISE DO SISTEMA ATUAL:**

### **Protocolo Binário Atual (25 bytes):**
```
[type:1][playerId:4][x:4][y:4][z:4][yaw:4][tsMs:4] = 25 bytes
```

### **O que está sendo transmitido:**
- ✅ Posição (Location X, Y, Z)
- ✅ Rotação (Yaw)
- ✅ Timestamp
- ❌ **Velocidade/Movimento** (não transmitido)
- ❌ **Estado de animação** (não transmitido)

### **O que precisa ser adicionado:**
- **Velocidade horizontal** (Speed) - para determinar walk vs run vs idle
- **Direção do movimento** (Movement Direction) - para animações de direção
- **Estado no ar** (IsInAir) - para animações de pulo/queda
- **Velocidade vertical** (Velocity Z) - para detectar pulo vs queda

---

## 🎯 **SOLUÇÃO PROPOSTA:**

### **Opção 1: Adicionar campos ao protocolo existente (RECOMENDADO)**

**Novo formato (33 bytes):**
```
[type:1][playerId:4][x:4][y:4][z:4][yaw:4][speed:4][velocityZ:4][isInAir:1][tsMs:4] = 34 bytes
```

**Campos adicionados:**
- `speed` (float): Velocidade horizontal (magnitude do movimento XY)
- `velocityZ` (float): Velocidade vertical (para detectar pulo/queda)
- `isInAir` (uint8): 0 = no chão, 1 = no ar

### **Opção 2: Protocolo estendido (mais flexível, mas maior)**

**Novo formato (37 bytes):**
```
[type:1][playerId:4][x:4][y:4][z:4][yaw:4][speed:4][velocityZ:4][isInAir:1][movementForward:4][movementRight:4][tsMs:4] = 38 bytes
```

**Campos adicionados:**
- `speed` (float): Velocidade horizontal
- `velocityZ` (float): Velocidade vertical
- `isInAir` (uint8): Estado no ar
- `movementForward` (float): Movimento para frente (-1 a 1)
- `movementRight` (float): Movimento para direita (-1 a 1)

**Recomendação:** Usar **Opção 1** inicialmente (mais simples, suficiente para animações básicas).

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **ETAPA 1: Modificar C++ - Adicionar campos ao protocolo**

#### **1.1. Modificar `WSBinaryBPFL.h`:**

Adicionar novas funções de serialização/deserialização:

```cpp
// Função existente (manter para compatibilidade):
UFUNCTION(BlueprintCallable, Category="Umbra|Net|WS|Binary", meta=(CallInEditor="true"))
static TArray<uint8> BuildMoveUpdateFrame(int32 PlayerId, const FVector& Location, float YawDegrees, int32 TimestampMs);

// NOVA função com animação:
UFUNCTION(BlueprintCallable, Category="Umbra|Net|WS|Binary", meta=(CallInEditor="true"))
static TArray<uint8> BuildMoveUpdateFrameWithAnimation(
    int32 PlayerId, 
    const FVector& Location, 
    float YawDegrees, 
    float Speed,
    float VelocityZ,
    bool IsInAir,
    int32 TimestampMs);

// Função de parse existente (manter para compatibilidade):
UFUNCTION(BlueprintCallable, Category="Umbra|Net|WS|Binary")
static bool ParseStateUpdateFrame(const TArray<uint8>& Data, int32& OutPlayerId, FVector& OutLocation, float& OutYawDegrees, int32& OutTimestampMs);

// NOVA função de parse com animação:
UFUNCTION(BlueprintCallable, Category="Umbra|Net|WS|Binary")
static bool ParseStateUpdateFrameWithAnimation(
    const TArray<uint8>& Data, 
    int32& OutPlayerId, 
    FVector& OutLocation, 
    float& OutYawDegrees,
    float& OutSpeed,
    float& OutVelocityZ,
    bool& OutIsInAir,
    int32& OutTimestampMs);
```

#### **1.2. Modificar `WSBinaryBPFL.cpp`:**

Implementar as novas funções:

```cpp
TArray<uint8> UWSBinaryBPFL::BuildMoveUpdateFrameWithAnimation(
    int32 PlayerId, 
    const FVector& Location, 
    float YawDegrees, 
    float Speed,
    float VelocityZ,
    bool IsInAir,
    int32 TimestampMs)
{
    TArray<uint8> OutBytes;
    OutBytes.Add(1u); // type = 1 (MoveUpdate)
    WriteU32LE(OutBytes, static_cast<uint32>(PlayerId));
    WriteF32LE(OutBytes, static_cast<float>(Location.X));
    WriteF32LE(OutBytes, static_cast<float>(Location.Y));
    WriteF32LE(OutBytes, static_cast<float>(Location.Z));
    WriteF32LE(OutBytes, YawDegrees);
    WriteF32LE(OutBytes, Speed);          // NOVO
    WriteF32LE(OutBytes, VelocityZ);     // NOVO
    OutBytes.Add(IsInAir ? 1u : 0u);      // NOVO (1 byte)
    WriteU32LE(OutBytes, static_cast<uint32>(TimestampMs));
    
    return OutBytes;
}

bool UWSBinaryBPFL::ParseStateUpdateFrameWithAnimation(
    const TArray<uint8>& Data, 
    int32& OutPlayerId, 
    FVector& OutLocation, 
    float& OutYawDegrees,
    float& OutSpeed,
    float& OutVelocityZ,
    bool& OutIsInAir,
    int32& OutTimestampMs)
{
    int32 Offset = 0;
    uint8 Type = 0;
    if (!ReadU8(Data, Offset, Type)) return false;
    if (Type != 2u) return false; // 2 = StateUpdate

    uint32 PlayerIdU32 = 0;
    float X=0, Y=0, Z=0, Yaw=0, Speed=0, VelocityZ=0;
    uint8 IsInAirU8 = 0;
    uint32 TsU32 = 0;

    if (!ReadU32LE(Data, Offset, PlayerIdU32)) return false;
    if (!ReadF32LE(Data, Offset, X)) return false;
    if (!ReadF32LE(Data, Offset, Y)) return false;
    if (!ReadF32LE(Data, Offset, Z)) return false;
    if (!ReadF32LE(Data, Offset, Yaw)) return false;
    if (!ReadF32LE(Data, Offset, Speed)) return false;        // NOVO
    if (!ReadF32LE(Data, Offset, VelocityZ)) return false;    // NOVO
    if (!ReadU8(Data, Offset, IsInAirU8)) return false;       // NOVO
    if (!ReadU32LE(Data, Offset, TsU32)) return false;

    OutPlayerId = static_cast<int32>(PlayerIdU32);
    OutLocation = FVector(X, Y, Z);
    OutYawDegrees = Yaw;
    OutSpeed = Speed;
    OutVelocityZ = VelocityZ;
    OutIsInAir = (IsInAirU8 != 0);
    OutTimestampMs = static_cast<int32>(TsU32);
    return true;
}
```

#### **1.3. Atualizar `ProcessBinaryBuffer`:**

O tamanho do frame muda de 25 para 34 bytes:

```cpp
// Constante: tamanho de um StateUpdateFrame = 34 bytes (com animação)
// Estrutura: [type:1][playerId:4][x:4][y:4][z:4][yaw:4][speed:4][velocityZ:4][isInAir:1][tsMs:4] = 34 bytes
constexpr int32 FrameSize = 34;
```

**⚠️ IMPORTANTE:** Manter compatibilidade com frames antigos (25 bytes) verificando o tamanho do buffer antes de parsear.

---

### **ETAPA 2: Modificar Blueprint - Enviar dados de animação**

#### **2.1. No `BP_NetMovementClient` - Função `SendMoveUpdate`:**

Antes de chamar `BuildMoveUpdateFrame`, adicionar lógica para obter dados de animação:

1. **Obter o Pawn do player local:**
   ```
   Get First Player Controller
     ↓
   Get Pawn
   ```

2. **Obter Character Movement Component:**
   ```
   Get Pawn
     ↓
   Get Character Movement
   ```

3. **Calcular Speed (velocidade horizontal):**
   ```
   Get Character Movement
     ↓
   Get Velocity
     ↓
   Break Vector → X, Y, Z
     ↓
   Make Vector (X, Y, 0)  ← Remove componente Z
     ↓
   Vector Length → Speed
   ```

4. **Obter Velocity Z:**
   ```
   Get Character Movement
     ↓
   Get Velocity
     ↓
   Break Vector → Z (ou usar Vector Z node)
   ```

5. **Obter IsInAir:**
   ```
   Get Character Movement
     ↓
   Is Falling → IsInAir (inverter se necessário)
   ```

6. **Chamar BuildMoveUpdateFrameWithAnimation:**
   ```
   BuildMoveUpdateFrameWithAnimation
     ├─ PlayerId: Get Active Player ID
     ├─ Location: Get Actor Location (Pawn)
     ├─ YawDegrees: Get Actor Rotation → Yaw
     ├─ Speed: [Do passo 3]
     ├─ VelocityZ: [Do passo 4]
     ├─ IsInAir: [Do passo 5]
     └─ TimestampMs: Get Game Time In Seconds → Converter para ms
   ```

---

### **ETAPA 3: Modificar Blueprint - Receber e aplicar animações**

#### **3.1. No `BP_NetMovementClient` - Função `ProcessNextFrame`:**

Após `ParseStateUpdateFrameWithAnimation`, aplicar dados de animação ao remote actor:

1. **Parsear frame com animação:**
   ```
   ParseStateUpdateFrameWithAnimation (Data)
     ├─ OutPlayerId
     ├─ OutLocation
     ├─ OutYawDegrees
     ├─ OutSpeed         ← NOVO
     ├─ OutVelocityZ    ← NOVO
     ├─ OutIsInAir      ← NOVO
     └─ OutTimestampMs
   ```

2. **Obter o Remote Actor:**
   ```
   Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
     ↓
   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
   ```

3. **Atualizar Animation Blueprint do Remote Actor:**

   **Opção A: Usar variáveis do Animation Blueprint diretamente**
   
   Se o `BP_RemotePlayer` tem um Animation Blueprint com variáveis expostas:
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

   **Opção B: Usar Character Movement Component (RECOMENDADO)**
   
   Atualizar o `CharacterMovementComponent` do remote actor para que o Animation Blueprint leia automaticamente:
   ```
   RemoteActorRef
     ↓
   Get Character Movement
     ↓
   Set Movement Mode: [Baseado em IsInAir]
     ├─ IsInAir = true → Set Movement Mode: Falling
     └─ IsInAir = false → Set Movement Mode: Walking
     ↓
   Set Velocity: Make Vector (Speed * Direction, Speed * Direction, VelocityZ)
   ```

   **⚠️ NOTA:** A segunda opção é mais automática, pois o Animation Blueprint já lê do `CharacterMovementComponent`.

---

### **ETAPA 4: Configurar BP_RemotePlayer**

#### **4.1. Garantir que BP_RemotePlayer tem Animation Blueprint:**

1. **No Blueprint Editor do `BP_RemotePlayer`:**
   - Aba "Components"
   - Selecionar "Mesh" (Skeletal Mesh Component)
   - Painel "Details" → "Animation"
   - Verificar se "Anim Class" está configurado
   - Se não estiver, configurar o mesmo Animation Blueprint usado no player local

#### **4.2. Configurar Character Movement Component:**

1. **No Blueprint Editor do `BP_RemotePlayer`:**
   - Aba "Components"
   - Selecionar "Character Movement"
   - Painel "Details" → "Movement Settings"
   - **"Max Walk Speed"**: Configurar para corresponder ao player local (ex: 500)
   - **"Can Ever Affect Navigation"**: Desmarcar (já deve estar desmarcado)

#### **4.3. Garantir que o Animation Blueprint está configurado corretamente:**

O Animation Blueprint deve ler variáveis do `CharacterMovementComponent`:
- `Speed`: Calculado automaticamente do `Velocity`
- `IsInAir`: Calculado automaticamente do `Movement Mode`
- `Direction`: Calculado automaticamente do `Velocity` e `Rotation`

**Se o Animation Blueprint não estiver usando essas variáveis automaticamente**, você precisará expor variáveis no Blueprint e atualizá-las manualmente (Opção A da ETAPA 3).

---

## 📊 **FLUXO COMPLETO:**

### **Envio (Client → Server):**
```
Player Local (Move)
  ↓
SendMoveUpdate (Event Tick ou Timer)
  ↓
Get Character Movement → Velocity, IsFalling
  ↓
Calculate Speed (Vector Length of Velocity XY)
  ↓
BuildMoveUpdateFrameWithAnimation
  ├─ Location
  ├─ Yaw
  ├─ Speed
  ├─ VelocityZ
  ├─ IsInAir
  └─ TimestampMs
  ↓
Send via WebSocket
```

### **Recepção (Server → Clients):**
```
Receive Binary Message
  ↓
ProcessBinaryBuffer
  ↓
ProcessNextFrame
  ↓
ParseStateUpdateFrameWithAnimation
  ├─ OutSpeed
  ├─ OutVelocityZ
  └─ OutIsInAir
  ↓
Get RemoteActorRef
  ↓
Update Character Movement Component
  ├─ Set Velocity (Speed * Direction, VelocityZ)
  └─ Set Movement Mode (Walking/Falling)
  ↓
Animation Blueprint atualiza automaticamente
```

---

## ⚠️ **COMPATIBILIDADE COM FRAMES ANTIGOS:**

### **Problema:**
- Frames antigos têm 25 bytes
- Frames novos têm 34 bytes
- Precisamos suportar ambos

### **Solução:**

Modificar `ProcessBinaryBuffer` para detectar o tamanho do frame:

```cpp
// Verificar tamanho do buffer
if (Buffer.Num() >= 34 && Buffer[0] == StateUpdateType)
{
    // Tentar parsear frame completo (34 bytes - com animação)
    // ...
}
else if (Buffer.Num() >= 25 && Buffer[0] == StateUpdateType)
{
    // Tentar parsear frame antigo (25 bytes - sem animação)
    // Usar valores padrão para Speed, VelocityZ, IsInAir
    // ...
}
```

**Ou usar um flag de versão no protocolo:**
- Adicionar um byte de versão após o `type`
- `type=2, version=1` → Frame antigo (25 bytes)
- `type=2, version=2` → Frame novo (34 bytes)

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Modificar C++ (`WSBinaryBPFL.h` e `.cpp`):**
   - Adicionar `BuildMoveUpdateFrameWithAnimation`
   - Adicionar `ParseStateUpdateFrameWithAnimation`
   - Atualizar `ProcessBinaryBuffer` para suportar 34 bytes

2. **Modificar Blueprint `BP_NetMovementClient`:**
   - Atualizar `SendMoveUpdate` para usar `BuildMoveUpdateFrameWithAnimation`
   - Adicionar lógica para obter Speed, VelocityZ, IsInAir
   - Atualizar `ProcessNextFrame` para usar `ParseStateUpdateFrameWithAnimation`
   - Adicionar lógica para atualizar `CharacterMovementComponent` do remote actor

3. **Testar:**
   - Verificar que frames são enviados corretamente
   - Verificar que remote players atualizam animações
   - Verificar compatibilidade com frames antigos (se necessário)

---

## 📝 **NOTAS IMPORTANTES:**

1. **Performance:**
   - Adicionar 9 bytes por frame aumenta o tráfego de rede em ~36%
   - Com 20 Hz de atualização: ~0.18 KB/s por player
   - Para 100 players: ~18 KB/s (aceitável)

2. **Animation Blueprint:**
   - O Animation Blueprint deve ler do `CharacterMovementComponent` automaticamente
   - Se não ler, será necessário expor variáveis e atualizá-las manualmente

3. **Interpolação:**
   - Velocidade e estado no ar podem ser interpolados junto com posição
   - Isso tornará as animações mais suaves

---

**Fim do Documento**

