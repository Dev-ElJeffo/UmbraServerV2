# 🎯 GUIA: DRAG & DROP DE ITENS COM C++ (SOLUÇÃO PROFISSIONAL)

## ✅ **O QUE FOI CRIADO:**

### **1. UmbraInventorySlotWidget**
Classe base para slots do inventário com suporte a drag & drop de itens.

### **2. UmbraItemDragDropOperation**
Classe que carrega os dados do item sendo arrastado.

---

## 📁 **ARQUIVOS CRIADOS:**

```
UmbraEternumUE/Source/UmbraEternumUE/UI/
├── UmbraInventorySlotWidget.h          (Declaração do slot)
├── UmbraInventorySlotWidget.cpp        (Implementação do slot)
├── UmbraItemDragDropOperation.h        (Declaração da operação)
└── UmbraItemDragDropOperation.cpp      (Implementação da operação)
```

---

## 🔧 **FUNÇÕES C++ DISPONÍVEIS:**

### **UmbraInventorySlotWidget:**

#### **1. SetSlotData(FUmbraInventorySlot NewSlotData)**
- Define os dados do slot
- Atualiza o visual automaticamente
- Chame quando carregar o inventário

#### **2. IsSlotEmpty()**
- Verifica se o slot está vazio
- Retorna true se não tem item

#### **3. HasItem()**
- Verifica se o slot tem um item
- Oposto de IsSlotEmpty()

#### **4. CreateItemDragOperation()**
- Cria a operação de drag & drop
- Chame no `OnDragDetected` do slot

#### **5. CanAcceptItem(FUmbraInventorySlot DraggedItemData)**
- Valida se o slot pode aceitar o item
- Chame no `OnDrop` antes de processar

#### **6. ProcessItemDrop(UUmbraInventorySlotWidget* DraggedSlotWidget)**
- Processa o drop do item
- Chama a API automaticamente
- Chame no `OnDrop` após validar

#### **7. ClearSlot()**
- Limpa o slot (torna vazio)
- Atualiza o visual

### **UmbraItemDragDropOperation:**

#### **1. GetDraggedItemData()**
- Retorna os dados do item sendo arrastado

#### **2. GetSourceSlotWidget()**
- Retorna o widget do slot de origem

#### **3. GetSourceSlotIndex()**
- Retorna o índice do slot de origem

#### **4. IsValid()**
- Valida se a operação está correta

---

## 🎮 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **PASSO 1: Mudar a Classe Pai do WBP_InventorySlot**

1. Abra `WBP_InventorySlot`
2. No painel **"Class Settings"** (no topo)
3. Em **"Parent Class"**, mude de:
   - ❌ `UserWidget`
   - ✅ `UmbraInventorySlotWidget`
4. Compile e salve

---

### **PASSO 2: OnDragDetected (SUPER SIMPLES!)**

No `WBP_InventorySlot`, adicione o override:

```blueprint
[On Drag Detected] (Override)
    │
    └─> [Create Item Drag Operation] ◄─── FUNÇÃO C++!
            │ Return Value: UmbraItemDragDropOperation
            │
            └─> [Return Node]
```

**Passo a passo:**
1. No painel **Functions**, clique em **Override**
2. Selecione **"On Drag Detected"**
3. Adicione a função **`Create Item Drag Operation`**
4. Conecte o Return Value ao Return Node

**Total: 1 nó!** (ao invés de 5+!)

---

### **PASSO 3: OnDrop (SIMPLES TAMBÉM!)**

No `WBP_InventorySlot`, adicione o override:

```blueprint
[On Drop] (Override)
    │ Operation (DragDropOperation)
    │
    ├─> [Cast to UmbraItemDragDropOperation]
    │       │ Object: Operation
    │       │ Output: UmbraItemDragDropOperation
    │       │
    │       └─> [Is Valid?] (Branch)
    │               │
    │               ├─ TRUE ───> [Get Source Slot Widget]
    │               │               │ Target: (Cast result)
    │               │               │ Output: SourceSlotWidget
    │               │               │
    │               │               └─> [Process Item Drop] ◄─── FUNÇÃO C++!
    │               │                       │ Dragged Slot Widget: (SourceSlotWidget)
    │               │                       │
    │               │                       └─> [Return Node]
    │               │                           │ Return Value: TRUE
    │               │
    │               └─ FALSE ──> [Return Node]
    │                           │ Return Value: FALSE
```

**Passo a passo:**

1. No painel **Functions**, clique em **Override**
2. Selecione **"On Drop"**

3. Do pin **Operation**, adicione **`Cast to UmbraItemDragDropOperation`**

4. Do output do Cast, adicione **`Is Valid?`** (Branch)

5. No Branch TRUE:
   - Adicione `Get Source Slot Widget` (do Cast result)
   - Adicione `Process Item Drop` (função C++!)
   - Conecte o Source Slot Widget ao pin Dragged Slot Widget
   - Conecte ao Return Node com Return Value = TRUE

6. No Branch FALSE:
   - Conecte ao Return Node com Return Value = FALSE

**Total: 5 nós** (ao invés de 15+!)

---

### **PASSO 4: Implementar os Eventos Blueprint**

O C++ chama dois eventos que você PODE implementar (opcional):

#### **OnItemMoved (Opcional - Feedback Visual):**

```blueprint
[Event On Item Moved]
    │ From Slot (int32)
    │ To Slot (int32)
    │
    └─> [Play Sound: ItemMove]
        │
        └─> [Print String: "Item movido!"]
```

**Útil para:**
- Tocar som de sucesso
- Animação de feedback
- Efeitos visuais

#### **OnDropFailed (Opcional - Feedback de Erro):**

```blueprint
[Event On Drop Failed]
    │ Reason (String)
    │
    └─> [Play Sound: Error]
        │
        └─> [Print String: "Não pode mover: " + Reason]
```

**Útil para:**
- Tocar som de erro
- Mostrar mensagem de erro
- Feedback visual de falha

---

## 📊 **COMPARAÇÃO: BLUEPRINT vs C++**

### **❌ ANTES (Tudo no Blueprint):**

**OnDragDetected:**
```
- Create DraggedItem Widget
- Create BP_ItemDragDropOperation
- Set Payload
- Set Default Drag Visual
- Set Pivot
= 5+ nós
```

**OnDrop:**
```
- Cast to BP_ItemDragDropOperation
- Get Payload
- Break Struct
- Get Source Slot
- Validate Slot
- Get Target Slot
- Check If Can Move
- Call API (MoveItem)
- Update Both Slots
= 15+ nós
```

**TOTAL:** 20+ nós + muita lógica complexa!

---

### **✅ AGORA (C++ + Blueprint):**

**OnDragDetected:**
```
- Create Item Drag Operation  (função C++)
= 1 nó
```

**OnDrop:**
```
- Cast to UmbraItemDragDropOperation
- Is Valid? (Branch)
- Get Source Slot Widget
- Process Item Drop  (função C++)
= 5 nós
```

**TOTAL:** 6 nós! (70% de redução!)

**Tudo mais (validação, API, atualização) está no C++!**

---

## 🐛 **DEBUGGING AUTOMÁTICO:**

O C++ já inclui logs detalhados:

```cpp
LogTemp: [UmbraInventorySlotWidget] SetSlotData - Slot: 5, InventoryID: 123, ItemID: 1, Quantity: 10
LogTemp: [UmbraInventorySlotWidget] CreateItemDragOperation - Criado! Slot: 5, ItemID: 1
LogTemp: [UmbraInventorySlotWidget] ProcessItemDrop - Movendo item do slot 5 para slot 10
LogTemp: [UmbraInventorySlotWidget] RequestMoveItem - InventoryID: 123, From: 5, To: 10
LogTemp: [UmbraInventorySlotWidget] OnItemMoved - Item movido com sucesso!
```

**Não precisa adicionar Print Strings!**

---

## 🔥 **COMO IMPLEMENTAR:**

### **1. Compilar o C++:**

```powershell
# Feche o Unreal Editor primeiro!

# Na pasta do projeto UE
cd D:\UmbraServerV2\UmbraEternumUE

# Compile
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" UmbraEternumUE.sln /t:Build /p:Configuration="Development Editor" /p:Platform=Win64
```

### **2. Abra o Unreal Editor**

### **3. Configure o WBP_InventorySlot:**
1. Abra `WBP_InventorySlot`
2. **Class Settings** → **Parent Class** → `UmbraInventorySlotWidget`
3. Compile e salve

### **4. Implemente os Overrides:**

#### **OnDragDetected:**
- Override → On Drag Detected
- Adicione `Create Item Drag Operation`
- Conecte ao Return Node

#### **OnDrop:**
- Override → On Drop
- Cast to `UmbraItemDragDropOperation`
- Branch (Is Valid?)
- TRUE: Get Source Slot Widget → Process Item Drop → Return TRUE
- FALSE: Return FALSE

### **5. (Opcional) Implementar Eventos:**
- Event On Item Moved (feedback de sucesso)
- Event On Drop Failed (feedback de erro)

---

## ✅ **CHECKLIST:**

**Compilação:**
- [ ] Arquivos C++ criados
- [ ] C++ compilado com sucesso
- [ ] Sem erros de compilação

**WBP_InventorySlot:**
- [ ] Parent Class = `UmbraInventorySlotWidget`
- [ ] OnDragDetected implementado
- [ ] OnDrop implementado
- [ ] (Opcional) OnItemMoved implementado
- [ ] (Opcional) OnDropFailed implementado

**Teste:**
- [ ] Consegue arrastar itens
- [ ] Consegue soltar em slots vazios
- [ ] Consegue trocar itens entre slots
- [ ] API é chamada corretamente
- [ ] Inventário atualiza após mover
- [ ] Logs aparecem no Output Log

---

## 🎯 **LÓGICA AUTOMÁTICA DO C++:**

### **O que o C++ faz automaticamente:**

1. **Validação:**
   - ✅ Verifica se o slot pode ser arrastado
   - ✅ Verifica se o slot está vazio
   - ✅ Verifica se pode aceitar o item
   - ✅ Valida tipos de itens (stack, swap)

2. **Operação:**
   - ✅ Cria a operação de drag
   - ✅ Carrega os dados do item
   - ✅ Mantém referência ao slot de origem

3. **API:**
   - ✅ Busca o UmbraGameInstance
   - ✅ Encontra o InventoryID correto
   - ✅ Chama `MoveItem()` com parâmetros corretos

4. **Feedback:**
   - ✅ Logs detalhados para debug
   - ✅ Eventos Blueprint para UI
   - ✅ Callbacks de sucesso/erro

### **O que você faz no Blueprint:**

1. Chamar `Create Item Drag Operation` (1 nó)
2. Chamar `Process Item Drop` (1 nó)
3. (Opcional) Adicionar feedback visual/sonoro

**MUITO MAIS SIMPLES!**

---

## 🚀 **FEATURES AVANÇADAS (JÁ INCLUÍDAS):**

### **1. Validação Inteligente:**
```cpp
// Slot vazio? Aceita qualquer item
// Mesmo item? Faz stack
// Item diferente? Troca de lugar
```

### **2. Proteção contra Erros:**
```cpp
// Não pode arrastar de slot que não permite
// Não pode dropar em slot que não aceita
// Não pode mover para o mesmo slot
// Valida se o InventoryID existe
```

### **3. Integração com API:**
```cpp
// Busca automaticamente o Game Instance
// Encontra o InventoryID correto
// Chama MoveItem() com parâmetros corretos
// Atualização automática via delegate OnInventoryUpdated
```

---

## 📝 **EXEMPLO VISUAL COMPLETO:**

### **WBP_InventorySlot - OnDragDetected:**

```
┌────────────────────────────────────────────────────┐
│ On Drag Detected (Override)                       │
├────────────────────────────────────────────────────┤
│                                                    │
│  [My Geometry]                                     │
│  [Pointer Event]                                   │
│        │                                           │
│        ▼                                           │
│  ┌──────────────────────────────────────┐         │
│  │ Create Item Drag Operation           │         │
│  │   Return Value: ItemDragDropOperation│         │
│  └──────────────┬───────────────────────┘         │
│                 │                                  │
│                 ▼                                  │
│  ┌──────────────────────────────────────┐         │
│  │ Return Node                          │         │
│  │   Return Value: (DragDropOperation)  │         │
│  └──────────────────────────────────────┘         │
│                                                    │
└────────────────────────────────────────────────────┘
```

### **WBP_InventorySlot - OnDrop:**

```
┌────────────────────────────────────────────────────┐
│ On Drop (Override)                                │
├────────────────────────────────────────────────────┤
│                                                    │
│  [Operation]                                       │
│        │                                           │
│        ▼                                           │
│  ┌──────────────────────────────────────┐         │
│  │ Cast to UmbraItemDragDropOperation   │         │
│  │   Object: Operation                  │         │
│  └──────────────┬───────────────────────┘         │
│                 │                                  │
│                 ▼                                  │
│  ┌──────────────────────────────────────┐         │
│  │ Is Valid? (Branch)                   │         │
│  └─────┬────────────────────────┬───────┘         │
│        │ TRUE                   │ FALSE            │
│        ▼                        ▼                  │
│  ┌─────────────────┐      ┌──────────┐           │
│  │ Get Source Slot │      │ Return   │           │
│  │   Widget        │      │  FALSE   │           │
│  └─────┬───────────┘      └──────────┘           │
│        │                                           │
│        ▼                                           │
│  ┌──────────────────────────────────────┐         │
│  │ Process Item Drop                    │         │
│  │   Dragged Slot Widget: (Source)     │         │
│  └──────────────┬───────────────────────┘         │
│                 │                                  │
│                 ▼                                  │
│  ┌──────────────────────────────────────┐         │
│  │ Return Node                          │         │
│  │   Return Value: TRUE                 │         │
│  └──────────────────────────────────────┘         │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 🎨 **PRÓXIMAS FEATURES (FÁCIL ADICIONAR):**

### **No C++ (IMPLEMENTADO! ✅):**
- ✅ `CanStackWith()` - Verifica se pode fazer stack
- ✅ `GetMaxStackSize()` - Retorna stack máximo
- ✅ `SplitStack()` - Dividir pilha de itens
- ✅ `ValidateItemType()` - Validar tipo de slot (arma, armadura, etc)
- ✅ `TryStackItems()` - Empilha automaticamente
- ✅ **Sistema de Armazém (100 slots)** - `UmbraStorageWidget` + `UmbraStorageActor`

**Veja:**
- `GUIA_FEATURES_AVANCADAS_INVENTARIO.md` - **GUIA COMPLETO!**
- `REFERENCIA_RAPIDA_ARMAZEM.md` - **QUICK START (5 min)**

### **No Blueprint:**
- Animação de drag (trail effect)
- Som ao arrastar/soltar
- Visual diferente para itens raros
- Highlight do slot ao passar o mouse

**Tudo isso se torna MUITO mais fácil com C++!**

---

## 💡 **DICAS PRO:**

### **1. Visual do Drag:**
Você pode criar um widget customizado para o drag:

```blueprint
[On Drag Detected]
    ├─> [Create Widget: WBP_DraggedItemVisual]
    │       │ Output: DragVisualWidget
    │       │
    │       └─> [Set Item Icon/Name]
    │
    └─> [Create Item Drag Operation]
            │ Default Drag Visual: DragVisualWidget
```

### **2. Feedback Visual:**
```blueprint
[Event On Item Moved]
    └─> [Play Animation: SuccessFlash]
        └─> [Play Sound: ItemMove]

[Event On Drop Failed]
    └─> [Play Animation: ErrorShake]
        └─> [Play Sound: Error]
```

### **3. Tooltips:**
O Drag & Drop não interfere com tooltips!
Você pode ter os dois ao mesmo tempo.

---

## 🚀 **RESULTADO FINAL:**

Com esta solução C++:

✅ **6 nós** ao invés de 20+
✅ **Lógica complexa em C++** (rápido e otimizado)
✅ **Debugging automático** (logs detalhados)
✅ **Validações robustas** (menos bugs)
✅ **Integração com API** (automática)
✅ **Fácil de manter** (um lugar para mudar)
✅ **Reutilizável** (qualquer slot pode usar)
✅ **Profissional** (código limpo e organizado)

**Seu inventário vai ficar incrível! 🎯✨**

---

## 📚 **REFERÊNCIAS:**

- `GUIA_CPP_DRAG_DROP_INVENTARIO.md` - Drag & Drop do widget inventário
- `COMPARACAO_BLUEPRINT_VS_CPP_DRAG.md` - Comparação das abordagens
- `GUIA_COMPLETO_DRAG_DROP.md` - Guia antigo (Blueprint puro)

---

**COMPILE E TESTE AGORA! É MUITO MAIS SIMPLES DO QUE PARECE! 🔥**

