# 🔧 CORREÇÃO: RequestMoveItem para Suportar Storage

## ❌ **PROBLEMAS IDENTIFICADOS:**

1. **`RequestMoveItem` só buscava no `CurrentInventory`**
   - Quando o item estava no storage, não encontrava o `InventoryID`
   - Erro: "InventoryID não encontrado para slot X"

2. **Validação incorreta de `TargetSlotIndex`**
   - Validava apenas `TargetSlotIndex < 50`
   - Slots de storage usam índices 50-149 (banco) ou 0-99 (storage)
   - Erro: "TargetSlotIndex INVÁLIDO: 53"

3. **Não detectava origem e destino corretamente**
   - Não verificava se o item estava no inventário ou storage
   - Não verificava se o destino era inventário ou storage
   - Sempre chamava `MoveItem` (só funciona para inventário → inventário)

---

## ✅ **CORREÇÕES IMPLEMENTADAS:**

### **1. Detecção de Origem e Destino:**

```cpp
// Determina se o destino é inventário ou storage
bool bDestIsStorage = (ParentStorageWidget != nullptr);

// Busca no inventário (slots 0-49)
if (SourceSlotIndex >= 0 && SourceSlotIndex < 50) {
    // Busca no CurrentInventory
}

// Busca no storage (slots 50-149 do banco ou 0-99 do storage)
// Compara tanto com índice do banco quanto com índice do storage
```

### **2. Conversão de Índices:**

```cpp
// Se o destino é storage, converte índice do banco (50-149) para índice do storage (0-99)
if (bDestIsStorage) {
    TargetSlotIndex = TargetSlotIndex - 50; // Converte 50-149 → 0-99
}
```

### **3. Chamada da Função Apropriada:**

```cpp
if (bSourceIsStorage && !bDestIsStorage) {
    // Storage → Inventário
    ParentStorageWidget->MoveItemFromStorage(StorageID, TargetSlotIndex);
}
else if (!bSourceIsStorage && bDestIsStorage) {
    // Inventário → Storage
    ParentStorageWidget->MoveItemToStorage(InventoryID, TargetSlotIndex);
}
else if (!bSourceIsStorage && !bDestIsStorage) {
    // Inventário → Inventário
    GameInstance->MoveItem(InventoryID, TargetSlotIndex);
}
```

---

## 📋 **TESTES NECESSÁRIOS:**

1. ✅ **Inventário → Inventário** (slots 0-49)
2. ✅ **Inventário → Storage** (slot 0-49 → slot 0-99)
3. ✅ **Storage → Inventário** (slot 0-99 → slot 0-49)
4. ⚠️ **Storage → Storage** (não suportado ainda)

---

## 🚀 **PRÓXIMOS PASSOS:**

1. Compilar o código C++
2. Testar movimentação entre inventário e storage
3. Verificar logs para confirmar que os índices estão corretos
4. Se necessário, implementar movimentação Storage → Storage

