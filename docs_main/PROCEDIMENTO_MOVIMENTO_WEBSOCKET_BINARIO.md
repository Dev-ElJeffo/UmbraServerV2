## Procedimento Completo: Movimento em WebSocket Binário (10–20 Hz) com Snapshots, Interpolação e Anti-Cheat

Este guia explica, em detalhes, a implementação do sistema de movimento em tempo real usando WebSocket binário, sua estrutura no servidor, o protocolo de frames, validações anti-cheat, snapshots periódicos e, principalmente, como testar e integrar no cliente UE5 (C++ e Blueprints), incluindo a ordem dos nós, onde colocar e como ligar o fluxo.

### Visão Geral
- Transporte: WebSocket (RFC 6455), payloads binários.
- Frequência: envio cliente→servidor em 15–20 Hz; snapshots do servidor em ~10 Hz; broadcasts imediatos a cada update recebido e aceito.
- Protocolo: frames binários little-endian compactos para reduzir overhead; uma estrutura simples com posição e yaw.
- Anti-cheat básico no servidor: atraso máximo, limite de velocidade e rejeição de teleporte.
- Sincronização Cliente: interpolação lado cliente, com pequena janela (100–150 ms), evitando jitter.

---

## 1) Estrutura da Implementação no Servidor

- `src/network/WebSocketServer.hpp/.cpp`
  - Servidor WS simples, handshake, leitura/escrita de frames (texto e binário).
  - Adição: `broadcastBinary(const std::vector<uint8_t>&)` para difusão de frames binários.

- `src/zone/MovementProtocol.hpp`
  - Define o protocolo binário (encode/decode) para frames de movimento.
  - Layout LE dos frames:
    - `[type:uint8]` — 1 = `MoveUpdate` (cliente→servidor), 2 = `StateUpdate` (servidor→clientes)
    - `[player_id:uint32]`
    - `[x:float][y:float][z:float]`
    - `[yaw:float]` — yaw em graus (ou radianos, escolha consistente)
    - `[ts_ms:uint32]` — carimbo do cliente (ms)

- `src/zone/MovementServer.hpp`
  - Encapsula o `WebSocketServer` e gerencia estados dos jogadores.
  - Recebe `MoveUpdate`, valida anti-cheat e faz broadcast imediato `StateUpdate` em binário para todos.
  - Fornece `broadcastSnapshot()` para envio periódico do estado atual (cada jogador emite um `StateUpdate`).
  - Limites configuráveis: `maxSpeed` (uu/s), `maxTeleportDist` (uu), `maxDelayMs` (ms).

- `src/zone/ZoneServer.hpp/.cpp`
  - Integra o `MovementServer` na zona.
  - Inicia/paralisa junto do `ZoneServer`.
  - Em `update(deltaTime)`, acumula tempo e chama `broadcastSnapshot()` a ~10 Hz.

- `src/zone/CMakeLists.txt`
  - Inclui os novos headers (`MovementServer.hpp`, `MovementProtocol.hpp`).

---

## 2) Protocolo Binário em Detalhes

Estrutura LE (tamanho fixo, 1 + 4 + 4*4 + 4 = 25 bytes):
```
[type:u8][player_id:u32][x:f32][y:f32][z:f32][yaw:f32][ts_ms:u32]
```
- `type = 1 (MoveUpdate)`: enviado pelo cliente com posição atual e yaw.
- `type = 2 (StateUpdate)`: enviado pelo servidor para outros clientes (e em snapshots) com o estado mais recente do jogador correspondente.

Validações anti-cheat (servidor):
- `maxDelayMs` (padrão 300): descarta updates com timestamp muito atrasado.
- `maxTeleportDist` (padrão 3000 uu): descarta saltos grandes de posição.
- `maxSpeed` (padrão 1200 uu/s): descarta velocidades acima do limite.

Notas:
- `ts_ms` deve ser monotônico por jogador (use `FDateTime::UtcNow()` → ms, ou contagem local via `GetWorld()->GetTimeSeconds()*1000`).
- Para reduzir ainda mais banda, futuramente podemos quantizar `(x,y,z,yaw)` para `int16` e comprimir pacotes; a estrutura atual privilegia simplicidade.

---

## 3) Como Testar/Integrar no Cliente UE5 – Opção Blueprints (WebSocket Plugin)

**NOTA IMPORTANTE SOBRE ESTRUTURAS DE DADOS**:
- Este guia usa **Array** como estrutura principal para armazenar estados de jogadores remotos.
- **Não use "MapRange"** - isso é uma função matemática (mapeia valores de um intervalo para outro), não uma estrutura de dados Map/Dictionary.
- O tipo Map pode não estar disponível no Blueprint do UE5 para todos os tipos de dados customizados.
- A solução com **Array + estrutura `PlayerStateEntry` (C++)** é mais simples, mais robusta e tem funções helper prontas.

Pré-requisitos:
- Habilitar o plugin "WebSockets" (Editor → Plugins → Networking → WebSockets).
- Ter o `player_id` selecionado (após tela de seleção) e um Pawn local possuído pelo PlayerController.
- **Recompilar o projeto C++** para que a estrutura `PlayerStateEntry` apareça no Blueprint (Tools → Refresh Visual Studio Project Files → Build).

### 3.1. Plugin correto
- Habilite "Experimental WebSocket Networking Plugin" (o primeiro da lista na imagem). Não use apenas "Web Socket Messaging".

### 3.2. Actor/Widget que gerencia a conexão
- Recomendo criar um `Actor` de sessão (ex.: `BP_NetMovementClient`) spawnado no level ao entrar no mundo (pós-seleção de personagem), ou gerenciado por `GameInstance`.

### 3.3. Variáveis necessárias (no `BP_NetMovementClient`) - PASSO A PASSO DETALHADO

#### PASSO 1: Criar a Estrutura para Armazenar Estados Remotos

Antes de criar as variáveis, precisamos criar uma estrutura que armazena dois estados (para interpolação):

1. No Content Browser, clique com botão direito → `Blueprint` → `Structure` (não Blueprint Class!)
2. Nome: `PlayerStateBuffer`
3. Dentro da estrutura, adicione estas variáveis:
   - `StateA_Location` (tipo: `Vector`, padrão: 0,0,0)
   - `StateA_Yaw` (tipo: `Float`, padrão: 0.0)
   - `StateA_TimestampMs` (tipo: `Integer`, padrão: 0)
   - `StateB_Location` (tipo: `Vector`, padrão: 0,0,0)
   - `StateB_Yaw` (tipo: `Float`, padrão: 0.0)
   - `StateB_TimestampMs` (tipo: `Integer`, padrão: 0)
   - `HasStateA` (tipo: `Boolean`, padrão: false)
   - `HasStateB` (tipo: `Boolean`, padrão: false)

**Por que dois estados?** Precisamos de pelo menos dois estados com timestamps diferentes para fazer interpolação linear entre eles. Quando chegamos um novo estado:
- Se não tem StateA → salva em StateA
- Se tem StateA mas não StateB → salva em StateB
- Se tem ambos → move StateB para StateA e salva o novo em StateB

#### PASSO 2: Criar as Variáveis no Blueprint

Agora, no `BP_NetMovementClient`:

1. Abra `Meu Blueprint` → `Variáveis` → `+ Variável`
2. Crie as seguintes variáveis **nesta ordem**:

**a) `WebSocketRef`**
- Tipo: `Umbra WS Client` (Object Reference)
- Pode ser deixado como `None` inicialmente (será setado no BeginPlay)

**b) `IsConnected`**
- Tipo: `Boolean`
- Padrão: `false`
- Pode deixar `Instance Editable` desmarcado

**c) `LocalPlayerId`**
- Tipo: `Integer`
- Padrão: `0` (será setado após seleção de personagem)

**d) `SendRateHz`**
- Tipo: `Float`
- Padrão: `20.0`
- Instance Editable: `true` (para ajustar no editor se necessário)

**e) `InterpDelayMs`**
- Tipo: `Float`
- Padrão: `120.0`
- Instance Editable: `true`

**f) `RemoteStates` (SOLUÇÃO PRINCIPAL: Array com estrutura C++)**
- **IMPORTANTE**: Não use "MapRange" - isso é uma função matemática, não uma estrutura de dados.
- **Solução recomendada**: Usar **Array** + estrutura `PlayerStateEntry` (criada no código C++).
- **Por que Array e não Map?**
  - Map pode não estar disponível no dropdown do Blueprint para todos os tipos de dados.
  - Array é mais simples de usar no Blueprint e tem funções helper prontas.
  - Performance é adequada para até centenas de jogadores simultâneos.
- **Tipo**: **Array** → `Array of Player State Entry`
- **Como criar**:
  1. No dropdown "Tipo de Variável", selecione "Array"
  2. No campo "Inner" (ou "Element Type"), selecione `Player State Entry` 
     - **Nota**: Esta estrutura aparece automaticamente no Blueprint após recompilar o projeto C++
     - Se não aparecer, recompile o projeto: Editor → Tools → Refresh Visual Studio Project Files → Build
  3. Padrão: deixe vazio (Array vazio)
- **Funções Helper disponíveis** (categoria "Umbra|Net|WS|State"):
  - `FindPlayerStateIndex(StatesArray, PlayerId)` → retorna índice ou -1
  - `GetOrCreatePlayerState(StatesArray, PlayerId)` → busca ou cria automaticamente
  - `UpdatePlayerStateBuffer(Entry, Location, Yaw, TimestampMs)` → atualiza buffer com rotação de estados

**g) `RemoteActors` (SOLUÇÃO: Array paralelo - mais simples)**
- **Recomendação**: Use dois Arrays paralelos (mais simples que criar estrutura):
  - `RemoteActorIds` (Array of Integer) - armazena os player_ids na mesma ordem
  - `RemoteActors` (Array of Actor Reference) - armazena as referências dos actors na mesma ordem
- **Por que Arrays paralelos?**
  - Mais simples que criar uma estrutura custom no Content Browser
  - Fácil de usar com `Find Item in Array` para buscar por PlayerId
  - Manter sincronização: sempre adicionar/remover em ambos os Arrays ao mesmo tempo
- **Alternativa (se preferir estrutura)**:
  - Crie uma estrutura no Content Browser: `RemoteActorEntry`
  - Variáveis: `PlayerId` (Integer), `ActorRef` (Actor Reference)
  - Tipo da variável: Array of RemoteActorEntry

**h) `SendTimerHandle`**
- Tipo: `Timer Handle`
- Padrão: deixe vazio

### 3.4. Criar Custom Events ANTES do BeginPlay (CRÍTICO!)

**IMPORTANTE**: Você precisa criar os Custom Events manualmente ANTES de fazer os binds. Eles não aparecem automaticamente quando você conecta o `Bind Event`.

#### PASSO 1: Criar os 4 Custom Events

No `BP_NetMovementClient`, vá em `Meu Blueprint` → `GRÁFICOS` → clique no `+` ao lado de "GRÁFICOS" → selecione "Add Custom Event":

**a) `OnWSConnected`**
1. Nome: `OnWSConnected`
2. Clique com botão direito no evento → `Add Input` → **NÃO adicione nenhum input** (este evento não tem parâmetros)

**b) `OnWSError`**
1. Nome: `OnWSError`
2. Clique com botão direito no evento → `Add Input`
3. Nome do parâmetro: `Error`
4. Tipo: `String`

**c) `OnWSClosed`**
1. Nome: `OnWSClosed`
2. Sem parâmetros (como OnWSConnected)

**d) `OnWSBinaryMessage`**
1. Nome: `OnWSBinaryMessage`
2. Clique com botão direito no evento → `Add Input`
3. Nome do parâmetro: `Data`
4. Tipo: `Array of Bytes` (não "Byte Array", procure por "Array of Bytes" ou "Array" e depois selecione o tipo base "Byte")

**Como verificar se criou corretamente**: Você deve ver 4 eventos na lista de "GRÁFICOS" em `Meu Blueprint`.

### 3.5. Evento BeginPlay (ordem dos nós) – com o wrapper “Umbra WS Client”

Criamos um wrapper C++ para expor nós em BP:
- Tipo da variável: `Umbra WS Client` (Object Reference)
- Funções: `CreateUmbraWebSocket(Url)`, `Connect()`, `Close()`, `SendBytes(Data)`
- Eventos: `OnConnected`, `OnConnectionError(Error)`, `OnClosed`, `OnRawMessage(Data)`

**Passos no `Event BeginPlay` (ordem exata dos nós)**:

**1) Create WebSocket**
- No `Event BeginPlay`, arraste um nó: `Create Umbra Web Socket` (procure em "All Actions" ou digite "Create Umbra")
- Input `Url`: digite `ws://127.0.0.1:8083/` (ou a porta configurada no seu ZoneServer)
- Output `Return Value` → ligue em um nó `Set WebSocketRef`

**2) Bind Event to OnConnected**
- Arraste a variável `WebSocketRef` no gráfico (Get WebSocketRef)
- Do `WebSocketRef`, procure o nó `Bind Event to OnConnected` (categoria "Umbra|Net|WS")
- O nó `Bind Event to OnConnected` tem um pin de saída "Event"
- **LIGUE este pin "Event" no seu Custom Event `OnWSConnected`** (arraste até o nó `OnWSConnected` que você criou)

**3) Bind Event to OnConnectionError**
- Do `WebSocketRef`, procure `Bind Event to OnConnectionError`
- Output "Event" → ligue no Custom Event `OnWSError`
- O output "Error (String)" do `OnWSError` será preenchido automaticamente pelo delegate

**4) Bind Event to OnClosed**
- Do `WebSocketRef`, procure `Bind Event to OnClosed`
- Output "Event" → ligue no Custom Event `OnWSClosed`

**5) Bind Event to OnRawMessage**
- Do `WebSocketRef`, procure `Bind Event to OnRawMessage`
- Output "Event" → ligue no Custom Event `OnWSBinaryMessage`
- O output "Data (Array of Bytes)" será preenchido automaticamente

**6) Connect**
- Do `WebSocketRef`, procure `Connect` (função sem parâmetros)
- Ligue o execution pin do último bind (ou use um nó `Sequence` para ligar todos os binds e depois o Connect)

**Fluxo Visual Sugerido**:
```
Event BeginPlay
    ↓
Create Umbra Web Socket (Url: ws://...)
    ↓
Set WebSocketRef
    ↓
Sequence (com 4 saídas "Then")
    ↓ (Then 0)
Bind Event to OnConnected → [Event pin] → OnWSConnected (custom event)
    ↓ (Then 1)
Bind Event to OnConnectionError → [Event pin] → OnWSError (custom event)
    ↓ (Then 2)
Bind Event to OnClosed → [Event pin] → OnWSClosed (custom event)
    ↓ (Then 3)
Bind Event to OnRawMessage → [Event pin] → OnWSBinaryMessage (custom event)
    ↓ (execution após Sequence)
Connect (no WebSocketRef)
```

### 3.5. OnWSConnected (ordem dos nós)
1) `Set IsConnected = true`
2) `Set Timer by Function Name` (Looping)
   - Function Name: `SendMoveUpdate`
   - Time: `1.0 / SendRateHz` (ex.: 0.05 p/ 20 Hz)
   - Store handle em `SendTimerHandle`
   - Screenshot: [Timer looping configurado para 20 Hz]

### 3.6. SendMoveUpdate (função chamada pelo Timer)
1) Obter Pawn local (ex.: `Get Player Pawn`)
2) `GetActorLocation` → `X,Y,Z`
3) `GetActorRotation` → `Yaw` (pode normalizar -180..180)
4) `NowMs` (construir ms – `Get Game Time in Seconds * 1000`, cast p/ int)
5) `BuildMoveUpdateFrame(PlayerId, Location, Yaw, TimestampMs, OutBytes)`
   - Nó da nossa BPFunctionLibrary: `UWSBinaryBPFL` (categoria Umbra|Net|WS|Binary)
6) `WSClient.SendBytes(OutBytes)`
   - Screenshot: [Montagem do frame com BuildMoveUpdateFrame e envio via SendBytes]

Observação: agora já incluímos uma BPFunctionLibrary no projeto com nós prontos:
- `AppendUInt32LE(Bytes, Value)`
- `AppendFloatLE(Bytes, Value)`
- `BuildMoveUpdateFrame(PlayerId, Location, YawDegrees, TimestampMs, OutBytes)`
Use diretamente `BuildMoveUpdateFrame` e envie `OutBytes` via `WebSocket.Send Bytes`.

### 3.7. OnWSBinaryMessage (recepção de updates do servidor) - PASSO A PASSO DETALHADO

Esta função é chamada automaticamente quando o servidor envia dados binários (StateUpdate).

**Ordem dos nós no gráfico `OnWSBinaryMessage`**:

**1) Parse do Frame**
- Use o nó `ParseStateUpdateFrame` (procure em "All Actions" → categoria "Umbra|Net|WS|Binary")
- Input `Data`: conecte o parâmetro `Data` do evento `OnWSBinaryMessage`
- Outputs:
  - `OutPlayerId` (Integer)
  - `OutLocation` (Vector)
  - `OutYawDegrees` (Float)
  - `OutTimestampMs` (Integer)
  - `Return Value` (Boolean) - `true` se parse foi bem-sucedido

**2) Verificar se Parse foi bem-sucedido**
- Do `ParseStateUpdateFrame`, ligue o `Return Value` em um nó `Branch`
- Se `false`, faça `Return` (sai da função)

**3) Verificar se é StateUpdate (opcional, mas recomendado)**
- `Data` → `Get (Array)` com Index `0` (primeiro byte)
- Se o valor != `2` (StateUpdate), faça `Return`

**4) Ignorar se é o próprio jogador**
- `OutPlayerId` → comparar (`==`) com `LocalPlayerId` (variável)
- Se `==`, faça `Return` (não precisamos atualizar nosso próprio estado)

**5) Obter ou Criar Entry no Array `RemoteStates` (USANDO FUNÇÃO HELPER)**
- Use o nó `GetOrCreatePlayerState` (categoria "Umbra|Net|WS|State")
- Inputs:
  - `StatesArray`: `Get RemoteStates` (Array of Player State Entry)
  - `PlayerId`: `OutPlayerId`
- Output:
  - `Return Value`: `Player State Entry` (a estrutura retornada)
- **IMPORTANTE**: Esta função modifica o Array automaticamente se criar um novo, mas você precisa salvar de volta:
  - `Get RemoteStates` → `Set Element` (Index: use `FindPlayerStateIndex` se quiser, mas a função já adicionou se não existia)
- **MAIS SIMPLES**: Use diretamente `UpdatePlayerStateBuffer` no passo 6, que já faz tudo automaticamente

**6) Atualizar o Buffer (USANDO FUNÇÃO HELPER - MAIS FÁCIL)**
- Use o nó `UpdatePlayerStateBuffer` (categoria "Umbra|Net|WS|State")
- Inputs:
  - `Entry`: `Get RemoteStates` → `Get Element` usando o index retornado por `FindPlayerStateIndex(RemoteStates, OutPlayerId)`, OU simplesmente:
  - `Entry`: a estrutura retornada pelo `GetOrCreatePlayerState` do passo 5
  - `NewLocation`: `OutLocation`
  - `NewYaw`: `OutYawDegrees`
  - `NewTimestampMs`: `OutTimestampMs`
- Esta função já faz toda a lógica de rotação automaticamente (StateA → StateB → novo em B)
- **ATUALIZAR O ARRAY**: Após chamar `UpdatePlayerStateBuffer`, você precisa salvar de volta no Array:
  - `Get RemoteStates` → `Set Element`
  - Index: use `FindPlayerStateIndex(RemoteStates, OutPlayerId)` para encontrar o índice
  - Element: a estrutura atualizada (do output do `UpdatePlayerStateBuffer`)

**Alternativa Simplificada (recomendada)**:
- Combine os passos 5 e 6 em uma única operação:
  1. `GetOrCreatePlayerState(RemoteStates, OutPlayerId)` → `Entry`
  2. `UpdatePlayerStateBuffer(Entry, OutLocation, OutYawDegrees, OutTimestampMs)` → `Entry` (modificado)
  3. Encontre o index: `FindPlayerStateIndex(RemoteStates, OutPlayerId)` → `Index`
  4. `Set Element` no `RemoteStates` (Index: `Index`, Element: `Entry`)

**7) Criar/Obter Actor Remoto (opcional mas recomendado)**
- `Get RemoteActors` → `Find in Map` (Key: `OutPlayerId`)
- Se não encontrado:
  - Spawn um actor/pawn remoto (ex.: uma cópia do seu player pawn, mas sem input)
  - `Add to Map` no `RemoteActors` (Key: `OutPlayerId`, Value: o actor spawnado)
- Se encontrado, use o actor existente

**Nota**: A interpolação real será feita no `Tick`, não aqui. Aqui apenas atualizamos o buffer de estados.

### 3.8. Tick (no `BP_NetMovementClient`) - PASSO A PASSO DETALHADO

No evento `Event Tick`, adicione a lógica de interpolação para cada jogador remoto.

**Ordem dos nós no `Event Tick`**:

**1) Iterar sobre o Array `RemoteStates`**
- `Get RemoteStates` → `For Each Loop` (ou `For Each Loop (Break)`)
- O loop fornece:
  - `Array Element` (`PlayerStateEntry`) - a estrutura com o buffer de estados e o PlayerId dentro dela
- **Nota**: A estrutura `PlayerStateEntry` já contém o `PlayerId` como primeiro campo, então você pode acessar `ArrayElement.PlayerId`

**2) Verificar se tem dados suficientes para interpolação**
- Do `Array Element` (PlayerStateEntry), verifique `HasStateA` e `HasStateB`
- Se ambos são `true`, continue (temos dois estados para interpolar)
- Se não, continue para o próximo item do loop (`Continue Loop`)

**3) Calcular Alpha (fator de interpolação)**
- Obtenha o tempo atual em ms: `Get Game Time in Seconds` × `1000` → converta para Integer
- Delta entre estados: `ArrayElement.StateB_TimestampMs` - `ArrayElement.StateA_TimestampMs`
- Tempo decorrido desde StateA: `(Tempo Atual Ms) - (ArrayElement.StateA_TimestampMs)`
- Alpha: `(Tempo Decorrido) / (Delta entre Estados)`
- Clampe Alpha entre `0.0` e `1.0` (use `Clamp (Float)`)

**4) Interpolar Posição**
- `Lerp (Vector)`:
  - `A` = `ArrayElement.StateA_Location`
  - `B` = `ArrayElement.StateB_Location`
  - `Alpha` = o valor calculado acima
- Output: `InterpolatedLocation` (Vector)

**5) Interpolar Yaw**
- `Lerp (Float)`:
  - `A` = `ArrayElement.StateA_Yaw`
  - `B` = `ArrayElement.StateB_Yaw`
  - `Alpha` = o mesmo alpha
- Output: `InterpolatedYaw` (Float)
- **Nota**: Se os yaws estão em diferentes direções (ex.: -179° e 179°), você pode precisar de uma lógica especial para "encurtar o caminho" (shortest path). Por enquanto, use o Lerp simples.

**6) Obter o Actor Remoto (usando Array)**
- **Opção A**: Se você está usando Array paralelo (`RemoteActorIds` + `RemoteActors`):
  - `Find Item in Array` no `RemoteActorIds` (Item: `ArrayElement.PlayerId`) → `Index`
  - Se `Index >= 0`:
    - `Get Element` no `RemoteActors` (Index: `Index`) → `ActorRef`
  - Se `Index < 0`, continue para o próximo item (não spawnou o actor ainda)
- **Opção B**: Se você está usando Array de estruturas `RemoteActorEntry`:
  - `For Each Loop` no `RemoteActors` → procure um elemento onde `PlayerId == ArrayElement.PlayerId`
  - Se encontrado, use o `ActorRef` dessa estrutura

**7) Aplicar Transformação**
- Do Actor obtido, chame `SetActorLocation` (Target: o Actor, New Location: `InterpolatedLocation`)
- Crie uma Rotator:
  - `Make Rotator`:
    - `Roll` = `0`
    - `Pitch` = `0`
    - `Yaw` = `InterpolatedYaw`
- `SetActorRotation` (Target: o Actor, New Rotation: o Rotator criado)

**8) Continuar Loop**
- O loop continuará automaticamente para o próximo `player_id` no Map

**Fluxo Visual Simplificado**:
```
Event Tick
    ↓
Get RemoteStates
    ↓
For Each Loop (Key: player_id, Value: buffer)
    ↓
Branch: HasStateA AND HasStateB?
    ↓ (true)
Calcular Alpha (tempo)
    ↓
Lerp Location (StateA → StateB)
    ↓
Lerp Yaw (StateA → StateB)
    ↓
Find RemoteActors[player_id]
    ↓
SetActorLocation (InterpolatedLocation)
    ↓
SetActorRotation (InterpolatedYaw)
    ↓ (loop continua)
```

**Otimizações futuras**:
- Se `Alpha >= 1.0`, você pode fazer "snap" direto para `StateB` (o servidor está muito atrasado)
- Limpar entradas do Map para players que não enviaram updates por muito tempo (> 5 segundos)

### 3.9. OnWSClosed / OnWSError
1) `Clear Timer by Handle (SendTimerHandle)`
2) `Set IsConnected = false`
3) Opcional: `Retry` com `Delay` exponencial.

---

## 4) Como Testar/Integrar no Cliente UE5 – Opção C++

Fluxo equivalente em C++ (resumo):
1) Habilite o módulo `WebSockets` no `.uproject` e `Build.cs`.
2) Crie um componente/objeto (ex.: `UUmbraWebSocketClient`) que usa `IWebSocket`.
3) Ao conectar (pós-seleção de personagem), inicie um `FTimerHandle` que chama `SendMoveUpdate()` em 15–20 Hz.
4) Em `OnRawMessage`, decodifique frames `StateUpdate` e atualize buffers de interpolação.
5) Em Tick, interpole e aplique transform nos pawns remotos.

Pontos de atenção:
- Use LE ao serializar/deserializar (converter via `reinterpret_cast` + `htole32` se necessário).
- Mantenha um mapa de `player_id→AActor*` para instanciar/gerenciar avatares remotos.

---

## 5) Sequência Recomendada no Jogo (UE5)

1) Login e Seleção de Personagem concluídos (já implementados).
2) Ao confirmar personagem:
   - Criar/ativar `BP_NetMovementClient`.
   - Conectar ao `ws://<host>:<zone_port>/`.
3) Ao `OnWSConnected`:
   - Iniciar `SendMoveUpdate` em 20 Hz.
4) Jogador se move normalmente (input local). O cliente envia snapshots de pos/yaw a 20 Hz.
5) `OnWSBinaryMessage` recebe `StateUpdate` de outros players → atualiza buffers → interpolação no Tick.
6) Desconexão limpa on EndPlay / retorno para seleção.

---

## 6) Parâmetros e Ajustes

- Frequências:
  - Envio cliente→servidor: 15–20 Hz (ajuste `SendRateHz`).
  - Snapshot servidor: ~10 Hz (fixo no ZoneServer via acumulador de tempo).

- Anti-Cheat:
  - `maxSpeed = 1200 uu/s`
  - `maxTeleportDist = 3000 uu`
  - `maxDelayMs = 300`
  - Ajuste direto no `MovementServer` (expor via config se desejar).

- Interpolação:
  - `InterpDelayMs = 100–150 ms` proporciona suavidade.
  - Guarde pelo menos dois estados recentes por jogador.

---

## 7) Checklist de Teste

Servidor:
- Iniciar o `ZoneServer` (porta conforme `Config.port`). Ver logs “ZoneServer ... started” e “WS client connected” quando o cliente conectar.

Cliente (Blueprint):
- `BeginPlay` do `BP_NetMovementClient` cria e conecta `WebSocketRef`.
- `OnWSConnected` arma Timer de 20 Hz para `SendMoveUpdate`.
- Movimentando o jogador local, os peers recebem `StateUpdate` (verifique com 2 instâncias PIE ou cliente externo).
- Se desconectar, `OnWSClosed` limpa o Timer.

Validações:
- Tentar valores extremos (teleporte forçado) para ver se o servidor rejeita.
- Inspecionar logs: updates atrasados/rápidos demais devem ser rejeitados silenciosamente.

---

## 8) Futuro / Extensões

- Autenticação WS: associar `clientId ↔ playerId` usando JWT no handshake (query param/primeira mensagem).
- Quantização agressiva (int16) e empacotamento de múltiplos jogadores por frame.
- Canais lógicos (streams) e priorização de eventos críticos.
- Correções do servidor (mensagens `correction`) quando erro > tolerância (ex.: 150 uu), com “snap” suave no cliente.

---

## 9) Referências de Código

```1:80:src/zone/MovementProtocol.hpp
// Protocolo binário LE e helpers encode/decode
```

```1:120:src/zone/MovementServer.hpp
// Recebe MoveUpdate, valida e difunde StateUpdate; snapshots periódicos
```

```1:120:src/network/WebSocketServer.hpp
// Servidor WebSocket + broadcastBinary
```

```1:140:src/zone/ZoneServer.cpp
// Integra MovementServer e snapshots em update()
```


