# 📘 **GUIA COMPLETO: Correções Detalhadas do Blueprint**

Este documento fornece instruções passo a passo para corrigir todos os problemas identificados no Blueprint `BP_NetMovementClient`.

---

## 📋 **ÍNDICE DE PROBLEMAS:**

1. **Player ID Duplicado/Incorreto** (19660867)
2. **Divide by Zero** (SendRateHz = 0)
3. **Spawn na Posição Errada** (0,0,0)
4. **Múltiplos Pawns Spawnados** (não reutilizados)
5. **Filtro de Tipo Invertido** (type == 2)

---

## 🔧 **CORREÇÃO 1: Proteger Divisão SendRateHz (Divide by Zero)**

### **📍 LOCALIZAÇÃO:**
- **Event Graph** → **OnWSConnected** (Custom Event)

### **🎯 PROBLEMA:**
- A variável `SendRateHz` já existe e está configurada com valor padrão **20.0 Hz** ✅
- O timer está sendo configurado no evento `OnWSConnected` ✅
- **Porém**, a divisão `1.0 / SendRateHz` não tem proteção contra valores zero
- Se `SendRateHz` for modificado ou resetado para `0.0`, causará **divide by zero**

### **✅ SOLUÇÃO PASSO A PASSO:**

#### **PASSO 1.1: Localizar o Nó Divide no OnWSConnected**

1. Abra o Blueprint `BP_NetMovementClient`
2. Vá para o **Event Graph**
3. Localize o **Custom Event "OnWSConnected"**
4. Encontre a sequência:
   ```
   Set IsConnected = true
     ↓
   Print String: "WebSocket Connected!"
     ↓
   Get SendRateHz
     ↓
   Divide (÷) ← ESTE NÓ!
     - A: 1.0
     - B: SendRateHz (conectado)
     ↓
   Set Timer by Function Name
     - Time: (resultado da divisão)
   ```

#### **PASSO 1.2: Adicionar Proteção com Max**

1. **Localize a conexão entre `Get SendRateHz` e o nó `Divide`**
   - Atualmente: `Get SendRateHz` → `B` (divisor) do `Divide`

2. **Desconecte temporariamente:**
   - Clique no pin `SendRateHz` do `Get SendRateHz` e desconecte do pin `B` do `Divide`

3. **Adicione nó "Max":**
   - **Clique direito** no espaço entre `Get SendRateHz` e `Divide`
   - Busque: **"Max (Double Double)"** ou **"Max (Float Float)"**
   - Tipo depende do tipo do `SendRateHz` (geralmente Float)

4. **Conecte as entradas do Max:**
   - **Entrada A:** Conecte `SendRateHz` (saída do `Get SendRateHz`)
   - **Entrada B:** Clique no campo e digite `1.0`
     - **Explicação:** Garante que o valor mínimo sempre será 1.0, evitando divisão por zero

5. **Conecte a saída do Max:**
   - **Saída ReturnValue:** Conecte ao pin `B` (divisor) do nó `Divide`
   - Isso garante que o divisor nunca será zero, mesmo que `SendRateHz` seja modificado

#### **PASSO 1.3: Verificar Valores (Opcional - Debug)**

1. **Se quiser adicionar um log para confirmar:**
   - Após `Set Timer by Function Name`, adicione **Print String**
   - **Configure:**
     - Use **Append String** para criar mensagem:
       - `A`: "Timer interval set to: "
       - `B`: Resultado do `Divide` (conecte usando outro `Append String` ou formatação)
   - **Nota:** Isso é opcional, apenas para debug

#### **📐 ESTRUTURA VISUAL:**

**ANTES (Sem Proteção):**
```
OnWSConnected
  ↓ (execute)
Set IsConnected = true
  ↓ (then)
Print String: "WebSocket Connected!"
  ↓ (then)
Get SendRateHz (20.0)
  ↓ (SendRateHz)
Divide (÷)
  - A: 1.0
  - B: SendRateHz (20.0) ← Sem proteção!
  - ReturnValue: 0.05
  ↓
Set Timer by Function Name
  - Time: 0.05
  - Function: "SendMoveUpdate"
  - Looping: true
```

**DEPOIS (Com Proteção):**
```
OnWSConnected
  ↓ (execute)
Set IsConnected = true
  ↓ (then)
Print String: "WebSocket Connected!"
  ↓ (then)
Get SendRateHz (20.0)
  ↓ (SendRateHz)
Max (Max Double Double)
  - A: SendRateHz (20.0)
  - B: 1.0
  - ReturnValue: 20.0 (ou mínimo 1.0) ← Proteção!
  ↓ (ReturnValue)
Divide (÷)
  - A: 1.0
  - B: Max_Result (20.0) ← Protegido!
  - ReturnValue: 0.05
  ↓
Set Timer by Function Name
  - Time: 0.05
  - Function: "SendMoveUpdate"
  - Looping: true
```

#### **📝 OBSERVAÇÕES IMPORTANTES:**

- **O valor padrão 20.0 Hz está correto!** Não precisa mudar.
- **A proteção `Max` é uma segurança adicional** caso a variável seja modificada em runtime
- Com `Max`, mesmo se `SendRateHz` for `0.0`, o divisor será `1.0`, resultando em intervalo de `1.0` segundo (seguro, mas lento)
- Se `SendRateHz` for `20.0`, o `Max` retorna `20.0`, mantendo o comportamento normal

#### **PASSO 1.4: Proteção Global no BeginPlay (RECOMENDADO se o erro persistir)**

**Se ainda houver erros de "divide by zero" após corrigir o timer**, significa que `SendRateHz` pode estar sendo resetado para `0.0` em runtime. Para garantir que isso nunca aconteça:

1. **No `BeginPlay`, adicione verificação:**
   - **Clique direito** após `Event BeginPlay`
   - Busque: **"Branch"**
   - Conecte: `Event BeginPlay` (execute) → `Branch` (execute)

2. **Criar condição de comparação:**
   - **Clique direito** → Busque: **"<= (Less or Equal)"** ou **"Compare Double"**
   - **Entrada A:** Conecte `SendRateHz` (de `Get SendRateHz`)
   - **Entrada B:** Digite `0.0`
   - **Saída ReturnValue:** Conecte ao pin `Condition` do `Branch`

3. **Se SendRateHz <= 0 (True):**
   - Conecte pin `true` do `Branch` → pin `execute` de `Set SendRateHz`
   - No campo `SendRateHz`, digite: **`20.0`**

4. **Estrutura:**
   ```
   BeginPlay
     ↓
   Get SendRateHz
     ↓ (SendRateHz)
   <= (Less or Equal)
     - A: SendRateHz
     - B: 0.0
     ↓ (ReturnValue)
   Branch
     - Condition: (SendRateHz <= 0?)
     ↓ (true)
   Set SendRateHz = 20.0
   ```

---

## 🔧 **CORREÇÃO 2: Corrigir Player ID (MyPlayerId)**

### **📍 LOCALIZAÇÃO:**
- **Event Graph** → **BeginPlay**

### **🎯 PROBLEMA:**
- `MyPlayerId` não está sendo inicializado corretamente
- Valores incorretos sendo enviados (ex: 19660867)

### **✅ SOLUÇÃO PASSO A PASSO:**

#### **PASSO 2.1: Adicionar Get Game Instance**

1. No `BeginPlay`, após inicializar `SendRateHz`, adicione:
   - **Clique direito** → Busque: **"Get Game Instance"**
   - Categoria: `Game` ou `Utilities`

2. **Conecte:**
   - Pin `then` do `Set SendRateHz` → Pin `execute` (se houver) ou apenas use a saída `ReturnValue`

#### **PASSO 2.2: Cast para UmbraGameInstance**

1. **Adicione um nó Cast:**
   - **Clique direito** → Busque: **"Cast to UmbraGameInstance"**
   - Ou: **"Cast to ..."** → Selecione `UmbraGameInstance` da lista

2. **Conecte:**
   - `ReturnValue` do `Get Game Instance` → Pin `Object` do `Cast to UmbraGameInstance`

3. **Resultado do Cast:**
   - Se o cast for bem-sucedido: pin `As Umbra Game Instance`
   - Se falhar: pin `Cast Failed` (não conecte nada aqui)

#### **PASSO 2.3: Obter Active Player ID**

1. **Com o `As Umbra Game Instance` conectado:**
   - **Clique direito** no pin `As Umbra Game Instance`
   - Busque: **"Get Active Player ID"**
   - Ou: Selecione a função na lista de funções disponíveis

2. **Saída:**
   - `ReturnValue` (int32): ID do player ativo

#### **PASSO 2.4: Setar MyPlayerId**

1. **Adicione:** **"Set MyPlayerId"**
   - **Clique direito** → Busque: **"Set MyPlayerId"**
   - Ou: Arraste a variável `MyPlayerId` do painel de variáveis e selecione **"Set"**

2. **Conecte:**
   - Pin `ReturnValue` do `Get Active Player ID` → Pin `MyPlayerId` do `Set MyPlayerId`
   - Pin `then` do `Cast to UmbraGameInstance` → Pin `execute` do `Set MyPlayerId`

#### **PASSO 2.5: Adicionar Log de Confirmação**

1. Após `Set MyPlayerId`, adicione **Print String**
2. **Configure:**
   - Use **Append String** para concatenar:
     - `A`: "MyPlayerId set to: "
     - `B`: `MyPlayerId` (de `Get MyPlayerId` após o set)

#### **📐 ESTRUTURA VISUAL:**

```
Event BeginPlay
  ↓
Set SendRateHz = 30.0
  ↓ (then)
Get Game Instance
  ↓ (ReturnValue)
Cast to UmbraGameInstance
  ↓ (As Umbra Game Instance, then)
Get Active Player ID
  ↓ (ReturnValue)
Set MyPlayerId
  ↓ (then)
Print String: "MyPlayerId set to: " + MyPlayerId
```

---

## 🔧 **CORREÇÃO 3: Usar Posição Correta no Spawn**

### **📍 LOCALIZAÇÃO:**
- **Event Graph** → **ProcessNextFrame** (Custom Event)

### **🎯 PROBLEMA:**
- `SpawnActorFromClass` está usando posição (0,0,0) em vez da posição recebida do servidor
- Erro: `SpawnActor failed because of collision at [X=0.000 Y=0.000 Z=0.000]`

### **✅ SOLUÇÃO PASSO A PASSO:**

#### **PASSO 3.1: Localizar ParseStateUpdateFrame**

1. No `ProcessNextFrame`, localize o nó **"ParseStateUpdateFrame"**
   - Este nó deve estar após `Break BinaryFrame`

2. **Saídas disponíveis:**
   - `OutPlayerId` (int32)
   - `OutLocation` (FVector) ← **USE ESTA!**
   - `OutYawDegrees` (float) ← **USE ESTA!**
   - `OutTimestampMs` (int32)
   - `ReturnValue` (bool)

#### **PASSO 3.2: Criar Transform com Posição e Rotação Corretas**

1. **Adicione nó "Make Transform":**
   - **Clique direito** → Busque: **"Make Transform"**
   - Categoria: `Transform`

2. **Conecte as entradas:**
   - **Location:**
     - Conecte `OutLocation` (FVector) do `ParseStateUpdateFrame`
     - **Explicação:** Esta é a posição recebida do servidor
   
   - **Rotation:**
     - Você precisa criar um `FRotator` a partir do `OutYawDegrees`
     - **Adicione nó "Make Rotator":**
       - **Clique direito** → Busque: **"Make Rotator"**
       - **Entradas:**
         - `Yaw`: Conecte `OutYawDegrees` (float)
         - `Pitch`: Digite `0.0`
         - `Roll`: Digite `0.0`
       - **Saída:**
         - `ReturnValue` (Rotator): Conecte ao pin `Rotation` do `Make Transform`
   
   - **Scale:**
     - Digite `1.0` para X, Y e Z (ou use o valor padrão)

3. **Saída do Make Transform:**
   - `ReturnValue` (Transform): Use esta no `SpawnActorFromClass`

#### **PASSO 3.3: Usar Transform no SpawnActorFromClass**

1. **Localize o nó "SpawnActorFromClass":**
   - Deve estar após `GetOrCreatePlayerState`

2. **No nó SpawnActorFromClass:**
   - Encontre o pin **"Transform"** ou **"Spawn Transform"**
   - **Desconecte** qualquer conexão existente (se houver)
   - **Conecte:** `ReturnValue` (Transform) do `Make Transform` criado acima

3. **Configuração adicional:**
   - Pin **"Spawn Collision Handling Override":**
     - Selecione: **"Always Spawn"**
     - Isso evita erros de colisão mesmo se houver algum objeto na posição

#### **PASSO 3.4: Adicionar Logs de Debug (Opcional)**

1. Antes do `SpawnActorFromClass`, adicione logs:
   - **Print String:**
     - `"Spawning at Location: " + OutLocation`
     - `"With Yaw: " + OutYawDegrees`

#### **📐 ESTRUTURA VISUAL:**

```
ParseStateUpdateFrame
  ↓ (then)
  ├─ OutLocation (FVector) ───────────┐
  ├─ OutYawDegrees (float) ──┐        │
  └─ OutPlayerId (int32)              │
                                      │
Make Rotator                         │
  - Yaw: OutYawDegrees                │
  - Pitch: 0.0                        │
  - Roll: 0.0                         │
  ↓ (ReturnValue: Rotator)            │
                                      │
Make Transform ───────────────────────┘
  - Location: OutLocation (FVector)
  - Rotation: Make Rotator ReturnValue (Rotator)
  - Scale: (1.0, 1.0, 1.0)
  ↓ (ReturnValue: Transform)
  
SpawnActorFromClass
  - Class: BP_RemotePlayer_C
  - Transform: Make Transform ReturnValue
  - Spawn Collision Handling Override: Always Spawn
```

---

## 🔧 **CORREÇÃO 4: Prevenir Spawn Duplicado (Reutilizar Atores)**

### **📍 LOCALIZAÇÃO:**
- **Event Graph** → **ProcessNextFrame**

### **🎯 PROBLEMA:**
- Um novo ator é spawnado para cada `StateUpdate` recebido
- Atores existentes não são reutilizados

### **✅ SOLUÇÃO PASSO A PASSO:**

#### **PASSO 4.1: Criar Estrutura para Rastrear Atores Remotos**

1. **Criar um Array de Atores:**
   - No painel **"My Blueprint"** (lado esquerdo)
   - Clique em **"+"** para adicionar nova variável
   - Nome: **"RemoteActors"**
   - Tipo: **Array**
   - Tipo do Array: **Actor Reference** (ou `Object Reference` → `Actor`)
   - **Alternativa:** Criar um Array de Structs (mais robusto):
     - Crie um Struct `FRemoteActorEntry`:
       - `PlayerId` (int32)
       - `ActorRef` (Actor Reference)
     - Array: `RemoteActorsArray` (Array of FRemoteActorEntry)

#### **PASSO 4.2: Criar Função Helper para Buscar Ator (Opcional)**

**Se usar Array simples de Actor References:**
- Você precisará manter um mapeamento manual (usar `Find Player State Index` como referência)

**Se usar Array de Structs (Recomendado):**
1. Crie uma **Custom Function**: **"FindRemoteActor"**
   - **Input:** `PlayerId` (int32)
   - **Output:** `ActorRef` (Actor Reference) ou `null` se não encontrado

2. **Implementação da função:**
   ```
   Loop através de RemoteActorsArray:
     Para cada elemento:
       Se element.PlayerId == Input PlayerId:
         Retornar element.ActorRef
     Retornar null (não encontrado)
   ```

#### **PASSO 4.3: Modificar ProcessNextFrame para Verificar Ator Existente**

1. **Após `GetOrCreatePlayerState`**, adicione uma verificação:

2. **Se usar Array simples:**
   - Você precisa manter um índice correspondente entre `RemoteStates` e `RemoteActors`
   - Use `FindPlayerStateIndex` para encontrar o índice
   - Use esse índice para buscar no `RemoteActors` array

3. **Se usar Array de Structs (Melhor opção):**
   - **Adicione nó "FindRemoteActor"** (função customizada acima)
   - **Conecte:**
     - `OutPlayerId` (do `ParseStateUpdateFrame`) → `PlayerId` (input da função)
   - **Saída:**
     - `ActorRef` (Actor Reference ou null)

#### **PASSO 4.4: Adicionar Branch para Verificar se Ator Existe**

1. **Adicione nó "Branch":**
   - **Clique direito** → Busque: **"Branch"**

2. **Conecte a condição:**
   - Se `FindRemoteActor` retorna `ActorRef`:
     - Use **"Is Valid"** node:
       - **Clique direito** → Busque: **"Is Valid"**
       - **Conecte:** `ActorRef` (do `FindRemoteActor`) → Pin `Object` do `Is Valid`
       - **Conecte:** `ReturnValue` (bool) → Pin `Condition` do `Branch`

3. **Conecte os pins do Branch:**
   - **True (ator existe):**
     - Conecte ao pin `execute` de **"SetActorLocation"** e **"SetActorRotation"**
     - Use o `ActorRef` existente
   
   - **False (ator não existe):**
     - Conecte ao pin `execute` do `SpawnActorFromClass`
     - Após spawnar, adicione o novo ator ao array

#### **PASSO 4.5: Atualizar Ator Existente (Branch True)**

1. **Adicione nó "SetActorLocation":**
   - **Entradas:**
     - `Target`: `ActorRef` (do `FindRemoteActor`)
     - `New Location`: `OutLocation` (do `ParseStateUpdateFrame`)

2. **Adicione nó "SetActorRotation":**
   - **Entradas:**
     - `Target`: `ActorRef` (do `FindRemoteActor`)
     - `New Rotation`: Use `Make Rotator` com `OutYawDegrees` (como na Correção 3)

3. **Conecte:**
   - Pin `then` do `Branch` (True) → Pin `execute` do `SetActorLocation`
   - Pin `then` do `SetActorLocation` → Pin `execute` do `SetActorRotation`

#### **PASSO 4.6: Adicionar Novo Ator ao Array (Branch False)**

1. **Após `SpawnActorFromClass`:**
   - Pin `ReturnValue` (Spawned Actor): Este é o novo ator criado

2. **Se usar Array simples:**
   - **Adicione nó "Array Add":**
     - **Clique direito** → Busque: **"Add"** ou arraste o array `RemoteActors`
     - **Conecte:**
       - `Item`: `ReturnValue` (Spawned Actor)
   
   - **IMPORTANTE:** Mantenha a ordem sincronizada com `RemoteStates` array!

3. **Se usar Array de Structs (Recomendado):**
   - **Adicione nó "Make RemoteActorEntry":**
     - **Clique direito** → Busque: **"Make [StructName]"** (ex: "Make RemoteActorEntry")
     - **Entradas:**
       - `PlayerId`: `OutPlayerId` (do `ParseStateUpdateFrame`)
       - `ActorRef`: `ReturnValue` (Spawned Actor do `SpawnActorFromClass`)
   
   - **Adicione ao Array:**
     - **Array Add:**
       - `Item`: `ReturnValue` (do `Make RemoteActorEntry`)

#### **📐 ESTRUTURA VISUAL:**

```
ParseStateUpdateFrame
  ↓ (then)
GetOrCreatePlayerState
  ↓ (ReturnValue)
FindRemoteActor (Custom Function)
  - Input: OutPlayerId
  - Output: ActorRef (ou null)
  ↓ (ReturnValue)
Is Valid
  - Input: ActorRef
  ↓ (ReturnValue: bool)
Branch
  │
  ├─ True (ator existe) ──────────────────────┐
  │                                             │
  │ SetActorLocation                            │
  │   - Target: ActorRef                        │
  │   - New Location: OutLocation               │
  │   ↓ (then)                                  │
  │ SetActorRotation                            │
  │   - Target: ActorRef                        │
  │   - New Rotation: Make Rotator (OutYawDegrees)
  │                                             │
  └─ False (ator não existe) ──────────────────┤
                                                 │
    SpawnActorFromClass                          │
      - Transform: Make Transform (Correção 3)  │
      - ReturnValue: Spawned Actor ──────────────┘
      ↓ (then)
    Make RemoteActorEntry
      - PlayerId: OutPlayerId
      - ActorRef: Spawned Actor
      ↓ (ReturnValue)
    Array Add (RemoteActorsArray)
      - Item: Make RemoteActorEntry ReturnValue
```

---

## 🔧 **CORREÇÃO 5: Corrigir Filtro de Tipo Invertido**

### **📍 LOCALIZAÇÃO:**
- **Event Graph** → **ProcessNextFrame**

### **🎯 PROBLEMA:**
- Branch `K2Node_IfThenElse_4` verifica `type == 2`
- Pin `then` (True) não está conectado
- Pin `else` (False) está conectado (invertido!)

### **✅ SOLUÇÃO PASSO A PASSO:**

#### **PASSO 5.1: Localizar o Branch K2Node_IfThenElse_4**

1. No `ProcessNextFrame`, localize o nó **"If Then Else"** que verifica o tipo
   - Deve estar após `ParseStateUpdateFrame` e `K2Node_IfThenElse_0`
   - A condição deve verificar se `type == 2`

#### **PASSO 5.2: Verificar Conexões Atuais**

1. **Identifique os pins:**
   - Pin `Condition`: Deve estar conectado a uma comparação `EqualEqual_ByteByte`
   - Pin `then` (True): **Deve estar VAZIO** (problema!)
   - Pin `else` (False): **Deve estar CONECTADO** → `K2Node_IfThenElse_3` (problema!)

#### **PASSO 5.3: Corrigir as Conexões**

1. **Desconecte:**
   - Pin `else` (False) do `K2Node_IfThenElse_4`
   - Clique no pin `else` e desconecte a conexão

2. **Conecte:**
   - Pin `then` (True) do `K2Node_IfThenElse_4` → Pin `execute` do `K2Node_IfThenElse_3`
   - Isso significa: "Se type == 2 (True), então processe o frame"

3. **Deixe o pin `else` desconectado:**
   - Se type != 2, o frame é ignorado (correto)

#### **📐 ESTRUTURA VISUAL:**

**ANTES (ERRADO):**
```
ParseStateUpdateFrame
  ↓ (then, ReturnValue == true)
K2Node_IfThenElse_0 (verifica se parse OK)
  ↓ (then = true)
K2Node_IfThenElse_4 (verifica type == 2)
  - Condition: type == 2
  - then (True): ❌ VAZIO
  - else (False): ✅ CONECTADO → K2Node_IfThenElse_3
```

**DEPOIS (CORRETO):**
```
ParseStateUpdateFrame
  ↓ (then, ReturnValue == true)
K2Node_IfThenElse_0 (verifica se parse OK)
  ↓ (then = true)
K2Node_IfThenElse_4 (verifica type == 2)
  - Condition: type == 2
  - then (True): ✅ CONECTADO → K2Node_IfThenElse_3
  - else (False): ❌ VAZIO (ignora se type != 2)
  ↓ (then = true, se type == 2)
K2Node_IfThenElse_3 (verifica PlayerId != LocalPlayerId)
  ↓ (else = false, se é outro player)
GetOrCreatePlayerState
  ↓
[Resto da lógica]
```

---

## 📋 **CHECKLIST FINAL DE VERIFICAÇÃO:**

### **✅ SendRateHz:**
- [ ] `SendRateHz` está sendo setado para `30.0` no `BeginPlay`
- [ ] Há proteção contra divisão por zero (usando `Max` ou `Branch`)
- [ ] Logs confirmam que `SendRateHz` tem valor válido

### **✅ MyPlayerId:**
- [ ] `MyPlayerId` está sendo inicializado no `BeginPlay`
- [ ] Usa `UmbraGameInstance::GetActivePlayerID()` corretamente
- [ ] Logs mostram valor correto (1, 2, 3, etc., não valores grandes)
- [ ] Cada cliente tem um `MyPlayerId` diferente

### **✅ Spawn na Posição Correta:**
- [ ] `SpawnActorFromClass` usa `OutLocation` do `ParseStateUpdateFrame`
- [ ] `Transform` inclui rotação correta (`OutYawDegrees`)
- [ ] `Spawn Collision Handling Override` está setado para "Always Spawn"
- [ ] Não há mais erros de spawn em (0,0,0)

### **✅ Prevenir Spawn Duplicado:**
- [ ] Há verificação se ator remoto já existe antes de spawnar
- [ ] Array `RemoteActors` está sendo mantido corretamente
- [ ] Atores existentes são reutilizados e atualizados
- [ ] Não há múltiplos pawns sendo spawnados

### **✅ Filtro de Tipo:**
- [ ] Pin `then` (True) do `K2Node_IfThenElse_4` está conectado
- [ ] Pin `else` (False) está desconectado (ignora frames que não são tipo 2)
- [ ] Apenas frames `type == 2` são processados

---

## 🎯 **ORDEM DE PRIORIDADE PARA CORREÇÃO:**

1. **🔥 CRÍTICO:** Correção 1 (SendRateHz) - causa crash
2. **🔥 CRÍTICO:** Correção 5 (Filtro Invertido) - impede processamento
3. **⚡ URGENTE:** Correção 2 (MyPlayerId) - causa problemas de sincronização
4. **⚡ URGENTE:** Correção 3 (Spawn Posição) - impede spawn correto
5. **⭐ IMPORTANTE:** Correção 4 (Reutilizar Atores) - otimização e correção de múltiplos spawns

---

## 📝 **NOTAS IMPORTANTES:**

### **Sobre SendRateHz:**
- Valor recomendado: `30.0` (30 updates por segundo)
- Alternativas: `20.0` (mais conservador) ou `60.0` (mais preciso, mas mais carga)

### **Sobre MyPlayerId:**
- Deve ser obtido do `UmbraGameInstance` após login e seleção de personagem
- Se ainda não houver personagem selecionado, use um valor temporário seguro

### **Sobre Spawn:**
- Sempre use `OutLocation` recebido do servidor
- Nunca use valores hardcoded ou padrão (0,0,0)
- Considere adicionar um pequeno offset (ex: Z + 10) se houver problemas de colisão

### **Sobre Reutilização de Atores:**
- Use Array de Structs para manter relação PlayerId ↔ Actor
- Sempre verifique se o ator existe antes de spawnar
- Limpe o array quando um player desconecta (evento de desconexão WebSocket)

---

**Após aplicar todas as correções nesta ordem, o sistema deve funcionar corretamente!**
