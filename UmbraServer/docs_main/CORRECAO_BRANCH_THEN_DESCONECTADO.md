# 🔧 **CORREÇÃO COMPLETA: Branch `then` (True) Desconectado - Actor Existente Não Está Sendo Atualizado**

## 📋 **PROBLEMA IDENTIFICADO (Análise Completa do XML):**

Analisando o XML completo do seu Blueprint `ProcessNextFrame`:

```xml
K2Node_IfThenElse_6 (Branch)
  - Condition: FoundIndex >= 0 (do Greater or Equal)
  - then (True): DESCONECTADO ← PROBLEMA PRINCIPAL!
  - else (False): Conectado ao SpawnActorFromClass
```

**Análise do fluxo atual:**
1. ✅ **Se `FoundIndex == -1` (actor não existe)**: 
   - Spawna novo actor (`K2Node_SpawnActorFromClass_0`)
   - Seta `NewActorRef` (`K2Node_VariableSet_10`)
   - Adiciona `OutPlayerId` a `RemoteActorIds` (`K2Node_CallArrayFunction_7`)
   - Atualiza `RemoteActorIds` (`K2Node_VariableSet_11`)
   - Adiciona `NewActorRef` a `RemoteActors` (`K2Node_CallArrayFunction_8`)
   - Verifica `IsValid(RemoteActorRef)` e atualiza (`K2Node_IfThenElse_9`)

2. ❌ **Se `FoundIndex >= 0` (actor já existe)**: 
   - **NADA ACONTECE** (pin `then` está desconectado)
   - O actor existente NUNCA é atualizado!

**Problema secundário identificado:**
- Há um `K2Node_IfThenElse_9` que verifica `IsValid(RemoteActorRef)` e atualiza o actor, mas essa lógica está DEPOIS do `Array_Add`, e `RemoteActorRef` é obtido de uma variável que não está sendo setada corretamente quando `FoundIndex >= 0`.

**Resultado:** Quando múltiplos frames chegam para o mesmo PlayerID:
1. **Primeiro frame**: `FoundIndex == -1` → Spawna actor → Adiciona aos arrays
2. **Segundos frames**: `FoundIndex >= 0` → **Nada acontece** → Actor não é atualizado
3. **Múltiplos frames continuam chegando** → Pode haver race condition ou múltiplos spawns se houver algum problema na ordem de execução

---

## ✅ **SOLUÇÃO:**

### **Fluxo Correto:**

```
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal (FoundIndex >= 0) → Boolean
  ↓
Branch [FoundIndex >= 0?]
  ├─ True (FoundIndex >= 0, actor JÁ EXISTE):
  │   ├─ Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │   ├─ SetActorLocation (RemoteActorRef, OutLocation)
  │   └─ SetActorRotation (RemoteActorRef, Make Rotator com OutYawDegrees)
  │
  └─ False (FoundIndex == -1, actor NÃO EXISTE):
      ├─ SpawnActorFromClass
      ├─ Array_Add (RemoteActorIds, OutPlayerId)
      └─ Array_Add (RemoteActors, SpawnedActor)
```

---

## 📝 **CORREÇÃO PASSO A PASSO (DETALHADA):**

### **⚠️ IMPORTANTE: O que você JÁ TEM e o que está FALTANDO:**

**Já existe no seu Blueprint:**
- ✅ `K2Node_IfThenElse_6`: Branch que verifica `FoundIndex >= 0`
- ✅ `K2Node_CallArrayFunction_4`: `Array_Find` que retorna `FoundIndex`
- ✅ `K2Node_VariableGet_16`: `Get RemoteActorIds` (array de inteiros)
- ✅ `K2Node_VariableGet_28`: `Get RemoteActors` (array de Actors) - **USE ESTE PARA OBTER O ACTOR!**
- ✅ `K2Node_IfThenElse_9`: Branch que verifica `IsValid(RemoteActorRef)` e atualiza

**O que está FALTANDO:**
- ❌ Conexão do `then` (True) do `K2Node_IfThenElse_6` ao fluxo de atualização
- ❌ `Get Array Item` para obter o actor de `RemoteActors` usando `FoundIndex`
- ❌ Conexão do actor obtido para a variável `RemoteActorRef` (para usar no `K2Node_IfThenElse_9`)

---

### **PASSO 1: Adicionar Get Array Item para Actor Existente**

**No Blueprint `ProcessNextFrame`:**

1. **Localize o Branch** `K2Node_IfThenElse_6`
2. **Encontre o pin `then` (True)** - deve estar desconectado
3. **Adicione um nó `Get Array Item`** (procure por "Get Array Item" ou "Array Get"):
   - **Target**: Deixe vazio (ou conecte a `K2Node_VariableGet_28` se necessário)
   - **Array**: Conecte ao `Get RemoteActors` (`K2Node_VariableGet_28`)
   - **Dimension 1** (Index): Conecte ao `ReturnValue` do `Array_Find` (`K2Node_CallArrayFunction_4` → `FoundIndex`)
     - ⚠️ **IMPORTANTE**: Use um `Knot` se necessário para conectar o `FoundIndex` ao `Get Array Item`
   - **ReturnValue**: Este será o `RemoteActorRef` do actor existente

4. **Após `Get Array Item`, adicione um `Set Variable`** (ou use `K2Node_VariableSet`):
   - **Variable**: `RemoteActorRef` (a mesma variável usada em `K2Node_IfThenElse_9`)
   - **Value**: `ReturnValue` do `Get Array Item`
   - **Execute**: Conecte ao pin `then` (True) do `K2Node_IfThenElse_6`

### **PASSO 2: Conectar ao Fluxo de Atualização**

**Após setar `RemoteActorRef`:**

1. **Você JÁ TEM** o `K2Node_IfThenElse_9` que verifica `IsValid(RemoteActorRef)` e faz `SetActorLocation` + `SetActorRotation`
2. **Duplique essa lógica** OU conecte diretamente após o `Set Variable` do `RemoteActorRef`

**Opção A: Reutilizar a lógica existente (mais simples):**
- Após setar `RemoteActorRef` no Passo 1, conecte diretamente ao mesmo fluxo usado no `K2Node_IfThenElse_9`:
  - `K2Node_CallFunction_37`: `Set Actor Location`
    - **Target**: `RemoteActorRef` (do `Set Variable`)
    - **New Location**: `OutLocation` (do `ParseStateUpdateFrame`)
    - **Teleport**: `true`
  - `K2Node_CallFunction_39`: `Set Actor Rotation`
    - **Target**: `RemoteActorRef`
    - **New Rotation**: `Make Rotator` com `OutYawDegrees`
    - **Teleport Physics**: `true`

**Opção B: Usar o mesmo `K2Node_IfThenElse_9` (mais complexo, mas reutiliza código):**
- Após setar `RemoteActorRef`, conecte ao mesmo `K2Node_IfThenElse_9` que verifica `IsValid(RemoteActorRef)`
- Isso evitará duplicação de código, mas você precisará garantir que `RemoteActorRef` está setado antes

---

## 🔍 **VERIFICAÇÃO DA LÓGICA ATUAL:**

### **Verificar se Array_Add está sendo executado após SpawnActor:**

**No pin `else` (False) do Branch, após `SpawnActorFromClass`:**

1. **Verifique se há `Array_Add` para `RemoteActorIds`**:
   - Target Array: `Get RemoteActorIds`
   - Item: `OutPlayerId` (do `ParseStateUpdateFrame`)

2. **Verifique se há `Array_Add` para `RemoteActors`**:
   - Target Array: `Get RemoteActors`
   - Item: `ReturnValue` do `SpawnActorFromClass` (o actor spawnado)

3. **Importante**: Os dois `Array_Add` devem ser executados **IMEDIATAMENTE** após o `SpawnActorFromClass`, na **mesma ordem**:
   - Primeiro: `Array_Add` em `RemoteActorIds`
   - Segundo: `Array_Add` em `RemoteActors`

---

## 📚 **GUIA DETALHADO: Como Adicionar o Nó `Array_Add` no Blueprint**

### **🎯 O QUE É O NÓ `Array_Add`?**

O nó `Array_Add` adiciona um novo elemento ao final de um array. Ele **modifica o array original** (não cria uma cópia).

**No Unreal Engine Blueprint Editor:**
- **Nome do nó**: `Array Add` ou `Add`
- **Categoria**: Pode ser encontrado em "Utilities" ou quando você busca por "Array"
- **Localização no menu**: Clique com botão direito no Blueprint → Procure por "Array Add" ou "Add"

---

### **📋 PASSO A PASSO COMPLETO: Adicionar `Array_Add` para `RemoteActorIds`**

#### **PASSO 1: Adicionar o Nó `Array_Add`**

1. **No Blueprint Editor**, localize o ponto onde você quer adicionar o `Array_Add` (após `SpawnActorFromClass`)
2. **Clique com botão direito** no espaço do Blueprint
3. **No menu de contexto**, digite: `Array Add` ou apenas `Add`
4. **Selecione o nó** que aparece como:
   - `Array Add` (Kismet Array Library)
   - Ou `Add` (da categoria de arrays)

**O nó aparecerá com os seguintes pins:**
- **Execute** (entrada, execução)
- **Then** (saída, execução)
- **Target Array** (entrada, array - com seta vermelha indicando referência)
- **New Item** (entrada, tipo do elemento do array)
- **Return Value** (saída, índice do elemento adicionado)

---

#### **PASSO 2: Conectar o Pin de Execução**

**IMPORTANTE**: O `Array_Add` precisa ser executado em sequência!

1. **Encontre o pin `then` (saída de execução)** do nó anterior (ex: `SpawnActorFromClass` ou `Set Variable`)
2. **Clique e arraste** desse pin `then` até o pin **`execute`** (entrada) do `Array_Add`
3. Você verá uma linha branca conectando os dois nós

**Visualmente:**
```
[SpawnActorFromClass]
    └─ then (out) ────────────┐
                                │
                          [Array_Add]
                                └─ execute (in)
```

---

#### **PASSO 3: Conectar `Target Array` para `RemoteActorIds`**

**⚠️ CRÍTICO**: O `Target Array` deve ser conectado ao **GET da variável**, não ao SET!

1. **Adicione um nó `Get`** para a variável `RemoteActorIds`:
   - Clique com botão direito → Digite `Get RemoteActorIds` ou `RemoteActorIds`
   - Selecione a variável que aparece (deve ter um ícone de array)

2. **Conecte o `ReturnValue` (ou saída)** do `Get RemoteActorIds` ao pin **`Target Array`** do `Array_Add`

**⚠️ IMPORTANTE:**
- O pin `Target Array` tem uma **seta vermelha** ao lado → isso indica que é uma **referência** (modifica o array original)
- Se você não conseguir conectar, verifique se o tipo do array está correto:
  - `RemoteActorIds` deve ser um **Array of Integer** (`TArray<int32>`)
  - O `Array_Add` deve estar configurado para **Array of Integer**

**Visualmente:**
```
[Get RemoteActorIds]
    └─ ReturnValue (Array of Integer) ────┐
                                            │
                                      [Array_Add]
                                            └─ Target Array (in)
```

---

#### **PASSO 4: Conectar `New Item` com `OutPlayerId`**

**O que conectar:**
- `OutPlayerId` do `ParseStateUpdateFrame` (já existe no seu Blueprint)

**Como fazer:**

1. **Localize o nó `ParseStateUpdateFrame`** no seu Blueprint
2. **Encontre o pin `OutPlayerId`** (saída, tipo Integer)
3. **Conecte `OutPlayerId`** ao pin **`New Item`** do `Array_Add`

**⚠️ DICA**: Se `OutPlayerId` estiver muito longe, use um **Knot** (nó de ligação):
- Clique com botão direito → `Knot` ou `Reroute Node`
- Conecte `OutPlayerId` → `Knot` → `New Item`

**Visualmente:**
```
[ParseStateUpdateFrame]
    └─ OutPlayerId (Integer) ────┐
                                   │
                             [Array_Add]
                                   └─ New Item (in)
```

---

#### **PASSO 5: Configuração Final do Primeiro `Array_Add`**

Após conectar todos os pins, o nó `Array_Add` deve estar assim:

```
[Get RemoteActorIds] ────┐
                          │
                    [Array_Add] ── then ──> [Próximo Nó]
                          │
[OutPlayerId] ────────────┘
```

**Configuração:**
- ✅ **Execute**: Conectado ao pin `then` do nó anterior
- ✅ **Target Array**: Conectado a `Get RemoteActorIds`
- ✅ **New Item**: Conectado a `OutPlayerId`
- ✅ **Then**: Será conectado ao próximo `Array_Add` ou ao próximo passo

---

### **📋 PASSO A PASSO COMPLETO: Adicionar `Array_Add` para `RemoteActors`**

#### **PASSO 1: Adicionar o Segundo Nó `Array_Add`**

1. **Adicione outro nó `Array_Add`** (mesmo processo do primeiro)
2. **Posicione-o** logo após o primeiro `Array_Add`

---

#### **PASSO 2: Conectar o Pin de Execução**

1. **Conecte o pin `then`** do primeiro `Array_Add` ao pin **`execute`** do segundo `Array_Add`

**Visualmente:**
```
[Array_Add para RemoteActorIds]
    └─ then ────────────┐
                         │
                   [Array_Add para RemoteActors]
                         └─ execute (in)
```

---

#### **PASSO 3: Conectar `Target Array` para `RemoteActors`**

1. **Adicione um nó `Get`** para `RemoteActors`:
   - Clique com botão direito → `Get RemoteActors`
2. **Conecte o `ReturnValue`** (saída) ao pin **`Target Array`** do segundo `Array_Add`

**⚠️ IMPORTANTE:**
- `RemoteActors` deve ser um **Array of Actor** ou **Array of BP_RemotePlayer**
- O `Array_Add` deve estar configurado para o mesmo tipo

---

#### **PASSO 4: Conectar `New Item` com o Actor Spawnado**

**O que conectar:**
- `ReturnValue` do `SpawnActorFromClass` (o actor que foi spawnado)

**Como fazer:**

1. **Localize o nó `SpawnActorFromClass`**
2. **Encontre o pin `ReturnValue`** (saída, tipo `BP_RemotePlayer` ou `Actor`)
3. **Conecte `ReturnValue`** ao pin **`New Item`** do segundo `Array_Add`

**⚠️ DICA**: Se você já setou `NewActorRef = ReturnValue do SpawnActorFromClass`, você pode usar:
- **Opção A**: `ReturnValue` diretamente do `SpawnActorFromClass`
- **Opção B**: `Get NewActorRef` (se você já fez `Set Variable`)

**Visualmente:**
```
[SpawnActorFromClass]
    └─ ReturnValue (BP_RemotePlayer) ────┐
                                           │
                                     [Array_Add]
                                           └─ New Item (in)
```

---

#### **PASSO 5: Configuração Final do Segundo `Array_Add`**

O segundo `Array_Add` deve estar assim:

```
[Get RemoteActors] ────┐
                        │
                  [Array_Add] ── then ──> [Próximo Nó]
                        │
[SpawnActorFromClass    │
 ReturnValue] ──────────┘
```

---

### **🎯 ESTRUTURA COMPLETA: Ambos os `Array_Add` Conectados**

```
[SpawnActorFromClass]
    └─ then ────┐
                 │
           [Array_Add] (RemoteActorIds)
                 │  └─ Target Array ← [Get RemoteActorIds]
                 │  └─ New Item ← [OutPlayerId]
                 │
                 └─ then ────┐
                              │
                        [Array_Add] (RemoteActors)
                              │  └─ Target Array ← [Get RemoteActors]
                              │  └─ New Item ← [SpawnActorFromClass ReturnValue]
                              │
                              └─ then ──> [Próximo Nó (ex: Set Variable ou IsValid)]
```

---

### **✅ CHECKLIST: Verificar se está Correto**

Após adicionar ambos os `Array_Add`, verifique:

- [ ] **Primeiro `Array_Add`**:
  - [ ] `Execute` conectado ao `then` do nó anterior
  - [ ] `Target Array` conectado a `Get RemoteActorIds` (não `Set`)
  - [ ] `New Item` conectado a `OutPlayerId` (Integer)
  - [ ] `Then` conectado ao `execute` do segundo `Array_Add`

- [ ] **Segundo `Array_Add`**:
  - [ ] `Execute` conectado ao `then` do primeiro `Array_Add`
  - [ ] `Target Array` conectado a `Get RemoteActors` (não `Set`)
  - [ ] `New Item` conectado a `ReturnValue` do `SpawnActorFromClass` (ou `Get NewActorRef`)
  - [ ] `Then` conectado ao próximo passo do fluxo

- [ ] **Ordem de Execução**:
  - [ ] Primeiro: `Array_Add` em `RemoteActorIds`
  - [ ] Segundo: `Array_Add` em `RemoteActors`
  - [ ] Nenhum outro nó entre os dois `Array_Add`

---

### **⚠️ ERROS COMUNS E COMO RESOLVER**

#### **Erro 1: "Cannot connect pins of incompatible types"**

**Causa**: O tipo do array não corresponde ao tipo do item.

**Solução**:
- Verifique se `RemoteActorIds` é realmente um `Array of Integer`
- Verifique se `RemoteActors` é realmente um `Array of Actor` (ou `BP_RemotePlayer`)
- Se necessário, ajuste os tipos das variáveis no Blueprint

#### **Erro 2: O array não está sendo modificado**

**Causa**: Conectou `Set RemoteActorIds` em vez de `Get RemoteActorIds`.

**Solução**:
- Use sempre **`Get`** (ícone de saída) para `Target Array`
- Não use **`Set`** (ícone de entrada) para `Target Array`

#### **Erro 3: "Target Array pin is not writable"**

**Causa**: O array está marcado como `Read Only` ou é uma constante.

**Solução**:
- Verifique as propriedades da variável `RemoteActorIds` e `RemoteActors`
- Certifique-se de que não estão marcadas como `Read Only` ou `Const`

#### **Erro 4: Múltiplos spawns do mesmo player**

**Causa**: Os `Array_Add` não estão sendo executados imediatamente após o spawn.

**Solução**:
- Certifique-se de que os `Array_Add` estão **imediatamente** após o `SpawnActorFromClass`
- Não há outros nós entre o spawn e os `Array_Add`
- A ordem de execução está correta (primeiro `RemoteActorIds`, depois `RemoteActors`)

---

### **📸 EXEMPLO VISUAL NO BLUEPRINT EDITOR**

```
┌─────────────────────────────────────────────────────────────┐
│  ProcessNextFrame                                            │
│                                                              │
│  ┌─────────────────┐                                        │
│  │ SpawnActorFrom  │                                        │
│  │ Class           │                                        │
│  │  └─ then ───────┼─┐                                      │
│  │  └─ ReturnValue│ │                                      │
│  └─────────────────┘ │                                      │
│                       │                                      │
│  ┌─────────────────┐  │  ┌──────────────────────────┐      │
│  │ Get RemoteActor │  │  │ Array Add                 │      │
│  │ Ids             │──┼──│ Target Array ←─           │      │
│  │  └─ ReturnValue │  │  │ New Item ←─ OutPlayerId   │      │
│  └─────────────────┘  │  │  └─ then ────────────────┼─┐    │
│                       │  └──────────────────────────┘ │    │
│                       │                                │    │
│  ┌─────────────────┐  │  ┌──────────────────────────┐ │    │
│  │ Get RemoteActor │  │  │ Array Add                 │ │    │
│  │ s               │──┼──│ Target Array ←─           │ │    │
│  │  └─ ReturnValue │  │  │ New Item ←─ ReturnValue   │ │    │
│  └─────────────────┘  │  │  └─ then ────────────────┼─┼───▶│
│                       │  └──────────────────────────┘ │    │
│                       └────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

### **🔧 DICAS AVANÇADAS**

1. **Usar `Array_AddUnique` em vez de `Array_Add`**:
   - `Array_AddUnique` só adiciona se o elemento ainda não existir
   - Útil para prevenir duplicatas, mas **mais lento**

2. **Verificar se o elemento já existe antes de adicionar**:
   - Use `Array_Find` antes de `Array_Add`
   - Se `FoundIndex == -1`, então adicione

3. **Debug com `Print String`**:
   - Após cada `Array_Add`, adicione `Print String` para verificar:
     - `"Added PlayerID " + To String(OutPlayerId) + " to RemoteActorIds. Size: " + To String(RemoteActorIds.Num())`
     - `"Added Actor to RemoteActors. Size: " + To String(RemoteActors.Num())`

---

### **📊 RESUMO RÁPIDO**

1. **Adicionar `Array_Add`** → Clique direito → `Array Add`
2. **Conectar `Execute`** → Do `then` do nó anterior
3. **Conectar `Target Array`** → Do `Get RemoteActorIds` (ou `Get RemoteActors`)
4. **Conectar `New Item`** → De `OutPlayerId` (ou `ReturnValue` do `SpawnActorFromClass`)
5. **Conectar `Then`** → Ao próximo passo do fluxo
6. **Repetir** para o segundo `Array_Add` (em `RemoteActors`)

---

## 🎯 **ESTRUTURA VISUAL COMPLETA (Baseada no seu XML):**

```
ProcessNextFrame (entrada)
  ↓
ParseStateUpdateFrame → ReturnValue, OutPlayerId, OutLocation, OutYawDegrees
  ↓ (ReturnValue == true)
Branch [ParseStateUpdateFrame.ReturnValue?] (K2Node_IfThenElse_0)
  ↓ (then)
Branch [OutPlayerId == MyPlayerId?] (K2Node_IfThenElse_1)
  ├─ True → IGNORAR (desconectado)
  └─ False → CONTINUAR:
      ↓
Branch [Data[0] == 2?] (K2Node_IfThenElse_4)
  ↓ (then)
Branch [OutPlayerId == LocalPlayerId?] (K2Node_IfThenElse_3)
  ├─ True → IGNORAR (desconectado)
  └─ False → CONTINUAR:
      ↓
GetOrCreatePlayerState → PlayerStateEntry
  ↓
UpdatePlayerStateBuffer
  ↓
FindPlayerStateIndex → StateIndex
  ↓
Array_Set (RemoteStates, StateIndex, PlayerStateEntry)
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex (K2Node_CallArrayFunction_4)
  ↓
Greater or Equal (FoundIndex >= 0) → Boolean (K2Node_PromotableOperator_10)
  ↓
Branch [FoundIndex >= 0?] (K2Node_IfThenElse_6) ← CORREÇÃO AQUI!
  ├─ True (actor JÁ EXISTE): ← ESTE PIN ESTÁ DESCONECTADO!
  │   ├─ Get Array Item (RemoteActors, FoundIndex) → ActorExistente ← ADICIONAR!
  │   ├─ Set Variable (RemoteActorRef = ActorExistente) ← ADICIONAR!
  │   ├─ Is Valid? (RemoteActorRef) → Boolean ← REUTILIZAR K2Node_IfThenElse_9?
  │   │   └─ True → Set Actor Location + Set Actor Rotation ← ADICIONAR/REUTILIZAR!
  │   └─ ProcessBinaryBuffer (verificar próximo frame) ← CONECTAR AO FINAL!
  │
  └─ False (actor NÃO EXISTE):
      ├─ Make Transform (OutLocation, Make Rotator(OutYawDegrees))
      ├─ SpawnActorFromClass (BP_RemotePlayer, Transform) → SpawnedActor
      ├─ Set Variable (NewActorRef = SpawnedActor)
      ├─ Array_Add (RemoteActorIds, OutPlayerId) ← JÁ EXISTE (K2Node_CallArrayFunction_7)
      ├─ Set Variable (RemoteActorIds = Array após Add)
      ├─ Array_Add (RemoteActors, NewActorRef) ← JÁ EXISTE (K2Node_CallArrayFunction_8)
      ├─ Is Valid? (RemoteActorRef) → Boolean ← JÁ EXISTE (K2Node_IfThenElse_9)
      │   └─ True → Set Actor Location + Set Actor Rotation ← JÁ EXISTE
      └─ ProcessBinaryBuffer (verificar próximo frame) ← JÁ EXISTE
```

---

## ⚠️ **PONTOS CRÍTICOS:**

### **1. Ordem dos Array_Add**
- ✅ Primeiro adicione `OutPlayerId` a `RemoteActorIds`
- ✅ Depois adicione `SpawnedActor` a `RemoteActors`
- ⚠️ **NÃO** inverta a ordem - isso quebra a sincronização dos arrays

### **2. Execução Imediata**
- ⚠️ Os `Array_Add` devem ser executados **imediatamente após** o `SpawnActorFromClass`
- ⚠️ **NÃO** há outros nós entre o spawn e os `Array_Add`
- ⚠️ Se houver delay ou outros nós, múltiplos frames podem chegar antes que o array seja atualizado

### **3. Atualização do Actor Existente**
- ✅ Quando `FoundIndex >= 0`, você DEVE atualizar a posição/rotação do actor existente
- ✅ Se o `then` estiver desconectado, o actor nunca será atualizado após o primeiro spawn
- ✅ Isso pode fazer com que o actor fique "congelado" na posição inicial

### **4. ProcessBinaryBuffer ao Final (AMBOS os caminhos)**
- ⚠️ **CRÍTICO**: Ambos os caminhos (`then` e `else`) devem terminar com `ProcessBinaryBuffer` (`K2Node_CallFunction_1`)
- ⚠️ Isso verifica se há mais frames no buffer e chama `ProcessNextFrame` recursivamente
- ⚠️ No seu XML, o `ProcessBinaryBuffer` já existe no fluxo `else` (False), mas está faltando no fluxo `then` (True)
- ⚠️ Após `SetActorLocation` e `SetActorRotation` no fluxo `then` (True), conecte ao mesmo `ProcessBinaryBuffer` usado no `else` (False)

---

## 🔬 **DEBUG SUGERIDO:**

Adicione `Print String` para verificar o fluxo:

1. **No `then` (True) do Branch**:
   ```
   "Actor EXISTS for PlayerID " + To String(OutPlayerId) + " at index " + To String(FoundIndex) + ". Updating position."
   ```

2. **No `else` (False) do Branch**:
   ```
   "Actor NOT FOUND for PlayerID " + To String(OutPlayerId) + ". Spawning new actor."
   ```

3. **Após Array_Add (RemoteActorIds)**:
   ```
   "Added PlayerID " + To String(OutPlayerId) + " to RemoteActorIds. Size: " + To String(RemoteActorIds.Num())
   ```

4. **Após Array_Add (RemoteActors)**:
   ```
   "Added Actor to RemoteActors. Size: " + To String(RemoteActors.Num())
   ```

---

## 📊 **RESUMO:**

**O problema:**
- O pin `then` (True) do Branch está desconectado
- Quando o actor já existe (`FoundIndex >= 0`), nada acontece
- O actor não é atualizado com novas posições/rotações

**A solução:**
1. Conecte o pin `then` (True) do Branch
2. Adicione `Get Array Item` para obter o `RemoteActorRef`
3. Adicione `SetActorLocation` e `SetActorRotation` para atualizar o actor existente
4. Verifique se `Array_Add` está sendo executado após `SpawnActorFromClass`

**Após esta correção:**
- ✅ Atores existentes serão atualizados com novas posições/rotações
- ✅ Apenas um actor será spawnado por PlayerID
- ✅ Múltiplos spawns serão prevenidos
- ✅ Movimento dos players remotos será sincronizado

