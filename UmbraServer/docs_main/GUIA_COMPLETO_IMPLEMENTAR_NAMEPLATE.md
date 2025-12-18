# ✅ **GUIA COMPLETO: Implementar Sincronização de Nameplate via Zone Server**

## 🎯 **OBJETIVO**

Sincronizar nome e título dos personagens entre todos os clientes via Zone Server (WebSocket), permitindo que cada cliente veja o nome/título de todos os outros jogadores.

---

## 📋 **ETAPAS DE IMPLEMENTAÇÃO**

### **ETAPA 1: Compilar Código C++ Modificado**

1. **Abra o projeto Unreal Engine**
2. **Compile o código C++:**
   - `Tools` → `Refresh Visual Studio Project Files` (se necessário)
   - `File` → `Compile` ou pressione `Ctrl+Alt+F11`
3. **Aguarde a compilação terminar**

---

### **ETAPA 2: Criar Função para Enviar PlayerInfoUpdate no BP_NetMovementClient**

**No Blueprint `BP_NetMovementClient`:**

1. **Abra o Blueprint `BP_NetMovementClient`**
2. **Crie uma nova função:**
   - **Nome:** `SendPlayerInfoUpdate`
   - **Categoria:** `Net|WebSocket`
3. **Adicione os parâmetros:**
   - `CharacterName` (String)
   - `CharacterTitle` (String)
4. **Implemente a lógica:**

```
[SendPlayerInfoUpdate]
    Inputs: CharacterName (String), CharacterTitle (String)
    ↓
[Get WebSocketRef]
    ↓
[Is Valid]
    ↓
[Branch]
    Condition: ReturnValue (do Is Valid)
    True → [WSBinaryBPFL::EncodePlayerInfoUpdate]
              PlayerID: MyPlayerId
              CharacterName: CharacterName
              CharacterTitle: CharacterTitle
           → [Send Binary] (do WebSocketRef)
              Data: ReturnValue (do EncodePlayerInfoUpdate)
           → [Print String]
              InString: "✅ [SendPlayerInfoUpdate] Enviado: PlayerID={MyPlayerId}, Name={CharacterName}, Title={CharacterTitle}"
    False → [Print String]
              InString: "❌ [SendPlayerInfoUpdate] WebSocketRef inválido!"
```

**⚠️ IMPORTANTE:**
- O `MyPlayerId` deve estar setado corretamente
- O `WebSocketRef` deve estar conectado antes de chamar esta função

---

### **ETAPA 3: Modificar OnWSBinaryMessage para Processar PlayerInfoUpdate**

**No Blueprint `BP_NetMovementClient`:**

1. **Localize o evento `OnWSBinaryMessage`**
2. **NO INÍCIO do evento, ANTES de processar StateUpdate, adicione:**

```
[OnWSBinaryMessage]
    Data: Array de bytes
    ↓
[WSBinaryBPFL::ParsePlayerInfoUpdate]
    Data: Data
    OutPlayerID: PlayerID
    OutCharacterName: CharacterName
    OutCharacterTitle: CharacterTitle
    ↓
[Branch]
    Condition: ReturnValue (do ParsePlayerInfoUpdate)
    True → [Print String]
              InString: "🔥 [OnWSBinaryMessage] PlayerInfoUpdate recebido: PlayerID={PlayerID}, Name={CharacterName}, Title={CharacterTitle}"
           → [Find Remote Actor by PlayerID]
              PlayerID: PlayerID
              (Esta função deve buscar o remote actor correspondente ao PlayerID)
           → [Is Valid] (do Remote Actor encontrado)
           → [Branch]
              Condition: ReturnValue (do Is Valid)
              True → [Get NameplateWidgetComponent] (do Remote Actor)
                   → [Is Valid] (do NameplateWidgetComponent)
                   → [Branch]
                      Condition: ReturnValue (do Is Valid)
                      True → [Get User Widget Object] (do NameplateWidgetComponent)
                           → [Cast to WBP Player Nameplate]
                           → [Branch]
                              Condition: bSuccess (do Cast)
                              True → [Update Nameplate] (do WBP Player Nameplate)
                                        CharacterName: CharacterName
                                        TitleName: CharacterTitle
                                   → [Print String]
                                        InString: "✅ [OnWSBinaryMessage] Nameplate atualizado para PlayerID={PlayerID}"
                              False → [Print String]
                                        InString: "❌ [OnWSBinaryMessage] Cast para WBP_PlayerNameplate falhou!"
                      False → [Print String]
                                InString: "❌ [OnWSBinaryMessage] NameplateWidgetComponent inválido!"
              False → [Print String]
                        InString: "⚠️ [OnWSBinaryMessage] Remote Actor não encontrado para PlayerID={PlayerID} (pode ser o próprio player)"
    False → (continuar com processamento normal de StateUpdate)
```

**⚠️ IMPORTANTE:**
- Esta verificação deve estar ANTES do processamento de `StateUpdate`
- Se `ParsePlayerInfoUpdate` retornar `true`, o frame foi processado e você pode retornar (não processar como StateUpdate)
- Se retornar `false`, continue com o processamento normal de `StateUpdate`

---

### **ETAPA 4: Criar Função Helper para Buscar Remote Actor por PlayerID**

**No Blueprint `BP_NetMovementClient`:**

1. **Crie uma nova função:**
   - **Nome:** `FindRemoteActorByPlayerID`
   - **Categoria:** `Net|RemoteActors`
   - **Tipo de Retorno:** `Actor` (ou `BP_ThirdPersonCharacter`)
2. **Adicione o parâmetro:**
   - `PlayerID` (Integer)
3. **Implemente a lógica:**

```
[FindRemoteActorByPlayerID]
    Input: PlayerID (Integer)
    ↓
[For Each Loop]
    Array: RemoteActorIds
    Array Element: CurrentID
    ↓
[Branch]
    Condition: CurrentID == PlayerID
    True → [Get Array Element]
              Array: RemoteActors
              Index: Loop Index
           → [Is Valid]
           → [Branch]
              Condition: ReturnValue (do Is Valid)
              True → [Return Node]
                        Return Value: Array Element (do RemoteActors)
    False → (continuar loop)
    ↓
[Completed] (do For Each Loop)
    ↓
[Return Node]
    Return Value: None (não encontrado)
```

**OU use uma função mais simples:**

```
[FindRemoteActorByPlayerID]
    Input: PlayerID (Integer)
    ↓
[Find Item in Array]
    Array: RemoteActorIds
    Item to Find: PlayerID
    Out Index: FoundIndex
    ↓
[Branch]
    Condition: FoundIndex >= 0
    True → [Get Array Element]
              Array: RemoteActors
              Index: FoundIndex
           → [Is Valid]
           → [Branch]
              Condition: ReturnValue (do Is Valid)
              True → [Return Node]
                        Return Value: Array Element
    False → [Return Node]
              Return Value: None
```

---

### **ETAPA 5: Enviar PlayerInfoUpdate Quando Conectar**

**No Blueprint `BP_ThirdPersonCharacter`:**

1. **Localize o `Event BeginPlay`**
2. **APÓS obter `CurrentCharacterInfo` e setar `ReplicatedCharacterName`, adicione:**

```
[Event BeginPlay]
    ↓
... (código existente para obter CharacterInfo)
    ↓
[Break Umbra Character Info]
    ↓
[Set ReplicatedCharacterName]
    Value: Character Name
    ↓
[Set ReplicatedCharacterTitle]
    Value: Title Name
    ↓
[Get All Actors of Class]
    Actor Class: BP_NetMovementClient
    ↓
[Get Array Element]
    Array: Out Actors
    Index: 0
    ↓
[Is Valid]
    ↓
[Branch]
    Condition: ReturnValue (do Is Valid)
    True → [Cast to BP Net Movement Client]
              Object: Array Element
           → [Branch]
              Condition: bSuccess (do Cast)
              True → [Send Player Info Update]
                        CharacterName: Character Name (do Break)
                        CharacterTitle: Title Name (do Break)
                   → [Print String]
                        InString: "✅ [BeginPlay] PlayerInfoUpdate enviado: Name={Character Name}, Title={Title Name}"
              False → [Print String]
                        InString: "❌ [BeginPlay] Falha ao fazer cast para BP_NetMovementClient"
    False → [Print String]
              InString: "⚠️ [BeginPlay] BP_NetMovementClient não encontrado! Aguardando..."
           → [Set Timer by Function Name]
              Function Name: RetrySendPlayerInfo
              Time: 1.0
              Looping: false
```

3. **Crie uma função `RetrySendPlayerInfo`:**

```
[RetrySendPlayerInfo]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info]
    ↓
[Break Umbra Character Info]
    ↓
[Get All Actors of Class]
    Actor Class: BP_NetMovementClient
    ↓
[Get Array Element]
    Array: Out Actors
    Index: 0
    ↓
[Is Valid]
    ↓
[Branch]
    Condition: ReturnValue (do Is Valid)
    True → [Cast to BP Net Movement Client]
              Object: Array Element
           → [Branch]
              Condition: bSuccess (do Cast)
              True → [Send Player Info Update]
                        CharacterName: Character Name
                        CharacterTitle: Title Name
                   → [Clear Timer by Function Name]
                        Function Name: RetrySendPlayerInfo
    False → [Set Timer by Function Name]
              Function Name: RetrySendPlayerInfo
              Time: 1.0
              Looping: false
```

---

### **ETAPA 6: Atualizar Nameplate Quando Remote Actor é Spawnado**

**No Blueprint `BP_NetMovementClient`:**

1. **Localize onde os remote actors são spawnados** (geralmente no `ProcessNextFrame` ou similar)
2. **APÓS spawnar um novo remote actor, adicione:**

```
[Spawn Actor from Class]
    ... (código existente)
    ↓
[Add to Array]
    Array: RemoteActors
    New Item: Spawned Actor
    ↓
[Add to Array]
    Array: RemoteActorIds
    New Item: PlayerID
    ↓
[Print String]
    InString: "✅ [Spawn] Remote Actor spawnado: PlayerID={PlayerID}"
    ↓
[Set Timer by Function Name]
    Function Name: RequestPlayerInfoForRemoteActor
    Time: 0.5
    Looping: false
    (Passar PlayerID como parâmetro se possível, ou usar variável temporária)
```

**⚠️ NOTA:** O servidor já envia `PlayerInfoUpdate` no snapshot inicial, então esta etapa pode não ser necessária. Mas é útil como fallback.

---

### **ETAPA 7: Verificar se WBP_PlayerNameplate Tem Função UpdateNameplate**

**No Blueprint `WBP_PlayerNameplate`:**

1. **Abra o Blueprint `WBP_PlayerNameplate`**
2. **Verifique se existe a função `UpdateNameplate`**
3. **Se não existir, crie:**
   - **Nome:** `UpdateNameplate`
   - **Parâmetros:**
     - `CharacterName` (String)
     - `TitleName` (String)
4. **Implemente:**

```
[UpdateNameplate]
    Inputs: CharacterName (String), TitleName (String)
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName
    ↓
[Is Empty]
    InString: TitleName
    ↓
[Branch]
    Condition: ReturnValue (do Is Empty)
    True → [Set Visibility] (TB_Title)
              Visibility: Collapsed
    False → [Set Text] (TB_Title)
              Text: TitleName
           → [Set Visibility] (TB_Title)
              Visibility: Visible
```

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO**

### **C++:**
- [ ] Funções `EncodePlayerInfoUpdate` e `ParsePlayerInfoUpdate` adicionadas ao `WSBinaryBPFL.h`
- [ ] Implementações adicionadas ao `WSBinaryBPFL.cpp`
- [ ] Código C++ compilado com sucesso

### **BP_NetMovementClient:**
- [ ] Função `SendPlayerInfoUpdate` criada
- [ ] Função `FindRemoteActorByPlayerID` criada (ou função helper similar)
- [ ] `OnWSBinaryMessage` modificado para processar `PlayerInfoUpdate` ANTES de `StateUpdate`
- [ ] Logs adicionados para diagnóstico

### **BP_ThirdPersonCharacter:**
- [ ] `Event BeginPlay` modificado para enviar `PlayerInfoUpdate` após obter `CharacterInfo`
- [ ] Função `RetrySendPlayerInfo` criada (opcional, para retry)
- [ ] Logs adicionados para diagnóstico

### **WBP_PlayerNameplate:**
- [ ] Função `UpdateNameplate` existe e funciona corretamente
- [ ] `TB_CharacterName` e `TB_Title` estão configurados corretamente

---

## 🔍 **TESTE E VERIFICAÇÃO**

### **Teste 1: Envio de PlayerInfoUpdate**

1. **Execute o jogo com 2 clientes**
2. **Verifique os logs:**
   - Deve aparecer: `"✅ [BeginPlay] PlayerInfoUpdate enviado: Name=..., Title=..."`
   - Deve aparecer: `"✅ [SendPlayerInfoUpdate] Enviado: PlayerID=..., Name=..., Title=..."`

### **Teste 2: Recebimento de PlayerInfoUpdate**

1. **No Cliente 2, verifique os logs:**
   - Deve aparecer: `"🔥 [OnWSBinaryMessage] PlayerInfoUpdate recebido: PlayerID=..., Name=..., Title=..."`
   - Deve aparecer: `"✅ [OnWSBinaryMessage] Nameplate atualizado para PlayerID=..."`

### **Teste 3: Visual**

1. **Cliente 1 deve ver:**
   - Próprio nome acima da cabeça
   - Nome do Cliente 2 acima da cabeça do Cliente 2

2. **Cliente 2 deve ver:**
   - Próprio nome acima da cabeça
   - Nome do Cliente 1 acima da cabeça do Cliente 1

---

## ⚠️ **PROBLEMAS COMUNS E SOLUÇÕES**

### **Problema 1: "ParsePlayerInfoUpdate retorna false"**

**Causa:** Frame não é `PlayerInfoUpdate` ou está corrompido.

**Solução:**
- Verifique se o primeiro byte do frame é `4` (PlayerInfoUpdate)
- Adicione logs para verificar o tamanho do `Data` array
- Verifique se o servidor está enviando corretamente

### **Problema 2: "Remote Actor não encontrado"**

**Causa:** Remote actor ainda não foi spawnado quando `PlayerInfoUpdate` chega.

**Solução:**
- O servidor envia `PlayerInfoUpdate` no snapshot inicial, então o remote actor deve existir
- Se não existir, armazene o `PlayerInfoUpdate` e aplique quando o remote actor for spawnado
- Ou use um `Map` (se disponível) para armazenar `PlayerID → CharacterName/Title`

### **Problema 3: "Nameplate não atualiza"**

**Causa:** Cast falha ou `UpdateNameplate` não está funcionando.

**Solução:**
- Verifique se o `NameplateWidgetComponent` está configurado corretamente
- Verifique se o `WBP_PlayerNameplate` está sendo usado como widget do componente
- Adicione logs dentro de `UpdateNameplate` para verificar se está sendo chamado

---

## 📋 **ORDEM DE EXECUÇÃO**

1. **Compilar C++** (Etapa 1)
2. **Criar função `SendPlayerInfoUpdate`** (Etapa 2)
3. **Criar função `FindRemoteActorByPlayerID`** (Etapa 4)
4. **Modificar `OnWSBinaryMessage`** (Etapa 3)
5. **Modificar `BP_ThirdPersonCharacter`** (Etapa 5)
6. **Verificar `WBP_PlayerNameplate`** (Etapa 7)
7. **Testar** (Seção de Teste)

---

**FIM DO GUIA**

