# 🎯 PLANO DE IMPLEMENTAÇÃO COMPLETO - UE5 + APIs

**Data**: 14/10/2025  
**Status**: ✅ Projeto compilado com sucesso!  
**Objetivo**: Conectar UE5 com APIs PHP funcionais

---

## 📊 ESTADO ATUAL

### ✅ O QUE JÁ TEMOS:

#### Backend (100% Pronto)
- ✅ **MySQL Database** configurado e funcionando
- ✅ **PHP APIs** funcionais:
  - `http://localhost/umbra_api/api/test.php`
  - `http://localhost/umbra_api/api/register.php`
  - `http://localhost/umbra_api/api/login.php`
  - `http://localhost/umbra_api/api/admin/*` (todas as APIs admin)
- ✅ **Páginas Web** funcionais (register.html, login.html, dashboard.html, admin.html)
- ✅ **WAMP Server** rodando

#### Unreal Engine (Classes Prontas)
- ✅ **Projeto compilado**: `UmbraEternumUE`
- ✅ **VaRest Plugin** instalado e funcionando
- ✅ **Classes C++** criadas:
  - `UmbraGameInstance` - Gerenciador de autenticação
  - `UmbraDataStructures` - Structs de dados
  - `UmbraSaveGame` - Sistema de save
- ✅ **API atualizada** para UE5.6

### ❌ O QUE FALTA IMPLEMENTAR:

1. **Configurar Game Instance** no projeto
2. **Criar UI Widgets** para teste
3. **Testar comunicação** com APIs
4. **Criar Level de teste**
5. **Implementar fluxo completo** de autenticação

---

## 🚀 PLANO DE IMPLEMENTAÇÃO

### FASE 1: CONFIGURAÇÃO DO PROJETO (15 min)

#### ✅ 1.1 - Configurar Game Instance

**Localização**: Unreal Editor

1. Abra `UmbraEternumUE` no Unreal Editor
2. **Edit → Project Settings**
3. **Maps & Modes** → **Game Instance Class**
4. Selecione: `UmbraGameInstance`
5. **Save**

#### ✅ 1.2 - Configurar URL do Servidor

**Editar**: `Source/UmbraEternumUE/Core/UmbraGameInstance.h`

Verifique se a URL está correta (já deve estar):
```cpp
private:
    FString ServerURL = TEXT("http://localhost/umbra_api");
```

Se precisar mudar:
- Localhost: `http://localhost/umbra_api`
- IP Local: `http://192.168.1.100/umbra_api`
- Servidor externo: `http://seuservidor.com/umbra_api`

#### ✅ 1.3 - Criar Level de Teste

1. **File → New Level → Empty Level**
2. Adicionar:
   - **Player Start** (coloque no 0,0,0)
   - **Light Source** (Directional Light)
   - **Sky Sphere** (para visualização)
3. **Save As**: `Lvl_TestAuth`
4. **Edit → Project Settings → Maps & Modes**
5. **Default Maps**:
   - Editor Startup Map: `Lvl_TestAuth`
   - Game Default Map: `Lvl_TestAuth`

---

### FASE 2: CRIAR UI DE TESTE (30 min)

#### ✅ 2.1 - Widget: Tela de Login

**Criar Widget**:
1. **Content Browser** → Botão direito → **User Interface → Widget Blueprint**
2. Nome: `WBP_Login`
3. Abra o Widget

**Layout**:
```
Canvas Panel
├── Vertical Box (Center Screen)
│   ├── Text Block (Título: "UMBRA ETERNUM - LOGIN")
│   ├── Editable Text Box (Username)
│   │   └── Name: "TXT_Username"
│   │   └── Hint Text: "Digite seu username"
│   ├── Editable Text Box (Password)
│   │   └── Name: "TXT_Password"
│   │   └── Hint Text: "Digite sua senha"
│   │   └── Is Password: TRUE
│   ├── Horizontal Box (Botões)
│   │   ├── Button "LOGIN"
│   │   │   └── Name: "BTN_Login"
│   │   └── Button "REGISTER"
│   │       └── Name: "BTN_Register"
│   └── Text Block (Status)
│       └── Name: "TXT_Status"
│       └── Color: Yellow
```

**Graph (Event Graph)**:

```blueprint
Event Construct
└─→ Set Input Mode UI Only
    └─→ Show Mouse Cursor = TRUE

BTN_Login → On Clicked
└─→ Get Text (TXT_Username) → ToText → Username
└─→ Get Text (TXT_Password) → ToText → Password
└─→ Get Game Instance → Cast to UmbraGameInstance
    └─→ Login User (Username, Password)

BTN_Register → On Clicked
└─→ Remove From Parent (WBP_Login)
└─→ Create Widget (WBP_Register)
└─→ Add to Viewport
```

**Bind Events (em Event Construct)**:

```blueprint
Get Game Instance
└─→ Cast to UmbraGameInstance
    ├─→ Bind Event to On Login Success
    │   └─→ Set Text (TXT_Status) = "Login bem-sucedido!"
    │   └─→ Delay 1.0s
    │   └─→ Remove From Parent (WBP_Login)
    │   └─→ Create Widget (WBP_Dashboard)
    │   └─→ Add to Viewport
    │
    └─→ Bind Event to On Login Failed
        └─→ Set Text (TXT_Status) = Error Message
        └─→ Set Color (TXT_Status) = Red
```

---

#### ✅ 2.2 - Widget: Tela de Registro

**Criar Widget**:
1. **Content Browser** → **Widget Blueprint**
2. Nome: `WBP_Register`

**Layout**:
```
Canvas Panel
├── Vertical Box (Center Screen)
│   ├── Text Block (Título: "CRIAR CONTA")
│   ├── Editable Text Box (Username)
│   │   └── Name: "TXT_Username"
│   ├── Editable Text Box (Email)
│   │   └── Name: "TXT_Email"
│   ├── Editable Text Box (Password)
│   │   └── Name: "TXT_Password"
│   │   └── Is Password: TRUE
│   ├── Editable Text Box (Confirm Password)
│   │   └── Name: "TXT_ConfirmPassword"
│   │   └── Is Password: TRUE
│   ├── Horizontal Box (Botões)
│   │   ├── Button "CRIAR CONTA"
│   │   │   └── Name: "BTN_Register"
│   │   └── Button "VOLTAR"
│   │       └── Name: "BTN_Back"
│   └── Text Block (Status)
│       └── Name: "TXT_Status"
```

**Graph**:

```blueprint
BTN_Register → On Clicked
├─→ Get Text (TXT_Password)
│   └─→ Branch (Password == ConfirmPassword?)
│       ├─→ TRUE:
│       │   └─→ Get Game Instance
│       │       └─→ Cast to UmbraGameInstance
│       │           └─→ Register User (Username, Email, Password)
│       │
│       └─→ FALSE:
│           └─→ Set Text (TXT_Status) = "Senhas não conferem!"
│           └─→ Set Color (TXT_Status) = Red

BTN_Back → On Clicked
└─→ Remove From Parent (WBP_Register)
└─→ Create Widget (WBP_Login)
└─→ Add to Viewport
```

**Bind Events**:

```blueprint
Get Game Instance → Cast to UmbraGameInstance
├─→ Bind to On Registration Success
│   └─→ Set Text (TXT_Status) = "Conta criada! Redirecionando..."
│   └─→ Delay 2.0s
│   └─→ Remove From Parent
│   └─→ Create Widget (WBP_Login)
│   └─→ Add to Viewport
│
└─→ Bind to On Registration Failed
    └─→ Set Text (TXT_Status) = Error Message
    └─→ Set Color (TXT_Status) = Red
```

---

#### ✅ 2.3 - Widget: Dashboard

**Criar Widget**:
1. Nome: `WBP_Dashboard`

**Layout**:
```
Canvas Panel
├── Vertical Box
│   ├── Text Block (Título: "DASHBOARD")
│   ├── Text Block (Username)
│   │   └── Name: "TXT_Username"
│   ├── Text Block (Account ID)
│   │   └── Name: "TXT_AccountID"
│   ├── Text Block (Status)
│   │   └── Name: "TXT_Status"
│   ├── Button "LOGOUT"
│   │   └── Name: "BTN_Logout"
│   └── Button "ADMIN PANEL" (se IsAdmin)
│       └── Name: "BTN_Admin"
│       └── Visibility: Collapsed
```

**Graph**:

```blueprint
Event Construct
├─→ Get Game Instance → Cast to UmbraGameInstance
│   ├─→ Get Current Username → Set Text (TXT_Username)
│   ├─→ Get Account Data → Account ID → ToString → Set Text (TXT_AccountID)
│   └─→ Is Admin?
│       └─→ Branch
│           └─→ TRUE: Set Visibility (BTN_Admin) = Visible

BTN_Logout → On Clicked
└─→ Get Game Instance → Cast to UmbraGameInstance
    └─→ Logout()
    └─→ Remove From Parent (WBP_Dashboard)
    └─→ Create Widget (WBP_Login)
    └─→ Add to Viewport
```

---

#### ✅ 2.4 - Level Blueprint: Mostrar Login ao Iniciar

**Abrir Level Blueprint** (`Lvl_TestAuth`):

1. No editor, com `Lvl_TestAuth` aberto
2. **Blueprints → Open Level Blueprint**

**Graph**:

```blueprint
Event BeginPlay
├─→ Create Widget (WBP_Login)
├─→ Add to Viewport
├─→ Set Input Mode UI Only
└─→ Show Mouse Cursor = TRUE
```

---

### FASE 3: TESTE DE COMUNICAÇÃO (15 min)

#### ✅ 3.1 - Teste de Registro

1. **Play** (Alt+P)
2. Clique em **"REGISTER"**
3. Preencha:
   - **Username**: `test_ue5`
   - **Email**: `test@ue5.com`
   - **Password**: `123456`
   - **Confirm**: `123456`
4. Clique em **"CRIAR CONTA"**

**Resultado Esperado**:
- ✅ Status: "Conta criada! Redirecionando..."
- ✅ Volta para tela de login após 2s
- ✅ No banco: Conta criada (verificar no phpMyAdmin)

**Se houver erro**:
- Abrir **Output Log** no Unreal (Window → Developer Tools → Output Log)
- Procurar por `[UmbraGameInstance]` para ver logs detalhados

---

#### ✅ 3.2 - Teste de Login

1. Na tela de Login
2. Preencha:
   - **Username**: `test_ue5`
   - **Password**: `123456`
3. Clique em **"LOGIN"**

**Resultado Esperado**:
- ✅ Status: "Login bem-sucedido!"
- ✅ Abre Dashboard após 1s
- ✅ Dashboard mostra username e ID

**Se houver erro**:
- Verificar Output Log
- Verificar se WAMP está rodando
- Testar API manualmente: `http://localhost/umbra_api/api/test.php`

---

#### ✅ 3.3 - Teste de Logout

1. No Dashboard
2. Clique em **"LOGOUT"**

**Resultado Esperado**:
- ✅ Volta para tela de login
- ✅ Campos limpos

---

### FASE 4: VERIFICAÇÃO E DEBUG (10 min)

#### ✅ 4.1 - Verificar Logs

**No Unreal Output Log**, procurar por:

```
[UmbraGameInstance] Registrando usuário: test_ue5
[UmbraGameInstance] 🌐 Request criada: http://localhost/umbra_api/api/register.php
[UmbraGameInstance] ✅ Registro bem-sucedido!
[UmbraGameInstance] Fazendo login: test_ue5
[UmbraGameInstance] ✅ Login bem-sucedido!
```

Se ver logs de **❌ Erro**, investigar:
- URL da API
- WAMP rodando?
- MySQL rodando?

---

#### ✅ 4.2 - Verificar Banco de Dados

**Abrir phpMyAdmin**:
1. `http://localhost/phpmyadmin`
2. Database: `umbra_game_db`
3. Tabela: `accounts`
4. **Verificar**:
   - Conta `test_ue5` foi criada?
   - `last_login` foi atualizado após login?

---

#### ✅ 4.3 - Testar APIs Manualmente

**No navegador ou Postman**:

```bash
# Test
http://localhost/umbra_api/api/test.php

# Register
POST http://localhost/umbra_api/api/register.php
Body: {"username":"test2","email":"test2@test.com","password":"123456"}

# Login
POST http://localhost/umbra_api/api/login.php
Body: {"username":"test2","password":"123456"}
```

---

### FASE 5: FUNCIONALIDADES AVANÇADAS (Opcional)

#### 🔧 5.1 - Auto-Login (Lembrar-me)

**Modificar WBP_Login**:

1. Adicionar **Checkbox**: "Lembrar-me"
   - Name: `CHK_RememberMe`

2. **Graph**:
```blueprint
BTN_Login → On Clicked
└─→ Is Checked (CHK_RememberMe)?
    └─→ Branch
        └─→ TRUE: 
            └─→ Get Game Instance
                └─→ Cast to UmbraGameInstance
                    └─→ Save Auth Token (TRUE)
```

**Modificar Level Blueprint**:

```blueprint
Event BeginPlay
├─→ Get Game Instance → Cast to UmbraGameInstance
│   └─→ Load Auth Token()
│       └─→ Is Authenticated?
│           └─→ Branch
│               ├─→ TRUE: Create Widget (WBP_Dashboard)
│               └─→ FALSE: Create Widget (WBP_Login)
```

---

#### 🔧 5.2 - Painel Admin

**Criar Widget**: `WBP_AdminPanel`

Similar ao `admin.html` existente, mas em UE5:
- Lista de contas
- Botões Ban/Unban
- Status do servidor
- Logs

---

#### 🔧 5.3 - Conectar ao Servidor C++

**No futuro** (quando o servidor C++ estiver pronto):

```blueprint
On Login Success
└─→ Get Game Instance → Cast to UmbraGameInstance
    └─→ Connect To Game Server()
```

---

## 📊 CHECKLIST DE IMPLEMENTAÇÃO

### Fase 1: Configuração
- [ ] Game Instance configurado
- [ ] URL do servidor definida
- [ ] Level de teste criado

### Fase 2: UI
- [ ] WBP_Login criado
- [ ] WBP_Register criado
- [ ] WBP_Dashboard criado
- [ ] Level Blueprint configurado

### Fase 3: Testes
- [ ] Registro funcional
- [ ] Login funcional
- [ ] Logout funcional
- [ ] Dados salvos no banco

### Fase 4: Verificação
- [ ] Logs do Unreal corretos
- [ ] Banco de dados atualizado
- [ ] APIs testadas manualmente

### Fase 5: Avançado (Opcional)
- [ ] Auto-login implementado
- [ ] Painel admin criado
- [ ] Conexão com servidor C++

---

## 🎯 RESULTADO ESPERADO

Após completar todas as fases:

✅ **Tela de Login funcional** no UE5  
✅ **Tela de Registro funcional** no UE5  
✅ **Dashboard funcional** no UE5  
✅ **Comunicação com APIs PHP** funcionando  
✅ **Dados salvos no MySQL**  
✅ **Sistema de autenticação completo**  

---

## 🐛 TROUBLESHOOTING

### Erro: "VaRest Subsystem não encontrado"

**Solução**:
1. Verificar se VaRest está habilitado em Plugins
2. Recompilar projeto C++
3. Verificar se `GEngine` está válido

### Erro: "Failed to open connection"

**Solução**:
1. Verificar se WAMP está rodando
2. Testar URL no navegador: `http://localhost/umbra_api/api/test.php`
3. Verificar firewall

### Erro: "Response inválida do servidor"

**Solução**:
1. Verificar Output Log do Unreal
2. Testar API no Postman
3. Verificar formato JSON da resposta

### Widgets não aparecem

**Solução**:
1. Verificar se `Add to Viewport` foi chamado
2. Verificar `Z-Order` do widget
3. Verificar se Input Mode está correto

---

## 📚 ARQUIVOS DE REFERÊNCIA

- **Classes C++**: `/UmbraServer/UE5_CLASSES_CREATED.md`
- **Exemplos C++**: `/UmbraServer/UE5_CPP_EXAMPLES.h`
- **Guia Completo**: `/UmbraServer/UE5_API_INTEGRATION.md`
- **Quick Start**: `/UmbraServer/UE5_QUICKSTART.md`
- **APIs PHP**: `C:\wamp64\www\umbra_api\`

---

## ✅ PRÓXIMOS PASSOS

Após implementar autenticação:

1. **Criar sistema de personagens**
2. **Integrar com servidor C++**
3. **Implementar matchmaking**
4. **Sistema de inventário**
5. **Chat multiplayer**

---

## 🎉 CONCLUSÃO

Este plano fornece uma implementação **completa** e **testável** do sistema de autenticação entre Unreal Engine 5 e as APIs PHP.

**Tempo estimado**: 1-2 horas para implementação completa

**Dificuldade**: ⭐⭐ (Intermediário)

**Status**: ✅ Pronto para implementar!

---

**Última atualização**: 14/10/2025  
**Autor**: AI Assistant  
**Projeto**: UmbraEternum

