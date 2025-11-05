# 🔍 **GUIA PASSO A PASSO DETALHADO: Diagnóstico de Spawn Duplicado**

## 📋 **PROBLEMA IDENTIFICADO:**

Quando um novo client se conecta, o client que já está conectado spawna **2 actors**:
1. **Actor 1:** Fica parado na posição inicial do spawn (não se move)
2. **Actor 2:** Se move corretamente e tem os dados transmitidos

**Isso indica spawn duplicado ocorrendo.**

---

## 🎯 **OBJETIVO DO GUIA:**

Adicionar logs detalhados em pontos críticos de `ProcessNextFrame` para identificar:
- Quando `Array_Find` é executado e qual `FoundIndex` retorna
- Quando `SpawnActorFromClass` é executado e com quais dados
- Quando `Array_Add` é executado
- Quando `Get Array Item` é executado para atualizar actors existentes
- Onde está ocorrendo o spawn duplicado

---

## 📍 **LOCALIZAÇÃO:**

- **Blueprint:** `BP_NetMovementClient`
- **Função:** `ProcessNextFrame` (Custom Event)

---

## 🔧 **ETAPA 1: Adicionar Log Após Array_Find**

### **PASSO 1.1: Localizar Array_Find**

1. **Abra `BP_NetMovementClient`** no Unreal Editor
2. **Abra a função `ProcessNextFrame`**
3. **Localize o nó `Array_Find`** após o filtro (`OutPlayerId != Active Player ID`)
   - Deve estar buscando em `RemoteActorIds`
   - Deve estar buscando `OutPlayerId`

### **PASSO 1.2: Adicionar Format Text**

1. **Clique com o botão direito** após o nó `Array_Find`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **O nó será criado automaticamente**

### **PASSO 1.3: Configurar Format Text**

1. **Clique no nó `Format Text`**
2. **No painel de detalhes**, localize o campo `Format Pattern`
3. **Digite exatamente:**
   ```
   [ProcessNextFrame] Array_Find - OutPlayerId: {0}, FoundIndex: {1}
   ```

### **PASSO 1.4: Conectar Variáveis ao Format Text**

#### **Conectar {0} - OutPlayerId:**

1. **Localize o pin `{0}` do `Format Text`**
   - Pode aparecer como `Argument 0` ou `{0}`

2. **Conecte `OutPlayerId`:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Format Text` → `{0}`

#### **Conectar {1} - FoundIndex:**

1. **Localize o pin `{1}` do `Format Text`**
   - Pode aparecer como `Argument 1` ou `{1}`

2. **Conecte `FoundIndex`:**
   - **Pin exato:** `Array_Find` → `Found Index` → `Format Text` → `{1}`

### **PASSO 1.5: Adicionar Print String**

1. **Clique com o botão direito** após o nó `Format Text`
2. **No menu de busca**, digite: `Print String`
3. **Selecione:** `Print String`
4. **O nó será criado automaticamente**

### **PASSO 1.6: Conectar Format Text ao Print String**

1. **Conecte:**
   - **Pin exato:** `Format Text` → `Result` → `Print String` → `In String`

### **PASSO 1.7: Conectar ao Fluxo de Execução**

1. **O `Format Text` deve estar no fluxo de execução após `Array_Find`**
   - Se `Array_Find` não tem pin de execução (é uma função pura), conecte o pin de execução que vem antes de `Array_Find` ao pin `execute` do `Format Text`
   - Ou conecte diretamente após o nó que vem antes de `Array_Find` no fluxo

2. **Conecte `Print String` ao próximo nó:**
   - **Pin exato:** `Print String` → `then` → `Greater or Equal` → `execute` (ou próximo nó no fluxo)

### **Estrutura Visual:**

```
[... filtros ...]
  ↓ execute
Array_Find (RemoteActorIds, OutPlayerId)
  ├─ Target ← Get RemoteActorIds
  ├─ Item To Find ← ParseStateUpdateFrame → Out Player Id
  └─ Found Index → Integer
  ↓ Found Index
Format Text
  ├─ Format Pattern: "[ProcessNextFrame] Array_Find - OutPlayerId: {0}, FoundIndex: {1}"
  ├─ {0} ← ParseStateUpdateFrame → Out Player Id
  └─ {1} ← Array_Find → Found Index
  ↓ Result
Print String
  └─ In String ← Format Text → Result
  ↓ then
Greater or Equal (FoundIndex >= 0)
```

---

## 🔧 **ETAPA 2: Adicionar Log Após Greater or Equal (Branch)**

### **PASSO 2.1: Localizar Branch Após Greater or Equal**

1. **Localize o nó `Branch`** após `Greater or Equal`
   - Deve ser o `K2Node_IfThenElse_6`
   - Deve ter `Condition` conectado ao `Greater or Equal`

### **PASSO 2.2: Adicionar Log no Caminho `then` (True - Actor Existe)**

1. **Clique com o botão direito** após o pin `then` do `Branch`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] Actor já existe - FoundIndex: {0}, atualizando"`
   - **{0}:** Conecte `Array_Find` → `Found Index`

5. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte ao próximo nó (provavelmente `Get Array Item`)

### **PASSO 2.3: Adicionar Log no Caminho `else` (False - Actor Não Existe)**

1. **Clique com o botão direito** após o pin `else` do `Branch`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] Actor não existe - FoundIndex: {0}, spawnando novo"`
   - **{0}:** Conecte `Array_Find` → `Found Index`

5. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte ao próximo nó (provavelmente `SpawnActorFromClass` ou verificação dupla)

### **Estrutura Visual:**

```
Greater or Equal (FoundIndex >= 0)
  ↓ Return Value
Branch (K2Node_IfThenElse_6)
  ├─ then (True): Format Text ("Actor já existe - FoundIndex: {0}")
  │                ├─ {0} ← Array_Find → Found Index
  │                ↓ Result
  │                Print String
  │                ↓ then
  │                Get Array Item
  │
  └─ else (False): Format Text ("Actor não existe - FoundIndex: {0}")
                   ├─ {0} ← Array_Find → Found Index
                   ↓ Result
                   Print String
                   ↓ then
                   [Verificação dupla ou SpawnActorFromClass]
```

---

## 🔧 **ETAPA 3: Adicionar Log Antes de SpawnActorFromClass**

### **PASSO 3.1: Localizar SpawnActorFromClass**

1. **No caminho `else` do `Branch`**, localize o nó `SpawnActorFromClass`
   - Deve estar após a verificação dupla (se implementada) ou diretamente após o `Branch`

### **PASSO 3.2: Adicionar Format Text ANTES de SpawnActorFromClass**

1. **Clique com o botão direito** ANTES do nó `SpawnActorFromClass` (no fluxo de execução)
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: {0}, Location: ({1}, {2}, {3}), Yaw: {4}"`

### **PASSO 3.3: Conectar Variáveis ao Format Text**

#### **Conectar {0} - OutPlayerId:**

1. **Conecte:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Format Text` → `{0}`

#### **Conectar {1}, {2}, {3} - OutLocation:**

1. **Adicione `Break Vector`:**
   - Clique direito → `Break Vector`
   - Conecte `ParseStateUpdateFrame` → `Out Location` → `Break Vector` → `Vector`

2. **Conecte X, Y, Z:**
   - **{1}:** `Break Vector` → `X` → `Format Text` → `{1}`
   - **{2}:** `Break Vector` → `Y` → `Format Text` → `{2}`
   - **{3}:** `Break Vector` → `Z` → `Format Text` → `{3}`

#### **Conectar {4} - OutYawDegrees:**

1. **Conecte:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Yaw Degrees` → `Format Text` → `{4}`

### **PASSO 3.4: Adicionar Print String**

1. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte `Print String` → `then` → `SpawnActorFromClass` → `execute`

### **Estrutura Visual:**

```
[... verificação dupla ou Branch else ...]
  ↓ execute
Break Vector (OutLocation)
  ├─ Vector ← ParseStateUpdateFrame → Out Location
  ├─ X → Float
  ├─ Y → Float
  └─ Z → Float
  ↓ (no fluxo de execução)
Format Text
  ├─ Format Pattern: "[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: {0}, Location: ({1}, {2}, {3}), Yaw: {4}"
  ├─ {0} ← ParseStateUpdateFrame → Out Player Id
  ├─ {1} ← Break Vector → X
  ├─ {2} ← Break Vector → Y
  ├─ {3} ← Break Vector → Z
  └─ {4} ← ParseStateUpdateFrame → Out Yaw Degrees
  ↓ Result
Print String
  └─ In String ← Format Text → Result
  ↓ then
SpawnActorFromClass
```

---

## 🔧 **ETAPA 4: Adicionar Log Após SpawnActorFromClass**

### **PASSO 4.1: Localizar ReturnValue do SpawnActorFromClass**

1. **Localize o nó `SpawnActorFromClass`**
2. **Localize o pin `Return Value`** (o actor spawnado)

### **PASSO 4.2: Adicionar Is Valid**

1. **Clique com o botão direito** após `SpawnActorFromClass`
2. **No menu de busca**, digite: `Is Valid`
3. **Selecione:** `Is Valid`
4. **Conecte:**
   - **Pin exato:** `SpawnActorFromClass` → `Return Value` → `Is Valid` → `Object`

### **PASSO 4.3: Adicionar Format Text**

1. **Clique com o botão direito** após `Is Valid`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] APÓS SpawnActorFromClass - PlayerID: {0}, Actor válido: {1}"`

### **PASSO 4.4: Conectar Variáveis ao Format Text**

#### **Conectar {0} - OutPlayerId:**

1. **Conecte:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Format Text` → `{0}`

#### **Conectar {1} - Is Valid:**

1. **Adicione `To String (Boolean)`:**
   - Clique direito → `To String (Boolean)`
   - Conecte `Is Valid` → `Return Value` → `To String (Boolean)` → `In Bool`

2. **Conecte:**
   - **Pin exato:** `To String (Boolean)` → `Return Value` → `Format Text` → `{1}`

### **PASSO 4.5: Adicionar Print String**

1. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte `Print String` → `then` → `Set Variable: RemoteActorRef` → `execute`

### **Estrutura Visual:**

```
SpawnActorFromClass
  ↓ Return Value
Is Valid
  └─ Object ← SpawnActorFromClass → Return Value
  ↓ Return Value
To String (Boolean)
  └─ In Bool ← Is Valid → Return Value
  ↓ Return Value
Format Text
  ├─ Format Pattern: "[ProcessNextFrame] APÓS SpawnActorFromClass - PlayerID: {0}, Actor válido: {1}"
  ├─ {0} ← ParseStateUpdateFrame → Out Player Id
  └─ {1} ← To String (Boolean) → Return Value
  ↓ Result
Print String
  └─ In String ← Format Text → Result
  ↓ then
Set Variable: RemoteActorRef
```

---

## 🔧 **ETAPA 5: Adicionar Log Após Array_Add**

### **PASSO 5.1: Localizar Array_Add para RemoteActorIds**

1. **No caminho `else` do `Branch`**, após `SpawnActorFromClass` e `Set Variable: RemoteActorRef`, localize o nó `Array_Add` para `RemoteActorIds`

### **PASSO 5.2: Adicionar Format Text APÓS Array_Add**

1. **Clique com o botão direito** após o nó `Array_Add` para `RemoteActorIds`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] Array_Add - RemoteActorIds adicionado: {0}, tamanho agora: {1}"`

### **PASSO 5.3: Conectar Variáveis ao Format Text**

#### **Conectar {0} - OutPlayerId:**

1. **Conecte:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Format Text` → `{0}`

#### **Conectar {1} - Tamanho do Array:**

1. **Adicione `Get Array Length`:**
   - Clique direito → `Get Array Length`
   - Conecte `Get RemoteActorIds` → `Return Value` → `Get Array Length` → `TargetArray`

2. **Conecte:**
   - **Pin exato:** `Get Array Length` → `Length` → `Format Text` → `{1}`

### **PASSO 5.4: Adicionar Print String**

1. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte `Print String` → `then` → `Array_Add (RemoteActors)` → `execute`

### **PASSO 5.5: Repetir para Array_Add de RemoteActors**

1. **Após o segundo `Array_Add` (para `RemoteActors`)**, adicione outro log:
   - **Format Pattern:** `"[ProcessNextFrame] Array_Add - RemoteActors adicionado, tamanho agora: {0}"`
   - **{0}:** `Get Array Length (RemoteActors)` → `Length`

### **Estrutura Visual:**

```
Set Variable: RemoteActorRef
  ↓ then
Array_Add (RemoteActorIds, OutPlayerId)
  ├─ Target ← Get RemoteActorIds
  └─ Item ← ParseStateUpdateFrame → Out Player Id
  ↓ then
Format Text ("Array_Add - RemoteActorIds adicionado: {0}, tamanho agora: {1}")
  ├─ {0} ← ParseStateUpdateFrame → Out Player Id
  └─ {1} ← Get Array Length (RemoteActorIds) → Length
  ↓ Result
Print String
  ↓ then
Array_Add (RemoteActors, RemoteActorRef)
  ↓ then
Format Text ("Array_Add - RemoteActors adicionado, tamanho agora: {0}")
  └─ {0} ← Get Array Length (RemoteActors) → Length
  ↓ Result
Print String
```

---

## 🔧 **ETAPA 6: Adicionar Log Antes de Set Actor Location**

### **PASSO 6.1: Localizar Set Actor Location**

1. **Localize o nó `Set Actor Location`** no final do fluxo (após `Array_Add` ou após `Get Array Item`)

### **PASSO 6.2: Adicionar Format Text ANTES de Set Actor Location**

1. **Clique com o botão direito** ANTES do nó `Set Actor Location`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] Set Actor Location - RemoteActorRef: {0}, Location: ({1}, {2}, {3})"`

### **PASSO 6.3: Conectar Variáveis ao Format Text**

#### **Conectar {0} - RemoteActorRef:**

1. **Adicione `To String`:**
   - Clique direito → `To String` (ou `Convert Object to String`)
   - Conecte `Get RemoteActorRef` → `Return Value` → `To String` → `Object`

2. **Conecte:**
   - **Pin exato:** `To String` → `Return Value` → `Format Text` → `{0}`

#### **Conectar {1}, {2}, {3} - OutLocation:**

1. **Use o mesmo `Break Vector` da Etapa 3** ou crie um novo:
   - Conecte `ParseStateUpdateFrame` → `Out Location` → `Break Vector` → `Vector`

2. **Conecte X, Y, Z:**
   - **{1}:** `Break Vector` → `X` → `Format Text` → `{1}`
   - **{2}:** `Break Vector` → `Y` → `Format Text` → `{2}`
   - **{3}:** `Break Vector` → `Z` → `Format Text` → `{3}`

### **PASSO 6.4: Adicionar Print String**

1. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte `Print String` → `then` → `Set Actor Location` → `execute`

### **Estrutura Visual:**

```
[... Array_Add ou Get Array Item ...]
  ↓ then
Break Vector (OutLocation)
  ├─ Vector ← ParseStateUpdateFrame → Out Location
  ├─ X → Float
  ├─ Y → Float
  └─ Z → Float
  ↓ (no fluxo de execução)
Format Text ("Set Actor Location - RemoteActorRef: {0}, Location: ({1}, {2}, {3})")
  ├─ {0} ← To String (RemoteActorRef) → Return Value
  ├─ {1} ← Break Vector → X
  ├─ {2} ← Break Vector → Y
  └─ {3} ← Break Vector → Z
  ↓ Result
Print String
  └─ In String ← Format Text → Result
  ↓ then
Set Actor Location
```

---

## 🔧 **ETAPA 7: Adicionar Log no Caminho `then` (Get Array Item)**

### **PASSO 7.1: Localizar Get Array Item**

1. **No caminho `then` do `Branch`**, localize o nó `Get Array Item`
   - Deve estar recuperando `RemoteActors[FoundIndex]`

### **PASSO 7.2: Adicionar Format Text APÓS Get Array Item**

1. **Clique com o botão direito** após o nó `Get Array Item`
2. **No menu de busca**, digite: `Format Text`
3. **Selecione:** `Format Text`
4. **Configure:**
   - **Format Pattern:** `"[ProcessNextFrame] Get Array Item - FoundIndex: {0}, ExistingActorRef: {1}, válido: {2}"`

### **PASSO 7.3: Conectar Variáveis ao Format Text**

#### **Conectar {0} - FoundIndex:**

1. **Conecte:**
   - **Pin exato:** `Array_Find` → `Found Index` → `Format Text` → `{0}`

#### **Conectar {1} - ExistingActorRef:**

1. **Adicione `To String`:**
   - Clique direito → `To String` (ou `Convert Object to String`)
   - Conecte `Get Array Item` → `Array Element` → `To String` → `Object`

2. **Conecte:**
   - **Pin exato:** `To String` → `Return Value` → `Format Text` → `{1}`

#### **Conectar {2} - Is Valid:**

1. **Adicione `Is Valid` após `Get Array Item`:**
   - Clique direito → `Is Valid`
   - Conecte `Get Array Item` → `Array Element` → `Is Valid` → `Object`

2. **Adicione `To String (Boolean)`:**
   - Conecte `Is Valid` → `Return Value` → `To String (Boolean)` → `In Bool`

3. **Conecte:**
   - **Pin exato:** `To String (Boolean)` → `Return Value` → `Format Text` → `{2}`

### **PASSO 7.4: Adicionar Print String**

1. **Adicione `Print String`:**
   - Conecte `Format Text` → `Result` → `Print String` → `In String`
   - Conecte `Print String` → `then` → `Set Variable: RemoteActorRef` → `execute`

### **Estrutura Visual:**

```
Branch (then)
  ↓ then
Get Array Item (RemoteActors, FoundIndex)
  ├─ Target ← Get RemoteActors
  ├─ Index ← Array_Find → Found Index
  └─ Array Element → Actor Reference
  ↓ Array Element
Is Valid
  └─ Object ← Get Array Item → Array Element
  ↓ Return Value
To String (Boolean)
  └─ In Bool ← Is Valid → Return Value
  ↓ (no fluxo de execução)
Format Text ("Get Array Item - FoundIndex: {0}, ExistingActorRef: {1}, válido: {2}")
  ├─ {0} ← Array_Find → Found Index
  ├─ {1} ← To String (Get Array Item → Array Element) → Return Value
  └─ {2} ← To String (Boolean) → Return Value
  ↓ Result
Print String
  └─ In String ← Format Text → Result
  ↓ then
Set Variable: RemoteActorRef
```

---

## 🔧 **ETAPA 8: Adicionar Log de Contador de Spawns**

### **PASSO 8.1: Adicionar Variável de Contador (Opcional, mas Recomendado)**

1. **No painel "Meu Blueprint"**, clique em **"+ New Variable"**
2. **Configure:**
   - **Nome:** `SpawnCount`
   - **Tipo:** `Integer`
   - **Visibilidade:** `Private`

### **PASSO 8.2: Adicionar Log ANTES de SpawnActorFromClass com Contador**

1. **Antes do `Format Text` da Etapa 3**, adicione:
   - **Format Pattern:** `"[ProcessNextFrame] SPAWN #{0} - PlayerID: {1}, Location: ({2}, {3}, {4})"`
   - **{0}:** `Get SpawnCount` → `Return Value` → Incrementar depois

2. **Após `SpawnActorFromClass`**, adicione `Increment Int`:
   - Clique direito → `Increment Int`
   - Conecte `Get SpawnCount` → `Return Value` → `Increment Int` → `Target`
   - Conecte `Increment Int` → `then` → `Set Variable: SpawnCount` → `execute`

### **Estrutura Visual:**

```
[... antes de SpawnActorFromClass ...]
  ↓ execute
Get SpawnCount
  ↓ Return Value
Format Text ("SPAWN #{0} - PlayerID: {1}, Location: ({2}, {3}, {4})")
  ├─ {0} ← Get SpawnCount → Return Value
  ├─ {1} ← ParseStateUpdateFrame → Out Player Id
  ├─ {2} ← Break Vector → X
  ├─ {3} ← Break Vector → Y
  └─ {4} ← Break Vector → Z
  ↓ Result
Print String
  ↓ then
SpawnActorFromClass
  ↓ then
Increment Int
  └─ Target ← Get SpawnCount → Return Value
  ↓ then
Set Variable: SpawnCount
  └─ Value ← Increment Int → Return Value
```

---

## ✅ **CHECKLIST FINAL:**

### **Logs Adicionados:**

- [ ] Log após `Array_Find`: `"[ProcessNextFrame] Array_Find - OutPlayerId: {0}, FoundIndex: {1}"`
- [ ] Log no caminho `then`: `"[ProcessNextFrame] Actor já existe - FoundIndex: {0}, atualizando"`
- [ ] Log no caminho `else`: `"[ProcessNextFrame] Actor não existe - FoundIndex: {0}, spawnando novo"`
- [ ] Log antes de `SpawnActorFromClass`: `"[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: {0}, Location: ({1}, {2}, {3}), Yaw: {4}"`
- [ ] Log após `SpawnActorFromClass`: `"[ProcessNextFrame] APÓS SpawnActorFromClass - PlayerID: {0}, Actor válido: {1}"`
- [ ] Log após `Array_Add (RemoteActorIds)`: `"[ProcessNextFrame] Array_Add - RemoteActorIds adicionado: {0}, tamanho agora: {1}"`
- [ ] Log após `Array_Add (RemoteActors)`: `"[ProcessNextFrame] Array_Add - RemoteActors adicionado, tamanho agora: {0}"`
- [ ] Log antes de `Set Actor Location`: `"[ProcessNextFrame] Set Actor Location - RemoteActorRef: {0}, Location: ({1}, {2}, {3})"`
- [ ] Log após `Get Array Item`: `"[ProcessNextFrame] Get Array Item - FoundIndex: {0}, ExistingActorRef: {1}, válido: {2}"`
- [ ] (Opcional) Log com contador de spawns: `"[ProcessNextFrame] SPAWN #{0} - PlayerID: {1}, Location: ({2}, {3}, {4})"`

---

## 🎯 **O QUE PROCURAR NOS LOGS:**

### **Padrão Normal (Esperado):**

```
[ProcessNextFrame] Array_Find - OutPlayerId: 14, FoundIndex: -1
[ProcessNextFrame] Actor não existe - FoundIndex: -1, spawnando novo
[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: 14, Location: (-320.0, 710.0, 92.0), Yaw: 0.0
[ProcessNextFrame] APÓS SpawnActorFromClass - PlayerID: 14, Actor válido: True
[ProcessNextFrame] Array_Add - RemoteActorIds adicionado: 14, tamanho agora: 1
[ProcessNextFrame] Array_Add - RemoteActors adicionado, tamanho agora: 1
[ProcessNextFrame] Set Actor Location - RemoteActorRef: [Actor], Location: (-320.0, 710.0, 92.0)
```

### **Padrão Problema (Spawn Duplicado):**

```
[ProcessNextFrame] Array_Find - OutPlayerId: 14, FoundIndex: -1
[ProcessNextFrame] Actor não existe - FoundIndex: -1, spawnando novo
[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: 14, Location: (-320.0, 710.0, 92.0), Yaw: 0.0
[ProcessNextFrame] APÓS SpawnActorFromClass - PlayerID: 14, Actor válido: True
[ProcessNextFrame] Array_Add - RemoteActorIds adicionado: 14, tamanho agora: 1
[ProcessNextFrame] Array_Add - RemoteActors adicionado, tamanho agora: 1

[ProcessNextFrame] Array_Find - OutPlayerId: 14, FoundIndex: -1  ← PROBLEMA: Deveria ser 0!
[ProcessNextFrame] Actor não existe - FoundIndex: -1, spawnando novo  ← SPAWN DUPLICADO!
[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: 14, Location: (0.0, 0.0, 0.0), Yaw: 0.0  ← Location incorreta!
```

**Isso indicaria que:**
- `Array_Add` não está atualizando o array antes do próximo frame chegar
- Ou `Array_Find` não está encontrando o actor recém-adicionado
- Ou há um frame com `Location (0,0,0)` sendo processado

---

## 📝 **PRÓXIMOS PASSOS APÓS ADICIONAR LOGS:**

1. **Compilar o Blueprint**
2. **Testar com 2 clients simultâneos**
3. **Coletar logs completos**
4. **Analisar padrões:**
   - Quantas vezes `SpawnActorFromClass` é chamado para o mesmo `PlayerID`?
   - Qual `FoundIndex` é retornado antes de cada spawn?
   - Qual `Location` é usada em cada spawn?
   - `Array_Add` está sendo executado antes do próximo frame chegar?

---

**Fim do Guia Detalhado**

