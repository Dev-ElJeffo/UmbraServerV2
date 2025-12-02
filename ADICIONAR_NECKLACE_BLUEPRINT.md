# 🔧 ADICIONAR NECKLACE AO BLUEPRINT

## ✅ **MUDANÇAS NO C++:**

1. ✅ Adicionado `Necklace` ao enum `EUmbraEquipmentSlot` em `UmbraDataStructures.h`
2. ✅ Adicionado mapeamento `"necklace"` → `EUmbraEquipmentSlot::Necklace` em `ParseEquipmentSlot`

---

## 🔧 **MUDANÇAS NO BLUEPRINT:**

### **1. Criar Variável Slot_Necklace**

**No `WBP_CharacterInfo`:**

1. Abra **My Blueprint** → **Variables**
2. Adicione variável:
   - Nome: `Slot_Necklace`
   - Tipo: `WBP Equipment Slot` (Object Reference)
   - Instance Editable: ✅

---

### **2. Criar Slot_Necklace no CreateEquipmentSlots**

**No `WBP_CharacterInfo`, função `CreateEquipmentSlots`:**

Adicione um bloco para criar o slot de Necklace (igual aos outros slots):

```
[Create Widget]
  ├─ Class: WBP_EquipmentSlot
  ├─ Owning Player: Get Player Controller (Index 0)
  └─ Return Value: (WBP Equipment Slot)
       ↓
[Knot] ← Use Knot para reutilizar Return Value
  └─ Return Value
       ├─ ↓ (conexão 1)
       │  [Set Slot Type]
       │    ├─ Target: Return Value (do Knot)
       │    └─ Slot Type: Necklace ← NOVO!
       │         ↓
       │    [Add Child to Uniform Grid]
       │      ├─ Target: Get Grid_EquipmentSlots
       │      ├─ Content: Return Value (do Knot)
       │      ├─ Column: 2 (ou o que for apropriado)
       │      └─ Row: 2 (ou o que for apropriado)
       │           ↓
       │      [Set Slot_Necklace] ← NOVO!
       │        └─ Slot_Necklace: Return Value (do Knot)
       │
       └─ ↓ (conexão 2 - para próximo slot, se houver)
```

**IMPORTANTE:** Copie a estrutura EXATA de `Slot_Ring` ou `Slot_Amulet` e apenas mude:
- `Slot Type`: `Ring` → `Necklace`
- `Set Slot_Ring` → `Set Slot_Necklace`
- `Column` e `Row` conforme seu layout

---

### **3. Adicionar Case Necklace no Switch**

**No `WBP_CharacterInfo`, função `Update Equipment Slots`:**

No `Switch on EUmbraEquipmentSlot`, adicione um case para `Necklace`:

```
[Switch on EUmbraEquipmentSlot]
  ├─ Case: Necklace ← ADICIONAR ESTE CASE!
  │    ↓
  │  [Get Slot_Necklace]
  │    ↓
  │  [Is Valid] (Slot_Necklace) ← ADICIONAR VERIFICAÇÃO!
  │    ├─ True:
  │    │    ↓
  │    │  [Update Slot Visual]
  │    │    ├─ Target: Slot_Necklace
  │    │    └─ Item Slot: Inventory Slot
  │    │
  │    └─ False:
  │         ↓
  │    [Print String] → "ERRO: Slot_Necklace é None!" (opcional)
```

**COMO ADICIONAR O CASE:**
1. No `Switch on EUmbraEquipmentSlot`, clique com botão direito no Switch
2. Selecione **"Add Pin"** ou **"Add Case"**
3. Escolha `Necklace` do dropdown
4. Conecte `Get Slot_Necklace` → `Is Valid` → `Branch` → `Update Slot Visual`

---

## 📋 **ORDEM DOS SLOTS NO ENUM:**

Agora o enum tem:
- Ring
- Amulet
- **Necklace** ← NOVO!
- Earring
- Bracelet
- Mount

---

## ✅ **RESUMO:**

1. ✅ C++ já atualizado (enum + ParseEquipmentSlot)
2. ⚠️ Blueprint precisa:
   - Criar variável `Slot_Necklace`
   - Criar slot no `CreateEquipmentSlots`
   - Adicionar case `Necklace` no `Switch` do `Update Equipment Slots`

**Recompile o C++ primeiro, depois atualize o Blueprint!**

