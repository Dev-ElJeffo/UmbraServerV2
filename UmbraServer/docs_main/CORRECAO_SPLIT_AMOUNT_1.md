# 🔧 CORREÇÃO: Split Amount = 1 Dividindo pela Metade

## ❌ **PROBLEMA:**

Quando você pede para dividir apenas 1 item do stack, ele divide pela metade. Quando pede para dividir mais de 1 item, funciona corretamente.

**CAUSA:** O valor do slider quando está em 1.0 pode estar sendo convertido incorretamente para inteiro, resultando em 0, que faz o PHP dividir pela metade.

---

## ✅ **SOLUÇÃO:**

### **PROBLEMA NO BLUEPRINT:**

No `WBP_SplitInput` → `Slider_Amount: On Value Changed`, o valor pode estar sendo arredondado incorretamente.

---

## ✅ **CORREÇÃO 1: Verificar Conversão do Slider**

### **NO WBP_SplitInput → Event Graph → Slider_Amount On Value Changed:**

**ESTRUTURA ATUAL (PROBLEMÁTICA):**

```
[Slider_Amount: On Value Changed]
  ├─ Input: In Value (Float)
  ↓
[Round to Int] ← ✅ PODE ESTAR ARREDONDANDO 1.0 PARA 0!
  └─ Value: In Value
  ↓
[Set Split Amount]
  └─ Value: (Resultado do Round to Int)
```

**PROBLEMA:** `Round to Int` pode estar arredondando valores próximos de 1.0 para 0.

---

## ✅ **CORREÇÃO: Usar Floor ou Ceil + Validação**

### **ESTRUTURA CORRIGIDA:**

```
[Slider_Amount: On Value Changed]
  ├─ Input: In Value (Float)
  ↓
[Max] ← ✅ GARANTIR MÍNIMO DE 1.0
  ├─ A: In Value
  └─ B: 1.0
  └─ Return Value: (Float, mínimo 1.0)
  ↓
[Floor] ← ✅ USAR FLOOR EM VEZ DE ROUND
  └─ Value: (Resultado do Max)
  └─ Return Value: (Float)
  ↓
[Convert Float to Int] ← ✅ CONVERTER PARA INT
  └─ Value: (Resultado do Floor)
  └─ Return Value: (Int)
  ↓
[Max] ← ✅ GARANTIR MÍNIMO DE 1
  ├─ A: (Resultado do Convert)
  └─ B: 1
  └─ Return Value: (Int, mínimo 1)
  ↓
[Set Split Amount]
  └─ Value: (Resultado do Max)
```

---

## ✅ **CORREÇÃO ALTERNATIVA (MAIS SIMPLES):**

### **ESTRUTURA SIMPLIFICADA:**

```
[Slider_Amount: On Value Changed]
  ├─ Input: In Value (Float)
  ↓
[Max] ← ✅ GARANTIR MÍNIMO DE 1.0
  ├─ A: In Value
  └─ B: 1.0
  └─ Return Value: (Float, mínimo 1.0)
  ↓
[Round64] ← ✅ USAR ROUND64 (MAIS PRECISO)
  └─ Value: (Resultado do Max)
  └─ Return Value: (Int64)
  ↓
[Convert Int64 to Int] ← ✅ CONVERTER PARA INT
  └─ Value: (Resultado do Round64)
  └─ Return Value: (Int)
  ↓
[Max] ← ✅ GARANTIR MÍNIMO DE 1
  ├─ A: (Resultado do Convert)
  └─ B: 1
  └─ Return Value: (Int, mínimo 1)
  ↓
[Set Split Amount]
  └─ Value: (Resultado do Max)
```

---

## 📝 **COMO OBTER OS NÓS:**

1. **Max:**
   - Right Click → "Max"
   - Selecione "Max (Float Float)" ou "Max (Int Int)"

2. **Floor:**
   - Right Click → "Floor"
   - Selecione "Floor (Float)"

3. **Round64:**
   - Right Click → "Round64"
   - Selecione "Round64 (Integer64)"

4. **Convert Float to Int:**
   - Right Click → "To Int"
   - Selecione "To Int (Integer)"

5. **Convert Int64 to Int:**
   - Right Click → "To Int"
   - Selecione "To Int (Integer)"

---

## ✅ **CORREÇÃO 2: Validar no Button Confirm**

### **NO WBP_SplitInput → Event Graph → Button_Confirm OnClicked:**

**ADICIONE VALIDAÇÃO:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Split Amount] ← Variável local
  └─ Return Value: Split Amount
  ↓
[Branch] ← ✅ VALIDAR SE É >= 1
  ├─ Condition: Split Amount >= 1
  ├─ TRUE: (Válido - CONTINUAR)
  │   └─ [Request Split Item]
  │       └─ Split Amount: Split Amount
  │
  └─ FALSE: (Inválido - ERRO!)
      └─ [Print String]
          └─ In String: "Erro: Split Amount inválido! Valor: [valor]"
          └─ (Use Format Text com Split Amount)
```

---

## 🎯 **RESUMO:**

1. ✅ **Usar `Max` para garantir mínimo de 1.0 antes de converter**
2. ✅ **Usar `Floor` ou `Round64` em vez de `Round to Int`**
3. ✅ **Garantir mínimo de 1 após conversão**
4. ✅ **Adicionar validação no Button Confirm**

---

## ⚠️ **IMPORTANTE:**

- O problema está na conversão do Float para Int
- Quando o slider está em 1.0, pode estar sendo arredondado para 0
- Use `Max` para garantir que o valor nunca seja menor que 1.0 antes e depois da conversão

---

**PRONTO! 🎉**

