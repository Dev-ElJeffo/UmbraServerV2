# Integração com Unreal Engine 5

Guia para integração do cliente UE5 com os servidores UmbraEternum.

## Plugins Necessários

### 1. VaRest
**Função**: Chamadas REST API  
**Download**: [Unreal Marketplace](https://www.unrealengine.com/marketplace/en-US/product/varest-plugin)

Usado para autenticação e requisições HTTP.

### 2. WebSockets
**Função**: Comunicação em tempo real  
**Built-in**: UE5 (Habilitar em Plugins)

### 3. Advanced Sessions (Opcional)
**Função**: Gerenciamento de sessões multiplayer  
**Download**: [Unreal Marketplace](https://www.unrealengine.com/marketplace/)

## Fluxo de Autenticação

### Blueprint: Login

```cpp
// C++ Example
#include "VaRestSubsystem.h"

void UMyLoginWidget::PerformLogin(FString Username, FString Password)
{
    // Create REST request
    UVaRestRequestJSON* Request = FVaRestSubsystem::Get()->CreateRequestJSON();
    Request->SetVerb(ERequestVerb::POST);
    Request->SetRequestURL("http://localhost:8080/auth/login");
    
    // Set payload
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("username", Username);
    JsonObject->SetStringField("password", Password);
    Request->SetRequestObject(JsonObject);
    
    // Bind response
    Request->OnRequestComplete.AddDynamic(this, &UMyLoginWidget::OnLoginResponse);
    Request->ProcessRequest();
}

void UMyLoginWidget::OnLoginResponse(UVaRestRequestJSON* Request)
{
    if (Request->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> Response = Request->GetResponseObject();
        
        FString Token = Response->GetStringField("token");
        int32 PlayerId = Response->GetIntegerField("player_id");
        
        // Store token for future requests
        UGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UGameInstanceSubsystem>();
        GI->SetAuthToken(Token);
        
        // Connect to game server
        ConnectToGameServer(Token);
    }
}
```

## Conexão WebSocket

### Blueprint: Connect to Zone Server

```cpp
#include "WebSocketsModule.h"
#include "IWebSocket.h"

void UMyGameInstance::ConnectToGameServer(FString AuthToken)
{
    // Create WebSocket connection
    FString ServerURL = "ws://localhost:8082";
    
    TSharedRef<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL);
    
    // Bind events
    WebSocket->OnConnected().AddLambda([]()
    {
        UE_LOG(LogTemp, Log, TEXT("WebSocket Connected"));
    });
    
    WebSocket->OnMessage().AddLambda([this](const FString& Message)
    {
        HandleServerMessage(Message);
    });
    
    WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        UE_LOG(LogTemp, Warning, TEXT("WebSocket Closed: %s"), *Reason);
    });
    
    // Connect
    WebSocket->Connect();
    
    // Store reference
    GameWebSocket = WebSocket;
}
```

## Sincronização de Posição

### C++: Send Player Position

```cpp
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Send position update every 50ms (~20Hz)
    static float TimeSinceLastUpdate = 0.0f;
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= 0.05f && GameWebSocket->IsConnected())
    {
        TimeSinceLastUpdate = 0.0f;
        SendPositionUpdate();
    }
}

void AMyCharacter::SendPositionUpdate()
{
    FVector Location = GetActorLocation();
    
    // Create JSON message
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("type", "player_move");
    JsonObject->SetNumberField("x", Location.X);
    JsonObject->SetNumberField("y", Location.Y);
    JsonObject->SetNumberField("z", Location.Z);
    
    FString MessageString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MessageString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    GameWebSocket->Send(MessageString);
}
```

## Recebimento de Atualizações

### C++: Handle Server Messages

```cpp
void UMyGameInstance::HandleServerMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        FString MessageType = JsonObject->GetStringField("type");
        
        if (MessageType == "player_update")
        {
            int32 PlayerId = JsonObject->GetIntegerField("player_id");
            float X = JsonObject->GetNumberField("x");
            float Y = JsonObject->GetNumberField("y");
            float Z = JsonObject->GetNumberField("z");
            
            UpdateOtherPlayerPosition(PlayerId, FVector(X, Y, Z));
        }
        else if (MessageType == "spawn_entity")
        {
            // Handle entity spawn
        }
        else if (MessageType == "chat_message")
        {
            // Handle chat message
        }
    }
}
```

## Sistema de Chat

### Blueprint: Send Chat Message

```cpp
void UMyChatWidget::SendChatMessage(FString Channel, FString Message)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("type", "chat_message");
    JsonObject->SetStringField("channel", Channel);
    JsonObject->SetStringField("message", Message);
    
    FString MessageString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MessageString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    ChatWebSocket->Send(MessageString);
}
```

## Otimizações

### 1. Client-Side Prediction
```cpp
void AMyCharacter::MoveForward(float Value)
{
    // Apply movement immediately (client-side prediction)
    AddMovementInput(GetActorForwardVector(), Value);
    
    // Send to server for validation
    SendMovementInput(Value);
}

void AMyCharacter::ServerReconciliation(FVector ServerPosition)
{
    // Server rejected movement, reconcile
    SetActorLocation(ServerPosition);
}
```

### 2. Interpolation
```cpp
void AOtherPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Smooth interpolation to server position
    FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetServerPosition, DeltaTime, 10.0f);
    SetActorLocation(NewLocation);
}
```

### 3. Delta Compression
```cpp
// Only send changed values
if (FVector::DistSquared(LastSentPosition, CurrentPosition) > 100.0f)
{
    SendPositionUpdate();
    LastSentPosition = CurrentPosition;
}
```

## Debugging

### Enable Verbose Logging
```cpp
UE_LOG(LogTemp, Verbose, TEXT("Sending: %s"), *MessageString);
UE_LOG(LogTemp, Verbose, TEXT("Received: %s"), *Message);
```

### Network Stats
```
stat net
net pktlag=100
net pktloss=5
```

## Melhores Práticas

1. **Server-Authoritative**: Nunca confie em dados do cliente
2. **Validate Everything**: Valide todas as ações no servidor
3. **Rate Limiting**: Limite número de mensagens por segundo
4. **Reconnection**: Implemente reconexão automática
5. **Timeout**: Detecte e trate timeouts de conexão
6. **Compression**: Comprima mensagens grandes
7. **Binary Protocol**: Use Protobuf para performance crítica

## Testes

### PIE (Play In Editor) Multiplayer
```
Editor Preferences → Play → Multiplayer Options
Number of Players: 2+
Net Mode: Play As Listen Server
```

### Standalone Testing
```bash
# Launch client
UE5Editor.exe "ProjectPath/Project.uproject" -game

# Launch multiple clients
UE5Editor.exe "ProjectPath/Project.uproject" -game -windowed -resx=800 -resy=600
```

## Referências

- [UE5 Network Documentation](https://docs.unrealengine.com/5.0/en-US/networking-and-multiplayer-in-unreal-engine/)
- [VaRest Plugin Documentation](https://github.com/ufna/VaRest/wiki)
- [WebSocket Protocol](https://tools.ietf.org/html/rfc6455)

