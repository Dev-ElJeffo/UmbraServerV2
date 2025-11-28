# 🔧 CORREÇÃO: Split Amount Não Está Sendo Setado no Início

## ❌ **PROBLEMA:**

Quando o slider é inicializado com valor 1.0 no `Setup Split Input`, a variável `SplitAmount` pode não estar sendo setada corretamente, então quando você clica em Confirm, o valor ainda é 0 ou não foi atualizado.

---

## ✅ **SOLUÇÃO:**

### **PROBLEMA 1: Split Amount Não Está Sendo Setado no Setup Split Input**

**NO `WBP_SplitInput` → `Setup Split Input`:**

**VERIFIQUE se após `Set Value` do slider, você está setando `SplitAmount`:**

```
[Set Value] (Slider_Amount)
  └─ Value: 1.0
  └─ then → [Set Split Amount] ← ✅ VERIFICAR SE EXISTE!
      └─ Value: 1
```

**SE NÃO EXISTIR, ADICIONE:**

1. **Após o `Set Value` do slider:**
   - Right Click → "Set Split Amount"
   - Conecte o `then` do `Set Value` ao `execute` do `Set Split Amount`
   - Defina `Value` = `1`

---

## ✅ **SOLUÇÃO 2: Adicionar Debug para Verificar o Valor**

### **NO `WBP_SplitInput` → `Button_Confirm OnClicked`:**

**ADICIONE LOGS ANTES DE CHAMAR `Request Split Item`:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Split Amount] ← Variável local
  └─ Return Value: Split Amount
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "Split Amount antes de dividir: [valor]"
  └─ (Use Format Text com Split Amount)
  └─ Text Color: Yellow
  ↓
[Request Split Item]
  └─ Split Amount: Split Amount
```

**ISSO VAI MOSTRAR NO LOG:**
- Qual o valor de `SplitAmount` quando você clica em Confirm
- Se o valor está correto (1) ou errado (0)

---

## ✅ **SOLUÇÃO 3: Garantir que Split Amount Seja Setado no Event Construct**

### **NO `WBP_SplitInput` → `Event Construct`:**

**ADICIONE:**

```
[Event Construct]
  ↓
[Set Split Amount] ← ✅ ADICIONAR
  └─ Value: 1
```

**ISSO GARANTE que `SplitAmount` sempre começa com 1, mesmo se o slider não disparar o evento.**

---

## 🎯 **RESUMO:**

1. ✅ **Verificar se `Set Split Amount` existe após `Set Value` no `Setup Split Input`**
2. ✅ **Adicionar `Set Split Amount` no `Event Construct`**
3. ✅ **Adicionar `Print String` no `Button_Confirm` para debug**

---

## ⚠️ **PROBLEMA MAIS PROVÁVEL:**

O `SplitAmount` não está sendo setado quando o slider é inicializado com 1.0. O evento `On Value Changed` só dispara quando o valor **muda**, então se o slider já está em 1.0 e você não move, o evento não dispara e `SplitAmount` fica com o valor padrão (0).

**SOLUÇÃO:** Setar `SplitAmount` explicitamente no `Setup Split Input` após setar o valor do slider.

---

**PRONTO! 🎉**

