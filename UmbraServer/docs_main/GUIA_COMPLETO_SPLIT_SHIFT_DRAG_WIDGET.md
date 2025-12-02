# 🎯 GUIA COMPLETO: Split com Shift + Drag + Widget de Input

## 📋 **VISÃO GERAL:**

Quando o jogador pressiona **Shift + arrasta um item**, aparece o widget `WBP_SplitInput` com um slider para escolher a quantidade a dividir. Após confirmar, a pilha é dividida.

---

## 🏗️ **ESTRUTURA:**

1. **WBP_InventorySlot** - Detecta Shift + Drag e abre o widget
2. **WBP_SplitInput** - Widget com slider para escolher quantidade
3. **Comunicação** - WBP_SplitInput chama RequestSplitItem no slot original

---

## 📝 **PARTE 1: Configurar WBP_SplitInput**

### **1.1 Variáveis do WBP_SplitInput**

**No Graph do WBP_SplitInput → Variables:**

1. **Criar variável:**
   - **Nome:** `SourceSlotWidget`
   - **Tipo:** `WBP Inventory Slot Object Reference`
   - **Category:** Default
   - **Description:** "Referência ao slot que está sendo dividido"

2. **Criar variável:**
   - **Nome:** `CurrentQuantity`
   - **Tipo:** `Integer`
   - **Category:** Default
   - **Description:** "Quantidade total do item"

3. **Criar variável:**
   - **Nome:** `SplitAmount`
   - **Tipo:** `Integer`
   - **Category:** Default
   - **Description:** "Quantidade escolhida para dividir"

---

### **1.2 Componentes do WBP_SplitInput (Designer)**

**No Designer do WBP_SplitInput:**

1. **TextBlock_ItemName:**
   - **Nome:** `TextBlock_ItemName`
   - **Texto:** "Item Name"
   - **Posição:** Topo do widget

2. **TextBlock_CurrentQuantity:**
   - **Nome:** `TextBlock_CurrentQuantity`
   - **Texto:** "Quantidade: 0"
   - **Posição:** Abaixo do nome

3. **Slider_Amount:**
   - **Nome:** `Slider_Amount`
   - **Tipo:** Slider
   - **Min Value:** 1.0
   - **Max Value:** 100.0 (será atualizado dinamicamente)
   - **Value:** 1.0
   - **Posição:** Centro do widget

4. **TextBlock_SplitAmount:**
   - **Nome:** `TextBlock_SplitAmount`
   - **Texto:** "Dividir: 1"
   - **Posição:** Abaixo do slider

5. **Button_Confirm:**
   - **Nome:** `Button_Confirm`
   - **Texto:** "Confirmar"
   - **Posição:** Inferior esquerdo

6. **Button_Cancel:**
   - **Nome:** `Button_Cancel`
   - **Texto:** "Cancelar"
   - **Posição:** Inferior direito

---

### **1.3 Função: Setup Split Input**

**No Graph do WBP_SplitInput → Functions → Create Function:**

**Nome da Função:** `Setup Split Input`

**Parâmetros:**
- `Source Slot` (WBP Inventory Slot Object Reference)
- `Item Quantity` (Integer)

**Implementação:**

```
[Setup Split Input]  ← Função criada
  ├─ Input: Source Slot (WBP Inventory Slot Object Reference)
  ├─ Input: Item Quantity (Integer)
  ↓
[Set Source Slot Widget]  ← Variável local
  └─ Value: Source Slot
  ↓
[Set Current Quantity]  ← Variável local
  └─ Value: Item Quantity
  ↓
[Get Slot Data]  ← Do Source Slot
  └─ Target: Source Slot
  └─ Return Value: SlotData (FUmbraInventorySlot)
  ↓
[Break Umbra Inventory Slot]  ← Do SlotData
  └─ Item Template
  ↓
[Break Umbra Item Template]  ← Do Item Template
  └─ Item Name
  ↓
[Set Text]  ← TextBlock_ItemName
  └─ Text: Item Name
  ↓
[Format Text]  ← Para TextBlock_CurrentQuantity
  └─ Format: "Quantidade: {0}"
  └─ {0}: Item Quantity
  ↓
[Set Text]  ← TextBlock_CurrentQuantity
  └─ Text: (Resultado do Format Text)
  ↓
[Set Min Value]  ← Slider_Amount
  └─ Value: 1.0
  ↓
[Subtract]  ← Para calcular Max Value
  ├─ A: Item Quantity (Float)
  └─ B: 1.0
  ↓
[Set Max Value]  ← Slider_Amount
  └─ Value: (Resultado do Subtract)
  ↓
[Set Value]  ← Slider_Amount
  └─ Value: 1.0
  ↓
[Set Split Amount]  ← Variável local
  └─ Value: 1
```

---

### **1.4 Event: Slider Value Changed**

**No Graph do WBP_SplitInput → Event Graph:**

```
[Slider_Amount: On Value Changed]  ← Event do Slider
  ├─ In Value: (Float do slider)
  ↓
[Round to Int]  ← Converter para inteiro
  └─ Value: In Value
  ↓
[Set Split Amount]  ← Variável local
  └─ Value: (Resultado do Round to Int)
  ↓
[Format Text]  ← Para TextBlock_SplitAmount
  └─ Format: "Dividir: {0}"
  └─ {0}: Split Amount (convertido para Text)
  ↓
[Set Text]  ← TextBlock_SplitAmount
  └─ Text: (Resultado do Format Text)
```

**COMO OBTER OS NÓS:**
1. **Slider_Amount: On Value Changed:**
   - No Designer, selecione `Slider_Amount`
   - No Details, encontre "Events" → "On Value Changed"
   - Arraste para o Event Graph

2. **Round to Int:**
   - Right Click no Event Graph
   - Digite "Round to Int"
   - Selecione "Round to Int (Integer)"

3. **Format Text:**
   - Right Click no Event Graph
   - Digite "Format Text"
   - Selecione "Format Text"

4. **Set Text:**
   - Selecione `TextBlock_SplitAmount` no Designer
   - Arraste para o Event Graph
   - Selecione "Set Text"

---

### **1.5 Event: Button Confirm Clicked**

**No Graph do WBP_SplitInput → Event Graph:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Source Slot Widget]  ← Variável local
  └─ Return Value: Source Slot
  ↓
[Is Valid?]  ← Validar Source Slot
  └─ Object: Source Slot
  ↓
[Branch]  ← Se Source Slot é válido
  ├─ TRUE:
  │   ├─ [Get Split Amount]  ← Variável local
  │   │     └─ Return Value: Split Amount
  │   ├─ [Request Split Item]  ← Função C++ do Source Slot
  │   │     └─ Target: Source Slot
  │   │     └─ Split Amount: Split Amount
  │   │     └─ Target Slot Index: -1 (auto)
  │   │
  │   └─ [Remove from Parent]  ← Fechar widget
  │         └─ Target: self
  │
  └─ FALSE:
      └─ [Print String] "Erro: Slot inválido"
```

**COMO OBTER OS NÓS:**
1. **Button_Confirm: OnClicked:**
   - No Designer, selecione `Button_Confirm`
   - No Details, encontre "Events" → "On Clicked"
   - Arraste para o Event Graph

2. **Get Source Slot Widget:**
   - Right Click no Event Graph
   - Digite "Get Source Slot Widget"
   - Selecione a variável que você criou

3. **Is Valid?:**
   - Right Click no Event Graph
   - Digite "Is Valid"
   - Selecione "Is Valid?"

4. **Request Split Item:**
   - Conecte o `Return Value` do `Get Source Slot Widget` ao `Target` de `Request Split Item`
   - Right Click no Event Graph
   - Digite "Request Split Item"
   - Selecione a função do `UmbraInventorySlotWidget`

5. **Remove from Parent:**
   - Right Click no Event Graph
   - Digite "Remove from Parent"
   - Conecte `self` ao `Target`

---

### **1.6 Event: Button Cancel Clicked**

**No Graph do WBP_SplitInput → Event Graph:**

```
[Button_Cancel: OnClicked]
  ↓
[Remove from Parent]  ← Fechar widget
  └─ Target: self
```

**COMO OBTER OS NÓS:**
1. **Button_Cancel: OnClicked:**
   - No Designer, selecione `Button_Cancel`
   - No Details, encontre "Events" → "On Clicked"
   - Arraste para o Event Graph

---

### **1.7 Event: Construct (Opcional - Resetar Valores)**

**No Graph do WBP_SplitInput → Event Graph:**

```
[Event Construct]
  ↓
[Set Source Slot Widget]  ← Variável local
  └─ Value: None
  ↓
[Set Current Quantity]  ← Variável local
  └─ Value: 0
  ↓
[Set Split Amount]  ← Variável local
  └─ Value: 0
```

---

## 📝 **PARTE 2: Implementar Shift + Drag no WBP_InventorySlot**

### **2.1 Override OnDragDetected**

**No Graph do WBP_InventorySlot → Event Graph:**

**IMPORTANTE:** Você precisa fazer **Override** do evento `OnDragDetected`.

**COMO FAZER OVERRIDE:**
1. No Event Graph, **Right Click**
2. Digite "OnDragDetected"
3. Selecione **"Add Event"** → **"OnDragDetected"** (não "OnDragDetected Event", mas o evento real)
4. OU: No Designer, selecione o widget raiz, no Details encontre "Events" → "On Drag Detected"

**Implementação Completa:**

```
[OnDragDetected]  ← Override do evento
  ├─ Input: MyGeometry (Geometry)
  ├─ Input: PointerEvent (Pointer Event)
  ↓
[Get Player Controller]  ← Obter Player Controller
  └─ Return Value: Player Controller
  ↓
[Get Key]  ← Verificar se Shift está pressionado
  └─ Target: Player Controller
  └─ Key: Left Shift  ← OU Right Shift
  └─ Return Value: Key
  ↓
[Is Key Down?]  ← Verificar se está pressionado
  └─ Key: (Key do Get Key)
  └─ Return Value: (Boolean)
  ↓
[Branch]  ← Verificar se Shift está pressionado
  ├─ TRUE: (Shift pressionado - ABRIR WIDGET DE SPLIT)
  │   ├─ [Get Slot Data]  ← Obter dados do slot atual
  │   │     └─ Target: self
  │   │     └─ Return Value: SlotData
  │   ├─ [Break Umbra Inventory Slot]  ← Quebrar dados
  │   │     └─ Inventory ID
  │   │     └─ Quantity
  │   │     └─ Item Template ID
  │   ├─ [Branch]  ← Verificar se pode dividir
  │   │     └─ Condition: Quantity > 1
  │   │     ├─ TRUE: (Pode dividir)
  │   │     │   ├─ [Create Widget]  ← Criar WBP_SplitInput
  │   │     │   │     └─ Class: WBP_SplitInput
  │   │     │   │     └─ Return Value: SplitInputWidget
  │   │     │   ├─ [Setup Split Input]  ← Configurar widget
  │   │     │   │     └─ Target: SplitInputWidget
  │   │     │   │     └─ Source Slot: self
  │   │     │   │     └─ Item Quantity: Quantity
  │   │     │   ├─ [Add to Viewport]  ← Mostrar widget
  │   │     │   │     └─ Target: SplitInputWidget
  │   │     │   ├─ [Set Input Mode UI Only]  ← Pausar jogo
  │   │     │   │     └─ Target: Player Controller
  │   │     │   │     └─ Widget to Focus: SplitInputWidget
  │   │     │   │
  │   │     │   └─ [Return Handled]  ← Não criar drag normal
  │   │     │
  │   │     └─ FALSE: (Não pode dividir)
  │   │         └─ [Print String] "Item não pode ser dividido"
  │   │         └─ [Return Unhandled]  ← Continuar com drag normal
  │   │
  │   └─ FALSE: (Shift NÃO pressionado - DRAG NORMAL)
  │       └─ [Create Item Drag Drop Operation]  ← Comportamento normal
  │             └─ Target: self
  │             └─ Return Value: (Operation)
  │             └─ [Return Handled]
```

**COMO OBTER CADA NÓ:**

1. **OnDragDetected:**
   - No Event Graph, **Right Click**
   - Digite "OnDragDetected"
   - Selecione **"Add Event"** → **"OnDragDetected"**
   - OU: No Designer, selecione o widget raiz, no Details encontre "Events" → "On Drag Detected"

2. **Get Player Controller:**
   - Right Click no Event Graph
   - Digite "Get Player Controller"
   - Selecione "Get Player Controller"

3. **Get Key:**
   - Right Click no Event Graph
   - Digite "Get Key"
   - Selecione "Get Key"
   - Conecte o `Return Value` do `Get Player Controller` ao `Target`
   - No Details do nó, encontre "Key" e selecione "Left Shift"

4. **Is Key Down?:**
   - Right Click no Event Graph
   - Digite "Is Key Down"
   - Selecione "Is Key Down?"
   - Conecte o `Return Value` do `Get Key` ao `Key` do `Is Key Down?`

5. **Get Slot Data:**
   - Right Click no Event Graph
   - Digite "Get Slot Data"
   - Selecione a função do `UmbraInventorySlotWidget`
   - Conecte `self` ao `Target`

6. **Break Umbra Inventory Slot:**
   - Right Click no Event Graph
   - Digite "Break Umbra Inventory Slot"
   - Selecione "Break Umbra Inventory Slot"
   - Conecte o `Return Value` do `Get Slot Data` ao input

7. **Create Widget:**
   - Right Click no Event Graph
   - Digite "Create Widget"
   - Selecione "Create Widget"
   - No Details, encontre "Class" e selecione `WBP_SplitInput`

8. **Setup Split Input:**
   - Conecte o `Return Value` do `Create Widget` ao `Target` de `Setup Split Input`
   - Right Click no Event Graph
   - Digite "Setup Split Input"
   - Selecione a função que você criou no `WBP_SplitInput`

9. **Add to Viewport:**
   - Right Click no Event Graph
   - Digite "Add to Viewport"
   - Selecione "Add to Viewport"
   - Conecte o `Return Value` do `Create Widget` ao `Target`

10. **Set Input Mode UI Only:**
    - Right Click no Event Graph
    - Digite "Set Input Mode UI Only"
    - Selecione "Set Input Mode UI Only"
    - Conecte o `Return Value` do `Get Player Controller` ao `Target`
    - Conecte o `Return Value` do `Create Widget` ao `Widget to Focus`

11. **Return Handled / Return Unhandled:**
    - Right Click no Event Graph
    - Digite "Return Handled" ou "Return Unhandled"
    - Selecione a opção apropriada

12. **Create Item Drag Drop Operation:**
    - Right Click no Event Graph
    - Digite "Create Item Drag Drop Operation"
    - Selecione a função do `UmbraInventorySlotWidget`
    - Conecte `self` ao `Target`

---

## 📝 **PARTE 3: Ajustes Finais no WBP_SplitInput**

### **3.1 Adicionar Input Mode no Button Cancel**

**No Event Graph do WBP_SplitInput:**

```
[Button_Cancel: OnClicked]
  ↓
[Get Player Controller]
  ↓
[Set Input Mode Game And UI]  ← Voltar ao jogo
  └─ Target: Player Controller
  ↓
[Remove from Parent]  ← Fechar widget
  └─ Target: self
```

**COMO OBTER:**
1. **Get Player Controller:** Right Click → "Get Player Controller"
2. **Set Input Mode Game And UI:** Right Click → "Set Input Mode Game And UI"
3. Conecte o `Return Value` do `Get Player Controller` ao `Target`

---

### **3.2 Adicionar Input Mode no Button Confirm**

**No Event Graph do WBP_SplitInput:**

```
[Button_Confirm: OnClicked]
  ↓
[Get Player Controller]
  ↓
[Set Input Mode Game And UI]  ← Voltar ao jogo
  └─ Target: Player Controller
  ↓
[Get Source Slot Widget]
  ↓
[Is Valid?]
  ↓
[Branch]
  ├─ TRUE:
  │   ├─ [Get Split Amount]
  │   ├─ [Request Split Item]
  │   │     └─ Target: Source Slot
  │   │     └─ Split Amount: Split Amount
  │   │     └─ Target Slot Index: -1
  │   │
  │   └─ [Remove from Parent]
  │
  └─ FALSE:
      └─ [Remove from Parent]
```

**IMPORTANTE:** O `Set Input Mode Game And UI` deve vir ANTES do `Request Split Item`, para que o jogo volte ao normal imediatamente.

---

## 🔧 **PARTE 4: Lógica Completa do Fluxo**

### **4.1 Fluxo Completo:**

1. **Jogador pressiona Shift + arrasta item**
   - `OnDragDetected` é chamado
   - Verifica se Shift está pressionado
   - Se SIM: Abre `WBP_SplitInput`
   - Se NÃO: Cria drag normal

2. **WBP_SplitInput é aberto**
   - `Setup Split Input` é chamado
   - Slider é configurado (Min: 1, Max: Quantity-1)
   - Textos são atualizados

3. **Jogador ajusta slider**
   - `On Value Changed` atualiza `SplitAmount`
   - Texto "Dividir: X" é atualizado

4. **Jogador clica Confirmar**
   - `Request Split Item` é chamado no slot original
   - Widget é fechado
   - Input mode volta ao normal

5. **C++ processa split**
   - Chama API `split_item.php`
   - Recarrega inventário e storage
   - Blueprint atualiza visual

---

## ⚠️ **IMPORTANTE - DETALHES CRÍTICOS:**

### **1. OnDragDetected - Override Correto**

**PROBLEMA COMUM:** Se você usar "OnDragDetected Event" em vez de fazer Override, o drag normal não funcionará.

**SOLUÇÃO:**
- Use o evento real `OnDragDetected` (Override)
- Se não conseguir fazer Override, você pode usar `OnMouseButtonDown` + `Detect Drag`, mas é mais complexo

### **2. Get Key - Key Correto**

**PROBLEMA COMUM:** `Get Key` pode não funcionar se o Player Controller não estiver configurado corretamente.

**SOLUÇÃO ALTERNATIVA:**
```
[Get Player Controller]
  ↓
[Get Input Key State]  ← Alternativa
  └─ Target: Player Controller
  └─ Key: Left Shift
  └─ Return Value: (Boolean)
```

### **3. Set Input Mode - Ordem Importante**

**IMPORTANTE:** Sempre volte o Input Mode ANTES de fechar o widget, ou o jogo pode ficar travado.

**ORDEM CORRETA:**
1. Set Input Mode Game And UI
2. Request Split Item (ou Remove from Parent)

### **4. Slider Max Value**

**IMPORTANTE:** O Max Value do slider deve ser `Quantity - 1`, não `Quantity`, porque você não pode dividir toda a quantidade (deve sobrar pelo menos 1).

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **WBP_SplitInput:**
- [ ] Variáveis criadas (SourceSlotWidget, CurrentQuantity, SplitAmount)
- [ ] Componentes criados (Slider, TextBlocks, Buttons)
- [ ] Função `Setup Split Input` implementada
- [ ] Event `On Value Changed` do Slider implementado
- [ ] Event `OnClicked` do Button_Confirm implementado
- [ ] Event `OnClicked` do Button_Cancel implementado
- [ ] Set Input Mode adicionado nos botões

### **WBP_InventorySlot:**
- [ ] Override de `OnDragDetected` feito
- [ ] Verificação de Shift implementada
- [ ] Branch para abrir widget implementado
- [ ] Create Widget de WBP_SplitInput implementado
- [ ] Setup Split Input chamado corretamente
- [ ] Add to Viewport implementado
- [ ] Set Input Mode UI Only implementado
- [ ] Return Handled/Unhandled implementado corretamente

---

## 🎮 **TESTE:**

1. **Teste Shift + Drag:**
   - Pegue um item empilhável (quantidade > 1)
   - Pressione Shift + arraste
   - Verifique se o widget aparece

2. **Teste Slider:**
   - Ajuste o slider
   - Verifique se o texto "Dividir: X" atualiza

3. **Teste Confirmar:**
   - Escolha uma quantidade
   - Clique em Confirmar
   - Verifique se a pilha foi dividida

4. **Teste Cancelar:**
   - Abra o widget
   - Clique em Cancelar
   - Verifique se o widget fecha e o jogo volta ao normal

---

## 🐛 **TROUBLESHOOTING:**

### **Widget não aparece:**
- Verifique se `Add to Viewport` está conectado
- Verifique se `Create Widget` está usando a classe correta
- Verifique se `Setup Split Input` está sendo chamado

### **Shift não detecta:**
- Tente usar `Get Input Key State` em vez de `Get Key`
- Verifique se o Player Controller está correto
- Tente usar Right Shift em vez de Left Shift

### **Slider não atualiza:**
- Verifique se `On Value Changed` está conectado
- Verifique se `Set Text` está conectado ao TextBlock correto

### **Split não funciona:**
- Verifique se `Request Split Item` está recebendo o Source Slot correto
- Verifique se `Split Amount` está sendo passado corretamente
- Verifique os logs do Unreal Engine para erros

---

**PRONTO! 🎉**

