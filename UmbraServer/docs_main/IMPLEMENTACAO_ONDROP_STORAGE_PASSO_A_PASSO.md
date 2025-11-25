# 🎯 IMPLEMENTAÇÃO: OnDrop para Storage - Passo a Passo

## ✅ **STATUS ATUAL:**

- ✅ `WBP_Storage` criado e funcionando
- ✅ `CreateStorageSlots` funcionando (100 slots criados)
- ✅ `UpdateAllSlotsVisual` funcionando
- ✅ Ambos os painéis (Inventory e Storage) abrem simultaneamente

---

## 🎯 **PRÓXIMO PASSO: Implementar OnDrop no WBP_InventorySlot**

### **OBJETIVO:**

Modificar o `OnDrop` do `WBP_InventorySlot` para detectar se o item veio do inventário ou do storage, e chamar a função apropriada.

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Abrir WBP_InventorySlot**

1. Abra o Blueprint `WBP_InventorySlot`
2. Vá para **Event Graph**
3. Localize o **Override Function** `OnDrop`

---

### **PASSO 2: Analisar a Estrutura Atual do OnDrop**

**A estrutura atual deve ser algo como:**

```
OnDrop
  ├─ Input: MyGeometry, PointerEvent, Operation
  ↓
Cast to Umbra Item Drag Drop Operation
  └─ Object: Operation
  ↓
then:
  ├─ Get Source Slot Widget
  ├─ Is Valid? (Source Slot Widget)
  │   ├─ TRUE:
  │   │   └─ Process Item Drop (Source Slot Widget)
  │   │       └─ Return Handled
  │   │
  │   └─ FALSE:
  │       Return Unhandled
  │
  └─ CastFailed:
      Return Unhandled
```

---

### **PASSO 3: Adicionar Verificação de Origem e Destino**

**Você precisa adicionar ANTES de `Process Item Drop`:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓
then:
  ├─ Get Source Slot Widget
  ├─ Is Valid? (Source Slot Widget)
  │   ├─ TRUE:
  │   │   ├─ Get Slot Data (Source Slot Widget)
  │   │   ├─ Break Umbra Inventory Slot (Source)
  │   │   │     └─ Inventory ID, Slot Index
  │   │   ├─ Get Slot Data (self)
  │   │   ├─ Break Umbra Inventory Slot (self)
  │   │   │     └─ Slot Index
  │   │   │
  │   │   └─ VERIFICAÇÃO DE ORIGEM:
  │   │       Get Parent Storage Widget (Source Slot Widget)
  │   │       ↓
  │   │       Is Valid? (Parent Storage Widget da origem)
  │   │       ├─ TRUE: Origem = STORAGE
  │   │       │   └─ (continuar abaixo)
  │   │       │
  │   │       └─ FALSE: Origem = INVENTÁRIO
  │   │           └─ (continuar abaixo)
  │   │
  │   └─ FALSE:
  │       Return Unhandled
```

---

### **PASSO 4: Implementar Lógica para Origem = STORAGE**

**Se origem = STORAGE:**

```
Is Valid? (Parent Storage Widget da origem) = TRUE
  ↓
VERIFICAÇÃO DE DESTINO:
  Get Parent Storage Widget (self)
  ↓
  Is Valid? (Parent Storage Widget do destino)
  ├─ TRUE: Destino = STORAGE
  │   └─ Return Unhandled  ← Não implementar mover dentro do storage
  │
  └─ FALSE: Destino = INVENTÁRIO
      ├─ Get Parent Storage Widget (Source Slot Widget)  ← Obtém WBP_Storage
      ├─ Is Valid? (WBP_Storage)
      ├─ TRUE:
      │   ├─ Move Item From Storage
      │   │     └─ Target: WBP_Storage (do Get acima)
      │   │     └─ Storage Item ID: InventoryID (do Source)
      │   │     └─ Target Slot Index: SlotIndex (do self, 0-49)
      │   ├─ Branch (Return Value)
      │   │   ├─ TRUE:
      │   │   │   ├─ Load Storage (WBP_Storage)
      │   │   │   ├─ Get Game Instance
      │   │   │   ├─ Cast to Umbra Game Instance
      │   │   │   └─ Load Inventory
      │   │   │   └─ Return Handled
      │   │   │
      │   │   └─ FALSE:
      │   │       └─ Return Unhandled
      │   │
      │   └─ FALSE:
      │       Return Unhandled
```

---

### **PASSO 5: Implementar Lógica para Origem = INVENTÁRIO**

**Se origem = INVENTÁRIO:**

```
Is Valid? (Parent Storage Widget da origem) = FALSE
  ↓
VERIFICAÇÃO DE DESTINO:
  Get Parent Storage Widget (self)
  ↓
  Is Valid? (Parent Storage Widget do destino)
  ├─ TRUE: Destino = STORAGE
  │   ├─ Get Parent Storage Widget (self)  ← Obtém WBP_Storage
  │   ├─ Is Valid? (WBP_Storage)
  │   ├─ TRUE:
  │   │   ├─ Get Slot Index (self)
  │   │   ├─ Subtract (SlotIndex - 50)  ← Converte 50-149 para 0-99
  │   │   ├─ Move Item To Storage
  │   │   │     └─ Target: WBP_Storage (do Get acima)
  │   │   │     └─ Inventory Item ID: InventoryID (do Source)
  │   │   │     └─ Target Slot Index: Resultado do Subtract (0-99)
  │   │   ├─ Branch (Return Value)
  │   │   │   ├─ TRUE:
  │   │   │   │   ├─ Load Storage (WBP_Storage)
  │   │   │   │   ├─ Get Game Instance
  │   │   │   │   ├─ Cast to Umbra Game Instance
  │   │   │   │   └─ Load Inventory
  │   │   │   │   └─ Return Handled
  │   │   │   │
  │   │   │   └─ FALSE:
  │   │   │       └─ Return Unhandled
  │   │   │
  │   │   └─ FALSE:
  │   │       Return Unhandled
  │   │
  │   └─ FALSE: Destino = INVENTÁRIO
  │       └─ (Lógica existente de mover dentro do inventário)
  │       └─ Process Item Drop (Source Slot Widget)
  │       └─ Return Handled
```

---

## 🔧 **DETALHES DE IMPLEMENTAÇÃO:**

### **COMO OBTER CADA NÓ:**

1. **`Get Parent Storage Widget`:**
   - Clique com botão direito → **"Get Parent Storage Widget"**
   - Esta função está no `UUmbraInventorySlotWidget` (C++)
   - Conecte o `WBP_InventorySlot` ao `Target` pin

2. **`Move Item To Storage`:**
   - Clique com botão direito → **"Move Item To Storage"**
   - Esta função está no `UUmbraStorageWidget` (C++)
   - **PROBLEMA:** Precisa de `Target` = `WBP_Storage`
   - **SOLUÇÃO:** Use `Get Parent Storage Widget (self)` para obter o `WBP_Storage`

3. **`Move Item From Storage`:**
   - Clique com botão direito → **"Move Item From Storage"**
   - Esta função está no `UUmbraStorageWidget` (C++)
   - **PROBLEMA:** Precisa de `Target` = `WBP_Storage`
   - **SOLUÇÃO:** Use `Get Parent Storage Widget (Source Slot Widget)` para obter o `WBP_Storage`

4. **`Subtract` (para converter índice):**
   - Clique com botão direito → **"Subtract"** (Integer - Integer)
   - Conecte `Get Slot Index (self)` ao primeiro pin
   - Conecte `Make Literal Int (50)` ao segundo pin
   - Isso converte 50-149 → 0-99

---

## ⚠️ **IMPORTANTE:**

1. **Ordem das verificações:**
   - Primeiro: Verificar origem (Source Slot Widget)
   - Depois: Verificar destino (self)
   - Por último: Chamar função apropriada

2. **Obter WBP_Storage:**
   - Use `Get Parent Storage Widget` do slot que pertence ao storage
   - Se origem = storage → use `Get Parent Storage Widget (Source Slot Widget)`
   - Se destino = storage → use `Get Parent Storage Widget (self)`

3. **Conversão de índices:**
   - Storage usa índices 50-149
   - Arrays do Blueprint usam 0-99
   - Use `Subtract (SlotIndex - 50)` para converter

4. **Recarregar após mover:**
   - Sempre chame `Load Storage` e `Load Inventory` após mover
   - Isso garante que ambos os widgets sejam atualizados

---

## 🧪 **TESTE:**

1. Abra o inventário e o storage
2. Arraste um item do inventário para o storage
   - **Deve:** Item sair do inventário e aparecer no storage
3. Arraste um item do storage para o inventário
   - **Deve:** Item sair do storage e aparecer no inventário
4. Arraste um item dentro do inventário
   - **Deve:** Funcionar como antes (lógica existente)

---

## 📝 **RESUMO:**

1. ✅ Adicione verificação de `ParentStorageWidget` da origem
2. ✅ Adicione verificação de `ParentStorageWidget` do destino
3. ✅ Se origem = storage e destino = inventário → `MoveItemFromStorage`
4. ✅ Se origem = inventário e destino = storage → `MoveItemToStorage`
5. ✅ Sempre recarregue ambos os widgets após mover

