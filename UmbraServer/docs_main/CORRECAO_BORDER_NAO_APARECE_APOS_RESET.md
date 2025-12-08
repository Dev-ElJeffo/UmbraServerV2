# 🔧 **CORREÇÃO: Border_WBP_Creator Não Aparece Após Reset e Nova Seleção**

## ❌ **PROBLEMA**

Quando você:
1. Seleciona uma classe pela primeira vez → `Border_WBP_Creator` aparece ✅
2. Clica em "Return to Selection" → `Border_WBP_Creator` some (Collapsed) ✅
3. Seleciona outra classe → `Border_WBP_Creator` **NÃO aparece** ❌

**O `VBox_ClassList` também não é populado com as informações da nova classe.**

---

## 🔍 **CAUSA**

Quando uma classe é selecionada, o código que:
- Torna o `Border_WBP_Creator` **Visible**
- Popula o `VBox_ClassList` com as informações da classe

**NÃO está sendo executado após o reset.**

---

## ✅ **SOLUÇÃO: Mostrar Border e Popular VBox Quando Classe é Selecionada**

### **PASSO 1: Verificar Onde a Classe é Selecionada**

**Procure no Blueprint `WBP_CreateCharacter` onde a seleção de classe acontece:**

1. **Botões de classe** (BTN_Class1, BTN_Class2, etc.)
2. **Click nos placeholders 3D**
3. **Função `SelectClassAndMoveCamera`**

**Exemplo de onde pode estar:**

```
[BTN_Class1 OnClicked]
    ↓
[SelectClassAndMoveCamera]
    • ClassID: 1
    • Return Value: Success
```

---

### **PASSO 2: Criar Função ShowClassInfo**

**No `WBP_CreateCharacter`, crie uma função `ShowClassInfo`:**

1. **My Blueprint** → **Functions** → **+ Function**
2. **Nome:** `ShowClassInfo`
3. **Inputs:**
   - `ClassID` (Integer)

**Implementação:**

```
[ShowClassInfo]
    • ClassID (input)
    ↓
[Get] Border_WBP_Creator
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible ← MOSTRAR O BORDER
    ↓
[Get] VBox_ClassList
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Clear Children] ← Limpar antes de popular
    • Target: VBox_ClassList
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Get Class Data By ID]
    • Target: (cast result)
    • Class ID: ClassID
    • Out Class Data: (variável local)
    • Return Value: bFound
    ↓
[Branch]
    • Condition: bFound
    ↓ (True)
[Get Player Controller] (Index: 0)
    ↓
[Create Widget]
    • Class: WBP_ClassSelectorItem (ou o widget que você usa)
    • Owning Player: (Get Player Controller result)
    • Return Value: ClassInfoWidget
    ↓
[Call Function: SetClassData] (no ClassInfoWidget)
    • Target: ClassInfoWidget
    • New Class Data: Out Class Data
    ↓
[Add Child to Vertical Box]
    • Target: VBox_ClassList
    • Content: ClassInfoWidget
    ↓
[Print String] (opcional - debug)
    • In String: "ClassInfo adicionado para ClassID: " + (ClassID como String)
```

---

### **PASSO 3: Chamar ShowClassInfo Quando Classe é Selecionada**

**Você precisa chamar `ShowClassInfo` TODA VEZ que uma classe for selecionada.**

#### **OPÇÃO A: Após SelectClassAndMoveCamera**

**Onde quer que `SelectClassAndMoveCamera` seja chamado, adicione:**

```
[SelectClassAndMoveCamera]
    • ClassID: [ID da classe]
    • Return Value: Success
    ↓
[Branch]
    • Condition: Success
    ↓ (True)
[Call Function: ShowClassInfo]
    • Target: Self (WBP_CreateCharacter)
    • ClassID: [mesmo ID da classe]
```

**Exemplo completo em um botão:**

```
[BTN_Class1 OnClicked]
    ↓
[Get] Manager
    ↓
[Is Valid] Manager
    ↓
[Branch] (True)
    ↓
[Select Class And Move Camera]
    • Target: Manager
    • Class ID: 1
    • Return Value: Success
    ↓
[Branch]
    • Condition: Success
    ↓ (True)
[Call Function: ShowClassInfo]
    • Target: Self
    • ClassID: 1
```

#### **OPÇÃO B: No Event Handler OnClassSelected**

**Se você usa um delegate/event `OnClassSelected`:**

```
[OnClassSelected] (Custom Event)
    • ClassID (input)
    ↓
[Get] Manager
    ↓
[Select Class And Move Camera]
    • Target: Manager
    • Class ID: ClassID
    • Return Value: Success
    ↓
[Branch]
    • Condition: Success
    ↓ (True)
[Call Function: ShowClassInfo]
    • Target: Self
    • ClassID: ClassID
```

---

### **PASSO 4: Garantir que Border Fica Collapsed no Reset**

**No `BTN_Return OnClicked`, certifique-se de que o `Border_WBP_Creator` fica Collapsed:**

```
[BTN_Return OnClicked]
    ↓
[Get] Border_WBP_Creator
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Collapsed ← ESCONDER O BORDER
    ↓
[Get] Manager
    ↓
[Is Valid] Manager
    ↓
[Branch] (True)
    ↓
[Reset Character Creation]
    • Target: Manager
    • Create Character Widget: Self
    • Placeholder Class: BP_Class_Placeholder
    • Spacing: 300.0
    ↓
[Call Function: SpawnAllPlaceholders]
    • Target: Self
```

---

## 🔍 **DIAGNÓSTICO: Adicionar Logs**

### **1. Verificar se ShowClassInfo Está Sendo Chamado**

**Adicione no início de `ShowClassInfo`:**

```
[ShowClassInfo]
    • ClassID (input)
    ↓
[Print String]
    • In String: "🔥 ShowClassInfo CHAMADO com ClassID: " + (ClassID como String)
    • bPrintToScreen: true
    • Text Color: (255, 255, 0) ← Amarelo
```

### **2. Verificar se Border Está Sendo Tornado Visible**

**Adicione após `Set Visibility` do Border:**

```
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible
    ↓
[Get Visibility]
    • Target: Border_WBP_Creator
    ↓
[Print String]
    • In String: "Border_WBP_Creator Visibility: " + (Visibility como String)
    • bPrintToScreen: true
    • Text Color: (0, 255, 0) ← Verde
```

### **3. Verificar se VBox_ClassList Está Sendo Populado**

**Adicione após `Add Child to Vertical Box`:**

```
[Add Child to Vertical Box]
    • Target: VBox_ClassList
    • Content: ClassInfoWidget
    ↓
[Get Children Count]
    • Target: VBox_ClassList
    ↓
[Print String]
    • In String: "VBox_ClassList agora tem " + (Children Count como String) + " children"
    • bPrintToScreen: true
    • Text Color: (0, 255, 0) ← Verde
```

---

## ✅ **CHECKLIST**

- [ ] Função `ShowClassInfo` existe e está implementada
- [ ] `ShowClassInfo` torna `Border_WBP_Creator` Visible
- [ ] `ShowClassInfo` popula `VBox_ClassList` com as informações da classe
- [ ] `ShowClassInfo` é chamado após `SelectClassAndMoveCamera`
- [ ] `ShowClassInfo` é chamado em TODOS os lugares onde uma classe é selecionada
- [ ] `BTN_Return` torna `Border_WBP_Creator` Collapsed
- [ ] Logs mostram que `ShowClassInfo` está sendo executado
- [ ] Logs mostram que o Border está sendo tornado Visible
- [ ] Logs mostram que o VBox está sendo populado

---

## 🎯 **RESUMO**

**O problema é simples:** Após o reset, quando você seleciona uma nova classe:
1. O `Border_WBP_Creator` não está sendo tornado **Visible**
2. O `VBox_ClassList` não está sendo populado com as informações da classe

**A solução:**
1. Criar função `ShowClassInfo` que:
   - Torna `Border_WBP_Creator` **Visible**
   - Popula `VBox_ClassList` com as informações da classe selecionada
2. Chamar `ShowClassInfo` **TODA VEZ** que uma classe for selecionada
3. Garantir que `BTN_Return` torna o Border **Collapsed**

---

## 📝 **ESTRUTURA COMPLETA DO FLUXO**

```
[Classe Selecionada]
    ↓
[SelectClassAndMoveCamera]
    • Return Value: Success
    ↓
[Branch] (Success = True)
    ↓
[ShowClassInfo]
    • ClassID: [ID da classe]
    ↓
[Set Visibility] Border_WBP_Creator → Visible
    ↓
[Clear Children] VBox_ClassList
    ↓
[Get Class Data By ID]
    ↓
[Create Widget] WBP_ClassSelectorItem
    ↓
[SetClassData] (no widget criado)
    ↓
[Add Child to Vertical Box] VBox_ClassList
```

**E no Return:**

```
[BTN_Return OnClicked]
    ↓
[Set Visibility] Border_WBP_Creator → Collapsed
    ↓
[Reset Character Creation]
    ↓
[SpawnAllPlaceholders]
```

---

**Fim do Guia**

