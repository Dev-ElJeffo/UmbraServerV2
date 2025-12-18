# 🎮 **GUIA: Widget HUD Principal do Jogador**

## 🎯 **OBJETIVO**

Criar um widget HUD principal que será exibido durante o jogo, mostrando:
- **Barras de HP e MP** (no canto da tela)
- **Nome e título do personagem** (acima da cabeça do personagem em 3D)

Este widget será expandido no futuro com minimapa, opções, skills, etc.

---

## 📋 **ESTRUTURA**

### **1. Widget Principal (WBP_PlayerHUD)**
- Barras de HP/MP no canto da tela
- Será expandido no futuro

### **2. Widget 3D (WBP_PlayerNameplate)**
- Nome e título acima da cabeça do personagem
- Widget em espaço 3D (World Space)

---

## ✅ **PASSO 1: Criar Widget Principal (WBP_PlayerHUD)**

### **1.1. Criar o Widget Blueprint**

1. **No Content Browser**, vá para `Content/Widgets/HUD/`
2. **Crie um novo Widget Blueprint** chamado `WBP_PlayerHUD`
3. **Abra o widget**

### **1.2. Estrutura do Widget**

**Hierarquia:**
```
Canvas Panel (Root)
├─ Horizontal Box: HB_StatsBars
│   ├─ Vertical Box: VB_HP
│   │   ├─ Text Block: TB_HP_Label ("HP")
│   │   └─ Progress Bar: PB_HP
│   └─ Vertical Box: VB_MP
│       ├─ Text Block: TB_MP_Label ("MP")
│       └─ Progress Bar: PB_MP
```

### **1.3. Configurar Barras de HP/MP**

**Para a barra de HP (`PB_HP`):**
- **Anchors:** Top Left
- **Position:** X: 20, Y: 20
- **Size:** Width: 200, Height: 20
- **Fill Color:** Vermelho (R: 255, G: 0, B: 0, A: 255)
- **Percent:** 1.0 (será atualizado via código)

**Para a barra de MP (`PB_MP`):**
- **Anchors:** Top Left
- **Position:** X: 20, Y: 50
- **Size:** Width: 200, Height: 20
- **Fill Color:** Azul (R: 0, G: 100, B: 255, A: 255)
- **Percent:** 1.0 (será atualizado via código)

**Text Blocks (opcional, para mostrar valores):**
- **TB_HP_Label:** "HP: 100/100"
- **TB_MP_Label:** "MP: 50/50"

---

## ✅ **PASSO 2: Criar Widget 3D para Nome/Título (WBP_PlayerNameplate)**

### **2.1. Criar o Widget Blueprint**

1. **No Content Browser**, vá para `Content/Widgets/HUD/`
2. **Crie um novo Widget Blueprint** chamado `WBP_PlayerNameplate`
3. **Abra o widget**

### **2.2. Estrutura do Widget**

**Hierarquia:**
```
Canvas Panel (Root)
└─ Vertical Box: VB_Nameplate
    ├─ Text Block: TB_CharacterName
    └─ Text Block: TB_Title
```

### **2.3. Configurar Text Blocks**

**Para `TB_CharacterName`:**
- **Font Size:** 18
- **Color:** Branco ou Amarelo
- **Text:** "Character Name" (placeholder)
- **Justification:** Center

**Para `TB_Title`:**
- **Font Size:** 14
- **Color:** Cinza claro ou Amarelo claro
- **Text:** "Title" (placeholder)
- **Justification:** Center
- **Visibility:** Collapsed (será mostrado apenas se houver título)

---

## ✅ **PASSO 3: Criar Funções no WBP_PlayerHUD**

### **3.1. Função: UpdateHPBar**

**Inputs:**
- `CurrentHP` (Integer)
- `MaxHP` (Integer)

**Lógica:**
```
[UpdateHPBar]
    Inputs: CurrentHP, MaxHP
    ↓
[Divide] (CurrentHP / MaxHP)
    A: CurrentHP (convertido para Float)
    B: MaxHP (convertido para Float)
    ↓
[Set Percent] (PB_HP)
    Percent: Resultado da divisão
    ↓
[Format Text] (opcional, para mostrar valores)
    Format: "HP: {CurrentHP}/{MaxHP}"
    ↓
[Set Text] (TB_HP_Label)
    Text: Resultado do Format Text
```

### **3.2. Função: UpdateMPBar**

**Inputs:**
- `CurrentMP` (Integer)
- `MaxMP` (Integer)

**Lógica:**
```
[UpdateMPBar]
    Inputs: CurrentMP, MaxMP
    ↓
[Divide] (CurrentMP / MaxMP)
    A: CurrentMP (convertido para Float)
    B: MaxMP (convertido para Float)
    ↓
[Set Percent] (PB_MP)
    Percent: Resultado da divisão
    ↓
[Format Text] (opcional)
    Format: "MP: {CurrentMP}/{MaxMP}"
    ↓
[Set Text] (TB_MP_Label)
    Text: Resultado do Format Text
```

### **3.3. Função: InitializeHUD**

**Lógica:**
```
[InitializeHUD]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info] (função do GameInstance)
    ↓
[UpdateHPBar]
    CurrentHP: CurrentCharacterInfo.CurrentHealth
    MaxHP: CurrentCharacterInfo.MaxHealthTotal
    ↓
[UpdateMPBar]
    CurrentMP: CurrentCharacterInfo.CurrentMana
    MaxMP: CurrentCharacterInfo.MaxManaTotal
```

---

## ✅ **PASSO 4: Criar Funções no WBP_PlayerNameplate**

### **4.1. Função: UpdateNameplate**

**Inputs:**
- `CharacterName` (String)
- `TitleName` (String)

**Lógica:**
```
[UpdateNameplate]
    Inputs: CharacterName, TitleName
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName
    ↓
[Is Empty] (TitleName)
    ↓
[Branch]
    True (vazio) → [Set Visibility] (TB_Title, Collapsed)
    False (tem título) → [Set Text] (TB_Title, TitleName)
                      → [Set Visibility] (TB_Title, Visible)
```

---

## ✅ **PASSO 5: Criar Componente Widget 3D no Personagem**

### **5.1. No Blueprint do Personagem (BP_Player ou similar)**

1. **Adicione um `Widget Component`**
   - Nome: `NameplateWidgetComponent`
   - **Widget Class:** `WBP_PlayerNameplate`
   - **Draw Size:** X: 200, Y: 100
   - **Space:** World (não Screen)
   - **Pivot:** Center, Bottom

2. **Configure a posição:**
   - **Attach to:** Head ou Mesh (onde a cabeça está)
   - **Relative Location:** X: 0, Y: 0, Z: 180 (acima da cabeça, ajuste conforme necessário)
   - **Relative Rotation:** Yaw: 0 (olhando para frente)

3. **No Event BeginPlay:**

   **IMPORTANTE:** O `CurrentCharacterInfo` pode não estar carregado quando o `Event BeginPlay` executa. Você precisa conectar o nameplate ao delegate `OnCharacterInfoLoaded` para atualização automática.

   **Crie um Custom Event `UpdateNameplateFromCharacterInfo`:**
   - **Input:** `CharacterInfo` (FUmbraCharacterInfo)
   - **Lógica:** Obtém o widget do `NameplateWidgetComponent`, faz cast para `WBP_PlayerNameplate`, quebra o `CharacterInfo` e chama `Update Nameplate`

   **No `Event BeginPlay`:**
   ```
   [Event BeginPlay]
       ↓
   [Get Game Instance]
       ↓
   [Cast to Umbra Game Instance]
       ↓
   [Assign OnCharacterInfoLoaded] (delegate do GameInstance)
       ↓
   [UpdateNameplateFromCharacterInfo] (seu Custom Event)
       ↓
   [Get Current Character Info] (tentativa de atualização inicial)
       ↓
   [Break Umbra Character Info]
       ↓
   [Is Empty] (Character Name)
       ↓
   [Branch]
       False (tem nome) → [UpdateNameplateFromCharacterInfo]
                             CharacterInfo: CurrentCharacterInfo
       True (vazio) → (não faz nada, aguarda o delegate)
   ```

   **Isso garante que:**
   - O nameplate será atualizado automaticamente quando o `CurrentCharacterInfo` for carregado (via delegate)
   - Se os dados já estiverem carregados quando o personagem spawnar, o nameplate será atualizado imediatamente

---

## ✅ **PASSO 6: Adicionar HUD ao Lvl_Tutorial**

### **6.1. No Level Blueprint ou Game Mode**

**No `Event BeginPlay` do Level Blueprint:**

```
[Event BeginPlay]
    ↓
[Create Widget]
    Class: WBP_PlayerHUD
    ↓
[Add to Viewport]
    ↓
[Initialize HUD] (no widget criado)
```

**OU criar um Game Mode Blueprint que faz isso automaticamente.**

---

## ✅ **PASSO 7: Atualizar Barras em Tempo Real**

### **7.1. Criar Função de Atualização**

**No `WBP_PlayerHUD`, crie uma função chamada `UpdateStatsFromCharacterInfo`:**

**Inputs:**
- `CharacterInfo` (FUmbraCharacterInfo)

**Lógica:**
```
[UpdateStatsFromCharacterInfo]
    Input: CharacterInfo
    ↓
[UpdateHPBar]
    CurrentHP: CharacterInfo.CurrentHealth
    MaxHP: CharacterInfo.MaxHealthTotal
    ↓
[UpdateMPBar]
    CurrentMP: CharacterInfo.CurrentMana
    MaxMP: CharacterInfo.MaxManaTotal
```

### **7.2. Conectar ao Delegate do GameInstance**

**No `Event Construct` do `WBP_PlayerHUD`:**

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Bind Event to OnCharacterInfoLoaded] (do GameInstance)
    ↓
[Create Custom Event: OnCharacterInfoUpdated]
    Input: CharacterInfo (FUmbraCharacterInfo)
    ↓
[UpdateStatsFromCharacterInfo]
    CharacterInfo: Input CharacterInfo
```

**IMPORTANTE:** 
- No Blueprint, você precisa **conectar o delegate** `OnCharacterInfoLoaded` do GameInstance ao seu Custom Event `OnCharacterInfoUpdated`
- Isso fará com que o HUD seja atualizado **automaticamente** sempre que o `CurrentCharacterInfo` for atualizado no GameInstance

### **7.3. Como Conectar o Delegate no Blueprint**

1. **No `Event Construct` do `WBP_PlayerHUD`:**
   - Obtenha o `Game Instance`
   - Faça `Cast to Umbra Game Instance`
   - **Arraste o pin `OnCharacterInfoLoaded`** (delegate) do GameInstance
   - **Selecione `Assign`** (não `Call`)
   - Isso abrirá um menu onde você pode criar um novo Custom Event ou selecionar um existente

2. **Crie um Custom Event chamado `OnCharacterInfoUpdated`:**
   - **Input:** `CharacterInfo` (FUmbraCharacterInfo)
   - **Lógica:** Chame `UpdateStatsFromCharacterInfo` passando o `CharacterInfo` recebido

3. **Conecte o delegate:**
   - O pin `OnCharacterInfoLoaded` do GameInstance deve estar conectado ao Custom Event `OnCharacterInfoUpdated`

### **7.4. Inicialização Inicial**

**No `Event Construct`, após conectar o delegate, também chame a atualização inicial:**

```
[Event Construct]
    ↓
[... conectar delegate ...]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info]
    ↓
[UpdateStatsFromCharacterInfo]
    CharacterInfo: CurrentCharacterInfo obtido
```

**Isso garante que o HUD seja atualizado mesmo se o `CurrentCharacterInfo` já estiver carregado quando o widget for criado.**

---

## 📋 **RESUMO**

1. ✅ Criar `WBP_PlayerHUD` com barras de HP/MP
2. ✅ Criar `WBP_PlayerNameplate` para nome/título
3. ✅ Adicionar `Widget Component` no personagem para o nameplate
4. ✅ Criar funções de atualização (`UpdateHPBar`, `UpdateMPBar`, `UpdateStatsFromCharacterInfo`)
5. ✅ Adicionar HUD ao nível
6. ✅ **Conectar delegate `OnCharacterInfoLoaded` do GameInstance ao HUD** (atualização automática)
7. ✅ Criar Custom Event `OnCharacterInfoUpdated` para receber atualizações do delegate

---

## ⚠️ **NOTAS**

- O `Widget Component` precisa estar configurado como **World Space** (não Screen Space)
- Ajuste a posição Z do nameplate conforme a altura do personagem
- **As barras são atualizadas automaticamente via delegate `OnCharacterInfoLoaded`** quando o `CurrentCharacterInfo` é atualizado no GameInstance
- Não é necessário usar Timers - o sistema é reativo e eficiente
- No futuro, este widget será expandido com minimapa, skills, etc.

## 📚 **GUIAS RELACIONADOS**

- **`GUIA_CONECTAR_DELEGATE_CHARACTERINFO.md`** - Guia completo sobre como conectar o delegate `OnCharacterInfoLoaded` ao HUD
- **`CORRECAO_NAMEPLATE_CHARACTERNAME_VAZIO.md`** - Correção para quando o nome do personagem não aparece no nameplate (CharacterName vazio)

## ⚠️ **PROBLEMA COMUM: Nome Não Aparece no Nameplate**

Se o nome do personagem não aparecer no nameplate, o problema geralmente é que o `Event BeginPlay` executa **antes** do `CurrentCharacterInfo` ser carregado. 

**Solução:** Conecte o nameplate ao delegate `OnCharacterInfoLoaded` do GameInstance para atualização automática quando os dados chegarem. Veja `CORRECAO_NAMEPLATE_CHARACTERNAME_VAZIO.md` para detalhes.

---

**FIM DO GUIA**

