# 🔧 ADICIONAR SLOT NECKLACE NO BLUEPRINT - PASSO A PASSO

## ❌ **PROBLEMA:**
O item "Colar do Mestre" está equipado no banco de dados, mas o ícone não aparece porque o Blueprint não tem o slot `Necklace` configurado.

## ✅ **SOLUÇÃO:**

### **PASSO 1: Criar Variável Slot_Necklace**

1. Abra o Blueprint `WBP_CharacterInfo`
2. Vá para **My Blueprint** (painel esquerdo)
3. Clique em **Variables** → **+ (Add Variable)**
4. Configure:
   - **Variable Name:** `Slot_Necklace`
   - **Variable Type:** `WBP Equipment Slot` (Object Reference)
   - **Instance Editable:** ✅ (marcado)
5. **Compile** o Blueprint

---

### **PASSO 2: Adicionar Slot_Necklace no CreateEquipmentSlots**

1. No `WBP_CharacterInfo`, abra a função **`CreateEquipmentSlots`**
2. **Localize** onde os outros slots de acessórios são criados (Ring, Amulet, Earring)
3. **Copie** a estrutura completa de um slot existente (por exemplo, `Slot_Ring` ou `Slot_Amulet`)
4. **Cole** após o slot anterior (provavelmente após `Slot_Amulet`)
5. **Modifique** o slot copiado:
   - Mude `Set Slot Type` → **Slot Type:** `Necklace` (em vez de `Ring` ou `Amulet`)
   - Mude `Set Slot_Ring` (ou `Set Slot_Amulet`) → **`Set Slot_Necklace`**
   - Ajuste **Column** e **Row** conforme o layout (provavelmente **Column: 1, Row: 1** ou similar, dependendo da posição desejada)

**Estrutura esperada:**
```
[Create Widget]
  ├─ Class: WBP_EquipmentSlot
  ├─ Owning Player: Get Player Controller (Index 0)
  └─ Return Value: (WBP Equipment Slot)
       ↓
[Knot] ← Use Knot para reutilizar Return Value
  └─ Return Value
       ├─ ↓
       │  [Set Slot Type]
       │    ├─ Target: Return Value (do Knot)
       │    └─ Slot Type: Necklace ← MUDAR PARA NECKLACE!
       │         ↓
       │    [Add Child to Uniform Grid]
       │      ├─ Target: Get Grid_EquipmentSlots
       │      ├─ Content: Return Value (do Knot)
       │      ├─ Column: 1 (ajustar conforme layout)
       │      └─ Row: 1 (ajustar conforme layout)
       │           ↓
       │      [Set Slot_Necklace] ← MUDAR PARA Slot_Necklace!
       │        └─ Slot_Necklace: Return Value (do Knot)
```

6. **Compile** o Blueprint

---

### **PASSO 3: Adicionar Case Necklace no Switch do UpdateEquipmentSlots**

1. No `WBP_CharacterInfo`, abra a função **`Update Equipment Slots`**
2. **Localize** o nó **`Switch on EUmbraEquipmentSlot`**
3. **Clique com botão direito** no `Switch`
4. Selecione **"Add Pin"** ou **"Add Case"**
5. **Escolha** `Necklace` do dropdown
6. **Conecte** os nós na seguinte ordem:

```
[Switch on EUmbraEquipmentSlot]
  ├─ Case: Necklace ← NOVO CASE!
  │    ↓
  │  [Get Slot_Necklace] ← Variável criada no PASSO 1
  │    ↓
  │  [Is Valid] (Slot_Necklace) ← Verificar se não é None
  │    ├─ True:
  │    │    ↓
  │    │  [Update Slot Visual]
  │    │    ├─ Target: Slot_Necklace (do Get)
  │    │    └─ Item Slot: Inventory Slot (do ForEach Loop)
  │    │
  │    └─ False:
  │         ↓
  │    [Print String] → "ERRO: Slot_Necklace é None!" (opcional, para debug)
```

**COMO OBTER OS NÓS:**

- **Get Slot_Necklace:**
  - Arraste do pin `Slot_Necklace` (variável) ou digite "Get Slot_Necklace"
  
- **Is Valid:**
  - Digite "Is Valid"
  - Conecte `Slot_Necklace` (do Get) ao pin de entrada
  
- **Update Slot Visual:**
  - Digite "Update Slot Visual"
  - **Target:** Conecte `Slot_Necklace` (do Get)
  - **Item Slot:** Conecte `Inventory Slot` (do `ForEach Loop` do `Get Equipped Items Array`)

**IMPORTANTE:** O `Item Slot` vem do `ForEach Loop` que itera sobre `Get Equipped Items Array`. Certifique-se de que o case `Necklace` está dentro desse loop!

7. **Compile** o Blueprint

---

### **PASSO 4: Verificar a Ordem dos Cases no Switch**

A ordem dos cases no `Switch on EUmbraEquipmentSlot` deve ser:
- None
- Head
- Chest
- Legs
- Feet
- Hands
- MainHand
- OffHand
- Ring
- Amulet
- **Necklace** ← Deve estar aqui, entre Amulet e Earring
- Earring
- Bracelet
- Mount

---

### **PASSO 5: Testar**

1. **Salve** e **Compile** o Blueprint
2. **Execute** o jogo
3. **Equipe** o item "Colar do Mestre"
4. **Abra** o Character Info (tecla C)
5. **Verifique** se o ícone aparece no slot correto (quarto slot do lado direito, entre Amulet e Earring)

---

## 🔍 **VERIFICAÇÃO RÁPIDA:**

Se o ícone ainda não aparecer, verifique:

1. ✅ A variável `Slot_Necklace` foi criada?
2. ✅ O slot foi criado no `CreateEquipmentSlots`?
3. ✅ O case `Necklace` foi adicionado no `Switch` do `UpdateEquipmentSlots`?
4. ✅ O `Update Slot Visual` está recebendo o `Item Slot` correto (do `ForEach Loop`)?
5. ✅ O `Target` do `Update Slot Visual` está conectado ao `Slot_Necklace`?

---

## 📋 **RESUMO:**

1. ✅ Criar variável `Slot_Necklace` (tipo: `WBP Equipment Slot`)
2. ✅ Adicionar criação do slot no `CreateEquipmentSlots` (copiar estrutura de outro slot e mudar para `Necklace`)
3. ✅ Adicionar case `Necklace` no `Switch` do `UpdateEquipmentSlots` (com `Is Valid` e `Update Slot Visual`)

**Após fazer essas 3 alterações, o ícone do item deve aparecer!**

