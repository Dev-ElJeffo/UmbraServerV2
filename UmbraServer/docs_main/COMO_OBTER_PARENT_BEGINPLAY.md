# 🔧 **COMO OBTER Parent: BeginPlay no Blueprint**

## 🎯 **PROBLEMA:**

**`Parent: BeginPlay` não aparece quando você procura diretamente no menu.**

---

## ✅ **SOLUÇÃO 1: Criar Event BeginPlay Primeiro**

**O `Parent: BeginPlay` só aparece DEPOIS de criar o `Event BeginPlay`!**

### **PASSO 1: Adicionar Event BeginPlay**

1. **Event Graph** → **Botão direito** → Procure por **`Event BeginPlay`**
2. **Selecione:** `Event BeginPlay` (deve aparecer como evento do Actor)
3. **Clique** para adicionar ao graph

### **PASSO 2: Obter Parent: BeginPlay**

**Depois de adicionar o `Event BeginPlay`:**

1. **Clique com botão direito** no **`Event BeginPlay`** que você acabou de adicionar
2. **Menu de contexto** → Procure por **`Add Call to Parent Function`** ou **`Call Parent Function`**
3. **OU:**
   - **Arraste** o pin `then` do `Event BeginPlay` para baixo
   - **Menu de contexto** deve aparecer automaticamente
   - Procure por **`Parent: BeginPlay`** ou **`Call Parent: BeginPlay`**

**Se ainda não aparecer:**

---

## ✅ **SOLUÇÃO 2: Usar "Call Parent Function"**

1. **Event Graph** → **Botão direito** → Procure por **`Call Parent Function`**
2. **Selecione:** `Call Parent Function`
3. **No node que aparece:**
   - **Function:** Clique na seta e selecione **`BeginPlay`**
   - **OU:** Digite `BeginPlay` no campo de busca

**Isso cria um node que chama o `BeginPlay` do parent (C++)!**

---

## ✅ **SOLUÇÃO 3: Verificar se Blueprint Herda Corretamente**

**Se `Parent: BeginPlay` não aparece, o Blueprint pode não estar reconhecendo a herança:**

1. **Content Browser** → Encontre o **novo** `BP_NetMovementClient`
2. **Duplo clique** para abrir
3. **Toolbar** → **File** → **Reparent Blueprint**
4. **Verifique:** Deve mostrar `NetMovementClient` (C++) como parent
5. **Se não estiver:**
   - **Selecione:** `NetMovementClient` (C++)
   - **Clique:** `Reparent`
   - **Compile** o Blueprint

---

## ✅ **SOLUÇÃO 4: Alternativa - Não Sobrescrever BeginPlay**

**Se não conseguir obter `Parent: BeginPlay`, use um Custom Event:**

### **PASSO 1: Criar Custom Event**

1. **Event Graph** → **Botão direito** → **Custom Event**
2. **Nome:** `ConnectWebSocketWhenReady`

### **PASSO 2: Chamar do BeginPlay (sem sobrescrever)**

**NÃO adicione `Event BeginPlay` no Blueprint!**

**Em vez disso, chame o Custom Event de outro lugar:**

**Opção A: Chamar do Character quando estiver pronto**

**No `BP_ThirdPersonCharacter`:**

```
[Event BeginPlay]
  ↓
[Delay: 2.0]
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Is Valid?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual] (do NetMovementClientRef)
  └─ False:
       ↓
     [Print String: "NetMovementClientRef inválido!"]
```

**Opção B: Usar Timer no Blueprint**

**No novo `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → **Event Tick** (ou crie um Custom Event)
2. **Adicione lógica:**

```
[Event Tick] OU [Custom Event: CheckAndConnect]
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual]
  │    ↓
  │  [Set Timer by Function Name] → DESABILITAR TICK OU REMOVER TIMER
  └─ False:
       ↓
     (nada - continua verificando)
```

**E chame esse Custom Event de um Timer:**

```
[Event BeginPlay] (do Blueprint - SEM Parent: BeginPlay)
  ↓
[Set Timer by Function Name]
  ├─ Function Name: "CheckAndConnect"
  ├─ Time: 2.0
  ├─ Looping: true
  ↓
[Print String: "[BP_NEW] Timer iniciado para verificar conexão..."]
```

---

## ✅ **SOLUÇÃO 5: Modificar C++ para Não Retornar Cedo**

**Se nada funcionar, modifique o C++ para não retornar quando `HasActiveCharacter` é `false`:**

**No `NetMovementClient.cpp`:**

```cpp
void ANetMovementClient::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ========== BeginPlay INICIADO! =========="));

    // Get Game Instance
    MyGameInstance = Cast<UUmbraGameInstance>(GetGameInstance());
    
    if (!MyGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[NetMovementClient] ❌ GameInstance não encontrado!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ✅ GameInstance encontrado!"));

    // Check if we're on a client (not server)
    ENetMode NetMode = GetNetMode();
    
    if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NetMovementClient] ⚠️ Executando no Server, ignorando..."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ✅ Executando no Client!"));

    // NÃO RETORNAR SE HasActiveCharacter É FALSE!
    // Apenas logar e deixar o Blueprint chamar ConnectWebSocketManual depois
    bool bHasActiveCharacter = MyGameInstance->HasActiveCharacter();
    UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] HasActiveCharacter: %d"), bHasActiveCharacter ? 1 : 0);
    
    if (bHasActiveCharacter)
    {
        // Set Player ID
        MyPlayerId = MyGameInstance->GetActivePlayerID();
        UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ✅ MyPlayerId setado: %d"), MyPlayerId);

        // Create WebSocket
        FString WebSocketUrl = MyGameInstance->GetZoneServerWebSocketURL();
        UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] WebSocket URL: %s"), *WebSocketUrl);
        
        if (!WebSocketUrl.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ✅ Criando e conectando WebSocket..."));
            CreateAndConnectWebSocket(WebSocketUrl);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[NetMovementClient] ❌ WebSocket URL vazia!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[NetMovementClient] ⚠️ Nenhum personagem selecionado ainda!"));
        UE_LOG(LogTemp, Warning, TEXT("[NetMovementClient] ⚠️ Blueprint deve chamar ConnectWebSocketManual quando pronto!"));
        // NÃO RETORNAR - deixar o Blueprint continuar
    }

    UE_LOG(LogTemp, Log, TEXT("[NetMovementClient] ========== BeginPlay FINALIZADO! =========="));
}
```

**Com isso, o `BeginPlay` do C++ não retorna cedo, e você pode chamar `ConnectWebSocketManual` do Blueprint sem precisar de `Parent: BeginPlay`!**

---

## 🎯 **RECOMENDAÇÃO:**

**Use a SOLUÇÃO 4 (Custom Event chamado do Character) ou SOLUÇÃO 5 (modificar C++).**

**São mais simples e não dependem de `Parent: BeginPlay`!**

