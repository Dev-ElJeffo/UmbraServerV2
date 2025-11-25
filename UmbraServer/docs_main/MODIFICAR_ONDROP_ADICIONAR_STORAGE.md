# 🔧 MODIFICAR OnDrop: Adicionar Lógica de Storage

## ✅ **STATUS:**

- ✅ Storage funcionando (100 slots criados)
- ✅ Inventory funcionando
- ✅ Ambos os painéis abrem simultaneamente

---

## 🎯 **OBJETIVO:**

Modificar o `OnDrop` existente no `WBP_InventorySlot` para suportar drag and drop entre inventário e storage.

---

## 📋 **ESTRUTURA ATUAL DO OnDrop:**

**Você provavelmente tem algo assim:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
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
```

---

## 🔧 **MODIFICAÇÃO: Adicionar Verificações ANTES de ProcessItemDrop**

**ADICIONE estas verificações ANTES de `Process Item Drop`:**

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
  │   │       │   └─ (ir para lógica de storage → inventário)
  │   │       │
  │   │       └─ FALSE: Origem = INVENTÁRIO
  │   │           └─ (ir para lógica de inventário → storage ou inventário)
  │   │
  │   └─ FALSE:
  │       Return Unhandled
```

---

## 📋 **IMPLEMENTAÇÃO COMPLETA:**

### **SEÇÃO 1: Origem = STORAGE**

**Após `Is Valid? (Parent Storage Widget da origem) = TRUE`:**

```
TRUE: Origem = STORAGE
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
      │   ├─ Move Item From Storage Blueprint
      │   │     └─ Target: WBP_Storage
      │   │     └─ Storage Item ID: InventoryID (do Source)
      │   │     └─ Target Slot Index: SlotIndex (do self, 0-49)
      │   └─ Return Handled
      │
      └─ FALSE:
          Return Unhandled
```

### **SEÇÃO 2: Origem = INVENTÁRIO**

**Após `Is Valid? (Parent Storage Widget da origem) = FALSE`:**

```
FALSE: Origem = INVENTÁRIO
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
  │   │   ├─ Move Item To Storage Blueprint
  │   │   │     └─ Target: WBP_Storage
  │   │   │     └─ Inventory Item ID: InventoryID (do Source)
  │   │   │     └─ Target Slot Index: Resultado do Subtract (0-99)
  │   │   └─ Return Handled
  │   │
  │   └─ FALSE:
  │       Return Unhandled
  │
  └─ FALSE: Destino = INVENTÁRIO
      └─ (Lógica existente - manter como está)
      └─ Process Item Drop (Source Slot Widget)
      └─ Return Handled
```

---

## 🔧 **COMO OBTER OS NÓS:**

### **1. Get Parent Storage Widget:**
- Clique com botão direito → **"Get Parent Storage Widget"**
- Esta função está no `UUmbraInventorySlotWidget` (C++)
- Conecte o `WBP_InventorySlot` ao `Target` pin

### **2. Move Item To Storage Blueprint:**
- Clique com botão direito → **"Move Item To Storage Blueprint"**
- Esta função está no `WBP_Storage` (Blueprint)
- **IMPORTANTE:** Precisa de `Target` = `WBP_Storage`
- Use `Get Parent Storage Widget (self)` para obter o `WBP_Storage`

### **3. Move Item From Storage Blueprint:**
- Clique com botão direito → **"Move Item From Storage Blueprint"**
- Esta função está no `WBP_Storage` (Blueprint)
- **IMPORTANTE:** Precisa de `Target` = `WBP_Storage`
- Use `Get Parent Storage Widget (Source Slot Widget)` para obter o `WBP_Storage`

### **4. Subtract (para converter índice):**
- Clique com botão direito → **"Subtract"** (Integer - Integer)
- Conecte `Get Slot Index (self)` ao primeiro pin
- Conecte `Make Literal Int (50)` ao segundo pin
- Isso converte 50-149 → 0-99

---

## ⚠️ **ORDEM DE IMPLEMENTAÇÃO:**

1. **PRIMEIRO:** Crie as funções Blueprint no `WBP_Storage`:
   - `MoveItemToStorageBlueprint`
   - `MoveItemFromStorageBlueprint`
   - Custom Events para callbacks

2. **DEPOIS:** Modifique o `OnDrop` no `WBP_InventorySlot`:
   - Adicione verificações de origem e destino
   - Chame as funções Blueprint quando necessário

---

## 📝 **RESUMO DA ESTRUTURA:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓
Get Source Slot Widget
  ↓
Is Valid? (Source Slot Widget)
  ├─ TRUE:
  │   ├─ Get Slot Data (Source e self)
  │   ├─ Break Umbra Inventory Slot (Source e self)
  │   │
  │   └─ VERIFICAÇÃO DE ORIGEM:
  │       Get Parent Storage Widget (Source)
  │       ↓
  │       Is Valid? (Parent Storage Widget da origem)
  │       ├─ TRUE: Origem = STORAGE
  │       │   └─ Verificar destino
  │       │       ├─ Destino = STORAGE → Return Unhandled
  │       │       └─ Destino = INVENTÁRIO → MoveItemFromStorageBlueprint
  │       │
  │       └─ FALSE: Origem = INVENTÁRIO
  │           └─ Verificar destino
  │               ├─ Destino = STORAGE → MoveItemToStorageBlueprint
  │               └─ Destino = INVENTÁRIO → ProcessItemDrop (lógica existente)
  │
  └─ FALSE:
      Return Unhandled
```

---

## 🧪 **TESTE:**

1. Implemente as funções Blueprint primeiro
2. Depois modifique o `OnDrop`
3. Teste arrastar item do inventário para storage
4. Teste arrastar item do storage para inventário
5. Teste arrastar item dentro do inventário (deve funcionar como antes)

