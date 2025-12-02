# 🔧 GUIA PASSO A PASSO: Corrigir Split Widget

## ⚠️ **PROBLEMA PRINCIPAL:**

A função `Setup Split Input` está tentando usar `Get Source Slot Widget` (variável de instância) que está **None**, em vez de usar o parâmetro `Source Slot` diretamente.

---

## 📝 **CORREÇÃO 1: Setup Split Input**

### **PASSO 1: Remover Nós Incorretos**

1. **No Graph do WBP_SplitInput → Setup Split Input:**
   - **ENCONTRE:** `K2Node_VariableGet_0` (Get Source Slot Widget) que está no início
   - **DELETE:** Este nó e todas as conexões dele
   - **ENCONTRE:** `K2Node_VariableSet_0` que seta SourceSlotWidget usando o Get acima
   - **DELETE:** Este nó e todas as conexões dele

### **PASSO 2: Corrigir Set Source Slot Widget**

1. **ENCONTRE:** `K2Node_VariableSet_8` (Set Source Slot - parâmetro local)
2. **ALTERE:** Este nó deve setar a variável de instância `SourceSlotWidget` (não o parâmetro local)
3. **CONECTE:**
   - `execute` do `K2Node_FunctionEntry_0` → `execute` do `Set Source Slot Widget`
   - `Source Slot` (PARÂMETRO DA FUNÇÃO) → `SourceSlotWidget` (pin de input do Set)

**COMO OBTER:**
- **Set Source Slot Widget:** Right Click → "Set Source Slot Widget" → Selecione a variável de instância `SourceSlotWidget`
- **Source Slot (parâmetro):** Vem do `K2Node_FunctionEntry_0` (entrada da função)

### **PASSO 3: Corrigir Get Slot Data**

1. **ENCONTRE:** `K2Node_VariableGet_5` (Get Slot Data)
2. **ALTERE:** O `Target` deve ser `Source Slot` (PARÂMETRO), não `Get Source Slot Widget`
3. **CONECTE:**
   - `Source Slot` (PARÂMETRO) → `Target` do `Get Slot Data`

**COMO OBTER:**
- **Source Slot (parâmetro):** Vem do `K2Node_FunctionEntry_0` (entrada da função)
- **Get Slot Data:** Right Click → "Get Slot Data" → Selecione a função do `UmbraInventorySlotWidget`

### **PASSO 4: Corrigir Format Text (Quantidade)**

1. **ENCONTRE:** `K2Node_FormatText_1` (Format Text para "Quantidade: {0}")
2. **ALTERE:** O `{0}` deve usar `Item Quantity` (PARÂMETRO), não `CurrentQuantity`
3. **CONECTE:**
   - `Item Quantity` (PARÂMETRO) → `{0}` do `Format Text`

**COMO OBTER:**
- **Item Quantity (parâmetro):** Vem do `K2Node_FunctionEntry_0` (entrada da função)

### **PASSO 5: Corrigir Subtract (Max Value do Slider)**

1. **ENCONTRE:** `K2Node_PromotableOperator_0` (Subtract)
2. **ALTERE:** O `A` deve usar `Item Quantity` (PARÂMETRO), não `CurrentQuantity`
3. **CONECTE:**
   - `Item Quantity` (PARÂMETRO) → `A` do `Subtract`
   - `B` = 1 (Make Literal Int)

**COMO OBTER:**
- **Item Quantity (parâmetro):** Vem do `K2Node_FunctionEntry_0` (entrada da função)
- **Make Literal Int:** Right Click → "Make Literal Int" → Digite 1

### **PASSO 6: Adicionar Convert Int to Double**

1. **ENCONTRE:** `K2Node_CallFunction_3` (Convert IntToDouble)
2. **GARANTA QUE:**
   - O `InInt` está conectado ao `ReturnValue` do `Subtract`
   - O `ReturnValue` está conectado ao `MaxValue` do `Set Max Value`

**COMO OBTER:**
- **Convert Int to Double:** Right Click → "Convert Int to Double"
- **Conecte:** `ReturnValue` do `Subtract` → `InInt` do `Convert`
- **Conecte:** `ReturnValue` do `Convert` → `MaxValue` do `Set Max Value`

---

## 📝 **CORREÇÃO 2: Button Confirm**

### **PASSO 1: Garantir Validação**

1. **ENCONTRE:** `K2Node_IfThenElse_0` (Branch após Is Valid?)
2. **GARANTA QUE:**
   - `Condition` está conectado ao `ReturnValue` do `Is Valid?`
   - `Is Valid?` está validando `SourceSlotWidget` (variável de instância)

**COMO OBTER:**
- **Is Valid?:** Right Click → "Is Valid?"
- **Get Source Slot Widget:** Right Click → "Get Source Slot Widget" → Selecione a variável de instância
- **Conecte:** `ReturnValue` do `Get Source Slot Widget` → `Object` do `Is Valid?`

### **PASSO 2: Garantir Get Split Amount**

1. **ENCONTRE:** `K2Node_VariableGet_3` (Get Split Amount)
2. **GARANTA QUE:**
   - Está pegando a variável de instância `SplitAmount`
   - Está conectado ao `SplitAmount` do `Request Split Item`

**COMO OBTER:**
- **Get Split Amount:** Right Click → "Get Split Amount" → Selecione a variável de instância
- **Conecte:** `ReturnValue` do `Get Split Amount` → `SplitAmount` do `Request Split Item`

---

## 📝 **CORREÇÃO 3: Slider On Value Changed**

### **PASSO 1: Garantir Conversão Correta**

1. **ENCONTRE:** `K2Node_CallFunction_0` (Round64)
2. **GARANTA QUE:**
   - `A` está conectado ao `Value` do `On Value Changed`
   - `ReturnValue` está conectado ao `InInt` do `Convert Int64 to Int`

**COMO OBTER:**
- **Round64:** Right Click → "Round64"
- **Convert Int64 to Int:** Right Click → "Convert Int64 to Int"
- **Conecte:** `Value` do `On Value Changed` → `A` do `Round64`
- **Conecte:** `ReturnValue` do `Round64` → `InInt` do `Convert Int64 to Int`

### **PASSO 2: Garantir Set Split Amount**

1. **ENCONTRE:** `K2Node_VariableSet_0` (Set Split Amount)
2. **GARANTA QUE:**
   - `SplitAmount` está conectado ao `ReturnValue` do `Convert Int64 to Int`
   - Está setando a variável de instância `SplitAmount`

**COMO OBTER:**
- **Set Split Amount:** Right Click → "Set Split Amount" → Selecione a variável de instância
- **Conecte:** `ReturnValue` do `Convert Int64 to Int` → `SplitAmount` do `Set Split Amount`

### **PASSO 3: Garantir Format Text**

1. **ENCONTRE:** `K2Node_FormatText_0` (Format Text para "Dividir: {0}")
2. **GARANTA QUE:**
   - `{0}` está usando `Get Split Amount` (não o valor direto do slider)

**COMO OBTER:**
- **Get Split Amount:** Right Click → "Get Split Amount" → Selecione a variável de instância
- **Conecte:** `ReturnValue` do `Get Split Amount` → `{0}` do `Format Text`

---

## 📝 **CORREÇÃO 4: Event Construct**

### **PASSO 1: Adicionar Reset de Variáveis**

1. **ENCONTRE:** `K2Node_Event_1` (Event Construct)
2. **ADICIONE:**
   - `Set Source Slot Widget` → Value: None
   - `Set Current Quantity` → Value: 0
   - `Set Split Amount` → Value: 0

**COMO OBTER:**
- **Set Source Slot Widget:** Right Click → "Set Source Slot Widget" → Selecione a variável de instância
- **Set Current Quantity:** Right Click → "Set Current Quantity" → Selecione a variável de instância
- **Set Split Amount:** Right Click → "Set Split Amount" → Selecione a variável de instância
- **Make Literal Int:** Right Click → "Make Literal Int" → Digite 0 (para CurrentQuantity e SplitAmount)

**ESTRUTURA:**

```
[Event Construct]
  ↓
[Set Source Slot Widget]
  └─ Value: None
  ↓
[Set Current Quantity]
  └─ Value: 0
  ↓
[Set Split Amount]
  └─ Value: 0
```

---

## ✅ **CHECKLIST FINAL:**

### **Setup Split Input:**
- [ ] Removido `Get Source Slot Widget` no início
- [ ] `Set Source Slot Widget` usa `Source Slot` (parâmetro)
- [ ] `Set Current Quantity` usa `Item Quantity` (parâmetro)
- [ ] `Get Slot Data` usa `Source Slot` (parâmetro) como Target
- [ ] `Format Text` (Quantidade) usa `Item Quantity` (parâmetro)
- [ ] `Subtract` usa `Item Quantity` (parâmetro)
- [ ] `Convert Int to Double` está entre `Subtract` e `Set Max Value`

### **Button Confirm:**
- [ ] `Is Valid?` valida `SourceSlotWidget` (variável de instância)
- [ ] `Get Split Amount` pega a variável de instância
- [ ] `Request Split Item` usa Source Slot validado

### **Slider On Value Changed:**
- [ ] `Round64` + `Convert Int64 to Int` converte Float → Int
- [ ] `Set Split Amount` atualiza variável de instância
- [ ] `Format Text` usa `Get Split Amount` (não valor direto)

### **Event Construct:**
- [ ] Reseta `SourceSlotWidget` para None
- [ ] Reseta `CurrentQuantity` para 0
- [ ] Reseta `SplitAmount` para 0

---

## 🎯 **RESUMO RÁPIDO:**

1. **Setup Split Input:** Use parâmetros diretamente, não variáveis de instância
2. **Button Confirm:** Valide antes de usar
3. **Slider:** Converta corretamente Float → Int
4. **Event Construct:** Resete todas as variáveis

---

**PRONTO! 🎉**

