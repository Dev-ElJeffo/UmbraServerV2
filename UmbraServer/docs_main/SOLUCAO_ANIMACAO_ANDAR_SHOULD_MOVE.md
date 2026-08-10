> **Atualização 2026-08:** `CMC->Acceleration` é protected no UE 5.6 — ajuste **obrigatório** no AnimBP (`Should Move` só com GroundSpeed). Ver [`GUIA_BP_ABP_SHOULD_MOVE_REMOTE_UE561.md`](../../docs_main/GUIA_BP_ABP_SHOULD_MOVE_REMOTE_UE561.md).

# 🔍 **ANÁLISE DO ANIMATION BLUEPRINT: Problema Identificado**

## 📊 **ANÁLISE DO XML FORNECIDO:**

### **✅ O QUE ESTÁ FUNCIONANDO:**

1. **`Is Falling` está sendo atualizado corretamente:**
   ```
   MovementComponent → IsFalling() → Set Is Falling
   ```
   - Isso explica por que a animação de pulo funciona ✅

2. **`Velocity` e `Ground Speed` estão sendo atualizados:**
   ```
   MovementComponent → Get Velocity → Set Velocity
   MovementComponent → Get Velocity → VSizeXY → Set Ground Speed
   ```
   - `Ground Speed` é calculado apenas com X e Y (ignorando Z) ✅

### **❌ PROBLEMA IDENTIFICADO:**

**`Should Move` está sendo calculado incorretamente:**

```xml
Should Move = (Ground Speed > 3.0) AND (GetCurrentAcceleration != (0,0,0))
```

**O PROBLEMA:** `GetCurrentAcceleration` retorna a **aceleração atual** do `Character Movement Component`, que é baseada no **input do jogador**. Para remote players, não há input do jogador, então `Acceleration` será sempre `(0,0,0)`, fazendo com que `Should Move` seja sempre `false`, mesmo quando `Ground Speed > 3.0`.

---

## 🔧 **SOLUÇÃO:**

### **OPÇÃO 1: Remover verificação de Acceleration (RECOMENDADA)**

**Modificar a lógica de `Should Move`:**

**ANTES (INCORRETO):**
```
Should Move = (Ground Speed > 3.0) AND (Acceleration != 0)
```

**DEPOIS (CORRETO):**
```
Should Move = (Ground Speed > 3.0)
```

**Justificativa:** Para remote players, a velocidade já indica movimento. Não precisamos verificar aceleração, pois ela será sempre zero (não há input do jogador).

### **OPÇÃO 2: Usar apenas Ground Speed (AINDA MAIS SIMPLES)**

**Modificar a lógica de `Should Move`:**

**ANTES (INCORRETO):**
```
Should Move = (Ground Speed > 3.0) AND (Acceleration != 0)
```

**DEPOIS (CORRETO):**
```
Should Move = (Ground Speed > threshold)
```

Onde `threshold` pode ser `3.0` ou um valor menor (ex: `1.0`) para detectar movimento mais facilmente.

---

## 📝 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **PASSO 1: Localizar o nó `K2Node_CommutativeAssociativeBinaryOperator_0`**

Este nó está fazendo `Boolean AND` entre:
- `Ground Speed > 3.0` (`K2Node_PromotableOperator_0`)
- `Acceleration != 0` (`K2Node_PromotableOperator_2`)

### **PASSO 2: Remover a verificação de Acceleration**

1. **Desconecte o pin `B` do `K2Node_CommutativeAssociativeBinaryOperator_0`**
   - Este pin está conectado ao `ReturnValue` de `K2Node_PromotableOperator_2` (Acceleration != 0)

2. **Conecte diretamente `Ground Speed > 3.0` ao `Set Should Move`:**
   - Conecte o `ReturnValue` de `K2Node_PromotableOperator_0` (Ground Speed > 3.0) diretamente ao pin `Should Move` de `K2Node_VariableSet_5`

3. **Remova os nós não utilizados (opcional):**
   - `K2Node_CommutativeAssociativeBinaryOperator_0` (Boolean AND)
   - `K2Node_PromotableOperator_2` (Acceleration != 0)
   - `K2Node_CallFunction_0` (GetCurrentAcceleration)
   - `K2Node_VariableGet_9` (MovementComponent para Acceleration)

---

## 🔄 **FLUXO CORRIGIDO:**

### **ANTES (INCORRETO):**
```
MovementComponent → Get Velocity → Set Velocity
MovementComponent → Get Velocity → VSizeXY → Set Ground Speed
  ↓
Ground Speed > 3.0? → Boolean AND → Acceleration != 0? → Set Should Move ❌
```

### **DEPOIS (CORRETO):**
```
MovementComponent → Get Velocity → Set Velocity
MovementComponent → Get Velocity → VSizeXY → Set Ground Speed
  ↓
Ground Speed > 3.0? → Set Should Move ✅
```

---

## ✅ **VERIFICAÇÃO:**

Após a correção:

1. **Quando o remote player está andando (`Ground Speed > 3.0`):**
   - `Should Move` deve ser `true` ✅
   - Animação de andar deve ser reproduzida ✅

2. **Quando o remote player está parado (`Ground Speed <= 3.0`):**
   - `Should Move` deve ser `false` ✅
   - Animação idle deve ser reproduzida ✅

3. **Quando o remote player está no ar (`Is Falling == true`):**
   - Animação de pulo/queda deve ser reproduzida ✅ (já funciona)

---

## 🎯 **RESUMO:**

**Causa do problema:** `Should Move` está verificando `Acceleration != 0`, que sempre será `false` para remote players (não há input do jogador).

**Solução:** Remover a verificação de `Acceleration` e usar apenas `Ground Speed > 3.0` para determinar `Should Move`.

**Implementação:** Desconectar o pin `B` do `Boolean AND` e conectar diretamente `Ground Speed > 3.0` ao `Set Should Move`.

---

**Esta é a causa raiz do problema! Remova a verificação de Acceleration e a animação de andar deve funcionar.**

