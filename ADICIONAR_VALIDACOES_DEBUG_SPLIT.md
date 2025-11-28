# 🔧 ADICIONAR VALIDAÇÕES E DEBUG: Setup Split Input

## 🎯 **OBJETIVO:**

Adicionar validações e logs de debug para identificar por que `Setup Split Input` não está funcionando.

---

## ✅ **PASSO 1: Adicionar Validação de SourceSlot**

### **NO INÍCIO DA FUNÇÃO `Setup Split Input`:**

**ADICIONE ESTES NÓS:**

```
[Setup Split Input]
  ├─ Input: SourceSlot
  ├─ Input: ItemQuantity
  ↓
[Is Valid] ← ✅ ADICIONAR
  └─ Object: SourceSlot (PARÂMETRO)
  └─ Return Value: (Boolean)
  ↓
[Branch] ← ✅ ADICIONAR
  ├─ Condition: (Is Valid Return Value)
  ├─ TRUE: (SourceSlot válido)
  │   └─ CONTINUAR COM O CÓDIGO ATUAL
  │
  └─ FALSE: (SourceSlot é None)
      └─ [Print String]
          └─ In String: "ERRO: SourceSlot é None em Setup Split Input!"
          └─ Text Color: Red
      └─ [Return] ← SAIR DA FUNÇÃO
```

**COMO OBTER:**
1. **Right Click → "Is Valid"**
2. **Conecte `SourceSlot` (PARÂMETRO) ao `Object`**
3. **Right Click → "Branch"**
4. **Conecte `Return Value` do `Is Valid` ao `Condition` do `Branch`**
5. **No `FALSE` do `Branch`, adicione `Print String` e `Return`**

---

## ✅ **PASSO 2: Adicionar Debug Logs**

### **APÓS O `Set Source Slot Widget`:**

**ADICIONE:**

```
[Set Source Slot Widget]
  └─ Value: SourceSlot (PARÂMETRO)
  └─ then → [Print String] ← ✅ ADICIONAR
      └─ In String: "SourceSlotWidget setado"
      └─ Text Color: Green
  └─ then → [Get Slot Data] ← CONTINUAR COM O CÓDIGO ATUAL
```

---

## ✅ **PASSO 3: Adicionar Validação após Get Slot Data**

### **APÓS O `Get Slot Data`:**

**ADICIONE:**

```
[Get Slot Data]
  └─ Target: SourceSlot (PARÂMETRO)
  └─ Return Value: SlotData
  ↓
[Break Umbra Inventory Slot]
  └─ Input: SlotData
  └─ Quantity
  └─ ItemTemplate ← ✅ VERIFICAR SE NÃO É None
  ↓
[Is Valid] ← ✅ ADICIONAR
  └─ Object: ItemTemplate (do Break)
  └─ Return Value: (Boolean)
  ↓
[Branch] ← ✅ ADICIONAR
  ├─ Condition: (Is Valid Return Value)
  ├─ TRUE: (ItemTemplate válido)
  │   └─ CONTINUAR COM O CÓDIGO ATUAL
  │
  └─ FALSE: (ItemTemplate é None)
      └─ [Print String]
          └─ In String: "ERRO: ItemTemplate é None!"
          └─ Text Color: Red
      └─ [Print String]
          └─ In String: "SlotData.Quantity: [valor]"
          └─ (Use Format Text com Quantity)
      └─ [Return] ← SAIR DA FUNÇÃO
```

---

## ✅ **PASSO 4: Adicionar Debug no Slider**

### **APÓS O `Set Max Value`:**

**ADICIONE:**

```
[Set Max Value]
  └─ MaxValue: (CurrentQuantity - 1) convertido para Double
  └─ then → [Print String] ← ✅ ADICIONAR
      └─ In String: "Slider MaxValue setado: [valor]"
      └─ (Use Format Text com MaxValue)
      └─ Text Color: Green
```

---

## ✅ **PASSO 5: Verificar Como a Função Está Sendo Chamada**

### **NO WBP_InventorySlot → OnDragDetected:**

**VERIFIQUE:**

```
[OnDragDetected]
  ↓
[Branch] (Is Shift Down?)
  ├─ TRUE:
  │   ├─ [Get Slot Data] (self)
  │   ├─ [Break Umbra Inventory Slot]
  │   │     └─ Quantity
  │   ├─ [Branch] (Quantity > 1)
  │   │   ├─ TRUE:
  │   │   │   ├─ [Create Widget] (WBP_SplitInput)
  │   │   │   │     └─ Return Value: SplitInputWidget
  │   │   │   ├─ [Print String] ← ✅ ADICIONAR PARA DEBUG
  │   │   │   │     └─ In String: "Criando SplitInputWidget"
  │   │   │   │     └─ Text Color: Yellow
  │   │   │   ├─ [Setup Split Input] ← ✅ VERIFICAR AQUI!
  │   │   │   │     └─ Target: SplitInputWidget
  │   │   │   │     └─ Source Slot: self ← ✅ DEVE SER self!
  │   │   │   │     └─ Item Quantity: Quantity
  │   │   │   ├─ [Print String] ← ✅ ADICIONAR PARA DEBUG
  │   │   │   │     └─ In String: "Setup Split Input chamado com SourceSlot: self"
  │   │   │   │     └─ Text Color: Yellow
```

**VERIFIQUE ESPECIALMENTE:**
- O pin `Source Slot` do `Setup Split Input` está conectado a `self`?
- O pin `Item Quantity` está conectado a `Quantity` (do Break)?

---

## 🎯 **RESUMO:**

1. ✅ **Adicionar `Is Valid` para `SourceSlot` no início**
2. ✅ **Adicionar `Is Valid` para `ItemTemplate` após `Get Slot Data`**
3. ✅ **Adicionar `Print String` em pontos-chave para debug**
4. ✅ **Verificar se `SourceSlot` está conectado a `self` em `OnDragDetected`**

---

## ⚠️ **PROBLEMA MAIS PROVÁVEL:**

**O `SourceSlot` está chegando como `None` quando a função é chamada.**

**SOLUÇÃO:**
- Verificar se está conectado a `self` em `OnDragDetected`
- Adicionar validação `Is Valid` no início da função

---

**PRONTO! 🎉**

