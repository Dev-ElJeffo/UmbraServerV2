# ✅ SOLUÇÃO: Ler Slider Diretamente no Button Confirm

## ❌ **PROBLEMA:**

Mesmo com `Set Split Amount` após `Set Value`, quando você coloca para dividir 1 unidade, ainda divide pela metade.

**CAUSA:** A variável `SplitAmount` pode não estar sendo atualizada corretamente ou pode estar sendo lida como 0.

---

## ✅ **SOLUÇÃO: Ler Diretamente do Slider**

**EM VEZ DE USAR `Get Split Amount`, LEIA DIRETAMENTE DO SLIDER:**

### **NO `WBP_SplitInput` → `Event Graph` → `Button_Confirm OnClicked`:**

**SUBSTITUA:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Source Slot Widget]
  └─ Return Value: Source Slot
  ↓
[Is Valid?]
  └─ Object: Source Slot
  ↓
[Branch]
  ├─ TRUE:
  │   ├─ [Get Split Amount] ← ❌ REMOVER ESTE!
  │   │     └─ Return Value: Split Amount
  │   ├─ [Request Split Item]
  │   │     └─ Split Amount: Split Amount ← ❌ REMOVER ESTA CONEXÃO!
```

**POR:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Source Slot Widget]
  └─ Return Value: Source Slot
  ↓
[Is Valid?]
  └─ Object: Source Slot
  ↓
[Branch]
  ├─ TRUE:
  │   ├─ [Get Slider Value] ← ✅ ADICIONAR ESTE!
  │   │     └─ Target: Slider_Amount
  │   │     └─ Return Value: (Float)
  │   ├─ [Max] ← ✅ ADICIONAR ESTE!
  │   │     ├─ A: (Slider Value)
  │   │     └─ B: 1.0
  │   │     └─ Return Value: (Float, mínimo 1.0)
  │   ├─ [Round64] ← ✅ ADICIONAR ESTE!
  │   │     └─ Value: (Resultado do Max)
  │   │     └─ Return Value: (Int64)
  │   ├─ [Convert Int64 to Int] ← ✅ ADICIONAR ESTE!
  │   │     └─ Value: (Resultado do Round64)
  │   │     └─ Return Value: (Int)
  │   ├─ [Max] ← ✅ ADICIONAR ESTE!
  │   │     ├─ A: (Resultado do Convert)
  │   │     └─ B: 1
  │   │     └─ Return Value: (Int, mínimo 1)
  │   ├─ [Request Split Item]
  │   │     └─ Split Amount: (Resultado do Max) ← ✅ USAR ESTE VALOR!
  │   │
  │   └─ [Remove from Parent]
  │
  └─ FALSE:
      └─ [Remove from Parent]
```

---

## 📝 **COMO OBTER OS NÓS:**

1. **Get Slider Value:**
   - Selecione `Slider_Amount` no Designer
   - Arraste para o Event Graph
   - Selecione "Get Value"

2. **Max (Float Float):**
   - Right Click → "Max"
   - Selecione "Max (Float Float)"
   - Defina `B` = `1.0`

3. **Round64:**
   - Right Click → "Round64"
   - Selecione "Round64 (Integer64)"

4. **Convert Int64 to Int:**
   - Right Click → "To Int"
   - Selecione "To Int (Integer)"

5. **Max (Int Int):**
   - Right Click → "Max"
   - Selecione "Max (Int Int)"
   - Defina `B` = `1`

---

## 🎯 **RESUMO:**

**SUBSTITUA `Get Split Amount` por ler diretamente do slider com validação `Max`.**

**ISSO GARANTE:**
- O valor sempre vem do slider atual
- O valor nunca é menor que 1
- Não depende da variável `SplitAmount` que pode estar desatualizada

---

**PRONTO! 🎉**

