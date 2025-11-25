# 🔍 DEBUG: Movimento Storage → Storage Não Funciona

## 🎯 **PROBLEMA:**

Quando você move um item dentro do storage (de um slot para outro), o som toca, o inventário é recarregado, mas o item **não muda de slot no banco de dados**.

---

## 🔍 **ANÁLISE DO FLUXO:**

### **1. `RequestMoveItem` no C++ (`UmbraInventorySlotWidget.cpp`):**

Para Storage → Storage (linhas 324-343):
```cpp
else if (bSourceIsStorage && bDestIsStorage)
{
    // Storage → Storage
    // O TargetSlotIndex já está convertido para 0-99, precisamos converter de volta para 50-149
    int32 DatabaseTargetSlotIndex = TargetSlotIndex + 50;
    
    // Usa a mesma função MoveItem do GameInstance, mas com índice do banco
    GameInstance->MoveItem(InventoryID, DatabaseTargetSlotIndex);
}
```

**Problema potencial:**
- O `SourceSlotIndex` pode estar em formato incorreto
- O código tenta detectar se está em formato 0-99 (storage) ou 50-149 (banco) nas linhas 248-261
- Mas pode estar falhando na detecção

---

## ✅ **VERIFICAÇÕES NECESSÁRIAS:**

### **1. Verificar os logs do Unreal:**

Quando você move um item dentro do storage, procure por estas mensagens:

```
[UmbraInventorySlotWidget] RequestMoveItem - InventoryID: X, From: Y (storage), To: Z (storage)
[UmbraGameInstance] 🔄 [AUDIT] Tentando mover item - Player: X, InventoryID: Y, TargetSlotIndex: Z
[UmbraGameInstance] ✅ [AUDIT] Item movido com sucesso - Player: X, InventoryID: Y, FromSlot: A, ToSlot: B
```

**O que verificar:**
- O `FromSlot` e `ToSlot` nos logs estão corretos?
- O `TargetSlotIndex` está entre 50-149 (formato do banco)?
- A requisição está sendo enviada?

---

### **2. Verificar o `SourceSlotIndex` no Blueprint:**

**No `OnDrop` do `WBP_InventorySlot`:**

Quando você arrasta um item do storage para outro slot do storage, verifique:

1. **O `Get Source Slot Widget` está retornando o widget correto?**
2. **O `Get Slot Index` do slot de origem está retornando o índice correto?**
   - Deve ser 0-99 (storage) ou 50-149 (banco)?
   - Depende de como o `SetSlotIndex` foi chamado no `CreateStorageSlots`

3. **O `Slot Index` do slot de destino está correto?**
   - Deve ser 0-99 (storage) ou 50-149 (banco)?

---

### **3. Verificar o `CreateStorageSlots`:**

**No `WBP_Storage` → `CreateStorageSlots`:**

Verifique se o `Set Slot Index` está sendo chamado com o índice correto:

```
For Loop (0 to 99)
  ├─ Create Widget (WBP_InventorySlot)
  ├─ Set Slot Index (Index + 50)  ← DEVE SER 50-149 (formato do banco)!
  └─ Set Parent Storage Widget (self)
```

**Se o `SetSlotIndex` está sendo chamado com `Index + 50` (50-149):**
- O `SlotData.SlotIndex` do slot de origem será 50-149
- O `SourceSlotIndex` passado para `RequestMoveItem` será 50-149
- O código nas linhas 248-261 deve detectar isso corretamente

**Se o `SetSlotIndex` está sendo chamado com `Index` (0-99):**
- O `SlotData.SlotIndex` do slot de origem será 0-99
- O `SourceSlotIndex` passado para `RequestMoveItem` será 0-99
- O código nas linhas 253-260 deve detectar isso corretamente

---

## 🔧 **CORREÇÃO POSSÍVEL:**

### **Problema: `SourceSlotIndex` não está sendo detectado corretamente**

**No `RequestMoveItem` (linhas 242-270):**

O código atual tenta detectar se o `SourceSlotIndex` está em formato 0-99 ou 50-149, mas pode estar falhando.

**Solução: Adicionar logs de debug:**

```cpp
// Se não encontrou no inventário, verifica no storage
if (InventoryID <= 0)
{
    UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - Procurando no storage, SourceSlotIndex: %d"), SourceSlotIndex);
    
    for (const FUmbraInventorySlot& StorageSlot : GameInstance->CurrentStorage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - Comparando: SourceSlotIndex=%d, StorageSlot.SlotIndex=%d (DB)"), 
            SourceSlotIndex, StorageSlot.SlotIndex);
        
        // Compara tanto com índice do banco (50-149) quanto com índice do storage (0-99)
        bool bMatches = false;
        if (StorageSlot.SlotIndex == SourceSlotIndex)
        {
            // SourceSlotIndex está no formato do banco (50-149)
            bMatches = true;
            UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - MATCH (formato DB): %d == %d"), 
                StorageSlot.SlotIndex, SourceSlotIndex);
        }
        else if (SourceSlotIndex >= 0 && SourceSlotIndex < 100)
        {
            // SourceSlotIndex está no formato do storage (0-99)
            int32 StorageSlotIndex = StorageSlot.SlotIndex - 50;
            if (StorageSlotIndex == SourceSlotIndex)
            {
                bMatches = true;
                UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - MATCH (formato storage): %d (DB) -> %d (storage) == %d"), 
                    StorageSlot.SlotIndex, StorageSlotIndex, SourceSlotIndex);
            }
        }
        
        if (bMatches && StorageSlot.InventoryID > 0)
        {
            InventoryID = StorageSlot.InventoryID;
            bSourceIsStorage = true;
            UE_LOG(LogTemp, Warning, TEXT("[UmbraInventorySlotWidget] RequestMoveItem - Item encontrado no storage! InventoryID: %d"), InventoryID);
            break;
        }
    }
}
```

---

## 📝 **PRÓXIMOS PASSOS:**

1. **Adicione os logs de debug acima no C++**
2. **Recompile o projeto**
3. **Tente mover um item dentro do storage**
4. **Verifique os logs no Unreal**
5. **Envie os logs para análise**

Os logs vão mostrar:
- Se o `SourceSlotIndex` está sendo detectado corretamente
- Se o `InventoryID` está sendo encontrado
- Se a requisição está sendo enviada com os parâmetros corretos

