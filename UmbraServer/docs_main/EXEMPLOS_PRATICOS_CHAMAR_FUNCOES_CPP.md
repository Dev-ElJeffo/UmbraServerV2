# 📘 **EXEMPLOS PRÁTICOS: Chamar Funções C++ no Blueprint**

## 🎯 **COMO ACESSAR FUNÇÕES E VARIÁVEIS C++ NO BLUEPRINT:**

---

## ✅ **EXEMPLO 1: Chamar `RemoveRemoteActor` do Blueprint**

**Cenário:** Você quer remover um remote actor específico quando recebe mensagem `PlayerDisconnected`.

**No `OnWSBinaryMessage`:**

```
[OnWSBinaryMessage]
  ├─ Input: Data (Array of Bytes)
  ↓
[Branch: Data.Num() == 5?]
  ├─ True:
  │    ↓
  │  [ParsePlayerDisconnected] (do WSBinaryBPFL)
  │    ├─ Input: Data
  │    └─ Output: PlayerId
  │    ↓
  │  [Call Function: RemoveRemoteActor] ← FUNÇÃO C++
  │    ├─ Target: Self (BP_NetMovementClient)
  │    └─ Input: PlayerId
  └─ False:
       [ProcessBinaryBuffer]
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `RemoveRemoteActor`
3. **Selecione:** `RemoveRemoteActor` (deve aparecer como função do `BP_NetMovementClient`)
4. **Conecte** o `PlayerId` ao input

---

## ✅ **EXEMPLO 2: Acessar `MyPlayerId` do Blueprint**

**Cenário:** Você quer verificar se uma mensagem é do próprio player.

**No `OnWSBinaryMessage`:**

```
[ProcessBinaryBuffer] (do WSBinaryBPFL)
  ├─ Output: PlayerId
  ↓
[Get Variable: MyPlayerId] ← VARIÁVEL C++
  ↓
[Equal: PlayerId == MyPlayerId?]
  ├─ True: (é o próprio player, ignorar)
  └─ False: (é outro player, processar)
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `Get MyPlayerId` ou `MyPlayerId`
3. **Selecione:** `Get MyPlayerId` (deve aparecer como variável do `BP_NetMovementClient`)
4. **Conecte** ao `Equal` node

---

## ✅ **EXEMPLO 3: Acessar `WebSocketRef` do Blueprint**

**Cenário:** Você quer enviar dados via WebSocket na função `SendMoveUpdate`.

**Na função `SendMoveUpdate`:**

```
[SendMoveUpdate] (Function)
  ↓
[Get Local Pawn Position]
[Get Local Pawn Rotation]
  ↓
[Make Array of Bytes] (com posição, rotação, etc.)
  ↓
[Get Variable: WebSocketRef] ← VARIÁVEL C++
  ↓
[Is Valid?]
  ├─ True:
  │    ↓
  │  [Call Function: SendBytes] (do WebSocketRef)
  │    ├─ Input: Data (Array of Bytes)
  │    └─ Return: Success (Boolean)
  └─ False:
       [Print String: "WebSocketRef inválido!"]
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `Get WebSocketRef` ou `WebSocketRef`
3. **Selecione:** `Get WebSocketRef` (deve aparecer como variável do `BP_NetMovementClient`)
4. **Conecte** ao `Is Valid` node
5. **Arraste** do pin de saída do `WebSocketRef` → Procure por `SendBytes`

---

## ✅ **EXEMPLO 4: Acessar `MyGameInstance` do Blueprint**

**Cenário:** Você quer obter dados do personagem ativo.

**No `OnWSConnected`:**

```
[OnWSConnected] (Event)
  ↓
[Get Variable: MyGameInstance] ← VARIÁVEL C++
  ↓
[Is Valid?]
  ├─ True:
  │    ↓
  │  [Call Function: GetActiveCharacter] (do MyGameInstance)
  │    └─ Output: ActiveCharacter (FUmbraPlayerData)
  │    ↓
  │  [Get Position] (do ActiveCharacter)
  │    ↓
  │  [Set Actor Location] (aplicar ao local pawn)
  └─ False:
       [Print String: "MyGameInstance inválido!"]
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `Get MyGameInstance` ou `MyGameInstance`
3. **Selecione:** `Get MyGameInstance` (deve aparecer como variável do `BP_NetMovementClient`)
4. **Arraste** do pin de saída → Procure por funções do `UmbraGameInstance`

---

## ✅ **EXEMPLO 5: Usar Arrays `RemoteActors` e `RemoteActorIds`**

**Cenário:** Você quer verificar se um remote actor já existe antes de spawnar.

**No `OnWSBinaryMessage`, após processar frame:**

```
[ProcessBinaryBuffer] (do WSBinaryBPFL)
  ├─ Output: PlayerId, Location, Rotation
  ↓
[Get Variable: RemoteActorIds] ← ARRAY C++
  ↓
[Array Find] (RemoteActorIds, PlayerId)
  ├─ Output: Found Index
  ↓
[Branch: Found Index >= 0?]
  ├─ True: (actor já existe)
  │    ↓
  │  [Get Variable: RemoteActors] ← ARRAY C++
  │    ↓
  │  [Get Array Item] (RemoteActors, Found Index)
  │    ↓
  │  [Set Actor Location]
  │  [Set Actor Rotation]
  └─ False: (criar novo)
       ↓
     [Spawn Actor from Class]
       ↓
     [Get Variable: RemoteActors]
       ↓
     [Array Add] (RemoteActors, Spawned Actor)
       ↓
     [Get Variable: RemoteActorIds]
       ↓
     [Array Add] (RemoteActorIds, PlayerId)
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `Get RemoteActorIds` ou `RemoteActorIds`
3. **Selecione:** `Get RemoteActorIds` (deve aparecer como variável do `BP_NetMovementClient`)
4. **Conecte** ao `Array Find` node
5. **Repita** para `RemoteActors` quando necessário

---

## ✅ **EXEMPLO 6: Chamar `CleanupRemoteActors` Manualmente**

**Cenário:** Você quer limpar todos os remote actors em uma situação específica.

```
[Qualquer Event ou Função]
  ↓
[Call Function: CleanupRemoteActors] ← FUNÇÃO C++
  ├─ Target: Self (BP_NetMovementClient)
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `CleanupRemoteActors`
3. **Selecione:** `CleanupRemoteActors` (deve aparecer como função do `BP_NetMovementClient`)
4. **Conecte** ao fluxo de execução

---

## ✅ **EXEMPLO 7: Chamar `CloseWebSocket` Manualmente**

**Cenário:** Você quer fechar o WebSocket quando pressionar F9.

**No `BP_ThirdPersonCharacter`, no evento F9:**

```
[F9 Key Pressed]
  ↓
[GetAllActorsOfClass: BP_NetMovementClient]
  ↓
[ForEachLoopWithBreak]
  ├─ Loop Body:
  │    ↓
  │  [Get Variable: MyPlayerId] (do elemento do loop)
  │    ↓
  │  [Get ActivePlayerID] (do GameInstance)
  │    ↓
  │  [Equal: MyPlayerId == ActivePlayerID?]
  │    ├─ True:
  │    │    ↓
  │    │  [Call Function: CloseWebSocket] ← FUNÇÃO C++
  │    │    ├─ Target: (elemento do loop)
  │    │    ↓
  │    │  [Break]
  │    └─ False: (continua loop)
  └─ Completed:
       [Print String: "BP_NetMovementClient não encontrado!"]
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `CloseWebSocket`
3. **Selecione:** `CloseWebSocket` (deve aparecer como função do `BP_NetMovementClient`)
4. **Conecte** ao fluxo de execução

---

## 🔍 **DICAS IMPORTANTES:**

### **1. Funções C++ aparecem automaticamente no Blueprint**

**Todas as funções marcadas com `UFUNCTION(BlueprintCallable)` aparecem automaticamente no Blueprint!**

- ✅ `RemoveRemoteActor` - Aparece como função
- ✅ `CleanupRemoteActors` - Aparece como função
- ✅ `CloseWebSocket` - Aparece como função
- ✅ `CreateAndConnectWebSocket` - Aparece como função
- ✅ `SetMyPlayerId` - Aparece como função

### **2. Variáveis C++ aparecem automaticamente no Blueprint**

**Todas as variáveis marcadas com `UPROPERTY(BlueprintReadWrite)` aparecem automaticamente no Blueprint!**

- ✅ `WebSocketRef` - Aparece como variável
- ✅ `MyPlayerId` - Aparece como variável
- ✅ `MyGameInstance` - Aparece como variável
- ✅ `RemoteActors` - Aparece como variável (array)
- ✅ `RemoteActorIds` - Aparece como variável (array)

### **3. Eventos C++ são chamados automaticamente**

**Todos os eventos marcados com `UFUNCTION(BlueprintImplementableEvent)` são chamados automaticamente pelo C++!**

- ✅ `OnWSConnected` - Chamado quando WebSocket conecta
- ✅ `OnWSClosed` - Chamado quando WebSocket fecha
- ✅ `OnWSBinaryMessage` - Chamado quando recebe mensagem binária
- ✅ `OnWSError` - Chamado quando há erro

**Você só precisa implementar a lógica dentro desses eventos no Blueprint!**

---

## 🎯 **RESUMO:**

**Para chamar funções C++:**
1. **Botão direito** → Procure pelo nome da função
2. **Selecione** a função (deve aparecer como função do `BP_NetMovementClient`)
3. **Conecte** os inputs necessários

**Para acessar variáveis C++:**
1. **Botão direito** → Procure por `Get [NomeVariável]`
2. **Selecione** a variável (deve aparecer como variável do `BP_NetMovementClient`)
3. **Use** o valor retornado

**Para implementar eventos C++:**
1. **Botão direito** → Procure pelo nome do evento
2. **Selecione** o evento (deve aparecer como evento)
3. **Implemente** a lógica dentro do evento

**Tudo funciona automaticamente! Não precisa fazer nada especial!**

