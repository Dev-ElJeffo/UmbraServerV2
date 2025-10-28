# Análise: Problema de Seleção de Personagem - 27 de Outubro de 2025

## 📋 Resumo do Problema

**Erro**: Ao selecionar um personagem recém-criado, o sistema envia `player_id: 0` em vez do ID correto, resultando em erro:

```json
{"success":false,"message":"player_id e account_id são obrigatórios"}
```

**Status**: Causa identificada

---

## 🔍 Análise dos Logs

### Logs do Erro

```
[2025.10.27-23.50.33:097][615]LogBlueprintUserMessages: [WBP_CharacterItem_C_3] 0
[2025.10.27-23.50.33:097][615]LogTemp: [UmbraGameInstance] Selecionando personagem ID: 0
```

**Padrão Observado**:
- Primeira conta (funcionando): `WBP_CharacterItem_C_0` → ID: 1
- Segunda conta (funcionando): Múltiplos personagens com IDs corretos (4, 5, 2)
- Terceira conta (erro): `WBP_CharacterItem_C_3` → ID: 0 ❌
- Quarta conta (erro): `WBP_CharacterItem_C_4` → ID: 0 ❌

### Tentativas de Seleção

Foram feitas **3 tentativas** para o mesmo personagem, todas com ID 0:

```
[2025.10.27-23.50.33:097] - Primeira tentativa: ID 0
[2025.10.27-23.50.41:948] - Segunda tentativa: ID 0
[2025.10.27-23.51.01:031] - Terceira tentativa: ID 0
```

---

## 🕵️ Investigação

### 1. Código C++ Verificado

**Função**: `UUmbraGameInstance::SelectCharacter(int32 PlayerID)`

```cpp
void UUmbraGameInstance::SelectCharacter(int32 PlayerID)
{
    if (!bIsAuthenticated)
    {
        OnCharacterSelectionFailed.Broadcast(TEXT("Você precisa estar autenticado"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] Selecionando personagem ID: %d"), PlayerID);

    UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/character/select_character.php"));
    if (!Request)
    {
        OnCharacterSelectionFailed.Broadcast(TEXT("Erro ao criar requisição"));
        return;
    }

    UVaRestSubsystem* VaRest = GEngine->GetEngineSubsystem<UVaRestSubsystem>();
    if (!VaRest)
    {
        OnCharacterSelectionFailed.Broadcast(TEXT("VaRest Subsystem não encontrado"));
        return;
    }

    UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
    JsonObject->SetNumberField(TEXT("player_id"), PlayerID);
    JsonObject->SetNumberField(TEXT("account_id"), CurrentAccount.ID);

    Request->SetRequestObject(JsonObject);
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnSelectCharacterRequestComplete);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnSelectCharacterRequestFail);

    Request->ExecuteProcessRequest();
}
```

**Resultado**: ✅ Código C++ está correto

### 2. Estrutura de Dados

**Estrutura**: `FUmbraPlayerData`

```cpp
USTRUCT(BlueprintType)
struct FUmbraPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 ID = 0;  // ← Este é o campo que contém o ID

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CharacterName;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 Level = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 Experience = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString CurrentZone;

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FVector Position = FVector::ZeroVector;
};
```

**Resultado**: ✅ Estrutura está correta

### 3. Parse de Resposta do Servidor

**Inconsistência Identificada**:

No arquivo `UmbraGameInstance.cpp`, há **duas implementações diferentes** de parse de personagens:

#### Implementação 1 (Linha ~417) - Resposta de Login
```cpp
PlayerData.ID = PlayerObject->GetIntegerField(TEXT("id"));
```

#### Implementação 2 (Linha ~530) - Resposta de LoadCharacterList
```cpp
PlayerData.ID = PlayerObject->GetIntegerField(TEXT("player_id"));
```

**⚠️ PROBLEMA**: Inconsistência nos nomes de campos JSON

---

## 🎯 Causa Raiz Identificada

### Problema Principal

**O ID do personagem não está sendo definido corretamente no Widget `WBP_CharacterItem`**.

Evidências:
1. ✅ Código C++ está correto
2. ✅ Estrutura de dados está correta
3. ✅ APIs do servidor estão funcionando
4. ❌ O Blueprint `WBP_CharacterItem` está recebendo/passando ID incorreto

### Possíveis Causas no Blueprint

1. **Variável não está sendo definida** quando o Widget é criado
2. **Binding de dados incorreto** no evento `PopulateCharacterList`
3. **Estrutura de dados não está sendo passada corretamente** para o Widget
4. **Índice do array em vez do ID** está sendo usado acidentalmente

---

## 🔧 Análise dos Logs de Criação

Procure nos logs por:
- "Novo personagem criado"
- "Character created"
- Response do servidor com o ID do personagem criado

Isso ajudará a confirmar se o servidor está retornando o ID corretamente.

---

## 📝 Checklist de Verificação

### Para Debugging no Blueprint

#### 1. Verificar se o Widget recebe os dados corretos

No Blueprint `WBP_CharacterItem`:

```cpp
// Adicionar log no evento "OnInitialized" ou "Construct"
UFUNCTION(BlueprintImplementableEvent)
void OnInitialized();

// No evento, adicionar:
void UMyCharacterItemWidget::OnInitialized()
{
    UE_LOG(LogTemp, Log, TEXT("[WBP_CharacterItem] Initialized with PlayerID: %d"), PlayerData.ID);
}
```

#### 2. Verificar o evento de clique do botão

No evento "OnCharacterClicked" do Blueprint:

```cpp
// Adicionar log antes de chamar SelectCharacter:
UE_LOG(LogTemp, Log, TEXT("[WBP_CharacterItem] Clicked! PlayerID: %d"), PlayerData.ID);
```

#### 3. Verificar o método "SetCharacterData"

```cpp
UFUNCTION(BlueprintCallable)
void SetCharacterData(const FUmbraPlayerData& Data)
{
    PlayerData = Data;
    UE_LOG(LogTemp, Log, TEXT("[WBP_CharacterItem] SetCharacterData - ID: %d, Name: %s"), 
           Data.ID, *Data.CharacterName);
}
```

---

## 🎨 Solução Proposta

### Passo 1: Adicionar Logs Debug no Blueprint

Adicionar variáveis de debug no `WBP_CharacterItem`:

1. No evento **OnConstruct** do Blueprint:
   - Print String: `"OnConstruct - PlayerID: " + (string)PlayerData.ID`

2. No evento **OnClicked**:
   - Print String: `"OnClicked - PlayerID: " + (string)PlayerData.ID`
   - Print String: `"Current Players Count: " + (string)CurrentPlayers.Num()`

### Passo 2: Verificar Passagem de Dados

No Blueprint `WBP_CharacterSelection`:

Quando populando a lista (função `PopulateCharacterList`):

```cpp
// Para cada personagem:
FUmbraPlayerData CurrentPlayer = CurrentPlayers[Index];
UE_LOG(LogTemp, Log, TEXT("[PopulateCharacterList] Index: %d, PlayerID: %d, Name: %s"), 
       Index, CurrentPlayer.ID, *CurrentPlayer.CharacterName);

// Criar Widget
UMyCharacterItemWidget* ItemWidget = CreateWidget<UMyCharacterItemWidget>(...);
ItemWidget->SetCharacterData(CurrentPlayer);  // ← Verificar se está passando corretamente
```

### Passo 3: Verificar Tipos de Variáveis no Blueprint

No Widget `WBP_CharacterItem`, verificar:
- ✅ Variável `PlayerData` está tipada como `FUmbraPlayerData`
- ✅ Variável `PlayerData.ID` é do tipo `int32` (não `float` ou `string`)
- ✅ O evento `OnClicked` está passando `PlayerData.ID` (não um índice)

---

## 🚨 Debug Imediato - Adicionar ao Código

Adicionar logs na função `SelectCharacter` para ver o valor real recebido:

```cpp
void UUmbraGameInstance::SelectCharacter(int32 PlayerID)
{
    // ✨ ADICIONAR ESTE LOG
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] SelectCharacter CALLED"));
    UE_LOG(LogTemp, Warning, TEXT("  Received PlayerID: %d"), PlayerID);
    UE_LOG(LogTemp, Warning, TEXT("  CurrentAccount.ID: %d"), CurrentAccount.ID);
    UE_LOG(LogTemp, Warning, TEXT("  Total Characters: %d"), CurrentPlayers.Num());
    
    // Imprimir todos os personagens para debug
    for (const auto& Player : CurrentPlayers)
    {
        UE_LOG(LogTemp, Warning, TEXT("    - Player ID: %d, Name: %s"), 
               Player.ID, *Player.CharacterName);
    }
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    
    // ... resto do código existente
}
```

---

## 📊 Situação Atual

| Item | Status | Observações |
|------|--------|-------------|
| Código C++ `SelectCharacter` | ✅ OK | Implementação correta |
| Estrutura `FUmbraPlayerData` | ✅ OK | Estrutura correta |
| API Server | ✅ OK | Retornando dados corretos |
| Widget `WBP_CharacterItem` | ❌ PROBLEMA | Passando ID incorreto (0) |
| Bindings Blueprint | ❓ PENDENTE | Precisa verificar |

---

## 🎯 Próximos Passos

1. **Imediato**: Adicionar logs de debug no C++ (`SelectCharacter`)
2. **Verificar**: Blueprint `WBP_CharacterItem` - evento `OnClicked`
3. **Verificar**: Como o Widget recebe `FUmbraPlayerData`
4. **Corrigir**: Binding de dados no evento de clique
5. **Testar**: Criar novo personagem e verificar logs

---

## 💡 Hipótese Final

**Problema**: O Blueprint `WBP_CharacterItem` está:
- Usando `0` (valor padrão) em vez do ID real
- Não está recebendo `FUmbraPlayerData` corretamente
- Ou está usando índice do array em vez do ID

**Solução**: Adicionar logs no Blueprint e C++ para confirmar onde a informação está sendo perdida.

---

**Documento criado em**: 27 de Outubro de 2025  
**Última atualização**: 27 de Outubro de 2025

