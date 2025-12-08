# 🔍 **COPIA EXATA: O Que Acontece na Primeira Vez**

## 🎯 **OBJETIVO**

Descobrir EXATAMENTE o que acontece na primeira vez que funciona e fazer a MESMA COISA na segunda vez.

---

## 📋 **PASSO 1: Verificar o Que Acontece na Primeira Vez**

### **1.1. No Event Construct**

**Abra `WBP_CreateCharacter` → Event Graph → Event Construct**

**Me diga:**
- O que está conectado ao `Event Construct`?
- Há algum código que mostra o `Border_WBP_Creator`?
- Há algum código que chama `PopulateClassSelector` ou similar?

**OU me envie uma screenshot do Event Construct.**

---

### **1.2. Quando Você Clica em um Botão de Classe (Primeira Vez)**

**Abra o botão (ex: BTN_Class1) → OnClicked**

**Me diga:**
- O que está conectado ao `OnClicked`?
- Há algum código que mostra o `Border_WBP_Creator`?
- Há algum código que popula o `VBox_ClassList`?

**OU me envie uma screenshot do OnClicked do botão.**

---

### **1.3. Verificar se Há Algum Delegate ou Event**

**Procure por:**
- `OnClassSelected`
- `OnClassesLoaded`
- Qualquer Custom Event relacionado a classe

**Me diga se existe algum e o que ele faz.**

---

## 📋 **PASSO 2: Solução Direta - Criar Função Única**

**Vamos criar UMA função que faz TUDO e chamar ela sempre que uma classe for selecionada.**

### **2.1. Criar Função ShowClassSelection**

1. **My Blueprint** → **Functions** → **+ Function**
2. **Nome:** `ShowClassSelection`
3. **Inputs:**
   - `ClassID` (Integer)

### **2.2. Implementar ShowClassSelection**

**Copie EXATAMENTE isso:**

```
[ShowClassSelection]
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
    • B: ClassID (input)
    ↓
[Branch]
    • Condition: (Equal result)
    ↓ (True)
    [Get Player Controller] (Index: 0)
        ↓
    [Create Widget]
        • Class: WBP_ClassSelectorItem
        • Owning Player: (Get Player Controller)
        • Return Value: ClassWidget
        ↓
    [Call Function: SetClassData]
        • Target: ClassWidget
        • New Class Data: Array Element
        ↓
    [Get] VBox_ClassList
        ↓
    [Add Child to Vertical Box]
        • Target: VBox_ClassList
        • Content: ClassWidget
    ↓ (False)
    [Continue]
```

---

## 📋 **PASSO 3: Chamar ShowClassSelection SEMPRE**

### **3.1. No Botão de Classe**

**No `BTN_Class1 OnClicked` (e TODOS os outros botões):**

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
[Call Function: ShowClassSelection]
    • Target: Self
    • ClassID: 1
```

**Repita para TODOS os botões (BTN_Class1, BTN_Class2, etc.) com o ClassID correto.**

---

### **3.2. Se Usar Event Handler**

**Se você tem um evento `OnClassSelected` ou similar:**

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
[Call Function: ShowClassSelection]
    • Target: Self
    • ClassID: ClassID
```

---

## 📋 **PASSO 4: Se WBP_ClassSelectorItem NÃO Existe**

### **4.1. Criar Widget**

1. **Content Browser** → **Botão direito** → **User Interface** → **Widget Blueprint**
2. **Nome:** `WBP_ClassSelectorItem`
3. **Abrir**

### **4.2. No Designer**

**Adicione:**
- **Canvas Panel** (Root)
- **Vertical Box** dentro do Canvas
- **Text Block** chamado `TXT_ClassName`
- **Text Block** chamado `TXT_ClassDescription` (opcional)

### **4.3. Variável**

**My Blueprint** → **Variables**:
- **`ClassData`** (Umbra Class Data)

### **4.4. Função SetClassData**

**My Blueprint** → **Functions** → **+ Function**:
- **Nome:** `SetClassData`
- **Input:** `NewClassData` (Umbra Class Data)

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
```

---

## 📋 **PASSO 5: BTN_Return**

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

## ⚠️ **IMPORTANTE**

**Se `WBP_ClassSelectorItem` não existe e você não quer criar agora:**

**Na função `ShowClassSelection`, em vez de `Create Widget`, você pode:**

1. **Criar os elementos diretamente no VBox_ClassList** (Text Blocks, etc.)
2. **OU usar uma função que já existe** que popula o VBox

**Me diga qual você prefere.**

---

## ✅ **TESTE**

1. **Compile o Blueprint**
2. **Execute o jogo**
3. **Selecione uma classe** → Border deve aparecer
4. **Clique Return** → Border deve sumir
5. **Selecione outra classe** → Border deve aparecer novamente

**Se não funcionar, me diga EXATAMENTE em qual passo parou e o que aconteceu.**

---

**Fim do Guia**

