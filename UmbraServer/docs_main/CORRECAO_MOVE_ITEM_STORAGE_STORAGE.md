# 🔴 CORREÇÃO: Move Item Dentro do Storage Não Funciona

## 🎯 **PROBLEMA IDENTIFICADO:**

Quando você move um item **dentro do storage** (Storage → Storage):
1. O `RequestMoveItem` chama `GameInstance->MoveItem(InventoryID, DatabaseTargetSlotIndex)`
2. O `GameInstance->MoveItem` faz a requisição e quando completa, chama `OnItemMoved.Broadcast(InventoryID, FromSlot, ToSlot)`
3. **MAS o `WBP_Storage` NÃO está escutando esse delegate!**

O `OnItemMoved()` (BlueprintImplementableEvent) do `WBP_Storage` só é chamado quando:
- `MoveItemToStorage` é bem-sucedido
- `MoveItemFromStorage` é bem-sucedido

**NÃO é chamado quando `GameInstance->MoveItem` é usado para Storage → Storage!**

---

## ✅ **SOLUÇÃO:**

### **PASSO 1: Conectar o Delegate `OnItemMoved` do GameInstance no WBP_Storage**

**No `WBP_Storage` → Event Graph → Event Construct:**

Após `Cast to Umbra Game Instance` e `Set MyGameInstance`, adicione:

```
Event Construct
  ↓
Create Storage Slots
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then (Cast bem-sucedido):
  ├─ Set MyGameInstance = As Umbra Game Instance
  ├─ Assign On Storage Loaded
  │     └─ Event: OnStorageLoaded_Event (Custom Event)
  ├─ Assign On Item Moved  ← NOVO! Conectar delegate do GameInstance
  │     └─ Event: OnItemMovedFromGameInstance_Event (Custom Event)
  └─ Load Storage
```

**Como fazer:**
1. No `Event Construct` do `WBP_Storage`
2. Após `Set MyGameInstance`
3. Arraste o `As Umbra Game Instance` → **Event** → **Assign On Item Moved**
4. Crie um **Custom Event** chamado `OnItemMovedFromGameInstance_Event`
5. Adicione 3 parâmetros ao Custom Event:
   - `InventoryID` (Integer)
   - `FromSlot` (Integer)
   - `ToSlot` (Integer)
6. Conecte o `Event` pin do `Assign On Item Moved` ao `OnItemMovedFromGameInstance_Event`

---

### **PASSO 2: Implementar o Custom Event `OnItemMovedFromGameInstance_Event`**

**No `WBP_Storage` → Event Graph:**

```
Custom Event: OnItemMovedFromGameInstance_Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Branch (FromSlot >= 50 OR ToSlot >= 50)  ← Verifica se movimento envolve storage
  ├─ TRUE:  ← Movimento do storage detectado!
  │   ├─ Print String: "Movimento do storage detectado! From: [FromSlot], To: [ToSlot]"  ← DEBUG
  │   ├─ Get Game Instance
  │   │   └─ Cast to Umbra Game Instance
  │   │       └─ then:
  │   │           └─ Load Storage  ← RECARREGAR DADOS DO STORAGE
  │   │
  │   └─ (O LoadStorage dispara OnStorageLoaded, que chama UpdateAllSlotsVisual)
  │
  └─ FALSE:  ← Movimento apenas no inventário (não afeta storage)
      (nada)
```

**Como fazer:**
1. Crie o Custom Event `OnItemMovedFromGameInstance_Event` com os 3 parâmetros
2. Adicione um `Branch` node
3. Conecte `FromSlot >= 50` OU `ToSlot >= 50` ao `Condition` do `Branch`
4. No `TRUE` do `Branch`:
   - Adicione `Get Game Instance` → `Cast to Umbra Game Instance`
   - No `then` do Cast, adicione `Load Storage` (do GameInstance)

---

### **PASSO 3: Garantir que `OnStorageLoaded_Event` chama `UpdateAllSlotsVisual`**

**No `WBP_Storage` → Event Graph → OnStorageLoaded_Event:**

```
Custom Event: OnStorageLoaded_Event
  ↓
Update All Slots Visual  ← ATUALIZAR VISUAL APÓS CARREGAR
```

**Como fazer:**
1. No `OnStorageLoaded_Event` (Custom Event já existente)
2. Adicione `Update All Slots Visual` após o evento ser disparado

---

## 📝 **RESUMO:**

1. ✅ **Conectar delegate `OnItemMoved` do GameInstance no `Event Construct` do `WBP_Storage`**
2. ✅ **Criar Custom Event `OnItemMovedFromGameInstance_Event` com 3 parâmetros**
3. ✅ **Verificar se movimento envolve storage (FromSlot >= 50 OR ToSlot >= 50)**
4. ✅ **Chamar `LoadStorage` do GameInstance quando movimento envolve storage**
5. ✅ **Garantir que `OnStorageLoaded_Event` chama `UpdateAllSlotsVisual`**

---

## ⚠️ **IMPORTANTE:**

- O `OnItemMoved()` (BlueprintImplementableEvent) é chamado apenas para `MoveItemToStorage` e `MoveItemFromStorage`
- O `OnItemMovedFromGameInstance_Event` (Custom Event) é chamado para **TODOS** os movimentos, incluindo Storage → Storage
- Use `OnStorageLoaded_Event` para chamar `UpdateAllSlotsVisual` após `LoadStorage` completar (mais confiável que usar `Delay`)

---

## 🐛 **TROUBLESHOOTING:**

Se ainda não funcionar:

1. **Verifique se o delegate está conectado:**
   - Adicione `Print String` no `OnItemMovedFromGameInstance_Event`
   - Deve aparecer no log quando mover um item

2. **Verifique se `LoadStorage` está sendo chamado:**
   - Adicione logs no C++ `LoadStorage` do `GameInstance`
   - Deve aparecer no log quando o movimento envolver storage

3. **Verifique se `UpdateAllSlotsVisual` está sendo chamado:**
   - Adicione `Print String` no início de `UpdateAllSlotsVisual`
   - Deve aparecer no log após `LoadStorage` completar

