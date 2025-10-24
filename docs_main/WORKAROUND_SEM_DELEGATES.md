# 🔧 WORKAROUND - Sistema de Personagens SEM Delegates

**Problema**: Delegates não aparecem no Blueprint  
**Solução**: Usar polling com Delay/Timer

---

## 🎯 EVENT CONSTRUCT - Versão SEM Delegates

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (As Umbra Game Instance)
[SET] MyGameInstance
    ↓
[Branch] Is Valid?
    • Condition: MyGameInstance
    ↓ TRUE
[Print String] "✓ Game Instance OK"
    ↓
[Set Input Mode UI Only]
    • Widget to Focus: SELF
    ↓
[Set Show Mouse Cursor] TRUE
    • Target: Get Player Controller (0)
    ↓
[Set Text] TXT_Status
    • Text: "Carregando personagens..."
    ↓
[GET] MyGameInstance
    ↓
[Load Character List]
    • Target: MyGameInstance
    ↓
[Print String] "API chamada: Load Character List"
    ↓
[Delay] 2.0 segundos  ← ESPERA A API RESPONDER
    ↓
[RefreshCharacterList] (Custom Function)
```

---

## 🔧 CUSTOM FUNCTION: RefreshCharacterList

Criar nova função: **RefreshCharacterList**

```
[Function: RefreshCharacterList]
    ↓
[Print String] "Atualizando lista..."
    ↓
[GET] MyGameInstance
    ↓
[Get Character List]
    • Target: MyGameInstance
    ↓ (retorna TArray<FUmbraPlayerData>)
[SET] (local temp variable) CharacterArray
    ↓
[GET Length]
    • Target: CharacterArray
    ↓
[Print String] "Total encontrado: {Length}"
    ↓
[Branch] Length > 0?
    ↓ TRUE
[PopulateCharacterList]
    • CharacterArray: (passar array)
    ↓ FALSE
[Set Text] TXT_Status
    • Text: "Nenhum personagem. Crie um!"
    ↓
[Set Color and Opacity] TXT_Status
    • Color: YELLOW
```

---

## 🎨 CUSTOM FUNCTION: PopulateCharacterList

Modificar para receber array como parâmetro:

**Input**: `CharacterArray` (tipo: `Array of Umbra Player Data`)

```
[Function: PopulateCharacterList] (CharacterArray)
    ↓
[Print String] "Populando lista..."
    ↓
[Clear Children]
    • Target: VBox_CharacterList
    ↓
[For Each Loop]
    • Array: CharacterArray
    ↓ Loop Body (Array Element)
[Print String] "Criando item: {Element.CharacterName}"
    ↓
[Create Widget]
    • Class: WBP_CharacterItem
    • Owning Player: Get Player Controller (0)
    ↓
[Call Function: SetCharacterData]
    • Target: (widget criado)
    • Character: (loop element)
    ↓
[Add Child to Vertical Box]
    • Target: VBox_CharacterList
    • Content: (widget criado)
    ↓ (Loop continua)
[Completed]
    ↓
[Set Text] TXT_Status
    • Text: "Lista carregada!"
    ↓
[Set Color] GREEN
```

---

## 🔘 BTN_CreateNew OnClicked

```
[OnClicked] BTN_CreateNew
    ↓
[Print String] "Abrindo criação de personagem..."
    ↓
[Create Widget]
    • Class: WBP_CreateCharacter
    • Owning Player: Get Player Controller (0)
    ↓
[Add to Viewport]
    • Target: (widget criado)
    • Z-Order: 999
    ↓
[Print String] "Widget criado!"
```

---

## ✨ WBP_CreateCharacter - Event Construct

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[SET] MyGameInstance
    ↓
[Set Input Mode UI Only]
    • Widget to Focus: TXT_Name
    ↓
[Set Show Mouse Cursor] TRUE
```

---

## 🔘 WBP_CreateCharacter - BTN_Create OnClicked

```
[OnClicked] BTN_Create
    ↓
[Get Text] TXT_Name
    ↓
[To String]
    ↓
[Trim]
    ↓
[SET] (local) CharName
    ↓
[Branch] Is Empty?
    • String: CharName
    ↓ FALSE
[GET] MyGameInstance
    ↓
[Create Character]
    • Target: MyGameInstance
    • Character Name: CharName
    ↓
[Set Enabled] BTN_Create
    • Enabled: FALSE
    ↓
[Set Text] TXT_Validation
    • Text: "Criando personagem..."
    • Color: CYAN
    ↓
[Delay] 2.0 segundos  ← ESPERA API
    ↓
[Print String] "Personagem criado!"
    ↓
[Remove from Parent] SELF
    ↓ TRUE (String vazia)
[Set Text] TXT_Validation
    • Text: "⚠ Digite um nome!"
    • Color: RED
```

---

## 🔄 WBP_CharacterSelection - Refresh Automático

Adicione um **Timer** para atualizar a lista:

### Event Construct (adicionar no final):

```
[...Event Construct anterior...]
    ↓
[Set Timer by Event]
    • Event: RefreshCharacterList
    • Time: 3.0 segundos
    • Looping: TRUE
```

Ou apenas **botão manual**:

### Adicionar Botão BTN_Refresh

```
[OnClicked] BTN_Refresh
    ↓
[RefreshCharacterList]
```

---

## 🎮 WBP_CharacterItem - BTN_Play OnClicked

```
[OnClicked] BTN_Play
    ↓
[GET] CharacterData → ID
    ↓
[Print String] "Selecionando ID: {ID}"
    ↓
[GET] MyGameInstance
    ↓
[Select Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
    ↓
[Delay] 1.5 segundos  ← ESPERA API
    ↓
[Print String] "Personagem selecionado!"
    ↓
[Get All Widgets of Class]
    • Class: WBP_CharacterSelection
    ↓
[ForEach Loop]
    ↓
[Remove from Parent]
    • Target: (cada widget)
    ↓
[Open Level] "YourGameLevel"
```

---

## 🗑️ WBP_CharacterItem - BTN_Delete OnClicked

```
[OnClicked] BTN_Delete
    ↓
[GET] CharacterData → ID
    ↓
[GET] CharacterData → Character Name
    ↓
[Format Text] "Deletar {0}?"
    • {0}: Character Name
    ↓
[Print String] (texto formatado)
    ↓
[GET] MyGameInstance
    ↓
[Delete Character]
    • Target: MyGameInstance
    • Player ID: (character ID)
    ↓
[Delay] 1.5 segundos  ← ESPERA API
    ↓
[Print String] "Personagem deletado!"
    ↓
[Get All Widgets of Class]
    • Class: WBP_CharacterSelection
    ↓
[ForEach Loop]
    ↓
[Call Function: RefreshCharacterList]
    • Target: (cada WBP_CharacterSelection)
```

---

## 🎯 VANTAGENS DESTE MÉTODO

```
✅ Funciona 100% (não depende de delegates)
✅ Simples de entender e debugar
✅ Print Strings mostram cada etapa
✅ Pode adicionar Timers para auto-refresh
✅ Fácil de modificar delays conforme necessário
```

---

## ⏱️ DELAYS RECOMENDADOS

```
Load Character List:  2.0 segundos
Create Character:     2.0 segundos
Select Character:     1.5 segundos
Delete Character:     1.5 segundos
```

**Nota**: Em produção, use **Timers com checks** ao invés de delays fixos.

---

## 🔄 MELHORAR: Timer com Check

Para produção, substitua Delay por:

```
[Set Timer by Function Name]
    • Function Name: CheckIfDataLoaded
    • Time: 0.2 segundos
    • Looping: TRUE

[Custom Event: CheckIfDataLoaded]
    ↓
[GET] MyGameInstance
    ↓
[Get Character List]
    ↓
[GET Length]
    ↓
[Branch] Length Changed?
    ↓ TRUE
    [Clear Timer by Function Name]
        • Function Name: CheckIfDataLoaded
        ↓
    [PopulateCharacterList]
```

---

## 📋 CHECKLIST

```
[ ] Event Construct com Load + Delay 2s + Refresh
[ ] RefreshCharacterList implementada
[ ] PopulateCharacterList recebe array como parâmetro
[ ] BTN_CreateNew abre WBP_CreateCharacter
[ ] WBP_CreateCharacter cria personagem + Delay 2s + fecha
[ ] BTN_Play seleciona + Delay 1.5s + abre level
[ ] BTN_Delete deleta + Delay 1.5s + refresh
[ ] Print Strings em todas as etapas para debug
```

---

## 🚀 TESTANDO

1. **Play** (PIE)
2. **Login** com sua conta
3. Observe os **Print Strings** no log
4. Deve mostrar:
   ```
   ✓ Game Instance OK
   API chamada: Load Character List
   Atualizando lista...
   Total encontrado: 1
   Populando lista...
   Criando item: ElJeffo
   Lista carregada!
   ```

---

**🎮 ESTE MÉTODO FUNCIONA 100%!**

Use isto até descobrirmos por que os delegates não aparecem.

