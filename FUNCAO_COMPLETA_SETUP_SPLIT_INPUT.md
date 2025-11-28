# ✅ FUNÇÃO COMPLETA: Setup Split Input

## 📋 **ESTRUTURA COMPLETA DA FUNÇÃO:**

```
[Setup Split Input]
  ├─ Input: Source Slot (WBP Inventory Slot Object Reference)
  ├─ Input: Item Quantity (Integer)
  ↓
[Set Source Slot Widget]  ← Variável de instância
  └─ Value: Source Slot (PARÂMETRO)
  ↓
[Set Current Quantity]  ← Variável de instância
  └─ Value: Item Quantity (PARÂMETRO)
  ↓
[Get Slot Data]
  └─ Target: Source Slot (PARÂMETRO)
  └─ Return Value: SlotData
  ↓
[Break Umbra Inventory Slot]
  └─ Input: SlotData
  └─ Output: Item Template
  └─ Output: Quantity
  ↓
[Break Umbra Item Template]
  └─ Input: Item Template
  └─ Output: Item Name
  ↓
[Set Text] (TextBlock_ItemName)
  └─ Text: Item Name
  ↓
[Format Text] (Quantidade)
  └─ Format: "Quantidade: {0}"
  └─ {0}: Item Quantity (PARÂMETRO)
  ↓
[Set Text] (TextBlock_CurrentQuantity)
  └─ Text: (Resultado do Format Text)
  ↓
[Set Min Value] (Slider_Amount)
  └─ Value: 1.0
  ↓
[Subtract]
  ├─ A: Item Quantity (PARÂMETRO)
  └─ B: 1.0
  ↓
[Convert Int to Double]
  └─ Input: (Resultado do Subtract)
  ↓
[Set Max Value] (Slider_Amount)
  └─ Value: (Resultado do Convert)
  ↓
[Set Value] (Slider_Amount)
  └─ Value: 1.0
  ↓
[Set Split Amount]  ← Variável de instância
  └─ Value: 1
```

---

## 📝 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Setar Variáveis de Instância**

1. **Set Source Slot Widget:**
   - Right Click → "Set Source Slot Widget"
   - Selecione a variável de instância `SourceSlotWidget`
   - **Value:** Conecte ao pin `Source Slot` (PARÂMETRO da função)
   - **execute:** Conecte ao pin `then` do `K2Node_FunctionEntry_0`

2. **Set Current Quantity:**
   - Right Click → "Set Current Quantity"
   - Selecione a variável de instância `CurrentQuantity`
   - **Value:** Conecte ao pin `Item Quantity` (PARÂMETRO da função)
   - **execute:** Conecte ao pin `then` do `Set Source Slot Widget`

---

### **PASSO 2: Obter Slot Data**

1. **Get Slot Data:**
   - Right Click → "Get Slot Data"
   - Selecione a função do `UmbraInventorySlotWidget`
   - **Target:** Conecte ao pin `Source Slot` (PARÂMETRO da função)
   - **execute:** Conecte ao pin `then` do `Set Current Quantity`

2. **Break Umbra Inventory Slot:**
   - Right Click → "Break Umbra Inventory Slot"
   - **Input:** Conecte ao `Return Value` do `Get Slot Data`
   - **Output:** Marque `Item Template` e `Quantity` (os outros podem ficar ocultos)

---

### **PASSO 3: Obter Item Name**

1. **Break Umbra Item Template:**
   - Right Click → "Break Umbra Item Template"
   - **Input:** Conecte ao `Item Template` do `Break Umbra Inventory Slot`
   - **Output:** Marque `Item Name` (os outros podem ficar ocultos)

2. **Set Text (TextBlock_ItemName):**
   - Selecione `TextBlock_ItemName` no Designer
   - Arraste para o Graph
   - Selecione "Set Text"
   - **Text:** Conecte ao `Item Name` do `Break Umbra Item Template`
   - **execute:** Conecte ao pin `then` do `Get Slot Data`

---

### **PASSO 4: Atualizar TextBlock_CurrentQuantity**

1. **Format Text:**
   - Right Click → "Format Text"
   - **Format:** Digite `"Quantidade: {0}"`
   - **{0}:** Conecte ao pin `Item Quantity` (PARÂMETRO da função)

2. **Set Text (TextBlock_CurrentQuantity):**
   - Selecione `TextBlock_CurrentQuantity` no Designer
   - Arraste para o Graph
   - Selecione "Set Text"
   - **Text:** Conecte ao `Result` do `Format Text`
   - **execute:** Conecte ao pin `then` do `Set Text (TextBlock_ItemName)`

---

### **PASSO 5: Configurar Slider**

1. **Set Min Value (Slider_Amount):**
   - Selecione `Slider_Amount` no Designer
   - Arraste para o Graph
   - Selecione "Set Min Value"
   - **Min Value:** Digite `1.0`
   - **execute:** Conecte ao pin `then` do `Set Text (TextBlock_CurrentQuantity)`

2. **Subtract:**
   - Right Click → "Subtract"
   - **A:** Conecte ao pin `Item Quantity` (PARÂMETRO da função)
   - **B:** Digite `1` (Make Literal Int)

3. **Convert Int to Double:**
   - Right Click → "Convert Int to Double"
   - **InInt:** Conecte ao `Return Value` do `Subtract`

4. **Set Max Value (Slider_Amount):**
   - Selecione `Slider_Amount` no Designer
   - Arraste para o Graph
   - Selecione "Set Max Value"
   - **Max Value:** Conecte ao `Return Value` do `Convert Int to Double`
   - **execute:** Conecte ao pin `then` do `Set Min Value`

5. **Set Value (Slider_Amount):**
   - Selecione `Slider_Amount` no Designer
   - Arraste para o Graph
   - Selecione "Set Value"
   - **Value:** Digite `1.0`
   - **execute:** Conecte ao pin `then` do `Set Max Value`

---

### **PASSO 6: Setar Split Amount Inicial**

1. **Set Split Amount:**
   - Right Click → "Set Split Amount"
   - Selecione a variável de instância `SplitAmount`
   - **Value:** Digite `1` (Make Literal Int)
   - **execute:** Conecte ao pin `then` do `Set Value (Slider_Amount)`

---

## 🔗 **CONEXÕES COMPLETAS:**

```
[K2Node_FunctionEntry_0] (Setup Split Input)
  ├─ then → [Set Source Slot Widget] execute
  ├─ Source Slot → [Set Source Slot Widget] Value
  │                 └─ [Get Slot Data] Target
  └─ Item Quantity → [Set Current Quantity] Value
                      └─ [Format Text] {0}
                      └─ [Subtract] A

[Set Source Slot Widget]
  └─ then → [Set Current Quantity] execute

[Set Current Quantity]
  └─ then → [Get Slot Data] execute

[Get Slot Data]
  └─ Return Value → [Break Umbra Inventory Slot] Input
  └─ then → [Set Text (TextBlock_ItemName)] execute

[Break Umbra Inventory Slot]
  └─ Item Template → [Break Umbra Item Template] Input
  └─ Quantity → (não usado, usar parâmetro Item Quantity)

[Break Umbra Item Template]
  └─ Item Name → [Set Text (TextBlock_ItemName)] Text

[Set Text (TextBlock_ItemName)]
  └─ then → [Set Text (TextBlock_CurrentQuantity)] execute

[Format Text]
  └─ Result → [Set Text (TextBlock_CurrentQuantity)] Text

[Set Text (TextBlock_CurrentQuantity)]
  └─ then → [Set Min Value (Slider_Amount)] execute

[Set Min Value (Slider_Amount)]
  └─ then → [Set Max Value (Slider_Amount)] execute

[Subtract]
  └─ Return Value → [Convert Int to Double] InInt

[Convert Int to Double]
  └─ Return Value → [Set Max Value (Slider_Amount)] Max Value

[Set Max Value (Slider_Amount)]
  └─ then → [Set Value (Slider_Amount)] execute

[Set Value (Slider_Amount)]
  └─ then → [Set Split Amount] execute
```

---

## ✅ **CHECKLIST:**

- [ ] `Set Source Slot Widget` usa `Source Slot` (parâmetro)
- [ ] `Set Current Quantity` usa `Item Quantity` (parâmetro)
- [ ] `Get Slot Data` usa `Source Slot` (parâmetro) como Target
- [ ] `Format Text` usa `Item Quantity` (parâmetro) no {0}
- [ ] `Subtract` usa `Item Quantity` (parâmetro) no A
- [ ] `Convert Int to Double` está entre `Subtract` e `Set Max Value`
- [ ] `Set Split Amount` = 1 (não 0)
- [ ] Todas as conexões `execute` estão conectadas em sequência

---

## 🎯 **RESUMO:**

1. **Setar variáveis de instância** com os parâmetros
2. **Obter Slot Data** usando o parâmetro `Source Slot`
3. **Atualizar TextBlocks** com Item Name e Quantity
4. **Configurar Slider** com Min=1.0, Max=ItemQuantity-1, Value=1.0
5. **Setar Split Amount** inicial = 1

---

**PRONTO! 🎉**

