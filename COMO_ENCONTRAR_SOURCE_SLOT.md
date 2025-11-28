# 🔍 COMO ENCONTRAR E USAR Source Slot

## ❓ **VOCÊ ESTÁ VENDO ISSO:**

- Só tem variável local `Source Slot` na função
- A variável local retorna None
- Não consegue encontrar o parâmetro

---

## ✅ **SOLUÇÃO: Verificar Como a Função Está Sendo Chamada**

### **PASSO 1: Verificar no WBP_InventorySlot → OnDragDetected**

**Abra o `OnDragDetected` do `WBP_InventorySlot` e encontre onde `Setup Split Input` é chamado.**

**Você deve ver algo assim:**

```
[Setup Split Input]
  ├─ Target: SplitInputWidget
  ├─ Source Slot: ??? ← ✅ AQUI DEVE ESTAR CONECTADO!
  └─ Item Quantity: Quantity
```

**Se o pin `Source Slot` estiver desconectado ou com None:**
- **CONECTE:** `self` (do WBP_InventorySlot) → `Source Slot` do `Setup Split Input`

---

## ✅ **SOLUÇÃO ALTERNATIVA: Usar Variável de Instância**

**Se o parâmetro não funcionar, use variável de instância:**

### **PASSO 1: No WBP_SplitInput → Variables**

**Crie ou verifique se existe:**
- **Nome:** `SourceSlotWidget`
- **Tipo:** `WBP Inventory Slot Object Reference`

### **PASSO 2: No WBP_InventorySlot → OnDragDetected**

**ANTES de chamar `Setup Split Input`, seta a variável:**

```
[Create Widget] (WBP_SplitInput)
  └─ Return Value: SplitInputWidget
  ↓
[Set Source Slot Widget]  ← ✅ ADICIONAR ESTE NÓ!
  └─ Target: SplitInputWidget
  └─ Value: self ← ✅ CONECTAR self AQUI!
  ↓
[Setup Split Input]
  └─ Target: SplitInputWidget
  └─ Item Quantity: Quantity
```

**COMO OBTER `Set Source Slot Widget`:**
1. Right Click no Event Graph
2. Digite "Set Source Slot Widget"
3. Selecione a variável `SourceSlotWidget` do `WBP_SplitInput`
4. **Target:** Conecte ao `Return Value` do `Create Widget`
5. **Value:** Conecte `self` (do WBP_InventorySlot)

### **PASSO 3: No WBP_SplitInput → Setup Split Input**

**Use a variável de instância:**

```
[Setup Split Input]
  ↓
[Get Source Slot Widget]  ← ✅ AGORA VAI FUNCIONAR!
  └─ Return Value: SourceSlotWidget
  ↓
[Get Slot Data]
  └─ Target: SourceSlotWidget ← ✅ USAR AQUI!
  └─ Return Value: SlotData
```

---

## 🎯 **RESUMO DA SOLUÇÃO:**

**OPÇÃO 1: Usar Parâmetro (se existir)**
- No `OnDragDetected`: Conecte `self` → `Source Slot` do `Setup Split Input`
- No `Setup Split Input`: Use o parâmetro `Source Slot` diretamente

**OPÇÃO 2: Usar Variável de Instância (mais simples)**
- No `OnDragDetected`: `Set Source Slot Widget` com `self` ANTES de chamar `Setup Split Input`
- No `Setup Split Input`: `Get Source Slot Widget` → usar no `Get Slot Data`

---

## ✅ **QUAL USAR?**

**Use a OPÇÃO 2 (Variável de Instância)** - É mais simples e sempre funciona!

---

**PRONTO! 🎉**

