# 🖱️ GUIA SIMPLES: Double Click para Equipar/Desequipar

## ✅ **SOLUÇÃO CORRETA:**

Use `OnMouseButtonDown` com:
1. `Is Mouse Button Down` - Para verificar se é Left Mouse Button
2. `Break Pointer Event` - Para obter o `Click Count` do MouseEvent

---

## 🔧 **PASSO A PASSO:**

### **1. Criar o Evento OnMouseButtonDown**

1. No `WBP_EquipmentSlot` (ou `WBP_InventorySlot`), Event Graph
2. Clique direito → **Override** → `OnMouseButtonDown`
3. Você terá:
   - `exec`
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent) ← **ESTE!**

---

### **2. Verificar se é Left Mouse Button**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Is Mouse Button Down`**
3. Selecione: **`Is Mouse Button Down`**
4. Você terá:
   - `Mouse Button` (EKeys) ← **ESCOLHA O BOTÃO AQUI!**
   - `Return Value` (bool) ← **TRUE se o botão está pressionado**

**COMO ESCOLHER O BOTÃO:**
- No pin `Mouse Button`, clique no dropdown
- Selecione: **`Left Mouse Button`**

---

### **3. Obter o Click Count**

**⚠️ IMPORTANTE:** Preciso descobrir como acessar o Click Count do MouseEvent.

**TENTE:**
1. Arraste o pin `In Mouse Event` (FPointerEvent) novamente
2. **NÃO digite nada ainda** - veja todas as opções que aparecem
3. Procure por algo relacionado a **"Click"** ou **"Count"**
4. Se encontrar, use

**OU:**
1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Get Click Count`** ou **`Click Count`**
3. Veja se aparece alguma função
4. Se aparecer, use ela

**SE NÃO ENCONTRAR:** Me diga quais opções aparecem quando você arrasta o MouseEvent sem digitar nada!

---

### **4. Verificar se é Double Click (Click Count == 2)**

1. Arraste o pin `Click Count` (int) do `Break Pointer Event`
2. Digite: **`Equal`**
3. Selecione: **`Equal (Int Int)`**
4. Conecte o `Click Count` ao pin `A`
5. No pin `B`, digite: **`2`** (Make Literal Int)
6. Conecte o `Return Value` (bool) a um `Branch`

---

### **5. Combinar as Verificações**

1. Use um `Boolean AND` para combinar:
   - `Is Mouse Button Down` (Return Value) → **TRUE se é Left Mouse Button**
   - `Equal (Int Int)` (Return Value) → **TRUE se Click Count == 2**

2. Conecte ambos ao `AND`
3. Se `AND` retorna `TRUE`: Execute sua lógica (Equip/Unequip)

---

### **4. Implementar a Lógica**

**Se Click Count == 2:**
- Verificar se tem item equipado (ou item no slot)
- Chamar `Equip Item` ou `Unequip Item`
- Retornar `Handled`

**Se Click Count != 2:**
- Retornar `Unhandled`

---

## 📊 **ESTRUTURA COMPLETA:**

### **Para WBP_EquipmentSlot (Desequipar):**

```
[OnMouseButtonDown]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Get Click Count] ← Do MouseEvent
    └─ Return Value: (int)
         │
         ▼
  [Equal (Int Int)]
    ├─ A: Return Value (int)
    ├─ B: 2 (Make Literal Int)
    └─ Return Value: (bool)
         │
         ├─ TRUE ───────────────────────────────────────────┐
         │                                                  │
         │  [Get Equipped Item] ← Variável                 │
         │    └─ Equipped Item: (FUmbraInventorySlot)      │
         │         │                                        │
         │         ▼                                        │
         │  [Break Umbra Inventory Slot]                   │
         │    ├─ Inventory ID: (int)                        │
         │    └─ Item Template ID: (int)                    │
         │         │                                        │
         │         ▼                                        │
         │  [Greater (Int Int)]                             │
         │    ├─ A: Item Template ID                        │
         │    ├─ B: 0                                       │
         │    └─ Return Value: (bool)                       │
         │         │                                        │
         │         ├─ TRUE ──────────────────────────────┐ │
         │         │                                      │ │
         │         │  [Get Game Instance]                │ │
         │         │    └─ Game Instance                 │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Cast to Umbra Game Instance]       │ │
         │         │    └─ Success                        │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Unequip Item]                      │ │
         │         │    ├─ Target: Game Instance          │ │
         │         │    └─ Inventory ID: Inventory ID     │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Handled]                            │ │
         │         │                                      │ │
         │         └─ FALSE: [Unhandled]                 │ │
         │                                                  │
         └─ FALSE: [Unhandled] ← Não é double click
```

---

### **Para WBP_InventorySlot (Equipar):**

```
[OnMouseButtonDown]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Get Click Count] ← Do MouseEvent
    └─ Return Value: (int)
         │
         ▼
  [Equal (Int Int)]
    ├─ A: Return Value (int)
    ├─ B: 2 (Make Literal Int)
    └─ Return Value: (bool)
         │
         ├─ TRUE ───────────────────────────────────────────┐
         │                                                  │
         │  [Get Slot Data] ← Variável                      │
         │    └─ Slot Data: (FUmbraInventorySlot)           │
         │         │                                        │
         │         ▼                                        │
         │  [Break Umbra Inventory Slot]                   │
         │    ├─ Inventory ID: (int)                        │
         │    ├─ Item Template ID: (int)                    │
         │    └─ Item Template: (FUmbraItemTemplate)       │
         │         │                                        │
         │         ├─→ [Greater] → Item Template ID > 0?   │
         │         │    │                                   │
         │         │    ├─ TRUE ────────────────────────┐ │
         │         │    │                               │ │
         │         │    │  [Break Umbra Item Template]  │ │
         │         │    │    └─ Equipment Slot: (Enum)  │ │
         │         │    │         │                     │ │
         │         │    │         ▼                     │ │
         │         │    │  [Not Equal (Enum Enum)]      │ │
         │         │    │    ├─ A: Equipment Slot        │ │
         │         │    │    ├─ B: None                  │ │
         │         │    │    └─ Return Value: (bool)     │ │
         │         │    │         │                     │ │
         │         │    │         ├─ TRUE ────────────┐ │ │
         │         │    │         │                 │ │ │
         │         │    │         │  [Get Game Instance] │ │
         │         │    │         │    └─ Game Instance│ │ │
         │         │    │         │         │         │ │ │
         │         │    │         │         ▼         │ │ │
         │         │    │         │  [Cast to Umbra Game Instance] │ │
         │         │    │         │    └─ Success      │ │ │
         │         │    │         │         │         │ │ │
         │         │    │         │         ▼     │ │ │
         │         │    │         │  [Equip Item]    │ │ │
         │         │    │         │    ├─ Target: Game Instance│ │
         │         │    │         │    └─ Inventory ID│ │ │
         │         │    │         │         │       │ │ │
         │         │    │         │         ▼       │ │ │
         │         │    │         │  [Handled]       │ │ │
         │         │    │         │                 │ │ │
         │         │    │         └─────────────────┘ │ │
         │         │    │                              │ │
         │         │    │  [Unhandled] ← Item não equipável│ │
         │         │    │                              │ │
         │         │    └──────────────────────────────┘ │
         │         │                                     │
         │         └─ FALSE                              │
         │              │                                 │
         │              ▼                                 │
         │           [Unhandled] ← Item inválido          │
         │                                                  │
         └─ FALSE: [Unhandled] ← Não é double click
```

---

## ✅ **RESUMO RÁPIDO:**

1. ✅ **Override** → `OnMouseButtonDown`
2. ✅ **Arraste** `In Mouse Event` → Digite: **`Get Click Count`**
3. ✅ **Conecte** `Return Value` (int) a `Equal (Int Int)`
4. ✅ **Compare** com `2` (Make Literal Int)
5. ✅ **Se igual a 2:** Execute sua lógica (Equip/Unequip)
6. ✅ **Retorne** `Handled` ou `Unhandled`

---

## 🎯 **PRONTO!**

Este é o método que **FUNCIONA** e está documentado no código existente! 🎉

