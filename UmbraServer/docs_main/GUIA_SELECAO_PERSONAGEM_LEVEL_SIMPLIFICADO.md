# 🎮 **GUIA SIMPLIFICADO: Seleção de Personagem (MÁXIMO C++)**

## ✅ **FUNÇÕES C++ CRIADAS**

Todas as funções principais foram movidas para C++! O Blueprint agora apenas chama as funções C++.

### **Funções C++ Disponíveis:**

1. **`Initialize(PC, Widget)`** - Inicializa o manager
2. **`SpawnAllCharacterPreviews(PreviewClass, Spacing)`** - Spawna todos os previews automaticamente
3. **`SelectCharacterAndMoveCamera(PlayerID, ...)`** - Seleciona, despawna outros e move câmera
4. **`GetPreviewByPlayerID(PlayerID)`** - Encontra preview pelo ID
5. **`AddCharacterItemToWidget(PlayerID, WidgetClass)`** - Cria e adiciona WBP_CharacterItem ao VBox
6. **`ResetSelection(PreviewClass, Spacing)`** - Reseta e respawna todos os previews

---

## 📝 **IMPLEMENTAÇÃO NO BLUEPRINT (MÍNIMA)**

### **1. Level Blueprint - Lvl_CharacterSelection**

```
[Event BeginPlay]
    ↓
[Delay] 0.2s
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Find and Set Level Camera Actor]
        ↓
        [Spawn Actor] BP_CharacterSelectionManager
        ↓
        [Is Valid?]
        ├─→ [FALSE] → STOP
        └─→ [TRUE] →
            ↓
            [Create Widget] WBP_CharacterSelection
            ↓
            [Is Valid?]
            ├─→ [FALSE] → STOP
            └─→ [TRUE] →
                ↓
                [Call Function: Initialize] (C++)
                • PC: (Player Controller)
                • Widget: (Widget criado)
                ↓
                [Call Function: Spawn All Character Previews] (C++)
                • Preview Class: BP_CharacterPreview
                • Spacing: 300.0
                ↓
                [Add to Viewport]
                ↓
                [Set Input Mode Game And UI]
                ↓
                [Set Show Mouse Cursor] TRUE
```

**IMPORTANTE**: As funções C++ fazem TUDO automaticamente! Não precisa de mais nada no Level Blueprint.

---

### **2. WBP_CharacterSelection - Event Construct**

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Set] MyGameInstance
    ↓
[Call Function: PopulateCharacterSelectButtons]
```

**PopulateCharacterSelectButtons** (Blueprint simples - apenas cria botões na lista):

```
[PopulateCharacterSelectButtons]
    ↓
[Clear Children] VBox_CharacterList
    ↓
[Get] MyGameInstance
    ↓
[Get Current Players] (Array)
    ↓
[For Each Loop] CurrentPlayers
    ↓
    [Create Widget] WBP_CharacterSelectButton
    ↓
    [Call Function: SetCharacterData]
    ↓
    [Call Function: SetSelectionWidget]
    ↓
    [Add Child to Vertical Box] VBox_CharacterList
```

---

### **3. WBP_CharacterSelection - OnSelectCharacterClicked**

```
[OnSelectCharacterClicked] (Custom Event)
    • PlayerID (input)
    ↓
[Get] SelectionManager
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: Select Character And Move Camera] (C++)
        • PlayerID: (PlayerID)
        • Target Location: (0, 0, 0)
        • Duration: 1.0
        • OffsetX: 200.0
        • OffsetY: 0.0
        • OffsetZ: 100.0
        ↓
        [Branch] (Return Value)
        ├─→ [FALSE] → [Print String] "Erro"
        └─→ [TRUE] →
            ↓
            [Get Game Instance]
            ↓
            [Cast to UmbraGameInstance]
            ↓
            [Load Character Info]
            ↓
            [Delay] 0.5s
            ↓
            [Call Function: Add Character Item To Widget] (C++)
            • PlayerID: (PlayerID)
            • Character Item Widget Class: WBP_CharacterItem
```

**PRONTO!** A função C++ `AddCharacterItemToWidget` faz TUDO:
- Cria o widget
- Configura os dados
- Limpa o VBox
- Adiciona ao VBox

---

### **4. WBP_CharacterItem - BTN_Return**

```
[BTN_Return OnClicked]
    ↓
[Get] SelectionManager
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: Reset Selection] (C++)
        • Preview Class: BP_CharacterPreview
        • Spacing: 300.0
        ↓
        [Remove from Parent] (Self)
```

**PRONTO!** A função C++ `ResetSelection` faz TUDO:
- Limpa o VBox
- Reseta SelectedPreview
- Respawna todos os previews

---

## ✅ **RESUMO: O QUE É C++ vs BLUEPRINT**

### **C++ (Faz Tudo):**
- ✅ `Initialize` - Seta variáveis
- ✅ `SpawnAllCharacterPreviews` - Spawna todos os previews, configura dados, seta manager
- ✅ `SelectCharacterAndMoveCamera` - Encontra preview, despawna outros, move câmera, seleciona no GameInstance
- ✅ `GetPreviewByPlayerID` - Busca no array
- ✅ `AddCharacterItemToWidget` - Cria widget, configura, limpa VBox, adiciona ao VBox
- ✅ `ResetSelection` - Limpa VBox, respawna previews

### **Blueprint (Apenas Chama C++):**
- ✅ Level Blueprint: Spawna manager, cria widget, chama `Initialize` e `SpawnAllCharacterPreviews`
- ✅ WBP_CharacterSelection: Popula lista de botões, chama `SelectCharacterAndMoveCamera` e `AddCharacterItemToWidget`
- ✅ WBP_CharacterItem: BTN_Play abre level, BTN_Return chama `ResetSelection`

---

## 🎯 **VANTAGENS**

1. **Menos código Blueprint** - Apenas chamadas simples
2. **Mais rápido** - C++ é mais performático
3. **Mais fácil de manter** - Lógica centralizada em C++
4. **Menos erros** - Reflection tratado em C++
5. **Reutilizável** - Funções C++ podem ser usadas em outros lugares

---

**FIM DO GUIA SIMPLIFICADO**

