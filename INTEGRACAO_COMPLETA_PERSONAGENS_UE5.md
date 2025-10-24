# 🎮 INTEGRAÇÃO COMPLETA - PERSONAGENS UE5

**Data**: 16/10/2025  
**Objetivo**: Implementar sistema completo de personagens na Unreal Engine  
**Tempo Estimado**: 2-3 horas  
**Dificuldade**: ⭐⭐⭐⭐ Intermediário/Avançado

---

## 📋 ÍNDICE

1. [Fase 1: Atualizar C++ - Parse de Stats](#fase-1-atualizar-c-parse-de-stats)
2. [Fase 2: Widget de Seleção](#fase-2-widget-de-seleção)
3. [Fase 3: Widget de Item de Personagem](#fase-3-widget-de-item)
4. [Fase 4: Widget de Criar Personagem](#fase-4-widget-de-criar)
5. [Fase 5: Integração com Login](#fase-5-integração-com-login)
6. [Fase 6: Testes Completos](#fase-6-testes)

---

## 🎯 O QUE VOCÊ VAI TER NO FINAL

```
Login → Lista de Personagens → Selecionar OU Criar
                                    ↓              ↓
                              Carregar Level   Novo Personagem
                                    ↓              ↓
                              Jogar!         Adiciona à Lista
```

---

# 📦 FASE 1: ATUALIZAR C++ - PARSE DE STATS

## 🎯 Objetivo

Atualizar `UmbraGameInstance.cpp` para parsear os **stats** dos personagens que vêm das APIs.

---

## 📝 PASSO 1.1: Atualizar Parse de Personagens

Abra: `D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Core\UmbraGameInstance.cpp`

### Localize a função `OnCharacterListRequestComplete`:

**Procure por** (aprox. linha 440-470):

```cpp
void UUmbraGameInstance::OnCharacterListRequestComplete(UVaRestRequestJSON* Request)
{
    // ... código existente ...
    
    for (UVaRestJsonValue* PlayerValue : PlayersArray)
    {
        UVaRestJsonObject* PlayerObject = PlayerValue->AsObject();
        
        FUmbraPlayerData PlayerData;
        PlayerData.ID = PlayerObject->GetIntegerField(TEXT("player_id"));
        PlayerData.CharacterName = PlayerObject->GetStringField(TEXT("character_name"));
        PlayerData.Level = PlayerObject->GetIntegerField(TEXT("level"));
        PlayerData.Experience = PlayerObject->GetIntegerField(TEXT("experience"));
        PlayerData.CurrentZone = PlayerObject->GetStringField(TEXT("current_zone"));

        // Posição
        if (PlayerObject->HasField(TEXT("position")))
        {
            UVaRestJsonObject* PosObject = PlayerObject->GetObjectField(TEXT("position"));
            float PosX = PosObject->GetNumberField(TEXT("x"));
            float PosY = PosObject->GetNumberField(TEXT("y"));
            float PosZ = PosObject->GetNumberField(TEXT("z"));
            PlayerData.Position = FVector(PosX, PosY, PosZ);
        }

        CurrentPlayers.Add(PlayerData);
    }
}
```

### **ADICIONE** o parse de stats **ANTES** de `CurrentPlayers.Add(PlayerData);`:

```cpp
        // Stats (ADICIONAR ISTO)
        if (PlayerObject->HasField(TEXT("stats")))
        {
            UVaRestJsonObject* StatsObject = PlayerObject->GetObjectField(TEXT("stats"));
            PlayerData.Health = StatsObject->GetIntegerField(TEXT("health"));
            PlayerData.MaxHealth = StatsObject->GetIntegerField(TEXT("max_health"));
            PlayerData.Mana = StatsObject->GetIntegerField(TEXT("mana"));
            PlayerData.MaxMana = StatsObject->GetIntegerField(TEXT("max_mana"));
            PlayerData.Stamina = StatsObject->GetIntegerField(TEXT("stamina"));
            PlayerData.MaxStamina = StatsObject->GetIntegerField(TEXT("max_stamina"));
            PlayerData.Strength = StatsObject->GetIntegerField(TEXT("strength"));
            PlayerData.Dexterity = StatsObject->GetIntegerField(TEXT("dexterity"));
            PlayerData.Intelligence = StatsObject->GetIntegerField(TEXT("intelligence"));
            PlayerData.Vitality = StatsObject->GetIntegerField(TEXT("vitality"));
        }

        CurrentPlayers.Add(PlayerData);
```

---

## 📝 PASSO 1.2: Atualizar Parse de Criar Personagem

### Localize a função `OnCreateCharacterRequestComplete`:

**Procure por** (aprox. linha 540-570):

```cpp
void UUmbraGameInstance::OnCreateCharacterRequestComplete(UVaRestRequestJSON* Request)
{
    // ... código existente ...
    
    if (Response->HasField(TEXT("player")))
    {
        UVaRestJsonObject* PlayerObject = Response->GetObjectField(TEXT("player"));
        
        FUmbraPlayerData NewPlayer;
        NewPlayer.ID = PlayerObject->GetIntegerField(TEXT("player_id"));
        NewPlayer.CharacterName = PlayerObject->GetStringField(TEXT("character_name"));
        NewPlayer.Level = PlayerObject->GetIntegerField(TEXT("level"));
        NewPlayer.Experience = PlayerObject->GetIntegerField(TEXT("experience"));
        NewPlayer.CurrentZone = PlayerObject->GetStringField(TEXT("current_zone"));

        if (PlayerObject->HasField(TEXT("position")))
        {
            UVaRestJsonObject* PosObject = PlayerObject->GetObjectField(TEXT("position"));
            float PosX = PosObject->GetNumberField(TEXT("x"));
            float PosY = PosObject->GetNumberField(TEXT("y"));
            float PosZ = PosObject->GetNumberField(TEXT("z"));
            NewPlayer.Position = FVector(PosX, PosY, PosZ);
        }

        CurrentPlayers.Add(NewPlayer);
        
        UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Personagem criado: %s (ID: %d)"), *NewPlayer.CharacterName, NewPlayer.ID);
        OnCharacterCreated.Broadcast(NewPlayer);
    }
}
```

### **ADICIONE** o parse de stats **ANTES** de `CurrentPlayers.Add(NewPlayer);`:

```cpp
        // Stats (ADICIONAR ISTO)
        if (PlayerObject->HasField(TEXT("stats")))
        {
            UVaRestJsonObject* StatsObject = PlayerObject->GetObjectField(TEXT("stats"));
            NewPlayer.Health = StatsObject->GetIntegerField(TEXT("health"));
            NewPlayer.MaxHealth = StatsObject->GetIntegerField(TEXT("max_health"));
            NewPlayer.Mana = StatsObject->GetIntegerField(TEXT("mana"));
            NewPlayer.MaxMana = StatsObject->GetIntegerField(TEXT("max_mana"));
            NewPlayer.Stamina = StatsObject->GetIntegerField(TEXT("stamina"));
            NewPlayer.MaxStamina = StatsObject->GetIntegerField(TEXT("max_stamina"));
            NewPlayer.Strength = StatsObject->GetIntegerField(TEXT("strength"));
            NewPlayer.Dexterity = StatsObject->GetIntegerField(TEXT("dexterity"));
            NewPlayer.Intelligence = StatsObject->GetIntegerField(TEXT("intelligence"));
            NewPlayer.Vitality = StatsObject->GetIntegerField(TEXT("vitality"));
        }

        CurrentPlayers.Add(NewPlayer);
```

---

## 📝 PASSO 1.3: Atualizar Parse de Selecionar Personagem

### Localize a função `OnSelectCharacterRequestComplete`:

**Procure por** (aprox. linha 630-660):

```cpp
void UUmbraGameInstance::OnSelectCharacterRequestComplete(UVaRestRequestJSON* Request)
{
    // ... código existente ...
    
    if (Response->HasField(TEXT("player")))
    {
        UVaRestJsonObject* PlayerObject = Response->GetObjectField(TEXT("player"));
        
        FUmbraPlayerData SelectedPlayer;
        SelectedPlayer.ID = PlayerObject->GetIntegerField(TEXT("player_id"));
        SelectedPlayer.CharacterName = PlayerObject->GetStringField(TEXT("character_name"));
        SelectedPlayer.Level = PlayerObject->GetIntegerField(TEXT("level"));
        SelectedPlayer.Experience = PlayerObject->GetIntegerField(TEXT("experience"));
        SelectedPlayer.CurrentZone = PlayerObject->GetStringField(TEXT("current_zone"));

        if (PlayerObject->HasField(TEXT("position")))
        {
            UVaRestJsonObject* PosObject = PlayerObject->GetObjectField(TEXT("position"));
            float PosX = PosObject->GetNumberField(TEXT("x"));
            float PosY = PosObject->GetNumberField(TEXT("y"));
            float PosZ = PosObject->GetNumberField(TEXT("z"));
            SelectedPlayer.Position = FVector(PosX, PosY, PosZ);
        }

        ActivePlayerID = SelectedPlayer.ID;
        
        UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Personagem selecionado: %s (ID: %d, Level: %d)"), 
            *SelectedPlayer.CharacterName, SelectedPlayer.ID, SelectedPlayer.Level);
        
        OnCharacterSelected.Broadcast(SelectedPlayer);
    }
}
```

### **ADICIONE** o parse de stats **ANTES** de `ActivePlayerID = SelectedPlayer.ID;`:

```cpp
        // Stats (ADICIONAR ISTO)
        if (PlayerObject->HasField(TEXT("stats")))
        {
            UVaRestJsonObject* StatsObject = PlayerObject->GetObjectField(TEXT("stats"));
            SelectedPlayer.Health = StatsObject->GetIntegerField(TEXT("health"));
            SelectedPlayer.MaxHealth = StatsObject->GetIntegerField(TEXT("max_health"));
            SelectedPlayer.Mana = StatsObject->GetIntegerField(TEXT("mana"));
            SelectedPlayer.MaxMana = StatsObject->GetIntegerField(TEXT("max_mana"));
            SelectedPlayer.Stamina = StatsObject->GetIntegerField(TEXT("stamina"));
            SelectedPlayer.MaxStamina = StatsObject->GetIntegerField(TEXT("max_stamina"));
            SelectedPlayer.Strength = StatsObject->GetIntegerField(TEXT("strength"));
            SelectedPlayer.Dexterity = StatsObject->GetIntegerField(TEXT("dexterity"));
            SelectedPlayer.Intelligence = StatsObject->GetIntegerField(TEXT("intelligence"));
            SelectedPlayer.Vitality = StatsObject->GetIntegerField(TEXT("vitality"));
        }

        ActivePlayerID = SelectedPlayer.ID;
```

---

## 🔨 PASSO 1.4: COMPILAR

1. **Salve** `UmbraGameInstance.cpp`
2. **Compile** o projeto:
   - Visual Studio: `Ctrl + Shift + B`
   - Ou clique direito no `.uproject` → **Generate Visual Studio project files** → Abra e compile

---

## ✅ CHECKLIST FASE 1

```
[ ] Parse de stats em OnCharacterListRequestComplete
[ ] Parse de stats em OnCreateCharacterRequestComplete
[ ] Parse de stats em OnSelectCharacterRequestComplete
[ ] Projeto compilado sem erros
```

---

# 🎨 FASE 2: WIDGET DE SELEÇÃO DE PERSONAGENS

## 🎯 Objetivo

Criar `WBP_CharacterSelection` - o widget principal que lista os personagens.

---

## 📝 PASSO 2.1: Criar Widget

1. No **Content Browser**, vá para `Content/UI` (crie se não existir)
2. **Botão Direito** → **User Interface** → **Widget Blueprint**
3. Nome: `WBP_CharacterSelection`

---

## 📝 PASSO 2.2: DESIGNER - Layout

### Hierarquia Completa:

```
Canvas Panel
  └─ Vertical Box (Fill, Align Top)
      ├─ Text Block: "TXT_Title"
      │   └─ Texto: "SELECIONE SEU PERSONAGEM"
      │   └─ Font Size: 48
      │
      ├─ Spacer (Height: 20)
      │
      ├─ Scroll Box: "ScrollBox_Characters"
      │   └─ Size: Fill
      │   └─ Vertical Box: "VBox_CharacterList" (dentro do Scroll Box)
      │
      ├─ Spacer (Height: 20)
      │
      ├─ Horizontal Box (botões)
      │   ├─ Button: "BTN_CreateNew"
      │   │   └─ Text: "+ CRIAR PERSONAGEM"
      │   ├─ Spacer (Horizontal)
      │   └─ Button: "BTN_Logout"
      │       └─ Text: "Sair"
      │
      └─ Text Block: "TXT_Status"
          └─ Font Size: 16
```

---

## 📝 PASSO 2.3: Configurar Widgets

### TXT_Title:
- **Is Variable**: ✓ TRUE
- **Font Size**: 48
- **Color**: Branco
- **Justification**: Center

### ScrollBox_Characters:
- **Is Variable**: ✓ TRUE
- **Size**: Fill tanto Width quanto Height
- **Always Show Scrollbar**: FALSE

### VBox_CharacterList:
- **Is Variable**: ✓ TRUE
- **Size**: Fill

### BTN_CreateNew:
- **Is Variable**: ✓ TRUE
- **Size**: Width: 300, Height: 60
- **Background Color**: Verde (#27ae60)

### BTN_Logout:
- **Is Variable**: ✓ TRUE
- **Size**: Width: 150, Height: 60
- **Background Color**: Cinza (#95a5a6)

### TXT_Status:
- **Is Variable**: ✓ TRUE
- **Font Size**: 16
- **Color**: Amarelo

---

## 📝 PASSO 2.4: Variáveis Blueprint

No painel **My Blueprint**, adicione variáveis:

1. **MyGameInstance**
   - Type: `Umbra Game Instance` (Object Reference)
   - Instance Editable: FALSE

2. **CharacterWidgets**
   - Type: `WBP_CharacterItem` (Array)
   - Instance Editable: FALSE

---

## 📝 PASSO 2.5: EVENT GRAPH - Event Construct

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (As Umbra Game Instance)
[SET] MyGameInstance
    ↓
[Is Valid?] MyGameInstance
    ↓ Is Valid
[Bind Event to OnCharacterListLoaded]
    • Target: MyGameInstance
    • Event: [Create Custom Event]
    
    → [OnCharacterListLoaded_Event]
        ↓
        [Print String] "DEBUG: Lista carregada"
        ↓
        [PopulateCharacterList] (Custom Function - criar)

[Bind Event to OnCharacterListFailed]
    • Target: MyGameInstance
    • Event: [Create Custom Event]
    
    → [OnCharacterListFailed_Event] (ErrorMessage: String)
        ↓
        [Print String] ErrorMessage
        ↓
        [Set Text] TXT_Status
            • Text: ErrorMessage
        ↓
        [Set Color and Opacity] TXT_Status
            • Color: RED

[Bind Event to OnCharacterCreated]
    • Target: MyGameInstance
    • Event: [Create Custom Event]
    
    → [OnCharacterCreated_Event] (NewCharacter: UmbraPlayerData)
        ↓
        [Format Text] "✓ {0} criado com sucesso!"
            • {0}: NewCharacter → Character Name
        ↓
        [Set Text] TXT_Status
        ↓
        [Set Color] GREEN
        ↓
        [Delay] 1.0
        ↓
        [Load Character List]
            • Target: MyGameInstance

[Bind Event to OnCharacterSelected]
    • Target: MyGameInstance
    • Event: [Create Custom Event]
    
    → [OnCharacterSelected_Event] (SelectedCharacter: UmbraPlayerData)
        ↓
        [Format Text] "Carregando {0}..."
            • {0}: SelectedCharacter → Character Name
        ↓
        [Set Text] TXT_Status
        ↓
        [Delay] 1.0
        ↓
        [Remove from Parent] Self
        ↓
        [Open Level] "YourGameLevel"  ← SUBSTITUA pelo seu level de jogo

[Bind Event to OnCharacterDeleted]
    • Target: MyGameInstance
    • Event: [Create Custom Event]
    
    → [OnCharacterDeleted_Event] (Message: String)
        ↓
        [Set Text] TXT_Status
            • Text: Message
        ↓
        [Set Color] ORANGE
        ↓
        (Já recarrega automaticamente pelo C++)

[Set Input Mode UI Only]
    • Widget to Focus: SELF
    ↓
[Set Show Mouse Cursor] TRUE
    ↓
[Set Text] TXT_Status
    • Text: "Carregando personagens..."
    ↓
[Load Character List]
    • Target: MyGameInstance
```

---

## 📝 PASSO 2.6: CUSTOM FUNCTION - PopulateCharacterList

Criar nova função: **PopulateCharacterList**

```
[Function: PopulateCharacterList]
    ↓
[Print String] "DEBUG: PopulateCharacterList chamada"
    ↓
[Clear Children]
    • Target: VBox_CharacterList
    ↓
[GET] MyGameInstance
    ↓
[Get Character List]
    • Target: MyGameInstance
    ↓ (retorna TArray<FUmbraPlayerData>)
[GET Length]
    ↓ (int32)
[Branch] Length > 0?
    ↓ TRUE
[For Each Loop]
    • Array: (character list)
    ↓ Loop Body
    [Create Widget]
        • Class: WBP_CharacterItem ← (criar na Fase 3)
        • Owning Player: Get Player Controller (0)
        ↓
    [Call Function: SetCharacterData]
        • Target: (widget criado)
        • Character: (loop element - FUmbraPlayerData)
        ↓
    [Add Child to Vertical Box]
        • Target: VBox_CharacterList
        • Content: (widget criado)
    ↓ FALSE (Length = 0)
[Set Text] TXT_Status
    • Text: "Nenhum personagem. Crie um!"
    ↓
[Set Color] YELLOW
```

---

## 📝 PASSO 2.7: BTN_CreateNew OnClicked

```
[OnClicked] BTN_CreateNew
    ↓
[Create Widget]
    • Class: WBP_CreateCharacter ← (criar na Fase 4)
    • Owning Player: Get Player Controller (0)
    ↓
[Add to Viewport]
    • Target: (widget criado)
    • Z-Order: 999
```

---

## 📝 PASSO 2.8: BTN_Logout OnClicked

```
[OnClicked] BTN_Logout
    ↓
[GET] MyGameInstance
    ↓
[Logout]
    • Target: MyGameInstance
    ↓
[Remove from Parent] Self
    ↓
[Open Level] "Lvl_TestAuth"  ← Seu level de login
```

---

## ✅ CHECKLIST FASE 2

```
[ ] Widget WBP_CharacterSelection criado
[ ] Layout Designer configurado
[ ] Variáveis criadas (MyGameInstance, CharacterWidgets)
[ ] Event Construct com 5 binds
[ ] PopulateCharacterList implementada
[ ] BTN_CreateNew implementado
[ ] BTN_Logout implementado
[ ] Widget compilado sem erros
```

---

# 🎴 FASE 3: WIDGET DE ITEM DE PERSONAGEM

## 🎯 Objetivo

Criar `WBP_CharacterItem` - um card individual para cada personagem.

---

## 📝 PASSO 3.1: Criar Widget

1. **Content Browser** → `Content/UI`
2. **Botão Direito** → **Widget Blueprint**
3. Nome: `WBP_CharacterItem`

---

## 📝 PASSO 3.2: DESIGNER - Layout

### Hierarquia:

```
Border (estilo de card)
  └─ Horizontal Box
      ├─ Vertical Box (Info do Personagem)
      │   ├─ Text Block: "TXT_Name"
      │   │   └─ Font Size: 32, Color: White
      │   ├─ Text Block: "TXT_Level"
      │   │   └─ Font Size: 18, Color: Yellow
      │   ├─ Text Block: "TXT_Zone"
      │   │   └─ Font Size: 16, Color: Gray
      │   └─ Horizontal Box (Stats)
      │       ├─ Text Block: "TXT_Health"
      │       ├─ Spacer
      │       ├─ Text Block: "TXT_Mana"
      │       ├─ Spacer
      │       └─ Text Block: "TXT_Stamina"
      │
      └─ Horizontal Box (Ações)
          ├─ Button: "BTN_Play"
          │   └─ Text: "▶ JOGAR"
          │   └─ Size: 120x50
          │   └─ Color: GREEN
          └─ Button: "BTN_Delete"
              └─ Text: "🗑"
              └─ Size: 50x50
              └─ Color: RED
```

---

## 📝 PASSO 3.3: Configurar Widgets

### Border:
- **Brush Color**: Semi-transparente (#2d2d44, Alpha: 0.9)
- **Padding**: 15

### TXT_Name:
- **Is Variable**: ✓ TRUE
- **Font Size**: 32
- **Color**: Branco

### TXT_Level, TXT_Zone, TXT_Health, TXT_Mana, TXT_Stamina:
- **Is Variable**: ✓ TRUE

### BTN_Play:
- **Is Variable**: ✓ TRUE
- **Background**: Verde (#27ae60)

### BTN_Delete:
- **Is Variable**: ✓ TRUE
- **Background**: Vermelho (#e74c3c)

---

## 📝 PASSO 3.4: Variáveis Blueprint

1. **CharacterData**
   - Type: `Umbra Player Data` (Struct)
   - Instance Editable: FALSE

2. **MyGameInstance**
   - Type: `Umbra Game Instance` (Object Reference)
   - Instance Editable: FALSE

---

## 📝 PASSO 3.5: EVENT GRAPH - Event Construct

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[SET] MyGameInstance
```

---

## 📝 PASSO 3.6: CUSTOM FUNCTION - SetCharacterData

**Criar função pública**: `SetCharacterData`

**Input**: `Character` (tipo: `FUmbraPlayerData`)

```
[Function: SetCharacterData] (Character)
    ↓
[SET] CharacterData = Character
    ↓
[Set Text] TXT_Name
    • Text: Character → Character Name
    ↓
[Format Text] "Level: {0}"
    • {0}: Character → Level
    ↓
[Set Text] TXT_Level
    ↓
[Format Text] "Zona: {0}"
    • {0}: Character → Current Zone
    ↓
[Set Text] TXT_Zone
    ↓
[Format Text] "❤ {0}/{1}"
    • {0}: Character → Health
    • {1}: Character → Max Health
    ↓
[Set Text] TXT_Health
    ↓
[Format Text] "🔵 {0}/{1}"
    • {0}: Character → Mana
    • {1}: Character → Max Mana
    ↓
[Set Text] TXT_Mana
    ↓
[Format Text] "⚡ {0}/{1}"
    • {0}: Character → Stamina
    • {1}: Character → Max Stamina
    ↓
[Set Text] TXT_Stamina
```

---

## 📝 PASSO 3.7: BTN_Play OnClicked

```
[OnClicked] BTN_Play
    ↓
[GET] CharacterData → ID
    ↓
[GET] MyGameInstance
    ↓
[Select Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
```

---

## 📝 PASSO 3.8: BTN_Delete OnClicked

```
[OnClicked] BTN_Delete
    ↓
[GET] CharacterData → Character Name
    ↓
[Format Text] "Deletar '{0}'? (NÃO PODE SER DESFEITO)"
    • {0}: (character name)
    ↓
[Create Widget]
    • Class: WidgetBlueprint'/Game/UI/WBP_ConfirmDialog.WBP_ConfirmDialog_C'
    • (ou use um simples Branch com Print String para teste)
    ↓
ALTERNATIVA SIMPLES (SEM DIÁLOGO):
[Print String] "DELETAR: confirme manualmente"
    ↓
[GET] CharacterData → ID
    ↓
[GET] MyGameInstance
    ↓
[Delete Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
```

---

## ✅ CHECKLIST FASE 3

```
[ ] Widget WBP_CharacterItem criado
[ ] Layout Designer configurado
[ ] Variáveis criadas
[ ] SetCharacterData implementada
[ ] BTN_Play implementado
[ ] BTN_Delete implementado
[ ] Widget compilado
```

---

# ✨ FASE 4: WIDGET DE CRIAR PERSONAGEM

## 🎯 Objetivo

Criar `WBP_CreateCharacter` - diálogo modal para criar novo personagem.

---

## 📝 PASSO 4.1: Criar Widget

1. **Content Browser** → `Content/UI`
2. **Widget Blueprint**
3. Nome: `WBP_CreateCharacter`

---

## 📝 PASSO 4.2: DESIGNER - Layout

```
Canvas Panel (Overlay escuro)
  └─ Border (Fundo preto 70% opaco)
      └─ Border (Painel central branco)
          └─ Vertical Box
              ├─ Text Block: "CRIAR NOVO PERSONAGEM"
              │   └─ Font Size: 36
              ├─ Spacer (20px)
              ├─ Horizontal Box
              │   ├─ Text Block: "Nome:"
              │   └─ Editable Text: "TXT_Name"
              │       └─ Hint Text: "Ex: Warrior123"
              ├─ Text Block: "TXT_Validation"
              │   └─ Color: Yellow
              ├─ Spacer
              └─ Horizontal Box (Botões)
                  ├─ Button: "BTN_Create" ("CRIAR")
                  └─ Button: "BTN_Cancel" ("CANCELAR")
```

---

## 📝 PASSO 4.3: Variáveis

1. **MyGameInstance** (Umbra Game Instance, Object Reference)

---

## 📝 PASSO 4.4: Event Construct

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[SET] MyGameInstance
    ↓
[Bind OnCharacterCreated]
    → [OnCreated_Event] (NewCharacter)
        ↓
        [Print String] "Personagem criado!"
        ↓
        [Delay] 0.5
        ↓
        [Remove from Parent] Self

[Bind OnCharacterCreateFailed]
    → [OnFailed_Event] (ErrorMessage)
        ↓
        [Set Text] TXT_Validation
            • Text: ErrorMessage
        ↓
        [Set Color] RED
        ↓
        [Set Enabled] BTN_Create
            • Enabled: TRUE

[Set Input Mode UI Only]
    • Widget to Focus: TXT_Name
```

---

## 📝 PASSO 4.5: TXT_Name OnTextChanged

```
[OnTextChanged] TXT_Name (Text)
    ↓
[To String] Text
    ↓
[Len]
    ↓ (int32)
[Branch] < 3?
    ↓ TRUE
    [Set Text] TXT_Validation
        • Text: "⚠ Mínimo 3 caracteres"
        • Color: ORANGE
    ↓ FALSE
[Branch] > 20?
    ↓ TRUE
    [Set Text] TXT_Validation
        • Text: "⚠ Máximo 20 caracteres"
        • Color: ORANGE
    ↓ FALSE
[Set Text] TXT_Validation
    • Text: "✓ Nome válido"
    • Color: GREEN
```

---

## 📝 PASSO 4.6: BTN_Create OnClicked

```
[OnClicked] BTN_Create
    ↓
[Get Text] TXT_Name
    ↓
[To String]
    ↓
[Trim]
    ↓
[Is Empty?] Branch
    ↓ FALSE
[GET] MyGameInstance
    ↓
[Create Character]
    • Target: MyGameInstance
    • Character Name: (trimmed text)
    ↓
[Set Enabled] BTN_Create
    • Enabled: FALSE
    ↓
[Set Text] TXT_Validation
    • Text: "Criando..."
    • Color: CYAN
```

---

## 📝 PASSO 4.7: BTN_Cancel OnClicked

```
[OnClicked] BTN_Cancel
    ↓
[Remove from Parent] Self
```

---

## ✅ CHECKLIST FASE 4

```
[ ] Widget WBP_CreateCharacter criado
[ ] Layout Designer configurado
[ ] Event Construct com binds
[ ] Validação em tempo real (OnTextChanged)
[ ] BTN_Create implementado
[ ] BTN_Cancel implementado
[ ] Widget compilado
```

---

# 🔗 FASE 5: INTEGRAÇÃO COM LOGIN

## 🎯 Objetivo

Fazer o Login redirecionar para a tela de personagens.

---

## 📝 PASSO 5.1: Modificar WBP_Login

Abra: `WBP_Login`

### Localize: **Custom Event OnLoginSuccess_Custom**

**ANTES** (você tem isto):
```
[OnLoginSuccess_Custom]
    ↓
[Remove from Parent] Self
    ↓
[Create Widget] WBP_Dashboard
    ↓
[Add to Viewport]
```

**DEPOIS** (modifique para):
```
[OnLoginSuccess_Custom]
    ↓
[Print String] "Login OK! Redirecionando..."
    ↓
[Remove from Parent] Self
    ↓
[Create Widget]
    • Class: WBP_CharacterSelection  ← MUDANÇA!
    • Owning Player: Get Player Controller (0)
    ↓
[Add to Viewport]
```

---

## 📝 PASSO 5.2: (Opcional) Modificar Dashboard

Se quiser acessar personagens pelo Dashboard:

No `WBP_Dashboard`, adicione um botão:

```
[BTN_ManageCharacters] OnClicked
    ↓
[Remove from Parent] Self
    ↓
[Create Widget] WBP_CharacterSelection
    ↓
[Add to Viewport]
```

---

## ✅ CHECKLIST FASE 5

```
[ ] WBP_Login modificado (vai para CharacterSelection)
[ ] (Opcional) Dashboard com botão para personagens
[ ] Widgets compilados
```

---

# 🧪 FASE 6: TESTES COMPLETOS

## 🎯 Objetivo

Testar todo o fluxo do sistema.

---

## 📝 TESTE 1: Login → Lista Vazia

1. **Play** (PIE)
2. **Login** com sua conta (account_id: 4)
3. Deve aparecer: `WBP_CharacterSelection`
4. Deve mostrar: "Nenhum personagem. Crie um!"

---

## 📝 TESTE 2: Criar Personagem

1. Clique: **"+ CRIAR PERSONAGEM"**
2. Deve aparecer: `WBP_CreateCharacter`
3. Digite nome: `"TestHero"`
4. Clique: **"CRIAR"**
5. Deve:
   - Fechar diálogo
   - Atualizar lista
   - Mostrar 1 personagem na lista

---

## 📝 TESTE 3: Ver Personagem

1. Deve aparecer um card com:
   - Nome: `TestHero`
   - Level: 1
   - Zona: Tutorial
   - Stats: ❤ 100/100, 🔵 50/50, ⚡ 100/100
   - Botões: **▶ JOGAR** e **🗑**

---

## 📝 TESTE 4: Selecionar Personagem

1. Clique: **"▶ JOGAR"**
2. Deve:
   - Mostrar "Carregando TestHero..."
   - Fechar WBP_CharacterSelection
   - Abrir level de jogo

---

## 📝 TESTE 5: Deletar Personagem

1. Login novamente
2. Clique: **🗑** no personagem
3. Deve:
   - Deletar personagem
   - Atualizar lista
   - Mostrar lista vazia

---

## 📝 TESTE 6: Criar Múltiplos

1. Crie 2-3 personagens
2. Deve mostrar todos na lista
3. Todos devem ter botões funcionais

---

## 📝 TESTE 7: Logout

1. Na tela de personagens
2. Clique: **"Sair"**
3. Deve voltar para tela de login

---

## ✅ CHECKLIST TESTE

```
[ ] Login → CharacterSelection funciona
[ ] Lista vazia mostra mensagem
[ ] Criar personagem funciona
[ ] Personagem aparece na lista
[ ] Stats aparecem corretamente
[ ] Botão Jogar funciona
[ ] Botão Deletar funciona
[ ] Logout funciona
```

---

# 🎯 RESULTADO FINAL

Após completar todas as fases, você terá:

```
✅ Login de usuário
✅ Lista de personagens
✅ Criar novo personagem (com validação)
✅ Ver stats do personagem (HP, Mana, Stamina, etc)
✅ Selecionar personagem para jogar
✅ Deletar personagem
✅ Logout
✅ Tudo sincronizado com MySQL via APIs PHP
```

---

# 📊 TEMPO ESTIMADO POR FASE

```
Fase 1 (C++ Parse):             30 min  ⭐⭐
Fase 2 (CharacterSelection):    45 min  ⭐⭐⭐
Fase 3 (CharacterItem):         30 min  ⭐⭐
Fase 4 (CreateCharacter):       25 min  ⭐⭐
Fase 5 (Integração Login):      10 min  ⭐
Fase 6 (Testes):                20 min  ⭐
──────────────────────────────────────
TOTAL:                         160 min  (2h40min)
```

---

# 🆘 TROUBLESHOOTING

## ❌ "CharacterList vazia mesmo tendo personagens"

**Causa**: Delegates não bindeados ou Load não chamado

**Solução**:
1. Verifique se `Bind Event to OnCharacterListLoaded` está no Event Construct
2. Verifique se `Load Character List` é chamado no final do Construct
3. Adicione Print String para debug

---

## ❌ "Widget não aparece"

**Causa**: Z-Order ou Add to Viewport

**Solução**:
```
[Create Widget]
    ↓
[Add to Viewport]
    • Z-Order: 100
```

---

## ❌ "Stats aparecem 0"

**Causa**: Parse de stats não implementado

**Solução**: Volte para Fase 1, Passo 1.1-1.3

---

## ❌ "Personagem não deleta"

**Causa**: Account ID errado ou API com erro

**Solução**:
1. Adicione Print String do Account ID antes de chamar Delete
2. Teste API diretamente: `http://localhost/umbra_api/test_character.html`

---

## ❌ "Compile Error no C++"

**Causa**: Typos ou estrutura incorreta

**Solução**:
1. Verifique se `FUmbraPlayerData` tem todos os campos (Experience, Health, etc)
2. Recompile com `Ctrl + Shift + B`

---

# 📚 ARQUIVOS CRIADOS

```
Content/UI/
├── WBP_CharacterSelection.uasset   ← Widget principal
├── WBP_CharacterItem.uasset        ← Card de personagem
└── WBP_CreateCharacter.uasset      ← Diálogo criar
```

---

# 🎮 PRÓXIMOS PASSOS (OPCIONAL)

Após completar o básico:

1. **Melhorar UI/UX**:
   - Animações de transição
   - Efeitos visuais
   - Sons de feedback

2. **Sistema de Confirmação**:
   - Widget de diálogo genérico
   - Confirmação antes de deletar

3. **Customização de Personagem**:
   - Escolher classe
   - Escolher aparência
   - Stats iniciais diferentes

4. **Spawn no Mundo**:
   - Carregar personagem no GameMode
   - Aplicar stats ao Character
   - Spawnar na posição salva

---

**🚀 BOA SORTE COM A IMPLEMENTAÇÃO!**

