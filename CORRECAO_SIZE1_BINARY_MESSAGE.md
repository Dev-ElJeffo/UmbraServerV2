# 🔧 Correção: Mensagens Binárias com Size=1 (Fragmentação WebSocket)

## 🔴 Problema Identificado

**Sintoma:**
- `OnWSBinaryMessage` recebe `Data.Num() == 1` (apenas 1 byte)
- Um `StateUpdateFrame` deveria ter **29 bytes**:
  - 1 byte (type = 2)
  - 4 bytes (playerId)
  - 4 bytes (x)
  - 4 bytes (y)
  - 4 bytes (z)
  - 4 bytes (yaw)
  - 4 bytes (tsMs)

**Causa Provável:**
- WebSocket do Unreal Engine pode estar **fragmentando** mensagens
- Callback `OnWSBinaryMessage` sendo chamado múltiplas vezes com fragmentos
- **Blueprints não têm buffer** para concatenar fragmentos automaticamente

## ✅ Correções Aplicadas no Servidor

### 1. Correção do "speed too high"
- **Arquivo:** `src/zone/MovementServer.hpp`
- **Problema:** Primeiro `MoveUpdate` rejeitado por velocidade infinita
- **Solução:** Validação melhorada de timestamps:
  - Se timestamp anterior = 0 ou muito antigo (>10s), usar dt padrão (0.033s)
  - Garantir dt mínimo razoável (>1ms)
  - Ignorar movimentos muito pequenos (<0.1 unidades)

### 2. Limpeza de Base64 no Gateway
- **Arquivo:** `src/gateway/GatewayServer.cpp`
- **Melhorias:**
  - Remove caracteres inválidos (ñ, ¿, etc.) antes de decodificar
  - Validação robusta do JSON extraído
  - Logs melhorados para debug

## ✅ Solução Implementada

### Função C++ de Buffering

Foi adicionada a função `ProcessBinaryBuffer` em `WSBinaryBPFL` que:
1. **Acumula mensagens fragmentadas** em um buffer
2. **Extrai frames completos** (29 bytes cada) automaticamente
3. **Retorna um frame por vez** em loop

**Assinatura:**
```cpp
ProcessBinaryBuffer(
    Buffer: Array of Bytes (ref) - mantido entre chamadas
    NewData: Array of Bytes - novos dados recebidos
    OutFrame: FBinaryFrame (ref) - frame completo extraído (válido apenas se retorno for true)
) → bool - true se um frame completo foi extraído, false caso contrário
```

**Estrutura `FBinaryFrame`:**
```cpp
struct FBinaryFrame {
    TArray<uint8> Data;  // Dados do frame (29 bytes)
}
```

---

## 📋 GUIA PASSO A PASSO - Implementação no Blueprint

### PASSO 1: Criar Variável para o Buffer

1. **Abra o Blueprint `BP_NetMovementClient`**
   - No Unreal Editor, navegue até `Content/Maps/BP_NetMovementClient`
   - Abra o Blueprint (duplo clique)

2. **Adicione uma nova variável:**
   - No painel **My Blueprint** (lado esquerdo), clique em **"+ Variable"**
   - **Nome da variável:** `BinaryMessageBuffer`
   - **Tipo:** `Array` → selecione `Byte` (não `Integer`, deve ser `Byte`)
   - **Editable:** Desmarcado (não precisa ser editável)
   - **Category:** Opcional (ex: "WebSocket")

3. **Configure a variável:**
   - Certifique-se de que o tipo está como `Array of Byte` (Array de Byte)
   - O valor padrão deve estar vazio (array vazio)

---

### PASSO 2: Preparar o Event `OnWSBinaryMessage`

1. **Localize o Event `OnWSBinaryMessage`:**
   - No **Event Graph**, procure pelo evento `OnWSBinaryMessage`
   - Este evento já deve estar conectado ao delegate do WebSocket
   - O evento tem um parâmetro `Data` do tipo `Array of Byte`

2. **Remova ou comente a lógica antiga (temporariamente):**
   - Você precisará substituir a chamada direta de `ParseStateUpdateFrame`
   - Mantenha a estrutura do evento, mas vamos adicionar o buffering primeiro

---

### PASSO 3: Adicionar Print String de Debug (Opcional mas Recomendado)

**Este print ajudará a confirmar que os dados estão chegando fragmentados.**

1. **Criar o Print String:**
   - Clique com botão direito no **Event Graph**
   - Digite: `Print String`
   - Selecione `Print String` (da categoria "Utilities|Debug")

2. **Criar os nós necessários para o debug:**

   **a) Obter o tamanho do array `Data`:**
   - Clique direito → digite `Length`
   - Selecione `Length (Array Integer)` ou use o nó `Get (Array Element)` se necessário
   - **Alternativa mais direta:**
     - Conecte `Data` (saída do evento) a um nó que tenha um pin de saída para o tamanho
     - No Unreal, você pode usar um nó `Break Array` ou simplesmente usar o nó `Array Length`
     - **Como obter:** Clique direito → digite "Array Length" ou "Get Array Length"
     - Se não aparecer, use: Clique direito → `Utilities` → `Array` → `Length`
   - **Conexão:** `Data` → entrada do nó `Length`
   - A saída será um `Integer` com o número de elementos no array

   **b) Converter Integer para String:**
   - Clique direito → digite `ToString`
   - Selecione `To String (Integer)` ou `ToString (Integer)`
   - **Conexão:** Saída do nó `Length` → entrada do `ToString`

   **c) Criar string concatenada:**
   - Clique direito → digite `Append`
   - Selecione `Append String` ou use o operador `+` de string
   - **Alternativa:** Use `Make String` com placeholders, ou múltiplos `Append String`
   
   **Montagem do Print de Debug:**
   ```
   OnWSBinaryMessage (Data)
       ↓
   [Array Length] (Data → Length)
       → Output (Integer)
       ↓
   [To String] (Integer → String)
       → Return Value (String)
       ↓
   [Append String]
       - String 1: "📥 Binary received: size="
       - String 2: [Saída do To String]
       → Return Value (String)
       ↓
   [Append String] (novo)
       - String 1: [Saída do Append anterior]
       - String 2: ", expected=29"
       → Return Value (String)
       ↓
   [Print String]
       - In String: [Saída do último Append]
       - bPrintToScreen: true
       - bPrintToLog: true
   ```

   **Versão mais simples (sem múltiplos Append):**
   - Use `Format Text` em vez de múltiplos `Append`:
     - Clique direito → digite `Format Text`
     - Selecione `Format Text`
     - Configure o formato: `"📥 Binary received: size={0}, expected=29"`
     - No campo de argumentos, adicione um argumento `{0}` e conecte a saída do `To String`

3. **Adicionar verificação do primeiro byte (opcional):**
   - Se `Data.Num() > 0`, você pode obter o primeiro byte:
     - Clique direito → digite `Get`
     - Selecione `Get (Array Element)` ou `Get Element`
     - **Conexão:** `Data` → `Array` (entrada do Get)
     - **Index:** `0` (valor fixo)
     - **Output:** `Element` (Byte)
     - Converta para String e adicione ao print:
       ```
       [Get Element] (Data, Index=0)
           → Element (Byte)
           ↓
       [To String] (Byte → String)
           → Return Value (String)
           ↓
       [Format Text] "... byte0={0}"
       ```

**Resultado do Print:**
Quando executar, você verá no log e na tela:
```
📥 Binary received: size=1, expected=29
📥 Binary received: size=28, expected=29
📥 Binary received: size=29, expected=29
```

Isso confirma se os dados estão chegando fragmentados (size=1, 28, etc.) ou completos (size=29).

---

### PASSO 4: Implementar o Buffering com ProcessBinaryBuffer

1. **Localizar a função `ProcessBinaryBuffer`:**
   - Clique direito no **Event Graph**
   - Digite: `Process Binary Buffer` ou `ProcessBinaryBuffer`
   - Procure na categoria: `Umbra|Net|WS|Binary`
   - Selecione `Process Binary Buffer`

2. **Criar variável local para `OutFrame`:**
   - No **Event Graph**, você pode usar diretamente um pin de saída ou criar uma variável local
   - **Opção 1 (recomendada):** Use diretamente o pin `OutFrame` sem variável intermediária
   - **Opção 2:** Crie uma variável local (se quiser mais controle):
     - Não é necessário criar variável no "My Blueprint"
     - O Blueprint criará automaticamente um pin de saída `OutFrame`

3. **Criar o loop While:**
   - Clique direito → digite `While Loop`
   - Selecione `While Loop`
   - Este loop executará enquanto `ProcessBinaryBuffer` retornar `true`

4. **Conectar os nós - PRIMEIRA ITERAÇÃO:**

   **a) Conexão inicial:**
   ```
   OnWSBinaryMessage (Data: Array of Byte)
       ↓ (execution pin)
   [While Loop]
       - Loop Body: [conectado internamente]
   ```

   **b) Dentro do While Loop:**
   ```
   [While Loop Body] (execution pin)
       ↓
   [ProcessBinaryBuffer]
       - Buffer: [Variável Get BinaryMessageBuffer]
       - NewData: [Data - apenas primeira vez!]
       - OutFrame: [será preenchido automaticamente]
       → Return Value (bool)
       ↓ (execution then)
   [Branch]
       - Condition: [Return Value do ProcessBinaryBuffer]
       - True: [processar frame]
       - False: [sair do loop]
   ```

   **c) Obter a variável `BinaryMessageBuffer`:**
   - Clique direito → digite `Get Binary Message Buffer` ou `Get`
   - Ou arraste `BinaryMessageBuffer` do painel **My Blueprint** para o gráfico
   - **Conexão:** Saída do `Get BinaryMessageBuffer` → entrada `Buffer` (ref) do `ProcessBinaryBuffer`
   - ⚠️ **IMPORTANTE:** O pin `Buffer` é `ref` (referência), então conecte diretamente

   **d) Para `NewData` (primeira iteração):**
   - Na **primeira vez dentro do loop**, conecte `Data` do evento `OnWSBinaryMessage`
   - Use um nó `Knot` (nó de redistribuição) se necessário para organizar:
     - Clique direito → digite `Knot`
     - Selecione `Reroute Node` ou `Knot`
     - Conecte `Data` → Knot → `NewData`

5. **PROBLEMA: Como passar `Data` apenas na primeira iteração?**

   **Solução: Use uma variável booleana para controlar:**
   
   **a) Criar variável `IsFirstCall`:**
   - No **My Blueprint**, crie uma variável:
     - Nome: `IsFirstCall`
     - Tipo: `Boolean`
     - Valor padrão: `true`
   
   **b) No Event Graph:**
   ```
   OnWSBinaryMessage
       ↓
   [Branch]
       - Condition: [Get IsFirstCall]
       - True: [Data do evento]
       - False: [Make Array vazio]
       ↓ (ambos os caminhos)
   [While Loop]
       ...
   ```

   **c) Criar Array vazio:**
   - Clique direito → digite `Make Array`
   - Selecione `Make Array` (tipo `Byte`)
   - Deixe vazio (sem elementos)
   - Ou use `Clear Array` em uma cópia

   **d) Alternativa mais simples:**
   - Use um `Select` node:
     - Clique direito → digite `Select`
     - Selecione `Select` (tipo `Array of Byte`)
     - **Index:** Conecte `Get IsFirstCall` (converta para int se necessário)
     - **A[0] (True):** `Data` do evento
     - **A[1] (False):** `Make Array` vazio

   **e) Após primeira chamada, marcar `IsFirstCall = false`:**
   ```
   [Set IsFirstCall] (após primeira iteração do ProcessBinaryBuffer)
       - IsFirstCall: false
   ```

   **⚠️ ALTERNATIVA AINDA MAIS SIMPLES (Recomendada):**
   
   **Não use controle de primeira chamada!** Passe `Data` apenas uma vez antes do loop:
   
   ```
   OnWSBinaryMessage (Data)
       ↓
   [Append Array] (ou use diretamente no primeiro ProcessBinaryBuffer)
       - Target Array: [Get BinaryMessageBuffer]
       - New Items: [Data]
   ```
   
   Mas isso não funciona bem porque `Append Array` não retorna o array modificado para uso no `ProcessBinaryBuffer`.
   
   **MELHOR SOLUÇÃO:**
   
   **Adicione `Data` ao buffer ANTES do loop:**
   ```
   OnWSBinaryMessage (Data)
       ↓
   [Append Array to Array] ou [Append]
       - Target: [Get BinaryMessageBuffer] → Set BinaryMessageBuffer
       - Array: [Data]
       ↓
   [While Loop]
       ↓
   [ProcessBinaryBuffer]
       - Buffer: [Get BinaryMessageBuffer]
       - NewData: [Make Array vazio - sempre vazio no loop!]
       - OutFrame: [variável local ou pin de saída]
       → Return Value (bool)
   ```

   **⚠️ ATENÇÃO:** O `ProcessBinaryBuffer` espera `NewData` para adicionar ao buffer. Se você já adicionou `Data` antes do loop, passe array vazio no loop!

   **SOLUÇÃO FINAL RECOMENDADA:**
   
   **Não adicione `Data` manualmente. Deixe o `ProcessBinaryBuffer` fazer isso:**
   
   ```
   OnWSBinaryMessage (Data)
       ↓
   [Set IsFirstCall = true] (resetar)
       ↓
   [While Loop]
       ↓
   [Branch: IsFirstCall]
       True: NewData = Data
       False: NewData = [Make Array vazio]
       ↓
   [ProcessBinaryBuffer]
       - Buffer: [Get BinaryMessageBuffer]
       - NewData: [saída do Branch]
       - OutFrame: [local]
       → Return Value (bool)
       ↓
   [Set IsFirstCall = false] (após primeira chamada)
       ↓
   [Branch: Return Value == true]
       True: [processar frame]
       False: [Break Loop]
   ```

---

### PASSO 5: Processar o Frame Extraído

1. **Após extrair o frame (Return Value == true):**

   ```
   [Branch: Return Value == true]
       True (execution)
           ↓
       [ParseStateUpdateFrame]
           - Data: [OutFrame.Data]
           - OutPlayerId: [variável local ou pin]
           - OutLocation: [variável local ou pin]
           - OutYawDegrees: [variável local ou pin]
           - OutTimestampMs: [variável local ou pin]
           → Return Value (bool)
           ↓ (True)
       [Branch: Return Value == true]
           True: [continuar processamento normal...]
   ```

2. **Acessar `OutFrame.Data`:**
   - O nó `ProcessBinaryBuffer` terá um pin de saída `OutFrame` (tipo `FBinaryFrame`)
   - Use um nó `Break Struct` para acessar o campo `Data`:
     - Clique direito → digite `Break`
     - Selecione `Break Binary Frame` ou `Break FBinaryFrame`
     - **Conexão:** `OutFrame` (saída do `ProcessBinaryBuffer`) → entrada do `Break`
     - **Saída:** `Data` (Array of Byte)

   **Ou use diretamente:**
   - No Unreal Engine 5, você pode expandir o pin `OutFrame` e acessar `Data` diretamente
   - Se o pin não expandir, use `Break Struct`

3. **Continuar com a lógica existente:**
   - Após `ParseStateUpdateFrame` retornar `true`, continue com a lógica de atualização de player state
   - Use os outputs (`OutPlayerId`, `OutLocation`, etc.) normalmente

---

### PASSO 6: Estrutura Completa Final

**Resumo da estrutura completa:**

```
OnWSBinaryMessage (Data: Array of Byte)
    ↓
[Print String] (Debug - opcional)
    "📥 Binary received: size=" + ToString(Array Length(Data)) + ", expected=29"
    ↓
[Set IsFirstCall = true]
    ↓
[While Loop]
    ↓
    [Branch: IsFirstCall]
        True → NewData = Data
        False → NewData = [Make Array vazio]
        ↓ (ambos)
    [ProcessBinaryBuffer]
        - Buffer: [Get BinaryMessageBuffer] (ref)
        - NewData: [saída do Branch]
        - OutFrame: [local - tipo FBinaryFrame]
        → Return Value: bool
        ↓ (execution then)
    [Set IsFirstCall = false]
        ↓
    [Branch: Return Value == true]
        True:
            ↓
            [Break Binary Frame] (OutFrame)
                → Data: Array of Byte
                ↓
            [ParseStateUpdateFrame]
                - Data: [Data do Break]
                - OutPlayerId: [local]
                - OutLocation: [local]
                - OutYawDegrees: [local]
                - OutTimestampMs: [local]
                → Return Value: bool
                ↓ (True)
            [Branch: Return Value == true]
                True:
                    ↓
                    [Verificar se é próprio player] (já existente)
                        ↓
                    [GetOrCreatePlayerState] (já existente)
                        ↓
                    [UpdatePlayerStateBuffer] (já existente)
                        ↓
                    [Resto da lógica existente...]
        False:
            [Break Loop] (sair do While Loop)
```

---

### PASSO 7: Verificação e Teste

1. **Compile o Blueprint:**
   - Clique em `Compile` no topo do editor
   - Verifique se há erros

2. **Execute o jogo:**
   - Execute em PIE (Play In Editor)
   - Conecte ao WebSocket
   - Observe os prints de debug (se adicionados)

3. **Verificar logs:**
   - Se `size=1` aparecer, mas depois aparecer `size=29`, o buffering está funcionando
   - Se apenas `size=29` aparecer, os dados já estão chegando completos

---

## 🔍 Debug Imediato - Explicação Detalhada

### Print String de Debug - Passo a Passo

O print de debug serve para **confirmar visualmente** que os dados estão chegando fragmentados. Vamos montá-lo passo a passo:

#### 1. Criar o Nó Array Length

**Como obter:**
- Clique direito no Event Graph
- Digite: `Array Length`
- Selecione `Array Length` (geralmente aparece como `Length` na categoria `Utilities`)

**Conexão:**
- `Data` (do evento `OnWSBinaryMessage`) → `Array` (entrada do `Array Length`)
- Saída: `Length` (Integer) - número de elementos no array

#### 2. Converter Integer para String

**Como obter:**
- Clique direito → digite `To String`
- Selecione `To String (Integer)` ou `ToString`

**Conexão:**
- `Length` (do `Array Length`) → entrada do `To String`
- Saída: `Return Value` (String)

#### 3. Concatenar Strings

**Opção A - Usando Append String (múltiplos):**

**Primeiro Append:**
- Clique direito → digite `Append String`
- Selecione `Append String`
- **String 1:** Digite manualmente: `"📥 Binary received: size="`
- **String 2:** Conecte a saída do `To String`
- **Saída:** `Return Value` (String)

**Segundo Append:**
- Crie outro `Append String`
- **String 1:** Conecte a saída do primeiro `Append String`
- **String 2:** Digite manualmente: `", expected=29"`
- **Saída:** `Return Value` (String) - esta será a string final

**Opção B - Usando Format Text (recomendado):**

**Format Text:**
- Clique direito → digite `Format Text`
- Selecione `Format Text`
- **Format Pattern:** Digite: `"📥 Binary received: size={0}, expected=29"`
- **Argumentos:**
  - Clique em `+` para adicionar argumento
  - Tipo: `Integer`
  - Conecte a saída `Length` do `Array Length` ao argumento `{0}`
- **Saída:** `Result` (String)

#### 4. Criar Print String

**Como obter:**
- Clique direito → digite `Print String`
- Selecione `Print String`

**Conexão:**
- A string final (do `Append` ou `Format Text`) → `In String` (entrada do `Print String`)
- **Configurações:**
  - `bPrintToScreen`: `true` (aparecerá na tela do jogo)
  - `bPrintToLog`: `true` (aparecerá no log do editor)
  - `Duration`: `2.0` segundos (quanto tempo aparece na tela)
  - `TextColor`: Opcional (cor do texto na tela)

#### 5. Estrutura Completa do Print de Debug

```
OnWSBinaryMessage (Data: Array of Byte)
    ↓ (execution)
[Array Length]
    - Array: Data (do evento)
    → Length: Integer
    ↓
[Format Text]
    - Format Pattern: "📥 Binary received: size={0}, expected=29"
    - {0}: Length (Integer)
    → Result: String
    ↓
[Print String]
    - In String: Result (do Format Text)
    - bPrintToScreen: true
    - bPrintToLog: true
    - Duration: 2.0
```

#### 6. Interpretação dos Resultados

**Cenário 1 - Fragmentação confirmada:**
```
📥 Binary received: size=1, expected=29
📥 Binary received: size=28, expected=29
📥 Binary received: size=1, expected=29
```
→ **Solução:** Implementar o buffering (ProcessBinaryBuffer)

**Cenário 2 - Dados completos:**
```
📥 Binary received: size=29, expected=29
📥 Binary received: size=29, expected=29
```
→ **Não precisa de buffering**, mas não faz mal implementar

**Cenário 3 - Dados inconsistentes:**
```
📥 Binary received: size=30, expected=29
📥 Binary received: size=58, expected=29
```
→ **Múltiplos frames chegando juntos** - o buffering ainda resolve, processando frame por frame

---

## 🎯 Resumo Final

### O que foi implementado:
1. ✅ Função C++ `ProcessBinaryBuffer` para acumular fragmentos
2. ✅ Estrutura `FBinaryFrame` para encapsular frames completos
3. ✅ Lógica de loop para processar múltiplos frames

### O que você precisa fazer no Blueprint:
1. ✅ Criar variável `BinaryMessageBuffer` (Array of Byte)
2. ✅ Modificar `OnWSBinaryMessage` para usar `ProcessBinaryBuffer` em loop
3. ✅ Processar cada frame extraído com `ParseStateUpdateFrame`
4. ✅ (Opcional) Adicionar Print String de debug

### Como testar:
1. Execute o jogo
2. Conecte ao WebSocket
3. Observe os prints (se adicionados)
4. Verifique se os players aparecem uns aos outros

---

**Data:** 2025-11-01  
**Status:** Implementação completa - Aguardando teste no Blueprint
