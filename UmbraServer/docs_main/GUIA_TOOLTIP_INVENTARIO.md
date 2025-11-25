# 🎯 GUIA COMPLETO: TOOLTIP DO INVENTÁRIO

## 📋 **O QUE VAMOS CRIAR**

Um tooltip que aparece quando você passa o mouse sobre um item, mostrando:
- 📦 Nome do item
- 📝 Descrição
- ⭐ Raridade (com cor)
- 🔢 Quantidade
- 💰 Valor
- ⚔️ Stats (Damage, Defense, etc.)
- 🔧 Durabilidade

---

## 🏗️ **ARQUITETURA**

```
WBP_InventorySlot (onde o mouse passa)
    ↓ (OnMouseEnter)
    ↓ Cria
WBP_ItemTooltip (widget do tooltip)
    ↓ (Recebe SlotData)
    ↓ Mostra
Informações do Item
```

---

## 📝 **PASSO 1: CRIAR O WIDGET DO TOOLTIP**

### **1.1 - Criar novo Widget Blueprint:**

1. **Content Browser** → Clique com botão direito
2. **User Interface** → **Widget Blueprint**
3. Nome: `WBP_ItemTooltip`
4. Abra o widget

---

### **1.2 - Designer do WBP_ItemTooltip:**

Crie esta estrutura na aba **Designer**:

```
Canvas Panel
└── Border (Background)
    └── Vertical Box (Main Container)
        ├── Text Block (Text_ItemName)        - Nome do item
        ├── Text Block (Text_Rarity)          - Raridade
        ├── Spacer (8px)
        ├── Text Block (Text_Description)     - Descrição
        ├── Spacer (8px)
        ├── Horizontal Box (Stats Container)
        │   ├── Text Block (Text_StatsLabel)  - "Stats:"
        │   └── Vertical Box (Stats List)
        │       ├── Text Block (Text_Damage)
        │       ├── Text Block (Text_Defense)
        │       └── Text Block (Text_Value)
        └── Horizontal Box (Footer)
            ├── Text Block (Text_Quantity)
            └── Text Block (Text_Durability)
```

---

### **1.3 - Configurações de Estilo:**

#### **Border (Background):**
- **Brush Color:** `(R=0.05, G=0.05, B=0.05, A=0.95)` (preto semi-transparente)
- **Padding:** `10, 10, 10, 10`

#### **Text_ItemName:**
- **Font Size:** `18`
- **Color:** `(R=1, G=1, B=1, A=1)` (branco)
- **Font Style:** **Bold**

#### **Text_Rarity:**
- **Font Size:** `14`
- **Color:** Será definido dinamicamente no Blueprint

#### **Text_Description:**
- **Font Size:** `12`
- **Color:** `(R=0.7, G=0.7, B=0.7, A=1)` (cinza claro)
- **Auto Wrap Text:** ✅ **True**
- **Wrap Text At:** `200`

#### **Text_StatsLabel:**
- **Font Size:** `12`
- **Color:** `(R=1, G=0.8, B=0, A=1)` (amarelo)
- **Text:** `"Stats:"`

#### **Stats (Damage, Defense, Value):**
- **Font Size:** `12`
- **Color:** `(R=0.8, G=0.8, B=0.8, A=1)`

#### **Footer (Quantity, Durability):**
- **Font Size:** `11`
- **Color:** `(R=0.6, G=0.6, B=0.6, A=1)`

---

### **1.4 - Marcar como Variables:**

Na aba **Designer**, selecione cada Text Block e marque:
- ✅ **Is Variable** (para todos os Text Blocks)

---

## 🔧 **PASSO 2: LÓGICA DO TOOLTIP (BLUEPRINT)**

### **2.1 - Criar Variável para Dados:**

Na aba **Graph** de `WBP_ItemTooltip`:

1. **Add Variable** → Nome: `TooltipData`
2. **Type:** `UmbraInventorySlot` (struct)
3. **Instance Editable:** ❌ False
4. **Expose on Spawn:** ✅ **True**

---

### **2.2 - Criar Função: SetTooltipData**

1. **Functions** → **+ (Add Function)**
2. Nome: `SetTooltipData`

**Inputs:**
- `InSlotData` (type: `UmbraInventorySlot`)

**Lógica:**

```
[SetTooltipData]
    │
    ├─> [Set TooltipData] = InSlotData
    │
    ├─> [Break UmbraInventorySlot] ← TooltipData
    │       │
    │       ├─ ItemTemplate → [Break UmbraItemTemplate]
    │       │                       │
    │       │                       ├─ ItemName → [Set Text (Text_ItemName)]
    │       │                       ├─ Description → [Set Text (Text_Description)]
    │       │                       ├─ Rarity → [UpdateRarityColor]
    │       │                       ├─ Value → [Format Text] → [Set Text (Text_Value)]
    │       │                       └─ Stats → [UpdateStats]
    │       │
    │       ├─ Quantity → [Format Text] → [Set Text (Text_Quantity)]
    │       └─ Durability → [Format Text] → [Set Text (Text_Durability)]
    │
    └─> [End]
```

---

### **2.3 - Criar Função: UpdateRarityColor**

**Purpose:** Define a cor do texto de raridade baseado no enum.

**Inputs:**
- `Rarity` (type: `EUmbraItemRarity`)

**Lógica:**

```
[UpdateRarityColor]
    │
    ├─> [Switch on EUmbraItemRarity] ← Rarity
    │       │
    │       ├─ Common → [Make Linear Color] (R=0.6, G=0.6, B=0.6, A=1) → [Set Color]
    │       ├─ Uncommon → [Make Linear Color] (R=0, G=1, B=0, A=1) → [Set Color]
    │       ├─ Rare → [Make Linear Color] (R=0, G=0.5, B=1, A=1) → [Set Color]
    │       ├─ Epic → [Make Linear Color] (R=0.7, G=0, B=1, A=1) → [Set Color]
    │       └─ Legendary → [Make Linear Color] (R=1, G=0.5, B=0, A=1) → [Set Color]
    │
    └─> [Get Rarity Text] → [Set Text (Text_Rarity)]
```

**Helper Function: GetRarityText**

```
[Switch on EUmbraItemRarity]
    ├─ Common → "Common"
    ├─ Uncommon → "Uncommon"
    ├─ Rare → "Rare"
    ├─ Epic → "Epic"
    └─ Legendary → "Legendary"
```

---

### **2.4 - Criar Função: UpdateStats**

**Purpose:** Mostra os stats do item (Damage, Defense, etc.)

**Inputs:**
- `Stats` (type: `UmbraItemStats` struct)

**Lógica:**

```
[UpdateStats]
    │
    ├─> [Break UmbraItemStats] ← Stats
    │       │
    │       ├─ Damage → [Branch] (if Damage > 0)
    │       │              └─> [Format Text] "⚔️ Damage: {0}"
    │       │                   └─> [Set Text (Text_Damage)]
    │       │
    │       ├─ Defense → [Branch] (if Defense > 0)
    │       │              └─> [Format Text] "🛡️ Defense: {0}"
    │       │                   └─> [Set Text (Text_Defense)]
    │       │
    │       └─ Value → [Format Text] "💰 Value: {0} gold"
    │                   └─> [Set Text (Text_Value)]
    │
    └─> [End]
```

---

## 🎮 **PASSO 3: INTEGRAR COM WBP_InventorySlot**

### **3.1 - Adicionar Variável no WBP_InventorySlot:**

1. Abra `WBP_InventorySlot`
2. **Add Variable** → Nome: `TooltipWidget`
3. **Type:** `WBP_ItemTooltip` (Object Reference)
4. **Default Value:** `None`

---

### **3.2 - Override: OnMouseEnter**

Na aba **Graph** de `WBP_InventorySlot`:

1. **Override** → **On Mouse Enter**

**Lógica:**

```
[OnMouseEnter]
    │
    ├─> [Branch] (Condition: InventoryID > 0)
    │       │
    │       └─ TRUE:
    │           │
    │           ├─> [Create Widget] (Class: WBP_ItemTooltip)
    │           │       │
    │           │       └─ Return Value → [Set TooltipWidget]
    │           │
    │           ├─> [TooltipWidget] → [SetTooltipData] (InSlotData = SlotData)
    │           │
    │           ├─> [TooltipWidget] → [Add to Viewport]
    │           │       └─ Z-Order: 999
    │           │
    │           └─> [Set Position in Viewport]
    │                   ├─ Widget: TooltipWidget
    │                   ├─ Position: [Get Mouse Position] + Offset(10, 10)
    │                   └─ Remove at: false
    │
    └─> FALSE: (do nothing)
```

---

### **3.3 - Override: OnMouseLeave**

```
[OnMouseLeave]
    │
    ├─> [Is Valid] (TooltipWidget)
    │       │
    │       └─ TRUE:
    │           │
    │           ├─> [TooltipWidget] → [Remove from Parent]
    │           │
    │           └─> [Set TooltipWidget] = None
    │
    └─> [End]
```

---

### **3.4 - Override: OnMouseMove** (Opcional - Tooltip seguir o mouse)

```
[OnMouseMove]
    │
    ├─> [Is Valid] (TooltipWidget)
    │       │
    │       └─ TRUE:
    │           │
    │           └─> [Set Position in Viewport]
    │                   ├─ Widget: TooltipWidget
    │                   └─ Position: [Get Mouse Position] + Offset(10, 10)
    │
    └─> [End]
```

---

## 🎨 **PASSO 4: MELHORIAS VISUAIS (OPCIONAL)**

### **4.1 - Adicionar Animação de Fade In:**

1. Na aba **Animations** de `WBP_ItemTooltip`
2. **+ Animation** → Nome: `FadeIn`
3. **Track:** `Border` → `Render Opacity`
   - **Keyframe 0.0s:** Opacity = `0`
   - **Keyframe 0.2s:** Opacity = `1`

4. No **Event Graph**, adicione ao final de `SetTooltipData`:
   ```
   [Play Animation] (FadeIn, PlayMode: Forward, PlaybackSpeed: 1.0)
   ```

---

### **4.2 - Adicionar Borda Colorida por Raridade:**

No `WBP_ItemTooltip`, adicione outro **Border** ao redor do principal:

```
Border (Outer Border)
├── Brush Color: Definir dinamicamente (baseado em Rarity)
├── Padding: 2, 2, 2, 2
└── Border (Inner Background) ... (resto do layout)
```

No Blueprint, na função `UpdateRarityColor`, adicione:

```
[Get Rarity Color] → [Set Brush Color (Outer Border)]
```

---

## 🧪 **PASSO 5: TESTAR**

### **Checklist:**

1. ✅ Compile e salve `WBP_ItemTooltip`
2. ✅ Compile e salve `WBP_InventorySlot`
3. ✅ Abra o jogo
4. ✅ Abra o inventário
5. ✅ **Passe o mouse sobre um item**

**Resultado esperado:**
- Tooltip aparece ao lado do mouse
- Mostra nome, raridade, descrição, stats
- Tooltip desaparece quando o mouse sai

---

## 🚨 **PROBLEMAS COMUNS**

### **Problema 1: Tooltip não aparece**

**Causa:** `OnMouseEnter` não está sendo chamado

**Solução:**
1. No `WBP_InventorySlot`, verifique se o componente raiz (ou um painel) tem:
   - **Visibility:** `Visible`
   - **Is Enabled:** ✅ True

---

### **Problema 2: Tooltip aparece mas está vazio**

**Causa:** `SlotData` não tem `ItemTemplate` preenchido

**Solução:**
1. Verifique se `ParseInventorySlot` no C++ está funcionando
2. Adicione logs de debug em `SetTooltipData`:
   ```
   Print String: "ItemName: {ItemTemplate.ItemName}"
   ```

---

### **Problema 3: Tooltip aparece na posição errada**

**Causa:** `Set Position in Viewport` não funciona bem com coordenadas do mouse

**Solução:** Use `Add to Viewport` com **Alignment** ajustado, ou crie um **Canvas Panel** no `WBP_Inventory` para posicionar o tooltip.

---

## 📊 **BLUEPRINT CODE - OnMouseEnter (COMPLETO)**

```cpp
Begin Object Class=/Script/UMGEditor.WidgetBlueprint Name="WBP_InventorySlot"
    // OnMouseEnter Event
    Begin Object Class=/Script/BlueprintGraph.K2Node_Event Name="OnMouseEnter_Event"
        EventReference=(MemberName="OnMouseEnter")
        NodePosX=100
        NodePosY=100
        
        // Check if slot has item
        CustomProperties Pin Name="Condition" -> Get SlotData -> Break -> InventoryID -> > (0)
        
        // TRUE Branch: Create Tooltip
        CustomProperties Pin Name="then" -> Branch
            // Create Widget
            -> Create Widget (Class: WBP_ItemTooltip)
            -> Set TooltipWidget
            
            // Set Data
            -> TooltipWidget -> SetTooltipData (InSlotData = SlotData)
            
            // Add to Viewport
            -> Add to Viewport (Z-Order: 999)
            
            // Position
            -> Set Position in Viewport
                Widget: TooltipWidget
                Position: Get Mouse Position + (10, 10)
    End Object
End Object
```

---

## 📝 **RESUMO**

✅ **Criado:** `WBP_ItemTooltip` (widget do tooltip)
✅ **Função:** `SetTooltipData` (popula as informações)
✅ **Cores:** Raridade com cores diferentes
✅ **Events:** `OnMouseEnter`, `OnMouseLeave`, `OnMouseMove`
✅ **Integrado:** Com `WBP_InventorySlot`

---

## 🚀 **PRÓXIMO PASSO: DRAG & DROP**

Após implementar o tooltip, vamos para o sistema de **Drag and Drop**!

---

**Boa implementação! 🎨**

