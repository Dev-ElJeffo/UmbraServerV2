# ✅ SOLUÇÃO DIRETA: Source Slot None

## 🔍 **PROBLEMA:**

A função `Setup Split Input` não está recebendo o parâmetro `Source Slot` corretamente, ou o parâmetro não existe na função.

---

## ✅ **SOLUÇÃO 1: Verificar se o Parâmetro Existe na Função**

### **PASSO 1: Abrir a Função Setup Split Input**

1. **No Graph do WBP_SplitInput:**
   - Clique em **"Functions"** (lado esquerdo)
   - Encontre **"Setup Split Input"**
   - Clique para abrir

### **PASSO 2: Verificar os Parâmetros**

1. **No topo da função, você deve ver:**
   - `Input: Source Slot` (WBP Inventory Slot Object Reference)
   - `Input: Item Quantity` (Integer)

2. **Se NÃO existir o parâmetro `Source Slot`:**
   - Clique no botão **"+"** ao lado de "Inputs"
   - Adicione:
     - **Nome:** `Source Slot`
     - **Tipo:** `WBP Inventory Slot Object Reference`
     - **Category:** Default

---

## ✅ **SOLUÇÃO 2: Usar Variável de Instância (Mais Simples)**

**Se você não conseguir fazer funcionar com parâmetro, use variável de instância:**

### **PASSO 1: Garantir que a Variável Existe**

1. **No Graph do WBP_SplitInput → Variables:**
   - Verifique se existe `SourceSlotWidget` (WBP Inventory Slot Object Reference)
   - Se não existir, crie:
     - **Nome:** `SourceSlotWidget`
     - **Tipo:** `WBP Inventory Slot Object Reference`
     - **Category:** Default

### **PASSO 2: Setar a Variável ANTES de Chamar Setup Split Input**

**No Graph do WBP_InventorySlot → OnDragDetected:**

```
[Create Widget] (WBP_SplitInput)
  └─ Return Value: SplitInputWidget
  ↓
[Set Source Slot Widget]  ← ✅ SETAR AQUI PRIMEIRO!
  └─ Target: SplitInputWidget
  └─ Value: self (WBP_InventorySlot)
  ↓
[Setup Split Input]  ← ✅ AGORA CHAMAR A FUNÇÃO
  └─ Target: SplitInputWidget
  └─ Source Slot: self (WBP_InventorySlot) ← ✅ PASSAR self
  └─ Item Quantity: Quantity (do Break Umbra Inventory Slot)
```

**COMO OBTER:**

1. **Set Source Slot Widget:**
   - Right Click → "Set Source Slot Widget"
   - Selecione a variável `SourceSlotWidget` do `WBP_SplitInput`
   - **Target:** Conecte ao `Return Value` do `Create Widget`
   - **Value:** Conecte `self` (do WBP_InventorySlot)

2. **Setup Split Input:**
   - Right Click → "Setup Split Input"
   - **Target:** Conecte ao `Return Value` do `Create Widget`
   - **Source Slot:** Conecte `self` (do WBP_InventorySlot)
   - **Item Quantity:** Conecte `Quantity` (do Break Umbra Inventory Slot)

---

## ✅ **SOLUÇÃO 3: Usar Variável de Instância Dentro da Função**

**Se você quer usar variável de instância dentro da função:**

### **No Setup Split Input:**

```
[Setup Split Input]
  ├─ Input: Source Slot (PARÂMETRO) ← ✅ Manter parâmetro
  ├─ Input: Item Quantity (PARÂMETRO) ← ✅ Manter parâmetro
  ↓
[Set Source Slot Widget]  ← ✅ Setar variável de instância com PARÂMETRO
  └─ Value: Source Slot (PARÂMETRO)
  ↓
[Get Slot Data]  ← ✅ Usar PARÂMETRO diretamente
  └─ Target: Source Slot (PARÂMETRO)
  └─ Return Value: SlotData
```

**IMPORTANTE:**
- Use o **PARÂMETRO** `Source Slot` diretamente no `Get Slot Data`
- Só depois seta na variável de instância para usar depois

---

## 🎯 **SOLUÇÃO RECOMENDADA (Mais Simples):**

**Use variável de instância e seta ANTES de chamar a função:**

### **No WBP_InventorySlot → OnDragDetected:**

```
[Create Widget] (WBP_SplitInput)
  ↓
[Set Source Slot Widget]  ← ✅ SETAR PRIMEIRO
  └─ Target: SplitInputWidget
  └─ Value: self
  ↓
[Setup Split Input]
  └─ Target: SplitInputWidget
  └─ Source Slot: self ← ✅ PASSAR self
  └─ Item Quantity: Quantity
```

### **No WBP_SplitInput → Setup Split Input:**

```
[Setup Split Input]
  ├─ Input: Source Slot (PARÂMETRO)
  ├─ Input: Item Quantity (PARÂMETRO)
  ↓
[Set Source Slot Widget]  ← ✅ Setar variável com PARÂMETRO
  └─ Value: Source Slot (PARÂMETRO)
  ↓
[Get Slot Data]  ← ✅ Usar PARÂMETRO diretamente
  └─ Target: Source Slot (PARÂMETRO)
```

---

## 🔧 **COMO ENCONTRAR O PARÂMETRO NA FUNÇÃO:**

1. **Abra a função `Setup Split Input`**
2. **No topo, você verá os pins de entrada:**
   - `Source Slot` (Object Reference)
   - `Item Quantity` (Integer)

3. **Para usar o parâmetro:**
   - Arraste do pin `Source Slot` (entrada da função)
   - OU: Right Click → "Get Source Slot" → Selecione o parâmetro da função

---

## ✅ **CHECKLIST:**

- [ ] Função `Setup Split Input` tem parâmetro `Source Slot`?
- [ ] No `OnDragDetected`, está passando `self` para `Source Slot`?
- [ ] No `Setup Split Input`, está usando o parâmetro `Source Slot` no `Get Slot Data`?
- [ ] Variável de instância `SourceSlotWidget` está sendo setada com o parâmetro?

---

**PRONTO! 🎉**

