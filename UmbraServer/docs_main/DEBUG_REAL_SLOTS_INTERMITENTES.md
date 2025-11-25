# 🔍 DEBUG REAL: Por Que Slots Não Funcionam Intermitentemente

**PROBLEMA REAL:**
- Slots não aceitam itens intermitentemente
- Alguns itens entram, outros não
- Funcionamento inconsistente

---

## 🔍 **ANÁLISE DO CÓDIGO C++:**

Olhando `RequestMoveItem` (linha 176-233), vejo que:

1. Ele usa `SlotData.SlotIndex` como `TargetSlotIndex`
2. Se `TargetSlotIndex < 0 || TargetSlotIndex >= 50`, ele **FALHA** e retorna
3. O problema: quando o slot está vazio, o `SlotData.SlotIndex` pode estar **0 ou incorreto**

---

## ✅ **CORREÇÃO NECESSÁRIA:**

O problema está em `RequestMoveItem` - ele depende de `SlotData.SlotIndex` estar correto, mas quando o slot está vazio, isso pode não estar garantido.

### **SOLUÇÃO 1: Adicionar Logs de Debug no C++**

Adicione logs em `RequestMoveItem` para ver o que está acontecendo:

**Arquivo:** `UmbraInventorySlotWidget.cpp`, função `RequestMoveItem`

**Adicione ANTES da validação:**
```cpp
UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - SourceSlotIndex: %d, SlotData.SlotIndex: %d, InventoryID: %d"),
    SourceSlotIndex, SlotData.SlotIndex, SlotData.InventoryID);
```

### **SOLUÇÃO 2: Verificar se `SetSlotData` está preservando `SlotIndex`**

No Blueprint `WBP_Inventory`, no `OnInventoryLoaded_Event`, no caminho `FALSE`:

**Verifique se o `Make Umbra Inventory Slot` está conectado corretamente:**
- `SlotIndex` deve estar conectado ao `Index` do `For Loop`
- O struct criado deve ser passado **diretamente** ao `Set Slot Data` (sem variáveis intermediárias)

### **SOLUÇÃO 3: Adicionar Logs no Blueprint `OnDrop`**

No `WBP_InventorySlot`, no evento `OnDrop`, adicione logs:

```
OnDrop
  ├─ Print String: "=== OnDrop INICIO ==="
  ├─ Get Source Slot Widget
  ├─ Is Valid? (Source Slot Widget)
  │   ├─ FALSE: Print String "ERRO: SourceSlotWidget NULL" → Return
  │   └─ TRUE:
  │       ├─ Print String: "SourceSlotWidget válido"
  │       ├─ Get Slot Data (Source Slot Widget)
  │       ├─ Break Umbra Inventory Slot
  │       │   └─ Slot Index → Print String: "Source SlotIndex: X"
  │       ├─ Get Slot Data (self)
  │       ├─ Break Umbra Inventory Slot
  │       │   └─ Slot Index → Print String: "Target SlotIndex: X"
  │       ├─ Process Item Drop
  │       └─ Print String: "ProcessItemDrop chamado"
```

---

## 📋 **O QUE VERIFICAR:**

1. **Quando você tenta dropar um item em um slot vazio:**
   - O `OnDrop` está sendo chamado?
   - O `SourceSlotWidget` é válido?
   - O `Target SlotIndex` está correto (não 0)?
   - O `ProcessItemDrop` está sendo chamado?
   - O `RequestMoveItem` está sendo chamado?
   - Qual é o valor de `SlotData.SlotIndex` no momento do drop?

2. **Compare slots que funcionam vs slots que não funcionam:**
   - Qual é a diferença entre eles?
   - O `SlotIndex` está correto em ambos?
   - O `InventoryID` está correto?

---

## 🎯 **ENVIE OS LOGS:**

Depois de adicionar os logs, tente dropar um item em um slot vazio que não está funcionando e envie:
1. Todos os logs do console (especialmente os de `OnDrop`, `ProcessItemDrop`, `RequestMoveItem`)
2. Qual slot específico está falhando
3. O que aparece nos logs para esse slot

Com esses logs, vou identificar exatamente onde está o problema.

---

**ADICIONE OS LOGS E ENVIE O RESULTADO!** 🔍

