# ✅ **SOLUÇÃO FINAL QUE FUNCIONA**

## 🎯 **O QUE PRECISA ACONTECER**

1. **Placeholders respawnam** ✅ (já feito com `SpawnPlaceholdersSimple`)
2. **Quando clicar em uma classe pela segunda vez:**
   - `Border_WBP_Creator` aparece (Visible)
   - `VBox_ClassList` é populado com as informações da classe

---

## 📋 **PASSO 1: Verificar se Existe PopulateClassSelector**

**No `WBP_CreateCharacter`:**
1. Abra o Blueprint
2. **My Blueprint** → **Functions**
3. Procure por `PopulateClassSelector`

**Se EXISTIR:**
- Vá para **PASSO 2A**

**Se NÃO EXISTIR:**
- Vá para **PASSO 2B**

---

## 📋 **PASSO 2A: Se PopulateClassSelector EXISTE**

### **2A.1. Verificar se PopulateClassSelector Aceita ClassID**

**Abra a função `PopulateClassSelector` e verifique se tem um input `ClassID` (Integer).**

**Se TEM ClassID:**
- Vá para **PASSO 3A**

**Se NÃO TEM ClassID:**
- Vá para **PASSO 2A.2**

### **2A.2. Adicionar Input ClassID à PopulateClassSelector**

1. **Abra a função `PopulateClassSelector`**
2. **Clique com botão direito na função** → **Add Input** → **Integer**
3. **Renomeie o pin para `ClassID`**
4. **Dentro da função, modifique o For Each Loop:**

```
[For Each Loop] (LoopBody)
    • Array Element: (FUmbraClassData)
    ↓
[Break Umbra Class Data]
    • Struct: Array Element
    ↓
[Get] ClassID (do Break)
    ↓
[Equal] (Integer)
    • A: ClassID (do Break)
    • B: ClassID (input da função)
    ↓
[Branch]
    • Condition: (Equal result)
    ↓ (True) ← SÓ EXECUTA SE O ClassID CORRESPONDER
    [Create Widget]
        • Class: WBP_ClassSelectorItem (ou o widget que você usa)
    ↓
    [Call Function: SetClassData]
        • Target: (Widget criado)
        • New Class Data: Array Element
    ↓
    [Get] VBox_ClassList
    ↓
    [Add Child to Vertical Box]
        • Target: VBox_ClassList
        • Content: (Widget criado)
    ↓ (False)
    [Continue] ← Pula se não corresponder
```

---

## 📋 **PASSO 3A: Chamar PopulateClassSelector Após Seleção**

**No botão de classe (BTN_Class1, BTN_Class2, etc.) ou onde `SelectClassAndMoveCamera` é chamado:**

```
[SelectClassAndMoveCamera]
    • ClassID: [ID da classe]
    • Return Value: Success
    ↓
[Branch]
    • Condition: Success
    ↓ (True)
[Get] Border_WBP_Creator
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible
    ↓
[Call Function: PopulateClassSelector]
    • Target: Self
    • ClassID: [mesmo ID da classe]
```

---

## 📋 **PASSO 2B: Se PopulateClassSelector NÃO EXISTE - Criar Função Simples**

### **2B.1. Criar Função ShowClassInfo**

1. **My Blueprint** → **Functions** → **+ Function**
2. **Nome:** `ShowClassInfo`
3. **Inputs:**
   - `ClassID` (Integer)

### **2B.2. Implementar ShowClassInfo**

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
    • New Visibility: Visible
    ↓
[Get] VBox_ClassList
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Clear Children]
    • Target: VBox_ClassList
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Get Available Classes]
    • Target: (cast result)
    • Return Value: ClassesArray
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ LoopBody
    • Array Element: (FUmbraClassData)
    ↓
[Break Umbra Class Data]
    • Struct: Array Element
    ↓
[Get] ClassID (do Break)
    ↓
[Equal] (Integer)
    • A: ClassID (do Break)
    • B: ClassID (input da função)
    ↓
[Branch]
    • Condition: (Equal result)
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
        • New Class Data: Array Element
        ↓
    [Get] VBox_ClassList
        ↓
    [Add Child to Vertical Box]
        • Target: VBox_ClassList
        • Content: ClassInfoWidget
    ↓ (False)
    [Continue]
```

---

## 📋 **PASSO 3B: Chamar ShowClassInfo Após Seleção**

**No botão de classe ou onde `SelectClassAndMoveCamera` é chamado:**

```
[SelectClassAndMoveCamera]
    • ClassID: [ID da classe]
    • Return Value: Success
    ↓
[Branch]
    • Condition: Success
    ↓ (True)
[Call Function: ShowClassInfo]
    • Target: Self
    • ClassID: [mesmo ID da classe]
```

---

## 📋 **PASSO 4: Se WBP_ClassSelectorItem NÃO EXISTE - Criar Widget**

### **4.1. Criar Widget Blueprint**

1. **Content Browser** → **Botão direito** → **User Interface** → **Widget Blueprint**
2. **Nome:** `WBP_ClassSelectorItem`
3. **Abrir o widget**

### **4.2. Estrutura do Widget**

**No Designer, adicione:**

```
Canvas Panel (Root)
└── Vertical Box
    ├── Text: TXT_ClassName
    ├── Text: TXT_ClassDescription
    └── (outros elementos que você quer mostrar)
```

### **4.3. Variáveis do Widget**

**My Blueprint** → **Variables**:

1. **`ClassData`** (Umbra Class Data)
   - Category: "Data"

### **4.4. Função SetClassData**

**My Blueprint** → **Functions** → **+ Function**:

- **Nome:** `SetClassData`
- **Inputs:**
  - `NewClassData` (Umbra Class Data)

**Implementação:**

```
[SetClassData]
    • NewClassData (input)
    ↓
[SET] ClassData = NewClassData
    ↓
[Break Umbra Class Data]
    • Struct: NewClassData
    ↓
[Set Text]
    • Target: TXT_ClassName
    • Text: ClassName (do Break)
    ↓
[Set Text]
    • Target: TXT_ClassDescription
    • Text: ClassDescription (do Break, se existir)
```

---

## 📋 **PASSO 5: Garantir que BTN_Return Esconde o Border**

**No `BTN_Return OnClicked`:**

```
[BTN_Return OnClicked]
    ↓
[Get] Border_WBP_Creator
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Collapsed
    ↓
[Get] Manager
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

## ✅ **RESUMO FINAL**

**O que fazer:**

1. **Se `PopulateClassSelector` existe:**
   - Adicionar input `ClassID` (se não tiver)
   - Chamar após `SelectClassAndMoveCamera` com o ClassID
   - Tornar `Border_WBP_Creator` Visible antes

2. **Se `PopulateClassSelector` NÃO existe:**
   - Criar função `ShowClassInfo` (Passo 2B)
   - Chamar após `SelectClassAndMoveCamera`

3. **Se `WBP_ClassSelectorItem` NÃO existe:**
   - Criar widget (Passo 4)
   - Criar função `SetClassData` no widget

4. **No `BTN_Return`:**
   - Tornar `Border_WBP_Creator` Collapsed

**Pronto!**

---

**Fim do Guia**

