# 🔧 **GUIA: Adicionar Botão para Desconectar WebSocket (Teste de Cleanup)**

## 🎯 **OBJETIVO:**

Adicionar um botão na UI que permite desconectar o WebSocket manualmente, permitindo testar se os actors remotos são destruídos corretamente quando um cliente desconecta.

---

## 📋 **OPÇÕES DE IMPLEMENTAÇÃO:**

### **OPÇÃO 1: Adicionar Botão no HUD (Recomendado para Teste)**

Criar um widget simples de HUD com um botão "Desconectar" que fica visível durante o gameplay.

### **OPÇÃO 2: Adicionar Botão em Widget Existente**

Adicionar o botão em um widget já existente (ex: `WBP_CharacterSelection` ou criar um widget de menu in-game).

### **OPÇÃO 3: Usar Console Command (Mais Rápido para Teste)**

Adicionar uma função C++ que pode ser chamada via console.

---

## 🛠️ **IMPLEMENTAÇÃO: OPÇÃO 1 - Widget HUD de Teste**

### **PASSO 1: Criar Widget Blueprint de Teste**

1. **No Content Browser:**
   - Clique com botão direito → **User Interface** → **Widget Blueprint**
   - Nome: `WBP_TestDisconnect`

2. **No Designer:**
   - Adicione um **Canvas Panel** (raiz)
   - Adicione um **Button** dentro do Canvas Panel
     - Nome: `BTN_Disconnect`
     - Posição: Top-Right (ex: X=1600, Y=50)
     - Tamanho: Width=200, Height=50
     - Text: "🔴 DESCONECTAR"
     - Background Color: Vermelho (#e74c3c)

3. **Configurar Variáveis:**
   - No painel **My Blueprint**, adicione:
     - **NetMovementClientRef**
       - Type: `BP Net Movement Client` (Object Reference)
       - Instance Editable: FALSE

---

### **PASSO 2: Event Graph - Conectar Botão**

**No Event Graph do `WBP_TestDisconnect`:**

```
[Event Construct]
  ↓
[Get All Actors of Class: BP_NetMovementClient]
  ↓
[Get Array Length]
  ↓
[Branch: Length > 0?]
  ├─ True:
  │   ├─ [Get Array Item] (índice 0)
  │   └─ [Set] NetMovementClientRef
  └─ False:
      └─ [Print String: "⚠️ BP_NetMovementClient não encontrado!"]
```

**Conectar o evento do botão:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Branch: Is Valid (NetMovementClientRef)?]
  ├─ True:
  │   ├─ [Print String: "🔴 Desconectando WebSocket..."]
  │   ├─ [Get Variable: WebSocketRef] (do NetMovementClientRef)
  │   ├─ [Branch: Is Valid (WebSocketRef)?]
  │   │   ├─ True:
  │   │   │   ├─ [Call Function: Close] (do WebSocketRef)
  │   │   │   └─ [Print String: "✅ WebSocket fechado!"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef é inválido!"]
  │   └─ False:
  │       └─ [Print String: "⚠️ NetMovementClientRef é inválido!"]
```

---

### **PASSO 3: Adicionar Widget ao Viewport**

**No `BP_NetMovementClient` ou no `BP_ThirdPersonCharacter`:**

**Opção A: No BeginPlay do BP_NetMovementClient:**

```
[Event BeginPlay]
  ↓
[Delay: 1.0] (aguardar inicialização)
  ↓
[Create Widget: WBP_TestDisconnect]
  ↓
[Add to Viewport]
  ↓
[Set Viewport Mode: Viewport]
```

**Opção B: No BeginPlay do BP_ThirdPersonCharacter:**

```
[Event BeginPlay]
  ↓
[Get All Actors of Class: BP_NetMovementClient]
  ↓
[Branch: Length > 0?]
  ├─ True:
  │   ├─ [Delay: 1.0]
  │   ├─ [Create Widget: WBP_TestDisconnect]
  │   ├─ [Add to Viewport]
  │   └─ [Set Viewport Mode: Viewport]
  └─ False:
      └─ [Print String: "⚠️ BP_NetMovementClient não encontrado!"]
```

---

## 🛠️ **IMPLEMENTAÇÃO: OPÇÃO 2 - Adicionar ao Widget Existente**

### **Se você já tem um Widget de Menu/HUD:**

1. **Abra o Widget existente** (ex: `WBP_CharacterSelection` ou crie `WBP_GameHUD`)

2. **Adicione um Button:**
   - Nome: `BTN_Disconnect`
   - Posição: Top-Right
   - Text: "🔴 DESCONECTAR"

3. **No Event Graph, conecte:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Get All Actors of Class: BP_NetMovementClient]
  ↓
[Get Array Item] (índice 0)
  ↓
[Get Variable: WebSocketRef]
  ↓
[Branch: Is Valid (WebSocketRef)?]
  ├─ True:
  │   ├─ [Call Function: Close] (do WebSocketRef)
  │   └─ [Print String: "🔴 WebSocket desconectado!"]
  └─ False:
      └─ [Print String: "⚠️ WebSocketRef inválido!"]
```

---

## 🛠️ **IMPLEMENTAÇÃO: OPÇÃO 3 - Console Command (Mais Rápido)**

### **PASSO 1: Adicionar Função C++ em UmbraGameInstance**

**No arquivo `UmbraGameInstance.h`:**

```cpp
UFUNCTION(Exec, Category="Umbra|Net|WS")
void DisconnectWebSocket();
```

**No arquivo `UmbraGameInstance.cpp`:**

```cpp
void UUmbraGameInstance::DisconnectWebSocket()
{
    // Encontrar BP_NetMovementClient no nível
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABP_NetMovementClient::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        ABP_NetMovementClient* NetClient = Cast<ABP_NetMovementClient>(FoundActors[0]);
        if (NetClient && NetClient->WebSocketRef)
        {
            NetClient->WebSocketRef->Close();
            UE_LOG(LogTemp, Warning, TEXT("🔴 [Console] WebSocket desconectado via console!"));
        }
    }
}
```

**NOTA:** Você precisará expor `WebSocketRef` como `UPROPERTY(BlueprintReadWrite)` em `BP_NetMovementClient` ou criar uma função pública `Disconnect()`.

### **PASSO 2: Usar no Console**

1. **Durante o gameplay, pressione ` (til) para abrir o console**
2. **Digite:** `DisconnectWebSocket`
3. **Pressione Enter**

---

## 🧪 **COMO TESTAR:**

### **TESTE 1: Desconexão Manual**

1. **Inicie o servidor C++ (`zone_server`)**
2. **Inicie 2 clientes PIE**
3. **No Cliente 1:**
   - Clique no botão "🔴 DESCONECTAR" (ou use console command)
   - O WebSocket deve fechar
   - `OnWSClosed` deve disparar
   - `CleanupRemoteActors` deve ser chamado
4. **No Cliente 2:**
   - O actor remoto do Cliente 1 deve desaparecer
   - Logs devem mostrar: `"PlayerDisconnected processado"` e `"RemoveRemoteActor executado"`

### **TESTE 2: Verificar Logs**

**Logs esperados no Cliente 1 (que desconectou):**

```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] 🔴 Desconectando WebSocket...
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] ✅ WebSocket fechado!
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] [OnWSClosed] WebSocket fechado, limpando remote actors...
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] [OnWSClosed] CleanupRemoteActors executado!
```

**Logs esperados no Cliente 2 (que permanece conectado):**

```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Received binary message, size:5
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] PlayerDisconnected processado
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] RemoveRemoteActor executado para PlayerID: X
```

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **OPÇÃO 1 (Widget HUD):**
- [ ] Widget `WBP_TestDisconnect` criado
- [ ] Botão `BTN_Disconnect` adicionado e configurado
- [ ] Variável `NetMovementClientRef` criada
- [ ] Event Graph: `Event Construct` busca `BP_NetMovementClient`
- [ ] Event Graph: `BTN_Disconnect.OnClicked` chama `Close()` no `WebSocketRef`
- [ ] Widget adicionado ao Viewport no `BeginPlay` do `BP_NetMovementClient` ou `BP_ThirdPersonCharacter`
- [ ] Testado: Botão aparece no jogo
- [ ] Testado: Clicar no botão desconecta o WebSocket
- [ ] Testado: `OnWSClosed` dispara
- [ ] Testado: `CleanupRemoteActors` é chamado
- [ ] Testado: Actors remotos são destruídos em outros clientes

### **OPÇÃO 2 (Widget Existente):**
- [ ] Botão `BTN_Disconnect` adicionado ao widget existente
- [ ] Event Graph: `BTN_Disconnect.OnClicked` implementado
- [ ] Testado: Botão funciona corretamente

### **OPÇÃO 3 (Console Command):**
- [ ] Função `DisconnectWebSocket()` adicionada em `UmbraGameInstance.h`
- [ ] Função `DisconnectWebSocket()` implementada em `UmbraGameInstance.cpp`
- [ ] `WebSocketRef` exposto como `BlueprintReadWrite` ou função pública `Disconnect()` criada
- [ ] Testado: Comando funciona no console
- [ ] Testado: WebSocket desconecta corretamente

---

## 🔍 **VERIFICAÇÕES ADICIONAIS:**

### **Se o Botão Não Aparecer:**

1. **Verifique se o Widget foi adicionado ao Viewport:**
   - Adicione um `Print String` após `Add to Viewport`
   - Verifique se o log aparece

2. **Verifique se o Widget está sendo criado:**
   - Adicione um `Print String` após `Create Widget`
   - Verifique se o log aparece

3. **Verifique Z-Order:**
   - O widget pode estar atrás de outros elementos
   - Tente aumentar o `Z-Order` do Canvas Panel

### **Se o WebSocket Não Desconectar:**

1. **Verifique se `WebSocketRef` é válido:**
   - Adicione `Is Valid` check antes de chamar `Close()`
   - Adicione logs para debug

2. **Verifique se `Close()` está sendo chamado:**
   - Adicione um `Print String` antes e depois de `Close()`
   - Verifique se os logs aparecem

3. **Verifique se `OnWSClosed` está conectado:**
   - No `BP_NetMovementClient`, verifique se `OnWSClosed` do `WebSocketRef` está conectado a `CleanupRemoteActors`

---

## 📝 **NOTAS IMPORTANTES:**

1. **O botão é apenas para TESTE.** Em produção, você pode remover ou esconder o botão.

2. **O `OnWSClosed` deve estar conectado ao `CleanupRemoteActors`** no `BP_NetMovementClient` para que os actors sejam destruídos corretamente.

3. **O servidor C++ deve estar enviando a mensagem `PlayerDisconnected`** quando um cliente desconecta. Verifique se `MovementServer.hpp` está implementado corretamente.

4. **O `RemoveRemoteActor` no Blueprint deve estar processando corretamente** a mensagem `PlayerDisconnected` de 5 bytes.

---

## 🎯 **PRÓXIMOS PASSOS APÓS TESTE:**

Se os actors **NÃO** estão sendo destruídos:

1. **Verifique se `OnWSClosed` está disparando:**
   - Adicione logs no início de `OnWSClosed`
   - Verifique se os logs aparecem quando você desconecta

2. **Verifique se `CleanupRemoteActors` está sendo chamado:**
   - Adicione logs no início de `CleanupRemoteActors`
   - Verifique se os logs aparecem

3. **Verifique se o servidor está enviando `PlayerDisconnected`:**
   - Verifique os logs do servidor C++
   - Deve aparecer: `"Broadcasted PlayerDisconnected message for player X"`

4. **Verifique se `OnWSBinaryMessage` está processando mensagens de 5 bytes:**
   - Adicione logs quando `DataLength == 5`
   - Verifique se `ParsePlayerDisconnected` está sendo chamado

---

## 📚 **REFERÊNCIAS:**

- `UmbraWSClient.h` - Função `Close()` disponível
- `BP_NetMovementClient` - Gerencia `WebSocketRef` e `CleanupRemoteActors`
- `MovementServer.hpp` - Envia mensagem `PlayerDisconnected` quando cliente desconecta
- `WSBinaryBPFL.cpp` - Função `ParsePlayerDisconnected` para decodificar mensagem

---

**✅ Guia completo para adicionar botão de desconexão e testar cleanup de actors remotos!**


