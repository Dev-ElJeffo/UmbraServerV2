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

Pré-requisitos:
- Habilitar o plugin “WebSockets” (Editor → Plugins → Networking → WebSockets).
- Ter o `player_id` selecionado (após tela de seleção) e um Pawn local possuído pelo PlayerController.

### 3.1. Actor/Widget que gerencia a conexão
- Recomendo criar um `Actor` de sessão (ex.: `BP_NetMovementClient`) spawnado no level ao entrar no mundo (pós-seleção de personagem), ou gerenciado por `GameInstance`.

### 3.2. Variáveis necessárias (no `BP_NetMovementClient`)
- `WebSocketRef` (tipo `WebSocket` – objeto do plugin)
- `IsConnected` (bool)
- `LocalPlayerId` (int, `player_id` atual)
- `SendRateHz` (float, padrão 20)
- `InterpDelayMs` (float, padrão 120)
- `RemoteStates` (map `int → Estrutura` com últimos dois estados para interpolação)
- `SendTimerHandle` (TimerHandle)

### 3.3. Evento BeginPlay (ordem dos nós)
1) `Create WebSocket`
   - URL: `ws://127.0.0.1:8082/` (use a porta configurada do `ZoneServer`)
   - Salvar em `WebSocketRef`.
2) `Bind Event to OnConnected` do `WebSocketRef`
   - Evento: `OnWSConnected`
3) `Bind Event to OnConnectionError` do `WebSocketRef`
   - Evento: `OnWSError`
4) `Bind Event to OnClosed` do `WebSocketRef`
   - Evento: `OnWSClosed`
5) `Bind Event to OnMessage` (texto) – não usado aqui (binário), deixe vazio.
6) `Bind Event to OnRawMessage` (binário)
   - Evento: `OnWSBinaryMessage` (parâmetros: `Data` [Array de Bytes])
7) `Connect` no `WebSocketRef`.

### 3.4. OnWSConnected (ordem dos nós)
1) `Set IsConnected = true`
2) `Set Timer by Function Name` (Looping)
   - Function Name: `SendMoveUpdate`
   - Time: `1.0 / SendRateHz` (ex.: 0.05 p/ 20 Hz)
   - Store handle em `SendTimerHandle`

### 3.5. SendMoveUpdate (função chamada pelo Timer)
1) Obter Pawn local (ex.: `Get Player Pawn`)
2) `GetActorLocation` → `X,Y,Z`
3) `GetActorRotation` → `Yaw` (pode normalizar -180..180)
4) `NowMs` (construir ms – `Get Game Time in Seconds * 1000`, cast p/ int)
5) `Build Binary Frame` (Array de Bytes)
   - `Append Byte` → `type = 1`
   - `Append UInt32 LE` → `LocalPlayerId`
   - `Append Float LE` → `X`, `Y`, `Z`, `Yaw`
   - `Append UInt32 LE` → `NowMs`
6) `WebSocketRef.Send Bytes` (usar função do plugin para enviar Array de Bytes)

Observação: como o Blueprint não tem nativamente “Append UInt32 LE/Float LE”, crie funções auxiliares em Blueprint (ou em C++/BPFunctionLibrary) que convertam para Array de Bytes no endianness correto. Exemplo de sequência para `UInt32`:
- Crie `AppendUInt32LE(Bytes, Value)` que usa `RightShift` e `Bitmask` para extrair 4 bytes e faz `Array Add` na ordem (LSB→MSB). Para `Float`, use `Make Literal Float`→`To Bytes` via `Reinterpret` (recomendado fazer via C++ BPFunctionLibrary para precisão).

### 3.6. OnWSBinaryMessage (recepção de updates do servidor)
1) `Data[0]` → `Type` (Byte)
2) Se `Type != 2` (StateUpdate), `Return`.
3) Decodificar na ordem: `player_id (u32 LE)`, `x(f32)`, `y(f32)`, `z(f32)`, `yaw(f32)`, `ts_ms (u32 LE)`
4) Se `player_id == LocalPlayerId` → ignorar (é o próprio)
5) Atualizar buffer de estados em `RemoteStates[player_id]` guardando os dois últimos com timestamp.

### 3.7. Tick (no `BP_NetMovementClient` ou em um Subsystem)
Para cada `player_id` em `RemoteStates`:
1) Se houver ao menos 2 estados com `ts_ms` diferentes, calcule `alpha` em relação à janela de interpolação (`InterpDelayMs`).
2) `Lerp` entre `pos A` e `pos B`; `Lerp` (ou `RInterpTo`) para `yaw`.
3) Aplique `SetActorLocation`/`SetActorRotation` no pawn remoto correspondente (mantenha um mapa `player_id→Actor`).

### 3.8. OnWSClosed / OnWSError
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


