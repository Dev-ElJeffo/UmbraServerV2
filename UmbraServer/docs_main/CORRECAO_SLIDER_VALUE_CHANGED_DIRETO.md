# 🔧 CORREÇÃO DIRETA: Slider On Value Changed

## ❌ **PROBLEMA:**

Quando o slider está em 1.0, o `Round64` pode estar arredondando valores como 0.9 ou 0.99 para 0, fazendo o PHP dividir pela metade.

---

## ✅ **SOLUÇÃO:**

Adicionar dois nós `Max`:
1. **ANTES do `Round64`** - Garantir que o valor Float nunca seja menor que 1.0
2. **DEPOIS do `Convert Int64 to Int`** - Garantir que o valor Int nunca seja menor que 1

---

## 📝 **ONDE ADICIONAR:**

### **PASSO 1: Adicionar Max ANTES do Round64**

**LOCALIZAÇÃO:** Entre `K2Node_ComponentBoundEvent_0` (Value) e `K2Node_CallFunction_0` (Round64)

**AÇÃO:**
1. **Desconecte:** `Value` (do `K2Node_ComponentBoundEvent_0`) → `A` (do `Round64`)
2. **Adicione:** Nó `Max` (Float Float)
3. **Conecte:**
   - `Value` (do `K2Node_ComponentBoundEvent_0`) → `A` (do `Max`)
   - `B` (do `Max`) = `1.0` (valor fixo)
   - `Return Value` (do `Max`) → `A` (do `Round64`)

**COMO OBTER:**
- Right Click → "Max"
- Selecione "Max (Float Float)"
- Defina `B` = `1.0`

---

### **PASSO 2: Adicionar Max DEPOIS do Convert Int64 to Int**

**LOCALIZAÇÃO:** Entre `K2Node_CallFunction_4` (Convert Int64 to Int) e `K2Node_VariableSet_0` (Set Split Amount)

**AÇÃO:**
1. **Desconecte:** `Return Value` (do `Convert Int64 to Int`) → `SplitAmount` (do `Set Split Amount`)
2. **Adicione:** Nó `Max` (Int Int)
3. **Conecte:**
   - `Return Value` (do `Convert Int64 to Int`) → `A` (do `Max`)
   - `B` (do `Max`) = `1` (valor fixo)
   - `Return Value` (do `Max`) → `SplitAmount` (do `Set Split Amount`)

**COMO OBTER:**
- Right Click → "Max"
- Selecione "Max (Int Int)"
- Defina `B` = `1`

---

## ✅ **ESTRUTURA FINAL:**

```
[Slider_Amount: On Value Changed]
  ├─ Value: (Float)
  ↓
[Max] ← ✅ ADICIONAR AQUI!
  ├─ A: Value (do evento)
  └─ B: 1.0 (fixo)
  └─ Return Value: (Float, mínimo 1.0)
  ↓
[Round64]
  └─ A: (Resultado do Max)
  └─ Return Value: (Int64)
  ↓
[Convert Int64 to Int]
  └─ InInt: (Resultado do Round64)
  └─ Return Value: (Int)
  ↓
[Max] ← ✅ ADICIONAR AQUI!
  ├─ A: (Resultado do Convert)
  └─ B: 1 (fixo)
  └─ Return Value: (Int, mínimo 1)
  ↓
[Set Split Amount]
  └─ SplitAmount: (Resultado do Max)
```

---

## 🎯 **RESUMO:**

1. ✅ **Adicionar `Max` (Float Float) ANTES do `Round64`**
   - `A`: `Value` (do evento)
   - `B`: `1.0` (fixo)

2. ✅ **Adicionar `Max` (Int Int) DEPOIS do `Convert Int64 to Int`**
   - `A`: `Return Value` (do Convert)
   - `B`: `1` (fixo)

---

**PRONTO! 🎉**

