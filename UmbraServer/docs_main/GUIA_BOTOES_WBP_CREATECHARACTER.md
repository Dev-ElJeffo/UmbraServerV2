# 📋 **GUIA: Botões de Classe no WBP_CreateCharacter**

## 🎯 **OBJETIVO**

Adicionar botões no `WBP_CreateCharacter` - um para cada classe. Ao clicar, move a câmera para o placeholder correspondente e seleciona a classe.

---

## 📝 **PASSO 1: Estrutura do Widget**

### **1.1. No WBP_CreateCharacter**

**Adicionar na hierarquia:**

```
Canvas Panel (Root)
└─ Horizontal Box: HB_ClassButtons
    ├─ Button: BTN_Class1 (Barbarian)
    ├─ Button: BTN_Class2 (Templar)
    ├─ Button: BTN_Class3 (Assassin)
    ├─ Button: BTN_Class4 (Cleric)
    ├─ Button: BTN_Class5 (DarkMage)
    └─ Button: BTN_Class6 (Monk)
```

**OU usar um Grid/Uniform Grid Panel** se preferir.

---

### **1.2. Configurar Botões**

**Cada botão:**
- **Size:** Width: 150, Height: 50
- **Text:** Nome da classe (ex: "Barbarian", "Templar")
- **Background Color:** Sua escolha

---

## 📝 **PASSO 2: Variáveis do Widget**

### **2.1. No WBP_CreateCharacter, My Blueprint → Variables**

1. **AllPlaceholders** (Array of BP_Class_Placeholder)
   - Category: "References"
   - Default Value: (vazio)

2. **CameraActor** (Actor Reference)
   - Category: "References"
   - Default Value: (vazio)

---

## 📝 **PASSO 3: Event Construct - Inicializar Referências**

### **3.1. No WBP_CreateCharacter, Event Construct**

```
[Event Construct]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    • Out Actors: AllPlaceholders
```

**Só isso!** O Manager será buscado quando necessário na função.

**NOTA:** Se `Get Actor Name` não existir, use a função C++ `FindCameraActor` do Manager.

---

## 📝 **PASSO 4: Criar Função SelectClassByID**

### **4.1. No WBP_CreateCharacter, criar função SelectClassByID**

**Inputs:**
- `ClassID` (Integer)

**Lógica:**

```
[SelectClassByID]
    • ClassID (input)
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    • Return Value: Manager
    ↓
[Is Valid]
    • Object: Manager
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [Get] AllPlaceholders
        ↓
    [For Each Loop]
        • Array: AllPlaceholders
        ↓ Loop Body
        • Array Element: (BP_Class_Placeholder)
        ↓
    [Get] ClassID (do Array Element)
        ↓
    [Equal] (Integer)
        • A: (ClassID do Array Element)
        • B: ClassID (input)
        ↓
    [Branch]
        • Condition: (Equal result)
        ↓ (True)
        [Call Function: RemoveHighlightsFromOthers]
            • Target: Manager
            • SelectedPlaceholder: Array Element
            ↓
        [Call Function: HighlightSelected]
            • Target: Array Element
            ↓
        [Get Game Instance]
            ↓
        [Cast to Umbra Game Instance]
            ↓ (Success)
            [Call Function: Select Class]
                • Target: (Cast result)
                • ClassID: ClassID (input)
                • Return Value: (bSuccess)
                ↓
            [Branch]
                • Condition: bSuccess
                ↓ (True)
                [Call Function: SelectClass]
                    • Target: Array Element
                    • ClassID: ClassID (input)
                    ↓
                [Call Function: Find Camera Actor]
                    • Target: Manager
                    • Return Value: CameraActor
                    ↓
                [Is Valid]
                    • Object: CameraActor
                    ↓
                [Branch]
                    • Condition: (Is Valid)
                    ↓ (True)
                    [Call Function: Move Camera To Selection]
                        • Target: Manager
                        • Target Placeholder: Array Element
                        • Camera Actor: CameraActor
                        • Duration: 1.0
                        • Offset X: 200.0
                        • Offset Z: 100.0
        [Break]
↓ (False)
[Print String]
    • In String: "ERRO: Manager não encontrado!"
    • bPrintToScreen: true
```

---

## 📝 **PASSO 5: Conectar Botões**

### **5.1. BTN_Class1 (Barbarian) - On Clicked**

```
[On Clicked] (BTN_Class1)
    ↓
[Call Function: SelectClassByID]
    • ClassID: 1 (ou o ID correto do Barbarian)
```

### **5.2. BTN_Class2 (Templar) - On Clicked**

```
[On Clicked] (BTN_Class2)
    ↓
[Call Function: SelectClassByID]
    • ClassID: 2 (ou o ID correto do Templar)
```

**Repetir para cada botão com o ClassID correto.**

---

## 📝 **PASSO 6: Remover Widget Component dos Placeholders**

### **6.1. No BP_Class_Placeholder**

1. Selecionar `Widget_ClassInfo` (se existir)
2. **Delete** (Delete ou botão direito → Delete)

**Os placeholders agora são apenas referências visuais e para movimento de câmera.**

---

## ✅ **RESUMO**

1. Botões no `WBP_CreateCharacter` - um para cada classe
2. Cada botão chama `SelectClassByID` com o ClassID correto
3. `SelectClassByID`:
   - Seleciona a classe no Game Instance
   - Encontra o placeholder correspondente
   - Remove highlights dos outros
   - Adiciona highlight no selecionado
   - Move a câmera
4. Placeholders são apenas visuais - sem widgets

---

**Fim do Guia**

