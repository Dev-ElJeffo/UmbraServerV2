# 🔧 CORREÇÃO: Problemas no Split Widget

## 📋 **PROBLEMAS IDENTIFICADOS:**

1. **`SourceSlotWidget` está None** - Tentando usar variável de instância antes de setar
2. **Slider não funciona** - Max Value calculado incorretamente
3. **"Slot inválido"** - `SourceSlotWidget` não foi setado corretamente

---

## ✅ **CORREÇÃO 1: Setup Split Input - Usar Parâmetro Diretamente**

**PROBLEMA:** A função está tentando usar `Get Source Slot Widget` (variável de instância) que está None, em vez de usar o parâmetro `Source Slot` diretamente.

**SOLUÇÃO:** Use o parâmetro `Source Slot` diretamente, e só depois seta na variável de instância.

### **No Graph do WBP_SplitInput → Setup Split Input:**

**REMOVA:**
- `K2Node_VariableGet_0` (Get SourceSlotWidget) que está no início
- `K2Node_VariableSet_0` que seta SourceSlotWidget com o Get acima

**SUBSTITUA POR:**

```
[Setup Split Input]  ← Função
  ├─ Input: Source Slot (WBP Inventory Slot Object Reference)
  ├─ Input: Item Quantity (Integer)
  ↓
[Set Source Slot Widget]  ← Variável de instância (NÃO o parâmetro local)
  └─ Value: Source Slot (PARÂMETRO DA FUNÇÃO, não Get Source Slot Widget)
  ↓
[Set Current Quantity]  ← Variável de instância
  └─ Value: Item Quantity (PARÂMETRO DA FUNÇÃO)
  ↓
[Get Slot Data]  ← DO Source Slot (PARÂMETRO)
  └─ Target: Source Slot (PARÂMETRO, não variável de instância)
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
[Format Text]
  └─ Format: "Quantidade: {0}"
  └─ {0}: Item Quantity (PARÂMETRO, não CurrentQuantity)
  ↓
[Set Text] (TextBlock_CurrentQuantity)
  └─ Text: (Resultado do Format Text)
  ↓
[Set Min Value] (Slider_Amount)
  └─ Value: 1.0
  ↓
[Subtract]
  ├─ A: Item Quantity (PARÂMETRO, convertido para Float)
  └─ B: 1.0
  ↓
[Convert Int to Double]  ← Converter resultado para Float
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

**IMPORTANTE:**
- Use `Source Slot` (parâmetro) diretamente, não `Get Source Slot Widget` (variável)
- Use `Item Quantity` (parâmetro) diretamente, não `Get Current Quantity` (variável)
- Só depois de usar os parâmetros, seta nas variáveis de instância

---

## ✅ **CORREÇÃO 2: Button Confirm - Validar Antes de Usar**

**PROBLEMA:** `SourceSlotWidget` pode estar None quando o botão é clicado.

**SOLUÇÃO:** Validar e usar o parâmetro que foi passado na função `Setup Split Input`.

### **No Graph do WBP_SplitInput → Event Graph → Button_Confirm OnClicked:**

**MANTENHA:**
- `Get Player Controller`
- `Set Input Mode Game And UI`
- `Get Source Slot Widget` (variável de instância)
- `Is Valid?`
- `Branch`

**GARANTA QUE:**
- O `Get Source Slot Widget` está pegando a variável de instância que foi setada em `Setup Split Input`
- O `Is Valid?` está validando corretamente
- Se inválido, apenas fecha o widget (não tenta chamar Request Split Item)

**ESTRUTURA CORRETA:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Player Controller]
  ↓
[Set Input Mode Game And UI]
  ├─ Target: Player Controller
  └─ Widget to Focus: None
  ↓
[Get Source Slot Widget]  ← Variável de instância
  └─ Return Value: Source Slot
  ↓
[Is Valid?]
  └─ Object: Source Slot
  ↓
[Branch]
  ├─ TRUE (Válido):
  │   ├─ [Get Split Amount]  ← Variável de instância
  │   │     └─ Return Value: Split Amount
  │   ├─ [Request Split Item]
  │   │     ├─ Target: Source Slot (do Get Source Slot Widget)
  │   │     ├─ Split Amount: Split Amount
  │   │     └─ Target Slot Index: -1
  │   │
  │   └─ [Remove from Parent]
  │       └─ Target: self
  │
  └─ FALSE (Inválido):
      └─ [Print String] "Erro: Slot inválido"
      └─ [Remove from Parent]
          └─ Target: self
```

---

## ✅ **CORREÇÃO 3: Slider On Value Changed - Usar Variável Correta**

**PROBLEMA:** O slider pode estar usando valores incorretos.

**SOLUÇÃO:** Garantir que está usando `SplitAmount` (variável de instância) corretamente.

### **No Graph do WBP_SplitInput → Event Graph → Slider_Amount On Value Changed:**

**ESTRUTURA CORRETA:**

```
[Slider_Amount: On Value Changed]
  ├─ Input: In Value (Float)
  ↓
[Round to Int]  ← OU Round64 + Convert Int64 to Int
  └─ Value: In Value
  ↓
[Set Split Amount]  ← Variável de instância
  └─ Value: (Resultado do Round to Int)
  ↓
[Format Text]
  └─ Format: "Dividir: {0}"
  └─ {0}: Split Amount (do Get Split Amount, convertido para Text)
  ↓
[Set Text] (TextBlock_SplitAmount)
  └─ Text: (Resultado do Format Text)
```

**IMPORTANTE:**
- Use `Round to Int` ou `Round64` + `Convert Int64 to Int`
- Garanta que está setando `SplitAmount` (variável de instância)
- Use `Get Split Amount` para pegar o valor atualizado

---

## ✅ **CORREÇÃO 4: Event Construct - Resetar Valores**

**PROBLEMA:** Variáveis podem ter valores antigos quando o widget é recriado.

**SOLUÇÃO:** Resetar todas as variáveis no `Event Construct`.

### **No Graph do WBP_SplitInput → Event Graph → Event Construct:**

```
[Event Construct]
  ↓
[Set Source Slot Widget]  ← Variável de instância
  └─ Value: None
  ↓
[Set Current Quantity]  ← Variável de instância
  └─ Value: 0
  ↓
[Set Split Amount]  ← Variável de instância
  └─ Value: 0
```

---

## 🔍 **CHECKLIST DE VERIFICAÇÃO:**

### **Setup Split Input:**
- [ ] Usa `Source Slot` (parâmetro) diretamente, não `Get Source Slot Widget`
- [ ] Usa `Item Quantity` (parâmetro) diretamente, não `Get Current Quantity`
- [ ] `Get Slot Data` usa `Source Slot` (parâmetro) como Target
- [ ] Max Value do Slider = `Item Quantity - 1` (convertido para Float)
- [ ] Set Split Amount = 1 (não 0)

### **Button Confirm:**
- [ ] `Get Source Slot Widget` pega a variável de instância
- [ ] `Is Valid?` valida antes de usar
- [ ] `Request Split Item` usa o Source Slot validado
- [ ] `Get Split Amount` pega a variável de instância atualizada

### **Slider On Value Changed:**
- [ ] `Round to Int` converte Float para Int
- [ ] `Set Split Amount` atualiza a variável de instância
- [ ] `Format Text` usa `Get Split Amount` (não o valor direto do slider)

### **Event Construct:**
- [ ] Reseta `SourceSlotWidget` para None
- [ ] Reseta `CurrentQuantity` para 0
- [ ] Reseta `SplitAmount` para 0

---

## 🐛 **TROUBLESHOOTING:**

### **Erro: "Acessado None ao tentar ler SourceSlotWidget"**
- **Causa:** Tentando usar `Get Source Slot Widget` antes de setar
- **Solução:** Use o parâmetro `Source Slot` diretamente na função `Setup Split Input`

### **Slider mostra sempre 1**
- **Causa:** Max Value não está sendo calculado corretamente
- **Solução:** Garanta que Max Value = `Item Quantity - 1` (convertido para Float)

### **"Erro: Slot inválido" ao confirmar**
- **Causa:** `SourceSlotWidget` não foi setado em `Setup Split Input`
- **Solução:** Garanta que `Set Source Slot Widget` usa o parâmetro `Source Slot`, não `Get Source Slot Widget`

---

## 📝 **RESUMO DAS MUDANÇAS:**

1. **Setup Split Input:**
   - Remover `Get Source Slot Widget` no início
   - Usar `Source Slot` (parâmetro) diretamente
   - Usar `Item Quantity` (parâmetro) diretamente
   - Só depois setar nas variáveis de instância

2. **Button Confirm:**
   - Validar `SourceSlotWidget` antes de usar
   - Se inválido, apenas fechar widget

3. **Slider On Value Changed:**
   - Garantir conversão correta Float → Int
   - Atualizar `SplitAmount` (variável de instância)

4. **Event Construct:**
   - Resetar todas as variáveis para valores padrão

---

**PRONTO! 🎉**

