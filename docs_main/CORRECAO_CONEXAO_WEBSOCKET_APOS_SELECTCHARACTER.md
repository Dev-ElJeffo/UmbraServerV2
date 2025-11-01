# Guia Ultra-Detalhado: Conexão WebSocket com Explicação de Cada Nó do Blueprint

## 📋 Índice

1. [Visão Geral do Sistema](#visão-geral-do-sistema)
2. [Classes C++ e Seu Uso](#classes-c-e-seu-uso)
3. [Preparação: Criar e Configurar Variáveis](#preparação-criar-e-configurar-variáveis)
4. [FASE 1: Implementar BeginPlay - Passo a Passo Completo](#fase-1-implementar-beginplay)
5. [FASE 2: Implementar OnWSConnected - Detalhamento Total](#fase-2-implementar-onwsconnected)
6. [FASE 3: Implementar SendMoveUpdate - Cada Nó Explicado](#fase-3-implementar-sendmoveupdate)
7. [FASE 4: Implementar OnWSBinaryMessage - Decodificação Detalhada](#fase-4-implementar-onwsbinarymessage)
8. [FASE 5: Implementar Event Tick - Interpolação Completa](#fase-5-implementar-event-tick)
9. [FASE 6: Spawn no Level Blueprint](#fase-6-spawn-no-level-blueprint)
10. [Teste Multi-Cliente](#teste-multi-cliente)

---

## 🎯 Visão Geral do Sistema

### Fluxo Completo

```
SelectCharacter (GameInstance)
    ↓
OnCharacterSelected (Delegate)
    ↓
Level Blueprint: Spawn BP_NetMovementClient
    ↓
BP_NetMovementClient: BeginPlay
    ↓
Criar WebSocket → Conectar → OnWSConnected
    ↓
Timer inicia SendMoveUpdate (20 Hz)
    ↓
OnWSBinaryMessage recebe StateUpdate
    ↓
Atualiza RemoteStates → Event Tick interpola
```

---

## 📚 Classes C++ e Seu Uso

### 1. `UUmbraWSClient`

**Onde encontrar no Blueprint:**
- Categoria: `Umbra|Net|WS`
- Procure por: `Create Umbra Web Socket` (função estática)

**Funções disponíveis:**
- `CreateUmbraWebSocket(URL)` → Retorna `Umbra WS Client` Object Reference
- `Connect()` → Void (sem retorno)
- `Close()` → Void
- `SendBytes(Data)` → Boolean (true = sucesso)

**Delegates (para Bind):**
- `OnConnected` → Dispara quando conecta
- `OnConnectionError` → Dispara em erro (parâmetro: Error String)
- `OnClosed` → Dispara quando fecha
- `OnRawMessage` → Dispara quando recebe dados binários (parâmetro: Data Array)

### 2. `UWSBinaryBPFL` (Blueprint Function Library)

**Onde encontrar no Blueprint:**
- Categoria: `Umbra|Net|WS|Binary` ou `Umbra|Net|WS|State`
- Todas as funções são estáticas (não precisa de instância)

**Funções principais:**
- `BuildMoveUpdateFrame(...)` → `TArray<uint8>` (retorna bytes)
- `ParseStateUpdateFrame(...)` → `Boolean` (true = sucesso, preenche Out parameters)
- `GetOrCreatePlayerState(...)` → `Player State Entry` (retorna cópia!)
- `UpdatePlayerStateBuffer(...)` → Void (modifica Entry via ref)
- `FindPlayerStateIndex(...)` → `Integer` (-1 se não encontrar)

### 3. `UUmbraGameInstance`

**Onde encontrar no Blueprint:**
- Use `Get Game Instance` → `Cast to Umbra Game Instance`

**Funções disponíveis:**
- `HasActiveCharacter()` → Boolean (verifica se há personagem selecionado)
- `GetActivePlayerID()` → Integer (retorna o ID do personagem ativo, **ADICIONADA RECENTEMENTE**)
  - **NOTA**: Requer recompilação do projeto C++ para aparecer no Blueprint
  - **Alternativa**: Use `GetActiveCharacter()` → Break Struct → campo `ID`
- `GetActiveCharacter()` → `Umbra Player Data` (Struct) (retorna dados completos do personagem)
- `GetZoneServerWebSocketURL()` → String (URL do WebSocket ZoneServer)

---

## 🔧 Preparação: Criar e Configurar Variáveis

### Passo 1: Abrir BP_NetMovementClient

1. **No Content Browser**, localize `BP_NetMovementClient`
2. **Duplo clique** para abrir o editor de Blueprint
3. **Verifique se está na aba correta**: Clique em **"Meu Blueprint"** (painel esquerdo)

### Passo 2: Criar Variável `WebSocketRef`

**2.1. Abrir painel de variáveis:**
- No painel **"Meu Blueprint"**, localize a seção **"Variáveis"**
- Clique no botão **"+ Variável"** (canto superior direito da seção)

**2.2. Configurar a variável:**
- **Nome**: Digite `WebSocketRef` (exatamente assim, case-sensitive)
- **Tipo**: Clique no dropdown ao lado de "Tipo"
  - Procure por: **"Object Reference"**
  - Depois procure por: **"Umbra WS Client"** (ou digite "Umbra" na busca)
  - Selecione: **"Umbra WS Client"** (Object Reference)

**2.3. Propriedades da variável:**
- **Instance Editable**: Desmarque (não precisa ser editável no Editor)
- **Private**: Pode deixar marcado (padrão)
- **Tooltip**: Opcional - "Referência ao objeto WebSocket criado"

**2.4. Confirmar:**
- Pressione **Enter** ou clique fora do campo
- A variável aparecerá na lista com o tipo `Umbra WS Client`

**2.5. Valor padrão:**
- Deixe como `None` (será setado no BeginPlay)

### Passo 3: Criar Variável `IsConnected`

**3.1. Criar nova variável:**
- Clique em **"+ Variável"** novamente

**3.2. Configurar:**
- **Nome**: `IsConnected`
- **Tipo**: **"Boolean"** (procure "Bool" ou "Boolean")
- **Instance Editable**: Desmarcado
- **Valor padrão**: `false` (marque a caixa ou deixe desmarcado)

### Passo 4: Criar Variável `MyPlayerId`

**4.1. Criar:**
- **"+ Variável"**

**4.2. Configurar:**
- **Nome**: `MyPlayerId`
- **Tipo**: **"Integer"** (procure "Int" ou "Integer")
- **Instance Editable**: ✅ **MARQUE ESTE** (para poder alterar no Editor se necessário)
- **Valor padrão**: `0`

### Passo 5: Criar Variável `SendRateHz`

**5.1. Criar:**
- **"+ Variável"**

**5.2. Configurar:**
- **Nome**: `SendRateHz`
- **Tipo**: **"Float"** (número decimal)
- **Instance Editable**: ✅ **MARQUE ESTE** (para ajustar no Editor)
- **Valor padrão**: `20.0` (digite 20.0)

### Passo 6: Criar Variável `InterpDelayMs`

**6.1. Criar:**
- **Nome**: `InterpDelayMs`
- **Tipo**: **"Float"**
- **Instance Editable**: ✅ **MARQUE**
- **Valor padrão**: `120.0`

### Passo 7: Criar Variável `RemoteStates`

**7.1. Criar:**
- **"+ Variável"**

**7.2. Configurar Tipo (PASSO CRÍTICO):**
- Clique no dropdown **"Tipo"**
- Selecione **"Array"** (primeira opção na lista)
- **IMPORTANTE**: Após selecionar "Array", aparece um novo campo **"Inner"**
- Clique no campo **"Inner"** (ao lado de "Array")
- Procure por: **"Player State Entry"**
  - Se não aparecer, digite "Player" na busca
  - **O que procurar**: A estrutura deve aparecer como **"Player State Entry"** (Struct)
  - Se não encontrar, verifique se o projeto C++ foi compilado e o Editor reiniciado

**7.3. Confirmar:**
- O tipo deve aparecer como: `Array of Player State Entry`
- **Valor padrão**: Deixe vazio (array vazio)

### Passo 8: Criar Variável `RemoteActorIds`

**8.1. Criar:**
- **"+ Variável"**

**8.2. Configurar:**
- **Nome**: `RemoteActorIds`
- **Tipo**: **"Array"**
- **Inner**: **"Integer"**
- Deve aparecer como: `Array of Integer`
- **Valor padrão**: Array vazio

### Passo 9: Criar Variável `RemoteActors`

**9.1. Criar:**
- **Nome**: `RemoteActors`
- **Tipo**: **"Array"**
- **Inner**: **"Actor Reference"**
  - Procure "Actor" na lista e selecione **"Object Reference"** → depois procure **"Actor"**
  - Ou simplesmente procure **"Actor Reference"** diretamente
- Deve aparecer como: `Array of Actor Reference`
- **Valor padrão**: Array vazio

### Passo 10: Criar Variável `SendMoveUpdateHandle` (Opcional)

**10.1. Criar:**
- **Nome**: `SendMoveUpdateHandle`
- **Tipo**: **"Timer Handle"**
  - Procure "Timer" na lista
  - Selecione **"Timer Handle"**
- **Valor padrão**: Deixe vazio (None)

---

## 🚀 FASE 1: Implementar BeginPlay - Passo a Passo Completo

### Navegar até o Event Graph

1. **No editor de Blueprint**, clique na aba **"Graph"** (ou **"EventGraph"**)
2. **Se já houver um Event BeginPlay**, você verá um nó laranja no gráfico
3. **Se não houver**, clique com botão direito no gráfico → **"Add Event"** → **"BeginPlay"**

---

### PASSO 1.1: Adicionar Nó `Delay`

**Objetivo**: Aguardar 0.1 segundos para garantir que tudo esteja inicializado.

**Como obter o nó:**
1. **Clique com botão direito** no gráfico (fora de qualquer nó)
2. **Digite**: `delay` na busca
3. **Selecione**: **"Delay"** (deve aparecer como um nó com um ícone de relógio)

**Configuração do nó:**
- **Duration**: Clique no campo e digite `0.1`
- **Padrão**: O campo já vem com `0.0`, altere para `0.1`

**Conexão:**
- **No Event BeginPlay**, localize o pino **"then"** (execução, seta branca)
- **Arraste** uma linha do pino **"then"** até o pino **"Exec"** (entrada) do nó Delay
- **Visual**: A linha deve ficar branca/cinza conectando os dois nós

**Saída do Delay:**
- O nó Delay tem um pino **"Completed"** (execução de saída)
- Este será conectado ao próximo passo

---

### PASSO 1.2: Adicionar Nó `Get Game Instance`

**Objetivo**: Obter a Game Instance para acessar dados do jogador.

**Como obter:**
1. **Clique com botão direito** no gráfico
2. **Digite**: `get game instance`
3. **Selecione**: **"Get Game Instance"**

**Configuração:**
- **Não há campos para configurar** - este nó obtém automaticamente a Game Instance

**Conexão de Execução:**
- **Do nó Delay**: Arraste do pino **"Completed"** até o pino **"Exec"** do `Get Game Instance`

**Saída:**
- O nó tem um pino **"Return Value"** (tipo: `Game Instance`)
- Este será usado no próximo passo (Cast)

**Saída de Execução:**
- O nó tem um pino **"then"** (execução)
- Conecte ao próximo passo

---

### PASSO 1.3: Adicionar Nó `Cast to Umbra Game Instance`

**Objetivo**: Converter a Game Instance genérica para `UmbraGameInstance` (para acessar funções específicas).

**Como obter:**
1. **Clique com botão direito** no gráfico
2. **Digite**: `cast to umbra`
3. **Selecione**: **"Cast to Umbra Game Instance"**

**Configuração:**
- **Object**: Conecte o pino **"Return Value"** do `Get Game Instance` ao pino **"Object"** do Cast

**Conexão de Execução:**
- **Do Get Game Instance**: Arraste do pino **"then"** até o pino **"Exec"** do Cast

**Saídas do Cast:**
- **"Casts Failed"**: Se a conversão falhar (não deveria acontecer se o projeto está configurado corretamente)
- **"As Umbra Game Instance"**: Retorna o objeto convertido (use este no próximo passo)
- **"then"**: Execução de saída (sucesso do cast)

**Validação:**
- Se você não encontrar "Cast to Umbra Game Instance", verifique:
  - Se o projeto C++ foi compilado
  - Se a classe `UUmbraGameInstance` está marcada como `UCLASS()` no código
  - Tente reiniciar o Editor

---

### PASSO 1.4: Adicionar Nó `Branch` com `HasActiveCharacter`

**Objetivo**: Verificar se há um personagem selecionado antes de conectar.

**4A. Obter a função `HasActiveCharacter`:**
1. **Do nó Cast**: Localize o pino **"As Umbra Game Instance"** (saída do objeto)
2. **Arraste** uma linha deste pino
3. **Solte** no gráfico (fora de qualquer nó)
4. **No menu de contexto**, digite: `has active character`
5. **Selecione**: **"Has Active Character"** (função pura, sem execução)

**4B. Obter o nó Branch:**
1. **Clique com botão direito** no gráfico
2. **Digite**: `branch`
3. **Selecione**: **"Branch"** (nó de decisão)

**4C. Conectar:**
- **Condição do Branch**: Conecte o pino **"Return Value"** (Boolean) do `HasActiveCharacter` ao pino **"Condition"** do Branch
- **Execução**: Conecte o pino **"then"** do Cast ao pino **"Exec"** do Branch

**Saídas do Branch:**
- **"True"**: Se `HasActiveCharacter` retornar `true` (há personagem)
- **"False"**: Se retornar `false` (não há personagem)

---

### PASSO 1.5: Implementar Branch False (Loop de Espera)

**Objetivo**: Se não há personagem, aguardar e tentar novamente.

**5A. Adicionar Print String:**
1. **Do Branch False**: Clique com botão direito próximo ao pino **"False"**
2. **Digite**: `print string`
3. **Selecione**: **"Print String"**
4. **Configure**: No campo **"In String"**, digite: `⚠️ Nenhum personagem selecionado. Aguardando...`

**5B. Adicionar Delay:**
1. **Clique com botão direito** após o Print String
2. **Digite**: `delay`
3. **Selecione**: **"Delay"**
4. **Configure**: `Duration = 1.0` (1 segundo)

**5C. Criar Loop:**
- **Conecte**: Do pino **"False"** do Branch até o **"Exec"** do Print String
- **Conecte**: Do pino **"then"** do Print String até o **"Exec"** do Delay
- **Conecte**: Do pino **"Completed"** do Delay de volta ao **"Exec"** do `Get Game Instance` (criando um loop)

**Visual do loop:**
```
Get Game Instance → Cast → Branch (False) → Print → Delay → [LOOP de volta para Get Game Instance]
```

---

### PASSO 1.6: Implementar Branch True - Obter Player ID

**⚠️ IMPORTANTE**: Após adicionar a função `GetActivePlayerID()` no código C++, você precisa **recompilar o projeto** para que o nó apareça no Blueprint.

**COMO OBTER O PLAYER ID - DUAS OPÇÕES:**

#### OPÇÃO 1: Usar `Get Active Player ID` (RECOMENDADO - Mais Simples)

**Esta função foi adicionada ao código C++ e estará disponível após recompilar:**

**6A. Adicionar `Get Active Player ID`:**
1. **Do pino "As Umbra Game Instance"** do Cast (do Branch True)
2. **Arraste** uma linha e solte no gráfico (fora de qualquer nó)
3. **No menu de contexto**, digite: `get active player id`
4. **Selecione**: **"Get Active Player ID"**
   - **Categoria**: Deve aparecer como `Character`
   - **Tipo**: Função pura (não precisa de execução)
   - **Return Value**: `Integer` (o ID do personagem)

**6B. Se o nó NÃO APARECER:**
- **Verifique se o projeto C++ foi compilado**: Feche o Editor → Compile no Visual Studio → Reabra o Editor
- **Verifique se o arquivo foi salvo**: `UmbraGameInstance.h` deve ter a função `GetActivePlayerID()`
- **Tente atualizar**: Tools → Refresh Visual Studio Project Files
- **OU use a OPÇÃO 2** abaixo (alternativa que sempre funciona)

**6C. Adicionar `Set MyPlayerId`:**
1. **No painel "Meu Blueprint"** → **"Variáveis"**
2. **Arraste** a variável `MyPlayerId` para o gráfico
3. **No menu de contexto**, selecione **"Set"** (não "Get")
   - Você verá um nó `Set MyPlayerId` com:
     - **Exec** (entrada de execução)
     - **MyPlayerId** (campo Integer para o valor)
     - **then** (saída de execução)

**6D. Conectar:**
- **Valor**: Do pino **"Return Value"** (Integer) do `Get Active Player ID` até o pino **"MyPlayerId"** (entrada) do `Set MyPlayerId`
  - **Como conectar**: Arraste do pino **"Return Value"** do `Get Active Player ID` até o campo **"MyPlayerId"** do `Set MyPlayerId`
- **Execução**: Do pino **"True"** do Branch até o pino **"Exec"** do `Set MyPlayerId`
  - **Como conectar**: Arraste do pino **"True"** (execução) até o pino **"Exec"** (entrada de execução) do Set

**6E. Confirmar Set:**
- O nó `Set MyPlayerId` deve ter:
  - **Entrada Exec**: Conectado ao Branch True ✅
  - **MyPlayerId**: Conectado ao Return Value do Get Active Player ID ✅
  - **Saída Exec (then)**: Para continuar o fluxo (conecte ao próximo passo)

---

#### OPÇÃO 2: Usar `Get Active Character` e Break Struct (ALTERNATIVA)

**Se `Get Active Player ID` não aparecer, use esta opção:**

**6A. Adicionar `Get Active Character`:**
1. **Do pino "As Umbra Game Instance"** do Cast
2. **Arraste** uma linha
3. **Digite**: `get active character`
4. **Selecione**: **"Get Active Character"**
   - **Return Value**: Tipo `Umbra Player Data` (Struct)

**6B. Adicionar `Break Umbra Player Data`:**
1. **Do Return Value** (Umbra Player Data) do `Get Active Character`
2. **Arraste** uma linha
3. **Digite**: `break umbra player data`
4. **Selecione**: **"Break Umbra Player Data"**
   - **Input**: Conecte o **"Return Value"** do `Get Active Character`
   - **Outputs**: Vários campos, incluindo:
     - `ID` (Integer) ← **USE ESTE**
     - `CharacterName` (String)
     - `Level` (Integer)
     - etc.

**6C. Conectar ao Set MyPlayerId:**
- **Do Break Struct**: Conecte o pino **"ID"** (Integer) ao campo **"MyPlayerId"** do Set
- **Execução**: Conecte do Branch True até o Exec do Set

**6D. Validação Opcional:**
- **Após Get Active Character**: Você pode adicionar um **Branch** para verificar se o personagem é válido:
  - **Compare**: Use **"!=" (Integer)** para comparar `ID` com `0`
  - Se `ID != 0`, continue; senão, há erro

---

### RESUMO: Qual Opção Usar?

- **OPÇÃO 1** (`Get Active Player ID`): ✅ **MAIS SIMPLES E DIRETA**
  - Retorna o ID diretamente (Integer)
  - Requer recompilação do projeto C++ após adicionar a função
  
- **OPÇÃO 2** (`Get Active Character` + Break): ✅ **SEMPRE DISPONÍVEL**
  - Não requer código C++ adicional
  - Retorna toda a estrutura (você precisa fazer Break)
  - Mais informações disponíveis (Level, Name, etc.)

**RECOMENDAÇÃO**: Use **OPÇÃO 1** se o nó aparecer após recompilar. Use **OPÇÃO 2** se não aparecer ou enquanto não recompilar.

---

### PASSO 1.7: Obter URL do WebSocket

**7A. Adicionar `Get Zone Server Web Socket URL`:**
1. **Do pino "As Umbra Game Instance"** (do mesmo Cast)
2. **Arraste** uma linha
3. **Digite**: `get zone server web socket url`
4. **Selecione**: **"Get Zone Server Web Socket URL"**

**7B. Adicionar Print String (Opcional - para debug):**
1. **Clique com botão direito** após o Set MyPlayerId
2. **Digite**: `print string`
3. **Configure**: `In String = "🔌 Conectando ao: "`
4. **Adicionar String ao String:**
   - **Digite**: `append string`
   - **Selecione**: **"Append String"**
   - **Conecte**: 
     - `A` = `"🔌 Conectando ao: "`
     - `B` = **Return Value** do `Get Zone Server Web Socket URL`
   - **Saída**: Conecte ao `In String` do Print String

**7C. Conexão de Execução:**
- Do pino **"then"** do `Set MyPlayerId` até o **"Exec"** do Print String (ou pule o Print e vá direto para o próximo passo)

---

### PASSO 1.8: Criar WebSocket usando Classe C++

**8A. Adicionar `Create Umbra Web Socket`:**
1. **Clique com botão direito** no gráfico
2. **Digite**: `create umbra web socket`
3. **Selecione**: **"Create Umbra Web Socket"**
   - **IMPORTANTE**: Esta é uma função **estática** (não precisa de objeto)
   - **Categoria**: Deve aparecer como `Umbra|Net|WS`

**8B. Configurar URL:**
- **Url**: Conecte o pino **"Return Value"** (String) do `Get Zone Server Web Socket URL` ao pino **"Url"** (String) do `Create Umbra Web Socket`

**8C. Conexão de Execução:**
- Do pino **"then"** do Print String (ou do Set MyPlayerId) até o **"Exec"** do `Create Umbra Web Socket`

**Saída:**
- O nó retorna **"Return Value"** (tipo: `Umbra WS Client` Object Reference)

---

### PASSO 1.9: Validar WebSocket Criado

**9A. Adicionar `IsValid`:**
1. **Clique com botão direito** no gráfico
2. **Digite**: `is valid`
3. **Selecione**: **"Is Valid"**
4. **Object**: Conecte o pino **"Return Value"** do `Create Umbra Web Socket` ao pino **"Object"** do `IsValid`

**9B. Adicionar Branch:**
1. **Digite**: `branch`
2. **Selecione**: **"Branch"**
3. **Condition**: Conecte o pino **"Is Valid"** (Boolean) do `IsValid` ao pino **"Condition"** do Branch

**9C. Conexão de Execução:**
- Do pino **"then"** do `Create Umbra Web Socket` até o **"Exec"** do Branch

**9D. Branch False (Falha):**
- **Adicionar Print String**: `"❌ Falha ao criar WebSocket"`
- **Adicionar Return**: Clique com botão direito → `return`
- **Conecte**: False → Print → Return

---

### PASSO 1.10: Setar WebSocketRef

**10A. Arrastar variável `WebSocketRef`:**
1. **No painel "Meu Blueprint"** → **"Variáveis"**
2. **Arraste** `WebSocketRef` para o gráfico
3. **No menu**, selecione **"Set"**

**10B. Conectar:**
- **WebSocketRef**: Conecte o pino **"Return Value"** do `Create Umbra Web Socket` ao pino **"WebSocketRef"** do Set
- **Execução**: Do pino **"True"** do Branch até o **"Exec"** do Set

---

### PASSO 1.11: Bind Event to OnConnected

**11A. Obter o nó Bind:**
1. **Do pino "WebSocketRef"** (saída) do `Set WebSocketRef`
   - **Nota**: Você precisa usar um nó **"Get WebSocketRef"** após o Set para obter a referência
   - **OU**: Arraste a variável `WebSocketRef` novamente e selecione **"Get"**

**11B. Arrastar do Get WebSocketRef:**
1. **Arraste** uma linha do pino **"WebSocketRef"** (saída do Get)
2. **Solte** no gráfico
3. **No menu**, procure por: `bind event`
4. **Selecione**: **"Bind Event to OnConnected"**
   - **IMPORTANTE**: Deve aparecer especificamente "OnConnected", não outros eventos

**11C. Configurar Bind:**
- **Target**: O pino **"WebSocketRef"** já estará conectado automaticamente
- **Event**: Deve mostrar "OnConnected" (delegate)

**11D. Criar Custom Event:**
1. **No painel "Meu Blueprint"** → **"Graph"** → **"Functions"**
2. Clique em **"+ Function"** ou **"Custom Event"**
3. **Nome**: `OnWSConnected`
4. **Tipo**: Selecione **"Custom Event"** (não Function)

**11E. Conectar Custom Event ao Bind:**
- **Do Bind**: O nó `Bind Event to OnConnected` tem um pino **"Event"** (ou similar)
- **Arraste** do pino **"Event"** até o **"Exec"** do Custom Event `OnWSConnected`
- **OU**: No Bind, há um dropdown/menu para selecionar o evento - selecione `OnWSConnected`

**Nota**: A forma exata de conectar depende da versão do Unreal Engine. Geralmente, após criar o Custom Event, ele aparece disponível no dropdown do Bind.

---

### PASSO 1.12: Bind Event to OnRawMessage (CRÍTICO)

**12A. Repetir processo para OnRawMessage:**
1. **Do Get WebSocketRef**, arraste novamente
2. **Digite**: `bind event`
3. **Procure**: **"Bind Event to OnRawMessage"** (não "OnMessage"!)
4. **Selecione**: **"Bind Event to OnRawMessage"**

**12B. Criar Custom Event OnWSBinaryMessage:**
1. **Criar Custom Event**: Nome = `OnWSBinaryMessage`
2. **Adicionar Input**: 
   - No Custom Event, clique em **"Inputs"** (aba do painel)
   - Clique em **"+ Input"**
   - **Nome**: `Data`
   - **Tipo**: `Array` → **Inner**: `Byte` (ou `Integer` de 8 bits)
     - Procure por **"Byte"** ou use **"Integer"** se Byte não existir
     - **OU**: Use **"TArray<uint8>"** se disponível diretamente
   - **Alternativa**: Tipo = `Array of Integer` (funciona, mas menos preciso)

**12C. Conectar:**
- O Bind deve conectar automaticamente ao Custom Event se ele existir
- **Verificar**: O Custom Event deve aparecer no dropdown do Bind

---

### PASSO 1.13: Bind OnConnectionError e OnClosed (Opcional mas Recomendado)

**13A. Bind OnConnectionError:**
- Repita o processo para **"Bind Event to OnConnectionError"**
- Crie Custom Event: `OnWSError`
- **Input**: `Error` (tipo: `String`)

**13B. Bind OnClosed:**
- Repita para **"Bind Event to OnClosed"**
- Crie Custom Event: `OnWSClosed`
- Sem inputs

---

### PASSO 1.14: Conectar ao Servidor

**14A. Obter `Connect`:**
1. **Do Get WebSocketRef**, arraste
2. **Digite**: `connect`
3. **Selecione**: **"Connect"** (deve aparecer como função de `Umbra WS Client`)

**14B. Conexão:**
- **Execução**: Do pino **"then"** do último Bind (ou do Set WebSocketRef) até o **"Exec"** do Connect
- **Target**: O `Connect` deve usar automaticamente o `WebSocketRef` do contexto

**14C. Print String Final:**
- Adicione `Print String`: `"🔌 Tentando conectar..."`

---

## 🔗 FASE 2: Implementar OnWSConnected - Detalhamento Total

### Abrir o Custom Event OnWSConnected

1. **No gráfico**, localize o nó **"OnWSConnected"** (Custom Event, cor roxa)
2. **Se não aparecer**, vá em **"Meu Blueprint"** → **"Graph"** → **"Custom Events"** → Clique em `OnWSConnected`

---

### PASSO 2.1: Setar IsConnected

**1A. Arrastar variável `IsConnected`:**
- **"Meu Blueprint"** → **"Variáveis"** → Arraste `IsConnected` → Selecione **"Set"**

**1B. Configurar:**
- **IsConnected**: **Marque a caixa** (ou use um nó **"Make Bool"** com valor `true`)
- **Execução**: Do pino **"Exec"** do `OnWSConnected` até o **"Exec"** do Set

**Alternativa - Usar Literal Bool:**
1. **Clique com botão direito** no campo `IsConnected` do Set
2. **Digite**: `make bool` ou procure por **"Make Bool"**
3. **Selecione**: **"Make Bool"**
4. **Value**: `true` (marque a caixa ou digite `true`)

---

### PASSO 2.2: Print String de Sucesso

**2A. Adicionar Print String:**
- `"✅ WebSocket Connected!"`
- Conecte após o Set IsConnected

---

### PASSO 2.3: Calcular Intervalo do Timer

**3A. Obter `Get SendRateHz`:**
1. **Arraste variável** `SendRateHz` → Selecione **"Get"**
2. Ou clique com botão direito → `get send rate hz`

**3B. Adicionar `Divide`:**
1. **Clique com botão direito** → `divide`
2. **Selecione**: **"Divide (Float)"** (não Integer!)
3. **Configure**:
   - **A**: `1.0` (literal float)
     - **Como criar literal**: Clique no campo → Digite `1.0`
     - Ou arraste um nó **"Make Float"** → Value = `1.0`
   - **B**: Conecte o pino **"SendRateHz"** (Float) do Get

**3C. Resultado:**
- **Return Value**: Este será o intervalo em segundos (ex: `1.0 / 20.0 = 0.05`)

---

### PASSO 2.4: Setar Timer

**4A. Adicionar `Set Timer by Function Name`:**
1. **Clique com botão direito** → `set timer`
2. **Selecione**: **"Set Timer by Function Name"**

**4B. Configurar campos:**
- **Function Name**: Digite `SendMoveUpdate` (nome da função que será criada)
- **Time**: Conecte o **"Return Value"** (Float) do Divide
- **Looping**: **Marque a caixa** (timer repetirá)
- **First Delay**: Pode deixar `0.0` ou conectar o mesmo valor do `Time`

**4C. Criar Function SendMoveUpdate:**
1. **"Meu Blueprint"** → **"Functions"** → **"+ Function"**
2. **Nome**: `SendMoveUpdate`
3. **Tipo**: Selecione **"Function"** (não Custom Event)
4. **Por enquanto, deixe vazio** - implementaremos depois

**4D. Conexão:**
- **Execução**: Do Print String até o **"Exec"** do Set Timer

---

### PASSO 2.5: Salvar Timer Handle (Opcional)

**5A. Se criou variável `SendMoveUpdateHandle`:**
- Arraste `SendMoveUpdateHandle` → **"Set"**
- **Valor**: Conecte o pino **"Return Value"** (Timer Handle) do Set Timer
- **Execução**: Após o Set Timer

---

### PASSO 2.6: Print String Final

- `"📤 Timer iniciado para SendMoveUpdate"`
- Conecte após tudo

---

## 📤 FASE 3: Implementar SendMoveUpdate - Cada Nó Explicado

### ⚠️ CONTEXTO CRÍTICO: Onde Adicionar Estes Passos

**🎯 OS PASSOS 3.1, 3.2, 3.3, etc. DEVEM SER ADICIONADOS DENTRO DA FUNCTION `SendMoveUpdate`:**

**PASSO A PASSO PARA ENCONTRAR O LOCAL CORRETO:**

1. **Abra o Blueprint `BP_NetMovementClient`**
   - Procure no **Content Browser** por `BP_NetMovementClient`
   - **Clique duas vezes** para abrir

2. **No painel "Meu Blueprint"** (lado esquerdo do Editor)
   - Veja as abas: **"Variables"**, **"Functions"**, **"Macros"**, etc.
   - Clique na aba **"Functions"**

3. **Localize a função `SendMoveUpdate`**
   - Deve aparecer na lista de funções
   - **Se não encontrar**, você precisa criar (veja Fase 2, Passo 2.4)

4. **Clique na função `SendMoveUpdate`**
   - O gráfico da função abrirá no painel principal
   - Deve ter um nó **"Exec"** (entrada de execução) no topo

5. **É AQUI que você adicionará todos os passos 3.1, 3.2, 3.3, etc.**
   - Todos os nós serão conectados ao nó **"Exec"** inicial
   - O fluxo começará do **"Exec"** e seguirá pelos passos

**Visualização do Fluxo:**
```
Event BeginPlay
  └─> OnWSConnected (Custom Event)
       └─> Set IsConnected = true
            └─> Set Timer by Function Name → "SendMoveUpdate" (a cada 0.05s)
                 
                 [TIMER REPETE] ──> SendMoveUpdate (Function) ← VOCÊ ESTÁ AQUI
                                      └─> Passo 3.1: Verificar Conexão
                                           └─> Passo 3.2: Get Player Controller
                                                └─> Passo 3.3: Get Pawn
                                                     └─> ... (continua)
```

---

### PASSO 3.1: Verificar Conexão

**📍 ONDE ADICIONAR:** Dentro da função `SendMoveUpdate`, logo após o pino **"Exec"** inicial.

**1A. Obter variável `IsConnected`:**
1. **No painel "Meu Blueprint"** → **"Variáveis"**
2. **Arraste** a variável `IsConnected` para o gráfico
3. **No menu de contexto**, selecione **"Get"** (não "Set"!)

**1B. Adicionar Branch:**
1. **Do pino "Return Value"** (Boolean) do `Get IsConnected`
2. **Arraste** uma linha para o gráfico (fora de qualquer nó)
3. **No menu de contexto**, digite: `branch`
4. **Selecione**: **"Branch"** (nó de decisão)
5. **Conecte**: O pino **"Return Value"** (Boolean) do `Get IsConnected` até o pino **"Condition"** do Branch

**1C. Conectar Execução:**
- **Do pino "Exec"** da Function `SendMoveUpdate` (parte superior do gráfico) até o pino **"Exec"** do Branch

**1D. Implementar Branch False:**
1. **Do pino "False"** (execução) do Branch, arraste
2. **Digite**: `print string`
3. **Selecione**: **"Print String"**
4. **In String**: Digite `"⚠️ WebSocket não conectado"` (ou marque a caixa e digite)

5. **Após Print String**, adicione **Return**:
   - **Do pino "then"** (execução) do Print String, arraste
   - **Digite**: `return`
   - **Selecione**: **"Return Node"** ou **"Return"**
   - **Este Return termina a função** (não continua se não estiver conectado)

**1E. Validar:**
- O fluxo deve ser:
  ```
  [Exec] → Branch (Condition: IsConnected)
           ├─> True → [continua no próximo passo]
           └─> False → Print String → Return
  ```

---

### PASSO 3.2: Obter Player Controller

**📍 ONDE ADICIONAR:** Após o **Branch True** (quando `IsConnected == true`).

**🎯 MÉTODO RECOMENDADO - Função Helper (APÓS RECOMPILAR):**

**MÉTODO 1 - Usar Função Helper do WS Binary BPFL (MAIS CONFIÁVEL):**

**⚠️ IMPORTANTE:** Esta função foi adicionada ao código C++. Após recompilar o projeto, ela estará disponível.

1. **Clique com botão direito** no gráfico
2. **Digite**: `get first player controller helper`
3. **Selecione**: **"Get First Player Controller Helper"** (deve aparecer como função de `WS Binary BPFL`)
4. **Características do Nó:**
   - **Tipo**: Função **"Pure"** (sem pinos de execução - não precisa de fluxo de execução)
   - **Parâmetros**:
     - **World Context Object**: 
       - Pode estar **oculto** (automaticamente conectado ao contexto do Blueprint)
       - OU pode aparecer como pino de entrada
       - Se aparecer: Conecte o nó **"Self"** (Clique com botão direito → `self`)
   - **Return Value**: Tipo `Player Controller` (objeto) - pino de saída azul
5. **Como usar:**
   - **Você NÃO precisa conectar um pino de execução** (não há pinos de execução neste nó)
   - **Apenas conecte o pino "Return Value"** ao próximo nó ou variável

**MÉTODO 2 - Get World → Get Gameplay Statics (ALTERNATIVA CONFIÁVEL):**

1. **Adicionar `Get World`**:
   - Clique com botão direito → `get world`
   - **Selecione**: **"Get World"**
   - **Target**: Use **"Self"** (se estiver em um Actor) ou deixe vazio

2. **Do Return Value** (World) do `Get World`, arraste
3. **Digite**: `get player controller`
4. **Selecione**: **"Get Player Controller"** (deve aparecer como função de `Gameplay Statics` ou similar)
5. **Parâmetros**:
   - **World Context Object**: Conecte o Return Value do `Get World`
   - **Player Index**: `0` (zero - literal Integer)
6. **Return Value**: Tipo `Player Controller`

**MÉTODO 3 - Busca Direta (se os métodos acima não funcionarem):**
1. **Clique com botão direito** no gráfico
2. **Digite**: `get first player controller`
3. **Selecione**: **"Get First Player Controller"** (se aparecer)

**MÉTODO 4 - Usar Self → Get Player Controller (se BP herda de Actor):**
1. **Self**: Clique com botão direito → `self`
2. **Do Self**, arraste → `get player controller`
3. **Se aparecer**, use este método

**2A. Configurar o Nó:**
- O nó `Get First Player Controller Helper` deve ter:
  - **Sem pinos de execução** (é uma função "Pure" - pode ser usada em qualquer lugar do gráfico)
  - **World Context Object**: Pode estar oculto (conectado automaticamente) ou aparecer como pino de entrada
  - **Return Value**: Tipo `Player Controller` (objeto) - pino de saída azul
- **Como conectar:**
  - **Não precisa de execução**: Este nó é "Pure", então você pode usá-lo diretamente
  - **Do Return Value**: Conecte diretamente ao próximo nó (ex: `IsValid` ou `Get Pawn`)

**2B. Adicionar IsValid:**
1. **Do pino "Return Value"** (Player Controller) do `Get First Player Controller`
2. **Arraste** uma linha
3. **Digite**: `is valid`
4. **Selecione**: **"IsValid"**
5. **Target**: O pino já deve estar conectado automaticamente ao Return Value

**2C. Adicionar Branch para IsValid:**
1. **Do pino "Return Value"** (Boolean) do `IsValid`, arraste
2. **Digite**: `branch`
3. **Selecione**: **"Branch"**
4. **Conecte**: Return Value do IsValid até Condition do Branch

**2D. Conectar Execução:**
- **Do pino "True"** do Branch anterior (IsConnected) até o pino **"Exec"** deste novo Branch (IsValid)

**2E. Implementar Branch False (IsValid = false):**
1. **Do pino "False"** deste Branch, arraste
2. **Print String**: `"⚠️ PlayerController não encontrado"`
3. **Após Print**, adicione **Return** (termina a função)

**2F. Validar:**
- O fluxo deve ser:
  ```
  [Exec] → Branch (IsConnected)
           └─> True → Get First Player Controller
                      └─> IsValid
                           └─> Branch (IsValid)
                                ├─> True → [continua próximo passo]
                                └─> False → Print → Return
  ```

### PASSO 3.3: Obter Pawn (CONTINUAÇÃO DO PASSO 3.2)

**📍 ONDE ADICIONAR:** Após o **Branch True** do IsValid do Player Controller (quando o Player Controller é válido).

**🎯 MÉTODO RECOMENDADO - Função Helper (APÓS RECOMPILAR):**

**MÉTODO 1 - Usar Função Helper do WS Binary BPFL (MAIS CONFIÁVEL):**

**⚠️ IMPORTANTE:** Esta função foi adicionada ao código C++. Após recompilar o projeto, ela estará disponível.

1. **Clique com botão direito** no gráfico
2. **Digite**: `get first player pawn helper`
3. **Selecione**: **"Get First Player Pawn Helper"** (deve aparecer como função de `WS Binary BPFL`)
4. **Características do Nó:**
   - **Tipo**: Função **"Pure"** (sem pinos de execução - não precisa de fluxo de execução)
   - **Parâmetros**:
     - **World Context Object**: 
       - Pode estar **oculto** (automaticamente conectado ao contexto do Blueprint)
       - OU pode aparecer como pino de entrada
       - Se aparecer: Conecte o nó **"Self"** (Clique com botão direito → `self`)
   - **Return Value**: Tipo `Pawn` (objeto) - este é o personagem do jogador - pino de saída azul
5. **Como usar:**
   - **Você NÃO precisa conectar um pino de execução** (não há pinos de execução neste nó)
   - **Apenas conecte o pino "Return Value"** ao próximo nó ou variável

**MÉTODO 2 - Atalho: Usar Get First Player Transform (RECOMENDADO - PULA PASSO 3.4):**

**Esta função retorna Location e Yaw diretamente, pulando os passos intermediários!**

1. **Clique com botão direito** no gráfico
2. **Digite**: `get first player transform`
3. **Selecione**: **"Get First Player Transform"** (função de `WS Binary BPFL`)
4. **Características do Nó:**
   - **Tipo**: Função **"Pure"** (sem pinos de execução - não precisa de fluxo de execução)
   - **Parâmetros**:
     - **World Context Object**: 
       - Pode estar **oculto** (automaticamente conectado ao contexto do Blueprint)
       - OU pode aparecer como pino de entrada
       - Se aparecer: Conecte o nó **"Self"**
   - **Return Value**: `Boolean` (true se encontrou o player)
   - **Outputs**:
     - **Out Location**: Tipo `Vector` - posição do jogador (USE ESTE)
     - **Out Yaw**: Tipo `Float` - rotação Yaw em graus (USE ESTE)
5. **Como usar:**
   - **Você NÃO precisa conectar um pino de execução** (não há pinos de execução neste nó)
   - **Conecte os outputs** (`Out Location` e `Out Yaw`) diretamente aos parâmetros do próximo nó

**✅ Se usar o MÉTODO 2**, você pode **PULAR** o Passo 3.4 (Location e Rotation) e ir direto para o Passo 3.5 (Timestamp)!

**MÉTODO 3 - Do Player Controller (se conseguiu obter o PC):**
1. **Do pino "Return Value"** (Player Controller) do passo 3.2
   - Use o mesmo Return Value que passou pelo IsValid

2. **Arraste** uma linha para o gráfico
3. **Digite**: `get pawn`
4. **Selecione**: **"Get Pawn"** (função do Player Controller)
5. **Target**: Deve conectar automaticamente, ou conecte manualmente ao Return Value do Player Controller

**MÉTODO 4 - Busca Alternativa:**
1. **Clique com botão direito** → `pawn`
2. **Procure** por: **"Get Pawn"** ou **"K2_GetPawn"**

**3A. Configurar Get Pawn (se usou MÉTODO 1 ou 3):**
- O nó `Get Pawn` deve ter:
  - **Target**: Conecte ao pino **"Return Value"** (Player Controller) do `Get First Player Controller`
    - **COMO CONECTAR**: Arraste do pino **"Target"** do Get Pawn até o pino **"Return Value"** do Get First Player Controller
  - **Return Value**: Tipo `Pawn` (objeto) - este é o personagem do jogador

**3B. Adicionar IsValid do Pawn:**
1. **Do pino "Return Value"** (Pawn) do `Get Pawn`
2. **Arraste** → `is valid`
3. **Selecione**: **"IsValid"**
4. **Target**: Deve conectar automaticamente

**3C. Adicionar Branch para IsValid:**
1. **IsValid Return Value** (Boolean) → Arraste → `branch`
2. **Conecte**: Return Value até Condition do Branch

**3D. Conectar Execução:**
- **Do pino "True"** do Branch anterior (IsValid Player Controller) até o pino **"Exec"** deste Branch (IsValid Pawn)

**3E. Branch False (Pawn inválido):**
1. **False** → **Print String**: `"⚠️ Pawn não encontrado"`
2. **Após Print** → **Return**

**3F. Validar:**
- O fluxo completo até aqui:
  ```
  [Exec] → Branch (IsConnected)
           └─> True → Get First Player Controller
                      └─> IsValid (PC)
                           └─> Branch (IsValid PC)
                                └─> True → Get Pawn
                                     └─> IsValid (Pawn)
                                          └─> Branch (IsValid Pawn)
                                               ├─> True → [continua próximo passo]
                                               └─> False → Print → Return
  ```

---

### PASSO 3.4: Obter Location e Rotation

**📍 ONDE ADICIONAR:** Após o **Branch True** do IsValid do Pawn (quando o Pawn é válido).

**4A. `Get Actor Location`:**
1. **Do pino "Return Value"** (Pawn) do `Get Pawn` (do passo 3.3)
2. **Arraste** uma linha para o gráfico
3. **Digite**: `get actor location`
4. **Selecione**: **"Get Actor Location"**
5. **Target**: O pino já deve estar conectado automaticamente ao Return Value do Pawn
6. **Return Value**: Tipo `Vector` (X, Y, Z) - esta é a posição do jogador

**4B. `Get Actor Rotation`:**
1. **Do mesmo pino "Return Value"** (Pawn) do `Get Pawn`
2. **Arraste** uma linha
3. **Digite**: `get actor rotation`
4. **Selecione**: **"Get Actor Rotation"**
5. **Return Value**: Tipo `Rotator` (Pitch, Yaw, Roll) - esta é a rotação do jogador

**4C. `Break Rotator`:**
1. **Do pino "Return Value"** (Rotator) do `Get Actor Rotation`
2. **Arraste** uma linha
3. **Digite**: `break rotator`
4. **Selecione**: **"Break Rotator"**
5. **Input**: O pino já deve estar conectado automaticamente
6. **Outputs**: Você verá três pinos:
   - `Roll` (Float)
   - `Pitch` (Float)
   - `Yaw` (Float) ← **USE ESTE!**
7. **Conecte o pino "Yaw"** - este é o valor de rotação horizontal que precisamos

**4D. Validar:**
- Você deve ter agora:
  - Um **Vector** (Location) do `Get Actor Location`
  - Um **Float** (Yaw) do `Break Rotator`
  - Estes serão usados no próximo passo para construir o frame

---

**⚠️ ATENÇÃO: Se você usou o MÉTODO 2 do Passo 3.3 (`Get First Player Transform`)**, você já tem Location e Yaw!
- **PULE** o Passo 3.4
- **Continue** no Passo 3.5 (Timestamp)
- Use `Out Location` e `Out Yaw` da função `Get First Player Transform` em vez dos valores do Passo 3.4

---

### PASSO 3.5: Calcular Timestamp

**5A. `Get Game Time in Milliseconds`:**
1. **Clique com botão direito** → `get game time`
2. **Procure**: **"Get Game Time in Milliseconds"** (se disponível)
   - **OU**: Use **"Get Game Time in Seconds"** e multiplique por 1000

**5B. Se usar "Get Game Time in Seconds":**
1. **Get Game Time in Seconds** → Return Value (Float)
2. **Multiply (Float)**:
   - **A**: Return Value do Get Game Time
   - **B**: `1000.0` (literal)
3. **Convert Float to Integer** (se necessário):
   - **Digite**: `convert float to integer`
   - **OU**: Use **"FTrunc"** ou **"Round"** dependendo da versão

**5C. Resultado:**
- Valor em milissegundos (Integer) para usar no frame

---

### PASSO 3.6: Construir Frame Binário

**6A. Adicionar `BuildMoveUpdateFrame`:**
1. **Clique com botão direito** → `build move update frame`
2. **Selecione**: **"Build MoveUpdateFrame"** (deve aparecer como função de `WS Binary BPFL`)

**6B. Configurar parâmetros:**
- **PlayerId**: Conecte **Get MyPlayerId** (variável) → **"Get"**
- **Location**: Conecte o **Return Value** (Vector) do `Get Actor Location`
- **YawDegrees**: Conecte o pino **"Yaw"** (Float) do `Break Rotator`
- **TimestampMs**: Conecte o valor calculado no Passo 3.5 (Integer)

**6C. Saída:**
- **Return Value**: Tipo `Array of Byte` (ou `Array of Integer`) - este é o frame pronto

---

### PASSO 3.7: Enviar Frame

**7A. Adicionar `Get WebSocketRef`:**
- Arraste variável `WebSocketRef` → **"Get"**

**7B. Adicionar `SendBytes`:**
1. **Do Get WebSocketRef**, arraste
2. **Digite**: `send bytes`
3. **Selecione**: **"SendBytes"**

**7C. Configurar:**
- **Data**: Conecte o **"Return Value"** (Array) do `BuildMoveUpdateFrame` ao pino **"Data"** do SendBytes

**7D. Validar Retorno:**
- **IsValid** do Get WebSocketRef antes de chamar SendBytes
- **Branch** com o resultado do SendBytes (Boolean)
- **False**: Print de erro

---

## 📥 FASE 4: Implementar OnWSBinaryMessage - Decodificação Detalhada

### Abrir Custom Event OnWSBinaryMessage

1. Localize o nó **"OnWSBinaryMessage"** (Custom Event)
2. **Input**: Deve ter um pino **"Data"** (Array)

---

### PASSO 4.1: Decodificar Frame

**1A. Adicionar `ParseStateUpdateFrame`:**
1. **Clique com botão direito** → `parse state update frame`
2. **Selecione**: **"ParseStateUpdateFrame"**

**1B. Criar variáveis locais para Outputs:**
- **Nota**: Esta função tem **outputs** que precisam ser conectados a variáveis ou "promoted to variables"
- **OutPlayerId**: 
  - **Opção 1**: Clique com botão direito no pino **"OutPlayerId"** → **"Promote to Variable"** → Nome: `OutPlayerId`
  - **Opção 2**: Crie variável local na Function: Tipo `Integer`, Nome `OutPlayerId`
- **OutLocation**: Promova ou crie variável `Vector`
- **OutYawDegrees**: Promova ou crie variável `Float`
- **OutTimestampMs**: Promova ou crie variável `Integer`

**1C. Configurar inputs:**
- **Data**: Conecte o pino **"Data"** (Array) do Custom Event ao pino **"Data"** do Parse
- **OutPlayerId**: Conecte à variável local criada (ou use "Set" se promovido)
- **OutLocation**: Conecte à variável local criada
- **OutYawDegrees**: Conecte à variável local criada
- **OutTimestampMs**: Conecte à variável local criada

**1D. Validar Retorno:**
- **Branch** com o **"Return Value"** (Boolean) do Parse
- **False**: Print `"⚠️ Frame inválido"` + Return

---

### PASSO 4.2: Obter ou Criar PlayerStateEntry

**2A. Adicionar `GetOrCreatePlayerState`:**
1. **Clique com botão direito** → `get or create player state`
2. **Selecione**: **"GetOrCreatePlayerState"**

**2B. Configurar:**
- **StatesArray**: **Get RemoteStates** (variável) → Conecte ao pino **"StatesArray"**
- **PlayerId**: Conecte a variável `OutPlayerId` (do Passo 4.1)

**2C. Criar variável local:**
- **Return Value**: Tipo `Player State Entry` (Struct)
- **Promova para variável**: Nome `CurrentEntry`
- **OU**: Use diretamente no próximo passo

**IMPORTANTE**: Esta função retorna uma **CÓPIA**, não uma referência!

---

### PASSO 4.3: Atualizar Buffer de Estados

**3A. Adicionar `UpdatePlayerStateBuffer`:**
1. **Clique com botão direito** → `update player state buffer`
2. **Selecione**: **"UpdatePlayerStateBuffer"**

**3B. Configurar:**
- **Entry**: Conecte a variável `CurrentEntry` (do Passo 4.2) ao pino **"Entry"**
  - **IMPORTANTE**: Este parâmetro é `UPARAM(ref)`, então será modificado
- **NewLocation**: Conecte `OutLocation` (do Passo 4.1)
- **NewYaw**: Conecte `OutYawDegrees` (do Passo 4.1)
- **NewTimestampMs**: Conecte `OutTimestampMs` (do Passo 4.1)

**3C. Resultado:**
- O `CurrentEntry` foi modificado (agora tem os novos estados A e B atualizados)

---

### PASSO 4.4: Salvar Entry de Volta no Array

**4A. Adicionar `FindPlayerStateIndex`:**
1. **Clique com botão direito** → `find player state index`
2. **Selecione**: **"FindPlayerStateIndex"**
3. **StatesArray**: **Get RemoteStates**
4. **PlayerId**: `OutPlayerId`
5. **Return Value**: Integer (índice ou -1)

**4B. Branch com o índice:**
- **Branch**: Condition = `FoundIndex >= 0`
- **Como comparar**:
  - **Add nó ">="**: Clique com botão direito → `greater or equal`
  - **Selecione**: **">= (Integer)"**
  - **A**: `FoundIndex`
  - **B**: `0` (literal)
  - Conecte ao **"Condition"** do Branch

**4C. Branch True (Index encontrado):**
- **Set Element**:
  1. **Get RemoteStates** → Arraste → `set element`
  2. **Selecione**: **"Set Element (Array)"**
  3. **Array**: Conecte **"Get RemoteStates"**
  4. **Index**: Conecte `FoundIndex`
  5. **Item**: Conecte `CurrentEntry` (modificado)
  6. **Return Value**: Este é o Array modificado - **conecte de volta ao Set RemoteStates**
     - **OU**: Use **"Set RemoteStates"** diretamente com o Return Value

**4D. Branch False (Novo Player):**
- **Add Element**:
  1. **Get RemoteStates** → Arraste → `add element`
  2. **Selecione**: **"Add Element (Array)"**
  3. **Item**: Conecte `CurrentEntry`
  4. **Return Value**: Conecte ao **"Set RemoteStates"**

---

### PASSO 4.5: Criar/Obter Actor Remoto (Opcional)

**5A. Verificar se Actor existe:**
- **Find Item in Array**:
  1. **Get RemoteActorIds** → Arraste → `find item in array`
  2. **Array**: Conecte **"Get RemoteActorIds"**
  3. **Item to Find**: Conecte `OutPlayerId`
  4. **Return Value**: `FoundIndex` (Integer)

**5B. Branch: FoundIndex >= 0?**
- **True**: Actor já existe, pular criação
- **False**: Criar novo Actor

**5C. Criar Actor (Branch False):**
1. **Make Rotator**:
   - **Roll**: `0.0`
   - **Pitch**: `0.0`
   - **Yaw**: Conecte `OutYawDegrees`

2. **Make Transform**:
   - **Location**: Conecte `OutLocation`
   - **Rotation**: Conecte o Rotator criado
   - **Scale**: `(1, 1, 1)` ou deixe padrão

3. **Spawn Actor from Class**:
   - **Class**: Use variável `BP_RemotePlayerClass` (se criada) ou selecione a classe diretamente
   - **Transform**: Conecte o Transform criado
   - **Return Value**: `NewActor` (Actor Reference)

4. **Adicionar aos Arrays**:
   - **Add Element** em `RemoteActorIds`: Item = `OutPlayerId`
   - **Add Element** em `RemoteActors`: Item = `NewActor`

**5D. Obter Actor Existente (Branch True):**
1. **Get a Copy**:
   - **Get RemoteActors** → Arraste → `get a copy`
   - **Selecione**: **"Get a Copy"** (não "Get Element"!)
   - **Array**: Conecte **"Get RemoteActors"**
   - **Index**: Conecte `FoundIndex`
   - **Return Value**: `RemoteActorRef` (Actor Reference)

---

## 🔄 FASE 5: Implementar Event Tick - Interpolação Completa

### Adicionar Event Tick

1. **Clique com botão direito** no gráfico
2. **Digite**: `event tick`
3. **Selecione**: **"Event Tick"**
4. **Parâmetro**: `DeltaSeconds` (Float) - já vem configurado

---

### PASSO 5.1: For Each Loop sobre RemoteStates

**1A. Adicionar `For Each Loop`:**
1. **Clique com botão direito** → `for each`
2. **Selecione**: **"For Each Loop"** (pode ter variações como "For Each Loop (Simple)" ou "For Each Loop with Break")

**1B. Configurar Array:**
- **Array**: **Get RemoteStates** → Conecte ao pino **"Array"** do For Each Loop

**1C. Saídas do Loop:**
- **Array Element**: Cada `Player State Entry` do array (iteração atual)
- **Array Index**: Índice atual (Integer) - não necessário, mas útil para debug
- **Loop Body**: Execução para cada iteração
- **Completed**: Execução quando o loop termina

**1D. Conexão:**
- Do pino **"Exec"** do Event Tick até o **"Loop Body"** do For Each Loop

---

### PASSO 5.2: Break Struct

**2A. Adicionar `Break Player State Entry`:**
1. **Do pino "Array Element"** (Player State Entry) do For Each Loop
2. **Arraste** uma linha
3. **No menu**, digite: `break player state entry`
4. **Selecione**: **"Break Player State Entry"**

**2B. Input:**
- **Player State Entry**: Conecte o **"Array Element"** do For Each Loop

**2C. Outputs (todos os campos):**
- `PlayerId` (Integer)
- `StateA_Location` (Vector)
- `StateA_Yaw` (Float)
- `StateA_TimestampMs` (Integer)
- `StateB_Location` (Vector)
- `StateB_Yaw` (Float)
- `StateB_TimestampMs` (Integer)
- `HasStateA` (Boolean)
- `HasStateB` (Boolean)

---

### PASSO 5.3: Verificar HasStateA AND HasStateB

**3A. Adicionar `Boolean AND`:**
1. **Clique com botão direito** → `and`
2. **Selecione**: **"AND (Boolean)"** ou **"Boolean AND"**

**3B. Conectar:**
- **A**: Conecte `HasStateA` (do Break Struct)
- **B**: Conecte `HasStateB` (do Break Struct)

**3C. Branch:**
- **Branch**: Condition = Resultado do AND
- **True**: Continua para interpolação
- **False**: Pular para próximo elemento (continue loop)

---

### PASSO 5.4: Calcular Alpha

**4A. Get Game Time in Milliseconds:**
- Use o mesmo processo do Passo 3.5

**4B. Calcular ElapsedMs:**
- **Subtract**: 
  - **A**: `CurrentTimeMs`
  - **B**: `StateA_TimestampMs` (do Break Struct)
  - **Return Value**: `ElapsedMs`

**4C. Calcular DeltaMs:**
- **Subtract**:
  - **A**: `StateB_TimestampMs` (do Break Struct)
  - **B**: `StateA_TimestampMs`
  - **Return Value**: `DeltaMs`

**4D. Validar DeltaMs > 0:**
- **Branch**: `DeltaMs > 0`
- **False**: Pular (evitar divisão por zero)

**4E. Calcular Alpha:**
- **Divide**:
  - **A**: `ElapsedMs`
  - **B**: `DeltaMs`
  - **Return Value**: `Alpha` (Float)

**4F. Clamp Alpha:**
- **Clamp (Float)**:
  1. **Clique com botão direito** → `clamp`
  2. **Selecione**: **"Clamp (Float)"**
  3. **Value**: Conecte `Alpha`
  4. **Min**: `0.0` (literal)
  5. **Max**: `1.0` (literal)
  6. **Result**: `ClampedAlpha`

---

### PASSO 5.5: Interpolar Location e Yaw

**5A. VLerp (Vector Lerp):**
1. **Clique com botão direito** → `lerp`
2. **Procure**: **"VLerp"** ou **"Vector Lerp"**
3. **Selecione**: **"VLerp"** (Lerp de Vector)

**5B. Configurar VLerp:**
- **A**: Conecte `StateA_Location` (Vector do Break Struct)
- **B**: Conecte `StateB_Location` (Vector do Break Struct)
- **Alpha**: Conecte `ClampedAlpha` (Float)
- **Return Value**: `InterpolatedLocation` (Vector)

**5C. Lerp (Float Lerp):**
1. **Clique com botão direito** → `lerp`
2. **Selecione**: **"Lerp (Float)"** (não Vector!)

**5D. Configurar Lerp:**
- **A**: Conecte `StateA_Yaw` (Float do Break Struct)
- **B**: Conecte `StateB_Yaw` (Float do Break Struct)
- **Alpha**: Conecte `ClampedAlpha`
- **Return Value**: `InterpolatedYaw` (Float)

---

### PASSO 5.6: Buscar Actor Remoto

**6A. Find Item in Array:**
- **Get RemoteActorIds** → **Find Item in Array**
- **Item to Find**: Conecte `PlayerId` (do Break Struct)
- **Return Value**: `FoundIndex`

**6B. Branch: FoundIndex >= 0?**
- Use o mesmo processo do Passo 4.5

**6C. Get a Copy (Branch True):**
- **Get RemoteActors** → **Get a Copy**
- **Index**: `FoundIndex`
- **Return Value**: `RemoteActorRef`

**6D. IsValid do Actor:**
- **IsValid** do `RemoteActorRef`
- **Branch**: Se não válido, pular

---

### PASSO 5.7: Aplicar Transform

**7A. Make Rotator:**
- **Roll**: `0.0`
- **Pitch**: `0.0`
- **Yaw**: Conecte `InterpolatedYaw`

**7B. Set Actor Location:**
1. **Do `RemoteActorRef`**, arraste
2. **Digite**: `set actor location`
3. **Selecione**: **"Set Actor Location"**
4. **New Location**: Conecte `InterpolatedLocation`

**7C. Set Actor Rotation:**
1. **Do mesmo `RemoteActorRef`**, arraste
2. **Digite**: `set actor rotation`
3. **Selecione**: **"Set Actor Rotation"**
4. **New Rotation**: Conecte o Rotator criado

**7D. Conexão de Execução:**
- Do Branch True (Actor válido) até Set Actor Location
- Do Set Actor Location até Set Actor Rotation

---

## 🏗️ FASE 6: Spawn no Level Blueprint

### Abrir Level Blueprint

1. **No Editor**, abra o nível `Lvl_TestAuth`
2. **Window** → **"Level Blueprint"**
3. Localize ou crie **Event BeginPlay**

---

### PASSO 6.1: Delay Inicial

- **Delay**: `0.5` segundos
- Conecte após Event BeginPlay

---

### PASSO 6.2: Get First Player Controller

- Use o mesmo processo da Fase 3, Passo 3.2
- **IsValid** → **Branch**

---

### PASSO 6.3: Configurar Input Mode

**3A. Set bShowMouseCursor:**
1. **Do Player Controller**, arraste
2. **Digite**: `set b show mouse cursor`
3. **Selecione**: **"Set bShowMouseCursor"**
4. **Value**: `false` (desmarcado)

**3B. Set Input Mode Game Only:**
1. **Do Player Controller**, arraste
2. **Digite**: `set input mode`
3. **Selecione**: **"Set Input Mode Game Only"**

---

### PASSO 6.4: Verificar HasActiveCharacter

- **Get Game Instance** → **Cast to Umbra Game Instance**
- **HasActiveCharacter** → **Branch**

---

### PASSO 6.5: Verificar se BP_NetMovementClient já existe

**5A. Get All Actors of Class:**
1. **Clique com botão direito** → `get all actors of class`
2. **Selecione**: **"Get All Actors of Class"**
3. **Actor Class**: Selecione `BP_NetMovementClient` no dropdown
4. **Return Value**: Array de Actors

**5B. Array Length:**
1. **Do Return Value** (Array), arraste
2. **Digite**: `array length`
3. **Selecione**: **"Length (Array)"**
4. **Return Value**: Integer (quantidade de elementos)

**5C. Branch: Length == 0?**
- **Compare**: Use **"==" (Integer)**
- **A**: Length
- **B**: `0` (literal)
- **Branch**: Se `true`, criar novo; se `false`, já existe

---

### PASSO 6.6: Spawn BP_NetMovementClient

**6A. Se Branch True (Length == 0):**

1. **Obter ActivePlayerID** (do Game Instance):
   
   **OPÇÃO 1 - Get Active Player ID (Se disponível):**
   - **Do "As Umbra Game Instance"**: Arraste → `get active player id`
   - **Selecione**: **"Get Active Player ID"**
   - **Return Value**: `ActivePlayerID` (Integer)
   
   **OPÇÃO 2 - Get Active Character + Break (Alternativa):**
   - **Get Active Character** → **Break Umbra Player Data**
   - Use o campo **"ID"** do Break Struct
   - Promova para variável ou use diretamente

2. **Get Pawn** (do Player Controller):
   - **Do Return Value** (Player Controller) do `Get First Player Controller`
   - Arraste → `get pawn`
   - **Target**: Conecte o Player Controller

3. **Get Actor Location** (do Pawn):
   - **Do Return Value** (Pawn) do Get Pawn
   - Arraste → `get actor location`
   - **Return Value**: `SpawnLocation` (Vector)

4. **Make Transform**:
   - **Clique com botão direito** → `make transform`
   - **Selecione**: **"Make Transform"**
   - **Location**: Conecte `SpawnLocation`
   - **Rotation**: 
     - **Make Rotator**: `(0, 0, 0)` (Roll=0, Pitch=0, Yaw=0)
     - Conecte ao campo **"Rotation"**
   - **Scale**: `(1, 1, 1)` (deixe padrão ou configure)

5. **Spawn Actor from Class**:
   - **Clique com botão direito** → `spawn actor from class`
   - **Selecione**: **"Spawn Actor from Class"**
   - **Class**: 
     - Clique no dropdown → Procure `BP_NetMovementClient`
     - OU selecione diretamente no Content Browser e arraste para o campo
   - **Transform**: Conecte o Transform criado no passo 4
   - **Return Value**: `SpawnedActor` (Actor Reference)

6. **Set MyPlayerId** (do SpawnedActor):
   - **Do Return Value** (SpawnedActor) do Spawn Actor
   - **Arraste** uma linha → `set my player id`
   - **Selecione**: **"Set MyPlayerId"** (função do BP_NetMovementClient)
   - **Target**: Deve ser automaticamente o `SpawnedActor`
   - **MyPlayerId**: Conecte `ActivePlayerID` (do passo 1)
   - **Execução**: Conecte do pino **"then"** do Spawn Actor até o **"Exec"** do Set

---

## ⚠️ TROUBLESHOOTING: WebSocket Não Conecta

### 🚨 Problema Identificado: "socket connect failed"

**Sintomas observados:**
- Log mostra: `[BP_NetMovementClient_C_1] socket connect failed`
- Overlay no viewport mostra: "socket connect failed"
- Cliente não consegue conectar ao WebSocket

**Causa mais comum**: O **Zone Server não está rodando**.

O Zone Server é um executável separado que **DEVE ser executado ANTES** do cliente Unreal Engine se conectar. Ele não é iniciado automaticamente - você precisa executá-lo manualmente.

**Por que isso acontece?**
- O Zone Server (`zone_server.exe`) é um servidor C++ independente
- Ele escuta conexões WebSocket na porta configurada (8082 por padrão)
- Se ele não estiver rodando, não há servidor para aceitar a conexão do cliente
- O erro "socket connect failed" ocorre porque o cliente tenta conectar mas não encontra nenhum servidor na porta

### Solução: Executar o Zone Server

#### Passo 1: Localizar o Executável

O executável está em: `build\bin\Release\zone_server.exe`

**Caminho completo** (a partir da raiz do projeto):
```
D:\UmbraServerV2\build\bin\Release\zone_server.exe
```

#### Passo 2: Executar o Zone Server na Porta 8082

**Importante**: O cliente Unreal está configurado para conectar na porta **8082**.

O Zone Server calcula a porta como: `8082 + ZoneID`

**Para usar a porta 8082, execute com Zone ID 0:**

**Opção A - PowerShell (Recomendado):**
```powershell
# Navegue até o diretório
cd D:\UmbraServerV2\build\bin\Release

# Execute com Zone ID 0 (porta 8082)
.\zone_server.exe 0
```

**Opção B - Script Automático:**
```powershell
# Execute o script PowerShell
.\scripts\start_zone_server_8082.ps1
```

**Opção C - Batch Script:**
```batch
# Execute o script batch
.\scripts\start_zone_server_8082.bat
```

#### Passo 3: Verificar se o Servidor Está Rodando

**Abra um novo terminal e execute:**

```powershell
# Verificar se a porta 8082 está em uso (LISTENING)
netstat -ano | findstr :8082
```

**Saída esperada**:
```
TCP    0.0.0.0:8082           0.0.0.0:0              LISTENING       [PID]
```

**Se aparecer "LISTENING"**, o servidor está rodando corretamente! ✅

#### Passo 4: Verificar os Logs do Zone Server

O Zone Server cria logs em: `build\bin\Release\logs\zone_server.log`

**Logs esperados quando o servidor inicia:**
```
[INFO] Starting Zone Server...
[INFO] ZoneServer 'Zone_0' (ID: 0) started on port 8082
[INFO] WebSocketServer started on port 8082
```

**Logs esperados quando um cliente conecta:**
```
[INFO] New WebSocket connection from 127.0.0.1:[PORTA]
[INFO] WS client [ID] connected
```

### Outros Problemas Comuns

#### Problema 1: Porta já em uso

**Erro**: "Failed to bind WebSocket server to port 8082"

**Solução**:
1. **Encontrar o processo usando a porta 8082:**
   ```powershell
   netstat -ano | findstr :8082
   # Anote o PID (última coluna)
   ```

2. **Matar o processo** (substitua [PID] pelo número):
   ```powershell
   taskkill /PID [PID] /F
   ```

3. **OU usar outra porta**: Execute `zone_server.exe 1` (porta 8083) e atualize o cliente para usar porta 8083.

#### Problema 2: Zone Server fecha imediatamente

**Solução**:
- Verifique os logs em `build\bin\Release\logs\zone_server.log`
- Execute no terminal (não dê duplo clique) para ver erros
- Verifique se há dependências faltando (ex: `libmysql.dll`)

#### Problema 3: Cliente conecta mas não recebe mensagens

**Possíveis causas**:
1. **Zone Server não está processando updates**: Verifique se `zoneServer.update(deltaTime)` está sendo chamado
2. **Callback não configurado**: Verifique se `setBinaryCallback` foi chamado no MovementServer
3. **Handshake não completado**: Verifique os logs do WebSocketServer

### Checklist de Verificação

Antes de testar o cliente Unreal:

- [ ] Zone Server está executando (`zone_server.exe 0`)
- [ ] Porta 8082 está em LISTENING (`netstat -ano | findstr :8082`)
- [ ] Logs mostram "WebSocketServer started on port 8082"
- [ ] Cliente Unreal está configurado para `ws://127.0.0.1:8082`
- [ ] Não há firewall bloqueando a porta 8082

### Ordem de Execução Correta

1. **PRIMEIRO**: Execute o Zone Server (`zone_server.exe 0`)
2. **SEGUNDO**: Inicie o cliente Unreal Engine (PIE)
3. **TERCEIRO**: Faça login e selecione personagem
4. **QUARTO**: O WebSocket deve conectar automaticamente

---

## 🧪 Teste Multi-Cliente

### Configurar PIE

1. **Edit** → **Editor Preferences** → **Play**
2. **Number of Players**: `2`
3. **Net Mode**: `Play As Listen Server`

### Verificar Player IDs

- Cada cliente deve ter `MyPlayerId` diferente
- Use Player Index ou configure manualmente

---

## 📋 Checklist Final

- [ ] Todas as variáveis criadas
- [ ] BeginPlay implementado completamente
- [ ] OnWSConnected implementado
- [ ] SendMoveUpdate implementado
- [ ] OnWSBinaryMessage implementado
- [ ] Event Tick implementado
- [ ] Level Blueprint configurado
- [ ] Teste com 2 clientes funcionando

---

**Documento Completo!** 🎉

Este guia cobre cada nó, cada conexão e cada interação no fluxo completo de implementação.
