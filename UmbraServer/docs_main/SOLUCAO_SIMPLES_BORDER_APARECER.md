# 🔧 **SOLUÇÃO SIMPLES: Border_WBP_Creator Aparecer Quando Selecionar Classe**

## 🎯 **OBJETIVO**

Fazer o `Border_WBP_Creator` aparecer (ficar **Visible**) quando você selecionar uma classe.

---

## ✅ **SOLUÇÃO: Adicionar Set Visibility Após Seleção**

### **PASSO 1: Encontrar Onde a Classe é Selecionada**

**No `WBP_CreateCharacter`, procure onde `SelectClassAndMoveCamera` é chamado:**

- Botões de classe (BTN_Class1, BTN_Class2, etc.)
- Event handler `OnClassSelected`
- Qualquer lugar onde uma classe é selecionada

---

### **PASSO 2: Adicionar Set Visibility Após SelectClassAndMoveCamera**

**Após `SelectClassAndMoveCamera`, adicione:**

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
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible ← ISSO AQUI!
```

---

### **PASSO 3: Exemplo Completo em um Botão**

**No `BTN_Class1 OnClicked` (ou qualquer botão de classe):**

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
[Get] Border_WBP_Creator
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible
```

**Repita isso para TODOS os botões de classe (BTN_Class1, BTN_Class2, etc.)**

---

### **PASSO 4: Se Usar Event Handler**

**Se você usa um evento `OnClassSelected` ou similar:**

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
[Get] Border_WBP_Creator
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible
```

---

### **PASSO 5: Garantir que Border Fica Collapsed no Return**

**No `BTN_Return OnClicked`, certifique-se de que o Border fica Collapsed:**

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

## ✅ **RESUMO**

**Só precisa fazer isso:**

1. **Após `SelectClassAndMoveCamera`** (em TODOS os lugares onde uma classe é selecionada):
   - Adicionar `Get Border_WBP_Creator`
   - Adicionar `Set Visibility` → **Visible**

2. **No `BTN_Return`**:
   - Adicionar `Set Visibility` → **Collapsed**

**Pronto!**

---

**Fim do Guia**

