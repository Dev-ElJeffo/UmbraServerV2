# 📋 GUIA COMPLETO: Preencher WBP_CharacterInfo com Dados da API

## 🎯 **OBJETIVO:**

Criar a função `Update Character Info Display` que preenche TODAS as variáveis do widget com os dados retornados pela API `get_character_info.php`.

---

## 📊 **VARIÁVEIS IDENTIFICADAS:**

### **Header:**
- `Text_Name` - Nome do personagem
- `Text_Level` - Nível (precisa ser criado se não existir)
- `Text_EXP` - EXP atual / EXP necessário
- `ProgressBar_EXP` - Barra de progresso de EXP

### **Character Info (Direita):**
- `Text_Class` - Classe (precisa ser criado)
- `Text_Faction` - Facção (precisa ser criado)
- `Text_Guild` - Guilda (precisa ser criado)
- `Text_Title` - Título (precisa ser criado)
- `Text_PVP` - PvP (precisa ser criado)
- `Text_Chaos` - Chaos (precisa ser criado)
- `Text_Honor` - Honor (precisa ser criado)

### **Status (Esquerda):**
- `Text_Strength` - Força (Base / Total)
- `Text_Dexterity` - Destreza (Base / Total)
- `Text_Intelligence` - Inteligência (Base / Total)
- `Text_Vitality` - Vitalidade (Base / Total)
- `Text_Luck` - Sorte (Base / Total)
- `Text_Health` - Vida (Current / Max)
- `ProgressBar_Health` - Barra de vida
- `Text_Mana` - Mana (Current / Max)
- `ProgressBar_Mana` - Barra de mana

### **Combat Stats (Centro e Direita):**
- `Text_PhysAtk` - Ataque Físico
- `Text_MagAtk` - Ataque Mágico
- `Text_PhysDef` - Defesa Física
- `Text_MagDef` - Defesa Mágica
- `Text_MoveSpeed` - Velocidade de Movimento
- `Text_CritRate` - Taxa de Crítico
- `Text_DoubleRate` - Taxa de Ataque Duplo
- `Text_CritRes` - Resistência a Crítico
- `Text_DoubleRes` - Resistência a Ataque Duplo
- `Text_Accuracy` - Precisão
- `Text_Dodge` - Esquiva

---

## 🔧 **PARTE 1: Criar Função Update Character Info Display**

### **1.1 Criar a Função:**

1. **No Blueprint Editor:**
   - Abra `WBP_CharacterInfo`
   - Vá para a aba **Functions**
   - Clique em **+ Function**
   - Nome: `Update Character Info Display`

2. **Adicionar Input:**
   - Clique em **Input** na função
   - Adicione: `Character Info` (tipo: **Umbra Character Info**)

---

## 🔧 **PARTE 2: Implementar a Função - Passo a Passo**

### **2.1 Break Umbra Character Info**

**NO EVENT GRAPH DA FUNÇÃO:**

1. **Break Umbra Character Info:**
   - Clique com botão direito → **Break Umbra Character Info**
   - Conecte **Character Info** (input) → **Character Info** (Break)

---

### **2.2 Atualizar Header (Nome, Nível, EXP)**

#### **Nome do Personagem:**

```
[Entry] (função)
  ↓
[Set Text]
  ├─ Target: Text_Name
  └─ Text: Character Name (Break)
```

**NÓS NECESSÁRIOS:**
- **Set Text** (TextBlock)
- **Target:** `Text_Name` (variável)
- **Text:** `Character Name` (do Break)

#### **Nível:**

```
[Set Text] (anterior)
  ↓
[Format Text]
  ├─ Format: "Nível: {Level}"
  └─ Level: Level (Break)
  ↓
[Set Text]
  ├─ Target: Text_Level
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "Nível: {Level}"
  - **Level:** `Level` (do Break)
- **Set Text**
  - **Target:** `Text_Level`
  - **Text:** Return Value (Format Text)

#### **EXP (Texto):**

```
[Format Text]
  ├─ Format: "{Experience} / {ExpForNextLevel}"
  ├─ Experience: Experience (Break)
  └─ ExpForNextLevel: Exp For Next Level (Break)
  ↓
[Set Text]
  ├─ Target: Text_EXP
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "{Experience} / {ExpForNextLevel}"
  - **Experience:** `Experience` (do Break)
  - **ExpForNextLevel:** `Exp For Next Level` (do Break)
- **Set Text**
  - **Target:** `Text_EXP`
  - **Text:** Return Value (Format Text)

#### **Progress Bar EXP:**

```
[Set Percent]
  ├─ Target: ProgressBar_EXP
  └─ Percent: Exp Progress Percent (Break) / 100.0
```

**NÓS NECESSÁRIOS:**
- **Set Percent** (Progress Bar)
  - **Target:** `ProgressBar_EXP`
  - **Percent:** `Exp Progress Percent` (do Break) → **Divide** por 100.0

**OU:**

```
[Exp Progress Percent] (Break)
  ↓
[Divide] (Float)
  ├─ A: Exp Progress Percent
  └─ B: 100.0 (Make Literal Float)
  ↓
[Set Percent]
  ├─ Target: ProgressBar_EXP
  └─ Percent: Return Value (Divide)
```

---

### **2.3 Atualizar Character Info (Class, Faction, Guild, Title, PvP)**

#### **Class:**

```
[Set Text]
  ├─ Target: Text_Class
  └─ Text: Class Name (Break)
```

**NÓS NECESSÁRIOS:**
- **Set Text**
  - **Target:** `Text_Class` (criar se não existir)
  - **Text:** `Class Name` (do Break)

#### **Faction:**

```
[Set Text]
  ├─ Target: Text_Faction
  └─ Text: Faction Name (Break)
```

**NÓS NECESSÁRIOS:**
- **Set Text**
  - **Target:** `Text_Faction` (criar se não existir)
  - **Text:** `Faction Name` (do Break)

#### **Guild:**

```
[Set Text]
  ├─ Target: Text_Guild
  └─ Text: Guild Name (Break)
```

**NÓS NECESSÁRIOS:**
- **Set Text**
  - **Target:** `Text_Guild` (criar se não existir)
  - **Text:** `Guild Name` (do Break)

#### **Title:**

```
[Set Text]
  ├─ Target: Text_Title
  └─ Text: Title Name (Break)
```

**NÓS NECESSÁRIOS:**
- **Set Text**
  - **Target:** `Text_Title` (criar se não existir)
  - **Text:** `Title Name` (do Break)

#### **PvP:**

```
[Format Text]
  ├─ Format: "PVP {PvP}"
  └─ PvP: PvP (Break)
  ↓
[Set Text]
  ├─ Target: Text_PVP
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "PVP {PvP}"
  - **PvP:** `PvP` (do Break)
- **Set Text**
  - **Target:** `Text_PVP` (criar se não existir)
  - **Text:** Return Value (Format Text)

#### **Chaos:**

```
[Format Text]
  ├─ Format: "Chaos {Chaos}"
  └─ Chaos: Chaos (Break)
  ↓
[Set Text]
  ├─ Target: Text_Chaos
  └─ Text: Return Value (Format Text)
```

#### **Honor:**

```
[Format Text]
  ├─ Format: "Honor {Honor}"
  └─ Honor: Honor (Break)
  ↓
[Set Text]
  ├─ Target: Text_Honor
  └─ Text: Return Value (Format Text)
```

---

### **2.4 Atualizar Status (Atributos Base e Totais)**

#### **Strength:**

```
[Format Text]
  ├─ Format: "Strength {Base} / {Total}"
  ├─ Base: Base Strength (Break)
  └─ Total: Total Strength (Break)
  ↓
[Set Text]
  ├─ Target: Text_Strength
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "Strength {Base} / {Total}"
  - **Base:** `Base Strength` (do Break)
  - **Total:** `Total Strength` (do Break)
- **Set Text**
  - **Target:** `Text_Strength`
  - **Text:** Return Value (Format Text)

#### **Dexterity:**

```
[Format Text]
  ├─ Format: "Dexterity {Base} / {Total}"
  ├─ Base: Base Dexterity (Break)
  └─ Total: Total Dexterity (Break)
  ↓
[Set Text]
  ├─ Target: Text_Dexterity
  └─ Text: Return Value (Format Text)
```

#### **Intelligence:**

```
[Format Text]
  ├─ Format: "Intelligence {Base} / {Total}"
  ├─ Base: Base Intelligence (Break)
  └─ Total: Total Intelligence (Break)
  ↓
[Set Text]
  ├─ Target: Text_Intelligence
  └─ Text: Return Value (Format Text)
```

#### **Vitality:**

```
[Format Text]
  ├─ Format: "Vitality {Base} / {Total}"
  ├─ Base: Base Vitality (Break)
  └─ Total: Total Vitality (Break)
  ↓
[Set Text]
  ├─ Target: Text_Vitality
  └─ Text: Return Value (Format Text)
```

#### **Luck:**

```
[Format Text]
  ├─ Format: "Luck {Base} / {Total}"
  ├─ Base: Base Luck (Break)
  └─ Total: Total Luck (Break)
  ↓
[Set Text]
  ├─ Target: Text_Luck
  └─ Text: Return Value (Format Text)
```

---

### **2.5 Atualizar Health (Vida)**

#### **Text Health:**

```
[Format Text]
  ├─ Format: "Health {Current} / {Max}"
  ├─ Current: Current Health (Break)
  └─ Max: Max Health Total (Break)
  ↓
[Set Text]
  ├─ Target: Text_Health
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "Health {Current} / {Max}"
  - **Current:** `Current Health` (do Break)
  - **Max:** `Max Health Total` (do Break)
- **Set Text**
  - **Target:** `Text_Health`
  - **Text:** Return Value (Format Text)

#### **Progress Bar Health:**

```
[Current Health] (Break)
  ↓
[Divide] (Float)
  ├─ A: Current Health
  └─ B: Max Health Total (Break)
  ↓
[Set Percent]
  ├─ Target: ProgressBar_Health
  └─ Percent: Return Value (Divide)
```

**NÓS NECESSÁRIOS:**
- **Divide** (Float)
  - **A:** `Current Health` (do Break)
  - **B:** `Max Health Total` (do Break)
- **Set Percent** (Progress Bar)
  - **Target:** `ProgressBar_Health`
  - **Percent:** Return Value (Divide)

---

### **2.6 Atualizar Mana**

#### **Text Mana:**

```
[Format Text]
  ├─ Format: "Mana {Current} / {Max}"
  ├─ Current: Current Mana (Break)
  └─ Max: Max Mana Total (Break)
  ↓
[Set Text]
  ├─ Target: Text_Mana
  └─ Text: Return Value (Format Text)
```

#### **Progress Bar Mana:**

```
[Current Mana] (Break)
  ↓
[Divide] (Float)
  ├─ A: Current Mana
  └─ B: Max Mana Total (Break)
  ↓
[Set Percent]
  ├─ Target: ProgressBar_Mana
  └─ Percent: Return Value (Divide)
```

---

### **2.7 Atualizar Combat Stats**

#### **Physical Attack:**

```
[Format Text]
  ├─ Format: "Phys. Atk {Value}"
  └─ Value: Physical Attack (Break)
  ↓
[Set Text]
  ├─ Target: Text_PhysAtk
  └─ Text: Return Value (Format Text)
```

**NÓS NECESSÁRIOS:**
- **Format Text**
  - **Format:** "Phys. Atk {Value}"
  - **Value:** `Physical Attack` (do Break)
- **Set Text**
  - **Target:** `Text_PhysAtk`
  - **Text:** Return Value (Format Text)

#### **Magic Attack:**

```
[Format Text]
  ├─ Format: "Mag. Atk {Value}"
  └─ Value: Magic Attack (Break)
  ↓
[Set Text]
  ├─ Target: Text_MagAtk
  └─ Text: Return Value (Format Text)
```

#### **Physical Defense:**

```
[Format Text]
  ├─ Format: "Phys. Def {Value}"
  └─ Value: Physical Defense (Break)
  ↓
[Set Text]
  ├─ Target: Text_PhysDef
  └─ Text: Return Value (Format Text)
```

#### **Magic Defense:**

```
[Format Text]
  ├─ Format: "Mag. Def {Value}"
  └─ Value: Magic Defense (Break)
  ↓
[Set Text]
  ├─ Target: Text_MagDef
  └─ Text: Return Value (Format Text)
```

#### **Move Speed:**

```
[Format Text]
  ├─ Format: "Move Speed {Value}"
  └─ Value: Movement (Break)
  ↓
[Set Text]
  ├─ Target: Text_MoveSpeed
  └─ Text: Return Value (Format Text)
```

#### **Critical Rate:**

```
[Format Text]
  ├─ Format: "Critical Atk {Value}"
  └─ Value: Critical (Break)
  ↓
[Set Text]
  ├─ Target: Text_CritRate
  └─ Text: Return Value (Format Text)
```

#### **Double Rate:**

```
[Format Text]
  ├─ Format: "Double Atk {Value}"
  └─ Value: Double Attack Rate (Break)
  ↓
[Set Text]
  ├─ Target: Text_DoubleRate
  └─ Text: Return Value (Format Text)
```

#### **Critical Resistance:**

```
[Format Text]
  ├─ Format: "Critical Res {Value}"
  └─ Value: Resistance (Break)
  ↓
[Set Text]
  ├─ Target: Text_CritRes
  └─ Text: Return Value (Format Text)
```

**NOTA:** `Resistance` pode não ser exatamente "Critical Resistance". Verifique se há um campo específico ou use `Resistance` como está.

#### **Double Resistance:**

```
[Format Text]
  ├─ Format: "Double Res {Value}"
  └─ Value: (pode não existir, usar 0 ou Resistance)
  ↓
[Set Text]
  ├─ Target: Text_DoubleRes
  └─ Text: Return Value (Format Text)
```

**NOTA:** Se não houver campo específico, pode usar `Resistance` ou deixar como 0.

#### **Accuracy:**

```
[Format Text]
  ├─ Format: "Accuracy {Value}"
  └─ Value: Accuracy (Break)
  ↓
[Set Text]
  ├─ Target: Text_Accuracy
  └─ Text: Return Value (Format Text)
```

#### **Dodge:**

```
[Format Text]
  ├─ Format: "Dodge {Value}"
  └─ Value: Dodge (Break)
  ↓
[Set Text]
  ├─ Target: Text_Dodge
  └─ Text: Return Value (Format Text)
```

---

## 🔧 **PARTE 3: Ordem de Execução**

### **3.1 Estrutura Completa da Função:**

```
[Entry] (Update Character Info Display)
  ↓
[Break Umbra Character Info]
  └─ Character Info: Character Info (input)
  ↓
[Set Text] → Text_Name
  ↓
[Format Text] → "Nível: {Level}" → [Set Text] → Text_Level
  ↓
[Format Text] → "{Experience} / {ExpForNextLevel}" → [Set Text] → Text_EXP
  ↓
[Divide] → Exp Progress Percent / 100.0 → [Set Percent] → ProgressBar_EXP
  ↓
[Set Text] → Text_Class
  ↓
[Set Text] → Text_Faction
  ↓
[Set Text] → Text_Guild
  ↓
[Set Text] → Text_Title
  ↓
[Format Text] → "PVP {PvP}" → [Set Text] → Text_PVP
  ↓
[Format Text] → "Chaos {Chaos}" → [Set Text] → Text_Chaos
  ↓
[Format Text] → "Honor {Honor}" → [Set Text] → Text_Honor
  ↓
[Format Text] → "Strength {Base} / {Total}" → [Set Text] → Text_Strength
  ↓
[Format Text] → "Dexterity {Base} / {Total}" → [Set Text] → Text_Dexterity
  ↓
[Format Text] → "Intelligence {Base} / {Total}" → [Set Text] → Text_Intelligence
  ↓
[Format Text] → "Vitality {Base} / {Total}" → [Set Text] → Text_Vitality
  ↓
[Format Text] → "Luck {Base} / {Total}" → [Set Text] → Text_Luck
  ↓
[Format Text] → "Health {Current} / {Max}" → [Set Text] → Text_Health
  ↓
[Divide] → Current Health / Max Health Total → [Set Percent] → ProgressBar_Health
  ↓
[Format Text] → "Mana {Current} / {Max}" → [Set Text] → Text_Mana
  ↓
[Divide] → Current Mana / Max Mana Total → [Set Percent] → ProgressBar_Mana
  ↓
[Format Text] → "Phys. Atk {Value}" → [Set Text] → Text_PhysAtk
  ↓
[Format Text] → "Mag. Atk {Value}" → [Set Text] → Text_MagAtk
  ↓
[Format Text] → "Phys. Def {Value}" → [Set Text] → Text_PhysDef
  ↓
[Format Text] → "Mag. Def {Value}" → [Set Text] → Text_MagDef
  ↓
[Format Text] → "Move Speed {Value}" → [Set Text] → Text_MoveSpeed
  ↓
[Format Text] → "Critical Atk {Value}" → [Set Text] → Text_CritRate
  ↓
[Format Text] → "Double Atk {Value}" → [Set Text] → Text_DoubleRate
  ↓
[Format Text] → "Critical Res {Value}" → [Set Text] → Text_CritRes
  ↓
[Format Text] → "Double Res {Value}" → [Set Text] → Text_DoubleRes
  ↓
[Format Text] → "Accuracy {Value}" → [Set Text] → Text_Accuracy
  ↓
[Format Text] → "Dodge {Value}" → [Set Text] → Text_Dodge
```

---

## 📋 **CHECKLIST DE VARIÁVEIS:**

### **Variáveis que JÁ EXISTEM (confirmadas na imagem):**
- ✅ `Text_Vitality`
- ✅ `Text_Strength`
- ✅ `Text_PhysDef`
- ✅ `Text_PhysAtk`
- ✅ `Text_Name`
- ✅ `Text_MoveSpeed`
- ✅ `Text_Mana`
- ✅ `Text_MagDef`
- ✅ `Text_MagAtk`
- ✅ `Text_Luck`
- ✅ `Text_Intelligence`
- ✅ `Text_Health`
- ✅ `Text_EXP`
- ✅ `Text_DoubleRes`
- ✅ `Text_DoubleRate`
- ✅ `Text_Dodge`
- ✅ `Text_Dexterity`
- ✅ `Text_CritRes`
- ✅ `Text_CritRate`
- ✅ `Text_Accuracy`
- ✅ `ProgressBar_Mana`
- ✅ `ProgressBar_Health`
- ✅ `ProgressBar_EXP`
- ✅ `Btn_Close`
- ✅ `Border_TitleBar`

### **Variáveis que PRECISAM SER CRIADAS:**
- ❌ `Text_Level` - Criar no Designer
- ❌ `Text_Class` - Criar no Designer
- ❌ `Text_Faction` - Criar no Designer
- ❌ `Text_Guild` - Criar no Designer
- ❌ `Text_Title` - Criar no Designer
- ❌ `Text_PVP` - Criar no Designer
- ❌ `Text_Chaos` - Criar no Designer
- ❌ `Text_Honor` - Criar no Designer

---

## 🔧 **PARTE 4: Criar Variáveis Faltantes**

### **4.1 No Designer:**

1. **Text_Level:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Level`
   - Posicione ao lado de "Name"

2. **Text_Class:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Class`
   - Posicione na seção "Character" (direita)

3. **Text_Faction:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Faction`
   - Posicione abaixo de Class

4. **Text_Guild:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Guild`
   - Posicione abaixo de Faction

5. **Text_Title:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Title`
   - Posicione abaixo de Guild

6. **Text_PVP:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_PVP`
   - Posicione abaixo de Title

7. **Text_Chaos:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Chaos`
   - Posicione abaixo de PVP

8. **Text_Honor:**
   - Adicione **TextBlock** no Designer
   - Nome: `Text_Honor`
   - Posicione abaixo de Chaos

---

## 🔧 **PARTE 5: Conectar ao Event OnCharacterInfoLoaded**

### **5.1 No Event Graph:**

```
[OnCharacterInfoLoaded_Event]
  └─ Character Info: (FUmbraCharacterInfo)
  ↓
[Update Character Info Display]
  └─ Character Info: Character Info (input)
```

**NÓS NECESSÁRIOS:**
- **OnCharacterInfoLoaded_Event** (já criado anteriormente)
- **Update Character Info Display** (função que acabamos de criar)
- Conecte **Character Info** (input do evento) → **Character Info** (input da função)

---

## 📝 **RESUMO RÁPIDO:**

### **Ordem de Implementação:**

1. ✅ Criar variáveis faltantes no Designer (8 TextBlocks)
2. ✅ Criar função `Update Character Info Display`
3. ✅ Adicionar input `Character Info` na função
4. ✅ Adicionar `Break Umbra Character Info`
5. ✅ Implementar todas as atualizações de TextBlocks
6. ✅ Implementar todas as atualizações de Progress Bars
7. ✅ Conectar função ao evento `OnCharacterInfoLoaded_Event`

---

## ✅ **PRONTO!**

Após seguir este guia, todas as variáveis do widget serão preenchidas automaticamente quando os dados forem carregados da API.

---

## 🎯 **DICAS:**

1. **Format Text:** Use para formatar números e strings
   - Exemplo: "Strength {Base} / {Total}"

2. **Divide:** Use para calcular percentuais
   - Exemplo: Current Health / Max Health Total

3. **Set Percent:** Valores entre 0.0 e 1.0
   - Exemplo: 0.75 = 75%

4. **Ordem:** Execute todas as atualizações em sequência, conectando uma após a outra

5. **Teste:** Após implementar, teste abrindo o widget (tecla C) e verifique se todos os valores aparecem corretamente

