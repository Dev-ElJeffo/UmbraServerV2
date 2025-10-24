# 🚀 QUICK START - Unreal Engine 5 Integration

**Objetivo**: Integrar APIs PHP do UmbraEternum no seu projeto UE5 em **5 minutos**!

---

## ⚡ PASSO A PASSO RÁPIDO

### 1️⃣ Criar Projeto UE5 (2 min)

1. Abra **Unreal Engine Launcher**
2. Launch **Unreal Engine 5.3** (ou superior)
3. New Project:
   - **Games** → **Third Person**
   - Nome: `UmbraEternumClient`
   - **C++** (recomendado) ou **Blueprint**
4. **Create**

---

### 2️⃣ Instalar VaRest Plugin (1 min)

#### Via Marketplace:
1. Epic Games Launcher → **Marketplace**
2. Busque: **"VaRest"**
3. **Free** → **Install to Engine** → Selecione **UE 5.3**

#### No Projeto:
1. No Unreal Editor: **Edit → Plugins**
2. Busque: **"VaRest"**
3. ☑️ **Enabled**
4. **Restart Now**

---

### 3️⃣ Adicionar Código C++ (2 min)

#### Se criou projeto C++:

1. Copie os arquivos:
   - `UE5_CPP_EXAMPLES.h` → `YourProject/Source/YourProject/UmbraGameInstance.h`
   - `UE5_CPP_EXAMPLES.cpp` → `YourProject/Source/YourProject/UmbraGameInstance.cpp`

2. Edite `YourProject.Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] { 
    "Core", 
    "CoreUObject", 
    "Engine", 
    "InputCore",
    "VaRest",        // ← ADICIONAR
    "Json",          // ← ADICIONAR
    "JsonUtilities"  // ← ADICIONAR
});
```

3. No topo de ambos os arquivos, substitua:
   - `YOURPROJECT_API` → Seu projeto (ex: `UMBRAETERNUM_API`)

4. **Compile** (Ctrl+F5 no Visual Studio ou Build no UE)

#### Se criou projeto Blueprint:

1. Pule esta etapa, use apenas Blueprints (veja seção abaixo)

---

### 4️⃣ Configurar Game Instance

1. No Unreal Editor: **Edit → Project Settings**
2. **Maps & Modes**
3. **Game Instance Class**:
   - C++: Selecione `UmbraGameInstance`
   - Blueprint: Create Blueprint based on `GameInstance` → Nome: `GI_Umbra`

---

### 5️⃣ Criar UI de Login (Blueprint)

#### Widget Blueprint:

1. Content Browser → **Right Click**
2. **User Interface → Widget Blueprint**
3. Nome: `WBP_Login`

#### Design (Designer tab):

Arraste da paleta:
```
Canvas Panel
└── Vertical Box (Center screen)
    ├── Text "UmbraEternum - Login"
    ├── Editable Text Box (Name: "TxtUsername")
    ├── Editable Text Box (Name: "TxtPassword", IsPassword = true)
    ├── Button (Name: "BtnLogin")
    │   └── Text "LOGIN"
    └── Text (Name: "TxtError", Visibility = Hidden, Color = Red)
```

#### Graph (Graph tab):

**Event: OnClicked (BtnLogin)**

```
OnClicked (BtnLogin)
  ↓
Get Game Instance
  ↓
Cast to GI_Umbra (ou UmbraGameInstance)
  ↓
Login User
  Username: TxtUsername → Get Text → To String
  Password: TxtPassword → Get Text → To String
```

**Event: Construct**

```
Event Construct
  ↓
Get Game Instance
  ↓
Cast to GI_Umbra
  ↓
Bind Event to On Login Success
  ↓
[Custom Event: OnLoginSucceeded]
    Remove from Parent (self)
    Open Level: "MainMenu"
  ↓
Bind Event to On Login Failed
  ↓
[Custom Event: OnLoginFailed]
    Set Text (TxtError): Error Message
    Set Visibility (TxtError): Visible
```

---

### 6️⃣ Mostrar Login na Inicialização

#### Level Blueprint (ou PlayerController):

1. Abra o **Level Blueprint** do seu mapa inicial
2. **Event BeginPlay**:

```
Event BeginPlay
  ↓
Create Widget (WBP_Login)
  ↓
Add to Viewport
  ↓
Set Input Mode UI Only
  ↓
Show Mouse Cursor
```

---

## ✅ TESTAR!

### 1. Configure o Server URL

**Se C++**:
- Em `UmbraGameInstance.h`, linha:
  ```cpp
  FString ServerURL = TEXT("http://localhost/umbra_api");
  ```

**Se Blueprint**:
- Na `GI_Umbra`, adicione variável:
  - Nome: `ServerURL`
  - Type: `String`
  - Default: `"http://localhost/umbra_api"`

### 2. Play!

1. Pressione **Play** (Alt+P)
2. Digite username/password de uma conta existente
3. Clique **LOGIN**
4. Se sucesso → Abre nível MainMenu
5. Se erro → Mostra mensagem

---

## 🎨 VERSÃO BLUEPRINT PURA (SEM C++)

Se não quer usar C++, veja `UE5_API_INTEGRATION.md` para versão 100% Blueprint com VaRest nodes.

**Principais nodes**:
- `Construct Json Request`
- `Apply URL`
- `Set Verb` (POST)
- `Set Content As String`
- `Process Request`
- `On Request Complete`

---

## 📋 CHECKLIST

- [ ] Projeto UE5 criado
- [ ] VaRest instalado e habilitado
- [ ] Código C++ adicionado (ou Blueprint criado)
- [ ] Game Instance configurada
- [ ] Widget de Login criado
- [ ] Server URL configurado
- [ ] Testado com Play

---

## 🐛 TROUBLESHOOTING

### ❌ "VaRest module not found"

**Solução**: 
1. Edit → Plugins → VaRest → Enabled
2. Restart Editor

### ❌ "Unresolved external symbol"

**Solução**:
1. Abra `.Build.cs`
2. Adicione `"VaRest", "Json", "JsonUtilities"` aos módulos
3. Recompile

### ❌ "Connection failed"

**Solução**:
1. Verifique se WAMP está rodando
2. Teste no navegador: `http://localhost/umbra_api/api/test.php`
3. Verifique `ServerURL` na Game Instance

### ❌ Widget não aparece

**Solução**:
1. Certifique-se de chamar `Add to Viewport`
2. `Set Input Mode UI Only`
3. `Show Mouse Cursor = true`

---

## 🎯 PRÓXIMOS PASSOS

Agora que tem login funcionando:

1. ✅ Login/Register
2. ⏭️ Criar Widget de Register
3. ⏭️ Character Selection Screen
4. ⏭️ Conectar ao C++ Server (TCP Socket)
5. ⏭️ Sincronizar personagem
6. ⏭️ Movimento replicado

**Veja documentação completa**: `UE5_API_INTEGRATION.md`

---

## 📚 ARQUIVOS DO PROJETO

```
UmbraServerV2/
├── UmbraServer/
│   ├── UE5_API_INTEGRATION.md  ← Guia completo
│   ├── UE5_QUICKSTART.md       ← Este arquivo
│   ├── UE5_CPP_EXAMPLES.h      ← Header C++
│   └── UE5_CPP_EXAMPLES.cpp    ← Implementação C++
│
└── UmbraEternumUE/             ← Seu projeto UE5
    └── Source/
        └── YourProject/
            ├── UmbraGameInstance.h   ← Copiar aqui
            └── UmbraGameInstance.cpp ← Copiar aqui
```

---

## 🚀 RESUMO DE 5 MINUTOS

```bash
1. Criar projeto UE5 Third Person C++
2. Instalar VaRest via Marketplace
3. Copiar arquivos .h e .cpp
4. Adicionar VaRest ao Build.cs
5. Compilar
6. Configurar Game Instance
7. Criar Widget Login
8. Testar!
```

---

**Tempo Total**: ~5-10 minutos  
**Dificuldade**: ⭐⭐ (Fácil/Médio)  
**Resultado**: Login funcional conectado às APIs! 🎉

---

**Criado**: 2025-10-14  
**Versão**: 1.0  
**Para**: UmbraEternum v1.3.0

🎮 **Bom desenvolvimento!** 🚀

