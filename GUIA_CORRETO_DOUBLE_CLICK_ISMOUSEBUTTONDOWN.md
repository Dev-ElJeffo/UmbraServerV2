# 🖱️ GUIA CORRETO: Double Click usando Is Mouse Button Down

## ✅ **SOLUÇÃO CORRETA:**

Use `OnMouseButtonDown` com:
1. `Is Mouse Button Down` - Para verificar se é Left Mouse Button
2. `Break Pointer Event` - Para obter o `Click Count` do MouseEvent

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Criar o Evento OnMouseButtonDown**

1. No `WBP_EquipmentSlot` (ou `WBP_InventorySlot`), Event Graph
2. Clique direito → **Override** → `OnMouseButtonDown`
3. Você terá:
   - `exec`
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent) ← **ESTE!**

---

### **PASSO 2: Verificar se é Left Mouse Button**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Is Mouse Button Down`**
3. Selecione: **`Is Mouse Button Down`**
4. Você terá:
   - `Mouse Button` (EKeys) ← **ESCOLHA O BOTÃO AQUI!**
   - `Return Value` (bool) ← **TRUE se o botão está pressionado**

**COMO ESCOLHER O BOTÃO:**
- No pin `Mouse Button`, clique no dropdown
- Selecione: **`Left Mouse Button`**

**OU:**
- Clique direito → Digite: **`EKeys`**
- Selecione: **`EKeys`** (enum)
- Expanda e procure: **`Left Mouse Button`**
- Arraste para o pin `Mouse Button`

---

### **PASSO 3: Obter o Click Count**

1. Arraste o pin `In Mouse Event` (FPointerEvent) novamente
2. Digite: **`Break Pointer Event`**
3. Selecione: **`Break Pointer Event`**
4. Expanda e procure: **`Click Count`** (int)
5. Use esse pin

---

### **PASSO 4: Verificar se é Double Click (Click Count == 2)**

1. Arraste o pin `Click Count` (int) do `Break Pointer Event`
2. Digite: **`Equal`**
3. Selecione: **`Equal (Int Int)`**
4. Conecte o `Click Count` ao pin `A`
5. No pin `B`, digite: **`2`** (Make Literal Int)
6. Conecte o `Return Value` (bool) a um `Branch`

---

### **PASSO 5: Combinar as Verificações**

1. Use um `AND` (Boolean AND) para combinar:
   - `Is Mouse Button Down` (Return Value) → **TRUE se é Left Mouse Button**
   - `Equal (Int Int)` (Return Value) → **TRUE se Click Count == 2**

2. Conecte ambos ao `AND`
3. Se `AND` retorna `TRUE`: Execute sua lógica (Equip/Unequip)

---

## 📊 **ESTRUTURA COMPLETA:**

### **Para WBP_EquipmentSlot (Desequipar):**

```
[OnMouseButtonDown]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ├─────────────────────────────────────┐
       │                                     │
       ▼                                     ▼
  [Is Mouse Button Down]          [Break Pointer Event]
    ├─ Mouse Button: Left Mouse Button      └─ Click Count: (int)
    └─ Return Value: (bool)                        │
         │                                         ▼
         │                              [Equal (Int Int)]
         │                                ├─ A: Click Count
         │                                ├─ B: 2
         │                                └─ Return Value: (bool)
         │                                         │
         │                                         │
         └─────────────────────────────────────────┼─┐
                                                   │ │
                                                   ▼ ▼
                                            [Boolean AND]
                                              ├─ A: Is Mouse Button Down (bool)
                                              ├─ B: Equal (bool)
                                              └─ Return Value: (bool)
                                                   │
                                                   ├─ TRUE ───────────────────────┐
                                                   │                              │
                                                   │  [Get Equipped Item]          │
                                                   │    └─ Equipped Item           │
                                                   │         │                      │
                                                   │         ▼                      │
                                                   │  [Break Umbra Inventory Slot] │
                                                   │    ├─ Inventory ID            │
                                                   │    └─ Item Template ID        │
                                                   │         │                      │
                                                   │         ▼                      │
                                                   │  [Greater (Int Int)]          │
                                                   │    ├─ A: Item Template ID     │
                                                   │    ├─ B: 0                    │
                                                   │    └─ Return Value: (bool)     │
                                                   │         │                      │
                                                   │         ├─ TRUE ────────────┐ │
                                                   │         │                   │ │
                                                   │         │  [Get Game Instance] │
                                                   │         │    └─ Game Instance│ │
                                                   │         │         │         │ │
                                                   │         │         ▼         │ │
                                                   │         │  [Cast to Umbra Game Instance] │
                                                   │         │    └─ Success     │ │
                                                   │         │         │         │ │
                                                   │         │         ▼         │ │
                                                   │         │  [Unequip Item]   │ │
                                                   │         │    ├─ Target: Game Instance│ │
                                                   │         │    └─ Inventory ID│ │
                                                   │         │         │         │ │
                                                   │         │         ▼         │ │
                                                   │         │  [Handled]        │ │
                                                   │         │                   │ │
                                                   │         └─ FALSE: [Unhandled]│
                                                   │                              │
                                                   └─ FALSE: [Unhandled] ← Não é Left Mouse Button ou não é double click
```

---

### **Para WBP_InventorySlot (Equipar):**

```
[OnMouseButtonDown]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ├─────────────────────────────────────┐
       │                                     │
       ▼                                     ▼
  [Is Mouse Button Down]          [Break Pointer Event]
    ├─ Mouse Button: Left Mouse Button      └─ Click Count: (int)
    └─ Return Value: (bool)                        │
         │                                         ▼
         │                              [Equal (Int Int)]
         │                                ├─ A: Click Count
         │                                ├─ B: 2
         │                                └─ Return Value: (bool)
         │                                         │
         │                                         │
         └─────────────────────────────────────────┼─┐
                                                   │ │
                                                   ▼ ▼
                                            [Boolean AND]
                                              ├─ A: Is Mouse Button Down (bool)
                                              ├─ B: Equal (bool)
                                              └─ Return Value: (bool)
                                                   │
                                                   ├─ TRUE ───────────────────────┐
                                                   │                              │
                                                   │  [Get Slot Data]              │
                                                   │    └─ Slot Data               │
                                                   │         │                      │
                                                   │         ▼                      │
                                                   │  [Break Umbra Inventory Slot] │
                                                   │    ├─ Inventory ID            │
                                                   │    ├─ Item Template ID        │
                                                   │    └─ Item Template           │
                                                   │         │                      │
                                                   │         ├─→ [Greater] → Item Template ID > 0? │
                                                   │         │    │                │
                                                   │         │    ├─ TRUE ───────┐ │
                                                   │         │    │             │ │
                                                   │         │    │  [Break Umbra Item Template] │
                                                   │         │    │    └─ Equipment Slot│ │
                                                   │         │    │         │      │ │
                                                   │         │    │         ▼      │ │
                                                   │         │    │  [Not Equal] → Equipment Slot != None│ │
                                                   │         │    │    │          │ │
                                                   │         │    │    ├─ TRUE ─┐ │ │
                                                   │         │    │    │        │ │ │
                                                   │         │    │    │  [Get Game Instance]│ │ │
                                                   │         │    │    │    └─ Game Instance│ │ │
                                                   │         │    │    │         │        │ │ │
                                                   │         │    │    │         ▼        │ │ │
                                                   │         │    │    │  [Cast to Umbra Game Instance]│ │ │
                                                   │         │    │    │    └─ Success   │ │ │
                                                   │         │    │    │         │       │ │ │
                                                   │         │    │    │         ▼       │ │ │
                                                   │         │    │    │  [Equip Item]   │ │ │
                                                   │         │    │    │    ├─ Target: Game Instance│ │ │
                                                   │         │    │    │    └─ Inventory ID│ │ │
                                                   │         │    │    │         │       │ │ │
                                                   │         │    │    │         ▼       │ │ │
                                                   │         │    │    │  [Handled]     │ │ │
                                                   │         │    │    │                │ │ │
                                                   │         │    │    └────────────────┘ │ │
                                                   │         │    │                      │ │
                                                   │         │    └─ FALSE: [Unhandled]  │ │
                                                   │         │                          │ │
                                                   │         └─ FALSE: [Unhandled]     │ │
                                                   │                                    │ │
                                                   └─ FALSE: [Unhandled]                │ │
```

---

## ✅ **RESUMO RÁPIDO:**

1. ✅ **Override** → `OnMouseButtonDown`
2. ✅ **Arraste** `In Mouse Event` → Digite: **`Is Mouse Button Down`**
3. ✅ **Escolha** `Left Mouse Button` no pin `Mouse Button`
4. ✅ **Arraste** `In Mouse Event` novamente → Digite: **`Break Pointer Event`**
5. ✅ **Use** o pin `Click Count` (int) do `Break Pointer Event`
6. ✅ **Compare** `Click Count` com `2` usando `Equal (Int Int)`
7. ✅ **Combine** ambos com `Boolean AND`
8. ✅ **Se AND == TRUE:** Execute sua lógica (Equip/Unequip)
9. ✅ **Retorne** `Handled` ou `Unhandled`

---

## 🎯 **PRONTO!**

Este é o método que **FUNCIONA** usando `Is Mouse Button Down` e `Break Pointer Event`! 🎉

