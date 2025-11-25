# 🔴 CORREÇÃO CRÍTICA: get_storage.php - slot_index Incorreto

## 🎯 **PROBLEMA IDENTIFICADO:**

O `get_storage.php` estava usando `s.slot_index` da tabela `player_storage` em vez de `i.slot_index` da tabela `player_inventory`.

**Consequências:**
- Quando um item é movido no storage usando `move_item.php`, apenas `player_inventory.slot_index` é atualizado
- O `player_storage.slot_index` não é atualizado (e não deveria ser, pois é redundante)
- O `get_storage.php` retornava o `slot_index` antigo da tabela `player_storage`
- O C++ atualizava o slot errado na UI

---

## ✅ **CORREÇÃO APLICADA:**

**Arquivo:** `www/umbra_api/api/storage/get_storage.php`

**Mudanças:**
1. ✅ Alterado `s.slot_index` para `i.slot_index` (linha 53)
2. ✅ Adicionado filtro `AND i.slot_index >= 50 AND i.slot_index < 150` para garantir apenas slots do storage
3. ✅ Alterado `ORDER BY s.slot_index` para `ORDER BY i.slot_index` (linha 78)

**Resultado:**
- O PHP agora retorna o `slot_index` correto da tabela `player_inventory` (50-149)
- O C++ atualiza o slot correto na UI
- O item aparece no slot correto após o movimento

---

## 🔍 **PROBLEMA ADICIONAL:**

O erro `Attempted to access index 137 from array 'SlotWidgets' of length 50` indica que o `WBP_Inventory` está tentando processar slots do storage.

**Causa:** O `UpdateAllSlotsVisual` do `WBP_Inventory` pode estar processando todos os slots retornados por `GetAllInventorySlots()`, incluindo slots do storage.

**Solução:** Verificar se o `UpdateAllSlotsVisual` do `WBP_Inventory` filtra corretamente apenas slots 0-49.

---

## 📝 **PRÓXIMOS PASSOS:**

1. ✅ **Correção do PHP aplicada** - O `get_storage.php` agora retorna o `slot_index` correto
2. ⚠️ **Verificar o `UpdateAllSlotsVisual` do `WBP_Inventory`** - Garantir que só processa slots 0-49
3. **Testar:** Mover um item no storage e verificar se aparece no slot correto

