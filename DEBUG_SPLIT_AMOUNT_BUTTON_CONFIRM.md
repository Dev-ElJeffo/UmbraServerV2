# 🔍 DEBUG: Verificar Split Amount no Button Confirm

## ❌ **PROBLEMA:**

Mesmo com `Set Split Amount` após `Set Value`, quando você coloca para dividir 1 unidade, ainda divide pela metade.

---

## ✅ **SOLUÇÃO: Adicionar Debug no Button Confirm**

### **NO `WBP_SplitInput` → `Event Graph` → `Button_Confirm OnClicked`:**

**ADICIONE LOGS ANTES DE CHAMAR `Request Split Item`:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Split Amount] ← Variável local
  └─ Return Value: Split Amount
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "Split Amount: [valor]"
  └─ (Use Format Text com Split Amount)
  └─ Text Color: Yellow
  ↓
[Get Slider Value] ← ✅ ADICIONAR PARA DEBUG
  └─ Target: Slider_Amount
  └─ Return Value: (Float)
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "Slider Value: [valor]"
  └─ (Use Format Text com Slider Value)
  └─ Text Color: Yellow
  ↓
[Request Split Item]
  └─ Split Amount: Split Amount
```

**ISSO VAI MOSTRAR NO LOG:**
- Qual o valor de `SplitAmount` quando você clica em Confirm
- Qual o valor do slider quando você clica em Confirm
- Se os valores estão corretos (1) ou errados (0)

---

## ✅ **VERIFICAÇÃO: Como Está Sendo Chamado Request Split Item**

### **VERIFIQUE O CÓDIGO DO `Button_Confirm`:**

**O `Request Split Item` DEVE estar assim:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Source Slot Widget] ← Variável local
  └─ Return Value: Source Slot
  ↓
[Is Valid?] ← Validar Source Slot
  └─ Object: Source Slot
  └─ Return Value: (Boolean)
  ↓
[Branch]
  ├─ TRUE: (Source Slot válido)
  │   ├─ [Get Split Amount] ← ✅ VERIFICAR SE ESTÁ AQUI!
  │   │     └─ Return Value: Split Amount
  │   ├─ [Request Split Item]
  │   │     └─ Target: Source Slot
  │   │     └─ Split Amount: Split Amount ← ✅ VERIFICAR SE ESTÁ CONECTADO!
  │   │
  │   └─ [Remove from Parent]
  │
  └─ FALSE: (Source Slot inválido)
      └─ [Print String] "Erro: Slot inválido"
```

---

## 🎯 **PROBLEMA MAIS PROVÁVEL:**

O `Split Amount` pode estar sendo lido como 0 ou não estar sendo passado corretamente para `Request Split Item`.

**VERIFICAÇÕES:**
1. ✅ O pin `Split Amount` do `Request Split Item` está conectado a `Get Split Amount`?
2. ✅ O valor de `Split Amount` está sendo setado corretamente no `On Value Changed`?
3. ✅ O valor de `Split Amount` está sendo setado no `Setup Split Input`?

---

## ✅ **SOLUÇÃO ALTERNATIVA: Ler Diretamente do Slider**

**SE O PROBLEMA PERSISTIR, LEIA DIRETAMENTE DO SLIDER:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Slider Value] ← LER DIRETAMENTE DO SLIDER
  └─ Target: Slider_Amount
  └─ Return Value: (Float)
  ↓
[Max] ← GARANTIR MÍNIMO DE 1.0
  ├─ A: (Slider Value)
  └─ B: 1.0
  └─ Return Value: (Float, mínimo 1.0)
  ↓
[Round64]
  └─ Value: (Resultado do Max)
  └─ Return Value: (Int64)
  ↓
[Convert Int64 to Int]
  └─ Value: (Resultado do Round64)
  └─ Return Value: (Int)
  ↓
[Max] ← GARANTIR MÍNIMO DE 1
  ├─ A: (Resultado do Convert)
  └─ B: 1
  └─ Return Value: (Int, mínimo 1)
  ↓
[Request Split Item]
  └─ Split Amount: (Resultado do Max) ← USAR DIRETAMENTE DO SLIDER
```

**ISSO GARANTE que o valor sempre vem do slider, não da variável.**

---

**PRONTO! 🎉**

