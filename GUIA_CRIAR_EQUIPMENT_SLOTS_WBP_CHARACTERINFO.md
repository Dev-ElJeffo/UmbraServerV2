# 🎯 GUIA: Criar 11 Instâncias de WBP_EquipmentSlot no WBP_CharacterInfo

## ✅ **ONDE CRIAR:**

**No `Event Construct` do `WBP_CharacterInfo`** ou em uma **função customizada** chamada `CreateEquipmentSlots`.

**Recomendação:** Criar uma função `CreateEquipmentSlots` e chamá-la no `Event Construct`.

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Criar Variáveis (se ainda não criou)**

No `WBP_CharacterInfo`, crie as seguintes variáveis:

1. **`Grid_EquipmentSlots`** (Uniform Grid Panel)
   - Tipo: `Uniform Grid Panel`
   - Instance Editable: ✅
   - **Conecte no Designer:** Arraste o UniformGridPanel do Hierarchy para esta variável

2. **`EquipmentSlotWidgets`** (Array) - OPCIONAL
   - Tipo: `Array of WBP Equipment Slot`
   - Para armazenar referências aos slots

3. **Variáveis individuais (OPCIONAL, mas recomendado):**
   - `Slot_Head` (WBP Equipment Slot)
   - `Slot_Chest` (WBP Equipment Slot)
   - `Slot_Hands` (WBP Equipment Slot)
   - `Slot_Feet` (WBP Equipment Slot)
   - `Slot_MainHand` (WBP Equipment Slot)
   - `Slot_OffHand` (WBP Equipment Slot)
   - `Slot_Ring` (WBP Equipment Slot)
   - `Slot_Amulet` (WBP Equipment Slot)
   - `Slot_Earring` (WBP Equipment Slot)
   - `Slot_Bracelet` (WBP Equipment Slot)
   - `Slot_Mount` (WBP Equipment Slot)

---

### **PASSO 2: Criar Função `CreateEquipmentSlots`**

1. No `WBP_CharacterInfo`, Event Graph
2. Clique direito → **Function** → Nome: `CreateEquipmentSlots`

---

### **PASSO 3: Criar os Slots (Um por Um)**

Para cada slot, você precisa:

1. **Create Widget**
   - Class: `WBP_EquipmentSlot`
   - Owning Player: `Get Player Controller` → Index 0

2. **Set Slot Type** (função do WBP_EquipmentSlot)
   - Target: Widget criado
   - Slot Type: O tipo correto (Head, Chest, etc.)

3. **Add Child to Uniform Grid**
   - Target: `Get Grid_EquipmentSlots`
   - Content: Widget criado
   - Column: Posição X (0, 1, 2, etc.)
   - Row: Posição Y (0, 1, 2, etc.)

4. **Set Variable** (se criou variáveis individuais)
   - Variável: `Slot_Head` (ou o slot correspondente)
   - Value: Widget criado

5. **Add to Array** (se estiver usando array)
   - Array: `EquipmentSlotWidgets`
   - Item: Widget criado

---

## 📊 **ESTRUTURA COMPLETA:**

### **Função: CreateEquipmentSlots**

```
[CreateEquipmentSlots]
  ↓
  [SLOT 1: HEAD]
  ├─ [Create Widget]
  │    ├─ Class: WBP_EquipmentSlot
  │    └─ Owning Player: Get Player Controller (Index 0)
  │         │
  │         ▼
  │    [Set Slot Type]
  │    ├─ Target: Widget criado
  │    └─ Slot Type: Head
  │         │
  │         ▼
  │    [Add Child to Uniform Grid]
  │    ├─ Target: Get Grid_EquipmentSlots
  │    ├─ Content: Widget criado
  │    ├─ Column: 0
  │    └─ Row: 0
  │         │
  │         ▼
  │    [Set Slot_Head] ← OPCIONAL
  │    └─ Value: Widget criado
  │
  ├─ [SLOT 2: CHEST]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Chest
  │    ├─ [Add Child to Uniform Grid] → Column: 1, Row: 0
  │    └─ [Set Slot_Chest] ← OPCIONAL
  │
  ├─ [SLOT 3: HANDS]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Hands
  │    ├─ [Add Child to Uniform Grid] → Column: 2, Row: 0
  │    └─ [Set Slot_Hands] ← OPCIONAL
  │
  ├─ [SLOT 4: FEET]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Feet
  │    ├─ [Add Child to Uniform Grid] → Column: 0, Row: 1
  │    └─ [Set Slot_Feet] ← OPCIONAL
  │
  ├─ [SLOT 5: MAIN HAND]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → MainHand
  │    ├─ [Add Child to Uniform Grid] → Column: 1, Row: 1
  │    └─ [Set Slot_MainHand] ← OPCIONAL
  │
  ├─ [SLOT 6: OFF HAND]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → OffHand
  │    ├─ [Add Child to Uniform Grid] → Column: 2, Row: 1
  │    └─ [Set Slot_OffHand] ← OPCIONAL
  │
  ├─ [SLOT 7: RING]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Ring
  │    ├─ [Add Child to Uniform Grid] → Column: 0, Row: 2
  │    └─ [Set Slot_Ring] ← OPCIONAL
  │
  ├─ [SLOT 8: AMULET]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Amulet
  │    ├─ [Add Child to Uniform Grid] → Column: 1, Row: 2
  │    └─ [Set Slot_Amulet] ← OPCIONAL
  │
  ├─ [SLOT 9: EARRING]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Earring
  │    ├─ [Add Child to Uniform Grid] → Column: 2, Row: 2
  │    └─ [Set Slot_Earring] ← OPCIONAL
  │
  ├─ [SLOT 10: BRACELET]
  │    ├─ [Create Widget] → WBP_EquipmentSlot
  │    ├─ [Set Slot Type] → Bracelet
  │    ├─ [Add Child to Uniform Grid] → Column: 0, Row: 3
  │    └─ [Set Slot_Bracelet] ← OPCIONAL
  │
  └─ [SLOT 11: MOUNT]
       ├─ [Create Widget] → WBP_EquipmentSlot
       ├─ [Set Slot Type] → Mount
       ├─ [Add Child to Uniform Grid] → Column: 1, Row: 3
       └─ [Set Slot_Mount] ← OPCIONAL
```

---

## 📐 **LAYOUT DO GRID (3 COLUNAS):**

```
Row 0: [Head] [Chest] [Hands]
Row 1: [Feet] [MainHand] [OffHand]
Row 2: [Ring] [Amulet] [Earring]
Row 3: [Bracelet] [Mount] [ ]
```

**OU (4 COLUNAS):**

```
Row 0: [Head] [Chest] [Hands] [Feet]
Row 1: [MainHand] [OffHand] [Ring] [Amulet]
Row 2: [Earring] [Bracelet] [Mount] [ ]
```

---

## 🔍 **DETALHAMENTO DE UM SLOT (EXEMPLO: HEAD):**

### **NÓ 1: Create Widget**

1. Clique direito → Digite: **`Create Widget`**
2. Configure:
   - **Class:** Selecione `WBP_EquipmentSlot`
   - **Owning Player:** 
     - Clique direito → `Get Player Controller`
     - Index: `0` (Make Literal Int)

---

### **NÓ 2: Set Slot Type**

1. Arraste o **Return Value** do `Create Widget`
2. Digite: **`Set Slot Type`**
3. Configure:
   - **Target:** Return Value do Create Widget
   - **Slot Type:** `Head` (EUmbraEquipmentSlot)

**COMO OBTER "Head":**
- Clique no pin `Slot Type`
- No dropdown, selecione: **`Head`**

---

### **NÓ 3: Add Child to Uniform Grid**

1. Arraste a variável **`Grid_EquipmentSlots`**
2. Selecione: **`Get Grid_EquipmentSlots`**
3. Arraste o pin → Digite: **`Add Child to Uniform Grid`**
4. Configure:
   - **Target:** `Get Grid_EquipmentSlots`
   - **Content:** Return Value do Create Widget
   - **Column:** `0` (Make Literal Int) - Para Head
   - **Row:** `0` (Make Literal Int) - Para Head

---

### **NÓ 4: Set Variable (OPCIONAL)**

1. Arraste a variável **`Slot_Head`**
2. Selecione: **`Set Slot_Head`**
3. Configure:
   - **Slot_Head:** Return Value do Create Widget

---

### **NÓ 5: Add to Array (OPCIONAL)**

1. Arraste a variável **`EquipmentSlotWidgets`**
2. Selecione: **`Add to Array`**
3. Configure:
   - **Array:** `EquipmentSlotWidgets`
   - **Item:** Return Value do Create Widget

---

## 📋 **TABELA DE POSIÇÕES (3 COLUNAS):**

| Slot | Column | Row | Slot Type |
|------|--------|-----|-----------|
| Head | 0 | 0 | Head |
| Chest | 1 | 0 | Chest |
| Hands | 2 | 0 | Hands |
| Feet | 0 | 1 | Feet |
| MainHand | 1 | 1 | MainHand |
| OffHand | 2 | 1 | OffHand |
| Ring | 0 | 2 | Ring |
| Amulet | 1 | 2 | Amulet |
| Earring | 2 | 2 | Earring |
| Bracelet | 0 | 3 | Bracelet |
| Mount | 1 | 3 | Mount |

---

## 🔧 **EVENT CONSTRUCT:**

No `Event Construct` do `WBP_CharacterInfo`:

```
[Event Construct]
  ↓
[CreateEquipmentSlots] ← Chama a função
```

---

## ✅ **RESUMO RÁPIDO:**

1. ✅ Criar função `CreateEquipmentSlots`
2. ✅ Para cada slot (11 slots):
   - `Create Widget` (WBP_EquipmentSlot)
   - `Set Slot Type` (com o tipo correto)
   - `Add Child to Uniform Grid` (com Column e Row corretos)
   - `Set Variable` (opcional, para armazenar referência)
3. ✅ Chamar `CreateEquipmentSlots` no `Event Construct`

---

## 🎯 **PRONTO!**

Agora você sabe exatamente onde e como criar as 11 instâncias de `WBP_EquipmentSlot`! 🎉

