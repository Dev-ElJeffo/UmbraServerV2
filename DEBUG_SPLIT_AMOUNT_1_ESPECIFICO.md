# 🔍 DEBUG: Problema Específico com Split Amount = 1

## ❌ **PROBLEMA:**

O split funciona perfeitamente para qualquer valor que não seja 1. Quando você tenta dividir 1 unidade, ele divide pela metade.

---

## ✅ **CORREÇÃO APLICADA:**

Adicionei logs no C++ para verificar o valor recebido e corrigi a condição de `> 0` para `>= 1`.

**MUDANÇA NO C++:**
- Antes: `if (SplitAmount > 0)` - Isso funciona para 1, mas pode estar recebendo 0
- Agora: `if (SplitAmount >= 1)` - Mais explícito e com logs

---

## 🔍 **COMO VERIFICAR:**

### **1. Recompile o C++:**
```
cd D:\UmbraServerV2\UmbraEternumUE
# Recompile o projeto no Unreal Engine
```

### **2. Teste o Split com Valor 1:**
1. Abra o jogo
2. Abra o inventário
3. Pressione Shift + Arraste um item com quantidade > 1
4. Coloque o slider em 1.0
5. Clique em Confirm

### **3. Verifique o Log:**
Procure por estas linhas no log do Unreal:

```
[UmbraGameInstance] 🔍 [DEBUG] SplitItem recebido - InventoryID: X, SplitAmount: Y, TargetSlotIndex: Z
[UmbraGameInstance] ✅ Enviando split_amount: Y
```

**OU:**

```
[UmbraGameInstance] ⚠️ [WARNING] SplitAmount inválido (0), não será enviado (PHP dividirá pela metade)
```

---

## 🎯 **POSSÍVEIS CAUSAS:**

### **CAUSA 1: Blueprint não está enviando o valor 1**

**SE O LOG MOSTRAR:**
```
SplitAmount: 0
```

**SOLUÇÃO:**
O problema está no Blueprint. Quando o slider está em 1.0, o `SplitAmount` pode não estar sendo setado corretamente.

**VERIFIQUE:**
1. No `WBP_SplitInput` → `Setup Split Input`:
   - O `Set Split Amount` está sendo chamado com valor `1` após `Set Value`?
   - O `Set Split Amount` está na sequência correta (após `Set Value`)?

2. No `WBP_SplitInput` → `Button_Confirm OnClicked`:
   - O `Get Split Amount` está retornando o valor correto?
   - Adicione um `Print String` antes de `Request Split Item` para verificar o valor

---

### **CAUSA 2: Round64 está arredondando 1.0 para 0**

**SE O LOG MOSTRAR:**
```
SplitAmount: 0
```

**SOLUÇÃO:**
O problema pode estar no `On Value Changed` do slider. Quando o valor é 1.0, o `Round64` pode estar arredondando incorretamente.

**VERIFIQUE:**
No `WBP_SplitInput` → `Slider_Amount: On Value Changed`:
- O `Max (Float Float)` está garantindo que o valor seja >= 1.0?
- O `Round64` está recebendo um valor >= 1.0?
- O `Max (Int Int)` está garantindo que o resultado seja >= 1?

**TESTE:**
Adicione `Print String` após cada conversão para ver os valores intermediários:
```
[Get Slider Value] → Print: "Slider Value: [valor]"
[Max Float] → Print: "Max Float: [valor]"
[Round64] → Print: "Round64: [valor]"
[Convert Int64 to Int] → Print: "Convert: [valor]"
[Max Int] → Print: "Max Int: [valor]"
[Set Split Amount] → Print: "Set Split Amount: [valor]"
```

---

### **CAUSA 3: On Value Changed não dispara quando slider está em 1.0**

**SE O LOG MOSTRAR:**
```
SplitAmount: 0
```

**SOLUÇÃO:**
Quando o slider é inicializado com 1.0, o evento `On Value Changed` não dispara (porque o valor não mudou). O `SplitAmount` fica com o valor padrão (0).

**VERIFIQUE:**
No `WBP_SplitInput` → `Setup Split Input`:
- O `Set Split Amount` está sendo chamado com valor `1` **APÓS** o `Set Value` do slider?
- A sequência está correta:
  ```
  [Set Value] (Slider = 1.0)
  ↓
  [Set Split Amount] (SplitAmount = 1) ← ✅ DEVE ESTAR AQUI!
  ```

---

## ✅ **SOLUÇÃO ALTERNATIVA: Ler Diretamente do Slider**

**SE O PROBLEMA PERSISTIR, USE A SOLUÇÃO DO ARQUIVO `SOLUCAO_LER_SLIDER_DIRETO_BUTTON_CONFIRM.md`:**

Em vez de usar `Get Split Amount`, leia diretamente do slider no `Button_Confirm`:

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

---

## 🎯 **PRÓXIMOS PASSOS:**

1. ✅ Recompile o C++ com os novos logs
2. ✅ Teste o split com valor 1
3. ✅ Verifique o log do Unreal
4. ✅ Identifique qual é a causa (Blueprint, Round64, ou On Value Changed)
5. ✅ Aplique a solução correspondente

---

**PRONTO! 🎉**

