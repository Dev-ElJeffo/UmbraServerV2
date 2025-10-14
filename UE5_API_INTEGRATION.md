# 🎮 INTEGRAÇÃO UNREAL ENGINE 5 - APIs UmbraEternum

**Versão**: 1.0  
**Data**: 2025-10-14  
**UE5 Version**: 5.3+

---

## 📋 ÍNDICE

1. [Requisitos](#requisitos)
2. [Setup Inicial](#setup-inicial)
3. [Plugin VaRest](#plugin-varest)
4. [Estruturas de Dados](#estruturas-de-dados)
5. [Sistema de Autenticação](#sistema-de-autenticação)
6. [Exemplos de Uso](#exemplos-de-uso)
7. [Blueprints](#blueprints)
8. [C++ (Avançado)](#c-avançado)
9. [Boas Práticas](#boas-práticas)

---

## 🔧 REQUISITOS

### Software Necessário

- ✅ Unreal Engine 5.3+ instalado
- ✅ Visual Studio 2022 (para C++)
- ✅ Plugin VaRest (via Marketplace)
- ✅ Servidor funcionando (localhost ou IP público)

### Conhecimento Recomendado

- ⭐ Blueprints básico
- ⭐⭐ HTTP/REST APIs
- ⭐⭐⭐ C++ UE5 (opcional)

---

## 🚀 SETUP INICIAL

### Passo 1: Criar Projeto UE5

1. Abra Unreal Engine Launcher
2. Crie novo projeto:
   - Template: **Third Person** ou **Blank**
   - Nome: `UmbraEternumClient`
   - Blueprint ou C++: **Escolha sua preferência**

### Passo 2: Instalar Plugin VaRest

#### Via Marketplace (Recomendado)

1. Abra Epic Games Launcher
2. Marketplace → Busque "VaRest"
3. Clique em "Free" / "Install to Engine"
4. Selecione UE 5.3
5. Install

#### Habilitar no Projeto

1. No Unreal Editor: **Edit → Plugins**
2. Busque "VaRest"
3. Marque checkbox "Enabled"
4. Restart Editor

### Passo 3: Configurar URLs Base

**Content Browser**:
1. Botão direito → **Miscellaneous → Data Asset**
2. Selecione **Data Asset**
3. Nome: `DA_ServerConfig`

**Criar Blueprint Function Library**:
1. Botão direito → **Blueprint Class**
2. Parent: **Blueprint Function Library**
3. Nome: `BPL_APIHelper`

---

## 📦 PLUGIN VAREST

### Principais Nodes (Blueprints)

| Node | Função |
|------|--------|
| `Construct Json Request` | Cria requisição HTTP |
| `Apply URL` | Define URL do endpoint |
| `Set Verb` | Define método (GET, POST, PUT, DELETE) |
| `Set Content As String` | Define body JSON |
| `Process Request` | Envia requisição |
| `On Request Complete` | Callback de sucesso |
| `On Request Fail` | Callback de erro |

### Principais Classes (C++)

```cpp
#include "VaRestSubsystem.h"
#include "VaRestRequestJSON.h"
#include "VaRestJsonObject.h"
#include "VaRestJsonValue.h"
```

---

## 📊 ESTRUTURAS DE DADOS

### 1. Criar Structs (Blueprint)

#### FAccountData

**Content Browser** → Botão direito → **Blueprints → Structure**

**Nome**: `S_AccountData`

**Campos**:
```
- ID (Integer)
- Username (String)
- Email (String)
- IsAdmin (Boolean)
- Token (String)
```

#### FPlayerData

**Nome**: `S_PlayerData`

**Campos**:
```
- ID (Integer)
- CharacterName (String)
- Level (Integer)
- CurrentZone (String)
- PosX (Float)
- PosY (Float)
- PosZ (Float)
```

#### FLoginResponse

**Nome**: `S_LoginResponse`

**Campos**:
```
- Success (Boolean)
- Message (String)
- Account (S_AccountData)
- Players (Array of S_PlayerData)
- Token (String)
```

---

## 🔐 SISTEMA DE AUTENTICAÇÃO

### Game Instance Blueprint

**Criar Game Instance**:
1. Content Browser → Blueprint Class
2. Parent: **Game Instance**
3. Nome: `GI_UmbraEternum`

**Variables** (na Game Instance):
```
- ServerURL (String) = "http://localhost/umbra_api"
- CurrentToken (String) = ""
- CurrentAccount (S_AccountData)
- CurrentPlayers (Array of S_PlayerData)
- IsAuthenticated (Boolean) = false
```

**Configurar no Project**:
1. Edit → Project Settings
2. Maps & Modes
3. Game Instance Class = `GI_UmbraEternum`

---

## 💡 EXEMPLOS DE USO

### 1. REGISTER - Criar Conta

#### Blueprint

**Event Graph**:

```
[Event BeginPlay ou Custom Event: RegisterUser]
  ↓
[Construct Json Request]
  ↓
[Apply URL] 
  URL: ServerURL + "/api/register.php"
  ↓
[Set Verb] = POST
  ↓
[Create JSON Object]
  - Add Field: "username" = UsernameInput
  - Add Field: "email" = EmailInput
  - Add Field: "password" = PasswordInput
  ↓
[Set Content As String]
  Content: JSON Object As String
  ↓
[Process Request]
  ↓
[Bind Event: On Request Complete]
  ↓
[Parse Response]
  - Get Field: "success" (Boolean)
  - Get Field: "message" (String)
  - Get Field: "account_id" (Integer)
  ↓
[Branch: If Success]
  TRUE → Show Success Message
  FALSE → Show Error Message
```

#### Código Equivalente (C++)

```cpp
void UMyGameInstance::RegisterUser(
    const FString& Username, 
    const FString& Email, 
    const FString& Password)
{
    // Criar requisição
    UVaRestRequestJSON* Request = UVaRestRequestJSON::ConstructRequestExt(
        this, 
        EVaRestRequestVerb::POST, 
        EVaRestRequestContentType::json
    );
    
    // URL
    Request->SetURL(ServerURL + "/api/register.php");
    
    // Body JSON
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("username", Username);
    JsonObject->SetStringField("email", Email);
    JsonObject->SetStringField("password", Password);
    Request->SetRequestObject(MakeShareable(new UVaRestJsonObject(JsonObject)));
    
    // Callbacks
    Request->OnRequestComplete.AddDynamic(this, &UMyGameInstance::OnRegisterComplete);
    Request->OnRequestFail.AddDynamic(this, &UMyGameInstance::OnRegisterFail);
    
    // Enviar
    Request->ProcessRequest();
}

void UMyGameInstance::OnRegisterComplete(UVaRestRequestJSON* Request)
{
    UVaRestJsonObject* JsonObject = Request->GetResponseObject();
    
    bool bSuccess = JsonObject->GetBoolField("success");
    FString Message = JsonObject->GetStringField("message");
    
    if (bSuccess)
    {
        int32 AccountID = JsonObject->GetIntegerField("account_id");
        FString Username = JsonObject->GetStringField("username");
        
        UE_LOG(LogTemp, Log, TEXT("Registro bem-sucedido! ID: %d, Username: %s"), 
               AccountID, *Username);
        
        // Mostrar mensagem de sucesso
        OnRegistrationSuccess.Broadcast(Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Erro no registro: %s"), *Message);
        OnRegistrationFailed.Broadcast(Message);
    }
}
```

---

### 2. LOGIN - Autenticar

#### Blueprint

**Event Graph**:

```
[Custom Event: LoginUser]
  ↓
[Construct Json Request]
  ↓
[Apply URL] 
  URL: ServerURL + "/api/login.php"
  ↓
[Set Verb] = POST
  ↓
[Create JSON Object]
  - Add Field: "username" = UsernameInput
  - Add Field: "password" = PasswordInput
  ↓
[Set Content As String]
  ↓
[Process Request]
  ↓
[On Request Complete]
  ↓
[Parse Response]
  - success (Boolean)
  - message (String)
  - token (String)
  - account (Object)
    - id (Integer)
    - username (String)
    - email (String)
    - isadmin (Boolean)
  - players (Array)
  ↓
[Branch: If Success]
  TRUE →
    [Save to Game Instance]
      - CurrentToken = token
      - CurrentAccount = account
      - CurrentPlayers = players
      - IsAuthenticated = true
    [Open Main Menu Level]
  FALSE →
    [Show Error Message]
```

#### Código C++

```cpp
void UMyGameInstance::LoginUser(const FString& Username, const FString& Password)
{
    UVaRestRequestJSON* Request = UVaRestRequestJSON::ConstructRequestExt(
        this, EVaRestRequestVerb::POST, EVaRestRequestContentType::json
    );
    
    Request->SetURL(ServerURL + "/api/login.php");
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("username", Username);
    JsonObject->SetStringField("password", Password);
    Request->SetRequestObject(MakeShareable(new UVaRestJsonObject(JsonObject)));
    
    Request->OnRequestComplete.AddDynamic(this, &UMyGameInstance::OnLoginComplete);
    Request->OnRequestFail.AddDynamic(this, &UMyGameInstance::OnLoginFail);
    
    Request->ProcessRequest();
}

void UMyGameInstance::OnLoginComplete(UVaRestRequestJSON* Request)
{
    UVaRestJsonObject* ResponseObj = Request->GetResponseObject();
    
    bool bSuccess = ResponseObj->GetBoolField("success");
    
    if (bSuccess)
    {
        // Salvar token
        CurrentToken = ResponseObj->GetStringField("token");
        
        // Salvar dados da conta
        UVaRestJsonObject* AccountObj = ResponseObj->GetObjectField("account");
        CurrentAccount.ID = AccountObj->GetIntegerField("id");
        CurrentAccount.Username = AccountObj->GetStringField("username");
        CurrentAccount.Email = AccountObj->GetStringField("email");
        CurrentAccount.IsAdmin = AccountObj->GetBoolField("isadmin");
        
        // Salvar personagens
        TArray<UVaRestJsonValue*> PlayersArray = ResponseObj->GetArrayField("players");
        CurrentPlayers.Empty();
        
        for (UVaRestJsonValue* PlayerValue : PlayersArray)
        {
            UVaRestJsonObject* PlayerObj = PlayerValue->AsObject();
            
            FPlayerData PlayerData;
            PlayerData.ID = PlayerObj->GetIntegerField("id");
            PlayerData.CharacterName = PlayerObj->GetStringField("character_name");
            PlayerData.Level = PlayerObj->GetIntegerField("level");
            PlayerData.CurrentZone = PlayerObj->GetStringField("current_zone");
            
            CurrentPlayers.Add(PlayerData);
        }
        
        IsAuthenticated = true;
        
        UE_LOG(LogTemp, Log, TEXT("Login bem-sucedido! Token: %s"), *CurrentToken);
        
        // Broadcast evento de sucesso
        OnLoginSuccess.Broadcast();
    }
    else
    {
        FString Message = ResponseObj->GetStringField("message");
        UE_LOG(LogTemp, Warning, TEXT("Login falhou: %s"), *Message);
        OnLoginFailed.Broadcast(Message);
    }
}
```

---

### 3. API COM AUTENTICAÇÃO

Para APIs que requerem autenticação (como admin), adicione o token:

#### Blueprint

```
[Construct Json Request]
  ↓
[Set Header]
  Header Name: "Authorization"
  Header Value: "Bearer " + CurrentToken
  ↓
[Apply URL]
  ↓
[Process Request]
```

#### C++

```cpp
Request->SetHeader("Authorization", "Bearer " + CurrentToken);
```

---

## 🎨 BLUEPRINTS - UI

### Widget de Login

**Criar Widget Blueprint**:
1. Content Browser → User Interface → Widget Blueprint
2. Nome: `WBP_Login`

**Hierarchy**:
```
- Canvas Panel
  - Vertical Box
    - Text Block (Title): "UmbraEternum - Login"
    - Editable Text Box: UsernameInput
    - Editable Text Box: PasswordInput (IsPassword = true)
    - Button: LoginButton
      - Text: "Login"
    - Button: RegisterButton
      - Text: "Criar Conta"
    - Text Block: ErrorMessage (Visibility = Collapsed)
```

**Event Graph**:

```
[Event: OnClicked (LoginButton)]
  ↓
[Get Username from UsernameInput]
  ↓
[Get Password from PasswordInput]
  ↓
[Get Game Instance]
  ↓
[Cast to GI_UmbraEternum]
  ↓
[Call: LoginUser]
  Username = UsernameInput Text
  Password = PasswordInput Text
  ↓
[Bind to OnLoginSuccess]
  ↓
  [Remove from Parent]
  [Open Level: MainMenu]
  ↓
[Bind to OnLoginFailed]
  ↓
  [Set Text: ErrorMessage]
  [Set Visibility: Visible]
```

---

### Widget de Registro

**Nome**: `WBP_Register`

**Campos**:
- Username (Editable Text)
- Email (Editable Text)
- Password (Editable Text, IsPassword)
- Confirm Password (Editable Text, IsPassword)
- Register Button
- Back Button

**Validação**:
```
[On Register Button Clicked]
  ↓
[Branch: Password == Confirm Password]
  FALSE → Show Error "Senhas não coincidem"
  TRUE →
    [Get Game Instance]
      ↓
    [Call: RegisterUser]
      ↓
    [On Success]
      → Show Message "Conta criada!"
      → Open Login Widget
```

---

## 🔌 CONECTAR AO SERVIDOR C++

### Depois do Login, Conectar ao Game Server

#### TCP Socket Connection

**Blueprint Function Library**: `BPL_ServerConnection`

```cpp
#include "Sockets.h"
#include "SocketSubsystem.h"

bool UBPLibraryServerConnection::ConnectToGameServer(
    const FString& ServerIP, 
    int32 Port)
{
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    
    // Criar socket
    FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GameSocket"), false);
    
    // Resolver endereço
    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    Addr->SetIp(*ServerIP, bIsValid);
    Addr->SetPort(Port);
    
    // Conectar
    bool bConnected = Socket->Connect(*Addr);
    
    if (bConnected)
    {
        UE_LOG(LogTemp, Log, TEXT("Conectado ao servidor em %s:%d"), *ServerIP, Port);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Falha ao conectar"));
    return false;
}
```

**Uso**:
```
[After Login Success]
  ↓
[Connect To Game Server]
  IP: "localhost" (ou IP do servidor)
  Port: 9000 (Gateway)
  ↓
[On Connected]
  → Send Auth Token
  → Join Game
```

---

## 📋 FLUXO COMPLETO

```
┌─────────────────────────────────────────┐
│  1. Usuário abre jogo (UE5)            │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  2. Mostra tela de Login/Register       │
│     (WBP_Login)                         │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  3. Usuário preenche credenciais        │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  4. VaRest: POST /api/login.php         │
│     Body: {username, password}          │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  5. Servidor retorna:                   │
│     {success, token, account, players}  │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  6. Game Instance salva:                │
│     - Token                             │
│     - Account data                      │
│     - Player list                       │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  7. Conecta ao C++ Server               │
│     TCP Socket → Gateway (9000)         │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  8. Envia Token para validação          │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│  9. Server valida token                 │
│     Auth Server verifica JWT            │
└──────────────┬──────────────────────────┘
               │
               ↓
┌─────────────────────────────────────────┐
│ 10. Usuário entra no jogo!              │
│     - Spawn no mundo                    │
│     - Sincronização começa              │
└─────────────────────────────────────────┘
```

---

## ⚙️ BOAS PRÁTICAS

### 1. Gerenciamento de Token

**Save Game**:
```cpp
// Salvar token para auto-login
USaveGame* SaveGameInstance = UGameplayStatics::CreateSaveGameObject(USaveGameClass::StaticClass());
MySaveGame->AuthToken = CurrentToken;
UGameplayStatics::SaveGameToSlot(SaveGameInstance, "PlayerData", 0);

// Carregar token
USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot("PlayerData", 0);
if (LoadedGame)
{
    CurrentToken = MySaveGame->AuthToken;
    // Validar token com servidor
}
```

### 2. Timeout de Requisição

```cpp
Request->SetTimeout(10.0f); // 10 segundos
```

### 3. Retry Logic

```cpp
void UMyGameInstance::LoginWithRetry(int32 Attempts)
{
    if (Attempts > 3)
    {
        UE_LOG(LogTemp, Error, TEXT("Login falhou após 3 tentativas"));
        return;
    }
    
    // Tentar login
    LoginUser(Username, Password);
    
    // Se falhar, retry
    OnLoginFailed.AddLambda([this, Attempts]()
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Attempts]()
        {
            LoginWithRetry(Attempts + 1);
        }, 2.0f, false); // Wait 2 seconds
    });
}
```

### 4. Error Handling

```cpp
void UMyGameInstance::OnRequestFail(UVaRestRequestJSON* Request)
{
    int32 ResponseCode = Request->GetResponseCode();
    
    switch (ResponseCode)
    {
        case 401: // Unauthorized
            UE_LOG(LogTemp, Warning, TEXT("Não autorizado"));
            break;
        case 403: // Forbidden
            UE_LOG(LogTemp, Warning, TEXT("Acesso negado"));
            break;
        case 404: // Not Found
            UE_LOG(LogTemp, Warning, TEXT("Endpoint não encontrado"));
            break;
        case 500: // Server Error
            UE_LOG(LogTemp, Error, TEXT("Erro no servidor"));
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("Erro desconhecido: %d"), ResponseCode);
    }
    
    OnError.Broadcast(ResponseCode);
}
```

### 5. Loading Screen

```cpp
// Mostrar loading durante requisição
ShowLoadingScreen();

Request->OnRequestComplete.AddLambda([this](UVaRestRequestJSON* Req)
{
    HideLoadingScreen();
    // Process response
});
```

---

## 🔐 SEGURANÇA

### 1. NUNCA armazenar senha em plain text

```cpp
// ❌ ERRADO
FString Password = "senha123";
SaveToFile(Password);

// ✅ CORRETO
// Apenas envie para servidor, não salve localmente
```

### 2. HTTPS em Produção

```cpp
#if !UE_BUILD_SHIPPING
    ServerURL = "http://localhost/umbra_api"; // Dev
#else
    ServerURL = "https://seudominio.com/api"; // Produção
#endif
```

### 3. Validar Respostas

```cpp
if (!ResponseObj || !ResponseObj->HasField("success"))
{
    UE_LOG(LogTemp, Error, TEXT("Resposta inválida do servidor"));
    return;
}
```

---

## 📊 DEBUGGING

### Print JSON Response

```cpp
FString ResponseString = Request->GetResponseContentAsString();
UE_LOG(LogTemp, Log, TEXT("Response: %s"), *ResponseString);
```

### Ver Headers

```cpp
TMap<FString, FString> Headers = Request->GetResponseHeaders();
for (auto& Header : Headers)
{
    UE_LOG(LogTemp, Log, TEXT("Header: %s = %s"), *Header.Key, *Header.Value);
}
```

### Status Code

```cpp
int32 StatusCode = Request->GetResponseCode();
UE_LOG(LogTemp, Log, TEXT("Status Code: %d"), StatusCode);
```

---

## 📝 CHECKLIST DE INTEGRAÇÃO

- [ ] Plugin VaRest instalado
- [ ] Game Instance criada e configurada
- [ ] Structs de dados criados
- [ ] Widget de Login criado
- [ ] Widget de Register criado
- [ ] Função de Login implementada
- [ ] Função de Register implementada
- [ ] Token salvo na Game Instance
- [ ] Conexão com C++ Server implementada
- [ ] Error handling implementado
- [ ] Loading screens adicionados
- [ ] Testado em ambiente local
- [ ] Testado com servidor remoto

---

## 🎯 PRÓXIMOS PASSOS

1. ✅ Integração básica (Login/Register)
2. ⏭️ Character Selection Screen
3. ⏭️ Sincronização de personagem com servidor
4. ⏭️ Movimento replicado
5. ⏭️ Inventário sincronizado
6. ⏭️ Chat system
7. ⏭️ Matchmaking
8. ⏭️ Party system

---

## 📚 RECURSOS ADICIONAIS

### VaRest Documentation
- https://github.com/ufna/VaRest

### Unreal Networking
- https://docs.unrealengine.com/en-US/InteractiveExperiences/Networking/

### HTTP Module
- https://docs.unrealengine.com/en-US/API/Runtime/HTTP/

---

**Criado**: 2025-10-14  
**Versão**: 1.0  
**Para**: UmbraEternum Server v1.3.0  
**UE5**: 5.3+

🎮 **Pronto para criar jogos incríveis!** 🚀

