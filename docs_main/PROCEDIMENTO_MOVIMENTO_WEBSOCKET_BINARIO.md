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

**g) `RemoteActorIds` e `RemoteActors` (SOLUÇÃO: Arrays Paralelos) - PASSO A PASSO DETALHADO**

Estes dois Arrays trabalham juntos para armazenar os Actors remotos (representações visuais de outros jogadores no seu mundo).

**O QUE SÃO ARRAYS PARALELOS?**
- Dois Arrays separados que mantêm os mesmos índices sincronizados
- `RemoteActorIds[0]` contém o PlayerId do primeiro jogador remoto
- `RemoteActors[0]` contém o Actor do primeiro jogador remoto
- O mesmo índice em ambos os Arrays sempre representa o mesmo jogador

**POR QUE USAR ARRAYS PARALELOS?**
- ✅ Mais simples que criar uma estrutura custom no Content Browser
- ✅ Fácil de usar com `Find Item in Array` para buscar por PlayerId
- ✅ Tipos nativos do Blueprint (Integer e Actor Reference)
- ✅ Performance adequada para até dezenas de jogadores simultâneos

---

**COMO CRIAR OS ARRAYS - PASSO A PASSO:**

### **PASSO 1: Criar `RemoteActorIds` (Array of Integer)**

1. **No painel `Meu Blueprint` (lado esquerdo):**
   - Clique no `+` ao lado de "VARIÁVEIS" → selecione "Add Variable"
   - OU: clique com botão direito na lista de variáveis → "Add Variable"

2. **Configurar a variável:**
   - **Nome**: Digite exatamente `RemoteActorIds` (sem espaços, case-sensitive)
   - **Tipo da Variável**: No dropdown, procure por **"Array"**
     - Se não aparecer diretamente, digite "Array" na busca
     - Selecione `Array` (não "Array Element" ou outras variações)

3. **Configurar o tipo interno do Array (Inner Type):**
   - Após selecionar "Array", aparecerá um campo adicional abaixo: **"Inner"** ou **"Element Type"**
   - Clique neste campo e procure por **"Integer"** ou **"Número Inteiro"**
   - Selecione `Integer` (ou `int` dependendo da versão do Unreal)
   - **Resultado**: O tipo completo será `Array of Integer`

4. **Configurações opcionais:**
   - **Padrão (Default Value)**: Deixe vazio (Array vazio inicialmente)
   - **Instance Editable**: Pode deixar desmarcado (não precisa editar no editor)
   - **Tooltip** (opcional): "Array de PlayerIds dos jogadores remotos"

5. **Verificar:**
   - A variável deve aparecer na lista como: `RemoteActorIds` (Array of Integer)

---

### **PASSO 2: Criar `RemoteActors` (Array of Actor Reference)**

1. **No painel `Meu Blueprint` (lado esquerdo):**
   - Clique no `+` ao lado de "VARIÁVEIS" → selecione "Add Variable"

2. **Configurar a variável:**
   - **Nome**: Digite exatamente `RemoteActors` (sem espaços, case-sensitive)
   - **Tipo da Variável**: No dropdown, procure por **"Array"**

3. **Configurar o tipo interno do Array (Inner Type):**
   - Após selecionar "Array", clique no campo **"Inner"** ou **"Element Type"**
   - Procure por **"Object Reference"** ou **"Actor Reference"**
   - **IMPORTANTE**: Você precisa especificar que é um Actor:
     - Opção 1: Digite "Actor" e selecione `Actor` (classe base)
     - Opção 2: Digite "Object" e depois clique no campo adicional que aparece para especificar `Actor`
     - **Resultado esperado**: `Array of Actor` ou `Array of Actor Reference`

4. **Se não aparecer "Actor" diretamente:**
   - Tente procurar por "Object Reference"
   - Após selecionar, procure um campo adicional para restringir o tipo
   - Selecione "Actor" como a classe base
   - **Alternativa**: Deixe como "Object Reference" genérico (funciona também, mas menos type-safe)

5. **Configurações opcionais:**
   - **Padrão (Default Value)**: Deixe vazio (Array vazio inicialmente)
   - **Instance Editable**: Pode deixar desmarcado
   - **Tooltip** (opcional): "Array de Actors remotos (mesma ordem que RemoteActorIds)"

6. **Verificar:**
   - A variável deve aparecer na lista como: `RemoteActors` (Array of Actor/Object Reference)

---

### **PASSO 3: Verificar que os Arrays estão corretos**

**Lista de variáveis esperada:**
```
✓ RemoteActorIds: Array of Integer
✓ RemoteActors: Array of Actor (ou Array of Object Reference)
```

**Como verificar no Blueprint:**
1. Selecione cada variável na lista
2. No painel "Detalhes" (direita), veja o tipo completo
3. **IMPORTANTE**: Certifique-se de que:
   - `RemoteActorIds` é realmente `Array of Integer`
   - `RemoteActors` é realmente `Array of Actor` (ou Object Reference)

---

### **COMO FUNCIONAM OS ARRAYS PARALELOS:**

**Exemplo prático:**
```
Situação: Você tem 2 jogadores remotos no mundo

RemoteActorIds: [100, 250]     ← PlayerIds
RemoteActors:   [Actor1, Actor2] ← Referências dos Actors
                  ↑      ↑
                Índice 0  Índice 1
```

**Regras de Sincronização:**
- ✅ `RemoteActorIds[0]` e `RemoteActors[0]` sempre representam o mesmo jogador
- ✅ Se adicionar um novo jogador:
  1. Adicione `PlayerId` em `RemoteActorIds` (usando `Add Item to Array`)
  2. Adicione `Actor` em `RemoteActors` (usando `Add Item to Array`)
  3. **SEMPRE na mesma ordem!**
- ✅ Se remover um jogador:
  1. Remova do `RemoteActorIds` (usando `Remove Item` com o índice)
  2. Remova do `RemoteActors` (usando `Remove Item` com o **mesmo índice**)

**Buscar um Actor por PlayerId:**
```
1. Find Item in Array (RemoteActorIds, PlayerId) → FoundIndex
2. Se FoundIndex >= 0:
   Get Element (RemoteActors, FoundIndex) → RemoteActorRef
```

---

### **ALTERNATIVA: Usar Estrutura Custom (Opcional)**

Se preferir usar uma estrutura única em vez de dois Arrays:

**Criar a estrutura no Content Browser:**
1. No Content Browser (aba "Content"), clique com botão direito
2. **Blueprint** → **Structure** (não "Blueprint Class"!)
3. Nome: `RemoteActorEntry`
4. Dentro da estrutura, adicione:
   - `PlayerId` (Integer)
   - `ActorRef` (Actor Reference)
5. Salve a estrutura

**Criar a variável:**
- Nome: `RemoteActors` (pode usar o mesmo nome)
- Tipo: `Array of RemoteActorEntry`
- Use `Find Item in Array` procurando pela estrutura completa (mais complexo)

**Recomendação**: Use os **Arrays paralelos** (mais simples e mais direto).

---

### **NOTAS IMPORTANTES:**

1. **Ordem de criação:**
   - Pode criar os Arrays em qualquer ordem
   - Mas sempre use `RemoteActorIds` primeiro quando for adicionar itens (para manter a ordem mental)

2. **Nomes das variáveis:**
   - Os nomes são case-sensitive no Blueprint
   - Use exatamente: `RemoteActorIds` e `RemoteActors`
   - Não use espaços ou caracteres especiais

3. **Inicialização:**
   - Ambos os Arrays começam vazios (sem itens)
   - Itens serão adicionados dinamicamente quando receber StateUpdates de novos jogadores

4. **Validação:**
   - Sempre mantenha os Arrays com o mesmo tamanho
   - Se `RemoteActorIds.Num()` != `RemoteActors.Num()`, há um erro de sincronização
   - Considere adicionar uma função de validação para debug (opcional)

---

### **RESUMO RÁPIDO:**

**Para criar `RemoteActorIds`:**
1. Criar variável → Nome: `RemoteActorIds`
2. Tipo: `Array` → Inner: `Integer`
3. Pronto!

**Para criar `RemoteActors`:**
1. Criar variável → Nome: `RemoteActors`
2. Tipo: `Array` → Inner: `Actor Reference`
3. Pronto!

**Uso:**
- Adicionar jogador: Adicione em ambos os Arrays na mesma ordem
- Buscar jogador: Use `Find Item in Array` no `RemoteActorIds`, depois `Get Element` no `RemoteActors`
- Remover jogador: Remova do mesmo índice em ambos os Arrays

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

### 3.5. OnWSConnected (ordem dos nós) - PASSO A PASSO DETALHADO

**O QUE É OnWSConnected?**
- É o Custom Event que você criou no passo 3.4.
- Este evento é chamado automaticamente quando o WebSocket conecta com sucesso ao servidor.

**IMPORTANTE: Criar a função SendMoveUpdate ANTES de usar o Timer**

Você precisa criar a função `SendMoveUpdate` PRIMEIRO antes de poder usá-la no Timer. Vá para a seção 3.6 abaixo para criar a função, depois volte aqui para conectar o Timer.

**Passos no gráfico `OnWSConnected`**:

**1) Set IsConnected = true**
- Arraste a variável `IsConnected` no gráfico → `Get IsConnected`
- Do `Get IsConnected`, procure `Set IsConnected` (ou use `Set IsConnected` diretamente)
- Conecte o pin de execução (branco) do evento `OnWSConnected` ao pin de execução do `Set IsConnected`
- Conecte `true` (constante booleana) ao pin de input `IsConnected` do `Set`
- O output de execução do `Set` será conectado ao próximo nó

**2) Set Timer by Function Name (Looping)**
- Procure o nó `Set Timer by Function Name` (categoria "Timers" ou digite "Timer")
- **Inputs do Timer:**
  - `Function Name`: Digite exatamente `SendMoveUpdate` (texto/string)
    - **IMPORTANTE**: Este é o nome da função que você criará no passo 3.6
    - O nome DEVE ser exatamente igual ao nome da função
  - `Time`: Crie um nó matemático:
    - `Get SendRateHz` (variável Float)
    - `1.0` (constante Float)
    - `Divide` (nó matemático): `1.0 / SendRateHz`
    - Exemplo: se `SendRateHz = 20.0`, então `Time = 0.05` (1 segundo / 20 = 0.05 segundos)
  - `Looping`: Conecte `true` (constante booleana)
  - `Object` (opcional): Deixe vazio ou conecte `self` (o próprio `BP_NetMovementClient`)
- **Outputs:**
  - `Return Value`: `Timer Handle` - conecte este ao pin de input do `Set SendTimerHandle`
    - Arraste a variável `SendTimerHandle` → `Set SendTimerHandle`
    - Conecte o `Return Value` do Timer ao pin `SendTimerHandle` do Set

**Fluxo visual**:
```
OnWSConnected (Custom Event)
    ↓ (execution pin - branco)
Set IsConnected = true
    ↓ (execution pin - branco)
Set Timer by Function Name
    - Function Name: "SendMoveUpdate"
    - Time: 1.0 / SendRateHz
    - Looping: true
    ↓ (Return Value - Timer Handle)
Set SendTimerHandle = [Timer Handle retornado]
```

### 3.6. SendMoveUpdate - CRIAR FUNÇÃO (CRÍTICO - FAZER ANTES DO PASSO 3.5!)

**O QUE É SendMoveUpdate?**
- É uma **Função** (Function) no Blueprint, não um evento.
- Esta função será chamada repetidamente pelo Timer a cada `1.0/SendRateHz` segundos (ex.: a cada 0.05s se SendRateHz = 20).
- Ela coleta a posição atual do jogador local e envia para o servidor via WebSocket.

**COMO CRIAR A FUNÇÃO**:

1. No painel `Meu Blueprint` (esquerda), vá em `FUNÇÕES`
2. Clique no `+` ao lado de "FUNÇÕES"
3. Selecione "Add Function"
4. Nomeie exatamente: `SendMoveUpdate` (sem espaços extras, maiúsculas/minúsculas importam)
5. A função será criada e você verá um novo gráfico "SendMoveUpdate" no editor

**Ordem dos nós no gráfico `SendMoveUpdate`**:

**1) Obter Pawn local**
- Procure `Get Player Pawn` (categoria "Utilities" ou digite "Player Pawn")
- Este nó retorna o Pawn/Actor controlado pelo jogador local
- Output: `Return Value` (tipo: Pawn ou Actor)

**2) GetActorLocation**
- Do `Get Player Pawn`, conecte o `Return Value` ao input `Target` do nó `GetActorLocation`
- Se `GetActorLocation` não aparecer diretamente, arraste o `Return Value` do Pawn e procure "Location" ou "Get Actor Location"
- Output: `Return Value` (tipo: Vector) - esta é a posição X, Y, Z

**3) GetActorRotation**
- Do mesmo `Get Player Pawn`, conecte o `Return Value` ao input `Target` do nó `GetActorRotation`
- Output: `Return Value` (tipo: Rotator) - contém Roll, Pitch, Yaw
- Extrair Yaw: do `Return Value`, conecte o campo `Yaw` (Float) para uso no passo 5

**4) Calcular Timestamp em Milissegundos**
- `Get Game Time in Seconds` (retorna Float, tempo em segundos desde o início do jogo)
- Multiplicar por 1000: crie um nó `Multiply` → `Get Game Time in Seconds * 1000.0`
- Converter para Integer: conecte o resultado ao nó `To Integer (Float)` ou `Convert Float to Integer`
- Output: `Return Value` (Integer) - timestamp em milissegundos

**5) BuildMoveUpdateFrame**
- Procure `BuildMoveUpdateFrame` (categoria "Umbra|Net|WS|Binary" ou digite "BuildMoveUpdateFrame")
- **IMPORTANTE**: Use a versão que retorna `Array of Bytes` diretamente (não a versão com `OutBytes` como parâmetro separado)
- **Inputs:**
  - `PlayerId`: `Get LocalPlayerId` (variável Integer)
  - `Location`: o Vector do passo 2 (GetActorLocation → Return Value)
  - `YawDegrees`: o Float Yaw extraído do passo 3 (GetActorRotation → Yaw)
  - `TimestampMs`: o Integer do passo 4 (resultado da conversão)
- **Output:**
  - `Return Value`: Array of Bytes (pin roxo/violeta) - este é o frame binário pronto para enviar
  - **NOTA**: Esta versão retorna o Array diretamente, facilitando a conexão com `SendBytes`

**6) SendBytes**
- `Get WebSocketRef` (variável) → conecte ao input `Target` (pin azul) do nó `Send Bytes`
- Procurar `Send Bytes` no `WebSocketRef` (categoria "Umbra|Net|WS")
- Se não aparecer diretamente:
  - Arraste a variável `WebSocketRef` no gráfico
  - Clique com botão direito no `WebSocketRef` → procure "Send Bytes" ou "Send"
- **Input `Data`** (pin vermelho/Array of Bytes):
  - **Conecte diretamente** o `Return Value` (Array of Bytes) do `BuildMoveUpdateFrame` do passo 5
  - Se ainda der erro de tipo:
    - Crie uma variável local temporária do tipo `Array of Bytes`
    - Conecte o `Return Value` do `BuildMoveUpdateFrame` a essa variável
    - Conecte a variável ao `Data` do `SendBytes`

**Fluxo visual completo de SendMoveUpdate**:
```
[Função: SendMoveUpdate]
    ↓
Get Player Pawn
    ↓
GetActorLocation → Location (Vector)
    ↓
GetActorRotation → Yaw (Float)
    ↓
Get Game Time in Seconds * 1000 → To Integer → TimestampMs (Integer)
    ↓
BuildMoveUpdateFrame(PlayerId, Location, Yaw, TimestampMs) → Return Value (Array of Bytes)
    ↓ (conexão direta)
Get WebSocketRef → Send Bytes(Data: Return Value do BuildMoveUpdateFrame)
```

**NOTA IMPORTANTE SOBRE TIPOS**:
- Se o pin de `Return Value` do `BuildMoveUpdateFrame` não conectar diretamente ao `Data` do `SendBytes`:
  1. **Solução rápida**: Delete a conexão atual e reconecte (às vezes o Blueprint precisa recompilar)
  2. **Solução alternativa**: Use uma variável local temporária como intermediário (veja detalhes no passo 6)
  3. **Verifique**: Após recompilar o C++, certifique-se de que está usando a versão correta de `BuildMoveUpdateFrame` (a que retorna `Array of Bytes`, não a versão antiga)

**IMPORTANTE**: Após criar esta função, volte ao passo 3.5 e configure o Timer para chamá-la.

Observação: agora já incluímos uma BPFunctionLibrary no projeto com nós prontos:
- `AppendUInt32LE(Bytes, Value)`
- `AppendFloatLE(Bytes, Value)`
- `BuildMoveUpdateFrame(PlayerId, Location, YawDegrees, TimestampMs, OutBytes)`
Use diretamente `BuildMoveUpdateFrame` e envie `OutBytes` via `WebSocket.Send Bytes`.

### 3.7. OnWSBinaryMessage (recepção de updates do servidor) - PASSO A PASSO DETALHADO

Esta função é chamada automaticamente quando o servidor envia dados binários (StateUpdate).

**O QUE É OnWSBinaryMessage?**
- É o Custom Event que você criou no passo 3.4 com parâmetro `Data` (Array of Bytes).
- Este evento é chamado automaticamente pelo WebSocket quando dados binários são recebidos.

**SOBRE O NÓ "RETURN" (Lido antes de começar)**:
- O nó `Return` interrompe a execução do evento/função atual.
- **Como encontrar**: Clique com botão direito → digite "Return" → selecione `Return` ou `Return Node`.
- **Alternativa se não aparecer**: Use um `Branch` e deixe o lado que você quer "ignorar" desconectado (sem conectar a nada).
- Você usará `Return` várias vezes nesta função para sair cedo se as condições não forem atendidas.

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
- Do `ParseStateUpdateFrame`, ligue o `Return Value` (Boolean) em um nó `Branch`
- **O QUE É "Return"?**
  - `Return` é um nó especial que **interrompe a execução** da função/evento atual.
  - Ele para imediatamente a execução e não executa nenhum nó que vier depois dele.
  - Em funções, você pode ter um pin de saída "Return" na própria função; em eventos, use o nó `Return` explícito.
- **COMO ACESSAR O NÓ RETURN:**
  1. Clique com botão direito no gráfico (no `OnWSBinaryMessage`)
  2. Digite "Return" na busca
  3. Selecione o nó `Return` ou `Return Node` (geralmente aparece como um nó especial com apenas um pin de execução de entrada)
  4. **ALTERNATIVA**: Se não aparecer, você pode usar um `Branch` com o lado `false` desconectado (deixar vazio) - isso efetivamente faz a função parar se a condição for false
- **COMO USAR:**
  - Se o `Branch` retornar `false` (parse falhou):
    - Conecte o pin "False" do `Branch` ao pin de execução (branco) do nó `Return`
    - Isso fará a função parar imediatamente
  - Se o `Branch` retornar `true` (parse OK):
    - Conecte o pin "True" ao próximo passo da lógica

**3) Verificar se é StateUpdate (opcional, mas recomendado)**
- `Data` → `Get (Array)` com Index `0` (primeiro byte)
- Comparar: crie um nó `Equal (Integer)` ou `!=` (Not Equal)
  - Um lado: o valor obtido do Array (Integer)
  - Outro lado: `2` (constante Integer - StateUpdate tem type = 2)
- **Se o valor != 2**: conecte ao pin "True" de um `Branch` e este ao nó `Return` (mesmo processo do passo 2)
- **Se o valor == 2**: conecte ao pin "False" e continue para o próximo passo

**4) Ignorar se é o próprio jogador**
- `OutPlayerId` → comparar (`==`) com `LocalPlayerId` (variável)
- Crie um nó `Equal (Integer)`:
  - `OutPlayerId` (do ParseStateUpdateFrame)
  - `Get LocalPlayerId` (variável)
- Conecte o resultado em um `Branch`
- **Se `==` (true)**: conecte ao pin "True" do `Branch` → nó `Return` (não precisamos atualizar nosso próprio estado)
- **Se `!=` (false)**: conecte ao pin "False" e continue para o próximo passo

**5) Obter ou Criar Entry no Array `RemoteStates` (USANDO FUNÇÃO HELPER) - PASSO A PASSO DETALHADO**

Este passo busca ou cria uma entrada no Array `RemoteStates` para o `player_id` recebido.

**O QUE FAZER:**
- **Conexão de execução**: Do pin "False" (não é o próprio jogador) do `Branch` do passo 4, conecte ao pin de execução (branco) do nó `GetOrCreatePlayerState`
- **Criar o nó**: Procure `GetOrCreatePlayerState` (categoria "Umbra|Net|WS|State" ou digite "GetOrCreatePlayerState")
- **Inputs do nó:**
  - `StatesArray`: Arraste a variável `RemoteStates` no gráfico → `Get RemoteStates`
    - Conecte o `Return Value` (Array of Player State Entry) ao input `StatesArray` do `GetOrCreatePlayerState`
    - **NOTA IMPORTANTE**: Este input é `UPARAM(ref)`, então ele modifica o Array automaticamente se criar um novo elemento
  - `PlayerId`: Conecte o `OutPlayerId` do `ParseStateUpdateFrame`
    - **DICA**: Se `OutPlayerId` estiver muito longe, use um `Knot` (Reroute Node) para organizar os fios:
      - Crie um `Knot` entre `ParseStateUpdateFrame` e `GetOrCreatePlayerState`
      - Conecte `OutPlayerId` → `Knot` → `PlayerId` do `GetOrCreatePlayerState`
- **Outputs do nó:**
  - `Return Value` (Player State Entry): Esta é a estrutura retornada (se encontrada) ou criada (se nova)
    - **Guarde esta saída** - você usará no próximo passo
  - Pin de execução "then": Conecte ao próximo nó (passo 6)

**FLUXO VISUAL:**
```
Branch (passo 4, false = não é próprio jogador)
    ↓ (execution pin - branco, lado "False")
GetOrCreatePlayerState
    - StatesArray: Get RemoteStates (Array)
    - PlayerId: OutPlayerId (via Knot se necessário)
    ↓ (Return Value: Player State Entry)
[GUARDE ESTE RESULTADO PARA O PRÓXIMO PASSO]
    ↓ (execution pin "then")
Próximo passo (UpdatePlayerStateBuffer)
```

**6) Atualizar o Buffer (USANDO FUNÇÃO HELPER) - PASSO A PASSO DETALHADO**

Este passo atualiza o buffer de estados com o novo estado recebido, fazendo a rotação automaticamente (StateA → StateB → novo em B).

**O QUE FAZER:**
- **Conexão de execução**: Do pin "then" do `GetOrCreatePlayerState` (passo 5), conecte ao pin de execução (branco) do nó `UpdatePlayerStateBuffer`
- **Criar o nó**: Procure `UpdatePlayerStateBuffer` (categoria "Umbra|Net|WS|State" ou digite "UpdatePlayerStateBuffer")
- **Inputs do nó:**
  - `Entry`: **Conecte diretamente o `Return Value` (Player State Entry) do `GetOrCreatePlayerState` do passo 5**
    - **IMPORTANTE**: Este input é `UPARAM(ref)`, então a função modifica a estrutura diretamente
    - Você não precisa fazer "Get Element" do Array aqui - use o resultado do passo 5 diretamente
  - `NewLocation`: Conecte o `OutLocation` (Vector) do `ParseStateUpdateFrame`
  - `NewYaw`: Conecte o `OutYawDegrees` (Float) do `ParseStateUpdateFrame`
  - `NewTimestampMs`: Conecte o `OutTimestampMs` (Integer) do `ParseStateUpdateFrame`
- **O QUE ESTA FUNÇÃO FAZ AUTOMATICAMENTE:**
  - Se não tem StateA → salva em StateA
  - Se tem StateA mas não StateB → salva em StateB
  - Se tem ambos → move StateB para StateA e salva novo em StateB
- **Outputs:**
  - Pin de execução "then": Conecte ao próximo passo (atualizar Array)

**ATUALIZAR O ARRAY `RemoteStates` (CRÍTICO - NÃO ESQUEÇA!)**

**IMPORTANTE**: `GetOrCreatePlayerState` retorna uma **CÓPIA** da estrutura (não uma referência). Quando você passa essa cópia para `UpdatePlayerStateBuffer`, ele modifica a **cópia**, não a estrutura original no Array. Por isso você **DEVE** salvar a estrutura modificada de volta no Array!

**PASSO A PASSO:**

1. **Encontrar o índice no Array:**
   - Do pin "then" do `UpdatePlayerStateBuffer`, conecte ao pin de execução do nó `FindPlayerStateIndex`
   - Procure `FindPlayerStateIndex` (categoria "Umbra|Net|WS|State")
   - Inputs:
     - `StatesArray`: `Get RemoteStates` (o mesmo Array usado no passo 5)
     - `PlayerId`: `OutPlayerId` (use o mesmo Knot do passo 5, ou conecte diretamente)
   - Output: `Return Value` (Integer) - o índice onde está a entry no Array
     - **NOTA**: Como `GetOrCreatePlayerState` já adicionou ao Array se não existia, este sempre retornará um índice válido (>= 0)

2. **Obter a estrutura modificada do nó UpdatePlayerStateBuffer:**
   - **PROBLEMA**: `UpdatePlayerStateBuffer` modifica a estrutura por referência (`UPARAM(ref)`), mas como você passou uma cópia (do `GetOrCreatePlayerState`), a cópia foi modificada, não a original no Array
   - **SOLUÇÃO**: Você precisa pegar a estrutura modificada. Mas `UpdatePlayerStateBuffer` não retorna nada!
   - **WORKAROUND**: Como `UpdatePlayerStateBuffer` modifica o `Entry` por referência, a estrutura que você passou (do `GetOrCreatePlayerState`) já está modificada. Use essa mesma estrutura:
     - **Reutilize o `Return Value` do `GetOrCreatePlayerState`** que você passou para `UpdatePlayerStateBuffer`
     - Após `UpdatePlayerStateBuffer`, essa estrutura (mesma referência) já está modificada
     - Conecte esse mesmo `Return Value` ao `Item` do `Set Element`

3. **Atualizar o Array com Set Element:**
   - Procure `Set Element` (categoria "Array" ou digite "Set Array Elem")
   - Inputs:
     - `Array`: `Get RemoteStates` 
       - **ATENÇÃO**: Conecte diretamente `Get RemoteStates` → `Set Element` → input `Array`
       - Este input é `UPARAM(ref)`, então modifica o Array diretamente
     - `Index`: Conecte o `Return Value` (Integer) do `FindPlayerStateIndex` do passo acima
     - `Item`: Conecte o **mesmo `Return Value` (Player State Entry) do `GetOrCreatePlayerState`** que você passou para `UpdatePlayerStateBuffer`
       - **Como funciona**: O `Return Value` do `GetOrCreatePlayerState` é uma cópia que foi modificada por `UpdatePlayerStateBuffer` (por referência), então agora ela contém os valores atualizados
   - Pin de execução: Não precisa conectar nada depois (ou conecte ao passo 7 se quiser criar o Actor remoto)

**EXPLICAÇÃO TÉCNICA**:
- `GetOrCreatePlayerState` retorna `FPlayerStateEntry` por valor (cópia)
- Você passa essa cópia para `UpdatePlayerStateBuffer` (que aceita `UPARAM(ref)`)
- O Blueprint cria uma referência temporária para a cópia e a passa para a função C++
- A função C++ modifica essa cópia através da referência
- Agora a cópia está modificada, mas a original no Array não foi modificada
- Por isso você precisa fazer `Set Element` para copiar a estrutura modificada de volta para o Array

**FLUXO VISUAL COMPLETO (Passos 5 e 6 combinados) - COM TODAS AS CONEXÕES:**

```
OnWSBinaryMessage (Data: Array of Bytes)
    ↓
ParseStateUpdateFrame (Data) → OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs, ReturnValue (bool)
    ↓ (execution then, se ReturnValue == true)
[Branch: verifica se parse OK]
    ↓ (True = parse OK)
[Branch: verifica se Data[0] == 2] (opcional)
    ↓ (True = é StateUpdate)
[Branch: verifica se OutPlayerId != LocalPlayerId]
    ↓ (False = não é próprio jogador, continua)
GetOrCreatePlayerState
    - StatesArray: Get RemoteStates (Array)
    - PlayerId: OutPlayerId (via Knot se necessário)
    ↓ (Return Value: Entry - CÓPIA da estrutura)
    [GUARDE ESTA SAÍDA - você usará duas vezes]
    ↓ (execution then)
UpdatePlayerStateBuffer
    - Entry: [Return Value do GetOrCreatePlayerState - a cópia]
    - NewLocation: OutLocation (do ParseStateUpdateFrame)
    - NewYaw: OutYawDegrees (do ParseStateUpdateFrame)
    - NewTimestampMs: OutTimestampMs (do ParseStateUpdateFrame)
    [NOTA: Entry agora está MODIFICADA (cópia modificada)]
    ↓ (execution then)
FindPlayerStateIndex
    - StatesArray: Get RemoteStates (mesmo Array)
    - PlayerId: OutPlayerId (mesmo usado acima)
    ↓ (Return Value: Index - Integer)
Set Element
    - Array: Get RemoteStates (conecte diretamente)
    - Index: [Return Value do FindPlayerStateIndex]
    - Item: [MESMO Return Value do GetOrCreatePlayerState - a cópia MODIFICADA]
    [NOTA: Isso copia a estrutura modificada de volta para o Array]
```

**CONEXÕES DE DADOS IMPORTANTES**:

1. **Para `OutPlayerId` (Integer) - usar Knot para organizar:**
   - O `OutPlayerId` é usado em múltiplos lugares
   - **Crie um Knot (Reroute Node)**:
     - Botão direito no gráfico → digite "Knot" ou "Reroute"
     - Selecione `Knot` (um nó simples com 2 pins: Input e Output)
   - Conecte:
     - `ParseStateUpdateFrame.OutPlayerId` → `Knot.InputPin`
     - `Knot.OutputPin` → `GetOrCreatePlayerState.PlayerId`
     - `Knot.OutputPin` → `FindPlayerStateIndex.PlayerId`
     - `Knot.OutputPin` → `Equal (Integer).A` (para comparar com `LocalPlayerId`)
   - **Vantagem**: Isso organiza o gráfico e evita fios cruzados

2. **Para `Return Value` (Entry) do `GetOrCreatePlayerState`:**
   - Esta saída é usada DUAS vezes:
     - Uma vez como entrada para `UpdatePlayerStateBuffer.Entry`
     - Uma vez como entrada para `Set Element.Item` (após modificação)
   - **Como conectar**:
     - Conecte diretamente `GetOrCreatePlayerState.ReturnValue` → `UpdatePlayerStateBuffer.Entry`
     - **Para o Set Element**: Use o MESMO `Return Value` (não precisa criar nova conexão, apenas conecte o mesmo pin novamente ao `Set Element.Item`)
     - **OU**: Use outro Knot para organizar:
       - `GetOrCreatePlayerState.ReturnValue` → `Knot.InputPin`
       - `Knot.OutputPin` → `UpdatePlayerStateBuffer.Entry`
       - `Knot.OutputPin` → `Set Element.Item`

3. **NÃO fazer**:
   - **NÃO** conecte `Get Element` do Array ao `Set Element.Item`
   - **NÃO** use `Get RemoteStates` → `Get Element` após `UpdatePlayerStateBuffer` - use o `Return Value` do `GetOrCreatePlayerState` diretamente

**NOTA IMPORTANTE SOBRE UPARAM(REF):**
- `GetOrCreatePlayerState` tem `StatesArray` como `UPARAM(ref)` - modifica o Array automaticamente (adiciona se não existe)
- `UpdatePlayerStateBuffer` tem `Entry` como `UPARAM(ref)` - modifica a estrutura automaticamente
- **MAS**: Como `GetOrCreatePlayerState` retorna uma CÓPIA, quando você modifica essa cópia com `UpdatePlayerStateBuffer`, a original no Array não é modificada
- **SOLUÇÃO OBRIGATÓRIA**: Sempre faça `Set Element` após `UpdatePlayerStateBuffer` para copiar a estrutura modificada de volta para o Array

**RESUMO RÁPIDO DOS PASSOS 5 E 6:**
1. `GetOrCreatePlayerState` → retorna Entry (cópia)
2. `UpdatePlayerStateBuffer` → modifica a Entry (cópia modificada)
3. `FindPlayerStateIndex` → encontra o índice no Array
4. `Set Element` → copia a Entry modificada de volta para o Array no índice encontrado

**ERRO COMUM**: Esquecer o `Set Element` - isso fará com que os estados nunca sejam atualizados no Array!

**7) Criar/Obter Actor Remoto (opcional mas recomendado) - PASSO A PASSO DETALHADO**

Este passo cria o Actor remoto quando recebemos o primeiro StateUpdate de um novo jogador. É recomendado fazer isso aqui em vez de no `Tick` para manter o `Tick` mais leve.

**IMPORTANTE**: Você precisa ter criado os Arrays `RemoteActorIds` e `RemoteActors` conforme a seção 3.3.

---

**PASSO 7.1: Verificar se o Actor já existe**

- **Do pin de execução após `Set Element` (passo 6)**, conecte ao pin de execução de um `Find Item in Array`
- **Criar `Find Item in Array`**:
  - Botão direito → digite "Find Item in Array"
  - Selecione `Find Item in Array` (categoria "Array")
- **Inputs:**
  - `Array`: `Get RemoteActorIds` → `Return Value` (Array of Integer)
  - `Item`: `OutPlayerId` (Integer, do `ParseStateUpdateFrame` - use o mesmo Knot do passo 5 se tiver)
- **Output:**
  - `Index`: `FoundIndex` (Integer) - índice encontrado, ou `-1` se não encontrado
- **Criar `Branch`**:
  - `Greater or Equal (FoundIndex, 0)` → conecte ao `Condition` do `Branch`
  - Pin "True": Actor já existe, pode pular este passo ou fazer validações
  - Pin "False": Actor não existe, precisa spawnar (continue para passo 7.2)

---

**PASSO 7.2: Spawn do Actor Remoto (apenas se não encontrado)**

- **Do pin "False" do `Branch`**, conecte ao pin de execução do `Spawn Actor from Class`
- **Criar `Spawn Actor from Class`**:
  - Botão direito → digite "Spawn Actor" ou "Spawn"
  - Selecione `Spawn Actor from Class` (categoria "Actor")
- **Inputs do `Spawn Actor from Class`:**
  - `Class`: **Selecione a classe do Actor remoto**
    - **Opções**:
      1. Se você tem uma classe Blueprint específica para players remotos (recomendado):
         - Arraste a classe Blueprint no gráfico, OU
         - Clique no dropdown `Class` e procure pelo nome da classe
      2. Se você quer usar a mesma classe do seu Player Pawn:
         - Procure a classe do seu Player Pawn (ex.: `BP_Player` ou similar)
      3. Se você quer uma classe simples:
         - Pode usar `Actor` genérico ou criar uma classe Blueprint específica
    - **DICA**: Crie uma classe Blueprint baseada no seu Player Pawn, mas remova os componentes de Input para economizar processamento
  - `Transform`: **Criar `Make Transform`**:
    - Botão direito → digite "Make Transform"
    - Inputs:
      - `Location`: Use `OutLocation` (Vector, do `ParseStateUpdateFrame`)
        - Isso spawna o Actor na posição recebida do servidor
      - `Rotation`: **Criar `Make Rotator`**:
        - Botão direito → digite "Make Rotator"
        - Inputs:
          - `Roll`: `0.0`
          - `Pitch`: `0.0`
          - `Yaw`: `OutYawDegrees` (Float, do `ParseStateUpdateFrame`)
        - Output: `NewRotation` (Rotator)
        - Conecte ao `Rotation` do `Make Transform`
      - `Scale`: `1.0, 1.0, 1.0` (constante Vector) ou use `Make Vector` com `X=1, Y=1, Z=1`
  - `World Context Object`: Deixe vazio ou conecte `Get World` (usa o contexto atual)
  - `No Collision Fail`: deixe `false` (ou `true` se quiser spawnar mesmo com colisão)
- **Output:**
  - `Return Value`: `NewActorRef` (Actor Reference) - o Actor recém-spawnado
  - **GUARDE ESTE RESULTADO** - você adicionará aos Arrays

---

**PASSO 7.3: Adicionar PlayerId ao Array `RemoteActorIds`**

- **Criar `Add Item to Array`**:
  - Botão direito → digite "Add Item to Array" ou "Add"
  - Selecione `Add Item to Array` (categoria "Array")
- **Inputs:**
  - `Array`: `Get RemoteActorIds` → `Return Value` (Array of Integer)
  - `Item`: `OutPlayerId` (Integer, do `ParseStateUpdateFrame`)
    - Use o mesmo Knot do passo 5 se tiver, ou conecte diretamente
- **Output:**
  - `Return Value`: O Array modificado (com o novo PlayerId adicionado)
  - **NOTA**: O `Add Item to Array` já modifica o Array automaticamente se for `UPARAM(ref)`
  - Se necessário, conecte este `Return Value` a `Set RemoteActorIds` para garantir

---

**PASSO 7.4: Adicionar Actor ao Array `RemoteActors`**

- **IMPORTANTE**: Faça isso **APÓS** adicionar ao `RemoteActorIds` para manter sincronização!
- **Criar `Add Item to Array`**:
  - `Array`: `Get RemoteActors` → `Return Value` (Array of Actor/Object Reference)
  - `Item`: `NewActorRef` (Actor Reference, do passo 7.2)
- **Output:**
  - `Return Value`: O Array modificado (com o novo Actor adicionado)
- **CRÍTICO**: Os Arrays agora estão sincronizados:
  - Se `RemoteActorIds[N] = OutPlayerId`, então `RemoteActors[N] = NewActorRef`

---

**Fluxo Visual Completo do Passo 7:**

```
[Após Set Element do passo 6]
    ↓
Find Item in Array (RemoteActorIds, OutPlayerId) → FoundIndex
    ↓
Branch: FoundIndex >= 0?
    ↓
    ├─ True: Actor já existe → [Fim - pode pular ou fazer validações]
    │
    └─ False: Actor não existe
        ↓
        Make Rotator (Roll: 0, Pitch: 0, Yaw: OutYawDegrees) → NewRotation
        ↓
        Make Transform (Location: OutLocation, Rotation: NewRotation, Scale: 1,1,1)
        ↓
        Spawn Actor from Class
            - Class: [sua classe de Actor remoto]
            - Transform: [do Make Transform acima]
        ↓ (NewActorRef)
        Add Item to Array (RemoteActorIds, OutPlayerId)
        ↓
        Add Item to Array (RemoteActors, NewActorRef)
        ↓
        [Arrays sincronizados - fim do passo 7]
```

---

**NOTAS IMPORTANTES:**

1. **Ordem de adicionar aos Arrays**:
   - **SEMPRE** adicione primeiro em `RemoteActorIds`, depois em `RemoteActors`
   - Isso garante que o índice seja o mesmo em ambos os Arrays

2. **Spawn na posição correta**:
   - Use `OutLocation` e `OutYawDegrees` do `ParseStateUpdateFrame` para spawnar o Actor na posição recebida
   - O Actor aparecerá imediatamente na posição correta (sem interpolação inicial)

3. **Classe do Actor remoto**:
   - **Recomendação**: Crie uma classe Blueprint baseada no seu Player Pawn
   - Remova componentes de Input (não precisa de input para Actors remotos)
   - Mantenha componentes visuais (Mesh, etc.)
   - Isso economiza processamento

4. **Validação opcional**:
   - Após spawnar, você pode fazer verificações adicionais (ex.: verificar se o Actor foi spawnado com sucesso)
   - Use `Is Valid` no `NewActorRef` antes de adicionar ao Array

**Nota**: A interpolação real será feita no `Tick`, não aqui. Aqui apenas criamos o Actor quando recebemos o primeiro StateUpdate de um novo jogador.

### 3.8. Tick (no `BP_NetMovementClient`) - PASSO A PASSO DETALHADO

No evento `Event Tick`, adicione a lógica de interpolação para cada jogador remoto.

**IMPORTANTE**: Este processo é executado a cada frame (~60-120 FPS), então otimize o código para evitar operações pesadas.

**Ordem dos nós no `Event Tick` - PASSO A PASSO:**

**1) Iterar sobre o Array `RemoteStates` - PASSO A PASSO**

- **Criar o nó `Event Tick`**: Já existe no Blueprint (se não existir, clique com botão direito → "Event Tick")
- **Obter `RemoteStates`**:
  - Arraste a variável `RemoteStates` no gráfico → `Get RemoteStates`
  - Output: `Return Value` (Array of Player State Entry)
- **Criar `For Each Loop`**:
  - Botão direito no gráfico → digite "For Each" ou "For Each Loop"
  - Selecione `For Each Loop` (categoria "Flow Control")
  - **Input `Array`**: Conecte o `Return Value` (Array) do `Get RemoteStates`
- **O que o loop fornece:**
  - Pin de execução `Loop Body`: conecte aqui todos os nós que processam cada elemento
  - `Array Element` (Player State Entry): a estrutura com o buffer de estados
  - `Array Index` (Integer): índice atual no Array (geralmente não usado)
  - Pin de execução `Completed`: executado quando o loop termina (pode deixar desconectado)

**2) Verificar se tem dados suficientes para interpolação - PASSO A PASSO**

- **Do pin `Loop Body` do `For Each Loop`**, conecte ao pin de execução de um `Branch`
- **Acessar `HasStateA`**:
  - Do `Array Element` (Player State Entry), arraste e solte → procure `HasStateA`
  - Ou: `Array Element` → pin de saída `HasStateA` (Boolean)
- **Acessar `HasStateB`**:
  - Do mesmo `Array Element`, arraste e solte → procure `HasStateB`
  - Ou: `Array Element` → pin de saída `HasStateB` (Boolean)
- **Combinar com AND**:
  - Procure `Boolean AND` (categoria "Boolean" ou digite "AND")
  - Input `A`: conecte `ArrayElement.HasStateA`
  - Input `B`: conecte `ArrayElement.HasStateB`
  - Output `Return Value`: conecte ao pin `Condition` (Boolean) do `Branch`
- **Conectar o Branch:**
  - Pin "True" do `Branch`: conecta aos próximos passos (temos dados suficientes)
  - Pin "False" do `Branch`: conecta a um `Continue Loop` (pula para o próximo item)
    - **NOTA**: Para `Continue Loop`, procure "Continue Loop" (categoria "Flow Control")
    - Se não encontrar, simplesmente deixe o pin "False" desconectado (o loop continuará naturalmente)

**3) Calcular Alpha (fator de interpolação) - PASSO A PASSO**

**3.1) Obter tempo atual em milissegundos:**
- `Get Game Time in Seconds`:
  - Botão direito → digite "Get Game Time" ou "Game Time"
  - Output: `Return Value` (Float) - tempo em segundos
- **Multiplicar por 1000**:
  - Procure `Multiply (Float)` ou use o operador `*`
  - Input `A`: `Return Value` do `Get Game Time in Seconds`
  - Input `B`: `1000.0` (constante Float)
  - Output: tempo em milissegundos (Float)
- **Converter para Integer**:
  - Procure `To Integer (Float)` ou `Convert Float to Integer`
  - Input: resultado da multiplicação acima
  - Output: `Return Value` (Integer) - **`CurrentTimeMs`** (guarde este resultado)

**3.2) Calcular Delta entre estados:**
- **Subtrair timestamps:**
  - Procure `Subtract (Integer)` ou operador `-`
  - Input `A`: `ArrayElement.StateB_TimestampMs` (Integer)
  - Input `B`: `ArrayElement.StateA_TimestampMs` (Integer)
  - Output: `DeltaMs` (Integer) - diferença em milissegundos entre os dois estados
  - **PROTEÇÃO**: Se `DeltaMs <= 0`, pode causar divisão por zero (veja passo 3.5)

**3.3) Calcular tempo decorrido desde StateA:**
- **Subtrair:**
  - `Subtract (Integer)` ou `-`
  - Input `A`: `CurrentTimeMs` (do passo 3.1)
  - Input `B`: `ArrayElement.StateA_TimestampMs`
  - Output: `ElapsedMs` (Integer) - tempo decorrido desde StateA

**3.4) Calcular Alpha (fator de interpolação):**
- **Converter para Float:**
  - `ElapsedMs` → `To Float (Integer)`
  - `DeltaMs` → `To Float (Integer)`
- **Dividir:**
  - Procure `Divide (Float)` ou operador `/`
  - Input `A`: `ElapsedMs` (convertido para Float)
  - Input `B`: `DeltaMs` (convertido para Float)
  - Output: `Alpha` (Float) - valor entre 0.0 e potencialmente > 1.0

**3.5) Clampear Alpha entre 0.0 e 1.0:**
- Procure `Clamp (Float)` (categoria "Math" → "Float")
- Inputs:
  - `Value`: `Alpha` (do passo 3.4)
  - `Min`: `0.0` (constante Float)
  - `Max`: `1.0` (constante Float)
- Output: `ClampedAlpha` (Float) - este é o valor final usado na interpolação
- **NOTA IMPORTANTE**: Se `Alpha >= 1.0`, significa que já passamos do StateB (servidor atrasado). O Clamp garante que não extrapolamos além do StateB.

**4) Interpolar Posição - PASSO A PASSO**

- Procure `Lerp (Vector)` ou `VInterp To` (categoria "Math" → "Vector")
- **IMPORTANTE**: Use `VInterp To` se quiser interpolação suavizada, ou `Lerp (Vector)` para interpolação linear simples
- **Para `Lerp (Vector)`:**
  - Input `A`: `ArrayElement.StateA_Location` (Vector)
  - Input `B`: `ArrayElement.StateB_Location` (Vector)
  - Input `Alpha`: `ClampedAlpha` (Float, do passo 3.5)
  - Output: `InterpolatedLocation` (Vector) - posição interpolada
- **Guarde este resultado** - será usado no passo 7

**5) Interpolar Yaw - PASSO A PASSO**

- Procure `Lerp (Float)` (categoria "Math" → "Float")
- Inputs:
  - `A`: `ArrayElement.StateA_Yaw` (Float)
  - `B`: `ArrayElement.StateB_Yaw` (Float)
  - `Alpha`: `ClampedAlpha` (Float, o mesmo usado no passo 4)
- Output: `InterpolatedYaw` (Float)
- **NOTA**: Se os yaws estão em direções opostas (ex.: -179° e 179°), o Lerp pode dar um caminho longo. Por enquanto, use o Lerp simples. Futuramente, você pode implementar "shortest path" (lerp entre -179° e 179° deve resultar em 180°, não 179°).

**6) Obter o Actor Remoto - PASSO A PASSO DETALHADO**

Este passo busca o Actor remoto correspondente ao `PlayerId` atual do loop, usando dois Arrays paralelos para manter a sincronização.

**PRÉ-REQUISITO: Criar os Arrays (se ainda não criou)**

Antes de usar, você precisa ter criado as variáveis conforme a seção 3.3:
- `RemoteActorIds`: Array of Integer (guarda os PlayerIds)
- `RemoteActors`: Array of Actor (guarda as referências dos Actors)

**IMPORTANTE**: Esses Arrays devem estar **sincronizados** - o mesmo índice em ambos representa o mesmo jogador:
- `RemoteActorIds[0]` = PlayerId do jogador
- `RemoteActors[0]` = Actor desse jogador

---

**PASSO 6.1: Obter o PlayerId do Array Element atual**

- **O que fazer**: Do `Array Element` (Player State Entry) do `For Each Loop`, você precisa extrair o `PlayerId`
- **Como fazer**:
  - Arraste o `Array Element` no gráfico ou clique nele
  - Procure o campo `PlayerId` ou expanda a estrutura
  - Conecte `ArrayElement.PlayerId` (Integer) - **GUARDE ESTA CONEXÃO** (você usará várias vezes)
- **Dica**: Se precisar usar em vários lugares, crie um `Knot`:
  - `ArrayElement.PlayerId` → `Knot.InputPin`
  - Use `Knot.OutputPin` nas próximas conexões

---

**PASSO 6.2: Buscar o índice no Array `RemoteActorIds`**

- **O que fazer**: Procurar o `PlayerId` no Array de IDs para encontrar o índice correspondente
- **Criar o nó `Find Item in Array`**:
  - Botão direito no gráfico → digite "Find Item in Array" ou "Find"
  - Selecione `Find Item in Array` (categoria "Array")
  - Este nó procura um item no Array e retorna o índice onde está
- **Inputs do `Find Item in Array`:**
  - `Array`: Conecte `Get RemoteActorIds` → `Return Value` (Array of Integer)
    - **Como obter**: Arraste a variável `RemoteActorIds` no gráfico → `Get RemoteActorIds`
    - Conecte o `Return Value` ao input `Array`
  - `Item`: Conecte o `PlayerId` (Integer) do passo 6.1
    - Se você criou um Knot, use `Knot.OutputPin`
    - Se não, conecte diretamente `ArrayElement.PlayerId`
- **Output do `Find Item in Array`:**
  - `Index`: `FoundIndex` (Integer)
    - Se encontrado: retorna o índice (0, 1, 2, ...)
    - Se não encontrado: retorna `-1`
- **GUARDE ESTE RESULTADO** (`FoundIndex`) - você usará no próximo passo

---

**PASSO 6.3: Verificar se o índice foi encontrado**

- **O que fazer**: Verificar se `FoundIndex >= 0` para saber se o Actor existe
- **Criar comparação `Greater or Equal`**:
  - Botão direito → digite "Greater or Equal" ou `>=`
  - Selecione `Greater or Equal (Integer)` (categoria "Math" → "Integer")
- **Inputs:**
  - `A`: Conecte o `FoundIndex` (Integer) do passo 6.2
  - `B`: Digite `0` (constante Integer)
- **Output:**
  - `Return Value` (Boolean): `true` se `FoundIndex >= 0`, `false` se `FoundIndex == -1`
- **Criar `Branch`**:
  - Botão direito → digite "Branch"
  - Selecione `Branch` (categoria "Flow Control")
- **Conectar:**
  - Input `Condition`: Conecte o `Return Value` (Boolean) da comparação acima
  - Pin "True": será usado se o Actor foi encontrado
  - Pin "False": será usado se o Actor não foi encontrado

---

**PASSO 6.4A: Se encontrado (Branch True) - Obter o Actor**

- **Do pin "True" do `Branch`**, conecte ao pin de execução de um `Get Element`
- **Criar `Get Element`**:
  - Botão direito → digite "Get Element" ou "Array Get"
  - Selecione `Get Element` (categoria "Array")
  - Ou: Arraste `Get RemoteActors` no gráfico → procure "Get Element"
- **Inputs do `Get Element`:**
  - `Array`: Conecte `Get RemoteActors` → `Return Value` (Array of Actor/Object Reference)
    - **Como obter**: Arraste a variável `RemoteActors` no gráfico → `Get RemoteActors`
    - Conecte o `Return Value` ao input `Array` do `Get Element`
  - `Index`: Conecte o `FoundIndex` (Integer) do passo 6.2
    - **IMPORTANTE**: Use o mesmo `FoundIndex` usado na busca!
- **Output:**
  - `Element`: `RemoteActorRef` (Actor Reference) - **ESTE É O ACTOR REMOTO**
  - **GUARDE ESTE RESULTADO** - será usado no passo 7
- **Pin de execução**: Conecte ao passo 7 (Aplicar Transformação)

---

**PASSO 6.4B: Se não encontrado (Branch False) - Decidir o que fazer**

Você tem duas opções quando o Actor não é encontrado (`FoundIndex == -1`):

### OPÇÃO 1: Pular para o próximo item (recomendado se Actors são spawnados em outro lugar)

- **Criar `Continue Loop`**:
  - Botão direito → digite "Continue Loop"
  - Selecione `Continue Loop` (categoria "Flow Control")
  - **ATENÇÃO**: O `Continue Loop` só funciona dentro de um loop (`For Each Loop`)
- **Conectar:**
  - Do pin "False" do `Branch`, conecte ao pin de execução do `Continue Loop`
  - Isso fará o loop pular para o próximo elemento do `RemoteStates`
  - **NOTA**: O Actor será spawnado automaticamente em outro momento (ex.: quando receber o primeiro StateUpdate)

### OPÇÃO 2: Spawn um novo Actor remoto aqui (se você quer spawnar no Tick)

**Se você escolher esta opção, faça:**

**6.4B.1) Spawn do Actor:**
- **Criar `Spawn Actor from Class`**:
  - Botão direito → digite "Spawn Actor" ou "Spawn"
  - Selecione `Spawn Actor from Class` (categoria "Actor")
- **Inputs:**
  - `Class`: Selecione a classe do Actor remoto (ex.: seu Player Pawn class ou uma classe específica para players remotos)
    - **NOTA**: Se você tem uma classe Blueprint para players remotos, arraste ela no gráfico ou procure pelo nome
  - `Transform`: Use `Make Transform`
    - `Location`: Você pode usar uma posição padrão (ex.: `0,0,0`) ou a posição interpolada atual
    - `Rotation`: Você pode usar `0,0,0` inicialmente
    - `Scale`: `1,1,1`
  - `World Context Object`: Conecte `Get World` ou deixe vazio (usa o contexto atual)
- **Output:**
  - `Return Value`: `NewActorRef` (Actor Reference) - o Actor recém-spawnado

**6.4B.2) Adicionar ao Array `RemoteActorIds`:**
- **Criar `Add Item to Array`**:
  - Botão direito → digite "Add" ou "Add Item"
  - Selecione `Add Item to Array` (categoria "Array")
- **Inputs:**
  - `Array`: Conecte `Get RemoteActorIds` → `Return Value`
  - `Item`: Conecte o `PlayerId` (do passo 6.1)
- **Output:**
  - `Return Value`: O novo Array (com o PlayerId adicionado)
- **IMPORTANTE**: Conecte este `Return Value` de volta a `Set RemoteActorIds` se necessário, OU simplesmente o `Add Item` já modifica o Array automaticamente (se for `UPARAM(ref)`)

**6.4B.3) Adicionar ao Array `RemoteActors`:**
- **Criar `Add Item to Array`**:
  - `Array`: Conecte `Get RemoteActors` → `Return Value`
  - `Item`: Conecte o `NewActorRef` (do passo 6.4B.1)
- **IMPORTANTE**: Faça isso na **MESMA ORDEM** que adicionou ao `RemoteActorIds`:
  - Primeiro adicione `PlayerId` ao `RemoteActorIds`
  - Depois adicione `Actor` ao `RemoteActors`
  - Isso mantém os Arrays sincronizados (mesmo índice = mesmo jogador)

**6.4B.4) Usar o Actor spawnado:**
- **Após spawnar e adicionar aos Arrays**, você pode usar o `NewActorRef` diretamente no passo 7
- Ou pode fazer `Find Item in Array` novamente para obter o índice e usar `Get Element`, mas não é necessário se você já tem `NewActorRef`

**Fluxo visual da OPÇÃO 2 (Spawn):**
```
Branch False (não encontrado)
    ↓
Spawn Actor from Class
    ↓ (NewActorRef)
Add Item to Array (RemoteActorIds, PlayerId)
    ↓
Add Item to Array (RemoteActors, NewActorRef)
    ↓
[Continue para passo 7 usando NewActorRef]
```

**RECOMENDAÇÃO**: Use a **OPÇÃO 1** (`Continue Loop`) se você prefere spawnar Actors em outro lugar (ex.: no `OnWSBinaryMessage` quando receber o primeiro StateUpdate). Isso mantém o `Tick` mais leve.

---

**PASSO 6.5: Resumo Visual Completo**

```
[Do passo 5 - após interpolar Yaw]
    ↓
Obter PlayerId: ArrayElement.PlayerId (via Knot se necessário)
    ↓
Get RemoteActorIds → Find Item in Array
    - Array: RemoteActorIds
    - Item: PlayerId
    ↓ (FoundIndex: Integer)
Greater or Equal (FoundIndex, 0) → Branch
    ↓
Branch
    ├─ True (FoundIndex >= 0): Actor existe
    │   ↓
    │   Get RemoteActors → Get Element
    │   - Array: RemoteActors
    │   - Index: FoundIndex
    │   ↓ (RemoteActorRef)
    │   [Continue para passo 7]
    │
    └─ False (FoundIndex == -1): Actor não existe
        ↓
        [OPÇÃO 1: Continue Loop] → pula para próximo item
        OU
        [OPÇÃO 2: Spawn Actor]
            ↓
            Spawn Actor from Class → NewActorRef
            ↓
            Add Item (RemoteActorIds, PlayerId)
            ↓
            Add Item (RemoteActors, NewActorRef)
            ↓
            [Continue para passo 7 usando NewActorRef]
```

---

**NOTAS IMPORTANTES:**

1. **Sincronização dos Arrays**:
   - **SEMPRE** mantenha `RemoteActorIds` e `RemoteActors` sincronizados
   - Se adicionar em `RemoteActorIds` no índice `N`, adicione em `RemoteActors` no mesmo índice `N`
   - Se remover de um Array, remova do outro também no mesmo índice

2. **Performance**:
   - `Find Item in Array` faz uma busca linear (O(n))
   - Se você tiver muitos jogadores remotos, considere usar uma estrutura mais eficiente
   - Para poucos jogadores (< 10), o Array está OK

3. **Validação adicional (opcional)**:
   - Após obter o Actor com `Get Element`, você pode verificar se ele ainda é válido
   - Use `Is Valid` antes de usar no passo 7 (veja passo 7.1)

4. **Quando spawnar Actors remotos**:
   - **Recomendação**: Spawn no `OnWSBinaryMessage` quando receber o primeiro StateUpdate de um novo jogador
   - Isso evita spawnar Actors para jogadores que podem nunca aparecer
   - Mantém o `Tick` mais leve

**7) Aplicar Transformação - PASSO A PASSO**

**7.1) Verificar se o Actor é válido:**
- **IMPORTANTE**: Sempre verifique se o Actor não é `None` antes de modificar
- `Branch`:
  - Input `Condition`: `Is Valid` (procure "Is Valid" ou arraste o `RemoteActorRef` → "Is Valid")
  - Input `Object`: `RemoteActorRef`
  - Se `Is Valid` retorna `false`, conecte o pin "False" ao `Continue Loop`
  - Se `true`, continue para o próximo passo

**7.2) Aplicar Posição:**
- `Set Actor Location` (procure "Set Actor Location" ou arraste o `RemoteActorRef` → "Set Actor Location")
- Inputs:
  - `Target`: `RemoteActorRef` (Actor)
  - `New Location`: `InterpolatedLocation` (Vector, do passo 4)
  - `bSweep`: deixe `false` (ou `true` se quiser detecção de colisão)

**7.3) Criar Rotator:**
- `Make Rotator` (categoria "Math" → "Rotator")
- Inputs:
  - `Roll`: `0.0` (constante Float)
  - `Pitch`: `0.0` (constante Float)
  - `Yaw`: `InterpolatedYaw` (Float, do passo 5)
- Output: `NewRotation` (Rotator)

**7.4) Aplicar Rotação:**
- `Set Actor Rotation` (procure "Set Actor Rotation" ou arraste o `RemoteActorRef` → "Set Actor Rotation")
- Inputs:
  - `Target`: `RemoteActorRef` (Actor)
  - `New Rotation`: `NewRotation` (Rotator, do passo 7.3)

**8) O Loop Continua Automaticamente**

Após processar um jogador, o `For Each Loop` automaticamente passa para o próximo elemento do Array `RemoteStates`. Não é necessário conectar nada explicitamente ao pin `Completed` do loop.

**Fluxo Visual Completo (com todos os nós)**:
```
Event Tick
    ↓
Get RemoteStates (Array of Player State Entry)
    ↓
For Each Loop
    - Array: RemoteStates
    ↓ (Loop Body - para cada elemento)
ArrayElement (Player State Entry)
    ↓
Branch: HasStateA AND HasStateB?
    ↓ (True = temos dados suficientes)
Get Game Time in Seconds * 1000 → To Integer → CurrentTimeMs
    ↓
Calculate DeltaMs: StateB_TimestampMs - StateA_TimestampMs
    ↓
Calculate ElapsedMs: CurrentTimeMs - StateA_TimestampMs
    ↓
Alpha = (ElapsedMs as Float) / (DeltaMs as Float)
    ↓
Clamp Alpha (0.0 to 1.0) → ClampedAlpha
    ↓
Lerp (Vector): StateA_Location → StateB_Location (Alpha: ClampedAlpha) → InterpolatedLocation
    ↓
Lerp (Float): StateA_Yaw → StateB_Yaw (Alpha: ClampedAlpha) → InterpolatedYaw
    ↓
Find Item in Array (RemoteActorIds, ArrayElement.PlayerId) → FoundIndex
    ↓
Branch: FoundIndex >= 0?
    ↓ (True = Actor existe)
Get Element (RemoteActors, Index: FoundIndex) → RemoteActorRef
    ↓
Is Valid (RemoteActorRef)?
    ↓ (True = Actor válido)
Set Actor Location (Target: RemoteActorRef, Location: InterpolatedLocation)
    ↓
Make Rotator (Roll: 0, Pitch: 0, Yaw: InterpolatedYaw) → NewRotation
    ↓
Set Actor Rotation (Target: RemoteActorRef, Rotation: NewRotation)
    ↓ (Loop Body continua para próximo elemento)
    ↓ (Completed - quando todos os elementos foram processados)
[Fim do Tick]
```

**Resumo Rápido dos Passos:**
1. `For Each Loop` no `RemoteStates`
2. Verificar `HasStateA AND HasStateB` → se `false`, `Continue Loop`
3. Calcular `Alpha` = (tempo atual - StateA) / (StateB - StateA), clampleado entre 0.0 e 1.0
4. `Lerp (Vector)` para Location
5. `Lerp (Float)` para Yaw
6. `Find Item in Array` para buscar Actor remoto
7. Se encontrado e válido: `Set Actor Location` e `Set Actor Rotation`
8. Loop continua automaticamente

**NOTAS IMPORTANTES:**
- **Performance**: Este código roda a cada frame. Se você tiver muitos jogadores remotos, considere:
  - Processar apenas um jogador por frame (round-robin)
  - Usar `Set Timer` em vez de `Tick` (ex.: a cada 0.05s)
- **Proteção contra divisão por zero**: Se `DeltaMs <= 0`, adicione uma verificação antes do passo 3.4:
  - `Branch`: `DeltaMs > 0?`
  - Se `false`, `Continue Loop` (dados inválidos)
- **Spawn de Actors remotos**: Se no passo 6 você não encontrar o Actor (`FoundIndex < 0`), você pode:
  - Spawn um novo Actor/Pawn remoto (ex.: clone do seu Player Pawn)
  - Adicionar ao Array `RemoteActorIds` (adicionar `ArrayElement.PlayerId`)
  - Adicionar ao Array `RemoteActors` (adicionar o Actor spawnado)
  - **IMPORTANTE**: Mantenha os Arrays sincronizados (mesmo índice = mesmo jogador)

**Otimizações futuras**:
- Se `Alpha >= 1.0`, você pode fazer "snap" direto para `StateB` (o servidor está muito atrasado)
- Limpar entradas do Array para players que não enviaram updates por muito tempo (> 5 segundos)
- Interpolação de rotação "shortest path" para yaws em direções opostas

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


