# Correção Completa: Conexão WebSocket Após SelectCharacter com Classes C++

## 🎯 Problemas Identificados

### 1. **Porta do ZoneServer**
- O servidor está na porta **8082** (Zone ID 0) ou **8083** (Zone ID 1)
- O cliente precisa conectar na porta correta

### 2. **Falta de Conexão WebSocket Após SelectCharacter**
- Após `SelectCharacter` ser bem-sucedido, **não há lógica** que conecta ao WebSocket
- O `BP_NetMovementClient` precisa ser criado/spawnado após a seleção

### 3. **Erros de Blueprint no Nível `Lvl_TestAuth`**
- Erro: "Acessado 'None' ao tentar ler a propriedade CallFunc_Create_ReturnValue"
- Erro: "Acessado 'None' ao tentar ler a propriedade CallFunc_GetPlayerController_ReturnValue"

---

## 📚 Classes C++ Disponíveis

### 1. `UUmbraWSClient` (UmbraWSClient.h/.cpp)

**Classe que encapsula o WebSocket nativo do Unreal Engine.**

#### Métodos Estáticos:
- **`CreateUmbraWebSocket(FString Url)`** → `UUmbraWSClient*`
  - Cria uma instância do cliente WebSocket
  - Retorna objeto `UUmbraWSClient*` que pode ser usado no Blueprint
  - **IMPORTANTE**: Esta função é estática, use diretamente no Blueprint sem instância

#### Métodos de Instância:
- **`Connect()`** → `void`
  - Conecta ao servidor WebSocket
  - Deve ser chamado após criar o objeto
  
- **`Close()`** → `void`
  - Fecha a conexão WebSocket
  
- **`SendBytes(TArray<uint8> Data)`** → `bool`
  - Envia dados binários através do WebSocket
  - Retorna `true` se o envio foi iniciado com sucesso

#### Delegates (Eventos):
- **`OnConnected`** → `FUmbraWSOnConnected`
  - Disparado quando a conexão WebSocket é estabelecida com sucesso
  - Use `Bind Event to OnConnected` no Blueprint
  
- **`OnConnectionError`** → `FUmbraWSOnError` (parâmetro: `FString Error`)
  - Disparado quando há erro na conexão
  - Contém a mensagem de erro
  
- **`OnClosed`** → `FUmbraWSOnClosed`
  - Disparado quando a conexão é fechada
  
- **`OnRawMessage`** → `FUmbraWSOnRawMessage` (parâmetro: `TArray<uint8> Data`)
  - Disparado quando recebe mensagem binária do servidor
  - **USAR ESTE** para receber frames de movimento (não `OnMessage` que é para texto)

### 2. `UWSBinaryBPFL` (WSBinaryBPFL.h/.cpp)

**Blueprint Function Library para manipulação de frames binários.**

#### Estrutura: `FPlayerStateEntry`
```cpp
struct FPlayerStateEntry
{
    int32 PlayerId;
    FVector StateA_Location;
    float StateA_Yaw;
    int32 StateA_TimestampMs;
    FVector StateB_Location;
    float StateB_Yaw;
    int32 StateB_TimestampMs;
    bool HasStateA;
    bool HasStateB;
}
```

#### Funções Disponíveis:

**Construção de Frames:**
- **`BuildMoveUpdateFrame(PlayerId, Location, YawDegrees, TimestampMs)`** → `TArray<uint8>`
  - Constrói um frame binário `MoveUpdate` (cliente → servidor)
  - Tipo do frame: `1` (MoveUpdate)
  - Retorna array de bytes pronto para enviar via `SendBytes`
  - **Tamanho**: 25 bytes (1 + 4 + 4*4 + 4 + 4)

**Decodificação de Frames:**
- **`ParseStateUpdateFrame(Data, OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs)`** → `bool`
  - Decodifica um frame binário `StateUpdate` (servidor → cliente)
  - Tipo esperado: `2` (StateUpdate)
  - Retorna `true` se o frame foi decodificado com sucesso
  - Preenche os parâmetros `Out*` com os valores decodificados

**Gerenciamento de Estados:**
- **`FindPlayerStateIndex(StatesArray, PlayerId)`** → `int32`
  - Busca o índice de um `PlayerStateEntry` no array pelo `PlayerId`
  - Retorna `-1` se não encontrar
  
- **`GetOrCreatePlayerState(StatesArray, PlayerId)`** → `FPlayerStateEntry`
  - Obtém um `PlayerStateEntry` existente ou cria um novo
  - **IMPORTANTE**: Retorna uma **cópia**, não uma referência
  - Após modificar, use `Set Element` no array para salvar de volta
  
- **`UpdatePlayerStateBuffer(Entry, NewLocation, NewYaw, NewTimestampMs)`** → `void`
  - Atualiza o buffer de estados (StateA e StateB) para interpolação
  - Move StateB para StateA se necessário
  - **IMPORTANTE**: Modifica o `Entry` passado (usa `UPARAM(ref)`)

**Funções Auxiliares:**
- **`AppendUInt32LE(Bytes, Value)`** → `void`
  - Adiciona um `int32` ao array em formato Little-Endian
  
- **`AppendFloatLE(Bytes, Value)`** → `void`
  - Adiciona um `float` ao array em formato Little-Endian

### 3. `UUmbraGameInstance` (Atualizado)

**Função Adicionada:**
- **`GetZoneServerWebSocketURL()`** → `FString`
  - Retorna a URL do WebSocket ZoneServer
  - Formato: `ws://127.0.0.1:8082` (padrão, Zone ID 0)
  - Pode ser alterado no código para porta 8083 (Zone ID 1)

---

## 🚀 PROCEDIMENTO COMPLETO: Conexão Simultânea de Múltiplos Clientes

### FASE 1: Preparação do Servidor

#### 1.1. Compilar e Iniciar ZoneServer

**Windows PowerShell:**
```powershell
# Navegar até o diretório de build
cd D:\UmbraServerV2\UmbraServer\build\bin\Release

# Executar ZoneServer com Zone ID 0 (porta 8082) - RECOMENDADO
.\zone_server.exe 0

# OU com Zone ID 1 (porta 8083)
.\zone_server.exe 1
```

**Verificar Logs:**
```
[INFO] ZoneServer 'Zone_0' (ID: 0) started on port 8082
[INFO] WebSocketServer started on port 8082
```

**Verificar Porta Aberta:**
```powershell
netstat -an | findstr "8082"
# Deve mostrar: TCP    0.0.0.0:8082           0.0.0.0:0              LISTENING
```

---

### FASE 2: Criar Blueprint `BP_NetMovementClient`

#### 2.1. Criar a Classe Blueprint

1. **No Content Browser:**
   - Clique direito → `Blueprint Class`
   - Selecione `Actor` como classe pai
   - Nome: `BP_NetMovementClient`

#### 2.2. Adicionar Variáveis (Meu Blueprint → Variables)

**Variáveis Essenciais:**

1. **`WebSocketRef`**
   - Tipo: `Umbra WS Client` (Object Reference)
   - Default Value: `None`
   - Instance Editable: `false`

2. **`IsConnected`**
   - Tipo: `Boolean`
   - Default Value: `false`
   - Instance Editable: `false`

3. **`MyPlayerId`**
   - Tipo: `Integer`
   - Default Value: `0`
   - Instance Editable: `true` (para debug/override)

4. **`SendRateHz`**
   - Tipo: `Float`
   - Default Value: `20.0`
   - Instance Editable: `true`

5. **`InterpDelayMs`**
   - Tipo: `Float`
   - Default Value: `120.0`
   - Instance Editable: `true`

6. **`RemoteStates`**
   - Tipo: `Array of Player State Entry`
   - Default Value: `[]` (array vazio)
   - Instance Editable: `false`

7. **`RemoteActorIds`**
   - Tipo: `Array of Integer`
   - Default Value: `[]`
   - Instance Editable: `false`

8. **`RemoteActors`**
   - Tipo: `Array of Actor Reference`
   - Default Value: `[]`
   - Instance Editable: `false`

9. **`SendMoveUpdateHandle`** (Opcional)
   - Tipo: `Timer Handle`
   - Default Value: `None`
   - Para cancelar o timer quando desconectar

10. **`BP_RemotePlayerClass`** (Opcional, mas recomendado)
    - Tipo: `Actor Class Reference`
    - Default Value: Classe do personagem remoto (ex: `BP_RemotePlayer`)
    - Instance Editable: `true`

---

### FASE 3: Implementar `BeginPlay` no `BP_NetMovementClient`

#### 3.1. Fluxo Completo do BeginPlay

**Passo 1: Delay Inicial**
```
Event BeginPlay
  ↓
Delay (0.1)
  ↓
[Permite inicialização completa antes de conectar]
```

**Passo 2: Obter Game Instance e Verificar Personagem Ativo**
```
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
Branch: HasActiveCharacter?
    ↓
    ├─ True:
    │   ↓
    │   Get ActivePlayerID
    │   ↓
    │   Set MyPlayerId (ReturnValue)
    │   ↓
    │   [Continuar para Passo 3]
    │
    └─ False:
        ↓
        Print String: "⚠️ Nenhum personagem selecionado. Aguardando..."
        ↓
        Delay (1.0)
        ↓
        [Retornar ao início do BeginPlay - Loop até HasActiveCharacter]
```

**Passo 3: Obter URL do WebSocket**
```
Get ZoneServerWebSocketURL (do GameInstance)
  ↓
Print String: "🔌 Conectando ao: [URL]"
  ↓
[Continuar para Passo 4]
```

**Passo 4: Criar WebSocket usando Classe C++**
```
Create Umbra Web Socket (URL: ReturnValue do Passo 3)
  ↓
IsValid (ReturnValue)
  ↓
Branch:
    ├─ True:
    │   ↓
    │   Set WebSocketRef (ReturnValue)
    │   ↓
    │   [Continuar para Passo 5]
    │
    └─ False:
        ↓
        Print String: "❌ Falha ao criar WebSocket"
        ↓
        Return
```

**Passo 5: Conectar Delegates (Bind Events)**

**5.1. Bind OnConnected:**
```
Get WebSocketRef
  ↓
Bind Event to OnConnected
    ↓
    Target: WebSocketRef
    Delegate: OnConnected
  ↓
Create Custom Event: OnWSConnected
  ↓
[Implementar OnWSConnected - ver Passo 6]
```

**5.2. Bind OnConnectionError:**
```
Get WebSocketRef
  ↓
Bind Event to OnConnectionError
    ↓
    Target: WebSocketRef
    Delegate: OnConnectionError
  ↓
Create Custom Event: OnWSError
  ↓
[Implementar OnWSError]
```

**5.3. Bind OnClosed:**
```
Get WebSocketRef
  ↓
Bind Event to OnClosed
    ↓
    Target: WebSocketRef
    Delegate: OnClosed
  ↓
Create Custom Event: OnWSClosed
  ↓
[Implementar OnWSClosed]
```

**5.4. Bind OnRawMessage (CRÍTICO para movimento):**
```
Get WebSocketRef
  ↓
Bind Event to OnRawMessage
    ↓
    Target: WebSocketRef
    Delegate: OnRawMessage
  ↓
Create Custom Event: OnWSBinaryMessage (parâmetro: Data - TArray<uint8>)
  ↓
[Implementar OnWSBinaryMessage - ver Passo 7]
```

**Passo 6: Conectar ao Servidor**
```
Get WebSocketRef
  ↓
Connect (ReturnValue)
  ↓
Print String: "🔌 Tentando conectar..."
```

---

### FASE 4: Implementar `OnWSConnected` (Custom Event)

#### 4.1. Fluxo Completo

```
OnWSConnected
  ↓
Set IsConnected (true)
  ↓
Print String: "✅ WebSocket Connected!"
  ↓
Get SendRateHz
  ↓
Divide (1.0 / SendRateHz) → IntervalSeconds
    A: 1.0
    B: SendRateHz
  ↓
Set Timer by Function Name
    Function Name: "SendMoveUpdate"
    Time: IntervalSeconds (ex: 0.05 para 20 Hz)
    Looping: true
  ↓
Set SendMoveUpdateHandle (ReturnValue)
  ↓
Print String: "📤 Timer iniciado para SendMoveUpdate"
```

**NOTA**: O Timer será executado a cada `1.0 / SendRateHz` segundos, chamando `SendMoveUpdate` periodicamente.

---

### FASE 5: Implementar `SendMoveUpdate` (Function)

#### 5.1. Verificar Conexão
```
SendMoveUpdate (Function)
  ↓
Branch: IsConnected?
    ↓
    ├─ True:
    │   ↓
    │   [Continuar para Passo 5.2]
    │
    └─ False:
        ↓
        Print String: "⚠️ WebSocket não conectado"
        ↓
        Return
```

#### 5.2. Obter Posição Atual do Player
```
Get First Player Controller
  ↓
IsValid (ReturnValue)
  ↓
Branch:
    ├─ True:
    │   ↓
    │   Get Pawn (ReturnValue)
    │   ↓
    │   IsValid (Pawn)
    │   ↓
    │   Branch:
    │       ├─ True:
    │       │   ↓
    │       │   Get Actor Location (Pawn) → CurrentLocation
    │       │   ↓
    │       │   Get Actor Rotation (Pawn) → CurrentRotation
    │       │   ↓
    │       │   Break Rotator (CurrentRotation) → OutYaw (campo Yaw)
    │       │   ↓
    │       │   [Continuar para Passo 5.3]
    │       │
    │       └─ False:
    │           ↓
    │           Print String: "⚠️ Pawn não encontrado"
    │           ↓
    │           Return
    │
    └─ False:
        ↓
        Print String: "⚠️ PlayerController não encontrado"
        ↓
        Return
```

#### 5.3. Calcular Timestamp
```
Get Game Time in Seconds
  ↓
Multiply (ReturnValue * 1000.0)
    A: ReturnValue
    B: 1000.0
  ↓
Convert Int to Int64 (opcional, dependendo da versão do UE)
  ↓
Convert Int64 to Int (para usar em BuildMoveUpdateFrame)
  ↓
Set CurrentTimestampMs (ReturnValue)
```

**OU mais simples:**
```
Get Game Time in Milliseconds (se disponível)
  ↓
Set CurrentTimestampMs (ReturnValue)
```

#### 5.4. Construir Frame Binário usando Classe C++
```
BuildMoveUpdateFrame
    PlayerId: MyPlayerId
    Location: CurrentLocation
    YawDegrees: OutYaw (do Break Rotator)
    TimestampMs: CurrentTimestampMs
  ↓
ReturnValue: FrameBytes (TArray<uint8>)
  ↓
[Continuar para Passo 5.5]
```

#### 5.5. Enviar Frame via WebSocket
```
Get WebSocketRef
  ↓
IsValid (ReturnValue)
  ↓
Branch:
    ├─ True:
    │   ↓
    │   SendBytes (ReturnValue, FrameBytes)
    │   ↓
    │   Branch: ReturnValue (bool)?
    │       ├─ True:
    │       │   ↓
    │       │   [Print String opcional: "📤 MoveUpdate enviado"]
    │       │
    │       └─ False:
    │           ↓
    │           Print String: "❌ Falha ao enviar MoveUpdate"
    │
    └─ False:
        ↓
        Print String: "❌ WebSocketRef inválido"
```

---

### FASE 6: Implementar `OnWSBinaryMessage` (Custom Event)

**Parâmetro**: `Data` (TArray<uint8>)

#### 6.1. Decodificar Frame usando Classe C++
```
OnWSBinaryMessage (Data: TArray<uint8>)
  ↓
ParseStateUpdateFrame
    Data: Data
    OutPlayerId: (Criar variável local ou usar Set)
    OutLocation: (Criar variável local)
    OutYawDegrees: (Criar variável local)
    OutTimestampMs: (Criar variável local)
  ↓
Branch: ReturnValue (bool)?
    ↓
    ├─ True:
    │   ↓
    │   Print String: "📦 Recebido StateUpdate de Player: [OutPlayerId]"
    │   ↓
    │   [Continuar para Passo 6.2]
    │
    └─ False:
        ↓
        Print String: "⚠️ Frame inválido ou tipo incorreto"
        ↓
        Return
```

#### 6.2. Obter ou Criar Entry no Array `RemoteStates`
```
GetOrCreatePlayerState
    StatesArray: RemoteStates
    PlayerId: OutPlayerId
  ↓
ReturnValue: PlayerStateEntry (cópia)
  ↓
[Continuar para Passo 6.3]
```

#### 6.3. Atualizar o Buffer de Estados
```
UpdatePlayerStateBuffer
    Entry: PlayerStateEntry (do Passo 6.2)
    NewLocation: OutLocation
    NewYaw: OutYawDegrees
    NewTimestampMs: OutTimestampMs
  ↓
[Entry foi modificado via UPARAM(ref)]
  ↓
[Continuar para Passo 6.4]
```

**NOTA**: `UpdatePlayerStateBuffer` modifica o `Entry` diretamente, mas como recebemos uma **cópia** de `GetOrCreatePlayerState`, precisamos salvar de volta no array.

#### 6.4. Salvar Entry Modificado de Volta no Array
```
FindPlayerStateIndex
    StatesArray: RemoteStates
    PlayerId: OutPlayerId
  ↓
ReturnValue: FoundIndex (int32)
  ↓
Branch: FoundIndex >= 0?
    ↓
    ├─ True:
    │   ↓
    │   Set Element (RemoteStates, FoundIndex, PlayerStateEntry modificado)
    │   ↓
    │   [Continuar para Passo 6.5]
    │
    └─ False:
        ↓
        Add Element (RemoteStates, PlayerStateEntry modificado)
        ↓
        [Continuar para Passo 6.5]
```

#### 6.5. Criar/Obter Actor Remoto (Opcional - para visualização)

**6.5A. Verificar se Actor já existe:**
```
Find Item in Array
    Array: RemoteActorIds
    Item to Find: OutPlayerId
  ↓
ReturnValue: FoundIndex
  ↓
Branch: FoundIndex >= 0?
    ↓
    ├─ True:
    │   ↓
    │   [Actor já existe, pular para Passo 6.6]
    │
    └─ False:
        ↓
        [Continuar para Passo 6.5B - Criar novo Actor]
```

**6.5B. Criar novo Actor:**
```
Make Rotator
    Roll: 0.0
    Pitch: 0.0
    Yaw: OutYawDegrees
  ↓
ReturnValue: NewRotation
  ↓
Make Transform
    Location: OutLocation
    Rotation: NewRotation
    Scale: (1, 1, 1)
  ↓
ReturnValue: NewTransform
  ↓
Spawn Actor from Class
    Class: BP_RemotePlayerClass (ou classe padrão)
    Transform: NewTransform
  ↓
ReturnValue: NewActor
  ↓
IsValid (NewActor)
  ↓
Branch:
    ├─ True:
    │   ↓
    │   Add Item to Array (RemoteActorIds, OutPlayerId)
    │   ↓
    │   Add Item to Array (RemoteActors, NewActor)
    │   ↓
    │   Print String: "✅ Actor remoto criado: Player [OutPlayerId]"
    │
    └─ False:
        ↓
        Print String: "❌ Falha ao criar Actor remoto"
```

#### 6.6. Fim do `OnWSBinaryMessage`
```
[Interpolação será feita no Event Tick - ver Fase 7]
```

---

### FASE 7: Implementar `Event Tick` (Interpolação)

#### 7.1. Iterar sobre `RemoteStates`
```
Event Tick (DeltaSeconds: float)
  ↓
For Each Loop (RemoteStates)
    Array: RemoteStates
    Array Element: CurrentEntry (Player State Entry)
  ↓
[Continuar para Passo 7.2]
```

#### 7.2. Break Struct para Extrair Campos
```
Break Player State Entry (CurrentEntry)
  ↓
[Extrai todos os campos: PlayerId, StateA_Location, StateA_Yaw, etc.]
  ↓
[Continuar para Passo 7.3]
```

#### 7.3. Verificar se Tem Ambos os Estados
```
Boolean AND
    A: HasStateA
    B: HasStateB
  ↓
ReturnValue: CanInterpolate (bool)
  ↓
Branch: CanInterpolate?
    ↓
    ├─ True:
    │   ↓
    │   [Continuar para Passo 7.4 - Interpolar]
    │
    └─ False:
        ↓
        [Pular para próximo elemento do loop]
```

#### 7.4. Calcular Alpha para Interpolação
```
Get Game Time in Milliseconds
  ↓
Set CurrentTimeMs (ReturnValue)
  ↓
Subtract (CurrentTimeMs - StateA_TimestampMs)
    A: CurrentTimeMs
    B: StateA_TimestampMs
  ↓
Set ElapsedMs (ReturnValue)
  ↓
Subtract (StateB_TimestampMs - StateA_TimestampMs)
    A: StateB_TimestampMs
    B: StateA_TimestampMs
  ↓
Set DeltaMs (ReturnValue)
  ↓
Branch: DeltaMs > 0?
    ↓
    ├─ True:
    │   ↓
    │   Divide (ElapsedMs / DeltaMs)
    │       A: ElapsedMs
    │       B: DeltaMs
    │   ↓
    │   Set Alpha (ReturnValue)
    │   ↓
    │   Clamp (Alpha, 0.0, 1.0)
    │       Value: Alpha
    │       Min: 0.0
    │       Max: 1.0
    │   ↓
    │   Set ClampedAlpha (ReturnValue)
    │   ↓
    │   [Continuar para Passo 7.5]
    │
    └─ False:
        ↓
        [Pular para próximo elemento]
```

#### 7.5. Interpolar Location e Yaw
```
VLerp (Vector Lerp)
    A: StateA_Location
    B: StateB_Location
    Alpha: ClampedAlpha
  ↓
ReturnValue: InterpolatedLocation
  ↓
Lerp (Float Lerp)
    A: StateA_Yaw
    B: StateB_Yaw
    Alpha: ClampedAlpha
  ↓
ReturnValue: InterpolatedYaw
  ↓
[Continuar para Passo 7.6]
```

#### 7.6. Buscar Actor Remoto nos Arrays
```
Find Item in Array
    Array: RemoteActorIds
    Item to Find: PlayerId (do Break Struct)
  ↓
ReturnValue: FoundIndex
  ↓
Branch: FoundIndex >= 0?
    ↓
    ├─ True:
    │   ↓
    │   Get a Copy (RemoteActors, FoundIndex)
    │   ↓
    │   ReturnValue: RemoteActorRef
    │   ↓
    │   IsValid (RemoteActorRef)
    │   ↓
    │   Branch:
    │       ├─ True:
    │       │   ↓
    │       │   [Continuar para Passo 7.7]
    │       │
    │       └─ False:
    │           ↓
    │           [Pular para próximo elemento]
    │
    └─ False:
        ↓
        [Actor não existe ainda, pular para próximo elemento]
```

#### 7.7. Aplicar Transform no Actor Remoto
```
Make Rotator
    Roll: 0.0
    Pitch: 0.0
    Yaw: InterpolatedYaw
  ↓
ReturnValue: NewRotation
  ↓
Set Actor Location
    Actor: RemoteActorRef
    New Location: InterpolatedLocation
  ↓
Set Actor Rotation
    Actor: RemoteActorRef
    New Rotation: NewRotation
```

#### 7.8. Fim do Loop
```
[Loop continua para próximo elemento]
```

---

### FASE 8: Implementar `OnWSClosed` e Cleanup

#### 8.1. OnWSClosed
```
OnWSClosed
  ↓
Set IsConnected (false)
  ↓
Clear Timer by Handle (SendMoveUpdateHandle)
  ↓
Print String: "🔌 WebSocket desconectado"
  ↓
[Opcional: Limpar RemoteStates e RemoteActors]
```

#### 8.2. OnWSError
```
OnWSError (Error: FString)
  ↓
Print String: "❌ Erro WebSocket: [Error]"
  ↓
Set IsConnected (false)
```

---

### FASE 9: Spawnar `BP_NetMovementClient` no Nível

#### 9.1. No Level Blueprint de `Lvl_TestAuth`

**Abra `Lvl_TestAuth` → Window → Level Blueprint**

**Adicionar no `BeginPlay`:**
```
Event BeginPlay
  ↓
Delay (0.5) [aguardar inicialização]
  ↓
Get First Player Controller
  ↓
IsValid (ReturnValue)
  ↓
Branch:
    ├─ True:
    │   ↓
    │   Set bShowMouseCursor (ReturnValue, false)
    │   ↓
    │   Set Input Mode Game Only (ReturnValue)
    │   ↓
    │   Get Game Instance → Cast to Umbra Game Instance
    │   ↓
    │   Branch: HasActiveCharacter?
    │       ↓
    │       ├─ True:
    │       │   ↓
    │       │   Get All Actors of Class (BP_NetMovementClient)
    │       │   ↓
    │       │   Array Length
    │       │   ↓
    │       │   Branch: Length == 0?
    │       │       ↓
    │       │       ├─ True:
    │       │       │   ↓
    │       │       │   Get ActivePlayerID
    │       │       │   ↓
    │       │       │   Get Pawn (PlayerController)
    │       │       │   ↓
    │       │       │   Get Actor Location (Pawn) → SpawnLocation
    │       │       │   ↓
    │       │       │   Make Transform (SpawnLocation, Rotation: 0, Scale: 1)
    │       │       │   ↓
    │       │       │   Spawn Actor from Class
    │       │       │       Class: BP_NetMovementClient
    │       │       │       Transform: [do Make Transform]
    │       │       │   ↓
    │       │       │   Set MyPlayerId (Spawned Actor, ActivePlayerID)
    │       │       │   ↓
    │       │       │   Print String: "✅ BP_NetMovementClient criado"
    │       │       │
    │       │       └─ False:
    │       │           ↓
    │       │           Print String: "⚠️ BP_NetMovementClient já existe"
    │       │
    │       └─ False:
    │           ↓
    │           Print String: "⚠️ Nenhum personagem selecionado"
    │
    └─ False:
        ↓
        Print String: "⚠️ PlayerController não encontrado"
        ↓
        Delay (0.2) → [Retry]
```

---

### FASE 10: Testar Conexão Simultânea de Múltiplos Clientes

#### 10.1. Configurar Play In Editor (PIE) com 2 Clientes

**No Unreal Editor:**
1. **Edit → Editor Preferences → Play**
2. Configure:
   - **Number of Players**: `2`
   - **Run Dedicated Server**: `false`
   - **Net Mode**: `Play As Listen Server`

#### 10.2. Configurar Player IDs Diferentes

**Método 1: Usar Player Index**
No `BeginPlay` do `BP_NetMovementClient`, após obter `ActivePlayerID`, você pode usar:
```
Get Player Controller
  ↓
Get Local Player (ReturnValue)
  ↓
Get Player Index (ReturnValue)
  ↓
Add (ActivePlayerID + PlayerIndex)
  ↓
Set MyPlayerId (ReturnValue)
```

**Método 2: Manual (para teste rápido)**
- **Player 0**: Configure `MyPlayerId = 1` no Editor
- **Player 1**: Configure `MyPlayerId = 2` no Editor

#### 10.3. Executar Teste

1. **Inicie o ZoneServer:**
   ```powershell
   .\zone_server.exe 0
   ```

2. **Abra o Editor e pressione Play:**
   - Faça login (ambos os clientes)
   - Selecione personagens diferentes (ou configure Player IDs diferentes)
   - O nível deve carregar
   - `BP_NetMovementClient` deve ser criado em ambos os clientes

3. **Verificar Logs:**

   **Servidor:**
   ```
   [INFO] WS client 1 connected
   [INFO] WS client 2 connected
   ```

   **Cliente 1:**
   ```
   ✅ WebSocket Connected!
   📤 Timer iniciado para SendMoveUpdate
   📤 MoveUpdate enviado
   📦 Recebido StateUpdate de Player: 2
   ```

   **Cliente 2:**
   ```
   ✅ WebSocket Connected!
   📤 Timer iniciado para SendMoveUpdate
   📤 MoveUpdate enviado
   📦 Recebido StateUpdate de Player: 1
   ```

4. **Testar Movimento:**
   - Movimente o personagem no **Cliente 1**
   - O **Cliente 2** deve ver o personagem remoto se movendo
   - O movimento deve ser **suave** (interpolação funcionando)

---

## 📋 Checklist Completo de Implementação

### Preparação
- [ ] ZoneServer compilado e rodando
- [ ] Projeto UE5 compilado sem erros
- [ ] Plugin WebSockets habilitado
- [ ] Classes C++ (`UUmbraWSClient`, `UWSBinaryBPFL`) compiladas

### Blueprint BP_NetMovementClient
- [ ] Variáveis criadas conforme Fase 2
- [ ] `BeginPlay` implementado conforme Fase 3
- [ ] `OnWSConnected` implementado conforme Fase 4
- [ ] `SendMoveUpdate` implementado conforme Fase 5
- [ ] `OnWSBinaryMessage` implementado conforme Fase 6
- [ ] `Event Tick` implementado conforme Fase 7
- [ ] `OnWSClosed` e `OnWSError` implementados conforme Fase 8

### Level Blueprint
- [ ] `Lvl_TestAuth` Level Blueprint configurado conforme Fase 9
- [ ] Validações `IsValid` adicionadas
- [ ] `Get First Player Controller` usado (não `Get Player Controller`)

### Teste
- [ ] ZoneServer iniciado e escutando na porta correta
- [ ] Cliente 1 conecta com sucesso
- [ ] Cliente 2 conecta com sucesso
- [ ] Cliente 1 vê Cliente 2 se mover
- [ ] Cliente 2 vê Cliente 1 se mover
- [ ] Interpolação está suave (sem teleportes)

---

## 🔍 Debug e Troubleshooting

### "WebSocket não conecta"

**Verificar:**
1. URL está correta? (`ws://localhost:8082` ou `ws://localhost:8083`)
2. Servidor está rodando? (`netstat -an | findstr "8082"`)
3. `CreateUmbraWebSocket` retorna válido?
4. `Connect()` está sendo chamado?

**Logs esperados:**
```
🔌 Conectando ao: ws://localhost:8082
🔌 Tentando conectar...
✅ WebSocket Connected!
```

### "MoveUpdate não está sendo enviado"

**Verificar:**
1. `OnWSConnected` foi disparado?
2. Timer foi criado? (`SendMoveUpdateHandle` não é `None`)
3. `IsConnected` está `true`?
4. `Get First Player Controller` retorna válido?
5. `Pawn` existe e tem posição válida?

**Adicionar Prints:**
```
📤 Timer iniciado para SendMoveUpdate
📤 SendMoveUpdate chamado
📤 MoveUpdate enviado: Player [ID] em [Location]
```

### "StateUpdate não está sendo recebido"

**Verificar:**
1. `OnRawMessage` está bindado? (não `OnMessage`)
2. `ParseStateUpdateFrame` retorna `true`?
3. Frame tem tamanho mínimo? (25 bytes)
4. Tipo do frame é `2` (StateUpdate)?

**Adicionar Prints:**
```
📦 OnWSBinaryMessage recebido: [Size] bytes
📦 Frame decodificado: Player [ID] em [Location]
```

### "Interpolação não funciona"

**Verificar:**
1. `HasStateA` e `HasStateB` estão ambos `true`?
2. `DeltaMs` não é zero?
3. `Alpha` está entre 0.0 e 1.0?
4. `Set Element` está sendo usado após `UpdatePlayerStateBuffer`?

**Adicionar Prints:**
```
🔄 Interpolando Player [ID]: Alpha=[Alpha], DeltaMs=[DeltaMs]
```

---

## 🎯 Resumo das Classes C++ e Seu Uso

### `UUmbraWSClient`
- **Criar**: `CreateUmbraWebSocket(URL)` → Retorna `UUmbraWSClient*`
- **Conectar**: `Connect()` no objeto retornado
- **Enviar**: `SendBytes(Data)` com `TArray<uint8>`
- **Eventos**: Bind `OnRawMessage` para receber dados binários

### `UWSBinaryBPFL`
- **Construir Frame**: `BuildMoveUpdateFrame(PlayerId, Location, Yaw, Timestamp)` → `TArray<uint8>`
- **Decodificar Frame**: `ParseStateUpdateFrame(Data, ...)` → `bool`
- **Gerenciar Estados**: `GetOrCreatePlayerState`, `UpdatePlayerStateBuffer`

### `UUmbraGameInstance`
- **URL WebSocket**: `GetZoneServerWebSocketURL()` → `FString`
- **Player Ativo**: `HasActiveCharacter()`, `GetActivePlayerID()`

---

**Boa sorte com a implementação!** 🚀
