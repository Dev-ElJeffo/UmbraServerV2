# Guia Completo: Implementação e Teste do Sistema de Movimento WebSocket Binário

## 📋 Índice

1. [Visão Geral do Sistema](#visão-geral-do-sistema)
2. [Pré-requisitos](#pré-requisitos)
3. [Compilação do Servidor C++](#compilação-do-servidor-c)
4. [Iniciando o ZoneServer](#iniciando-o-zoneserver)
5. [Configuração do Cliente UE5](#configuração-do-cliente-ue5)
6. [Teste com Dois Clientes Simultâneos](#teste-com-dois-clientes-simultâneos)
7. [Verificação e Validação](#verificação-e-validação)
8. [Troubleshooting](#troubleshooting)
9. [Parâmetros de Configuração](#parâmetros-de-configuração)

---

## 1. Visão Geral do Sistema

O sistema de movimento WebSocket binário permite que múltiplos clientes UE5 se conectem simultaneamente a um servidor C++ e sincronizem seus movimentos em tempo real.

### Arquitetura

```
┌─────────────┐         WebSocket Binary        ┌──────────────┐
│  Cliente 1  │ ←──────────────────────────→ │              │
│    (UE5)    │                               │  ZoneServer  │
└─────────────┘                               │   (C++)      │
                                              │              │
┌─────────────┐         WebSocket Binary        │  Porta: 8082 │
│  Cliente 2  │ ←──────────────────────────→ │              │
│    (UE5)    │                               └──────────────┘
└─────────────┘
```

### Fluxo de Dados

1. **Cliente → Servidor (MoveUpdate)**:
   - Cliente envia posição atual (x, y, z) e yaw a **15-20 Hz**
   - Frame binário: `[type:1][playerId][x][y][z][yaw][ts_ms]`
   - Tamanho: **25 bytes** por frame

2. **Servidor → Clientes (StateUpdate)**:
   - Servidor valida movimento (anti-cheat)
   - Broadcast imediato para todos os clientes conectados
   - Snapshot periódico a **~10 Hz** (via `broadcastSnapshot()`)

3. **Cliente recebe StateUpdate**:
   - Decodifica frame binário
   - Atualiza buffer de interpolação (2 estados: A e B)
   - Interpola suavemente entre estados no `Event Tick`

---

## 2. Pré-requisitos

### Servidor C++

- ✅ **Windows 10/11** (ou Linux)
- ✅ **CMake 3.20+**
- ✅ **Compilador C++17** (Visual Studio 2019+ ou GCC 7+)
- ✅ **MySQL Server 8.0** (para autenticação - opcional para teste de movimento)
- ✅ **Git** (para submodules)

### Cliente UE5

- ✅ **Unreal Engine 5.0+**
- ✅ **Plugin WebSockets** habilitado
- ✅ **Blueprint `BP_NetMovementClient`** implementado conforme documentação
- ✅ **C++ Classes** compiladas (`UmbraWSClient`, `WSBinaryBPFL`)

---

## 3. Compilação do Servidor C++

### 3.1. Preparação do Ambiente

#### Windows (PowerShell ou CMD)

```powershell
# Navegue até o diretório do servidor
cd D:\UmbraServerV2\UmbraServer

# Verifique se os submodules estão atualizados
git submodule update --init --recursive
```

#### Linux (Bash)

```bash
cd /caminho/para/UmbraServer
git submodule update --init --recursive
```

### 3.2. Configuração do Build com CMake

#### Windows (Visual Studio)

```powershell
# Crie o diretório de build
mkdir build
cd build

# Configure o projeto (Visual Studio Generator)
cmake .. -G "Visual Studio 17 2022" -A x64

# Compile (ou abra o .sln no Visual Studio)
cmake --build . --config Release --target zone_server
```

**Alternativa (via Visual Studio):**
1. Abra o Visual Studio
2. File → Open → CMake → Selecione `CMakeLists.txt` na raiz do projeto
3. Build → Build All
4. Aguarde a compilação

#### Linux (GCC/Clang)

```bash
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compile (use -j para paralelizar)
make -j$(nproc) zone_server
```

### 3.3. Verificação da Compilação

Após a compilação, verifique se o executável foi criado:

#### Windows
```powershell
# Verifique se existe
Test-Path build\bin\Release\zone_server.exe
# ou
dir build\bin\Release\zone_server.exe
```

#### Linux
```bash
ls -lh build/bin/zone_server
```

**Saída esperada:**
```
zone_server.exe (ou zone_server) deve existir em build/bin/Release/ (Windows)
ou build/bin/ (Linux)
```

### 3.4. Dependências Opcionais

O `zone_server` pode funcionar **sem MySQL** para testes básicos de movimento, mas algumas funcionalidades (autenticação, persistência) podem não estar disponíveis.

---

## 4. Iniciando o ZoneServer

### 4.1. Método 1: Executável Direto (Recomendado para Teste)

#### Windows (PowerShell)

```powershell
# Navegue até o diretório de build
cd D:\UmbraServerV2\UmbraServer\build\bin\Release

# Execute o ZoneServer na porta padrão 8082 (Zone ID = 1)
.\zone_server.exe 1

# OU especifique um Zone ID diferente (porta = 8082 + ZoneID)
.\zone_server.exe 2  # Porta 8084
```

#### Linux

```bash
cd build/bin
./zone_server 1
```

**Log esperado:**
```
[INFO] Starting Zone Server...
[INFO] ZoneServer 'Zone_1' (ID: 1) started on port 8082
[INFO] WebSocketServer started on port 8082
```

### 4.2. Método 2: Script de Inicialização

#### Windows

Edite `scripts/start.bat` para iniciar apenas o ZoneServer:

```batch
@echo off
cd ..\build\bin\Release
start "Zone Server" zone_server.exe 1
pause
```

Ou execute diretamente:
```powershell
.\scripts\start.bat
```

### 4.3. Verificação do Servidor em Execução

#### Verificar Porta 8082

**Windows (PowerShell):**
```powershell
# Verificar se a porta está aberta
netstat -an | findstr "8082"

# Deve mostrar algo como:
# TCP    0.0.0.0:8082           0.0.0.0:0              LISTENING
```

**Linux:**
```bash
netstat -tuln | grep 8082
# ou
ss -tuln | grep 8082
```

#### Teste Rápido com Cliente WebSocket (Opcional)

Use uma ferramenta como **Postman** ou **WebSocket King** para testar a conexão:

1. Abra Postman
2. New → WebSocket Request
3. URL: `ws://localhost:8082`
4. Connect
5. Deve conectar com sucesso

**Ou via PowerShell (teste simples):**
```powershell
# Teste de conexão TCP (não WebSocket completo, mas verifica se o servidor está escutando)
Test-NetConnection -ComputerName localhost -Port 8082
```

---

## 5. Configuração do Cliente UE5

### 5.1. URL do WebSocket no Blueprint

No Blueprint `BP_NetMovementClient`, você deve configurar a URL do WebSocket no `BeginPlay`.

#### 5.1.1. Criar Variável de URL (Recomendado)

1. Abra `BP_NetMovementClient` no Unreal Editor
2. Na aba **Variables**, adicione uma nova variável:
   - **Nome**: `WebSocketURL`
   - **Tipo**: `String`
   - **Valor Padrão**: `ws://localhost:8082`
   - **Editable**: ✅ (para poder alterar no Editor)
   - **Instance Editable**: ✅ (para poder alterar por instância)

#### 5.1.2. Configurar no BeginPlay

No `BeginPlay` do `BP_NetMovementClient`, altere o nó `Create Umbra Web Socket`:

**Antes:**
```
Create Umbra Web Socket → URL: "ws://localhost:8082" (hardcoded)
```

**Depois (Recomendado):**
```
Get WebSocketURL → Create Umbra Web Socket → URL: (conectado)
```

**Passo a passo:**
1. No `BeginPlay`, após obter `WebSocketURL` (variável), conecte ao input `Url` do `Create Umbra Web Socket`
2. Isso permite alterar a URL sem recompilar o Blueprint

### 5.2. Player ID no Cliente

O `Player ID` usado no frame binário deve ser único para cada cliente.

#### 5.2.1. Opção 1: Usar Player ID do GameInstance (Recomendado)

Se você já tem o Player ID após o login/seleção de personagem, armazene-o no `GameInstance` e obtenha-o no `BP_NetMovementClient`:

1. No `BP_NetMovementClient`, crie uma variável:
   - **Nome**: `MyPlayerId`
   - **Tipo**: `Integer`
   - **Editable**: ✅

2. No `BeginPlay`, obtenha o Player ID do `GameInstance`:
   - Use `Get Game Instance` → Cast para `Umbra Game Instance`
   - Obtenha o `CurrentPlayerId` (ou como você armazena)
   - `Set MyPlayerId` com esse valor

3. No `SendMoveUpdate`, use `MyPlayerId` como `Player ID` no `BuildMoveUpdateFrame`

#### 5.2.2. Opção 2: Valor Temporário para Teste

Para teste rápido, use valores diferentes manualmente:

- **Cliente 1**: `MyPlayerId = 1`
- **Cliente 2**: `MyPlayerId = 2`

⚠️ **IMPORTANTE**: Cada cliente deve ter um `Player ID` único, caso contrário os updates podem conflitar.

### 5.3. Compilação do Cliente UE5

Antes de testar, certifique-se de que o projeto UE5 está compilado:

1. Abra o **Unreal Editor**
2. **Tools → Refresh Visual Studio Project Files** (se necessário)
3. **File → Refresh C++ Code** (se necessário)
4. Feche o Editor
5. Abra o **Visual Studio** (`.sln` do projeto)
6. **Build → Build Solution** (ou `Ctrl+Shift+B`)
7. Aguarde a compilação
8. Abra o Editor novamente

**Verificação:**
- Verifique se não há erros de compilação
- Verifique se os nós `Create Umbra Web Socket`, `BuildMoveUpdateFrame`, etc. aparecem no Blueprint

---

## 6. Teste com Dois Clientes Simultâneos

### 6.1. Preparação

1. ✅ **ZoneServer rodando** na porta 8082
2. ✅ **Projeto UE5 compilado** sem erros
3. ✅ **Blueprint `BP_NetMovementClient`** implementado e configurado

### 6.2. Método 1: Play In Editor (PIE) com Múltiplas Instâncias

Este é o método mais rápido para teste local.

#### 6.2.1. Configurar PIE com 2 Clientes

1. No **Unreal Editor**, vá em **Edit → Editor Preferences**
2. Procure por **"Play"** ou **"Level Editor Play Settings"**
3. Configure:
   - **Number of Players**: `2`
   - **Run Dedicated Server**: ❌ (desabilitado)
   - **Net Mode**: `Play As Listen Server` ou `Play As Client`

#### 6.2.2. Configurar Player IDs Diferentes

**Problema**: Em PIE, ambos os clientes podem ter o mesmo `Player ID` por padrão.

**Solução**: Use uma variável editável no Editor para definir IDs diferentes:

1. No `BP_NetMovementClient`, torne `MyPlayerId` **Instance Editable**
2. No Editor, antes de iniciar PIE:
   - Selecione a instância do `BP_NetMovementClient` no mundo
   - No **Details**, defina:
     - **Player 0**: `MyPlayerId = 1`
     - **Player 1**: `MyPlayerId = 2`

**OU** use um sistema mais dinâmico:

No `BeginPlay` do `BP_NetMovementClient`:
```
Get Player Controller → Get Local Player → Get Player Index → Set MyPlayerId
```

Isso garante que cada cliente tenha um ID único baseado no índice do player.

#### 6.2.3. Iniciar o Teste

1. Coloque o `BP_NetMovementClient` no nível (ou spawn dinamicamente)
2. Pressione **Play** (ou `Alt+P`)
3. Duas janelas de jogo devem abrir

**Verificação Visual:**
- Ambas as janelas devem mostrar o mundo
- Movimente um personagem na **Janela 1**
- O personagem deve aparecer (ou atualizar) na **Janela 2**
- O movimento deve ser suave (interpolação)

### 6.3. Método 2: Editor + Cliente Standalone

Este método é mais realista, simulando dois clientes distintos.

#### 6.3.1. Configurar Cliente Standalone

1. No **Unreal Editor**, vá em **File → Package Project → Windows → Windows (64-bit)**
2. Escolha um diretório de saída (ex: `D:\UmbraServerV2\Builds\Client`)
3. Aguarde o packaging concluir

#### 6.3.2. Executar Dois Clientes

1. **Cliente 1**: Execute o Editor em **PIE** (Player ID = 1)
2. **Cliente 2**: Execute o executável standalone (Player ID = 2)

**Configurar Player ID no Standalone:**
- Crie um arquivo de configuração ou use argumentos de linha de comando
- Ou use um sistema de auto-incremento baseado em timestamp/random

#### 6.3.3. Alternativa: Duas Instâncias do Editor

Se você tiver recursos suficientes:

1. Abra o **Editor** normalmente (instância 1)
2. Abra uma **segunda instância** do Editor (mesmo projeto)
3. Em cada instância, configure `MyPlayerId` diferente
4. Inicie **PIE** em ambas

⚠️ **Nota**: Executar dois Editores simultaneamente pode ser pesado. Use com cuidado.

### 6.4. Configuração de Rede no Editor

#### 6.4.1. Portas e Conflitos

Por padrão, o Unreal Editor usa portas específicas para PIE. Verifique se não há conflitos:

**Portas usadas pelo Editor:**
- PIE Player 0: Porta `7777` (UDP)
- PIE Player 1: Porta `7778` (UDP)
- WebSocket: Porta `8082` (TCP) - **usada pelo ZoneServer**

**Não deve haver conflito**, pois são protocolos diferentes (UDP vs TCP/WebSocket).

---

## 7. Verificação e Validação

### 7.1. Logs do Servidor

Monitore os logs do `zone_server` para verificar conexões e mensagens:

#### Windows (PowerShell)

```powershell
# Se o servidor estiver rodando em uma janela separada, monitore os logs
# Os logs são salvos em logs/zone_server.log
Get-Content logs\zone_server.log -Wait -Tail 20
```

**Logs esperados:**

```
[INFO] Starting Zone Server...
[INFO] ZoneServer 'Zone_1' (ID: 1) started on port 8082
[INFO] WebSocketServer started on port 8082
[INFO] WS client 1 connected
[INFO] WS client 2 connected
```

Quando um cliente enviar movimento:
```
[INFO] WS client 1 connected
[INFO] WS client 2 connected
# (logs de movimento são silenciosos por padrão, mas podem ser adicionados)
```

#### Verificar Conexões Ativas

Adicione logs temporários no `MovementServer` se necessário, ou use `netstat`:

```powershell
netstat -an | findstr "8082"
```

Deve mostrar conexões **ESTABLISHED** para cada cliente conectado.

### 7.2. Logs do Cliente UE5

No **Unreal Editor**, abra a aba **Output Log** (Window → Developer Tools → Output Log).

**Configure Log Verbosity:**
1. No Editor, vá em **Edit → Project Settings → Engine → Logging**
2. Aumente a verbosidade para `VeryVerbose` temporariamente

**Ou use Print String no Blueprint:**
Adicione `Print String` nodes para debug:

- No `OnWSConnected`: `"✅ WebSocket Connected!"`
- No `OnWSBinaryMessage`: `"📦 Received StateUpdate from Player: [PlayerId]"`
- No `SendMoveUpdate`: `"📤 Sending MoveUpdate: [Location]"`

### 7.3. Verificação Visual

#### 7.3.1. Player Remoto Aparece?

**Teste:**
1. Cliente 1 se move
2. Cliente 2 deve ver o personagem do Cliente 1 aparecer/spawnar
3. O personagem remoto deve estar na mesma posição (ou próxima) do Cliente 1

**Se não aparecer:**
- Verifique se `BP_RemotePlayer` está sendo spawnado no Branch False do `Event Tick`
- Verifique se `RemoteActorRef` está sendo setado corretamente
- Verifique se `Set Actor Location` está sendo executado

#### 7.3.2. Interpolação Suave?

**Teste:**
1. Cliente 1 se move em linha reta
2. Cliente 2 observa o personagem remoto
3. O movimento deve ser **suave**, sem teleportes ou jitter

**Se houver jitter:**
- Verifique se `HasStateA` e `HasStateB` estão ambos `true` antes de interpolar
- Verifique se `Alpha` está sendo calculado corretamente (0.0 a 1.0)
- Verifique se `ClampedAlpha` está sendo usado na interpolação

#### 7.3.3. Sincronização de Rotação (Yaw)?

**Teste:**
1. Cliente 1 rotaciona (yaw muda)
2. Cliente 2 observa
3. O personagem remoto deve rotacionar suavemente

**Verificação:**
- `Set Actor Rotation` deve usar `MakeRotator` com `InterpolatedYaw` no campo `Yaw`
- Roll e Pitch devem ser 0.0 (apenas Yaw importa para movimento horizontal)

### 7.4. Teste de Latência

#### 7.4.1. Teste Local (Latência Baixa)

Em ambiente local (localhost), a latência deve ser < 5ms. O movimento deve ser quase instantâneo.

#### 7.4.2. Teste Remoto (Latência Simulada)

Para simular latência real:

1. Use uma ferramenta como **Clumsy** (Windows) para adicionar delay artificial
2. Ou conecte dois computadores na mesma rede
3. Verifique se a interpolação compensa o delay (deve funcionar até ~200ms de latência)

### 7.5. Checklist de Validação Completo

#### Servidor
- [ ] ✅ ZoneServer inicia sem erros
- [ ] ✅ Porta 8082 está escutando
- [ ] ✅ Logs mostram "WS client X connected" quando cliente conecta
- [ ] ✅ Dois clientes podem conectar simultaneamente

#### Cliente UE5 - Conexão
- [ ] ✅ `BeginPlay` cria WebSocket corretamente
- [ ] ✅ `OnWSConnected` é disparado após conexão
- [ ] ✅ Timer de `SendMoveUpdate` inicia após conexão
- [ ] ✅ `OnWSBinaryMessage` recebe dados quando outro cliente se move

#### Cliente UE5 - Envio
- [ ] ✅ `SendMoveUpdate` é chamado periodicamente (15-20 Hz)
- [ ] ✅ Frame binário é construído corretamente (`BuildMoveUpdateFrame`)
- [ ] ✅ `SendBytes` envia o frame com sucesso
- [ ] ✅ `Player ID` é único por cliente

#### Cliente UE5 - Recebimento
- [ ] ✅ `OnWSBinaryMessage` decodifica o frame (`DecodeStateUpdateFrame`)
- [ ] ✅ `GetOrCreatePlayerState` cria/atualiza entry no `RemoteStates`
- [ ] ✅ `UpdatePlayerStateBuffer` atualiza estados A e B
- [ ] ✅ `Set Element` salva o estado modificado de volta no Array

#### Cliente UE5 - Interpolação
- [ ] ✅ `Event Tick` itera sobre `RemoteStates`
- [ ] ✅ Verifica `HasStateA AND HasStateB` antes de interpolar
- [ ] ✅ Calcula `Alpha` corretamente (ElapsedMs / DeltaMs)
- [ ] ✅ `ClampedAlpha` está entre 0.0 e 1.0
- [ ] ✅ `VLerp` interpola Location entre StateA e StateB
- [ ] ✅ `Lerp` interpola Yaw entre StateA e StateB
- [ ] ✅ `Set Actor Location` atualiza a posição do personagem remoto
- [ ] ✅ `Set Actor Rotation` atualiza a rotação do personagem remoto

#### Teste Multiplayer
- [ ] ✅ Cliente 1 vê Cliente 2 quando este se move
- [ ] ✅ Cliente 2 vê Cliente 1 quando este se move
- [ ] ✅ Movimento é suave (sem teleportes)
- [ ] ✅ Rotação (Yaw) é sincronizada
- [ ] ✅ Múltiplos clientes (>2) funcionam simultaneamente

---

## 8. Troubleshooting

### 8.1. Servidor não Inicia

#### Erro: "Port already in use"

**Causa**: Porta 8082 já está em uso por outro processo.

**Solução:**
```powershell
# Windows: Encontrar processo usando a porta
netstat -ano | findstr "8082"

# Linux:
lsof -i :8082

# Mate o processo ou use outra porta
# Edite main_zone.cpp para mudar a porta padrão
```

#### Erro: "Failed to bind socket"

**Causa**: Permissões insuficientes ou porta reservada.

**Solução:**
- Execute como Administrador (Windows)
- Ou use uma porta > 1024 (Linux não-root)

### 8.2. Cliente não Conecta ao Servidor

#### Erro: "Connection Refused"

**Verificação:**
1. Servidor está rodando? (verifique logs)
2. Porta correta? (`ws://localhost:8082`)
3. Firewall bloqueando? (Windows Firewall pode bloquear conexões)

**Solução Firewall Windows:**
```powershell
# Permitir conexões na porta 8082
New-NetFirewallRule -DisplayName "ZoneServer WS" -Direction Inbound -LocalPort 8082 -Protocol TCP -Action Allow
```

#### Erro: "WebSocket handshake failed"

**Causa**: Servidor WebSocket não está implementando o handshake corretamente.

**Verificação:**
- Verifique se `WebSocketServer::start()` retornou `true`
- Verifique logs do servidor para erros de handshake
- Teste com cliente WebSocket externo (Postman) para isolar o problema

### 8.3. Cliente Conecta mas não Recebe Dados

#### Servidor não está enviando snapshots?

**Verificação:**
1. Verifique se `ZoneServer::update()` está sendo chamado
2. Verifique se `snapshotAccumulator_ >= 0.1f` está sendo atingido
3. Adicione logs temporários em `broadcastSnapshot()`:

```cpp
// Em MovementServer.cpp (se tiver) ou MovementServer.hpp
void broadcastSnapshot() {
  std::lock_guard<std::mutex> lock(mu_);
  Core::Logger::getInstance().info("Broadcasting snapshot: {} players", players_.size());
  for (const auto& [pid, st] : players_) {
    // ... código existente
  }
}
```

#### Cliente não está processando mensagens?

**Verificação:**
1. Adicione `Print String` no `OnWSBinaryMessage` do Blueprint
2. Verifique se o delegate está conectado corretamente
3. Verifique se `OnRawMessage` está sendo disparado (não `OnMessage`)

**Correção:**
- Certifique-se de usar `OnRawMessage` (binário), não `OnMessage` (texto)
- No `UUmbraWSClient`, o delegate correto é `OnRawMessage`

### 8.4. Personagens Remotos não Aparecem

#### Branch False não está spawnando?

**Verificação:**
1. Verifique se `FoundIndex < 0` quando deveria spawnar
2. Verifique se `Spawn Actor from Class` está usando `BP_RemotePlayer`
3. Verifique se `BP_RemotePlayer` existe e é válido

**Debug:**
- Adicione `Print String` no Branch False: `"Spawning new remote player: [PlayerId]"`
- Verifique se o Actor é spawnado (use `IsValid` após spawn)

#### `RemoteActorRef` está sempre None?

**Verificação:**
1. Verifique se `Get Array Item` está retornando um Actor válido
2. Verifique se `FoundIndex >= 0` antes de usar `Get Array Item`
3. Verifique se `RemoteActors` array está sendo populado corretamente

**Correção:**
- Certifique-se de que `Array_Add` está adicionando o Actor spawnado ao `RemoteActors`
- Verifique se `RemoteActorIds` e `RemoteActors` estão sincronizados (mesmo índice)

### 8.5. Movimento não é Suave (Jitter/Teleporte)

#### Interpolação não está funcionando?

**Verificação:**
1. `HasStateA` e `HasStateB` estão ambos `true`?
2. `Alpha` está sendo calculado? (não é 0.0 ou NaN?)
3. `DeltaMs` não é zero? (evitar divisão por zero)

**Debug:**
- Adicione `Print String` mostrando `Alpha`, `DeltaMs`, `ElapsedMs`
- Verifique se `StateA_TimestampMs` < `StateB_TimestampMs` (ordem correta)

#### Valores de Location estão corretos?

**Verificação:**
- `StateA_Location` e `StateB_Location` são diferentes?
- `InterpolatedLocation` está entre StateA e StateB?
- `Set Actor Location` está sendo executado a cada frame?

### 8.6. Player ID Conflitante

#### Dois clientes com mesmo Player ID?

**Sintoma:**
- Apenas um personagem aparece para ambos os clientes
- Movimento de um cliente sobrescreve o do outro

**Solução:**
- Certifique-se de que cada cliente tem um `Player ID` único
- Use `Get Player Controller → Get Local Player → Get Player Index` para IDs únicos
- Ou defina manualmente em cada instância (PIE Player 0 = ID 1, Player 1 = ID 2)

### 8.7. Logs e Debug Avançado

#### Habilitar Logs Detalhados no Servidor

Edite `src/core/Logger.hpp` ou use configuração de log para aumentar verbosidade:

```cpp
// Em main_zone.cpp, antes de iniciar
Core::Logger::getInstance().setLevel(spdlog::level::debug);
```

#### Habilitar Logs WebSocket no Cliente

No Blueprint, adicione `Print String` em pontos críticos:
- `BeginPlay`: "Starting WebSocket connection..."
- `OnWSConnected`: "✅ Connected!"
- `OnWSBinaryMessage`: "📦 Received [Size] bytes"
- `SendMoveUpdate`: "📤 Sending update: Player [ID]"
- `Event Tick`: "🔄 Interpolating [Count] players"

---

## 9. Parâmetros de Configuração

### 9.1. Servidor (MovementServer.hpp)

```cpp
// Limites Anti-Cheat (valores padrão)
float maxSpeed_ = 1200.0f;           // Unidades/s (Unreal Units por segundo)
float maxTeleportDist_ = 3000.0f;    // Unidades (distância máxima em um update)
uint32_t maxDelayMs_ = 300;          // Milissegundos (atraso máximo aceito)

// Snapshot Rate (ZoneServer.cpp)
float snapshotInterval_ = 0.1f;      // Segundos (10 Hz)
```

**Ajustes Recomendados:**
- **maxSpeed**: 1200 uu/s é ~6.8 m/s (caminhada rápida). Ajuste conforme velocidade do personagem.
- **maxTeleportDist**: 3000 uu é ~17 metros. Ajuste conforme necessário.
- **maxDelayMs**: 300ms é razoável para latência. Aumente se houver problemas.

### 9.2. Cliente (Blueprint)

#### Variáveis Configuráveis

No `BP_NetMovementClient`, adicione variáveis editáveis:

1. **`SendRateHz`** (Float, Default: 20.0)
   - Frequência de envio de movimento (15-20 Hz recomendado)
   - Usado no cálculo do Timer: `1.0 / SendRateHz`

2. **`WebSocketURL`** (String, Default: "ws://localhost:8082")
   - URL do servidor WebSocket
   - Permite mudar sem recompilar

3. **`InterpDelayMs`** (Integer, Default: 100)
   - Delay de interpolação (100-150 ms recomendado)
   - Usado para calcular se deve interpolar entre StateA e StateB

4. **`MyPlayerId`** (Integer)
   - ID único do jogador local
   - Deve ser diferente para cada cliente

### 9.3. Cálculo de Timer (SendRateHz)

No `OnWSConnected`:

```
Get SendRateHz → Divide (1.0 / SendRateHz) → Set Timer by Function Name
```

**Valores Recomendados:**
- **15 Hz**: `1.0 / 15.0 = 0.0667` segundos
- **20 Hz**: `1.0 / 20.0 = 0.05` segundos
- **25 Hz**: `1.0 / 25.0 = 0.04` segundos (máximo recomendado)

### 9.4. Ajuste de Performance

#### Reduzir Frequência se Necessário

Se houver problemas de performance:

1. **Servidor**: Reduza frequência de snapshots (aumente `snapshotInterval_`)
2. **Cliente**: Reduza `SendRateHz` para 15 Hz ou menos
3. **Cliente**: Reduza número de players remotos sendo interpolados por frame

#### Otimizações Futuras

- **Culling**: Não interpolar players muito distantes
- **LOD**: Reduzir frequência de updates para players distantes
- **Priorização**: Priorizar updates de players próximos

---

## 10. Exemplo de Teste Completo (Passo a Passo)

### Cenário: Teste Local com PIE (2 Clientes)

1. **Inicie o Servidor:**
   ```powershell
   cd D:\UmbraServerV2\UmbraServer\build\bin\Release
   .\zone_server.exe 1
   ```
   ✅ **Verificar**: Log mostra "ZoneServer 'Zone_1' (ID: 1) started on port 8082"

2. **Abra o Unreal Editor:**
   - Abra o projeto `UmbraEternumUE`
   - Aguarde a compilação se necessário

3. **Configure o Blueprint:**
   - Abra `BP_NetMovementClient`
   - Verifique se `WebSocketURL` está configurado como `ws://localhost:8082`
   - Verifique se `MyPlayerId` é editável

4. **Coloque o Blueprint no Nível:**
   - Arraste `BP_NetMovementClient` para o nível
   - OU spawn dinamicamente no `BeginPlay` de outro Actor

5. **Configure PIE:**
   - Editor Preferences → Play → Number of Players: `2`
   - Net Mode: `Play As Listen Server`

6. **Configure Player IDs:**
   - No nível, selecione a instância de `BP_NetMovementClient`
   - No Details, para cada instância (se houver):
     - Player 0: `MyPlayerId = 1`
     - Player 1: `MyPlayerId = 2`
   - **OU** use lógica dinâmica baseada em Player Index

7. **Inicie o Teste:**
   - Pressione **Play** (`Alt+P`)
   - Duas janelas devem abrir

8. **Teste de Movimento:**
   - Na **Janela 1**, mova o personagem (WASD)
   - Observe a **Janela 2**: O personagem remoto deve aparecer e se mover
   - Na **Janela 2**, mova o personagem
   - Observe a **Janela 1**: O personagem remoto deve se mover

9. **Verificação de Logs:**
   - **Servidor**: Deve mostrar "WS client 1 connected" e "WS client 2 connected"
   - **Cliente**: Output Log deve mostrar mensagens de conexão e dados recebidos

10. **Teste de Desconexão:**
    - Feche uma janela
    - Servidor deve mostrar "WS client X disconnected"
    - Cliente remanescente não deve receber mais updates do cliente desconectado

---

## 11. Próximos Passos e Melhorias

### Melhorias Sugeridas

1. **Autenticação WebSocket:**
   - Adicionar handshake com token JWT
   - Validar Player ID no servidor (não confiar no cliente)

2. **Prediction e Reconciliation:**
   - Cliente prediz movimento local
   - Servidor corrige quando necessário

3. **Compression:**
   - Comprimir frames binários se necessário
   - Usar delta compression para updates

4. **Spatial Partitioning:**
   - Enviar updates apenas para clientes próximos
   - Reduzir overhead de rede

5. **Metrics e Monitoring:**
   - Adicionar métricas de latência
   - Monitorar taxa de pacotes perdidos
   - Dashboard de saúde do servidor

---

## 12. Referências Rápidas

### URLs e Portas

| Serviço | URL/Porta | Protocolo |
|---------|-----------|-----------|
| ZoneServer | `ws://localhost:8082` | WebSocket (Binary) |
| Auth Server | `http://localhost:8080` | HTTP/REST |
| Gateway Server | `tcp://localhost:9000` | TCP |
| MySQL | `localhost:3306` | MySQL |

### Comandos Úteis

#### Windows PowerShell

```powershell
# Verificar porta
netstat -an | findstr "8082"

# Testar conexão
Test-NetConnection -ComputerName localhost -Port 8082

# Ver processos
Get-Process | Where-Object {$_.ProcessName -like "*zone*"}

# Monitorar logs
Get-Content logs\zone_server.log -Wait -Tail 20
```

#### Linux Bash

```bash
# Verificar porta
netstat -tuln | grep 8082
ss -tuln | grep 8082

# Testar conexão
telnet localhost 8082

# Ver processos
ps aux | grep zone_server

# Monitorar logs
tail -f logs/zone_server.log
```

---

## 13. Conclusão

Este guia cobre toda a implementação e teste do sistema de movimento WebSocket binário. Com este guia, você deve ser capaz de:

1. ✅ Compilar e iniciar o servidor C++
2. ✅ Configurar o cliente UE5 para conectar
3. ✅ Testar com múltiplos clientes simultâneos
4. ✅ Verificar se tudo está funcionando corretamente
5. ✅ Resolver problemas comuns

**Boa sorte com os testes!** 🚀

---

**Última Atualização**: 30 de Outubro de 2025
**Versão do Documento**: 1.0.0

