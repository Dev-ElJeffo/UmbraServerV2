# 🎮 SISTEMA DE PERSONAGENS - GUIA COMPLETO

**Data**: 16/10/2025  
**Versão**: 1.0  
**Status**: ✅ Implementado e Funcional

---

## 📑 ÍNDICE

1. [Visão Geral](#visão-geral)
2. [APIs PHP Criadas](#apis-php-criadas)
3. [Classes C++ Atualizadas](#classes-c-atualizadas)
4. [Estrutura de Dados](#estrutura-de-dados)
5. [Fluxo de Uso](#fluxo-de-uso)
6. [Integração com UE5](#integração-com-ue5)
7. [Teste das APIs](#teste-das-apis)
8. [Próximos Passos](#próximos-passos)

---

## 🎯 VISÃO GERAL

O sistema de personagens permite que cada conta (usuário logado) tenha **até 5 personagens**.

### Funcionalidades Implementadas:

✅ **Listar Personagens** da conta logada  
✅ **Criar Novo Personagem** (nome único, validações)  
✅ **Selecionar Personagem** para jogar  
✅ **Deletar Personagem**  
✅ **Gerenciar Personagem Ativo** no `UmbraGameInstance`  

---

## 📡 APIS PHP CRIADAS

### 1. **List Characters** (Listar Personagens)

**Arquivo**: `C:\wamp64\www\umbra_api\api\character\list_characters.php`

**Método**: POST

**Request**:
```json
{
  "account_id": 1
}
```

**Response (Sucesso)**:
```json
{
  "success": true,
  "count": 2,
  "max_characters": 5,
  "players": [
    {
      "player_id": 1,
      "account_id": 1,
      "character_name": "Warrior123",
      "level": 10,
      "experience": 1500,
      "current_zone": "Forest",
      "position": {
        "x": 100.5,
        "y": 200.0,
        "z": 50.0
      },
      "created_at": "2025-10-16 12:00:00",
      "last_login": "2025-10-16 15:30:00"
    },
    {
      "player_id": 2,
      "account_id": 1,
      "character_name": "Mage456",
      "level": 5,
      "experience": 500,
      "current_zone": "Tutorial",
      "position": {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
      },
      "created_at": "2025-10-16 13:00:00",
      "last_login": null
    }
  ]
}
```

---

### 2. **Create Character** (Criar Personagem)

**Arquivo**: `C:\wamp64\www\umbra_api\api\character\create_character.php`

**Método**: POST

**Request**:
```json
{
  "account_id": 1,
  "character_name": "MyNewHero"
}
```

**Validações**:
- Nome: 3-20 caracteres
- Apenas letras, números e underscore
- Nome único (não pode existir outro personagem com mesmo nome)
- Limite de 5 personagens por conta

**Response (Sucesso)**:
```json
{
  "success": true,
  "message": "Personagem criado com sucesso!",
  "player": {
    "player_id": 3,
    "account_id": 1,
    "character_name": "MyNewHero",
    "level": 1,
    "experience": 0,
    "current_zone": "Tutorial",
    "position": {
      "x": 0.0,
      "y": 0.0,
      "z": 0.0
    },
    "created_at": "2025-10-16 16:00:00",
    "last_login": null
  }
}
```

**Response (Erro)**:
```json
{
  "success": false,
  "message": "Nome de personagem já está em uso"
}
```

---

### 3. **Select Character** (Selecionar Personagem)

**Arquivo**: `C:\wamp64\www\umbra_api\api\character\select_character.php`

**Método**: POST

**Request**:
```json
{
  "player_id": 1,
  "account_id": 1
}
```

**Funcionalidade**:
- Valida que o personagem pertence à conta
- Atualiza `last_login` para `NOW()`
- Retorna dados completos do personagem

**Response (Sucesso)**:
```json
{
  "success": true,
  "message": "Personagem selecionado com sucesso!",
  "player": {
    "player_id": 1,
    "account_id": 1,
    "character_name": "Warrior123",
    "level": 10,
    "experience": 1500,
    "current_zone": "Forest",
    "position": {
      "x": 100.5,
      "y": 200.0,
      "z": 50.0
    },
    "created_at": "2025-10-16 12:00:00",
    "last_login": "2025-10-16 16:05:00"
  }
}
```

---

### 4. **Delete Character** (Deletar Personagem)

**Arquivo**: `C:\wamp64\www\umbra_api\api\character\delete_character.php`

**Método**: POST

**Request**:
```json
{
  "player_id": 2,
  "account_id": 1
}
```

**Response (Sucesso)**:
```json
{
  "success": true,
  "message": "Personagem 'Mage456' deletado com sucesso!"
}
```

---

## 🔧 CLASSES C++ ATUALIZADAS

### `UmbraGameInstance.h`

**Delegates Adicionados**:
```cpp
// Carregar lista
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterListLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterListFailed, const FString&, ErrorMessage);

// Criar personagem
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterCreated, const FUmbraPlayerData&, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterCreateFailed, const FString&, ErrorMessage);

// Selecionar personagem
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, const FUmbraPlayerData&, SelectedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelectionFailed, const FString&, ErrorMessage);

// Deletar personagem
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeleted, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeleteFailed, const FString&, ErrorMessage);
```

**Métodos Públicos Adicionados**:
```cpp
// Carregar lista de personagens da conta atual
UFUNCTION(BlueprintCallable, Category = "Character")
void LoadCharacterList();

// Criar novo personagem
UFUNCTION(BlueprintCallable, Category = "Character")
void CreateCharacter(const FString& CharacterName);

// Selecionar personagem para jogar
UFUNCTION(BlueprintCallable, Category = "Character")
void SelectCharacter(int32 PlayerID);

// Deletar personagem
UFUNCTION(BlueprintCallable, Category = "Character")
void DeleteCharacter(int32 PlayerID);

// Obter lista de personagens
UFUNCTION(BlueprintPure, Category = "Character")
TArray<FUmbraPlayerData> GetCharacterList() const;

// Obter personagem ativo
UFUNCTION(BlueprintPure, Category = "Character")
FUmbraPlayerData GetActiveCharacter() const;

// Verificar se há personagem ativo
UFUNCTION(BlueprintPure, Category = "Character")
bool HasActiveCharacter() const;
```

---

## 📦 ESTRUTURA DE DADOS

### `FUmbraPlayerData` (já existente)

Definido em `UmbraDataStructures.h`:

```cpp
USTRUCT(BlueprintType)
struct FUmbraPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    int32 ID = 0;

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

---

## 🔄 FLUXO DE USO

### 1. **Login → Lista de Personagens**

```
[Login bem-sucedido]
    ↓
[OnLoginSuccess disparado]
    ↓
[Chamar LoadCharacterList()]
    ↓
[OnCharacterListLoaded disparado]
    ↓
[Mostrar Widget de Seleção de Personagem]
```

---

### 2. **Criar Novo Personagem**

```
[Usuário digita nome]
    ↓
[Validar nome (3-20 chars)]
    ↓
[Chamar CreateCharacter(nome)]
    ↓
[OnCharacterCreated disparado]
    ↓
[Adiciona personagem à CurrentPlayers]
    ↓
[Atualizar UI]
```

---

### 3. **Selecionar Personagem e Entrar no Jogo**

```
[Usuário clica em personagem]
    ↓
[Chamar SelectCharacter(player_id)]
    ↓
[OnCharacterSelected disparado]
    ↓
[ActivePlayerID setado]
    ↓
[Carregar Level do Jogo]
    ↓
[Spawnar personagem com dados]
```

---

### 4. **Deletar Personagem**

```
[Usuário confirma exclusão]
    ↓
[Chamar DeleteCharacter(player_id)]
    ↓
[OnCharacterDeleted disparado]
    ↓
[LoadCharacterList() automático]
    ↓
[OnCharacterListLoaded disparado]
    ↓
[UI atualizada]
```

---

## 🎮 INTEGRAÇÃO COM UE5

### Widget: WBP_CharacterSelection (A CRIAR)

**Layout Sugerido**:
```
╔═══════════════════════════════════════╗
║  SELECIONE SEU PERSONAGEM             ║
╠═══════════════════════════════════════╣
║                                       ║
║  ┌─────────────────────────┐          ║
║  │ Warrior123              │          ║
║  │ Level: 10               │  [Play]  ║
║  │ Zone: Forest            │  [Del]   ║
║  └─────────────────────────┘          ║
║                                       ║
║  ┌─────────────────────────┐          ║
║  │ Mage456                 │          ║
║  │ Level: 5                │  [Play]  ║
║  │ Zone: Tutorial          │  [Del]   ║
║  └─────────────────────────┘          ║
║                                       ║
║  [ + CRIAR NOVO PERSONAGEM ]          ║
║                                       ║
║  [Logout]                             ║
╚═══════════════════════════════════════╝
```

---

### Blueprint Event Graph - WBP_CharacterSelection

#### Event Construct:
```
[Event Construct]
    ↓
[Get Umbra Game Instance]
    ↓
[Bind OnCharacterListLoaded]
    ↓ (evento)
    [PopulateCharacterList] (custom function)
    ↓
    [For Each Player in CurrentPlayers]
        ↓
        [Create Widget: WBP_CharacterItem]
        ↓
        [Set Character Data]
        ↓
        [Add to Vertical Box]
    ↓
[Bind OnCharacterCreated]
    ↓ (evento)
    [PopulateCharacterList] (atualiza lista)
    ↓
[Bind OnCharacterSelected]
    ↓ (evento)
    [Open Level: GameMap]
    ↓
[Load Character List]
```

---

#### Botão "Criar Personagem":
```
[BTN_CreateCharacter] OnClicked
    ↓
[Get Text] TXT_NewCharacterName
    ↓
[Is Empty?] Branch
    ↓ False
[Get Umbra Game Instance]
    ↓
[Create Character]
    • Character Name: (text)
```

---

#### Item de Personagem - Botão "Play":
```
[BTN_Play] OnClicked
    ↓
[Get Umbra Game Instance]
    ↓
[Select Character]
    • Player ID: (this character's ID)
```

---

#### Item de Personagem - Botão "Delete":
```
[BTN_Delete] OnClicked
    ↓
[Show Confirmation Dialog]
    ↓ Confirmed
[Get Umbra Game Instance]
    ↓
[Delete Character]
    • Player ID: (this character's ID)
```

---

## 🧪 TESTE DAS APIS

### Teste Via cURL (PowerShell):

#### 1. Listar Personagens:
```powershell
$body = @{
    account_id = 1
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/character/list_characters.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"
```

---

#### 2. Criar Personagem:
```powershell
$body = @{
    account_id = 1
    character_name = "TestHero"
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/character/create_character.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"
```

---

#### 3. Selecionar Personagem:
```powershell
$body = @{
    player_id = 1
    account_id = 1
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/character/select_character.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"
```

---

#### 4. Deletar Personagem:
```powershell
$body = @{
    player_id = 2
    account_id = 1
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/character/delete_character.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"
```

---

### Teste Via Web (HTML)

Um arquivo `test_character.html` foi criado em:
```
C:\wamp64\www\umbra_api\test_character.html
```

Acesse: **http://localhost/umbra_api/test_character.html**

---

## 🚀 PRÓXIMOS PASSOS

### Fase 1: Widgets de Personagem ✅ (Você está aqui!)
- [x] APIs PHP criadas
- [x] Classes C++ atualizadas
- [ ] **WBP_CharacterSelection** (a criar)
- [ ] **WBP_CharacterItem** (widget individual de personagem)
- [ ] **WBP_CreateCharacter** (diálogo de criação)

---

### Fase 2: Integração com Gameplay
- [ ] Carregar personagem ao entrar no nível
- [ ] Spawnar personagem com stats corretos
- [ ] Salvar posição/progresso do personagem
- [ ] Atualizar experiência/level

---

### Fase 3: Recursos Avançados
- [ ] Customização visual do personagem (skins, etc.)
- [ ] Inventário por personagem
- [ ] Skills/habilidades por personagem
- [ ] Sistema de equipamentos

---

## 📊 DIAGRAMA DO FLUXO COMPLETO

```
┌─────────────┐
│   LOGIN     │
│  (Success)  │
└──────┬──────┘
       ↓
┌──────────────────────┐
│ Load Character List  │
│ (API Call)           │
└──────┬───────────────┘
       ↓
┌──────────────────────────────┐
│ WBP_CharacterSelection       │
│                              │
│  [Character 1] [Play] [Del]  │
│  [Character 2] [Play] [Del]  │
│                              │
│  [ + Create New ]            │
└──────┬───────────────────────┘
       │
       ├─→ [Create] → API → OnCharacterCreated → Update List
       │
       ├─→ [Play] → Select Character API → OnCharacterSelected → Load Game Level
       │
       └─→ [Delete] → API → OnCharacterDeleted → Reload List
```

---

## ✅ CHECKLIST DE IMPLEMENTAÇÃO

### Backend (APIs PHP):
- [x] `list_characters.php`
- [x] `create_character.php`
- [x] `select_character.php`
- [x] `delete_character.php`
- [x] Validações de nome (3-20 chars, alfanumérico)
- [x] Limite de 5 personagens por conta
- [x] Verificação de ownership (personagem pertence à conta)

### C++ (UmbraGameInstance):
- [x] Delegates de personagem
- [x] Métodos: `LoadCharacterList()`
- [x] Métodos: `CreateCharacter()`
- [x] Métodos: `SelectCharacter()`
- [x] Métodos: `DeleteCharacter()`
- [x] Métodos: `GetActiveCharacter()`
- [x] Métodos: `HasActiveCharacter()`
- [x] Callbacks HTTP
- [x] Parsing de respostas JSON
- [x] Gerenciamento de `ActivePlayerID`

### UE5 Widgets (A FAZER):
- [ ] `WBP_CharacterSelection` (lista de personagens)
- [ ] `WBP_CharacterItem` (widget individual)
- [ ] `WBP_CreateCharacter` (diálogo de criação)
- [ ] Bind de eventos
- [ ] Atualização dinâmica de UI

---

## 📝 NOTAS IMPORTANTES

1. **Autenticação Obrigatória**: Todos os métodos de personagem verificam `bIsAuthenticated`
2. **Account ID**: Sempre passa `CurrentAccount.ID` nas APIs
3. **Ownership**: APIs validam que o personagem pertence à conta antes de qualquer operação
4. **Recarregar Lista**: Após deletar, `LoadCharacterList()` é chamado automaticamente
5. **Active Player**: Use `GetActiveCharacter()` para obter dados do personagem em jogo

---

## 🎯 RESULTADO ESPERADO

Após completar os widgets:

1. ✅ Usuário faz login
2. ✅ Vê lista de seus personagens
3. ✅ Pode criar novo personagem (se < 5)
4. ✅ Pode selecionar e jogar com um personagem
5. ✅ Pode deletar personagem (com confirmação)
6. ✅ Sistema sincroniza com banco de dados MySQL
7. ✅ Dados do personagem disponíveis no C++ durante gameplay

---

## 📚 REFERÊNCIAS

- **APIs PHP**: `C:\wamp64\www\umbra_api\api\character\`
- **C++ Header**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Core\UmbraGameInstance.h`
- **C++ Source**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Core\UmbraGameInstance.cpp`
- **Data Structures**: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Data\UmbraDataStructures.h`

---

**🚀 Sistema pronto para uso! Compile o projeto UE5 e crie os widgets!**

