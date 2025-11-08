# 🔄 **GUIA: Trocar BP_ThirdPersonCharacter para BP_Player**

## 🎯 **OBJETIVO:**

Encontrar e substituir todas as referências a `BP_ThirdPersonCharacter` por `BP_Player` no projeto.

---

## 🔍 **LOCAIS ONDE PODE ESTAR CONFIGURADO:**

### **1. Project Settings (Configuração Global)**

**LOCALIZAÇÃO:** `Edit` → `Project Settings` → `Game` → `Default Modes`

**VERIFICAÇÃO:**
1. Abrir `Project Settings`
2. Ir em `Game` → `Default Modes`
3. Verificar `Default Pawn Class`
4. **SE ESTIVER COMO `BP_ThirdPersonCharacter`:**
   - Trocar para `BP_Player`
   - Salvar

---

### **2. Game Mode (Blueprint ou C++)**

**LOCAIS POSSÍVEIS:**
- `BP_GameMode` (Blueprint)
- `UmbraEternumUEGameMode` (C++)
- Game Mode configurado no nível

**VERIFICAÇÃO:**

#### **A. Game Mode Blueprint:**
1. Abrir o Game Mode Blueprint (geralmente `BP_GameMode` ou similar)
2. Verificar `Default Pawn Class`
3. **SE ESTIVER COMO `BP_ThirdPersonCharacter`:**
   - Trocar para `BP_Player`
   - Compilar e salvar

#### **B. Game Mode C++:**
1. Abrir `UmbraEternumUEGameMode.h` ou `.cpp`
2. Procurar por `DefaultPawnClass`
3. **SE ESTIVER REFERENCIANDO `BP_ThirdPersonCharacter`:**
   - Trocar para `BP_Player`
   - Recompilar

---

### **3. World Settings (Configuração do Nível)**

**LOCALIZAÇÃO:** No nível atual → `Window` → `World Settings`

**VERIFICAÇÃO:**
1. Abrir o nível atual
2. Abrir `World Settings` (`Window` → `World Settings`)
3. Verificar `Game Mode Override`
4. Verificar `Default Pawn Class` no Game Mode Override
5. **SE ESTIVER COMO `BP_ThirdPersonCharacter`:**
   - Trocar para `BP_Player`
   - Salvar o nível

---

### **4. Config Files (.ini)**

**LOCAIS POSSÍVEIS:**
- `Config/DefaultEngine.ini`
- `Config/DefaultGame.ini`
- `Config/DefaultEditor.ini`

**VERIFICAÇÃO:**
1. Abrir `Config/DefaultEngine.ini`
2. Procurar por `BP_ThirdPersonCharacter` ou `ThirdPersonCharacter`
3. **SE ENCONTRAR:**
   - Trocar para `BP_Player`
   - Salvar

---

### **5. Blueprints que Referenciam**

**VERIFICAÇÃO:**
1. Usar `Content Browser` → `Search`
2. Buscar por `BP_ThirdPersonCharacter`
3. Verificar todos os Blueprints que referenciam
4. Trocar referências para `BP_Player`

---

## ✅ **PASSO A PASSO PARA TROCAR:**

### **PASSO 1: Project Settings**

1. **Abrir `Edit` → `Project Settings`**
2. **Ir em `Game` → `Default Modes`**
3. **Verificar `Default Pawn Class`:**
   - Se estiver como `BP_ThirdPersonCharacter`
   - Clicar no dropdown
   - Selecionar `BP_Player`
4. **Salvar** (`Ctrl+S`)

---

### **PASSO 2: Game Mode Blueprint**

1. **Abrir o Game Mode Blueprint** (geralmente `BP_GameMode` ou similar)
2. **Verificar `Default Pawn Class` na aba `Details`:**
   - Se estiver como `BP_ThirdPersonCharacter`
   - Clicar no dropdown
   - Selecionar `BP_Player`
3. **Compilar** (`Compile`)
4. **Salvar** (`Ctrl+S`)

---

### **PASSO 3: World Settings**

1. **Abrir o nível atual**
2. **Abrir `World Settings`** (`Window` → `World Settings`)
3. **Verificar `Game Mode Override`:**
   - Se houver um Game Mode configurado
   - Verificar `Default Pawn Class`
   - Se estiver como `BP_ThirdPersonCharacter`
   - Trocar para `BP_Player`
4. **Salvar o nível** (`Ctrl+S`)

---

### **PASSO 4: Config Files**

1. **Abrir `Config/DefaultEngine.ini`**
2. **Procurar por `BP_ThirdPersonCharacter` ou `ThirdPersonCharacter`:**
   ```ini
   # Procurar por linhas como:
   DefaultPawnClass=/Game/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C
   ```
3. **SE ENCONTRAR:**
   - Trocar para:
   ```ini
   DefaultPawnClass=/Game/Blueprints/BP_Player.BP_Player_C
   ```
4. **Salvar** (`Ctrl+S`)

---

### **PASSO 5: Buscar em Todos os Blueprints**

1. **Abrir `Content Browser`**
2. **Usar `Search` (lupa)**
3. **Buscar por `BP_ThirdPersonCharacter`**
4. **Verificar todos os resultados:**
   - Abrir cada Blueprint que referenciar
   - Trocar referências para `BP_Player`
   - Compilar e salvar

---

## 🔍 **BUSCA AUTOMÁTICA NO UNREAL:**

### **MÉTODO 1: Content Browser Search**

1. **Abrir `Content Browser`**
2. **Clicar na lupa de busca**
3. **Digitar:** `BP_ThirdPersonCharacter`
4. **Verificar todos os resultados**
5. **Trocar referências manualmente**

---

### **MÉTODO 2: Reference Viewer**

1. **Selecionar `BP_ThirdPersonCharacter` no Content Browser**
2. **Botão direito → `Reference Viewer`**
3. **Verificar todos os Blueprints que referenciam**
4. **Abrir cada um e trocar**

---

### **MÉTODO 3: Asset Audit**

1. **Selecionar `BP_ThirdPersonCharacter` no Content Browser**
2. **Botão direito → `Asset Audit`**
3. **Verificar `Referencers` (quem referencia)**
4. **Abrir cada um e trocar**

---

## 📋 **CHECKLIST DE VERIFICAÇÃO:**

### **Project Settings:**
- [ ] `Default Pawn Class` está como `BP_Player`
- [ ] Salvo

### **Game Mode Blueprint:**
- [ ] `Default Pawn Class` está como `BP_Player`
- [ ] Compilado
- [ ] Salvo

### **World Settings:**
- [ ] `Game Mode Override` verificado
- [ ] `Default Pawn Class` está como `BP_Player` (se configurado)
- [ ] Nível salvo

### **Config Files:**
- [ ] `DefaultEngine.ini` verificado
- [ ] `DefaultGame.ini` verificado
- [ ] Todas as referências trocadas

### **Blueprints:**
- [ ] Busca por `BP_ThirdPersonCharacter` realizada
- [ ] Todas as referências trocadas
- [ ] Todos os Blueprints compilados e salvos

---

## 🧪 **TESTE APÓS TROCAR:**

1. **Fechar e reabrir o Unreal Editor**
2. **Compilar o projeto** (`Compile`)
3. **Conectar um client**
4. **Verificar logs:**
   - Deve aparecer: "🔵 [BP_Player BeginPlay] INÍCIO"
   - Deve aparecer: "🔵 [BP_Player BeginPlay] Timer SavePositionTimer iniciado"

---

## ⚠️ **IMPORTANTE:**

### **APÓS TROCAR:**

1. **Recompilar o projeto completo:**
   - `Build` → `Compile` (ou `Ctrl+Alt+F11`)

2. **Verificar se não há erros:**
   - Se houver erros, verificar se `BP_Player` existe
   - Verificar se está compilado corretamente

3. **Testar em PIE (Play In Editor):**
   - Verificar se o `BP_Player` está sendo spawnado
   - Verificar se o `BeginPlay` está executando

---

## 🔧 **SE AINDA NÃO FUNCIONAR:**

### **VERIFICAÇÃO ADICIONAL:**

1. **Verificar se `BP_Player` existe:**
   - Abrir `Content Browser`
   - Buscar por `BP_Player`
   - Verificar se existe e está compilado

2. **Verificar se `BP_Player` está no caminho correto:**
   - Deve estar em `/Game/Blueprints/BP_Player`
   - Ou no caminho configurado no Project Settings

3. **Verificar se há erros de compilação:**
   - Abrir `BP_Player`
   - Verificar se compila sem erros

---

## 📊 **RESUMO:**

**LOCAIS PRINCIPAIS PARA VERIFICAR:**
1. ✅ **Project Settings** → `Game` → `Default Modes` → `Default Pawn Class`
2. ✅ **Game Mode Blueprint** → `Default Pawn Class`
3. ✅ **World Settings** → `Game Mode Override` → `Default Pawn Class`
4. ✅ **Config Files** → `DefaultEngine.ini`, `DefaultGame.ini`
5. ✅ **Content Browser** → Buscar por `BP_ThirdPersonCharacter`

**APÓS TROCAR:**
- Recompilar o projeto
- Testar em PIE
- Verificar se `BP_Player` está sendo spawnado

---

**Status:** 🔄 **TROCAR TODAS AS REFERÊNCIAS PARA BP_PLAYER**

