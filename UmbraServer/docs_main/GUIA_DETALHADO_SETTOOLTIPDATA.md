# 🔧 GUIA DETALHADO: Função SetTooltipData

## 📋 **VISÃO GERAL**

A função `SetTooltipData` recebe os dados de um slot do inventário e preenche todos os campos visuais do tooltip (nome, raridade, descrição, stats, etc.).

---

## 🎯 **O QUE A FUNÇÃO FAZ**

```
INPUT: UmbraInventorySlot (struct com todos os dados do item)
   ↓
PROCESSA: Quebra o struct e extrai informações
   ↓
OUTPUT: Atualiza todos os Text Blocks do tooltip
```

---

## 📝 **PASSO A PASSO COMPLETO**

### **PASSO 1: Criar a Função**

1. Abra o widget `WBP_ItemTooltip` no Unreal Editor
2. Vá para a aba **Graph** (Event Graph)
3. Na aba **My Blueprint** (lado esquerdo), clique em **Functions** → **+ (Add Function)**
4. Nome da função: `SetTooltipData`
5. Pressione **Enter**

A função vai aparecer com um nó verde **"SetTooltipData"** (Entry Point).

---

### **PASSO 2: Adicionar Input Parameter**

1. Selecione o nó **SetTooltipData** (Entry Point)
2. No painel **Details** (lado direito), procure a seção **Inputs**
3. Clique em **+ (Add Parameter)**
4. Configure:
   - **Parameter Name:** `InSlotData`
   - **Parameter Type:** `UmbraInventorySlot` (struct)
   - **Pass by Reference:** ✅ **True** (para performance)

Agora o nó de entrada tem um pin de saída chamado `InSlotData`.

---

### **PASSO 3: Quebrar o Struct UmbraInventorySlot**

**Objetivo:** Extrair as informações do slot (ItemTemplate, Quantity, Durability, etc.)

#### **3.1 - Adicionar nó Break Struct:**

1. Arraste do pin `InSlotData` (do nó de entrada)
2. **Solte no espaço vazio**
3. Digite: `Break UmbraInventorySlot`
4. Selecione o nó **Break UmbraInventorySlot**

#### **3.2 - Expandir o nó para mostrar todos os campos:**

1. Clique na **setinha** (▼) no canto superior direito do nó `Break UmbraInventorySlot`
2. Isso vai mostrar **todos os pins** do struct:
   - `InventoryID` (int)
   - `PlayerID` (int)
   - `ItemTemplateID` (int)
   - `Quantity` (int)
   - `SlotIndex` (int)
   - `bIsEquipped` (bool)
   - `Durability` (float)
   - **`ItemTemplate`** (UmbraItemTemplate struct) ⭐ **IMPORTANTE**
   - `AcquiredAt` (string)

---

### **PASSO 4: Quebrar o Struct ItemTemplate**

**Objetivo:** Extrair informações detalhadas do item (Nome, Descrição, Raridade, Stats, Ícone, etc.)

#### **4.1 - Adicionar nó Break ItemTemplate:**

1. Arraste do pin **`ItemTemplate`** do nó `Break UmbraInventorySlot`
2. **Solte no espaço vazio**
3. Digite: `Break UmbraItemTemplate`
4. Selecione o nó **Break UmbraItemTemplate**

#### **4.2 - Expandir o nó:**

1. Clique na **setinha** (▼) no canto superior direito
2. Pins disponíveis:
   - `ItemID` (int)
   - **`ItemName`** (string) ⭐
   - **`Description`** (string) ⭐
   - **`Rarity`** (EUmbraItemRarity enum) ⭐
   - `MaxStack` (int)
   - `Value` (int)
   - **`Stats`** (UmbraItemStats struct) ⭐
   - `ItemIcon` (Texture 2D) - não vamos usar aqui

---

### **PASSO 5: Atualizar o Nome do Item**

**Objetivo:** Mostrar o nome no Text_ItemName

#### **5.1 - Get do componente Text_ItemName:**

1. Na aba **Variables** (lado esquerdo), encontre `Text_ItemName`
2. **Arraste** para o Graph
3. Escolha: **Get Text_ItemName**

#### **5.2 - Set Text:**

1. Arraste do pin de saída de **Get Text_ItemName**
2. Digite: `Set Text`
3. Selecione **Set Text (Text Block)**

#### **5.3 - Conectar:**

```
[Break UmbraItemTemplate]
    └─ ItemName (string) 
        └─> [Set Text (Text_ItemName)]
              └─ In Text
```

**Como conectar:**
1. Arraste do pin **`ItemName`** do nó `Break UmbraItemTemplate`
2. Conecte ao pin **`In Text`** do nó `Set Text (Text_ItemName)`

---

### **PASSO 6: Atualizar a Descrição**

**Similar ao passo anterior:**

```
[Break UmbraItemTemplate]
    └─ Description (string)
        └─> [Get Text_Description]
            └─> [Set Text]
                └─ In Text
```

**Passos:**
1. Arraste `Text_Description` das variáveis → **Get**
2. Arraste do Get → digite `Set Text`
3. Conecte `Description` → `In Text`

---

### **PASSO 7: Atualizar Raridade (com cor)**

**Objetivo:** Mostrar a raridade com cor apropriada (Common = cinza, Rare = azul, etc.)

#### **7.1 - Converter Enum para Texto:**

Precisamos converter o enum `EUmbraItemRarity` para texto legível.

**Opção A - Switch on Enum:**

1. Arraste do pin **`Rarity`** do nó `Break UmbraItemTemplate`
2. Digite: `Switch on EUmbraItemRarity`
3. Selecione o nó **Switch on EUmbraItemRarity**

O nó Switch tem várias saídas (uma para cada valor do enum):
- `Common`
- `Uncommon`
- `Rare`
- `Epic`
- `Legendary`

4. Para cada saída, adicione um nó **Make Literal String** com o texto:
   - Common → `"Common"`
   - Uncommon → `"Uncommon"`
   - Rare → `"Rare"`
   - Epic → `"Epic"`
   - Legendary → `"Legendary"`

5. Use um **Select** ou **Manual Merge** para unificar em uma string final

**Opção B - Usar ToString (mais simples):**

Se o Unreal não tiver conversão automática, você pode criar uma função helper chamada `GetRarityText`.

---

#### **7.2 - Definir Cor da Raridade:**

**Criar nova função auxiliar: GetRarityColor**

1. **Functions** → **+ (Add Function)** → Nome: `GetRarityColor`
2. **Input:** `Rarity` (type: `EUmbraItemRarity`)
3. **Output:** `ReturnValue` (type: `Linear Color`)

**Lógica da função GetRarityColor:**

```
[GetRarityColor Entry] → Rarity
    │
    └─> [Switch on EUmbraItemRarity] ← Rarity
            │
            ├─ Common → [Make Linear Color]
            │              └─ R=0.6, G=0.6, B=0.6, A=1 (Cinza)
            │              └─> [Return Node]
            │
            ├─ Uncommon → [Make Linear Color]
            │              └─ R=0, G=1, B=0, A=1 (Verde)
            │              └─> [Return Node]
            │
            ├─ Rare → [Make Linear Color]
            │              └─ R=0, G=0.5, B=1, A=1 (Azul)
            │              └─> [Return Node]
            │
            ├─ Epic → [Make Linear Color]
            │              └─ R=0.7, G=0, B=1, A=1 (Roxo)
            │              └─> [Return Node]
            │
            └─ Legendary → [Make Linear Color]
                           └─ R=1, G=0.5, B=0, A=1 (Laranja)
                           └─> [Return Node]
```

**Como criar o nó Make Linear Color:**
1. Clique com botão direito no Graph
2. Digite: `Make Linear Color`
3. Nos pins do nó, digite os valores RGB

---

#### **7.3 - Aplicar cor ao Text_Rarity:**

De volta à função `SetTooltipData`:

```
[Break UmbraItemTemplate]
    └─ Rarity
        ├─> [GetRarityColor] (função que criamos)
        │       └─ ReturnValue (Linear Color)
        │           └─> [Get Text_Rarity]
        │               └─> [Set Color and Opacity]
        │                   └─ In Color and Opacity
        │
        └─> [GetRarityText] (função helper)
            └─ ReturnValue (string)
                └─> [Get Text_Rarity]
                    └─> [Set Text]
                        └─ In Text
```

**Passos detalhados:**

1. **Set Color:**
   - Arraste `Rarity` → Chame `GetRarityColor`
   - Arraste do ReturnValue → `Get Text_Rarity` → `Set Color and Opacity`

2. **Set Text:**
   - Arraste `Rarity` novamente → Chame `GetRarityText` (função helper)
   - Arraste do ReturnValue → `Get Text_Rarity` → `Set Text`

---

### **PASSO 8: Atualizar Quantidade**

**Objetivo:** Mostrar "Quantidade: 15" no texto de rodapé

```
[Break UmbraInventorySlot]
    └─ Quantity (int)
        └─> [Format Text]
              └─ Format: "Quantidade: {0}"
              └─ 0: Quantity
              └─> [Get Text_Quantity]
                  └─> [Set Text]
```

**Como fazer:**

1. Arraste do pin **`Quantity`** do `Break UmbraInventorySlot`
2. **Solte no espaço vazio** → Digite: `Format Text`
3. No nó `Format Text`, clique no pin **Format** e digite:
   ```
   Quantidade: {0}
   ```
4. Conecte `Quantity` ao pin **`0`** (primeiro argumento)
5. Arraste `Text_Quantity` das variáveis → **Get**
6. Do Get, arraste → `Set Text`
7. Conecte o resultado do `Format Text` ao `In Text`

---

### **PASSO 9: Atualizar Durabilidade**

**Objetivo:** Mostrar "Durabilidade: 85%"

```
[Break UmbraInventorySlot]
    └─ Durability (float)
        └─> [Format Text]
              └─ Format: "Durabilidade: {0}%"
              └─ 0: Durability
              └─> [Get Text_Durability]
                  └─> [Set Text]
```

**Igual ao passo anterior, mas com `Durability`.**

---

### **PASSO 10: Atualizar Stats (Damage, Defense, Value)**

**Objetivo:** Mostrar os stats do item (se houver)

#### **10.1 - Quebrar o struct Stats:**

```
[Break UmbraItemTemplate]
    └─ Stats (UmbraItemStats struct)
        └─> [Break UmbraItemStats]
                ├─ Damage (int)
                ├─ Defense (int)
                ├─ AttackSpeed (float)
                ├─ CritChance (float)
                └─ (outros stats)
```

**Como fazer:**
1. Arraste do pin **`Stats`** do `Break UmbraItemTemplate`
2. Digite: `Break UmbraItemStats`
3. Expanda o nó (seta ▼)

---

#### **10.2 - Mostrar Damage (se > 0):**

**Lógica:** Só mostrar se o item tiver dano.

```
[Break UmbraItemStats]
    └─ Damage (int)
        └─> [Branch] (Condition: Damage > 0)
                │
                ├─ TRUE:
                │   └─> [Format Text] "⚔️ Damage: {0}"
                │       └─> [Get Text_Damage]
                │           └─> [Set Text]
                │
                └─ FALSE:
                    └─> [Get Text_Damage]
                        └─> [Set Text] ""  (vazio)
```

**Passos:**

1. **Comparar Damage > 0:**
   - Arraste do pin `Damage`
   - Digite: `> (Greater)`
   - No segundo pin do `>`, digite: `0`

2. **Branch:**
   - Arraste do resultado do `>` 
   - Digite: `Branch`

3. **TRUE (tem damage):**
   - Do pin `True` do Branch → `Format Text`
   - Format: `⚔️ Damage: {0}`
   - Argumento 0: `Damage`
   - Resultado → `Get Text_Damage` → `Set Text`

4. **FALSE (sem damage):**
   - Do pin `False` do Branch → `Get Text_Damage` → `Set Text`
   - Deixe `In Text` vazio (ou conecte uma string vazia)

---

#### **10.3 - Repetir para Defense e Value:**

Use a mesma lógica:

**Defense:**
```
Defense > 0 ? 
    Format Text "🛡️ Defense: {Defense}" 
    : ""
```

**Value:**
```
Format Text "💰 Value: {Value} gold"
```

---

### **PASSO 11: Conectar Fluxo de Execução**

**IMPORTANTE:** Todos os nós de `Set Text` e `Set Color` precisam ser conectados em **sequência** pelo fluxo de execução (pins brancos).

**Ordem sugerida:**

```
[SetTooltipData Entry]
    │ (execution pin)
    ├─> [Set Text (Text_ItemName)]
    │       │
    │       └─> [Set Text (Text_Description)]
    │               │
    │               └─> [Set Text (Text_Rarity)]
    │                       │
    │                       └─> [Set Color (Text_Rarity)]
    │                               │
    │                               └─> [Branch] (Damage > 0?)
    │                                       ├─ TRUE → [Set Text (Text_Damage)]
    │                                       │               │
    │                                       │               └─> [Branch] (Defense > 0?)
    │                                       │
    │                                       └─ FALSE → [Set Text (Text_Damage)] (vazio)
    │                                                       │
    │                                                       └─> [Branch] (Defense > 0?)
    │                                                               │
    │                                                               └─> ... (continua)
```

**Como conectar:**

1. Do nó **SetTooltipData Entry**, arraste do pin de **execução** (branco, à direita)
2. Conecte ao primeiro nó `Set Text`
3. Do pin de saída de execução desse `Set Text`, conecte ao próximo
4. Continue conectando todos em cadeia

**Dica:** Use nós **Sequence** para organizar melhor:

```
[Entry] → [Sequence]
              ├─ Then 0 → [Atualizar Textos]
              ├─ Then 1 → [Atualizar Cores]
              └─ Then 2 → [Atualizar Stats]
```

---

## 📊 **ESTRUTURA VISUAL COMPLETA**

```
┌─────────────────────────────────────────────────────────────────┐
│                    SetTooltipData Function                       │
└─────────────────────────────────────────────────────────────────┘

[Entry] InSlotData (UmbraInventorySlot)
    │
    ├─> [Break UmbraInventorySlot]
    │       ├─ Quantity ──────┬──> [Format Text] → [Set Text_Quantity]
    │       ├─ Durability ────┼──> [Format Text] → [Set Text_Durability]
    │       └─ ItemTemplate ──┘
    │               │
    │               └─> [Break UmbraItemTemplate]
    │                       ├─ ItemName ──────> [Set Text_ItemName]
    │                       ├─ Description ───> [Set Text_Description]
    │                       ├─ Rarity ────┬──> [GetRarityText] → [Set Text_Rarity]
    │                       │             └──> [GetRarityColor] → [Set Color_Rarity]
    │                       └─ Stats ─────> [Break UmbraItemStats]
    │                                           ├─ Damage ──> [Branch > 0?] → [Set Text_Damage]
    │                                           ├─ Defense ─> [Branch > 0?] → [Set Text_Defense]
    │                                           └─ Value ───> [Format Text] → [Set Text_Value]
    │
    └─> [Return]
```

---

## 🎨 **EXEMPLO PRÁTICO - MINI VERSÃO**

Se você quer começar simples, crie uma versão mínima primeiro:

### **SetTooltipData - Versão Simples:**

```
[Entry] InSlotData
    │
    └─> [Break UmbraInventorySlot]
            └─ ItemTemplate
                └─> [Break UmbraItemTemplate]
                        ├─ ItemName → [Set Text (Text_ItemName)]
                        └─ Description → [Set Text (Text_Description)]
```

**Teste isso primeiro!** Depois adicione os outros campos.

---

## 🧪 **TESTAR A FUNÇÃO**

Para testar se a função está funcionando:

1. No **Event Graph** de `WBP_ItemTooltip`, adicione:

```
[Event Construct]
    │
    └─> [Make UmbraInventorySlot] (struct com dados fictícios)
            └─> [SetTooltipData]
```

2. Compile e abra o widget no Preview
3. Verifique se os textos aparecem

---

## 🚨 **PROBLEMAS COMUNS**

### **Problema 1: "Break UmbraInventorySlot não mostra ItemTemplate"**

**Solução:** Clique na **setinha** (▼) no canto do nó para **mostrar pins avançados**.

---

### **Problema 2: "Não consigo encontrar o componente Text_ItemName"**

**Solução:** 
1. Vá para a aba **Designer**
2. Selecione o Text Block
3. Marque **"Is Variable"** no painel Details
4. Volte para a aba **Graph**

---

### **Problema 3: "Format Text não funciona"**

**Solução:** Use esta sintaxe:
```
Format: "Quantidade: {Quantity}"
```

E clique em **"Create Binding"** para criar um pin de entrada chamado `Quantity`.

---

### **Problema 4: "Enum não converte para String"**

**Solução:** Crie uma função helper `GetRarityText`:

```cpp
[Switch on EUmbraItemRarity]
    ├─ Common → Return "Common"
    ├─ Uncommon → Return "Uncommon"
    ├─ Rare → Return "Rare"
    ├─ Epic → Return "Epic"
    └─ Legendary → Return "Legendary"
```

---

## 📝 **CHECKLIST FINAL**

Antes de considerar a função completa, verifique:

- [ ] Função `SetTooltipData` criada
- [ ] Input `InSlotData` adicionado (UmbraInventorySlot)
- [ ] `Break UmbraInventorySlot` funcionando
- [ ] `Break UmbraItemTemplate` funcionando
- [ ] `Text_ItemName` atualizado
- [ ] `Text_Description` atualizado
- [ ] `Text_Rarity` atualizado (texto e cor)
- [ ] `Text_Quantity` atualizado
- [ ] `Text_Durability` atualizado
- [ ] Stats (Damage, Defense, Value) atualizados
- [ ] Fluxo de execução conectado
- [ ] Função compila sem erros
- [ ] Testado com dados fictícios

---

## 🎯 **RESUMO RÁPIDO**

**Entrada:** Struct `UmbraInventorySlot`
**Processamento:**
1. Quebrar `UmbraInventorySlot` → pegar `ItemTemplate`, `Quantity`, `Durability`
2. Quebrar `ItemTemplate` → pegar `ItemName`, `Description`, `Rarity`, `Stats`
3. Quebrar `Stats` → pegar `Damage`, `Defense`, `Value`
4. Atualizar cada Text Block com `Set Text`
5. Atualizar cor da raridade com `Set Color and Opacity`
**Saída:** Tooltip completamente preenchido

---

**Boa implementação! 🚀**

