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
   ```
   [Event BeginPlay]
       ↓
   [Get Widget] (do NameplateWidgetComponent)
       ↓
   [Cast to WBP Player Nameplate]
       ↓
   [Get Game Instance]
       ↓
   [Cast to Umbra Game Instance]
       ↓
   [Get Current Character Info] (função do GameInstance)
       ↓
   [Update Nameplate] (no widget)
       CharacterName: CurrentCharacterInfo.CharacterName
       TitleName: CurrentCharacterInfo.TitleName
   ```

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

### **7.1. Criar Evento de Atualização**

**No `WBP_PlayerHUD`, crie um Custom Event:**

```
[Custom Event: OnStatsUpdated]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info] (função do GameInstance)
    ↓
[UpdateHPBar]
[UpdateMPBar]
```

### **7.2. Chamar o Evento**

**Você pode chamar esse evento:**
- Quando o personagem recebe dano/cura
- Quando o personagem usa mana
- Em um Timer (a cada 0.1 segundos, por exemplo)
- Quando o GameInstance atualiza CurrentCharacterInfo

---

## 📋 **RESUMO**

1. ✅ Criar `WBP_PlayerHUD` com barras de HP/MP
2. ✅ Criar `WBP_PlayerNameplate` para nome/título
3. ✅ Adicionar `Widget Component` no personagem para o nameplate
4. ✅ Criar funções de atualização
5. ✅ Adicionar HUD ao nível
6. ✅ Conectar com `CurrentCharacterInfo` do GameInstance

---

## ⚠️ **NOTAS**

- O `Widget Component` precisa estar configurado como **World Space** (não Screen Space)
- Ajuste a posição Z do nameplate conforme a altura do personagem
- As barras podem ser atualizadas via Timer ou eventos
- No futuro, este widget será expandido com minimapa, skills, etc.

---

**FIM DO GUIA**

