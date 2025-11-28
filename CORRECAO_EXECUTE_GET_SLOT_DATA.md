# 🔧 CORREÇÃO: Get Slot Data não está na sequência Execute

## ❌ **PROBLEMA IDENTIFICADO:**

O `Get Slot Data` (`K2Node_CallFunction_7`) está com o `Target` conectado, mas **NÃO está na sequência de `execute`**. Ele precisa ser chamado DEPOIS de setar as variáveis.

---

## ✅ **CORREÇÃO:**

### **PROBLEMA ATUAL:**

```
[Set Source Slot Widget]
  └─ then → [Set Current Quantity] execute
      └─ then → ??? (não conectado ao Get Slot Data!)
```

### **CORREÇÃO NECESSÁRIA:**

```
[Set Source Slot Widget]
  └─ then → [Set Current Quantity] execute
      └─ then → [Get Slot Data] execute ← ✅ ADICIONAR ESTA CONEXÃO!
```

---

## 📝 **PASSO A PASSO:**

### **PASSO 1: Conectar Execute do Get Slot Data**

1. **ENCONTRE:** `K2Node_CallFunction_7` (Get Slot Data)
2. **ENCONTRE:** O pin `execute` deste nó (pode estar desconectado)
3. **CONECTE:** 
   - `then` do `K2Node_VariableSet_1` (Set Current Quantity) 
   - → `execute` do `K2Node_CallFunction_7` (Get Slot Data)

**COMO FAZER:**
- Arraste do pin `then` do `Set Current Quantity`
- Conecte ao pin `execute` do `Get Slot Data`

---

### **PASSO 2: Conectar Execute do Set Text (ItemName)**

1. **ENCONTRE:** `K2Node_VariableSet_3` (Set Text - TextBlock_ItemName)
2. **CONECTE:**
   - `Return Value` do `Get Slot Data` → `Input` do `Break Umbra Inventory Slot`
   - `then` do `Get Slot Data` → `execute` do `Set Text (ItemName)`

**IMPORTANTE:** O `Get Slot Data` precisa ter um pin `then` (execute output). Se não tiver, você precisa verificar se é uma função pura ou não.

---

## 🔍 **VERIFICAÇÃO:**

### **Se Get Slot Data é função pura:**

Se `Get Slot Data` for uma função pura (sem pin `execute`), você precisa:

1. **Conectar o `then` do `Set Current Quantity` diretamente ao `Set Text (ItemName)`**
2. **O `Get Slot Data` será executado automaticamente quando você conectar o `Target`**

### **Se Get Slot Data NÃO é função pura:**

Se `Get Slot Data` tiver pin `execute`, conecte:

```
[Set Current Quantity]
  └─ then → [Get Slot Data] execute
      └─ then → [Set Text (ItemName)] execute
```

---

## ✅ **ESTRUTURA CORRETA COMPLETA:**

```
[Setup Split Input]
  └─ then → [Set Source Slot Widget] execute
      └─ then → [Set Current Quantity] execute
          └─ then → [Get Slot Data] execute ← ✅ ADICIONAR!
              └─ then → [Set Text (ItemName)] execute
                  └─ then → [Set Text (CurrentQuantity)] execute
                      └─ then → [Set Min Value] execute
                          └─ then → [Set Max Value] execute
                              └─ then → [Set Value] execute
                                  └─ then → [Set Split Amount] execute
```

---

## 🎯 **RESUMO:**

**O problema é:** `Get Slot Data` não está sendo chamado na sequência de `execute`.

**A solução é:** Conectar o `then` do `Set Current Quantity` ao `execute` do `Get Slot Data`.

---

**PRONTO! 🎉**

