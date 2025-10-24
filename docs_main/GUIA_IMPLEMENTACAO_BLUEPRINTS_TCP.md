# 🎮 Guia Completo de Implementação de Blueprints com Validação TCP

## 📋 Visão Geral

Este guia detalha a implementação completa das Blueprints para **Login**, **Registro**, **Character Selection** e **Character Item** utilizando o protocolo TCP com validação nos servidores UmbraEternum.

## 🏗️ Arquitetura do Sistema

### 🔄 Fluxo de Autenticação TCP

```
UE5 Client → Gateway Server (9000) → Auth Server (8080) via TCP
     ↓              ↓                        ↓
  Blueprint    Validação TCP           Validação JWT
     ↓              ↓                        ↓
  Response    Cache de Sessões         Resposta TCP
```

### 📊 Componentes Principais

| Componente | Função | Porta | Protocolo |
|------------|--------|-------|-----------|
| **UE5 Client** | Interface Blueprint | - | TCP → Gateway |
| **Gateway Server** | Validação TCP | 9000 | TCP |
| **Auth Server** | Validação JWT | 8080 | TCP |
| **World Server** | Servidor de Jogo | 7000 | TCP |

## 🎯 Implementação das Blueprints

### 1. 🔐 Blueprint de Login (WBP_Login)

#### **Estrutura da Blueprint:**

```blueprint
WBP_Login
├── 📱 UI Components
│   ├── TextBox_Username (Input)
│   ├── TextBox_Password (Input)
│   ├── Button_Login (Action)
│   ├── Button_Register (Action)
│   ├── Text_Status (Feedback)
│   └── ProgressBar_Loading (Loading)
│
├── 🔗 Event Bindings
│   ├── OnLoginSuccess → Navigate to Character Selection
│   ├── OnLoginFailed → Show Error Message
│   ├── OnTCPConnectionSuccess → Validate Token
│   └── OnTCPConnectionFailed → Fallback to HTTP
│
└── 📡 TCP Integration
    ├── ConnectToGatewayTCP()
    ├── ValidateCurrentTokenViaTCP()
    └── HandleTCPResponse()
```

#### **Implementação Step-by-Step:**

**1. Configurar Event Graph:**

```blueprint
Event BeginPlay
├── Get Game Instance (UmbraGameInstance)
├── Bind Events:
│   ├── OnLoginSuccess → HandleLoginSuccess
│   ├── OnLoginFailed → HandleLoginFailed
│   ├── OnTCPConnectionSuccess → HandleTCPSuccess
│   └── OnTCPConnectionFailed → HandleTCPFailed
└── Load Saved Token (Auto-login)
```

**2. Implementar Login Button:**

```blueprint
Button_Login OnClicked
├── Validate Input Fields
├── Show Loading State
├── Call LoginUser(Username, Password)
└── Start TCP Connection Timer (1s delay)
```

**3. Implementar TCP Validation:**

```blueprint
HandleTCPSuccess
├── Call ValidateCurrentTokenViaTCP()
├── Wait for Token Validation
├── On Success → Navigate to Character Selection
└── On Failure → Show Error Message
```

**4. Implementar Fallback HTTP:**

```blueprint
HandleTCPFailed
├── Log TCP Error
├── Continue with HTTP Authentication
├── On Success → Navigate to Character Selection
└── On Failure → Show Error Message
```

#### **Código C++ Integration:**

```cpp
// No UmbraGameInstance.cpp
void UUmbraGameInstance::OnLoginRequestComplete(UVaRestRequestJSON* Request)
{
    // ... existing code ...
    
    if (Response.bSuccess)
    {
        // ... save data ...
        
        // Conectar TCP após login bem-sucedido
        if (bUseTCPConnection && TCPClient)
        {
            UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🔌 Conectando ao Gateway Server via TCP"));
            
            // Delay de 1 segundo antes de conectar TCP
            FTimerHandle TCPConnectionTimer;
            GetWorld()->GetTimerManager().SetTimer(TCPConnectionTimer, [this]()
            {
                ConnectToGatewayTCP();
            }, 1.0f, false);
        }
        else
        {
            // Se não usar TCP, broadcast imediatamente
            OnLoginSuccess.Broadcast();
        }
    }
}
```

### 2. 📝 Blueprint de Registro (WBP_Register)

#### **Estrutura da Blueprint:**

```blueprint
WBP_Register
├── 📱 UI Components
│   ├── TextBox_Username (Input)
│   ├── TextBox_Email (Input)
│   ├── TextBox_Password (Input)
│   ├── TextBox_ConfirmPassword (Input)
│   ├── Button_Register (Action)
│   ├── Button_Back (Action)
│   ├── Text_Status (Feedback)
│   └── ProgressBar_Loading (Loading)
│
├── 🔗 Event Bindings
│   ├── OnRegistrationSuccess → Navigate to Login
│   ├── OnRegistrationFailed → Show Error Message
│   └── OnValidationError → Show Field Error
│
└── ✅ Validation Rules
    ├── Username: 3-20 caracteres, único
    ├── Email: formato válido, único
    ├── Password: mínimo 6 caracteres
    └── Confirm Password: deve coincidir
```

#### **Implementação Step-by-Step:**

**1. Configurar Event Graph:**

```blueprint
Event BeginPlay
├── Get Game Instance (UmbraGameInstance)
├── Bind Events:
│   ├── OnRegistrationSuccess → HandleRegistrationSuccess
│   └── OnRegistrationFailed → HandleRegistrationFailed
└── Setup Input Validation
```

**2. Implementar Validação de Campos:**

```blueprint
ValidateInputFields
├── Check Username Length (3-20)
├── Check Email Format
├── Check Password Length (6+)
├── Check Password Match
├── On Valid → Enable Register Button
└── On Invalid → Show Error Messages
```

**3. Implementar Register Button:**

```blueprint
Button_Register OnClicked
├── Validate All Fields
├── Show Loading State
├── Call RegisterUser(Username, Email, Password)
└── Wait for Response
```

**4. Implementar Response Handling:**

```blueprint
HandleRegistrationSuccess
├── Show Success Message
├── Clear Form Fields
├── Navigate to Login Screen
└── Log Registration Success
```

### 3. 👤 Blueprint de Character Selection (WBP_CharacterSelection)

#### **Estrutura da Blueprint:**

```blueprint
WBP_CharacterSelection
├── 📱 UI Components
│   ├── ScrollBox_CharacterList (Container)
│   ├── Button_CreateCharacter (Action)
│   ├── Button_DeleteCharacter (Action)
│   ├── Button_SelectCharacter (Action)
│   ├── Button_Logout (Action)
│   ├── Text_CharacterCount (Info)
│   └── ProgressBar_Loading (Loading)
│
├── 🔗 Event Bindings
│   ├── OnCharacterListLoaded → PopulateCharacterList
│   ├── OnCharacterListFailed → Show Error Message
│   ├── OnCharacterCreated → Refresh Character List
│   ├── OnCharacterDeleted → Refresh Character List
│   └── OnCharacterSelected → Navigate to Game
│
└── 🔄 TCP Validation
    ├── ValidateCurrentTokenViaTCP()
    ├── On Success → Load Character List
    └── On Failure → Redirect to Login
```

#### **Implementação Step-by-Step:**

**1. Configurar Event Graph:**

```blueprint
Event BeginPlay
├── Get Game Instance (UmbraGameInstance)
├── Bind Events:
│   ├── OnCharacterListLoaded → HandleCharacterListLoaded
│   ├── OnCharacterListFailed → HandleCharacterListFailed
│   ├── OnCharacterCreated → HandleCharacterCreated
│   ├── OnCharacterDeleted → HandleCharacterDeleted
│   └── OnCharacterSelected → HandleCharacterSelected
├── Validate TCP Connection
└── Load Character List
```

**2. Implementar TCP Validation:**

```blueprint
ValidateTCPConnection
├── Check if TCP Client is Connected
├── If Connected → Validate Current Token
├── If Valid → Load Character List
├── If Invalid → Redirect to Login
└── If Not Connected → Try Reconnect
```

**3. Implementar Character List Loading:**

```blueprint
LoadCharacterList
├── Show Loading State
├── Call LoadCharacterList() from Game Instance
├── Wait for Response
└── Handle Response
```

**4. Implementar Character List Population:**

```blueprint
HandleCharacterListLoaded
├── Clear Existing Character Items
├── For Each Character in List:
│   ├── Create Character Item Widget
│   ├── Set Character Data
│   ├── Bind Selection Events
│   └── Add to ScrollBox
├── Update Character Count
└── Hide Loading State
```

**5. Implementar Character Actions:**

```blueprint
Button_CreateCharacter OnClicked
├── Open Create Character Dialog
├── Validate Character Name
├── Call CreateCharacter(CharacterName)
└── Wait for Response

Button_SelectCharacter OnClicked
├── Get Selected Character ID
├── Validate Selection
├── Call SelectCharacter(PlayerID)
└── Wait for Response

Button_DeleteCharacter OnClicked
├── Show Confirmation Dialog
├── Get Character ID to Delete
├── Call DeleteCharacter(PlayerID)
└── Wait for Response
```

### 4. 🎭 Blueprint de Character Item (WBP_CharacterItem)

#### **Estrutura da Blueprint:**

```blueprint
WBP_CharacterItem
├── 📱 UI Components
│   ├── Image_CharacterAvatar (Visual)
│   ├── Text_CharacterName (Info)
│   ├── Text_CharacterLevel (Info)
│   ├── Text_CharacterZone (Info)
│   ├── Text_LastLogin (Info)
│   ├── Button_Select (Action)
│   ├── Button_Delete (Action)
│   └── Border_Selection (Visual)
│
├── 🔗 Event Bindings
│   ├── OnCharacterSelected → Highlight Selection
│   ├── OnCharacterDeleted → Remove from List
│   └── OnCharacterUpdated → Update Display
│
└── 📊 Character Data
    ├── PlayerID (int32)
    ├── CharacterName (FString)
    ├── Level (int32)
    ├── CurrentZone (FString)
    ├── LastLogin (FString)
    └── Stats (FUmbraPlayerStats)
```

#### **Implementação Step-by-Step:**

**1. Configurar Event Graph:**

```blueprint
Event BeginPlay
├── Get Game Instance (UmbraGameInstance)
├── Bind Events:
│   ├── OnCharacterSelected → HandleCharacterSelected
│   ├── OnCharacterDeleted → HandleCharacterDeleted
│   └── OnCharacterUpdated → HandleCharacterUpdated
└── Setup Initial State
```

**2. Implementar Data Binding:**

```blueprint
SetCharacterData
├── Set Character Name
├── Set Character Level
├── Set Current Zone
├── Set Last Login
├── Set Character Avatar
└── Update Display
```

**3. Implementar Selection Logic:**

```blueprint
Button_Select OnClicked
├── Get Character ID
├── Validate Selection
├── Call SelectCharacter(PlayerID) from Game Instance
├── Show Selection Feedback
└── Wait for Response
```

**4. Implementar Delete Logic:**

```blueprint
Button_Delete OnClicked
├── Show Confirmation Dialog
├── Get Character ID
├── Call DeleteCharacter(PlayerID) from Game Instance
├── Show Delete Feedback
└── Wait for Response
```

**5. Implementar Visual Feedback:**

```blueprint
HandleCharacterSelected
├── Highlight Selected Character
├── Dim Other Characters
├── Show Selection Border
└── Update Button States
```

## 🔧 Configuração TCP

### 📁 Arquivo de Configuração (`config/server.json`)

```json
{
  "gateway": {
    "port": 9000,
    "auth_host": "localhost",
    "auth_port": 8080,
    "auth_timeout_ms": 5000,
    "use_connection_pool": true,
    "max_connections_per_host": 3,
    "health_check_interval_ms": 30000
  },
  "auth": {
    "jwt_secret": "umbra_secret_key_2025",
    "token_expiration_minutes": 60,
    "refresh_token_expiration_days": 7
  }
}
```

### 🔄 Protocolo de Comunicação TCP

#### **Mensagem de Validação:**

```json
{
  "action": "validate_token",
  "data": {
    "token": "jwt_token_string",
    "account_id": 12345
  },
  "timestamp": 1640995200000
}
```

#### **Resposta do Servidor:**

```json
{
  "success": true,
  "valid": true,
  "account_id": 12345,
  "player_id": 67890,
  "username": "player_name",
  "message": "Token válido"
}
```

## 🚀 Implementação Prática

### 1. **Preparação do Ambiente:**

```bash
# 1. Compilar projeto UE5
cd UmbraEternumUE
./scripts/compile.bat

# 2. Iniciar servidores
cd UmbraServer
./scripts/run_server.bat

# 3. Verificar conexões TCP
netstat -an | findstr :9000
netstat -an | findstr :8080
```

### 2. **Implementação das Blueprints:**

**Passo 1: Criar Blueprint de Login**
1. Abrir UE5 Editor
2. Criar Widget Blueprint `WBP_Login`
3. Adicionar componentes UI
4. Configurar Event Graph
5. Implementar TCP integration

**Passo 2: Criar Blueprint de Registro**
1. Criar Widget Blueprint `WBP_Register`
2. Adicionar campos de input
3. Implementar validação
4. Configurar eventos

**Passo 3: Criar Blueprint de Character Selection**
1. Criar Widget Blueprint `WBP_CharacterSelection`
2. Adicionar ScrollBox para lista
3. Implementar carregamento de personagens
4. Configurar ações de personagem

**Passo 4: Criar Blueprint de Character Item**
1. Criar Widget Blueprint `WBP_CharacterItem`
2. Adicionar componentes de exibição
3. Implementar ações de seleção/deleção
4. Configurar feedback visual

### 3. **Teste e Validação:**

```bash
# 1. Testar conexão TCP
python tests/test_tcp_integration.py

# 2. Testar múltiplas conexões
python tests/test_multiple_tcp_connections.py

# 3. Monitorar servidor
./scripts/monitor_server.bat
```

## 🔍 Validação e Testes

### ✅ **Checklist de Implementação:**

- [ ] **Login Blueprint** implementado com TCP
- [ ] **Registro Blueprint** implementado com validação
- [ ] **Character Selection** implementado com TCP validation
- [ ] **Character Item** implementado com ações
- [ ] **TCP Connection** funcionando
- [ ] **Token Validation** funcionando
- [ ] **Error Handling** implementado
- [ ] **Loading States** implementados
- [ ] **Visual Feedback** implementado
- [ ] **Navigation Flow** funcionando

### 🧪 **Testes de Validação:**

1. **Teste de Login TCP:**
   - Login com credenciais válidas
   - Validação TCP automática
   - Navegação para Character Selection

2. **Teste de Registro:**
   - Registro com dados válidos
   - Validação de campos
   - Navegação para Login

3. **Teste de Character Selection:**
   - Carregamento de lista de personagens
   - Seleção de personagem
   - Criação de novo personagem
   - Deleção de personagem

4. **Teste de TCP Validation:**
   - Validação de token em tempo real
   - Reconexão automática
   - Fallback para HTTP

## 📚 Recursos Adicionais

### 📖 **Documentação Relacionada:**

- `docs_main/TCP_INTEGRATION_COMPLETE.md` - Integração TCP completa
- `docs_main/SISTEMA_PERSONAGENS.md` - Sistema de personagens
- `docs_main/UE5_API_INTEGRATION.md` - Integração UE5 API
- `docs_main/GUIA_UI_WIDGETS_UE5.md` - Guia de Widgets UE5

### 🔧 **Scripts de Teste:**

- `scripts_main/test_tcp_integration.py` - Teste de integração TCP
- `scripts_main/test_multiple_tcp_connections.py` - Teste múltiplas conexões
- `scripts_main/monitor_server.bat` - Monitoramento do servidor

### 🎯 **Próximos Passos:**

1. Implementar Blueprints seguindo este guia
2. Testar cada componente individualmente
3. Integrar todos os componentes
4. Validar fluxo completo
5. Otimizar performance
6. Implementar testes automatizados

---

**🎉 Este guia fornece uma implementação completa das Blueprints com validação TCP, garantindo segurança e performance no sistema UmbraEternum! 🚀**
