# 🎯 **GUIA: Completar Botão Create com ClassID, Hair e Head**

## 📋 **OBJETIVO**

Atualizar o botão `BTN_Create` para:
1. Obter o `ClassID` selecionado do `UmbraGameInstance`
2. Adicionar seletores de `Hair` (1-10) e `Head` (1-10) no widget
3. Passar todos os parâmetros (`CharacterName`, `ClassID`, `Hair`, `Head`) para `CreateCharacter`

---

## ✅ **PARTE 1: Adicionar Seletores Hair e Head no Widget**

### **PASSO 1.1: Adicionar Variáveis no Widget**

1. **Abra `WBP_CreateCharacter`**
2. No painel **My Blueprint** → **Variables**, adicione:
   - **`SpinBox_Hair`** (tipo: `SpinBox` Reference)
   - **`SpinBox_Head`** (tipo: `SpinBox` Reference)

### **PASSO 1.2: Adicionar SpinBoxes no Designer**

1. **No Designer do `WBP_CreateCharacter`**:
   - Arraste **SpinBox** do painel **Palette** para o canvas
   - Posicione abaixo do campo `TXT_Name`
   - Renomeie para **`SpinBox_Hair`**
   - Configure:
     - **Min Value**: `1`
     - **Max Value**: `10`
     - **Value**: `1` (valor padrão)
     - **Delta**: `1`
   - Adicione um **TextBlock** ao lado com o texto "Hair:"

2. **Repita para Head**:
   - Arraste outro **SpinBox**
   - Renomeie para **`SpinBox_Head`**
   - Configure os mesmos valores (Min: 1, Max: 10, Value: 1)

### **PASSO 1.3: Conectar Variáveis**

1. **Selecione `SpinBox_Hair`** no Designer
2. No **Details**, marque **Is Variable**
3. Repita para `SpinBox_Head`

---

## ✅ **PARTE 2: Obter ClassID Selecionado**

### **PASSO 2.1: No Event Graph do Botão Create**

**ANTES** de chamar `CreateCharacter`, adicione:

```
[On Clicked] (BTN_Create)
    ↓
[Get] TXT_Name
    ↓
[Get] Text
    ↓
[Convert Text to String]
    ↓
[Trim]
    ↓
[Is Empty]
    ↓
[Branch]
    • Condition: (Is Empty result) ← INVERTIDO (True = vazio = erro)
    ↓ (False) ← Nome não está vazio, pode continuar
    [Get Game Instance]
        ↓
    [Cast to Umbra Game Instance]
        ↓ (Success)
        [Get Selected Class ID]
            • Target: (Cast result)
            • Return Value: SelectedClassID
        ↓
        [Branch]
            • Condition: SelectedClassID > 0
            ↓ (True) ← Classe foi selecionada
            [Get] SpinBox_Hair
                ↓
            [Get] Value
                ↓
            [Get] SpinBox_Head
                ↓
            [Get] Value
                ↓
            [Call Function: Create Character]
                • Target: MyGameInstance
                • Character Name: (Trim result)
                • ClassID: SelectedClassID ← NOVO
                • Hair: (SpinBox_Hair Value) ← NOVO
                • Head: (SpinBox_Head Value) ← NOVO
            ↓ (False) ← Classe não selecionada
            [Print String]
                • In String: "Por favor, selecione uma classe primeiro!"
            ↓ (True) ← Nome vazio
            [Print String]
                • In String: "Por favor, insira um nome válido!"
```

---

## 📝 **ESTRUTURA COMPLETA DO BOTÃO CREATE**

```
[On Clicked] (BTN_Create)
    ↓
[Get] TXT_Name
    ↓
[Get] Text
    ↓
[Convert Text to String]
    ↓
[Trim]
    ↓
[Is Empty]
    ↓
[Branch] ← Verifica se nome está vazio
    • Condition: (Is Empty result)
    ↓ (False) ← Nome válido
    [Get Game Instance]
        ↓
    [Cast to Umbra Game Instance]
        ↓ (Success)
        [Get Selected Class ID]
            • Return Value: SelectedClassID
        ↓
        [Branch] ← Verifica se classe foi selecionada
            • Condition: SelectedClassID > 0
            ↓ (True) ← Classe selecionada
            [Get] SpinBox_Hair
                ↓
            [Get] Value
                • Return Value: HairValue
            ↓
            [Get] SpinBox_Head
                ↓
            [Get] Value
                • Return Value: HeadValue
            ↓
            [Get] MyGameInstance
                ↓
            [Call Function: Create Character]
                • Target: MyGameInstance
                • Character Name: (Trim result)
                • ClassID: SelectedClassID ← OBTIDO DO GAME INSTANCE
                • Hair: HairValue ← OBTIDO DO SPINBOX
                • Head: HeadValue ← OBTIDO DO SPINBOX
            ↓
            [Set Is Enabled]
                • Target: BTN_Create
                • b In Is Enabled: false
            ↓
            [Set Text]
                • Target: TXT_Validation
                • Text: "Criando..."
            ↓
            [Set Color and Opacity]
                • Target: TXT_Validation
                • Color: (cor roxa/magenta)
            ↓ (False) ← Classe não selecionada
            [Print String]
                • In String: "Por favor, selecione uma classe primeiro!"
            ↓ (True) ← Nome vazio
            [Print String]
                • In String: "Por favor, insira um nome válido!"
```

**✅ NOTA:** A função `CreateCharacter` já aceita os parâmetros `ClassID`, `Hair` e `Head` no C++.

---

## 🔧 **PARTE 3: Validações Adicionais (Opcional)**

### **PASSO 3.1: Validar Hair e Head**

Você pode adicionar validações para garantir que `Hair` e `Head` estão entre 1 e 10:

```
[Get] SpinBox_Hair
    ↓
[Get] Value
    ↓
[Branch]
    • Condition: (Value >= 1 AND Value <= 10)
    ↓ (True)
    [Get] SpinBox_Head
        ↓
    [Get] Value
        ↓
    [Branch]
        • Condition: (Value >= 1 AND Value <= 10)
        ↓ (True)
        [Call Function: Create Character]
            • Hair: HairValue
            • Head: HeadValue
        ↓ (False)
        [Print String]
            • In String: "Head deve estar entre 1 e 10!"
    ↓ (False)
    [Print String]
        • In String: "Hair deve estar entre 1 e 10!"
```

**NOTA:** Como os SpinBoxes já têm Min=1 e Max=10 configurados, essa validação é opcional, mas pode ser útil para garantir.

---

## 📌 **RESUMO DAS MUDANÇAS**

### **1. Widget Designer:**
- ✅ Adicionar `SpinBox_Hair` (Min: 1, Max: 10, Value: 1)
- ✅ Adicionar `SpinBox_Head` (Min: 1, Max: 10, Value: 1)
- ✅ Marcar ambos como **Is Variable**

### **2. Event Graph:**
- ✅ Obter `SelectedClassID` usando `Get Selected Class ID` do `UmbraGameInstance`
- ✅ Obter `Hair` usando `Get Value` do `SpinBox_Hair`
- ✅ Obter `Head` usando `Get Value` do `SpinBox_Head`
- ✅ Passar todos os parâmetros para `CreateCharacter`:
  - `CharacterName` (do `TXT_Name`)
  - `ClassID` (do `GetSelectedClassID`)
  - `Hair` (do `SpinBox_Hair`)
  - `Head` (do `SpinBox_Head`)

---

## 🎯 **EXEMPLO VISUAL DO LAYOUT**

```
┌─────────────────────────────────┐
│  Character Name: [________]    │
│                                 │
│  Hair: [▼ 1 ▲]                 │
│  Head: [▼ 1 ▲]                  │
│                                 │
│  [Create]  [Back]               │
└─────────────────────────────────┘
```

---

## ✅ **VERIFICAÇÃO FINAL**

Após implementar, verifique:

1. ✅ `SpinBox_Hair` e `SpinBox_Head` aparecem no widget
2. ✅ Valores padrão são 1
3. ✅ Valores podem ser alterados de 1 a 10
4. ✅ Ao clicar em `Create`, o `ClassID` é obtido do `UmbraGameInstance`
5. ✅ `Hair` e `Head` são obtidos dos SpinBoxes
6. ✅ Todos os parâmetros são passados para `CreateCharacter`

---

## 🔍 **DEBUG**

Se algo não funcionar, adicione `Print String` para verificar valores:

```
[Get Selected Class ID]
    • Return Value: SelectedClassID
    ↓
[Print String]
    • In String: "ClassID selecionado: {SelectedClassID}"
    • {SelectedClassID}: (conecte o SelectedClassID)
    ↓
[Get] SpinBox_Hair
    ↓
[Get] Value
    ↓
[Print String]
    • In String: "Hair: {HairValue}"
    • {HairValue}: (conecte o Value)
```

Isso ajudará a identificar se os valores estão sendo obtidos corretamente.

