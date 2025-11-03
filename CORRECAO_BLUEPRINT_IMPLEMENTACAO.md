# 🔧 Correção Detalhada: Implementação do Buffering no Blueprint

## 🔴 Problemas Identificados no XML

Após análise do `BP_NetMovementClient.T3D`, foram identificados os seguintes problemas na implementação atual:

### 1. **Pin `NewData` Marcado como Ignorado**
- **Localização:** `K2Node_CallFunction_41` (ProcessBinaryBuffer), linha 2351
- **Problema:** O pin `NewData` está conectado ao `OnWSBinaryMessage.Data`, mas está marcado como `bIsIgnored=True`
- **Efeito:** O Unreal Engine ignora a conexão, causando erro "New Data inválido"

### 2. **ParseStateUpdateFrame Usando Data Direto**
- **Localização:** `K2Node_CallFunction_4` (ParseStateUpdateFrame), linha 740
- **Problema:** O pin `Data` está conectado diretamente ao `OnWSBinaryMessage.Data` em vez de usar `OutFrame.Data`
- **Efeito:** O ParseStateUpdateFrame recebe dados fragmentados (25 bytes) em vez do frame completo (29 bytes)

### 3. **WhileLoop Não Conectado Corretamente**
- **Localização:** `K2Node_MacroInstance_2` (WhileLoop), linha 2336
- **Problema:** O pin `Condition` não está conectado ao retorno do `ProcessBinaryBuffer`
- **Efeito:** O loop não processa múltiplos frames no buffer

---

## ✅ Solução Passo a Passo

### **PASSO 1: Remover Conexões Antigas**

1. **Abra o Blueprint `BP_NetMovementClient`**
2. **Localize o Event `OnWSBinaryMessage`** no Event Graph
3. **Desconecte TODAS as conexões do pin `Data` do evento:**
   - Desconecte de `ParseStateUpdateFrame`
   - Desconecte de `Get Array Item`
   - Desconecte de `Array Length` (se houver)
   - Mantenha apenas a conexão com `ProcessBinaryBuffer` (mas vamos corrigir)

### **PASSO 2: Corrigir ProcessBinaryBuffer**

1. **Localize o nó `ProcessBinaryBuffer`** (geralmente chamado de `K2Node_CallFunction_41`)
2. **Verifique as conexões:**
   - **Pin `Buffer` (ref):** Deve estar conectado a `Get BinaryMessageBuffer` (variável)
   - **Pin `NewData`:** Deve estar conectado ao pin `Data` do evento `OnWSBinaryMessage`
   - **Pin `OutFrame` (ref):** Deve estar conectado a uma variável local `OutFrame` (tipo `FBinaryFrame`)

3. **Se o pin `NewData` estiver marcado como "ignored":**
   - **Clique com botão direito** no pin `NewData`
   - **Selecione "Remove Connection"** (se houver)
   - **Reconecte** ao pin `Data` do evento `OnWSBinaryMessage`
   - **Certifique-se** de que não há avisos de erro no nó

### **PASSO 3: Criar Variável Local `OutFrame`**

1. **No painel "My Blueprint" (painel esquerdo), clique em `+ Variable`**
2. **Nome:** `OutFrame`
3. **Tipo:** `Binary Frame` (ou `FBinaryFrame` se aparecer)
4. **Categoria:** Pode deixar "None" ou criar categoria "Network"
5. **Certifique-se** de que a variável está marcada como **instância** (não estática)

### **PASSO 4: Estrutura do Loop CORRIGIDA (EVITA LOOP INFINITO)**

⚠️ **IMPORTANTE:** A estrutura abaixo evita loop infinito chamando `ProcessBinaryBuffer` DENTRO do loop para verificar se há mais frames.

1. **Localize o nó `WhileLoop`** (macro padrão do Unreal)
2. **Primeira chamada ao `ProcessBinaryBuffer` (FORA do loop):**
   - Conecte o pin `execute` do evento `OnWSBinaryMessage` ao pin `execute` do `ProcessBinaryBuffer`
   - Conecte `Buffer`: Get BinaryMessageBuffer (variável)
   - Conecte `NewData`: Data (do evento `OnWSBinaryMessage`)
   - Conecte `OutFrame`: OutFrame (variável local)

3. **Conecte o pin `then` do `ProcessBinaryBuffer` ao pin `execute` do `WhileLoop`**

4. **Conecte o pin `Return Value` (bool) do `ProcessBinaryBuffer` ao pin `Condition` do `WhileLoop`:**
   - Isso verifica se há um frame disponível ANTES de entrar no loop

### **PASSO 5: Processar Frame Dentro do Loop**

Dentro do `Loop Body` do WhileLoop:

1. **Adicione um nó `Get OutFrame`** (arraste a variável `OutFrame` para o grafo)
2. **Adicione um nó `Break BinaryFrame`**:
   - Conecte `Get OutFrame` ao nó `Break BinaryFrame`
   - Isso extrairá o `Data` (Array of Bytes) do frame

3. **Corrija o `ParseStateUpdateFrame`:**
   - **Desconecte** o pin `Data` do `OnWSBinaryMessage`
   - **Conecte** o pin `Data` do `ParseStateUpdateFrame` ao pin `Data` do `Break BinaryFrame`
   - Agora o `ParseStateUpdateFrame` receberá o frame completo (29 bytes)

4. **Conecte o pin `then` do `ParseStateUpdateFrame` à sua lógica de atualização de estado:**
   - Use os pins `OutPlayerId`, `OutLocation`, `OutYawDegrees`, `OutTimestampMs` normalmente

### **PASSO 6: Chamar ProcessBinaryBuffer DENTRO do Loop (CRÍTICO)**

⚠️ **IMPORTANTE:** Para evitar loop infinito, você DEVE chamar `ProcessBinaryBuffer` novamente DENTRO do loop para verificar se há mais frames:

1. **Após processar o frame, adicione OUTRO nó `ProcessBinaryBuffer`:**
   - **Pin `Buffer`:** Get BinaryMessageBuffer (mesma variável, por referência)
   - **Pin `NewData`:** Crie um nó `Make Array` com 0 elementos (array vazio)
   - **Pin `OutFrame`:** OutFrame (mesma variável, será atualizada se houver mais frames)
   - **Pin `Return Value`:** Conecte ao pin `Loop Body` do WhileLoop

2. **A estrutura dentro do loop deve ser:**
   ```
   Loop Body (WhileLoop)
       ↓
   Get OutFrame
       ↓
   Break BinaryFrame
       ↓
   ParseStateUpdateFrame
       ↓
   [Sua lógica]
       ↓
   ProcessBinaryBuffer (NOVAMENTE, com NewData = array vazio)
       ↓
   Loop Body (volta para o início, mas agora com novo ReturnValue)
   ```

3. **Conecte o pin `Return Value` do SEGUNDO `ProcessBinaryBuffer` ao pin `Loop Body` do `WhileLoop`:**
   - ⚠️ **CUIDADO:** Isso não é possível diretamente! O WhileLoop não permite atualizar a condição dinamicamente.
   - **SOLUÇÃO:** Use um nó `Branch` (If-Then-Else) após o segundo `ProcessBinaryBuffer`:
     - Se `ReturnValue == true`: Conecte ao pin `Loop Body` do WhileLoop (continua o loop)
     - Se `ReturnValue == false`: NÃO conecte ao Loop Body (sai do loop via `Completed`)

### **PASSO 7: Solução Alternativa (RECOMENDADA) - Evitar Loop Infinito**

Como o WhileLoop padrão não permite atualizar a condição dinamicamente, use esta estrutura:

1. **Primeira chamada `ProcessBinaryBuffer` (fora do loop)**
2. **WhileLoop com condição inicial**
3. **Dentro do Loop Body:**
   - Processe o frame
   - Chame `ProcessBinaryBuffer` novamente (com array vazio)
   - Use `Branch` para decidir se continua:
     - Se retornar `true`: Conecte ao `Loop Body` novamente
     - Se retornar `false`: NÃO conecte ao `Loop Body` (sai automaticamente)

---

## 📋 Estrutura Final Correta (EVITA LOOP INFINITO)

```
OnWSBinaryMessage (Data)
    ↓
ProcessBinaryBuffer (PRIMEIRA VEZ)
    - Buffer: Get BinaryMessageBuffer (ref) ✅
    - NewData: Data (do evento) ✅
    - OutFrame: OutFrame (ref, variável local) ✅
    → ReturnValue (bool)
    ↓
WhileLoop
    - Condition: ReturnValue do ProcessBinaryBuffer (primeira vez) ✅
    - Loop Body:
        ↓
    Get OutFrame
        ↓
    Break BinaryFrame (Data)
        ↓
    ParseStateUpdateFrame (Data do Break BinaryFrame) ✅
        ↓
    [Sua lógica de atualização de estado]
        ↓
    ProcessBinaryBuffer (SEGUNDA VEZ, com NewData = array vazio) ⚠️
        - Buffer: Get BinaryMessageBuffer (ref, mesmo buffer)
        - NewData: Make Array (vazio) ⚠️
        - OutFrame: OutFrame (ref, será atualizado)
        → ReturnValue (bool)
        ↓
    Branch (If ReturnValue == true)
        - True: → Loop Body (continua o loop)
        - False: → (não conecta, sai do loop)
    ↓
Completed (do WhileLoop) - executado quando não há mais frames
    ↓
[Fim do processamento]
```

⚠️ **IMPORTANTE:** O segundo `ProcessBinaryBuffer` dentro do loop DEVE ter `NewData = array vazio`, pois os dados já foram adicionados na primeira chamada. Isso evita adicionar os mesmos dados repetidamente e causa loop infinito.

---

## 🔍 Verificações Finais

1. **Nenhum erro de compilação** no Blueprint
2. **Nenhum pin `ignored`** ou `not connected`
3. **O pin `Data` do `ParseStateUpdateFrame`** está conectado ao `Data` do `Break BinaryFrame`, NÃO ao `Data` do evento
4. **O WhileLoop** usa o `ReturnValue` do `ProcessBinaryBuffer` como condição
5. **A variável `BinaryMessageBuffer`** existe e é do tipo `Array of Bytes`

---

## 🐛 Debug

### **❌ Loop Infinito e Crash**
- **Causa 1:** O `ProcessBinaryBuffer` dentro do loop está recebendo `NewData` do evento novamente, adicionando os mesmos dados repetidamente
- **Solução:** O `NewData` do segundo `ProcessBinaryBuffer` (dentro do loop) DEVE ser um **array vazio** (use `Make Array` sem elementos)

- **Causa 2:** A condição do `WhileLoop` não está sendo atualizada dentro do loop
- **Solução:** Use um `Branch` após o segundo `ProcessBinaryBuffer` e conecte apenas se `ReturnValue == true` ao `Loop Body`

- **Causa 3:** O `ProcessBinaryBuffer` não está removendo o frame do buffer corretamente
- **Solução:** Verifique se o `Buffer` está sendo passado por referência e a função está removendo os 29 bytes após extrair

### **Mensagem: "Binary Buffer Process failed"**
- **Causa:** `ProcessBinaryBuffer` retornou `false` (nenhum frame completo disponível)
- **Solução:** Isso é normal quando há fragmentação. O buffer acumula dados até ter 29 bytes.

### **Mensagem: "Received binary message, size:25 expected=29"**
- **Causa:** O `ParseStateUpdateFrame` ainda está usando `Data` do evento diretamente
- **Solução:** Certifique-se de que está usando `OutFrame.Data` (do `Break BinaryFrame`)

### **Erro: "New Data inválido"**
- **Causa:** O pin `NewData` não está conectado ou está marcado como ignorado
- **Solução:** Reconecte o pin `NewData` ao `Data` do evento `OnWSBinaryMessage` (primeira chamada) ou use `Make Array` vazio (segunda chamada)

---

## 🚨 SOLUÇÃO DEFINITIVA PARA LOOP INFINITO

### **PROBLEMA IDENTIFICADO NO SEU XML:**
Analisando seu código, vejo que:
1. ✅ Primeiro `ProcessBinaryBuffer` está correto (fora do loop)
2. ✅ `WhileLoop` está conectado corretamente
3. ❌ **PROBLEMA:** O segundo `ProcessBinaryBuffer` dentro do loop tem `MakeArray_2` com `[0] = 0` (não é vazio!)
4. ❌ **PROBLEMA:** O `ReturnValue` do segundo `ProcessBinaryBuffer` NÃO está conectado ao `LoopBody` do `WhileLoop`

### **CORREÇÃO EXATA (PASSO A PASSO):**

#### **PASSO 1: Corrigir o MakeArray Dentro do Loop**

1. **Localize o nó `K2Node_MakeArray_2`** (MakeArray que está conectado ao segundo `ProcessBinaryBuffer`)
2. **Se ele tiver um elemento `[0] = 0`:**
   - **Clique com botão direito** no elemento `[0]`
   - **Selecione "Remove Pin"** ou **Delete**
   - **O array deve ficar COMPLETAMENTE VAZIO (0 elementos)**
   - ⚠️ **CRÍTICO:** Um array com `[0] = 0` não é vazio! Precisamos de 0 elementos.

#### **PASSO 2: Conectar o ReturnValue ao Loop Body**

O problema principal é que o `ReturnValue` do segundo `ProcessBinaryBuffer` não está sendo usado para continuar o loop.

**Solução:** O `WhileLoop` padrão do Unreal NÃO permite atualizar a condição dinamicamente dentro do loop. Você precisa usar uma abordagem diferente:

**OPÇÃO A - Usar Branch (RECOMENDADO):**

1. **Após o segundo `ProcessBinaryBuffer` dentro do loop:**
   - Adicione um nó `Branch` (If-Then-Else)
   - Conecte o `ReturnValue` (bool) do segundo `ProcessBinaryBuffer` ao pin `Condition` do `Branch`

2. **No `Branch`:**
   - **Pin `True`:** Conecte ao pin `LoopBody` do `WhileLoop` (volta para o início do loop)
   - **Pin `False`:** NÃO conecte nada (o loop termina automaticamente via `Completed`)

3. **Estrutura dentro do Loop Body:**
   ```
   Loop Body (entra aqui)
       ↓
   Get OutFrame
       ↓
   Break BinaryFrame
       ↓
   ParseStateUpdateFrame
       ↓
   [Sua lógica de atualização]
       ↓
   ProcessBinaryBuffer (SEGUNDA VEZ)
       - Buffer: Get BinaryMessageBuffer
       - NewData: Make Array (COMPLETAMENTE VAZIO - sem nenhum elemento!)
       - OutFrame: OutFrame
       → ReturnValue (bool)
       ↓
   Branch (If ReturnValue == true)
       - True → Loop Body (volta ao início do loop)
       - False → (não conecta, loop termina)
   ```

**OPÇÃO B - Remover WhileLoop e Usar Branch Simples (MAIS SEGURO):**

Se o `WhileLoop` continua causando problemas, use uma abordagem sem loop explícito:

1. **Primeira chamada `ProcessBinaryBuffer`:**
   ```
   OnWSBinaryMessage
       ↓
   ProcessBinaryBuffer
       - Buffer: Get BinaryMessageBuffer
       - NewData: Data (do evento)
       - OutFrame: OutFrame
       → ReturnValue
       ↓
   Branch (se ReturnValue == true)
       - True → ProcessarFrame (função customizada)
       - False → [Fim - buffer não tem frame completo ainda]
   ```

2. **Criar Função Customizada `ProcessFrame`:**
   - Esta função será chamada recursivamente até não haver mais frames
   ```
   ProcessFrame (sem parâmetros)
       ↓
   Get OutFrame
       ↓
   Break BinaryFrame
       ↓
   ParseStateUpdateFrame
       ↓
   [Sua lógica de atualização]
       ↓
   ProcessBinaryBuffer (segunda vez)
       - Buffer: Get BinaryMessageBuffer
       - NewData: Make Array (VAZIO)
       - OutFrame: OutFrame
       → ReturnValue
       ↓
   Branch (se ReturnValue == true)
       - True → ProcessFrame (chama a si mesma - recursão)
       - False → [Fim - não há mais frames]
   ```

### **📸 ONDE EXATAMENTE COLOCAR O SEGUNDO ProcessBinaryBuffer:**

**Visualização da Estrutura no Blueprint:**

```
┌─────────────────────────────────────────────────────────────┐
│ OnWSBinaryMessage                                           │
│ ┌─────────────┐                                             │
│ │ Data (ref)  │──────────────────────────────────┐         │
│ └─────────────┘                                  │         │
│                                                  │         │
│                                                  ▼         │
│                                    ┌──────────────────────┐ │
│                                    │ ProcessBinaryBuffer  │ │
│                                    │ (PRIMEIRA VEZ)      │ │
│                                    │ - Buffer: Get...    │ │
│                                    │ - NewData: Data     │ │
│                                    │ - OutFrame: Out...  │ │
│                                    │ → ReturnValue (bool)│ │
│                                    └──────────────────────┘ │
│                                                  │         │
│                                                  ▼         │
│                                    ┌──────────────────────┐ │
│                                    │ WhileLoop            │ │
│                                    │ - Condition: Return  │ │
│                                    │ - Loop Body ──────────┼─┐
│                                    └──────────────────────┘ │ │
│                                                            │ │
│                                                            │ │
│ ┌──────────────────────────────────────────────────────┐  │ │
│ │ LOOP BODY (dentro do WhileLoop)                      │  │ │
│ │                                                       │  │ │
│ │ Get OutFrame                                          │  │ │
│ │    ↓                                                  │  │ │
│ │ Break BinaryFrame                                     │  │ │
│ │    ↓                                                  │  │ │
│ │ ParseStateUpdateFrame                                 │  │ │
│ │    ↓                                                  │  │ │
│ │ [Sua lógica]                                          │  │ │
│ │    ↓                                                  │  │ │
│ │ ┌─────────────────────────────────────────────────┐  │  │ │
│ │ │ ProcessBinaryBuffer (SEGUNDA VEZ) ───────────────┼──┼──┘ │
│ │ │ - Buffer: Get BinaryMessageBuffer                │  │    │
│ │ │ - NewData: Make Array (VAZIO!) ──────┐          │  │    │
│ │ │ - OutFrame: OutFrame                 │          │  │    │
│ │ │ → ReturnValue (bool) ────────────┐  │          │  │    │
│ │ └──────────────────────────────────┼──┼──────────┘  │    │
│ │                                     │  │              │    │
│ │                                     │  │              │    │
│ │                                     ▼  ▼              │    │
│ │                            ┌──────────────────┐     │    │
│ │                            │ Branch            │     │    │
│ │                            │ - Condition: Ret  │     │    │
│ │                            │ - True ───────────┼─────┘    │
│ │                            │ - False → (nada)  │          │
│ │                            └──────────────────┘            │
│ └──────────────────────────────────────────────────────────┘
│
│ Make Array (dentro do segundo ProcessBinaryBuffer)
│ ┌──────────────────┐
│ │ Array (vazio)    │ (SEM elementos [0], [1], etc!)
│ │                  │
│ │ ⚠️ CRÍTICO:      │
│ │ Não pode ter     │
│ │ nenhum elemento! │
│ └──────────────────┘
└─────────────────────────────────────────────────────────────┘
```

### **⚠️ PASSOS EXATOS NO UNREAL EDITOR:**

1. **Abra o Event Graph do `BP_NetMovementClient`**

2. **Localize o `WhileLoop`** - você deve ver o pin `Loop Body` (saída de execução)

3. **DENTRO do Loop Body (conectado ao pin `Loop Body` do WhileLoop):**
   - Após toda a sua lógica de processamento do frame (`ParseStateUpdateFrame`, etc.)
   - Adicione um novo nó `ProcessBinaryBuffer`
   - **IMPORTANTE:** Este é o SEGUNDO `ProcessBinaryBuffer`, dentro do loop

4. **Configure o segundo `ProcessBinaryBuffer`:**
   - **Pin `Buffer`:** Conecte a `Get BinaryMessageBuffer` (mesma variável do primeiro)
   - **Pin `NewData`:** 
     - Adicione um nó `Make Array` (procure por "Make Array" no menu)
     - **NÃO adicione nenhum elemento ao array!** (clique em "Remove Pin" se houver `[0]`)
     - Conecte o output do `Make Array` ao `NewData`
   - **Pin `OutFrame`:** Conecte a `Get OutFrame` (mesma variável)

5. **Após o segundo `ProcessBinaryBuffer`:**
   - ⚠️ **PROBLEMA:** O `WhileLoop` padrão do Unreal NÃO permite atualizar a condição dinamicamente dentro do loop!
   - **SOLUÇÃO:** Você precisa usar uma estrutura diferente:

   **OPÇÃO 1 - Usar Custom Event (RECOMENDADO - EVITA LOOP INFINITO):**
   
   Em vez de usar `WhileLoop`, crie um **Custom Event** chamado `ProcessNextFrame`:
   
   ```
   OnWSBinaryMessage
       ↓
   ProcessBinaryBuffer (primeira vez, com Data do evento)
       → ReturnValue
       ↓
   Branch
       - True → ProcessNextFrame (Custom Event)
       - False → [Fim]
   
   ProcessNextFrame (Custom Event)
       ↓
   Get OutFrame
       ↓
   Break BinaryFrame
       ↓
   ParseStateUpdateFrame
       ↓
   [Sua lógica]
       ↓
   ProcessBinaryBuffer (segunda vez, com Make Array VAZIO)
       → ReturnValue
       ↓
   Branch
       - True → ProcessNextFrame (chama a si mesmo - recursão controlada)
       - False → [Fim - não há mais frames]
   ```
   
   **Por que isso funciona:**
   - Cada chamada de `ProcessNextFrame` processa apenas UM frame
   - Se houver mais frames, chama a si mesmo recursivamente
   - Não há loop infinito porque a recursão para quando não há mais frames
   - Muito mais seguro que `WhileLoop`
   
   **OPÇÃO 2 - Se insistir em usar WhileLoop:**
   
   ⚠️ **ATENÇÃO:** O `WhileLoop` só verifica a condição ANTES de entrar, não durante!
   
   Para fazer funcionar:
   1. Após o segundo `ProcessBinaryBuffer`, conecte o `then` diretamente ao `Loop Body` do `WhileLoop`
   2. O `ReturnValue` do segundo `ProcessBinaryBuffer` deve ser conectado a um `Branch`
   3. O `Branch` deve verificar: se `false`, use um `Gate` ou `Sequence` para quebrar o loop
   4. **Mas isso é complicado e pode causar problemas!** Prefira a OPÇÃO 1.

6. **Verificação final:**
   - O `Make Array` deve mostrar "0 Elements" quando selecionado
   - O segundo `ProcessBinaryBuffer` deve estar DENTRO do `Loop Body` (se usar WhileLoop) ou no Custom Event (se usar OPÇÃO 1)
   - O `NewData` do segundo `ProcessBinaryBuffer` NÃO deve vir do evento `OnWSBinaryMessage`

---

## 🎯 SOLUÇÃO DEFINITIVA: CRIAR CUSTOM EVENT (RECOMENDADO)

### **Por que usar Custom Event em vez de WhileLoop:**
- ✅ **Evita loop infinito** - recursão controlada
- ✅ **Mais fácil de debugar** - cada frame é processado em uma chamada separada
- ✅ **Sem problemas de condição** - o Unreal gerencia a pilha de chamadas automaticamente
- ✅ **Mais seguro** - se houver erro, não trava o editor

### **PASSO A PASSO COMPLETO:**

#### **PASSO 1: Remover o WhileLoop Existente**

1. **No Event Graph, localize o `WhileLoop`**
2. **Desconecte TODAS as conexões:**
   - Desconecte o `execute` do `WhileLoop`
   - Desconecte o `Condition`
   - Desconecte o `Loop Body`
3. **Delete o `WhileLoop`** (selecione e pressione Delete)

#### **PASSO 2: Criar Custom Event `z`**

1. **No painel "My Blueprint" (painel esquerdo), clique em `+ Function` ou `+ Custom Event`**
2. **Selecione "Custom Event"**
3. **Nome:** `ProcessNextFrame`
4. **Certifique-se** de que não tem parâmetros de entrada

#### **PASSO 3: Implementar o Custom Event `ProcessNextFrame`**

⚠️ **IMPORTANTE:** Tudo abaixo deve estar **DENTRO** do Custom Event `ProcessNextFrame`, não no `OnWSBinaryMessage`!

**Ordem exata de execução dentro do `ProcessNextFrame`:**

1. **`Get OutFrame`** (arraste a variável `OutFrame` para o grafo)
   - Isso obtém o frame que foi extraído pelo primeiro `ProcessBinaryBuffer`

2. **`Break BinaryFrame`:**
   - Conecte `Get OutFrame` ao pin de entrada do `Break BinaryFrame`
   - Isso extrai o `Data` (Array of Bytes) do frame
   - ⚠️ **Use o `Data` do `Break BinaryFrame`, NÃO do evento `OnWSBinaryMessage`!**

3. **`ParseStateUpdateFrame`:** ✅ **SIM, deve estar dentro do evento!**
   - Conecte o pin `Data` do `Break BinaryFrame` ao pin `Data` do `ParseStateUpdateFrame`
   - Isso converte os 29 bytes binários em valores: `OutPlayerId`, `OutLocation`, `OutYawDegrees`, `OutTimestampMs`

4. **Toda a sua lógica de atualização (em ordem):**
   - **`Get Array Item`** - verificar se o primeiro byte é `2` (type do frame)
   - **`If-Then-Else`** - verificar se type == 2 e playerId != localPlayerId
   - **`GetOrCreatePlayerState`** - obter ou criar entrada do player
   - **`UpdatePlayerStateBuffer`** - atualizar posição, yaw, timestamp
   - **`FindPlayerStateIndex`** - encontrar índice no array
   - **`Array_Set`** - atualizar o array de estados
   - **`Array_Find`** - verificar se o RemoteActorId já existe
   - **`SpawnActorFromClass`** - criar ator remoto se não existir
   - **`SetActorLocation`** - atualizar posição do ator
   - **`SetActorRotation`** - atualizar rotação do ator
   - ⚠️ **Tudo isso deve estar dentro do `ProcessNextFrame`!**

5. **Após `SetActorRotation` (ou sua última operação de atualização), adicione `ProcessBinaryBuffer` (SEGUNDA VEZ):**
   - **Pin `Buffer`:** Get BinaryMessageBuffer (mesma variável)
   - **Pin `NewData`:** Crie um `Make Array` **COMPLETAMENTE VAZIO** (0 elementos) ⚠️ CRÍTICO
   - **Pin `OutFrame`:** Get OutFrame (será atualizado se houver mais frames)
   - **Pin `ReturnValue`:** Conecte ao pin `Condition` de um `Branch`

6. **Adicione `Branch`:**
   - **Pin `Condition`:** ReturnValue do segundo `ProcessBinaryBuffer`
   - **Pin `True`:** Conecte ao pin `execute` do evento `ProcessNextFrame` (chama a si mesmo recursivamente)
   - **Pin `False`:** Não conecte nada (fim do processamento - não há mais frames)

### **📋 Estrutura Completa Dentro do `ProcessNextFrame`:**

```
ProcessNextFrame (Custom Event)
    ↓
Get OutFrame
    ↓
Break BinaryFrame → Data
    ↓
ParseStateUpdateFrame (Data do Break BinaryFrame)
    → OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs
    ↓
Get Array Item (Data[0]) → verificar type == 2
    ↓
If-Then-Else (se type == 2 e playerId != localPlayerId)
    ↓ (True)
GetOrCreatePlayerState
    ↓
UpdatePlayerStateBuffer
    ↓
FindPlayerStateIndex
    ↓
Array_Set (atualizar RemoteStates)
    ↓
Array_Find (verificar se RemoteActorId existe)
    ↓
If-Then-Else (se não existe)
    ↓ (True)
SpawnActorFromClass (criar BP_RemotePlayer)
    ↓
SetActorLocation (usar InterpolatedLocation)
    ↓
SetActorRotation (usar InterpolatedYaw)
    ↓
ProcessBinaryBuffer (SEGUNDA VEZ) ← ADICIONAR AQUI!
    - Buffer: Get BinaryMessageBuffer
    - NewData: Make Array (VAZIO!)
    - OutFrame: Get OutFrame
    → ReturnValue
    ↓
Branch
    - True → ProcessNextFrame (recursão)
    - False → [Fim]
```

### **⚠️ ONDE EXATAMENTE ADICIONAR O SEGUNDO `ProcessBinaryBuffer`:**

**Após a última operação de atualização do frame:**
- Se você usa `SetActorRotation` como última operação → adicione APÓS `SetActorRotation`
- Se você usa `Array_Add` para adicionar RemoteActors → adicione APÓS essa operação
- Em geral: **após toda a lógica que atualiza o estado do player remoto**

**Exemplo prático:**
```
SetActorRotation
    ↓ (then - pin de saída de execução)
ProcessBinaryBuffer (SEGUNDA VEZ) ← AQUI! (dentro do ProcessNextFrame)
    ↓ (then - pin de saída de execução)
Branch
    - Condition: ReturnValue do ProcessBinaryBuffer
    - True: → ProcessNextFrame (chama a si mesmo)
    - False: (não conecta nada - fim)
```

### **🎯 DIAGRAMA VISUAL DA ESTRUTURA COMPLETA:**

```
┌──────────────────────────────────────────────────────────────┐
│ OnWSBinaryMessage (Event Graph principal)                   │
│                                                              │
│  Data (do WebSocket)                                         │
│    ↓                                                         │
│  ProcessBinaryBuffer (1ª VEZ)                                │
│    - Buffer: Get BinaryMessageBuffer                        │
│    - NewData: Data (do evento)                              │
│    - OutFrame: Get OutFrame                                 │
│    → ReturnValue                                             │
│    ↓                                                         │
│  Branch                                                      │
│    - True → ProcessNextFrame ───────────────────────────────┼─┐
│    - False → (fim - buffer não tem frame completo)           │ │
└──────────────────────────────────────────────────────────────┘ │
                                                                 │
┌──────────────────────────────────────────────────────────────┐ │
│ ProcessNextFrame (Custom Event) ────────────────────────────┼─┘
│                                                              │
│  Get OutFrame                                                │
│    ↓                                                         │
│  Break BinaryFrame → Data                                    │
│    ↓                                                         │
│  ParseStateUpdateFrame                                       │
│    → OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs │
│    ↓                                                         │
│  [Toda sua lógica de atualização]                           │
│    - Get Array Item                                          │
│    - If-Then-Else                                            │
│    - GetOrCreatePlayerState                                  │
│    - UpdatePlayerStateBuffer                                 │
│    - FindPlayerStateIndex                                    │
│    - Array_Set                                               │
│    - Array_Find                                              │
│    - SpawnActorFromClass (se necessário)                     │
│    - SetActorLocation                                        │
│    - SetActorRotation ← ÚLTIMA operação de atualização       │
│    ↓ (then)                                                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ProcessBinaryBuffer (2ª VEZ) ← ADICIONAR AQUI!      │  │
│  │   - Buffer: Get BinaryMessageBuffer                 │  │
│  │   - NewData: Make Array (VAZIO - 0 elementos!)      │  │
│  │   - OutFrame: Get OutFrame                           │  │
│  │   → ReturnValue                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│    ↓ (then)                                                  │
│  Branch                                                      │
│    - Condition: ReturnValue do ProcessBinaryBuffer          │
│    - True → ProcessNextFrame (recursão) ────────────────────┼─┐
│    - False → (fim - não há mais frames)                     │ │
│                                                              │ │
│                                                (volta aqui) │ │
└──────────────────────────────────────────────────────────────┘ │
                                                                  │
                                    (recursão continua se houver │
                                     mais frames no buffer)       │
```

---

## 📖 GUIA VISUAL: O Que É Recursão e Como Conectar

### **🔍 O Que É Recursão?**

**Recursão** significa que uma função chama a **ela mesma**. No Unreal Engine, isso funciona assim:

1. **Custom Event `ProcessNextFrame`** processa um frame
2. Se houver mais frames no buffer, ela **chama a si mesma** novamente
3. Isso continua até não haver mais frames
4. A função para automaticamente

### **🎯 Analogia Simples:**
Imagine uma pilha de livros:
- Você pega o livro do topo (processa 1 frame)
- Se ainda houver livros na pilha, você pega o próximo (chama a função novamente)
- Continua até a pilha estar vazia
- Para quando não há mais livros

### **💡 Por Que Usar Recursão Aqui?**

Quando o servidor envia múltiplos frames (29, 58, 87 bytes), o buffer pode ter:
- **Frame 1:** 29 bytes (completo) ✅
- **Frame 2:** 29 bytes (completo) ✅
- **Frame 3:** 29 bytes (completo) ✅

Após processar o Frame 1, precisamos verificar se há mais frames. A recursão permite processar todos sequencialmente.

---

### **🔌 Como Conectar os Nós Corretamente**

#### **PASSO 1: Estrutura Completa do Branch**

Após o segundo `ProcessBinaryBuffer`, você terá:

```
ProcessBinaryBuffer (2ª VEZ)
    → ReturnValue (bool)
    → then (exec)
```

#### **PASSO 2: Criar o Branch Node**

1. **Arraste do pin `then` do `ProcessBinaryBuffer`** e solte no espaço vazio
2. Procure por **"Branch"** no menu
3. Adicione o node `Branch`

#### **PASSO 3: Conectar os Pins do Branch**

O node `Branch` tem 4 pins:

**Entradas:**
- **`execute`** (exec) ← Conecte o pin `then` do `ProcessBinaryBuffer` aqui
- **`Condition`** (bool) ← Conecte o `ReturnValue` do `ProcessBinaryBuffer` aqui

**Saídas:**
- **`then`** (exec, True) ← Se `ReturnValue == true` (há mais frames)
- **`else`** (exec, False) ← Se `ReturnValue == false` (não há mais frames)

#### **PASSO 4: Conectar a Recursão (Pin True → ProcessNextFrame)**

1. **Arraste do pin `then` (True) do Branch** e solte no espaço
2. Procure pelo evento **"ProcessNextFrame"** no menu (ou digite o nome)
3. Selecione **"Call ProcessNextFrame"** ou **"Add Custom Event" → "ProcessNextFrame"**
4. Conecte o pin `then` (True) do `Branch` ao pin `execute` do `ProcessNextFrame`

#### **PASSO 5: Deixar o Pin False Desconectado**

- **NÃO conecte nada** ao pin `else` (False) do `Branch`
- Isso significa "fim do processamento - não há mais frames"

---

### **📊 Diagrama Visual da Conexão:**

```
┌─────────────────────────────────────────────────────────┐
│ SetActorRotation (última operação)                     │
│    ↓ (then - pin de saída de execução)                │
└─────────────────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ ProcessBinaryBuffer (2ª VEZ)                            │
│    • Buffer: Get BinaryMessageBuffer                   │
│    • NewData: Make Array (VAZIO - 0 elementos!)        │
│    • OutFrame: Get OutFrame                            │
│    → ReturnValue (bool) ────────────┐                  │
│    → then (exec) ────────────────────┼──┐               │
└────────────────────────────────────────┼──────────────┘
                                          │
                      ┌───────────────────┘
                      │
                      ↓
┌─────────────────────────────────────────────────────────┐
│ Branch                                                  │
│    • execute ← pin `then` do ProcessBinaryBuffer        │
│    • Condition ← ReturnValue do ProcessBinaryBuffer     │
│    → then (True) ───────────────┐                       │
│    → else (False) ──────── (NÃO CONECTAR NADA)          │
└──────────────────────────────────┼───────────────────────┘
                                   │
                                   │ Se ReturnValue == true
                                   │ (há mais frames no buffer)
                                   ↓
┌─────────────────────────────────────────────────────────┐
│ ProcessNextFrame (Custom Event)                        │
│    • execute ← pin `then` (True) do Branch              │
│                                                         │
│    [Aqui dentro está toda a lógica de processamento]   │
│    • Get OutFrame                                       │
│    • Break BinaryFrame                                  │
│    • ParseStateUpdateFrame                              │
│    • GetOrCreatePlayerState                             │
│    • UpdatePlayerStateBuffer                            │
│    • SetActorLocation                                   │
│    • SetActorRotation                                   │
│    • ProcessBinaryBuffer (2ª VEZ) ────┐                │
│    • Branch ──────────────────────────┼──┐             │
│    • ProcessNextFrame (chama a si mesmo) │             │
│                                         │              │
└─────────────────────────────────────────┼──────────────┘
                                          │
                          (recursão - volta aqui se houver mais frames)
```

---

### **🎬 Fluxo de Execução Passo a Passo:**

#### **Cenário 1: Buffer tem 1 frame (29 bytes)**

1. `ProcessBinaryBuffer` extrai o frame → `ReturnValue = true`
2. `Branch` recebe `Condition = true`
3. `Branch` executa pin `then` (True)
4. Chama `ProcessNextFrame` (primeira vez)
5. `ProcessNextFrame` processa o frame (SetActorLocation, SetActorRotation)
6. Chama `ProcessBinaryBuffer` novamente (com array vazio)
7. `ProcessBinaryBuffer` verifica buffer → `ReturnValue = false` (não há mais frames)
8. `Branch` recebe `Condition = false`
9. `Branch` executa pin `else` (False) → **Nada acontece (fim)**

#### **Cenário 2: Buffer tem 2 frames (58 bytes)**

1. `ProcessBinaryBuffer` extrai Frame 1 → `ReturnValue = true`
2. Chama `ProcessNextFrame` → processa Frame 1
3. `ProcessNextFrame` chama `ProcessBinaryBuffer` (com array vazio)
4. `ProcessBinaryBuffer` extrai Frame 2 → `ReturnValue = true`
5. `Branch` recebe `Condition = true`
6. Chama `ProcessNextFrame` **NOVAMENTE** (recursão - segunda vez)
7. `ProcessNextFrame` processa Frame 2
8. Chama `ProcessBinaryBuffer` novamente (com array vazio)
9. `ProcessBinaryBuffer` verifica buffer → `ReturnValue = false`
10. `Branch` recebe `Condition = false`
11. **Fim do processamento**

---

### **⚠️ Pontos Importantes:**

1. **O pin `then` (True) do Branch chama o evento `ProcessNextFrame`**, que é o **mesmo evento** que está sendo executado no momento. Isso é recursão.

2. **O pin `else` (False) NÃO deve ser conectado a nada** - significa "pare, não há mais frames".

3. **Cada chamada de `ProcessNextFrame` processa apenas 1 frame** - a recursão permite processar múltiplos frames sequencialmente.

4. **A recursão é segura** porque:
   - Cada chamada processa 1 frame e remove ele do buffer
   - O buffer diminui a cada iteração
   - Quando o buffer fica vazio, `ProcessBinaryBuffer` retorna `false` e a recursão para

---

### **🛠️ Passo a Passo no Unreal Editor:**

1. **Após `SetActorRotation`**, adicione `ProcessBinaryBuffer` (2ª VEZ)
2. **Arraste do pin `then`** do `ProcessBinaryBuffer` → Adicione `Branch`
3. **Conecte:**
   - Pin `then` do `ProcessBinaryBuffer` → Pin `execute` do `Branch`
   - `ReturnValue` do `ProcessBinaryBuffer` → Pin `Condition` do `Branch`
4. **Arraste do pin `then` (True) do Branch** → Digite "ProcessNextFrame" → Selecione o evento
5. **NÃO conecte nada** ao pin `else` (False) do `Branch`
6. **No `Make Array`** do segundo `ProcessBinaryBuffer`, **delete o elemento `[0]`** para deixar vazio

---

## ❓ PERGUNTAS FREQUENTES

### **P: Preciso fazer `ParseStateUpdateFrame` dentro do `ProcessNextFrame` ou só extrair o Data?**

**R:** ✅ **SIM, você DEVE fazer o `ParseStateUpdateFrame` DENTRO do `ProcessNextFrame`!**

Por quê:
- O `Break BinaryFrame` só extrai o `Data` (Array of Bytes)
- O `ParseStateUpdateFrame` converte esses bytes em valores úteis (`OutPlayerId`, `OutLocation`, etc.)
- Você PRECISA desses valores para toda a sua lógica (`GetOrCreatePlayerState`, `UpdatePlayerStateBuffer`, `SetActorLocation`, etc.)

**Fluxo correto:**
1. `Get OutFrame` → obtém o frame extraído
2. `Break BinaryFrame` → extrai o `Data` (29 bytes)
3. `ParseStateUpdateFrame` → converte bytes em valores (playerId, location, yaw, timestamp)
4. Use esses valores na sua lógica de atualização

### **P: Após qual lógica adicionar o segundo `ProcessBinaryBuffer`?**

**R:** ⚠️ **IMPORTANTE:** O segundo `ProcessBinaryBuffer` vai DENTRO do `ProcessNextFrame`, NÃO no `OnWSBinaryMessage`!

Após TODA a lógica de atualização do frame DENTRO do `ProcessNextFrame`:
- Após `ParseStateUpdateFrame`
- Após `GetOrCreatePlayerState`
- Após `UpdatePlayerStateBuffer`
- Após `Array_Set`
- Após `SetActorLocation`
- Após `SetActorRotation` ← **Adicione o segundo `ProcessBinaryBuffer` AQUI!**
- Após o segundo `ProcessBinaryBuffer`, adicione um `Branch` que verifica se há mais frames

**Resumo:** 
- **No `OnWSBinaryMessage`:** Apenas o primeiro `ProcessBinaryBuffer` + `Branch` que chama `ProcessNextFrame`
- **No `ProcessNextFrame`:** Toda a lógica de processamento + segundo `ProcessBinaryBuffer` + `Branch` para recursão

### **P: O que vai dentro do `ProcessNextFrame`?**

**R:** ✅ **SIM, TODA a lógica do `ParseStateUpdateFrame` e toda a lógica de atualização vai DENTRO do `ProcessNextFrame`!**

✅ **DENTRO do `ProcessNextFrame` (tudo relacionado ao processamento de UM frame):**
- Get OutFrame
- Break BinaryFrame
- ParseStateUpdateFrame ← **AQUI DENTRO!**
- Get Array Item (verificar type == 2)
- If-Then-Else (filtrar frames)
- GetOrCreatePlayerState
- UpdatePlayerStateBuffer
- FindPlayerStateIndex
- Array_Set
- Array_Find
- SpawnActorFromClass (se necessário)
- SetActorLocation
- SetActorRotation
- ProcessBinaryBuffer (segunda vez)
- Branch (para recursão)

❌ **FORA do `ProcessNextFrame` (no `OnWSBinaryMessage` - apenas adiciona dados ao buffer):**
- ProcessBinaryBuffer (primeira vez, com Data do evento)
- Branch (que chama ProcessNextFrame se ReturnValue == true)

**Por quê?**
- O `ProcessNextFrame` processa **UM frame por vez**
- Cada vez que é chamado, processa um frame completo (desde o Parse até a atualização)
- Se houver mais frames no buffer, ele chama a si mesmo recursivamente
- Isso é mais seguro e fácil de debugar do que usar `WhileLoop`

### **P: Por que preciso do segundo `ProcessBinaryBuffer`?**

**R:** Porque pode haver múltiplos frames completos no buffer!

- O primeiro `ProcessBinaryBuffer` (no `OnWSBinaryMessage`) adiciona os dados e extrai o PRIMEIRO frame
- Mas pode haver mais frames completos no buffer (29, 58, 87 bytes...)
- O segundo `ProcessBinaryBuffer` (no `ProcessNextFrame`) verifica se há mais frames e extrai o próximo
- Isso continua até não haver mais frames (ReturnValue == false)

#### **PASSO 4: Conectar o Evento Inicial**

No evento `OnWSBinaryMessage`:

1. **Mantenha o primeiro `ProcessBinaryBuffer`:**
   - `Buffer`: Get BinaryMessageBuffer
   - `NewData`: Data (do evento)
   - `OutFrame`: Get OutFrame

2. **Após o primeiro `ProcessBinaryBuffer`:**
   - Adicione um `Branch`
   - Conecte o `ReturnValue` do primeiro `ProcessBinaryBuffer` ao `Condition` do `Branch`
   - **Pin `True`:** Conecte ao pin `execute` do evento `ProcessNextFrame` (inicia o processamento)
   - **Pin `False`:** Não conecte nada (buffer não tem frame completo ainda)

⚠️ **IMPORTANTE:** O segundo `ProcessBinaryBuffer` NÃO vai aqui! Ele vai DENTRO do `ProcessNextFrame`!

#### **ESTRUTURA FINAL COMPLETA:**

```
OnWSBinaryMessage (Data)
    ↓
ProcessBinaryBuffer (primeira vez)
    - Buffer: Get BinaryMessageBuffer
    - NewData: Data (do evento)
    - OutFrame: OutFrame
    → ReturnValue
    ↓
Branch
    - True → ProcessNextFrame (Custom Event) ───────────────────────┐
    - False → [Fim - buffer não tem frame completo ainda]           │
                                                                      │
ProcessNextFrame (Custom Event) ─────────────────────────────────────┘
    ↓
Get OutFrame
    ↓
Break BinaryFrame (Data) ← Extrai Data do frame
    ↓
ParseStateUpdateFrame (Data do Break BinaryFrame) ← CONVERTE bytes em valores
    → OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs
    ↓
Get Array Item (Data[0]) ← Verificar se type == 2
    ↓
If-Then-Else (type == 2 && playerId != localPlayerId) ← Filtrar frames
    ↓ (True)
GetOrCreatePlayerState ← Obter/criar entrada do player
    ↓
UpdatePlayerStateBuffer ← Atualizar posição, yaw, timestamp
    ↓
FindPlayerStateIndex ← Encontrar índice no array
    ↓
Array_Set ← Atualizar RemoteStates
    ↓
Array_Find ← Verificar se RemoteActorId existe
    ↓
If-Then-Else (se não existe)
    ↓ (True)
SpawnActorFromClass ← Criar BP_RemotePlayer
    ↓
SetActorLocation ← Atualizar posição do ator
    ↓
SetActorRotation ← Atualizar rotação do ator
    ↓
ProcessBinaryBuffer (segunda vez) ← Verificar se há mais frames
    - Buffer: Get BinaryMessageBuffer
    - NewData: Make Array (VAZIO!)
    - OutFrame: OutFrame
    → ReturnValue
    ↓
Branch
    - True → ProcessNextFrame (recursão - chama a si mesmo) ────────┐
    - False → [Fim - não há mais frames]                            │
                                                                    │
                                                    (volta para cima)│
```

**📌 Resumo:**
- **`OnWSBinaryMessage`:** Apenas adiciona dados ao buffer e chama `ProcessNextFrame` se houver um frame
- **`ProcessNextFrame`:** Processa UM frame completamente (Parse + toda lógica de atualização + verifica próximo frame)

### **✅ VERIFICAÇÃO FINAL:**

1. ✅ Nenhum `WhileLoop` no Event Graph
2. ✅ Custom Event `ProcessNextFrame` criado
3. ✅ Primeiro `ProcessBinaryBuffer` no `OnWSBinaryMessage` recebe `Data` do evento
4. ✅ Segundo `ProcessBinaryBuffer` no `ProcessNextFrame` recebe `Make Array` VAZIO
5. ✅ `ParseStateUpdateFrame` recebe `Data` do `Break BinaryFrame`, NÃO do evento
6. ✅ O `Make Array` mostra "0 Elements" quando selecionado

---

### **⚠️ ERRO COMUM QUE CAUSA LOOP INFINITO:**
- ❌ **ERRADO:** Conectar `NewData` do segundo `ProcessBinaryBuffer` ao `Data` do evento
- ❌ **ERRADO:** Ter um `Make Array` com `[0] = 0` (isso não é vazio!)
- ✅ **CORRETO:** Conectar `NewData` do segundo `ProcessBinaryBuffer` a um `Make Array` completamente vazio (0 elementos)

### **Como Criar Array Vazio no Blueprint:**
1. Procure por `Make Array` no painel de busca
2. **NÃO adicione nenhum elemento** ao array
3. O array ficará vazio (0 elementos)
4. ⚠️ **CRÍTICO:** Se você vir `[0]` no `Make Array`, clique com botão direito e selecione **"Remove Pin"** para remover esse elemento. Um array com `[0] = 0` NÃO é vazio - tem 1 elemento!

### **🔍 Como Verificar se o Array Está Vazio:**
1. **Selecione o nó `Make Array`**
2. **No painel de detalhes (direita), procure por "Elements"**
3. **Deve mostrar "0 Elements"** - se mostrar "1 Elements" ou mais, você tem um problema!
4. **Se houver elementos listados como `[0]`, `[1]`, etc., clique em cada um e selecione "Remove Pin"**

---

## 📝 RESUMO FINAL

**O problema de loop infinito acontece porque:**
1. ❌ O `MakeArray_2` tem `[0] = 0` (não é vazio - tem 1 elemento)
2. ❌ Quando você passa `NewData` com 1 elemento, o `ProcessBinaryBuffer` adiciona esse elemento ao buffer
3. ❌ Se o buffer já tinha dados, adicionar `[0]` repetidamente pode causar comportamento inesperado
4. ❌ O `WhileLoop` não atualiza a condição dinamicamente dentro do loop

**A solução:**
1. ✅ Use Custom Event `ProcessNextFrame` em vez de `WhileLoop`
2. ✅ Certifique-se de que o `Make Array` está COMPLETAMENTE VAZIO (0 elementos)
3. ✅ O segundo `ProcessBinaryBuffer` deve receber `NewData = array vazio`
4. ✅ Use recursão controlada (Custom Event chama a si mesmo) em vez de loop

**Estrutura recomendada:**
- `OnWSBinaryMessage` → `ProcessBinaryBuffer` (primeira vez, com Data) → `Branch` → `ProcessNextFrame`
- `ProcessNextFrame` → processa frame → `ProcessBinaryBuffer` (segunda vez, com array vazio) → `Branch` → `ProcessNextFrame` (recursão) ou fim

---

## 📝 Notas Importantes

1. **O `ProcessBinaryBuffer` processa UM frame por vez.** Se houver múltiplos frames completos no buffer, o Custom Event `ProcessNextFrame` os processará sequencialmente via recursão.

2. **A variável `BinaryMessageBuffer` é persistente** entre chamadas do evento `OnWSBinaryMessage`, acumulando dados fragmentados.

3. **A variável `OutFrame` deve ser uma variável de instância** (não local), pois precisa persistir entre chamadas recursivas do `ProcessNextFrame`.

4. **O debug "size:25" é normal** ANTES do buffering. Após a correção, você deve ver "size:29" nos logs de sucesso.

5. **⚠️ CRÍTICO:** O segundo `ProcessBinaryBuffer` dentro do `ProcessNextFrame` DEVE receber um array vazio (0 elementos), pois os dados já foram adicionados na primeira chamada. Adicionar os mesmos dados novamente causa loop infinito.

6. **A recursão do `ProcessNextFrame` é segura** porque cada chamada processa apenas um frame e para quando não há mais frames disponíveis. O Unreal Engine gerencia a pilha de chamadas automaticamente.

---

## 🔧 CORREÇÃO URGENTE: Loop Infinito no Make Array

### **Problema Identificado:**
Se o `Make Array` conectado ao `NewData` do segundo `ProcessBinaryBuffer` tiver um elemento `[0] = 0` (mesmo que seja zero), isso causa loop infinito porque sempre há "dados novos" sendo adicionados ao buffer.

### **Sintoma:**
- Cliente recebe "Binary Buffer Process failed" repetidamente
- Mensagens fragmentadas não são processadas corretamente
- Crash ou travamento do cliente

### **Solução Passo a Passo:**

#### **PASSO 1: Verificar o Make Array**
1. Localize o `Make Array` conectado ao pin `NewData` do segundo `ProcessBinaryBuffer` (dentro do `ProcessNextFrame`)
2. Clique com o botão direito no `Make Array` → **"Delete Pin"** ou **"Remove Element [0]"**
3. **Certifique-se de que o array está COMPLETAMENTE VAZIO** (0 elementos)

#### **PASSO 2: Verificar o Branch Após o Segundo ProcessBinaryBuffer**
1. Após o segundo `ProcessBinaryBuffer`, deve haver um `Branch` conectado ao pin `then`
2. O `Branch` deve ter:
   - **Condition:** Conectado ao `ReturnValue` do segundo `ProcessBinaryBuffer`
   - **True:** Conectado ao pin `execute` do evento `ProcessNextFrame` (recursão)
   - **False:** **NÃO CONECTAR NADA** (fim do processamento)

#### **PASSO 3: Estrutura Final Correta**
```
SetActorRotation
    ↓ (then)
ProcessBinaryBuffer (2ª VEZ)
    - Buffer: Get BinaryMessageBuffer
    - NewData: Make Array (VAZIO - sem elementos!)
    - OutFrame: Get OutFrame
    → ReturnValue
    ↓ (then)
Branch
    - Condition: ReturnValue (do ProcessBinaryBuffer)
    - True: → ProcessNextFrame (recursão)
    - False: [NÃO CONECTAR]
```

### **⚠️ Verificação Visual:**
Quando você selecionar o `Make Array`:
- ✅ **CORRETO:** O array mostra "0 Elements" ou não mostra nenhum elemento
- ❌ **ERRADO:** O array mostra "1 Element" ou "[0] = 0"

### **⚠️ Erro Comum:**
- Criar um `Make Array` com um elemento e depois setar esse elemento para `0` **AINDA É UM ARRAY COM 1 ELEMENTO** e causará loop infinito!
- Você precisa **DELETAR** o elemento `[0]` completamente, não apenas setá-lo para zero.

### **🔍 Como Deletar o Elemento no Unreal Editor:**
1. Selecione o `Make Array` node
2. No painel de detalhes (abaixo), você verá a lista de elementos
3. Clique com o botão direito no elemento `[0]` ou use o botão **"-"** (menos) ao lado do elemento
4. **Confirme que o array está vazio** - deve mostrar "0 Elements" ou nenhum elemento listado

### **⚠️ Problema Adicional Identificado:**
Olhando a estrutura do seu Blueprint, o pin `then` do segundo `ProcessBinaryBuffer` **não está conectado** ao `Branch`. Você precisa:

1. **Adicionar um `Branch`** após o segundo `ProcessBinaryBuffer`
2. Conectar o pin `then` do `ProcessBinaryBuffer` ao pin `execute` do `Branch`
3. Conectar o `ReturnValue` do `ProcessBinaryBuffer` ao `Condition` do `Branch`
4. Conectar o pin `True` do `Branch` ao pin `execute` do evento `ProcessNextFrame` (recursão)
5. **Deixar o pin `False` do `Branch` desconectado** (fim do processamento)

---

## ✅ Resultado Esperado

Após aplicar todas as correções:
- ✅ Nenhum erro de compilação
- ✅ Mensagens binárias fragmentadas são acumuladas no buffer
- ✅ Frames completos (29 bytes) são extraídos e processados
- ✅ Múltiplos frames no buffer são processados sequencialmente
- ✅ Logs mostram "Frame Processed: Size=29" em vez de "size:25"

---

## 🔍 ANÁLISE DOS LOGS: Quando "Binary Buffer Process failed" É Normal

### **⚠️ IMPORTANTE:**

Se você ver logs como:
```
Received binary message, size:25expected=29
Binary Buffer Process failed
Received binary message, size:4expected=29
```

**Isso é NORMAL!** Significa que:

1. ✅ **O buffer está funcionando corretamente** - ele está acumulando os dados fragmentados
2. ✅ **O primeiro `ProcessBinaryBuffer` retorna `false`** quando não há 29 bytes ainda - isso é esperado
3. ✅ **O log "Binary Buffer Process failed" aparece** quando o `Branch` no `OnWSBinaryMessage` vai para `False` - isso é o comportamento esperado

### **🎯 Como Saber Se Está Funcionando:**

1. **✅ O buffer acumula dados:**
   - Mensagens fragmentadas chegam: size:25, size:4, etc.
   - Cada mensagem é adicionada ao buffer
   - Quando o buffer atinge 29 bytes, um frame completo é extraído

2. **✅ `ProcessNextFrame` é chamado:**
   - Quando há um frame completo, o `Branch` no `OnWSBinaryMessage` deve ir para `True`
   - Isso chama `ProcessNextFrame`
   - O frame é processado (SetActorLocation, SetActorRotation, etc.)

3. **✅ A recursão funciona:**
   - Após processar um frame, o segundo `ProcessBinaryBuffer` verifica se há mais frames
   - Se houver mais frames, `ProcessNextFrame` é chamado novamente
   - Isso continua até não haver mais frames completos

### **🔴 Quando É Um Problema:**

Se você ver **APENAS** "Binary Buffer Process failed" e **NUNCA**:
- `ProcessNextFrame` sendo chamado
- Players aparecendo no mundo
- SetActorLocation sendo executado

**Então há um problema na estrutura do Blueprint.**

### **✅ Verificação Rápida:**

1. **Adicione um log no início de `ProcessNextFrame`:**
   ```
   Print String: "ProcessNextFrame called!"
   ```

2. **Se esse log aparecer**, significa que a estrutura está funcionando e o buffer está extraindo frames completos.

3. **Se o log NUNCA aparecer**, significa que:
   - O primeiro `ProcessBinaryBuffer` nunca retorna `true`
   - O buffer pode não estar acumulando dados corretamente
   - Verifique se o `BinaryMessageBuffer` está sendo passado como `ref` (não como cópia)

