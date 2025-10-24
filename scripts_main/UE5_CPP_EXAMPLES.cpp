// UmbraGameInstance.cpp
// Implementação completa da Game Instance
// Cole este arquivo em: YourProject/Source/YourProject/

#include "UmbraGameInstance.h"
#include "VaRestSubsystem.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Kismet/GameplayStatics.h"

UUmbraGameInstance::UUmbraGameInstance()
{
    // Inicialização
    bIsAuthenticated = false;
}

// ========== REGISTER USER ==========

void UUmbraGameInstance::RegisterUser(const FString& Username, const FString& Email, const FString& Password)
{
    UE_LOG(LogTemp, Log, TEXT("Registrando usuário: %s"), *Username);

    // Criar requisição
    UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/register.php"));

    // Criar JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("email"), Email);
    JsonObject->SetStringField(TEXT("password"), Password);

    // Converter para string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // Configurar requisição
    Request->SetContentAsString(OutputString);

    // Bind callbacks
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnRegisterRequestComplete);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnRegisterRequestFail);

    // Enviar
    Request->ProcessRequest();
}

void UUmbraGameInstance::OnRegisterRequestComplete(UVaRestRequestJSON* Request)
{
    UVaRestJsonObject* ResponseObject = Request->GetResponseObject();

    if (!ResponseObject)
    {
        OnRegistrationFailed.Broadcast(TEXT("Resposta inválida do servidor"));
        return;
    }

    bool bSuccess = ResponseObject->GetBoolField(TEXT("success"));
    FString Message = ResponseObject->GetStringField(TEXT("message"));

    if (bSuccess)
    {
        int32 AccountID = ResponseObject->GetIntegerField(TEXT("account_id"));
        FString RegisteredUsername = ResponseObject->GetStringField(TEXT("username"));

        UE_LOG(LogTemp, Log, TEXT("✅ Registro bem-sucedido! ID: %d, Username: %s"), 
               AccountID, *RegisteredUsername);

        OnRegistrationSuccess.Broadcast(Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Erro no registro: %s"), *Message);
        OnRegistrationFailed.Broadcast(Message);
    }
}

void UUmbraGameInstance::OnRegisterRequestFail(UVaRestRequestJSON* Request)
{
    LogError(TEXT("Register"), Request);
    OnRegistrationFailed.Broadcast(TEXT("Erro de conexão com servidor"));
}

// ========== LOGIN USER ==========

void UUmbraGameInstance::LoginUser(const FString& Username, const FString& Password)
{
    UE_LOG(LogTemp, Log, TEXT("Fazendo login: %s"), *Username);

    // Criar requisição
    UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/login.php"));

    // Criar JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("password"), Password);

    // Converter para string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // Configurar requisição
    Request->SetContentAsString(OutputString);

    // Bind callbacks
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnLoginRequestComplete);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnLoginRequestFail);

    // Enviar
    Request->ProcessRequest();
}

void UUmbraGameInstance::OnLoginRequestComplete(UVaRestRequestJSON* Request)
{
    UVaRestJsonObject* ResponseObject = Request->GetResponseObject();

    if (!ResponseObject)
    {
        OnLoginFailed.Broadcast(TEXT("Resposta inválida do servidor"));
        return;
    }

    bool bSuccess = ResponseObject->GetBoolField(TEXT("success"));
    FString Message = ResponseObject->GetStringField(TEXT("message"));

    if (bSuccess)
    {
        // Salvar token
        CurrentToken = ResponseObject->GetStringField(TEXT("token"));

        // Salvar dados da conta
        UVaRestJsonObject* AccountObject = ResponseObject->GetObjectField(TEXT("account"));
        CurrentAccount.ID = AccountObject->GetIntegerField(TEXT("id"));
        CurrentAccount.Username = AccountObject->GetStringField(TEXT("username"));
        CurrentAccount.Email = AccountObject->GetStringField(TEXT("email"));
        CurrentAccount.bIsAdmin = AccountObject->GetBoolField(TEXT("isadmin"));

        // Salvar personagens
        TArray<UVaRestJsonValue*> PlayersArray = ResponseObject->GetArrayField(TEXT("players"));
        CurrentPlayers.Empty();

        for (UVaRestJsonValue* PlayerValue : PlayersArray)
        {
            UVaRestJsonObject* PlayerObject = PlayerValue->AsObject();

            FPlayerData PlayerData;
            PlayerData.ID = PlayerObject->GetIntegerField(TEXT("id"));
            PlayerData.CharacterName = PlayerObject->GetStringField(TEXT("character_name"));
            PlayerData.Level = PlayerObject->GetIntegerField(TEXT("level"));
            PlayerData.CurrentZone = PlayerObject->GetStringField(TEXT("current_zone"));

            // Posição (se disponível)
            if (PlayerObject->HasField(TEXT("pos_x")))
            {
                float PosX = PlayerObject->GetNumberField(TEXT("pos_x"));
                float PosY = PlayerObject->GetNumberField(TEXT("pos_y"));
                float PosZ = PlayerObject->GetNumberField(TEXT("pos_z"));
                PlayerData.Position = FVector(PosX, PosY, PosZ);
            }

            CurrentPlayers.Add(PlayerData);
        }

        // Marcar como autenticado
        bIsAuthenticated = true;

        UE_LOG(LogTemp, Log, TEXT("✅ Login bem-sucedido!"));
        UE_LOG(LogTemp, Log, TEXT("   Token: %s"), *CurrentToken);
        UE_LOG(LogTemp, Log, TEXT("   Username: %s"), *CurrentAccount.Username);
        UE_LOG(LogTemp, Log, TEXT("   Email: %s"), *CurrentAccount.Email);
        UE_LOG(LogTemp, Log, TEXT("   Admin: %s"), CurrentAccount.bIsAdmin ? TEXT("Sim") : TEXT("Não"));
        UE_LOG(LogTemp, Log, TEXT("   Personagens: %d"), CurrentPlayers.Num());

        // Salvar token para auto-login
        SaveAuthToken();

        // Broadcast evento de sucesso
        OnLoginSuccess.Broadcast();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Login falhou: %s"), *Message);
        OnLoginFailed.Broadcast(Message);
    }
}

void UUmbraGameInstance::OnLoginRequestFail(UVaRestRequestJSON* Request)
{
    LogError(TEXT("Login"), Request);
    OnLoginFailed.Broadcast(TEXT("Erro de conexão com servidor"));
}

// ========== LOGOUT ==========

void UUmbraGameInstance::Logout()
{
    CurrentToken.Empty();
    CurrentAccount = FAccountData();
    CurrentPlayers.Empty();
    bIsAuthenticated = false;

    // Deletar token salvo
    if (UGameplayStatics::DoesSaveGameExist(TEXT("AuthData"), 0))
    {
        UGameplayStatics::DeleteGameInSlot(TEXT("AuthData"), 0);
    }

    UE_LOG(LogTemp, Log, TEXT("🚪 Logout realizado"));
}

// ========== SAVE/LOAD TOKEN ==========

void UUmbraGameInstance::SaveAuthToken()
{
    // TODO: Implementar SaveGame para persistir token
    // Exemplo básico:
    /*
    UAuthSaveGame* SaveGame = Cast<UAuthSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UAuthSaveGame::StaticClass())
    );
    
    if (SaveGame)
    {
        SaveGame->AuthToken = CurrentToken;
        SaveGame->Username = CurrentAccount.Username;
        UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("AuthData"), 0);
        UE_LOG(LogTemp, Log, TEXT("💾 Token salvo"));
    }
    */
}

void UUmbraGameInstance::LoadAuthToken()
{
    // TODO: Implementar LoadGame para recuperar token
    // Exemplo básico:
    /*
    UAuthSaveGame* LoadedGame = Cast<UAuthSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("AuthData"), 0)
    );
    
    if (LoadedGame && !LoadedGame->AuthToken.IsEmpty())
    {
        CurrentToken = LoadedGame->AuthToken;
        // Validar token com servidor
        UE_LOG(LogTemp, Log, TEXT("📂 Token carregado"));
    }
    */
}

// ========== HELPERS ==========

UVaRestRequestJSON* UUmbraGameInstance::CreateRequest(const FString& Endpoint, const FString& Verb)
{
    UVaRestRequestJSON* Request = UVaRestRequestJSON::ConstructRequestExt(
        this,
        EVaRestRequestVerb::POST,
        EVaRestRequestContentType::json
    );

    // URL completa
    FString FullURL = ServerURL + Endpoint;
    Request->SetURL(FullURL);

    // Headers
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Timeout
    Request->SetTimeout(10.0f);

    UE_LOG(LogTemp, Log, TEXT("🌐 Request criada: %s"), *FullURL);

    return Request;
}

void UUmbraGameInstance::LogError(const FString& Context, UVaRestRequestJSON* Request)
{
    int32 ResponseCode = Request->GetResponseCode();
    FString ResponseContent = Request->GetResponseContentAsString();

    UE_LOG(LogTemp, Error, TEXT("❌ Erro em %s:"), *Context);
    UE_LOG(LogTemp, Error, TEXT("   Status Code: %d"), ResponseCode);
    UE_LOG(LogTemp, Error, TEXT("   Response: %s"), *ResponseContent);

    // Log detalhado de erro
    switch (ResponseCode)
    {
        case 401:
            UE_LOG(LogTemp, Error, TEXT("   → Não autorizado"));
            break;
        case 403:
            UE_LOG(LogTemp, Error, TEXT("   → Acesso negado"));
            break;
        case 404:
            UE_LOG(LogTemp, Error, TEXT("   → Endpoint não encontrado"));
            break;
        case 500:
            UE_LOG(LogTemp, Error, TEXT("   → Erro interno do servidor"));
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("   → Erro desconhecido"));
    }
}

