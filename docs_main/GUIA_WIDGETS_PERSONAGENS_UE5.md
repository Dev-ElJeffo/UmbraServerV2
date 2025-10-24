# 🎮 WIDGETS DE PERSONAGENS - GUIA RÁPIDO UE5

**Data**: 16/10/2025  
**Objetivo**: Criar interface completa de seleção/criação de personagens  
**Tempo Estimado**: 45-60 minutos

---

## 📋 WIDGETS A CRIAR

```
WBP_CharacterSelection (Widget principal)
  ├── WBP_CharacterItem (Card de personagem individual)
  └── WBP_CreateCharacter (Diálogo de criação)
```

---

## 🎨 WIDGET 1: WBP_CharacterSelection

### Layout (Designer):

```
Canvas Panel
  └── Vertical Box (preencher tela)
      ├── Text Block: "SELECIONE SEU PERSONAGEM" (título)
      │
      ├── Scroll Box (Character_List)
      │   └── Vertical Box (Dynamic - será populado via BP)
      │
      ├── Horizontal Box (botões)
      │   ├── Button: BTN_CreateNew ("+ CRIAR PERSONAGEM")
      │   └── Button: BTN_Logout ("Logout")
      │
      └── Text Block: TXT_Status (mensagens)
```

### Variáveis:

```cpp
// Designer Variables (Is Variable = TRUE)
- UScrollBox* Character_List
- UButton* BTN_CreateNew
- UButton* BTN_Logout
- UTextBlock* TXT_Status
- UVerticalBox* CharacterContainer  // Dentro do Scroll Box

// Blueprint Variables
- UmbraGameInstance: MyGameInstance (Object Reference)
- TArray<UWBPCharacterItem*>: CharacterWidgets
```

---

### Event Graph:

#### **Event Construct**:

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Set] MyGameInstance
    ↓
[Is Valid?] MyGameInstance
    ↓ Valid
[Bind Event to OnCharacterListLoaded]
    • Event: [Custom Event] HandleCharacterListLoaded
    ↓
[Bind Event to OnCharacterListFailed]
    • Event: [Custom Event] HandleCharacterListFailed
    ↓
[Bind Event to OnCharacterCreated]
    • Event: [Custom Event] HandleCharacterCreated
    ↓
[Bind Event to OnCharacterSelected]
    • Event: [Custom Event] HandleCharacterSelected
    ↓
[Bind Event to OnCharacterDeleted]
    • Event: [Custom Event] HandleCharacterDeleted
    ↓
[Set Input Mode UI Only]
    • Widget to Focus: SELF
    ↓
[Set Show Mouse Cursor]: TRUE
    ↓
[Load Character List]
    • Target: MyGameInstance
    ↓
[Set Text] TXT_Status
    • Text: "Carregando personagens..."
    ↓
[Set Color] TXT_Status
    • Color: Yellow
```

---

#### **Custom Event: HandleCharacterListLoaded**:

```
[Event HandleCharacterListLoaded]
    ↓
[Print String] "DEBUG: Lista carregada!"
    ↓
[Clear Children]
    • Target: CharacterContainer
    ↓
[Get Character List]
    • Target: MyGameInstance
    ↓ (retorna TArray<FUmbraPlayerData>)
[For Each Loop]
    • Array: (character list)
    ↓ Loop Body
    [Create Widget]
        • Class: WBP_CharacterItem
        • Owning Player: Get Player Controller (0)
        ↓
    [Set Character Data] (função do WBP_CharacterItem)
        • Character: (loop element)
        ↓
    [Add Child to Vertical Box]
        • Target: CharacterContainer
        • Content: (widget criado)
        ↓
    [Add to Array]
        • Target: CharacterWidgets
        • Item: (widget criado)
    ↓
[For Each Completed]
    ↓
[Get] Character List → [Length]
    ↓
[Set Text] TXT_Status
    • Text: Format("{0} personagem(ns) encontrado(s)", Length)
    ↓
[Set Color] TXT_Status
    • Color: Green
```

---

#### **Custom Event: HandleCharacterListFailed**:

**Parâmetros**: `ErrorMessage` (FString)

```
[Event HandleCharacterListFailed] (ErrorMessage)
    ↓
[Print String] ErrorMessage
    • Color: Red
    ↓
[Set Text] TXT_Status
    • Text: ErrorMessage
    ↓
[Set Color] TXT_Status
    • Color: Red
```

---

#### **Custom Event: HandleCharacterCreated**:

**Parâmetros**: `NewCharacter` (FUmbraPlayerData)

```
[Event HandleCharacterCreated] (NewCharacter)
    ↓
[Format Text]
    • Format: "✓ Personagem {0} criado com sucesso!"
    • Args: NewCharacter → Character Name
    ↓
[Set Text] TXT_Status
    • Text: (formatted text)
    ↓
[Set Color] TXT_Status
    • Color: Green
    ↓
[Delay] 1.0
    ↓
[Load Character List]
    • Target: MyGameInstance
```

---

#### **Custom Event: HandleCharacterSelected**:

**Parâmetros**: `SelectedCharacter` (FUmbraPlayerData)

```
[Event HandleCharacterSelected] (SelectedCharacter)
    ↓
[Print String]
    • Text: Format("Entrando no jogo com {0}...", SelectedCharacter.CharacterName)
    ↓
[Set Text] TXT_Status
    • Text: "Carregando mundo..."
    ↓
[Set Color] TXT_Status
    • Color: Cyan
    ↓
[Delay] 1.5
    ↓
[Remove from Parent]
    • Target: Self
    ↓
[Open Level]
    • Level Name: "GameWorld" (ou seu mapa de jogo)
```

---

#### **Custom Event: HandleCharacterDeleted**:

**Parâmetros**: `Message` (FString)

```
[Event HandleCharacterDeleted] (Message)
    ↓
[Set Text] TXT_Status
    • Text: Message
    ↓
[Set Color] TXT_Status
    • Color: Orange
```

---

#### **BTN_CreateNew OnClicked**:

```
[OnClicked] BTN_CreateNew
    ↓
[Create Widget]
    • Class: WBP_CreateCharacter
    • Owning Player: Get Player Controller (0)
    ↓
[Add to Viewport]
    • Target: (widget criado)
    • Z-Order: 999 (aparecer na frente)
```

---

#### **BTN_Logout OnClicked**:

```
[OnClicked] BTN_Logout
    ↓
[Show Confirmation Dialog] (opcional)
    • Text: "Deseja realmente sair?"
    ↓ Confirmed
[Get] MyGameInstance
    ↓
[Logout]
    • Target: MyGameInstance
    ↓
[Remove from Parent]
    • Target: Self
    ↓
[Open Level]
    • Level Name: "Lvl_TestAuth" (seu level de login)
```

---

## 🎨 WIDGET 2: WBP_CharacterItem

### Layout (Designer):

```
Border (fundo do card)
  └── Horizontal Box
      ├── Vertical Box (info do personagem)
      │   ├── Text Block: TXT_Name (nome, grande)
      │   ├── Text Block: TXT_Level ("Level: X")
      │   ├── Text Block: TXT_Zone ("Zona: X")
      │   └── Text Block: TXT_XP ("XP: X")
      │
      └── Horizontal Box (ações)
          ├── Button: BTN_Play ("▶ JOGAR")
          └── Button: BTN_Delete ("🗑")
```

### Variáveis:

```cpp
// Designer Variables
- UTextBlock* TXT_Name
- UTextBlock* TXT_Level
- UTextBlock* TXT_Zone
- UTextBlock* TXT_XP
- UButton* BTN_Play
- UButton* BTN_Delete

// Blueprint Variables
- FUmbraPlayerData: CharacterData (Struct)
- UmbraGameInstance: MyGameInstance (Object Reference)
```

---

### Event Graph:

#### **Event Construct**:

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Set] MyGameInstance
```

---

#### **Função: SetCharacterData** (Custom Function):

**Parâmetros**: `Character` (FUmbraPlayerData)

```
[Function: SetCharacterData] (Character)
    ↓
[Set] CharacterData = Character
    ↓
[Set Text] TXT_Name
    • Text: Character → Character Name
    ↓
[Format Text]
    • Format: "Level: {0}"
    • Args: Character → Level
    ↓
[Set Text] TXT_Level
    • Text: (formatted)
    ↓
[Format Text]
    • Format: "Zona: {0}"
    • Args: Character → Current Zone
    ↓
[Set Text] TXT_Zone
    • Text: (formatted)
    ↓
[Format Text]
    • Format: "XP: {0}"
    • Args: Character → Experience
    ↓
[Set Text] TXT_XP
    • Text: (formatted)
```

---

#### **BTN_Play OnClicked**:

```
[OnClicked] BTN_Play
    ↓
[Get] CharacterData → ID
    ↓
[Get] MyGameInstance
    ↓
[Select Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
```

---

#### **BTN_Delete OnClicked**:

```
[OnClicked] BTN_Delete
    ↓
[Get] CharacterData → Character Name
    ↓
[Format Text]
    • Format: "Deseja deletar '{0}'?"
    ↓
[Show Confirmation Dialog]
    • Message: (formatted text)
    • Type: Yes/No
    ↓ Result = Yes
[Get] CharacterData → ID
    ↓
[Get] MyGameInstance
    ↓
[Delete Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
```

---

## 🎨 WIDGET 3: WBP_CreateCharacter

### Layout (Designer):

```
Canvas Panel (fundo escuro translúcido)
  └── Border (painel central)
      └── Vertical Box
          ├── Text Block: "CRIAR NOVO PERSONAGEM" (título)
          │
          ├── Horizontal Box (input)
          │   ├── Text Block: "Nome:"
          │   └── Editable Text: TXT_Name
          │
          ├── Text Block: TXT_Validation (mensagens)
          │
          └── Horizontal Box (botões)
              ├── Button: BTN_Create ("CRIAR")
              └── Button: BTN_Cancel ("CANCELAR")
```

### Variáveis:

```cpp
// Designer Variables
- UEditableText* TXT_Name
- UTextBlock* TXT_Validation
- UButton* BTN_Create
- UButton* BTN_Cancel

// Blueprint Variables
- UmbraGameInstance: MyGameInstance (Object Reference)
```

---

### Event Graph:

#### **Event Construct**:

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Set] MyGameInstance
    ↓
[Set Input Mode UI Only]
    • Widget to Focus: TXT_Name
    ↓
[Set Show Mouse Cursor]: TRUE
    ↓
[Bind Event to OnCharacterCreated]
    • Event: [Custom] HandleCreated
    ↓
[Bind Event to OnCharacterCreateFailed]
    • Event: [Custom] HandleFailed
```

---

#### **TXT_Name OnTextChanged**:

**Parâmetros**: `Text` (FText)

```
[OnTextChanged] TXT_Name (Text)
    ↓
[To String] Text
    ↓
[Length]
    ↓
[Branch] < 3
    ↓ True
    [Set Text] TXT_Validation
        • Text: "⚠ Nome deve ter no mínimo 3 caracteres"
        • Color: Orange
    ↓ False
[Branch] > 20
    ↓ True
    [Set Text] TXT_Validation
        • Text: "⚠ Nome deve ter no máximo 20 caracteres"
        • Color: Orange
    ↓ False
[Regex Match] "^[a-zA-Z0-9_]+$"
    • Input: (text)
    ↓ Matches
    [Set Text] TXT_Validation
        • Text: "✓ Nome válido"
        • Color: Green
    ↓ Not Matches
    [Set Text] TXT_Validation
        • Text: "⚠ Use apenas letras, números e underscore"
        • Color: Red
```

---

#### **BTN_Create OnClicked**:

```
[OnClicked] BTN_Create
    ↓
[Get Text] TXT_Name
    ↓
[To String]
    ↓
[Trim] (remover espaços)
    ↓
[Is Empty?] Branch
    ↓ False
[Get] MyGameInstance
    ↓
[Create Character]
    • Target: MyGameInstance
    • Character Name: (trimmed text)
    ↓
[Set Text] TXT_Validation
    • Text: "Criando personagem..."
    ↓
[Set Color] TXT_Validation
    • Color: Cyan
    ↓
[Set Enabled] BTN_Create
    • Enabled: FALSE
```

---

#### **Custom Event: HandleCreated**:

**Parâmetros**: `NewCharacter` (FUmbraPlayerData)

```
[Event HandleCreated] (NewCharacter)
    ↓
[Print String]
    • Text: Format("✓ {0} criado!", NewCharacter.CharacterName)
    • Color: Green
    ↓
[Delay] 0.5
    ↓
[Remove from Parent]
    • Target: Self
```

---

#### **Custom Event: HandleFailed**:

**Parâmetros**: `ErrorMessage` (FString)

```
[Event HandleFailed] (ErrorMessage)
    ↓
[Set Text] TXT_Validation
    • Text: ErrorMessage
    ↓
[Set Color] TXT_Validation
    • Color: Red
    ↓
[Set Enabled] BTN_Create
    • Enabled: TRUE
```

---

#### **BTN_Cancel OnClicked**:

```
[OnClicked] BTN_Cancel
    ↓
[Remove from Parent]
    • Target: Self
```

---

## 🔄 INTEGRAÇÃO COM LOGIN

### Modificar WBP_Login:

No **Custom Event OnLoginSuccess_Custom**, ao invés de ir direto para Dashboard:

```
[Event OnLoginSuccess_Custom]
    ↓
[Remove from Parent]
        • Target: Self
    ↓
[Create Widget]
    • Class: WBP_CharacterSelection  ← AQUI!
    • Owning Player: Get Player Controller (0)
    ↓
[Add to Viewport]
    • Target: (widget criado)
```

---

## 🧪 CHECKLIST DE TESTE

### WBP_CharacterSelection:
- [ ] Aparece após login
- [ ] Lista personagens existentes
- [ ] Botão "Criar Personagem" abre WBP_CreateCharacter
- [ ] TXT_Status mostra feedback correto
- [ ] Botão Logout funciona

### WBP_CharacterItem:
- [ ] Mostra nome, level, zona, XP
- [ ] Botão "Jogar" seleciona personagem
- [ ] Botão "Deletar" pede confirmação
- [ ] Após deletar, lista atualiza

### WBP_CreateCharacter:
- [ ] Aparece como overlay
- [ ] Validação em tempo real
- [ ] Não permite nomes inválidos
- [ ] Após criar, widget fecha
- [ ] Lista de personagens atualiza automaticamente

---

## 🎨 ESTILOS SUGERIDOS

### WBP_CharacterSelection:
```
Fundo: Gradient escuro (#1a1a2e → #0f0f23)
Título: Fonte grande (48px), branco
Cards: Fundo semi-transparente (#2d2d44, 90%)
Botões: Azul (#667eea), hover (+10% brilho)
Status: Amarelo (loading), Verde (sucesso), Vermelho (erro)
```

### WBP_CharacterItem:
```
Card Border: 2px, azul claro (#667eea)
Nome: Fonte grande (32px), branco
Info: Fonte média (16px), cinza claro
Botão Play: Verde (#27ae60), destaque
Botão Delete: Vermelho (#e74c3c), pequeno
Hover: Aumentar scale 1.05, sombra
```

### WBP_CreateCharacter:
```
Overlay: Fundo preto 70% opacidade
Painel: Branco, sombra forte
Input: Borda azul no focus
Validação: Ícones + cor (✓ verde, ⚠ laranja, ❌ vermelho)
Botões: Criar (azul), Cancelar (cinza)
```

---

## 📊 FLUXO COMPLETO

```
┌─────────────┐
│   LOGIN     │
│  (Success)  │
└──────┬──────┘
       ↓
┌──────────────────────────────┐
│ WBP_CharacterSelection       │
│  - Carrega lista             │
│  - Mostra cards              │
└──────┬───────────────────────┘
       │
       ├─→ [+ Criar] → WBP_CreateCharacter
       │                  ↓
       │              [Cria personagem]
       │                  ↓
       │              [Fecha widget]
       │                  ↓
       │              [Lista atualiza]
       │
       ├─→ [Jogar] → Select Character API
       │                  ↓
       │              [Open Level: GameWorld]
       │
       └─→ [Delete] → Confirma?
                          ↓ Sim
                      [Delete API]
                          ↓
                      [Lista atualiza]
```

---

## ⏱️ TEMPO ESTIMADO

```
WBP_CharacterSelection:   25 min  ⭐⭐⭐
WBP_CharacterItem:        15 min  ⭐⭐
WBP_CreateCharacter:      15 min  ⭐⭐
Integração c/ Login:       5 min  ⭐
Testes:                   10 min  ⭐
──────────────────────────────────
TOTAL:                    70 min  ⭐⭐⭐
```

---

## 🚀 RESULTADO FINAL

Após completar, você terá:

✅ Sistema completo de seleção de personagens  
✅ Criação de personagens com validação  
✅ Visualização de stats (level, XP, zona)  
✅ Seleção para entrar no jogo  
✅ Deleção com confirmação  
✅ Feedback visual em tempo real  
✅ Integração perfeita com login/logout  

---

## 📝 PRÓXIMO PASSO

Após criar os widgets:

1. **Compile o projeto UE5**
2. **Teste o fluxo completo**: Login → Lista → Criar → Selecionar → Jogar
3. **Ajuste estilos** conforme necessário
4. **Implemente gameplay**: Use `GetActiveCharacter()` no Level para spawnar o personagem correto

---

**🎮 Widgets prontos = Sistema de personagens 100% funcional!**

