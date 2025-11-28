# 🔧 CORREÇÃO FINAL: Get Slot Data usando valor errado

## ❌ **PROBLEMA IDENTIFICADO:**

O `Get Slot Data` (`K2Node_CallFunction_7`) está usando:
- **Target:** `Output_Get` do `Set Source Slot Widget` 

**ISSO ESTÁ ERRADO!** Deve usar o **parâmetro `SourceSlot`** diretamente.

---

## ✅ **CORREÇÃO:**

### **PROBLEMA ATUAL:**

```
[Set Source Slot Widget]
  └─ Output_Get → [Get Slot Data] self (Target) ← ❌ ERRADO!
```

### **CORREÇÃO NECESSÁRIA:**

```
[SourceSlot] (PARÂMETRO) → [Get Slot Data] self (Target) ← ✅ CORRETO!
```

---

## 📝 **PASSO A PASSO:**

### **PASSO 1: Desconectar Output_Get**

1. **ENCONTRE:** `K2Node_CallFunction_7` (Get Slot Data)
2. **ENCONTRE:** O pin `self` (Target) que está conectado ao `Output_Get` do `Set Source Slot Widget`
3. **DESCONECTE:** Esta conexão

### **PASSO 2: Conectar Parâmetro SourceSlot**

1. **ENCONTRE:** O pin `SourceSlot` do `K2Node_FunctionEntry_0` (entrada da função)
2. **CONECTE:**
   - `SourceSlot` (PARÂMETRO) → `self` (Target) do `Get Slot Data`

**COMO FAZER:**
- Arraste do pin `SourceSlot` do `K2Node_FunctionEntry_0`
- Conecte ao pin `self` (Target) do `Get Slot Data`

---

## ✅ **ESTRUTURA CORRETA:**

```
[Setup Split Input]
  ├─ SourceSlot (PARÂMETRO) → [Get Slot Data] self ← ✅ CORRETO!
  ├─ ItemQuantity (PARÂMETRO) → [Set Current Quantity] Value
  │                              → [Format Text] {0}
  │                              → [Subtract] A
  │
  └─ then → [Set Source Slot Widget] execute
      └─ then → [Set Current Quantity] execute
          └─ then → [Set Text (ItemName)] execute
              └─ then → [Set Text (CurrentQuantity)] execute
                  └─ then → [Set Min Value] execute
                      └─ then → [Set Max Value] execute
                          └─ then → [Set Value] execute
                              └─ then → [Set Split Amount] execute
```

---

## 🎯 **RESUMO:**

**O problema é:** `Get Slot Data` está usando `Output_Get` do `Set Source Slot Widget` em vez do parâmetro `SourceSlot`.

**A solução é:** Conectar o parâmetro `SourceSlot` diretamente ao `Target` do `Get Slot Data`.

---

## ⚠️ **IMPORTANTE:**

- `Get Slot Data` é uma função **pura** (não tem pin `execute`)
- Ele será executado automaticamente quando você conectar o `Target`
- Use o **parâmetro** `SourceSlot`, não a variável de instância

---

**PRONTO! 🎉**

