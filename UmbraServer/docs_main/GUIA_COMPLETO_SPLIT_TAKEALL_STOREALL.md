# 🎯 GUIA COMPLETO: Split de Itens + Take All / Store All

## 📋 **RESUMO:**

Este guia implementa:
1. **Split de Itens** - Dividir pilhas de itens (Shift + Drag ou botão)
2. **Take All** - Mover todos os itens do storage para o inventário
3. **Store All** - Mover todos os itens do inventário para o storage

---

## ✅ **PARTE 1: APIs PHP Criadas**

### **1.1 split_item.php**
- **Localização:** `www/umbra_api/api/inventory/split_item.php`
- **Função:** Divide uma pilha de itens em duas pilhas
- **Parâmetros:**
  - `token` (obrigatório)
  - `inventory_id` (obrigatório)
  - `split_amount` (opcional, padrão: metade)
  - `target_slot_index` (opcional, auto-atribui se não fornecido)

### **1.2 store_all.php**
- **Localização:** `www/umbra_api/api/storage/store_all.php`
- **Função:** Move todos os itens do inventário (0-49) para o storage (50-149)
- **Parâmetros:**
  - `token` (obrigatório)

### **1.3 take_all.php**
- **Localização:** `www/umbra_api/api/storage/take_all.php`
- **Função:** Move todos os itens do storage (50-149) para o inventário (0-49)
- **Parâmetros:**
  - `token` (obrigatório)

---

## ✅ **PARTE 2: Funções C++ Implementadas**

### **2.1 UmbraGameInstance::SplitItem**
- **Localização:** `UmbraGameInstance.cpp`
- **Função:** Chama a API `split_item.php`
- **Parâmetros:**
  - `InventoryID` - ID do item a dividir
  - `SplitAmount` - Quantidade a separar (0 = metade)
  - `TargetSlotIndex` - Slot de destino (-1 = auto)

### **2.2 UmbraInventorySlotWidget::RequestSplitItem**
- **Localização:** `UmbraInventorySlotWidget.cpp`
- **Função:** Wrapper para chamar `GameInstance->SplitItem`
- **Parâmetros:**
  - `SplitAmount` - Quantidade a separar (0 = metade)
  - `TargetSlotIndex` - Slot de destino (-1 = auto)

### **2.3 UmbraStorageWidget::DepositAll**
- **Localização:** `UmbraStorageWidget.cpp`
- **Função:** Chama a API `store_all.php`
- **Retorna:** Quantidade de itens movidos (via callback)

### **2.4 UmbraStorageWidget::WithdrawAll**
- **Localização:** `UmbraStorageWidget.cpp`
- **Função:** Chama a API `take_all.php`
- **Retorna:** Quantidade de itens movidos (via callback)

---

## 🎨 **PARTE 3: Implementação no Blueprint**

### **3.1 Adicionar Botões no WBP_Storage**

**No Designer do WBP_Storage:**

1. **Adicionar Button_StoreAll:**
   - **Nome:** `Button_StoreAll`
   - **Texto:** "Store All" ou "Depositar Tudo"
   - **Posição:** Acima ou ao lado do grid de storage

2. **Adicionar Button_TakeAll:**
   - **Nome:** `Button_TakeAll`
   - **Texto:** "Take All" ou "Retirar Tudo"
   - **Posição:** Ao lado do `Button_StoreAll`

3. **Adicionar Button_Split (opcional):**
   - **Nome:** `Button_Split`
   - **Texto:** "Split" ou "Dividir"
   - **Posição:** Pode ser um botão de contexto no slot

---

### **3.2 Implementar Store All**

**No Event Graph do WBP_Storage:**

```
[Button_StoreAll: OnClicked]
  ↓
[Deposit All]  ← Função C++ do UmbraStorageWidget
  │ Return Value: ItemsDeposited (int32)
  ↓
[Branch (ItemsDeposited > 0)]
  ├─ TRUE:
  │   ├─ [Print String] "X itens depositados"
  │   └─ [Play Sound] (opcional)
  │
  └─ FALSE:
      └─ [Print String] "Nenhum item depositado"
```

**NOTA:** O `DepositAll` já recarrega automaticamente o inventário e storage via callbacks.

---

### **3.3 Implementar Take All**

**No Event Graph do WBP_Storage:**

```
[Button_TakeAll: OnClicked]
  ↓
[Withdraw All]  ← Função C++ do UmbraStorageWidget
  │ Return Value: ItemsWithdrawn (int32)
  ↓
[Branch (ItemsWithdrawn > 0)]
  ├─ TRUE:
  │   ├─ [Print String] "X itens retirados"
  │   └─ [Play Sound] (opcional)
  │
  └─ FALSE:
      └─ [Print String] "Nenhum item retirado"
```

**NOTA:** O `WithdrawAll` já recarrega automaticamente o inventário e storage via callbacks.

---

### **3.4 Implementar Split (Shift + Drag)**

**No Event Graph do WBP_InventorySlot:**

**OPÇÃO A: Shift + Drag (Recomendado)**

```
[OnDragDetected]  ← Override do evento
  ↓
[Get Key]  ← Verificar se Shift está pressionado
  │ Key: Left Shift (ou Right Shift)
  │
  └─ [Is Key Down?]
      ├─ TRUE: (Shift pressionado - dividir)
      │   ├─ [Get Slot Data] (self)
      │   ├─ [Break Umbra Inventory Slot]
      │   │     └─ Quantity
      │   ├─ [Branch (Quantity > 1)]
      │   │   ├─ TRUE:
      │   │   │   ├─ [Divide] (Quantity / 2)  ← Metade
      │   │   │   └─ [Request Split Item]  ← Função C++ do UmbraInventorySlotWidget
      │   │   │         └─ Split Amount: (Resultado do Divide)
      │   │   │         └─ Target Slot Index: -1 (auto)
      │   │   │
      │   │   └─ FALSE:
      │   │       └─ [Print String] "Item não pode ser dividido"
      │   │
      │   └─ [Return Handled]  ← Não criar drag normal
      │
      └─ FALSE: (Shift NÃO pressionado - drag normal)
          └─ [Create Item Drag Drop Operation]  ← Comportamento normal
```

**OPÇÃO B: Botão de Split no Slot**

```
[Button_Split: OnClicked]  ← Botão no slot
  ↓
[Get Slot Data] (self)
  ↓
[Break Umbra Inventory Slot]
  └─ Quantity
  ↓
[Branch (Quantity > 1)]
  ├─ TRUE:
  │   ├─ [Request Split Item]  ← Função C++
  │   │     └─ Split Amount: 0 (metade)
  │   │     └─ Target Slot Index: -1 (auto)
  │   │
  │   └─ [Print String] "Pilha dividida"
  │
  └─ FALSE:
      └─ [Print String] "Item não pode ser dividido"
```

---

### **3.5 Implementar Split com Input Numérico (Avançado)**

**Para permitir que o jogador escolha a quantidade:**

1. **Criar Widget de Input:**
   - **Nome:** `WBP_SplitInput`
   - **Componentes:**
     - `TextBlock_CurrentQuantity` - Mostra quantidade atual
     - `Slider_Amount` - Slider para escolher quantidade (1 até Quantity-1)
     - `TextBlock_Amount` - Mostra quantidade selecionada
     - `Button_Confirm` - Confirmar
     - `Button_Cancel` - Cancelar

2. **No WBP_InventorySlot:**

```
[Button_Split: OnClicked]
  ↓
[Get Slot Data] (self)
  ↓
[Break Umbra Inventory Slot]
  └─ Quantity
  ↓
[Branch (Quantity > 1)]
  ├─ TRUE:
  │   ├─ [Create Widget] (WBP_SplitInput)
  │   ├─ [Set Current Quantity] (Quantity)
  │   ├─ [Add to Viewport]
  │   └─ [Set Input Mode UI Only]
  │
  └─ FALSE:
      └─ [Print String] "Item não pode ser dividido"
```

3. **No WBP_SplitInput (Button_Confirm):**

```
[Button_Confirm: OnClicked]
  ↓
[Get Slider Value] (Slider_Amount)
  ↓
[Round to Int]
  ↓
[Request Split Item]  ← Do slot original
  └─ Split Amount: (Valor do slider)
  └─ Target Slot Index: -1
  ↓
[Remove from Parent]  ← Fechar widget
  ↓
[Set Input Mode Game And UI]  ← Voltar ao jogo
```

---

## 🔧 **PARTE 4: Detalhes de Implementação**

### **4.1 Split Item - Fluxo Completo**

1. **Jogador pressiona Shift + arrasta item**
2. **OnDragDetected detecta Shift**
3. **Chama `RequestSplitItem` com quantidade = metade**
4. **C++ chama `GameInstance->SplitItem`**
5. **API PHP divide a pilha no banco**
6. **C++ recarrega inventário e storage**
7. **Blueprint atualiza visual via `OnItemMoved`**

### **4.2 Store All - Fluxo Completo**

1. **Jogador clica "Store All"**
2. **Chama `DepositAll()` do C++**
3. **C++ chama API `store_all.php`**
4. **PHP move todos os itens (0-49 → 50-149)**
5. **C++ recarrega inventário e storage**
6. **Blueprint atualiza visual via `OnItemMoved`**

### **4.3 Take All - Fluxo Completo**

1. **Jogador clica "Take All"**
2. **Chama `WithdrawAll()` do C++**
3. **C++ chama API `take_all.php`**
4. **PHP move todos os itens (50-149 → 0-49)**
5. **C++ recarrega inventário e storage**
6. **Blueprint atualiza visual via `OnItemMoved`**

---

## 📝 **PARTE 5: Exemplo Completo de Blueprint**

### **5.1 Store All (Completo)**

```
[Event Construct]
  ↓
[Button_StoreAll: OnClicked]  ← Bind no Construct
  ↓
[Deposit All]  ← Função C++ (self)
  │ Return Value: ItemsDeposited
  ↓
[Branch (ItemsDeposited > 0)]
  ├─ TRUE:
  │   ├─ [Format Text] "{0} itens depositados"
  │   │     └─ {0}: ItemsDeposited
  │   ├─ [Print String]
  │   └─ [Play Sound] (opcional)
  │
  └─ FALSE:
      └─ [Print String] "Nenhum item para depositar"
```

### **5.2 Take All (Completo)**

```
[Event Construct]
  ↓
[Button_TakeAll: OnClicked]  ← Bind no Construct
  ↓
[Withdraw All]  ← Função C++ (self)
  │ Return Value: ItemsWithdrawn
  ↓
[Branch (ItemsWithdrawn > 0)]
  ├─ TRUE:
  │   ├─ [Format Text] "{0} itens retirados"
  │   │     └─ {0}: ItemsWithdrawn
  │   ├─ [Print String]
  │   └─ [Play Sound] (opcional)
  │
  └─ FALSE:
      └─ [Print String] "Nenhum item para retirar"
```

### **5.3 Split (Shift + Drag - Completo)**

```
[OnDragDetected]  ← Override
  ↓
[Get Player Controller]
  ↓
[Get Key] (Left Shift)
  ↓
[Is Key Down?]
  ├─ TRUE: (Shift pressionado)
  │   ├─ [Get Slot Data] (self)
  │   ├─ [Break Umbra Inventory Slot]
  │   │     └─ Inventory ID
  │   │     └─ Quantity
  │   │     └─ Item Template ID
  │   ├─ [Branch (Quantity > 1)]
  │   │   ├─ TRUE:
  │   │   │   ├─ [Divide] (Quantity / 2)
  │   │   │   ├─ [Round to Int]
  │   │   │   ├─ [Request Split Item] (self)
  │   │   │   │     └─ Split Amount: (Resultado)
  │   │   │   │     └─ Target Slot Index: -1
  │   │   │   │
  │   │   │   └─ [Return Handled]  ← Não criar drag normal
  │   │   │
  │   │   └─ FALSE:
  │   │       └─ [Return Unhandled]  ← Drag normal
  │   │
  │   └─ FALSE: (Shift NÃO pressionado)
  │       └─ [Create Item Drag Drop Operation]  ← Comportamento normal
  │             └─ Return Value: (Operation)
  │             └─ [Return Handled]
```

---

## ⚠️ **IMPORTANTE:**

1. **DepositAll e WithdrawAll retornam 0 imediatamente** - O valor real vem via callback
2. **Os callbacks já recarregam inventário e storage automaticamente**
3. **Split funciona tanto no inventário quanto no storage**
4. **SplitAmount = 0 divide ao meio automaticamente**
5. **TargetSlotIndex = -1 encontra slot automaticamente**

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **C++:**
- [x] API `split_item.php` criada
- [x] API `store_all.php` criada
- [x] API `take_all.php` criada
- [x] `UmbraGameInstance::SplitItem` implementado
- [x] `UmbraInventorySlotWidget::RequestSplitItem` implementado
- [x] `UmbraStorageWidget::DepositAll` implementado
- [x] `UmbraStorageWidget::WithdrawAll` implementado
- [x] Callbacks implementados

### **Blueprint:**
- [ ] Botões `Button_StoreAll` e `Button_TakeAll` adicionados no `WBP_Storage`
- [ ] `OnClicked` de `Button_StoreAll` conectado a `DepositAll`
- [ ] `OnClicked` de `Button_TakeAll` conectado a `WithdrawAll`
- [ ] `OnDragDetected` do `WBP_InventorySlot` verifica Shift
- [ ] Split implementado (Shift + Drag ou botão)
- [ ] Feedback visual implementado (mensagens, sons)

---

## 🎮 **TESTE:**

1. **Teste Split:**
   - Pegue um item empilhável (quantidade > 1)
   - Pressione Shift + arraste
   - Verifique se a pilha foi dividida

2. **Teste Store All:**
   - Tenha itens no inventário
   - Clique em "Store All"
   - Verifique se todos os itens foram para o storage

3. **Teste Take All:**
   - Tenha itens no storage
   - Clique em "Take All"
   - Verifique se todos os itens foram para o inventário

---

**PRONTO! 🎉**

