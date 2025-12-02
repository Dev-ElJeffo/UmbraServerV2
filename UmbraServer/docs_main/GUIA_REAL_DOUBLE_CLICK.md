# 🖱️ GUIA REAL: Double Click para Equipar/Desequipar

## ✅ **SOLUÇÃO SIMPLES:**

O `OnMouseButtonDoubleClick` **JÁ É DISPARADO APENAS QUANDO HÁ DOUBLE CLICK!**

Não precisa verificar Click Count - o evento já faz isso automaticamente!

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Criar o Evento OnMouseButtonDoubleClick**

1. No `WBP_EquipmentSlot` (ou `WBP_InventorySlot`), Event Graph
2. Clique direito → **Override** → `OnMouseButtonDoubleClick`
3. Você terá:
   - `exec`
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent)

---

### **PASSO 2: Verificar se é Left Mouse Button (Opcional)**

**Se você quiser garantir que é Left Mouse Button:**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Is Mouse Button Down`**
3. Selecione: **`Is Mouse Button Down`**
4. No pin `Mouse Button`, escolha: **`Left Mouse Button`**
5. Conecte o `Return Value` (bool) a um `Branch`
6. Se `TRUE`: Execute sua lógica

**OU simplesmente execute a lógica direto** - o evento já garante que é double click!

---

## 📊 **ESTRUTURA COMPLETA:**

### **Para WBP_EquipmentSlot (Desequipar):**

```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Is Mouse Button Down] ← OPCIONAL (se quiser verificar o botão)
    ├─ Mouse Button: Left Mouse Button
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
         │    ├─ B: 0                                      │
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
         │         │    ├─ Target: Game Instance         │ │
         │         │    └─ Inventory ID: Inventory ID    │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Handled]                            │ │
         │         │                                      │ │
         │         └─ FALSE: [Unhandled]                 │ │
         │                                                  │
         └─ FALSE: [Unhandled] ← Não é Left Mouse Button (se verificou)
```

---

### **Para WBP_InventorySlot (Equipar):**

```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Is Mouse Button Down] ← OPCIONAL (se quiser verificar o botão)
    ├─ Mouse Button: Left Mouse Button
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
         │    └─ Item Template: (FUmbraItemTemplate)        │
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
         │         │    │    ├─ B: None                 │ │
         │         │    │    └─ Return Value: (bool)    │ │
         │         │    │         │                     │ │
         │         │    │         ├─ TRUE ────────────┐ │ │
         │         │    │         │                 │ │ │
         │         │    │         │  [Get Game Instance]│ │
         │         │    │         │    └─ Game Instance│ │
         │         │    │         │         │         │ │ │
         │         │    │         │         ▼         │ │ │
         │         │    │         │  [Cast to Umbra Game Instance]│ │
         │         │    │         │    └─ Success     │ │ │
         │         │    │         │         │         │ │ │
         │         │    │         │         ▼         │ │ │
         │         │    │         │  [Equip Item]     │ │ │
         │         │    │         │    ├─ Target: Game Instance│ │
         │         │    │         │    └─ Inventory ID│ │ │
         │         │    │         │         │         │ │ │
         │         │    │         │         ▼         │ │ │
         │         │    │         │  [Handled]        │ │ │
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
         └─ FALSE: [Unhandled] ← Não é Left Mouse Button (se verificou)
```

---

## ✅ **RESUMO:**

1. ✅ **Override** → `OnMouseButtonDoubleClick`
2. ✅ **Opcional:** Verificar se é Left Mouse Button com `Is Mouse Button Down`
3. ✅ **Executar sua lógica** (Equip/Unequip)
4. ✅ **Retornar** `Handled` ou `Unhandled`

**NÃO PRECISA:**
- ❌ Verificar Click Count (o evento já garante que é double click!)
- ❌ Usar Break Pointer Event (não existe!)
- ❌ Complicar com timers ou contadores

---

## 🎯 **PRONTO!**

Simples assim! O `OnMouseButtonDoubleClick` já faz todo o trabalho de detectar double click! 🎉

