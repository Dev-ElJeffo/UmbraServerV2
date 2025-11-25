# 🚀 GUIA COMPLETO: Drag and Drop entre Inventário e Storage

## 🎯 **OBJETIVO:**

Implementar drag and drop funcional entre:
- **Inventário → Storage** (mover item do inventário para o armazém)
- **Storage → Inventário** (mover item do armazém para o inventário)
- **Inventário → Inventário** (mover dentro do inventário - já funciona)
- **Storage → Storage** (mover dentro do storage - opcional, não implementar por enquanto)

---

## 📋 **PRÉ-REQUISITOS:**

✅ `WBP_Storage` criado e funcionando  
✅ `WBP_Inventory` criado e funcionando  
✅ `WBP_InventorySlot` criado e funcionando  
✅ `CreateStorageSlots` funcionando  
✅ `LoadStorage()` funcionando  
✅ Variável `ParentStorageWidget` existe no `WBP_InventorySlot` (C++)

---

## 🔧 **PARTE 1: Verificar Variável `ParentStorageWidget`**

### **PASSO 1.1: Verificar se a variável existe**

1. **Abra o `WBP_InventorySlot`** no Unreal Engine
2. **Vá para o painel "Variables"**
3. **Procure por `ParentStorageWidget`** (tipo: `Umbra Storage Widget Object Reference`)
4. **Se NÃO existir**, você precisa compilar o projeto C++ novamente (a variável já existe no C++)

### **PASSO 1.2: Verificar se está sendo setada no `CreateStorageSlots`**

1. **Abra o `WBP_Storage`**
2. **Vá para a função `CreateStorageSlots`**
3. **Verifique se existe um nó `Set Parent Storage Widget`:**
   - Deve estar após `Create Widget (WBP_InventorySlot)`
   - Deve ter `Target: Return Value` (do Create Widget)
   - Deve ter `Parent Storage Widget: self` (WBP_Storage)

**Se não existir, adicione:**
```
Create Widget (WBP_InventorySlot)
  ↓
Set Parent Storage Widget (Target: Return Value do Create Widget)
  └─ Parent Storage Widget: self (WBP_Storage)
```

---

## 🔧 **PARTE 2: Implementar OnDrop no `WBP_InventorySlot`**

### **PASSO 2.1: Localizar o OnDrop Atual**

1. **Abra o `WBP_InventorySlot`**
2. **Vá para o Event Graph**
3. **Localize o Override Function `OnDrop`**

### **PASSO 2.2: Estrutura Atual (Provavelmente)**

O `OnDrop` atual provavelmente tem esta estrutura:

```
OnDrop (Override)
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

### **PASSO 2.3: Modificar o OnDrop para Suportar Storage**

**Substitua o `Process Item Drop` por esta lógica:**

```
OnDrop (Override)
  ├─ Input: MyGeometry, PointerEvent, Operation
  ↓
Cast to Umbra Item Drag Drop Operation
  └─ Object: Operation
  ↓
then:
  ├─ Get Source Slot Widget
  ├─ Is Valid? (Source Slot Widget)
  │   ├─ TRUE:
  │   │   ├─ Get Slot Data (Source Slot Widget)
  │   │   ├─ Break Umbra Inventory Slot (Source)
  │   │   │     └─ Inventory ID, Slot Index (origem)
  │   │   │
  │   │   ├─ Get Slot Index (self)
  │   │   │     └─ Return Value: Slot Index (destino)
  │   │   │
  │   │   ├─ Branch (Inventory ID > 0?)  ← Verifica se há item para mover
  │   │   │   ├─ TRUE:
  │   │   │   │   │
  │   │   │   │   └─ VERIFICAR ORIGEM E DESTINO:
  │   │   │   │
  │   │   │   │       Get Parent Storage Widget (Source Slot Widget)
  │   │   │   │       ↓
  │   │   │   │       Is Valid? (Parent Storage Widget da origem)
  │   │   │   │       ├─ TRUE: Origem = STORAGE
  │   │   │   │       │   │
  │   │   │   │       │   └─ VERIFICAR DESTINO:
  │   │   │   │       │       Get Parent Storage Widget (self)
  │   │   │   │       │       ↓
  │   │   │   │       │       Is Valid? (Parent Storage Widget do destino)
  │   │   │   │       │       ├─ TRUE: Destino = STORAGE
  │   │   │   │       │       │   └─ Return Unhandled  ← Não implementar mover dentro do storage
  │   │   │   │       │       │
  │   │   │   │       │       └─ FALSE: Destino = INVENTÁRIO
  │   │   │   │       │           │
  │   │   │   │       │           └─ MOVER DO STORAGE PARA O INVENTÁRIO:
  │   │   │   │       │               Get Parent Storage Widget (Source Slot Widget)
  │   │   │   │       │               ↓
  │   │   │   │       │               Is Valid? (WBP_Storage)
  │   │   │   │       │               ├─ TRUE:
  │   │   │   │       │               │   ├─ Get Slot Data (Source Slot Widget)
  │   │   │   │       │               │   ├─ Break Umbra Inventory Slot
  │   │   │   │       │               │   │     └─ Inventory ID
  │   │   │   │       │               │   │
  │   │   │   │       │               │   ├─ Get Game Instance
  │   │   │   │       │               │   ├─ Cast to Umbra Game Instance
  │   │   │   │       │               │   ├─ Get Storage Item ID (do CurrentStorage usando InventoryID)
  │   │   │   │       │               │   │   └─ (Precisa buscar o storage_id no GameInstance)
  │   │   │   │       │               │   │
  │   │   │   │       │               │   ├─ Move Item From Storage
  │   │   │   │       │               │   │     └─ Target: WBP_Storage (do Get Parent Storage Widget)
  │   │   │   │       │               │   │     └─ Storage Item ID: (storage_id obtido acima)
  │   │   │   │       │               │   │     └─ Target Slot Index: Slot Index (do self, 0-49)
  │   │   │   │       │               │   │
  │   │   │   │       │               │   └─ Branch (Return Value)
  │   │   │   │       │               │       ├─ TRUE: Return Handled
  │   │   │   │       │               │       └─ FALSE: Return Unhandled
  │   │   │   │       │               │
  │   │   │   │       │               └─ FALSE:
  │   │   │   │       │                   Return Unhandled
  │   │   │   │       │
  │   │   │   │       └─ FALSE: Origem = INVENTÁRIO
  │   │   │   │           │
  │   │   │   │           └─ VERIFICAR DESTINO:
  │   │   │   │               Get Parent Storage Widget (self)
  │   │   │   │               ↓
  │   │   │   │               Is Valid? (Parent Storage Widget do destino)
  │   │   │   │               ├─ TRUE: Destino = STORAGE
  │   │   │   │               │   │
  │   │   │   │               │   └─ MOVER DO INVENTÁRIO PARA O STORAGE:
  │   │   │   │               │       Get Parent Storage Widget (self)
  │   │   │   │               │       ↓
  │   │   │   │               │       Is Valid? (WBP_Storage)
  │   │   │   │               │       ├─ TRUE:
  │   │   │   │               │       │   ├─ Get Slot Index (self)
  │   │   │   │               │       │   │     └─ Return Value: Slot Index (0-99 do storage)
  │   │   │   │               │       │   │
  │   │   │   │               │       │   ├─ Move Item To Storage
  │   │   │   │               │       │   │     └─ Target: WBP_Storage (do Get Parent Storage Widget)
  │   │   │   │               │       │   │     └─ Inventory Item ID: Inventory ID (do Source)
  │   │   │   │               │       │   │     └─ Target Slot Index: Slot Index (do self, 0-99)
  │   │   │   │               │       │   │
  │   │   │   │               │       │   └─ Branch (Return Value)
  │   │   │   │               │       │       ├─ TRUE: Return Handled
  │   │   │   │               │       │       └─ FALSE: Return Unhandled
  │   │   │   │               │       │
  │   │   │   │               │       └─ FALSE:
  │   │   │   │               │           Return Unhandled
  │   │   │   │               │
  │   │   │   │               └─ FALSE: Destino = INVENTÁRIO
  │   │   │   │                   │
  │   │   │   │                   └─ MOVER DENTRO DO INVENTÁRIO (já funciona):
  │   │   │   │                       Process Item Drop (Source Slot Widget)
  │   │   │   │                       └─ Return Handled
  │   │   │   │
  │   │   │   └─ FALSE:
  │   │   │       Return Unhandled
  │   │   │
  │   │   └─ FALSE:
  │   │       Return Unhandled
  │   │
  │   └─ FALSE:
  │       Return Unhandled
  │
  └─ CastFailed:
      Return Unhandled
```

---

## ⚠️ **PROBLEMA: Como obter `storage_id` para `MoveItemFromStorage`?**

A função `MoveItemFromStorage` precisa do `storage_id` (não do `inventory_id`). Precisamos buscar o `storage_id` no `CurrentStorage` do `GameInstance` usando o `inventory_id`.

### **SOLUÇÃO: Criar Função Helper no GameInstance**

Vou criar uma função C++ para obter o `storage_id` a partir do `inventory_id`.

---

## 🔧 **PARTE 3: Implementação Detalhada - Passo a Passo**

### **CENÁRIO 1: Inventário → Storage**

**Quando:**
- Origem: `ParentStorageWidget` da origem é `None` (inventário)
- Destino: `ParentStorageWidget` do destino é válido (storage)

**Ação:**
1. Obter `InventoryID` do slot de origem
2. Obter `SlotIndex` do slot de destino (0-99 do storage)
3. Chamar `MoveItemToStorage` no `WBP_Storage`

**Implementação no Blueprint:**

```
Get Parent Storage Widget (self)  ← Destino
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Get Slot Index (self)
  │   │     └─ Return Value: Slot Index (0-99)
  │   │
  │   ├─ Move Item To Storage
  │   │     └─ Target: WBP_Storage (do Get Parent Storage Widget)
  │   │     └─ Inventory Item ID: Inventory ID (do Source)
  │   │     └─ Target Slot Index: Slot Index (do self, 0-99)
  │   │
  │   └─ Branch (Return Value)
  │       ├─ TRUE: Return Handled
  │       └─ FALSE: Return Unhandled
  │
  └─ FALSE:
      Return Unhandled
```

### **CENÁRIO 2: Storage → Inventário**

**Quando:**
- Origem: `ParentStorageWidget` da origem é válido (storage)
- Destino: `ParentStorageWidget` do destino é `None` (inventário)

**Ação:**
1. Obter `InventoryID` do slot de origem
2. Buscar `storage_id` no `CurrentStorage` do `GameInstance` usando o `inventory_id`
3. Obter `SlotIndex` do slot de destino (0-49 do inventário)
4. Chamar `MoveItemFromStorage` no `WBP_Storage`

**Implementação no Blueprint:**

```
Get Parent Storage Widget (Source Slot Widget)  ← Origem
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Get Game Instance
  │   ├─ Cast to Umbra Game Instance
  │   ├─ Get Storage Slot By Inventory ID (Inventory ID do Source)
  │   │     └─ Return Value: Storage ID
  │   │
  │   ├─ Get Slot Index (self)
  │   │     └─ Return Value: Slot Index (0-49)
  │   │
  │   ├─ Move Item From Storage
  │   │     └─ Target: WBP_Storage (do Get Parent Storage Widget)
  │   │     └─ Storage Item ID: Storage ID (obtido acima)
  │   │     └─ Target Slot Index: Slot Index (do self, 0-49)
  │   │
  │   └─ Branch (Return Value)
  │       ├─ TRUE: Return Handled
  │       └─ FALSE: Return Unhandled
  │
  └─ FALSE:
      Return Unhandled
```

---

## 🔧 **PARTE 4: Criar Função Helper no C++**

Precisamos criar uma função no `UmbraGameInstance` para obter o `storage_id` a partir do `inventory_id`.

### **Adicionar no `UmbraGameInstance.h`:**

```cpp
/**
 * Obtém o storage_id de um item no storage usando o inventory_id
 * @param InventoryID - ID do item no inventário
 * @return StorageID se encontrado, 0 caso contrário
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Storage")
int32 GetStorageIDByInventoryID(int32 InventoryID) const;
```

### **Implementar no `UmbraGameInstance.cpp`:**

```cpp
int32 UUmbraGameInstance::GetStorageIDByInventoryID(int32 InventoryID) const
{
    // Buscar no CurrentStorage
    for (const FUmbraInventorySlot& Slot : CurrentStorage)
    {
        if (Slot.InventoryID == InventoryID && Slot.InventoryID > 0)
        {
            // O storage_id está no slot_index convertido
            // Mas precisamos retornar o storage_id real da tabela player_storage
            // Por enquanto, vamos usar uma busca diferente
            // TODO: Adicionar storage_id ao FUmbraInventorySlot ou buscar na API
            UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] GetStorageIDByInventoryID - Precisa implementar busca de storage_id"));
            return 0; // Temporário
        }
    }
    return 0;
}
```

**⚠️ PROBLEMA:** O `FUmbraInventorySlot` não tem `storage_id`. Precisamos adicionar ou buscar de outra forma.

---

## 🔧 **SOLUÇÃO ALTERNATIVA: Usar SlotIndex para Identificar**

Como alternativa, podemos usar o `SlotIndex` do slot de origem (que está no storage, então é 50-149) para identificar qual item mover. Mas a API `move_from_storage.php` precisa do `storage_id`, não do `slot_index`.

**Vou verificar a API para ver se podemos usar `slot_index` em vez de `storage_id`.**

---

## 📝 **PRÓXIMOS PASSOS:**

1. **Verificar se a API `move_from_storage.php` aceita `slot_index` em vez de `storage_id`**
2. **Se não aceitar, adicionar `storage_id` ao `FUmbraInventorySlot`**
3. **Implementar a lógica completa no Blueprint**

---

**Vou criar uma versão simplificada primeiro que funcione, depois podemos melhorar.**

