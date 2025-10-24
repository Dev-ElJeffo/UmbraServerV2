# 🎨 GUIA DETALHADO - UI WIDGETS UE5 + CLASSES C++

**Objetivo**: Criar interface completa de autenticação usando as classes C++ existentes  
**Tempo**: 30-45 minutos  
**Dificuldade**: ⭐⭐ Intermediário

---

## 📚 ÍNDICE

1. [Entendendo as Classes C++](#entendendo-as-classes-c)
2. [Widget 1: Tela de Login](#widget-1-tela-de-login)
3. [Widget 2: Tela de Registro](#widget-2-tela-de-registro)
4. [Widget 3: Dashboard](#widget-3-dashboard)
5. [Level Blueprint: Inicialização](#level-blueprint-inicialização)
6. [Fluxogramas Completos](#fluxogramas-completos)
7. [Troubleshooting](#troubleshooting)

---

## 🔍 ENTENDENDO AS CLASSES C++

### 📦 UmbraGameInstance (A Classe Principal)

**Localização**: `Source/UmbraEternumUE/Core/UmbraGameInstance.h`

#### Métodos Disponíveis (Blueprint Callable):

```cpp
// ✅ Métodos que você pode chamar do Blueprint:

UFUNCTION(BlueprintCallable)
void RegisterUser(const FString& Username, const FString& Email, const FString& Password);
// Registra um novo usuário
// Username: nome do usuário
// Email: email@exemplo.com
// Password: senha

UFUNCTION(BlueprintCallable)
void LoginUser(const FString& Username, const FString& Password);
// Faz login de um usuário existente
// Username: nome do usuário
// Password: senha

UFUNCTION(BlueprintCallable)
void Logout();
// Faz logout do usuário atual

UFUNCTION(BlueprintPure)
bool IsAuthenticated() const;
// Verifica se o usuário está autenticado
// Retorna: TRUE se logado, FALSE se não

UFUNCTION(BlueprintPure)
FString GetCurrentUsername() const;
// Retorna o username do usuário logado

UFUNCTION(BlueprintPure)
bool IsAdmin() const;
// Verifica se o usuário é administrador
// Retorna: TRUE se admin, FALSE se não

UFUNCTION(BlueprintPure)
FUmbraAccountData GetAccountData() const;
// Retorna todos os dados da conta
// Struct com: AccountID, Username, Email, IsAdmin
```

#### Delegates (Eventos que você pode escutar):

```cpp
// ✅ Eventos que disparam automaticamente:

UPROPERTY(BlueprintAssignable)
FOnRegistrationSuccess OnRegistrationSuccess;
// Dispara quando registro é bem-sucedido
// Parâmetro: FString Message

UPROPERTY(BlueprintAssignable)
FOnRegistrationFailed OnRegistrationFailed;
// Dispara quando registro falha
// Parâmetro: FString ErrorMessage

UPROPERTY(BlueprintAssignable)
FOnLoginSuccess OnLoginSuccess;
// Dispara quando login é bem-sucedido
// Sem parâmetros

UPROPERTY(BlueprintAssignable)
FOnLoginFailed OnLoginFailed;
// Dispara quando login falha
// Parâmetro: FString ErrorMessage

UPROPERTY(BlueprintAssignable)
FOnLogout OnLogout;
// Dispara quando faz logout
// Sem parâmetros
```

### 📊 Estruturas de Dados:

```cpp
// FUmbraAccountData (retornado por GetAccountData)
struct FUmbraAccountData
{
    int32 AccountID;      // ID da conta no banco
    FString Username;     // Nome do usuário
    FString Email;        // Email
    bool bIsAdmin;        // É admin?
};
```

---

## 📱 WIDGET 1: TELA DE LOGIN

### PASSO 1: Criar Widget Blueprint

1. **Content Browser** → Botão direito → **User Interface** → **Widget Blueprint**
2. Nome: `WBP_Login`
3. Duplo clique para abrir

### PASSO 2: Designer (Layout Visual)

#### Hierarquia de Widgets:

```
Canvas Panel (Root)
└── Overlay
    └── Vertical Box [center, center]
        ├── Spacer [Fill, 0.3]
        ├── Text Block (Título)
        │   └── Text: "UMBRA ETERNUM - LOGIN"
        │   └── Font Size: 48
        │   └── Color: White
        │   └── Justification: Center
        ├── Spacer [Fill, 0.1]
        ├── Editable Text (Username)
        │   └── Name: TXT_Username
        │   └── Hint Text: "Digite seu username"
        │   └── Size: 400x40
        ├── Spacer [Fixed, 10]
        ├── Editable Text (Password)
        │   └── Name: TXT_Password
        │   └── Hint Text: "Digite sua senha"
        │   └── Size: 400x40
        │   └── Is Password: ✓ TRUE
        ├── Spacer [Fixed, 20]
        ├── Horizontal Box
        │   ├── Button (Login)
        │   │   └── Name: BTN_Login
        │   │   └── Size: 180x50
        │   │   └── Text: "ENTRAR"
        │   └── Spacer [Fixed, 20]
        │   └── Button (Register)
        │       └── Name: BTN_Register
        │       └── Size: 180x50
        │       └── Text: "CRIAR CONTA"
        ├── Spacer [Fixed, 20]
        ├── Text Block (Status)
        │   └── Name: TXT_Status
        │   └── Text: ""
        │   └── Color: Yellow
        │   └── Font Size: 16
        │   └── Justification: Center
        └── Spacer [Fill, 0.6]
```

**Configurações Importantes**:

**Vertical Box**:
- Anchor: Center
- Alignment: 0.5, 0.5
- Size to Content: TRUE

**Editable Text (ambos)**:
- Is Read Only: FALSE
- Is Password: TRUE (apenas para TXT_Password)

**Buttons**:
- Normal: Cinza escuro
- Hovered: Cinza claro
- Pressed: Azul

### PASSO 3: Event Graph (Lógica)

#### 📋 OVERVIEW DO FLUXO:

```
Event Construct
    ↓
Configurar Input Mode
    ↓
Obter Game Instance
    ↓
Fazer Cast para UmbraGameInstance
    ↓
Criar Variável Local (MyGameInstance)
    ↓
Bind Events (Login Success/Failed)
    ↓
Limpar campos de texto
```

#### 🔧 IMPLEMENTAÇÃO DETALHADA:

---

#### **1. Event Construct** (Executado quando o widget é criado)

**Nós e Conexões**:

```
[Event Construct]
    ↓ (execution pin branco)
[Set Input Mode UI Only]
    • Target: Get Player Controller (Index 0)
    • Widget to Focus: Self
    ↓
[Show Mouse Cursor]
    • Get Player Controller (Index 0)
    • Show Mouse Cursor: TRUE
    ↓
[Get Game Instance]
    ↓ (objeto azul)
[Cast to UmbraGameInstance]
    • Object: (conectar do Get Game Instance)
    ↓ (execution pin branco)
[Promote to Variable]
    • Nome da variável: "MyGameInstance"
    • Type: UmbraGameInstance (Object Reference)
    ↓
[Branch] (condicional)
    • Condition: Is Valid (MyGameInstance)
    ↓ True
[BIND EVENTS] (ver próxima seção)
```

**DETALHES DE CADA NÓ**:

**Set Input Mode UI Only**:
- O que faz: Define que apenas a UI recebe input
- Por quê: Para o player não controlar o personagem enquanto está no menu
- Target: Player Controller 0 (o jogador local)
- Widget to Focus: Self (o próprio widget de login)

**Show Mouse Cursor**:
- O que faz: Mostra o cursor do mouse
- Por quê: Para o player poder clicar nos botões

**Get Game Instance**:
- O que faz: Retorna a instância única do Game Instance
- Por quê: Para acessar nosso UmbraGameInstance

**Cast to UmbraGameInstance**:
- O que faz: Converte de UGameInstance genérico para nosso UmbraGameInstance específico
- Por quê: Para acessar os métodos customizados (RegisterUser, LoginUser, etc)

**Promote to Variable (MyGameInstance)**:
- O que faz: Salva a referência em uma variável
- Por quê: Para reutilizar em outros eventos sem fazer Cast novamente

---

#### **2. Bind Events** (Conectar aos eventos de sucesso/erro)

Continuando do Event Construct, após o Cast:

```
[MyGameInstance]
    ↓
[Bind Event to OnLoginSuccess]
    • Event: Create Event (criar novo)
    ↓
[Event OnLoginSuccess_Event] (novo Event Graph criado)
    ↓
[Set Text (TXT_Status)]
    • Text: "✓ Login bem-sucedido!"
    ↓
[Set Color and Opacity (TXT_Status)]
    • Color: Verde (0, 1, 0, 1)
    ↓
[Delay]
    • Duration: 1.0
    ↓
[Remove from Parent] (Self)
    ↓
[Create Widget]
    • Class: WBP_Dashboard
    ↓
[Add to Viewport]
    • Target: (widget criado acima)
    • Z-Order: 0
```

**Repetir para OnLoginFailed**:

```
[MyGameInstance]
    ↓
[Bind Event to OnLoginFailed]
    • Event: Create Event
    ↓
[Event OnLoginFailed_Event] (parâmetro: ErrorMessage)
    ↓
[Set Text (TXT_Status)]
    • Text: [ErrorMessage]
    ↓
[Set Color and Opacity (TXT_Status)]
    • Color: Vermelho (1, 0, 0, 1)
```

**EXPLICAÇÃO DOS DELEGATES**:

- **Bind Event to OnLoginSuccess**: "Quando o login der certo, execute esta função"
- **Bind Event to OnLoginFailed**: "Quando o login falhar, execute esta função"
- Esses eventos são disparados automaticamente pelo C++ quando a API responde

---

#### **3. Button Login - On Clicked**

**Fluxo Completo**:

```
[OnClicked (BTN_Login)]
    ↓
[Get Text] (TXT_Username)
    ↓ (FText)
[To String]
    ↓ (FString) → [Variável: Username]
    ↓
[Get Text] (TXT_Password)
    ↓ (FText)
[To String]
    ↓ (FString) → [Variável: Password]
    ↓
[Branch]
    • Condition: NOT (Is Empty or Whitespace - Username)
    ↓ True
[Branch]
    • Condition: NOT (Is Empty or Whitespace - Password)
    ↓ True
[MyGameInstance]
    ↓
[Login User]
    • Username: [Username]
    • Password: [Password]
    ↓
[Set Text (TXT_Status)]
    • Text: "Autenticando..."
    ↓
[Set Color and Opacity (TXT_Status)]
    • Color: Amarelo (1, 1, 0, 1)

    ↓ False (de qualquer Branch)
[Set Text (TXT_Status)]
    • Text: "⚠ Preencha todos os campos!"
    ↓
[Set Color and Opacity (TXT_Status)]
    • Color: Vermelho (1, 0, 0, 1)
```

**DETALHES**:

**Get Text → To String**:
- Por quê: Widget usa FText, mas API precisa de FString
- Conversão automática

**Branch (Is Empty)**:
- O que faz: Verifica se o campo não está vazio
- Por quê: Validação básica antes de chamar a API

**Login User**:
- O que faz: Chama o método C++ que faz requisição HTTP
- Assíncrono: Não trava o jogo enquanto espera resposta
- Resultado: Dispara OnLoginSuccess ou OnLoginFailed

**Set Text (Status)**:
- Feedback visual para o usuário
- "Autenticando..." → Processando
- "Login bem-sucedido!" → Sucesso (vem do Bind)
- "Erro: ..." → Falha (vem do Bind)

---

#### **4. Button Register - On Clicked**

**Fluxo Simples**:

```
[OnClicked (BTN_Register)]
    ↓
[Remove from Parent] (Self)
    ↓
[Create Widget]
    • Class: WBP_Register
    ↓
[Add to Viewport]
    • Z-Order: 0
```

**EXPLICAÇÃO**:

- Remove o widget de login da tela
- Cria o widget de registro
- Adiciona à viewport para mostrar na tela

---

### PASSO 4: Variáveis do Widget

**No painel Variables (lado esquerdo)**:

| Nome | Type | Default | Exposição |
|------|------|---------|-----------|
| `MyGameInstance` | UmbraGameInstance (Object Reference) | None | Private |

**Por quê ter essa variável?**:
- Salva a referência ao Game Instance
- Evita fazer Cast toda vez
- Performance melhor

---

## 📝 WIDGET 2: TELA DE REGISTRO

### PASSO 1: Criar Widget

1. Content Browser → Widget Blueprint
2. Nome: `WBP_Register`

### PASSO 2: Designer

```
Canvas Panel (Root)
└── Overlay
    └── Vertical Box [center]
        ├── Spacer [Fill, 0.2]
        ├── Text Block (Título)
        │   └── Text: "CRIAR NOVA CONTA"
        │   └── Font Size: 40
        ├── Spacer [Fill, 0.1]
        ├── Editable Text (Username)
        │   └── Name: TXT_Username
        │   └── Hint: "Escolha um username"
        ├── Spacer [Fixed, 10]
        ├── Editable Text (Email)
        │   └── Name: TXT_Email
        │   └── Hint: "seu@email.com"
        ├── Spacer [Fixed, 10]
        ├── Editable Text (Password)
        │   └── Name: TXT_Password
        │   └── Hint: "Senha (min 6 caracteres)"
        │   └── Is Password: TRUE
        ├── Spacer [Fixed, 10]
        ├── Editable Text (Confirm Password)
        │   └── Name: TXT_ConfirmPassword
        │   └── Hint: "Confirme sua senha"
        │   └── Is Password: TRUE
        ├── Spacer [Fixed, 20]
        ├── Horizontal Box
        │   ├── Button (Create)
        │   │   └── Name: BTN_Create
        │   │   └── Text: "CRIAR CONTA"
        │   └── Spacer [Fixed, 20]
        │   └── Button (Back)
        │       └── Name: BTN_Back
        │       └── Text: "VOLTAR"
        ├── Spacer [Fixed, 20]
        ├── Text Block (Status)
        │   └── Name: TXT_Status
        │   └── Color: Yellow
        └── Spacer [Fill, 0.4]
```

### PASSO 3: Event Graph

#### **Event Construct** (igual ao Login):

```
[Event Construct]
    ↓
[Set Input Mode UI Only]
    ↓
[Show Mouse Cursor]
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Promote to Variable: MyGameInstance]
    ↓
[Bind Event to OnRegistrationSuccess]
    • Event: Create Event
    ↓
[Event OnRegistrationSuccess_Event] (param: Message)
    ↓
[Set Text (TXT_Status)]
    • Text: [Message]
    ↓
[Set Color (TXT_Status)]
    • Color: Verde
    ↓
[Delay] 2.0s
    ↓
[Remove from Parent] (Self)
    ↓
[Create Widget: WBP_Login]
    ↓
[Add to Viewport]

[Bind Event to OnRegistrationFailed]
    • Event: Create Event
    ↓
[Event OnRegistrationFailed_Event] (param: ErrorMessage)
    ↓
[Set Text (TXT_Status)]
    • Text: [ErrorMessage]
    ↓
[Set Color (TXT_Status)]
    • Color: Vermelho
```

---

#### **Button Create - On Clicked**

**FLUXO COMPLETO COM VALIDAÇÕES**:

```
[OnClicked (BTN_Create)]
    ↓
[Get Text (TXT_Username)] → [To String] → Username
[Get Text (TXT_Email)] → [To String] → Email
[Get Text (TXT_Password)] → [To String] → Password
[Get Text (TXT_ConfirmPassword)] → [To String] → ConfirmPassword
    ↓
[Branch 1: Campos vazios?]
    • Condition: OR
        - Is Empty (Username)
        - Is Empty (Email)
        - Is Empty (Password)
    ↓ True
    [Set Text (TXT_Status)]
        • "⚠ Preencha todos os campos!"
    [Set Color] Red
    [STOP]
    
    ↓ False
[Branch 2: Senhas conferem?]
    • Condition: Equal (Password, ConfirmPassword)
    ↓ False
    [Set Text (TXT_Status)]
        • "⚠ As senhas não conferem!"
    [Set Color] Red
    [STOP]
    
    ↓ True
[Branch 3: Senha muito curta?]
    • Condition: Len (Password) >= 6
    ↓ False
    [Set Text (TXT_Status)]
        • "⚠ Senha deve ter no mínimo 6 caracteres!"
    [Set Color] Red
    [STOP]
    
    ↓ True
[Branch 4: Email válido?]
    • Condition: Contains (Email, "@")
    ↓ False
    [Set Text (TXT_Status)]
        • "⚠ Email inválido!"
    [Set Color] Red
    [STOP]
    
    ↓ True (TODAS VALIDAÇÕES PASSARAM)
[MyGameInstance]
    ↓
[Register User]
    • Username: [Username]
    • Email: [Email]
    • Password: [Password]
    ↓
[Set Text (TXT_Status)]
    • "Criando conta..."
    ↓
[Set Color] Yellow
```

**EXPLICAÇÃO DAS VALIDAÇÕES**:

1. **Campos Vazios**: Verifica se todos os campos foram preenchidos
2. **Senhas Conferem**: Compara Password com ConfirmPassword
3. **Senha Mínima**: Verifica se tem pelo menos 6 caracteres
4. **Email Válido**: Verifica se contém @ (validação básica)

Se alguma falhar, mostra mensagem e para o fluxo.  
Se todas passarem, chama `Register User` do C++.

---

#### **Button Back - On Clicked**

```
[OnClicked (BTN_Back)]
    ↓
[Remove from Parent] (Self)
    ↓
[Create Widget: WBP_Login]
    ↓
[Add to Viewport]
```

---

## 🏠 WIDGET 3: DASHBOARD

### PASSO 1: Criar Widget

1. Content Browser → Widget Blueprint
2. Nome: `WBP_Dashboard`

### PASSO 2: Designer

```
Canvas Panel
└── Overlay
    ├── Image (Background)
    │   └── Color: Preto semi-transparente (0,0,0,0.7)
    │   └── Brush: SlateBrush (None)
    └── Vertical Box [center]
        ├── Spacer [Fill, 0.3]
        ├── Text Block (Título)
        │   └── Text: "🎮 DASHBOARD"
        │   └── Font Size: 48
        ├── Spacer [Fixed, 30]
        ├── Horizontal Box (Info)
        │   ├── Vertical Box (Labels)
        │   │   ├── Text: "Username:"
        │   │   ├── Text: "Account ID:"
        │   │   ├── Text: "Status:"
        │   │   └── Text: "Admin:"
        │   └── Spacer [Fixed, 20]
        │   └── Vertical Box (Values)
        │       ├── Text Block
        │       │   └── Name: TXT_Username
        │       │   └── Color: Cyan
        │       ├── Text Block
        │       │   └── Name: TXT_AccountID
        │       │   └── Color: White
        │       ├── Text Block
        │       │   └── Name: TXT_Status
        │       │   └── Color: Green
        │       │   └── Text: "✓ Conectado"
        │       └── Text Block
        │           └── Name: TXT_IsAdmin
        │           └── Color: Gold
        ├── Spacer [Fixed, 40]
        ├── Horizontal Box (Botões)
        │   ├── Button (Admin Panel)
        │   │   └── Name: BTN_AdminPanel
        │   │   └── Text: "👑 PAINEL ADMIN"
        │   │   └── Visibility: Collapsed
        │   ├── Spacer [Fixed, 20]
        │   └── Button (Logout)
        │       └── Name: BTN_Logout
        │       └── Text: "🚪 SAIR"
        └── Spacer [Fill, 0.4]
```

### PASSO 3: Event Graph

#### **Event Construct**

**FLUXO COMPLETO**:

```
[Event Construct]
    ↓
[Set Input Mode UI Only]
    ↓
[Show Mouse Cursor]
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Promote to Variable: MyGameInstance]
    ↓
[Is Valid?] (MyGameInstance)
    ↓ True
    ├── [Branch 1: Is Authenticated?]
    │   • Call: Is Authenticated (MyGameInstance)
    │   ↓ False
    │   [Print String: "Usuário não autenticado!"]
    │   [Remove from Parent] (Self)
    │   [Create Widget: WBP_Login]
    │   [Add to Viewport]
    │   [STOP]
    │
    │   ↓ True (usuário está autenticado)
    │   [Get Current Username] (MyGameInstance)
    │       ↓ (FString)
    │   [Set Text (TXT_Username)]
    │       • Text: [Username]
    │
    │   [Get Account Data] (MyGameInstance)
    │       ↓ (FUmbraAccountData struct)
    │   [Break FUmbraAccountData]
    │       • AccountID → [To String] → [Set Text (TXT_AccountID)]
    │       • Username → (já pegamos acima)
    │       • Email → (não usado aqui)
    │       • bIsAdmin → [Branch 2]
    │
    │   [Branch 2: Is Admin?]
    │       • Condition: [bIsAdmin]
    │       ↓ True
    │       [Set Visibility (BTN_AdminPanel)]
    │           • Visibility: Visible
    │       [Set Text (TXT_IsAdmin)]
    │           • Text: "✓ SIM"
    │       [Set Color (TXT_IsAdmin)]
    │           • Color: Gold
    │
    │       ↓ False
    │       [Set Text (TXT_IsAdmin)]
    │           • Text: "✗ NÃO"
    │       [Set Color (TXT_IsAdmin)]
    │           • Color: Gray
```

**EXPLICAÇÃO DETALHADA**:

**Break FUmbraAccountData**:
- O que faz: Divide o struct em suas partes individuais
- Retorna: AccountID (int32), Username (FString), Email (FString), bIsAdmin (bool)
- Por quê: Para pegar cada campo separadamente

**Is Authenticated**:
- Verifica se o usuário fez login
- Se não, redireciona para tela de login
- Segurança: Previne acesso sem autenticação

**Visibility do Admin Panel**:
- Collapsed = Invisível e não ocupa espaço
- Visible = Visível
- Hidden = Invisível mas ocupa espaço

---

#### **Button Logout - On Clicked**

```
[OnClicked (BTN_Logout)]
    ↓
[MyGameInstance]
    ↓
[Logout]
    ↓
[Remove from Parent] (Self)
    ↓
[Create Widget: WBP_Login]
    ↓
[Add to Viewport]
    ↓
[Print String]
    • "Logout realizado com sucesso!"
```

**O que acontece no C++**:
- Limpa token de autenticação
- Limpa dados da conta
- Dispara evento OnLogout (se você bindar)

---

#### **Button Admin Panel - On Clicked**

```
[OnClicked (BTN_AdminPanel)]
    ↓
[Print String]
    • "TODO: Implementar painel admin"
    ↓
[Create Widget: WBP_AdminPanel] (criar futuramente)
    ↓
[Add to Viewport]
```

---

## 🎬 LEVEL BLUEPRINT: INICIALIZAÇÃO

### Como Abrir:

1. Com o level aberto (`Lvl_TestAuth`)
2. **Blueprints** (menu superior) → **Open Level Blueprint**

### Event Graph:

```
[Event BeginPlay]
    ↓
[Delay] 0.1s (para garantir que tudo carregou)
    ↓
[Create Widget]
    • Class: WBP_Login
    • Owning Player: Get Player Controller (Index 0)
    ↓
[Add to Viewport]
    • Target: (widget criado)
    • Z-Order: 0
    ↓
[Set Input Mode UI Only]
    • Player Controller: Get Player Controller (Index 0)
    • Widget to Focus: (widget criado)
    ↓
[Show Mouse Cursor]
    • Target: Get Player Controller (Index 0)
    • Show Mouse Cursor: TRUE
```

**EXPLICAÇÃO**:

**Event BeginPlay**:
- Executado quando o level inicia
- Primeiro evento do jogo

**Delay 0.1s**:
- Dá tempo para inicializar sistemas
- Evita crashes em projetos complexos
- Opcional, mas recomendado

**Create Widget**:
- Cria instância do WBP_Login
- Owning Player: Necessário para inputs funcionarem

**Add to Viewport**:
- Adiciona o widget na tela
- Z-Order: 0 = Na frente de tudo

**Set Input Mode UI Only**:
- Bloqueia input do jogo
- Apenas UI recebe input
- Widget to Focus: Define qual widget recebe input primeiro

---

## 📊 FLUXOGRAMAS COMPLETOS

### 🔄 FLUXO DE REGISTRO

```
┌─────────────────────┐
│   WBP_Register      │
│  (Tela de Registro) │
└──────────┬──────────┘
           │
           │ (1) Usuário preenche campos
           │     Username, Email, Password
           ↓
    ┌─────────────┐
    │ Validações  │
    │   Client    │
    └──────┬──────┘
           │
    ┌──────┴──────────────────┐
    │ Campos vazios?          │
    │ Senhas conferem?        │
    │ Senha >= 6 caracteres?  │
    │ Email tem @?            │
    └──────┬──────────────────┘
           │
           ├─→ FALHOU → Mostra erro vermelho → FIM
           │
           ↓ PASSOU
    ┌────────────────────┐
    │ MyGameInstance     │
    │ →RegisterUser()    │
    └──────┬─────────────┘
           │
           │ (2) Chamada HTTP Assíncrona
           │     para /api/register.php
           ↓
    ┌────────────────────┐
    │   PHP API          │
    │ (umbra_api)        │
    └──────┬─────────────┘
           │
           │ (3) INSERT no MySQL
           ↓
    ┌────────────────────┐
    │  MySQL Database    │
    │  (umbra_game_db)   │
    └──────┬─────────────┘
           │
           │ (4) Resposta JSON
           ↓
    ┌────────────────────────────┐
    │ Success?                   │
    └──────┬─────────────────────┘
           │
           ├─→ TRUE → OnRegistrationSuccess
           │          ├─→ Mostra: "Conta criada!"
           │          ├─→ Delay 2s
           │          ├─→ Remove WBP_Register
           │          └─→ Create WBP_Login
           │
           └─→ FALSE → OnRegistrationFailed
                       └─→ Mostra erro vermelho
```

### 🔐 FLUXO DE LOGIN

```
┌─────────────────────┐
│   WBP_Login         │
│  (Tela de Login)    │
└──────────┬──────────┘
           │
           │ (1) Usuário preenche
           │     Username, Password
           ↓
    ┌─────────────┐
    │ Validações  │
    └──────┬──────┘
           │
           ├─→ Campos vazios? → Mostra erro → FIM
           │
           ↓ OK
    ┌────────────────────┐
    │ MyGameInstance     │
    │ →LoginUser()       │
    └──────┬─────────────┘
           │
           │ (2) HTTP POST para /api/login.php
           ↓
    ┌────────────────────┐
    │   PHP API          │
    └──────┬─────────────┘
           │
           │ (3) SELECT no MySQL
           │     Verifica credenciais
           ↓
    ┌────────────────────┐
    │  MySQL Database    │
    └──────┬─────────────┘
           │
           │ (4) Resposta JSON
           │     { success, token, account, player }
           ↓
    ┌────────────────────────────┐
    │ Success?                   │
    └──────┬─────────────────────┘
           │
           ├─→ TRUE → OnLoginSuccess
           │          ├─→ Salva token no C++
           │          ├─→ Salva dados da conta
           │          ├─→ Mostra: "Login bem-sucedido!"
           │          ├─→ Delay 1s
           │          ├─→ Remove WBP_Login
           │          └─→ Create WBP_Dashboard
           │
           └─→ FALSE → OnLoginFailed
                       └─→ Mostra erro
```

### 🏠 FLUXO DO DASHBOARD

```
┌─────────────────────┐
│  WBP_Dashboard      │
│   (Event Construct) │
└──────────┬──────────┘
           │
           │ (1) Get Game Instance
           ↓
    ┌─────────────────┐
    │ Is Authenticated?│
    └──────┬──────────┘
           │
           ├─→ FALSE → Remove Dashboard
           │          → Create WBP_Login
           │          → FIM (volta pro login)
           │
           ↓ TRUE
    ┌─────────────────┐
    │ Get Account Data│
    │ (do C++)        │
    └──────┬──────────┘
           │
           │ Retorna:
           │ • AccountID
           │ • Username
           │ • Email
           │ • bIsAdmin
           ↓
    ┌────────────────────────┐
    │ Preenche UI:           │
    │ • TXT_Username         │
    │ • TXT_AccountID        │
    │ • TXT_Status           │
    └──────┬─────────────────┘
           │
           ↓
    ┌────────────────────────┐
    │ Is Admin?              │
    └──────┬─────────────────┘
           │
           ├─→ TRUE → Show BTN_AdminPanel
           │          Set TXT_IsAdmin: "✓ SIM"
           │
           └─→ FALSE → Hide BTN_AdminPanel
                       Set TXT_IsAdmin: "✗ NÃO"
```

---

## 🎨 DIAGRAMA DE CONEXÃO C++ ↔ BLUEPRINT

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│              CLASSE C++                             │
│          UmbraGameInstance                          │
│                                                     │
│  ┌─────────────────────────────────────┐           │
│  │         MÉTODOS                     │           │
│  │  ✅ RegisterUser()                  │ ←─────────┼─── BTN_Create (WBP_Register)
│  │  ✅ LoginUser()                     │ ←─────────┼─── BTN_Login (WBP_Login)
│  │  ✅ Logout()                        │ ←─────────┼─── BTN_Logout (WBP_Dashboard)
│  │  ✅ IsAuthenticated()               │ ←─────────┼─── Event Construct (WBP_Dashboard)
│  │  ✅ GetCurrentUsername()            │ ←─────────┼─── Event Construct (WBP_Dashboard)
│  │  ✅ GetAccountData()                │ ←─────────┼─── Event Construct (WBP_Dashboard)
│  │  ✅ IsAdmin()                       │ ←─────────┼─── Event Construct (WBP_Dashboard)
│  └─────────────────────────────────────┘           │
│                                                     │
│  ┌─────────────────────────────────────┐           │
│  │         DELEGATES (EVENTOS)         │           │
│  │  📢 OnRegistrationSuccess           │ ─────────→┼─── Bind Event (WBP_Register)
│  │  📢 OnRegistrationFailed            │ ─────────→┼─── Bind Event (WBP_Register)
│  │  📢 OnLoginSuccess                  │ ─────────→┼─── Bind Event (WBP_Login)
│  │  📢 OnLoginFailed                   │ ─────────→┼─── Bind Event (WBP_Login)
│  │  📢 OnLogout                        │ ─────────→┼─── Bind Event (Opcional)
│  └─────────────────────────────────────┘           │
│                                                     │
└─────────────────────────────────────────────────────┘
                        ↕
                 HTTP Requests
                        ↕
            ┌─────────────────────┐
            │    PHP APIs         │
            │  (localhost/umbra)  │
            └──────────┬──────────┘
                       ↕
            ┌─────────────────────┐
            │   MySQL Database    │
            │  (umbra_game_db)    │
            └─────────────────────┘
```

---

## 🐛 TROUBLESHOOTING

### ❌ Erro: "Failed to find VaRest Subsystem"

**Causa**: VaRest não está carregado

**Solução**:
1. Edit → Plugins → VaRest → Enabled
2. Restart Editor
3. Recompilar projeto C++

---

### ❌ Erro: "Attempted to access None"

**Causa**: MyGameInstance é null

**Solução**:
1. Verificar se Cast to UmbraGameInstance está retornando válido
2. Adicionar Branch após Cast (Is Valid?)
3. Verificar se Game Instance Class está configurada

---

### ❌ UI não aparece na tela

**Causa**: Add to Viewport não foi chamado

**Solução**:
1. Verificar se Create Widget → Add to Viewport
2. Verificar Z-Order (deve ser >= 0)
3. Verificar se widget pai não está escondido

---

### ❌ Não consigo clicar nos botões

**Causa**: Input Mode está errado

**Solução**:
1. Verificar Set Input Mode UI Only
2. Verificar Show Mouse Cursor = TRUE
3. Verificar se há outro widget bloqueando

---

### ❌ Delegates não disparam

**Causa**: Bind Event não foi chamado

**Solução**:
1. Verificar se Bind Event está no Event Construct
2. Verificar se MyGameInstance é válido antes do Bind
3. Testar criar Print String dentro do Event para debug

---

### ❌ API retorna erro 404

**Causa**: URL incorreta ou WAMP não está rodando

**Solução**:
1. Verificar se WAMP está verde (rodando)
2. Testar URL no navegador: `http://localhost/umbra_api/api/test.php`
3. Verificar ServerURL no UmbraGameInstance.h

---

### ❌ Texto não aparece nos campos

**Causa**: Text binding incorreto

**Solução**:
1. Usar Set Text, não Set String
2. Converter FString para FText se necessário
3. Verificar se a variável Text está exposta

---

## ✅ CHECKLIST FINAL

### WBP_Login:
- [ ] Widget criado com nome correto
- [ ] Layout completo (2 Editable Text, 2 Buttons, 1 Text Status)
- [ ] TXT_Password → Is Password = TRUE
- [ ] Event Construct → Set Input Mode UI Only
- [ ] Event Construct → Bind to OnLoginSuccess
- [ ] Event Construct → Bind to OnLoginFailed
- [ ] BTN_Login → Validações implementadas
- [ ] BTN_Login → Chama LoginUser()
- [ ] BTN_Register → Abre WBP_Register

### WBP_Register:
- [ ] Widget criado
- [ ] 4 Editable Text (Username, Email, Password, Confirm)
- [ ] Event Construct → Bind events
- [ ] BTN_Create → Validações (4 checks)
- [ ] BTN_Create → Chama RegisterUser()
- [ ] BTN_Back → Volta para WBP_Login

### WBP_Dashboard:
- [ ] Widget criado
- [ ] Layout com informações do usuário
- [ ] Event Construct → GetAccountData()
- [ ] Event Construct → Preenche textos
- [ ] Event Construct → Verifica IsAdmin
- [ ] BTN_Logout → Chama Logout()
- [ ] BTN_AdminPanel visível apenas se admin

### Level Blueprint:
- [ ] Event BeginPlay → Create WBP_Login
- [ ] Event BeginPlay → Add to Viewport
- [ ] Event BeginPlay → Set Input Mode UI Only

---

## 🎉 CONCLUSÃO

Após seguir este guia, você terá:

✅ **Sistema completo de autenticação**  
✅ **3 Widgets funcionais** (Login, Register, Dashboard)  
✅ **Conexão perfeita** entre Blueprint e C++  
✅ **Validações client-side**  
✅ **Feedback visual** para o usuário  
✅ **Fluxo completo** de registro → login → dashboard  

**Tempo total**: 30-45 minutos (se seguir passo a passo)  
**Resultado**: Sistema profissional de autenticação!

---

**Próximo passo**: Testar tudo e depois implementar funcionalidades avançadas!

**Arquivo criado**: `D:\UmbraServerV2\UmbraServer\GUIA_UI_WIDGETS_UE5.md`

