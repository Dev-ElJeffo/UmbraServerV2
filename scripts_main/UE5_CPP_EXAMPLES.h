// UmbraGameInstance.h
// Exemplo completo de Game Instance para UmbraEternum
// Cole este arquivo em: YourProject/Source/YourProject/

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VaRestRequestJSON.h"
#include "VaRestJsonObject.h"
#include "UmbraGameInstance.generated.h"

// Estruturas de Dados
USTRUCT(BlueprintType)
struct FAccountData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Account")
    int32 ID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Account")
    FString Username;

    UPROPERTY(BlueprintReadWrite, Category = "Account")
    FString Email;

    UPROPERTY(BlueprintReadWrite, Category = "Account")
    bool bIsAdmin = false;
};

USTRUCT(BlueprintType)
struct FPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 ID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CharacterName;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 Level = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CurrentZone;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FVector Position;
};

// Delegates (Eventos)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegistrationSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegistrationFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginFailed, const FString&, ErrorMessage);

/**
 * Game Instance principal do UmbraEternum
 * Gerencia autenticação e comunicação com APIs
 */
UCLASS()
class YOURPROJECT_API UUmbraGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UUmbraGameInstance();

    // ========== CONFIGURAÇÃO ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
    FString ServerURL = TEXT("http://localhost/umbra_api");

    // ========== DADOS DO USUÁRIO ==========
    
    UPROPERTY(BlueprintReadOnly, Category = "Authentication")
    FString CurrentToken;

    UPROPERTY(BlueprintReadOnly, Category = "Authentication")
    FAccountData CurrentAccount;

    UPROPERTY(BlueprintReadOnly, Category = "Authentication")
    TArray<FPlayerData> CurrentPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Authentication")
    bool bIsAuthenticated = false;

    // ========== DELEGATES ==========
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRegistrationSuccess OnRegistrationSuccess;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRegistrationFailed OnRegistrationFailed;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLoginSuccess OnLoginSuccess;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLoginFailed OnLoginFailed;

    // ========== FUNÇÕES PÚBLICAS ==========
    
    /**
     * Registrar novo usuário
     * @param Username Nome de usuário
     * @param Email Email do usuário
     * @param Password Senha
     */
    UFUNCTION(BlueprintCallable, Category = "Authentication")
    void RegisterUser(const FString& Username, const FString& Email, const FString& Password);

    /**
     * Fazer login
     * @param Username Nome de usuário
     * @param Password Senha
     */
    UFUNCTION(BlueprintCallable, Category = "Authentication")
    void LoginUser(const FString& Username, const FString& Password);

    /**
     * Fazer logout
     */
    UFUNCTION(BlueprintCallable, Category = "Authentication")
    void Logout();

    /**
     * Verificar se está autenticado
     */
    UFUNCTION(BlueprintPure, Category = "Authentication")
    bool IsAuthenticated() const { return bIsAuthenticated; }

    /**
     * Obter token atual
     */
    UFUNCTION(BlueprintPure, Category = "Authentication")
    FString GetCurrentToken() const { return CurrentToken; }

    /**
     * Salvar token para auto-login
     */
    UFUNCTION(BlueprintCallable, Category = "Authentication")
    void SaveAuthToken();

    /**
     * Carregar token salvo
     */
    UFUNCTION(BlueprintCallable, Category = "Authentication")
    void LoadAuthToken();

protected:
    // Callbacks de requisições
    UFUNCTION()
    void OnRegisterRequestComplete(UVaRestRequestJSON* Request);

    UFUNCTION()
    void OnRegisterRequestFail(UVaRestRequestJSON* Request);

    UFUNCTION()
    void OnLoginRequestComplete(UVaRestRequestJSON* Request);

    UFUNCTION()
    void OnLoginRequestFail(UVaRestRequestJSON* Request);

private:
    // Helper para criar requisição base
    UVaRestRequestJSON* CreateRequest(const FString& Endpoint, const FString& Verb = TEXT("POST"));
    
    // Helper para log de erro
    void LogError(const FString& Context, UVaRestRequestJSON* Request);
};

