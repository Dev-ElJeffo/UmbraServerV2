# 🔧 CORREÇÃO: Diferença entre Necklace e Earring

## ❌ **PROBLEMA IDENTIFICADO:**

Comparando o código do `Slot_Necklace` com o `Slot_Earring` (que funciona):

**Slot_Necklace (ERRADO):**
- `Set Slot Type` → Target: `Return Value` do `Create Widget` **DIRETAMENTE**
- `Add Child to Uniform Grid` → Content: `Return Value` do `Create Widget` **DIRETAMENTE**

**Slot_Earring (CORRETO):**
- `Set Slot Type` → Target: `Knot_29` (OutputPin) ← **USA KNOT!**
- `Add Child to Uniform Grid` → Content: `Return Value` do `Create Widget` diretamente

---

## ✅ **SOLUÇÃO:**

**No `CreateEquipmentSlots`, para o `Slot_Necklace`:**

1. **CONECTE** o `Target` do `Set Slot Type` ao **OutputPin do `Knot_24`** (não ao `Return Value` do `Create Widget` diretamente)

**Estrutura correta:**

```
[Create Widget] → Return Value
       ↓
[Knot_24] → InputPin: Return Value do Create Widget
       └─ OutputPin
            ├─→ [Set Slot Type] (Target: OutputPin do Knot_24) ← CORRIGIR AQUI!
            ├─→ [Add Child to Uniform Grid] (Content: OutputPin do Knot_24) ← CORRIGIR AQUI!
            └─→ [Knot_25] → [Set Slot_Necklace]
```

**OU** mantenha o `Add Child to Uniform Grid` usando o `Return Value` diretamente (como no Earring), mas **SEMPRE** use o Knot para o `Set Slot Type`:

```
[Create Widget] → Return Value
       ↓
[Knot_24] → InputPin: Return Value do Create Widget
       └─ OutputPin
            ├─→ [Set Slot Type] (Target: OutputPin do Knot_24) ← CORRIGIR!
            └─→ [Knot_25] → [Set Slot_Necklace]
       ↓
[Add Child to Uniform Grid] (Content: Return Value do Create Widget) ← PODE FICAR ASSIM
```

---

## 🎯 **CORREÇÃO ESPECÍFICA:**

**No `K2Node_CallFunction_25` (Set Slot Type do Necklace):**

**ANTES (ERRADO):**
```
CustomProperties Pin (PinId=0DAE33AF47268874FBA0E888AEE982BE,PinName="self",...)
  LinkedTo=(K2Node_CreateWidget_5 8806349B49D3E11422C7E5BFE3CBA38B,)
```

**DEPOIS (CORRETO):**
```
CustomProperties Pin (PinId=0DAE33AF47268874FBA0E888AEE982BE,PinName="self",...)
  LinkedTo=(K2Node_Knot_24 1D277CA1427608DB99ECA5918AADCECF,) ← OutputPin do Knot_24!
```

**E no `K2Node_CallFunction_26` (Add Child to Uniform Grid do Necklace):**

**ANTES (ERRADO):**
```
CustomProperties Pin (PinId=F9CD1A68420AC1A56BFDBF924EF99247,PinName="Content",...)
  LinkedTo=(K2Node_CreateWidget_5 8806349B49D3E11422C7E5BFE3CBA38B,)
```

**DEPOIS (CORRETO - opção 1, igual ao Earring):**
```
CustomProperties Pin (PinId=F9CD1A68420AC1A56BFDBF924EF99247,PinName="Content",...)
  LinkedTo=(K2Node_CreateWidget_5 8806349B49D3E11422C7E5BFE3CBA38B,) ← PODE FICAR ASSIM
```

**OU (opção 2, mais consistente):**
```
CustomProperties Pin (PinId=F9CD1A68420AC1A56BFDBF924EF99247,PinName="Content",...)
  LinkedTo=(K2Node_Knot_24 1D277CA1427608DB99ECA5918AADCECF,) ← OutputPin do Knot_24
```

---

## 📋 **RESUMO:**

**A ÚNICA diferença crítica é:**
- `Set Slot Type` do Necklace está usando `Return Value` do `Create Widget` diretamente
- Deveria usar o `OutputPin` do `Knot_24` (como o Earring usa `Knot_29`)

**Corrija isso e o problema será resolvido!** 🎯

