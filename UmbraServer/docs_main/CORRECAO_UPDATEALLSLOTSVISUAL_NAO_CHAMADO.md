# 🔴 CORREÇÃO CRÍTICA: UpdateAllSlotsVisual Não Está Sendo Chamado

## 🎯 **PROBLEMA IDENTIFICADO:**

1. **Storage → Storage:** Quando movemos um item dentro do storage, o `OnItemMoved` do `GameInstance` é disparado, mas o `WBP_Storage` **NÃO está escutando esse delegate**.

2. **UpdateAllSlotsVisual:** O `OnItemMoved()` do `WBP_Storage` só é chamado quando `MoveItemToStorage` ou `MoveItemFromStorage` são bem-sucedidos, mas **NÃO** quando `MoveItem` (do GameInstance) é usado para Storage → Storage.

3. **Erro PHP:** A mensagem de erro do PHP está correta, mas pode estar confundindo. O código aceita 0-149, mas a mensagem menciona "0-49 para inventário ou 50-149 para storage".

---

## ✅ **SOLUÇÃO:**

### **PARTE 1: Conectar o Delegate `OnItemMoved` do GameInstance no WBP_Storage**

O `WBP_Storage` precisa escutar o delegate `OnItemMoved` do `GameInstance` para ser notificado quando um item é movido (incluindo Storage → Storage).

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: No Event Construct do WBP_Storage**

**Adicione a conexão do delegate `OnItemMoved`:**

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
  │     └─ Event:    (Custom Event)
  └─ Load Storage
```

**Como fazer:**
1. No `Event Construct` do `WBP_Storage`
2. Após `Cast to Umbra Game Instance`
3. Arraste o `As Umbra Game Instance` → **Event** → **Assign On Item Moved**
4. Crie um **Custom Event** chamado `OnItemMovedFromGameInstance_Event`
5. Conecte o `Event` pin do `Assign On Item Moved` ao `OnItemMovedFromGameInstance_Event`

---

### **PASSO 2: Criar Custom Event `OnItemMovedFromGameInstance_Event`**

**No `WBP_Storage` → Event Graph:**

```
Custom Event: OnItemMovedFromGameInstance_Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Print String: "Item movido! From: [FromSlot], To: [ToSlot]"  ← DEBUG (opcional)
  ↓
Branch (FromSlot >= 50 OR ToSlot >= 50)  ← Verifica se é movimento do storage
  ├─ TRUE:  ← Movimento envolve storage
  │   ├─ Print String: "Movimento do storage detectado!"  ← DEBUG
  │   ├─ Get Game Instance
  │   │   └─ Cast to Umbra Game Instance
  │   │       └─ then:
  │   │           └─ Load Storage  ← RECARREGAR DADOS DO STORAGE (GameInstance)
  │   ├─ Delay (0.1)  ← Pequeno delay para garantir que LoadStorage complete
  │   │   └─ Update All Slots Visual  ← ATUALIZAR VISUAL DO STORAGE
  │   │
  │   └─ (Alternativa sem delay: usar delegate OnStorageLoaded)
  │
  └─ FALSE:  ← Movimento apenas no inventário
      (nada - não afeta o storage)
```

**IMPORTANTE:** 
- O `LoadStorage` do `GameInstance` já atualiza o `CurrentStorage` automaticamente
- Após o `LoadStorage` completar, o delegate `OnStorageLoaded` é disparado
- Você pode escutar o `OnStorageLoaded` e chamar `UpdateAllSlotsVisual` lá, ou usar um pequeno delay

**Como fazer:**
1. **Criar Custom Event:**
   - Clique com botão direito → **Add Custom Event**
   - Nome: `OnItemMovedFromGameInstance_Event`
   - Adicione 3 parâmetros:
     - `InventoryID` (Integer)
     - `FromSlot` (Integer)
     - `ToSlot` (Integer)

2. **Adicionar Branch:**
   - Adicione `Branch` node
   - Adicione `OR` node
   - Adicione `Greater or Equal (Int Int)` node (2x)
   - Conecte:
     - `FromSlot` → `A` do primeiro `Greater or Equal`
     - `ToSlot` → `A` do segundo `Greater or Equal`
     - `50` → `B` de ambos os `Greater or Equal`
     - Saídas dos `Greater or Equal` → `A` e `B` do `OR`
     - Saída do `OR` → `Condition` do `Branch`

3. **No caminho TRUE do Branch:**
   - Adicione `Get Game Instance`
   - Adicione `Cast to Umbra Game Instance`
   - Conecte `Return Value` de `Get Game Instance` ao `Object` pin do `Cast`
   - No `then` do `Cast`:
     - Adicione `Load Storage` (função C++ do `GameInstance`, não do `WBP_Storage`)
   - Adicione `Delay` (0.1 segundos)
   - No `Completed` do `Delay`:
     - Adicione `Update All Slots Visual` (função do `WBP_Storage`)

**ALTERNATIVA (Recomendada):** Em vez de usar `Delay`, escute o delegate `OnStorageLoaded` do `GameInstance` e chame `UpdateAllSlotsVisual` lá. Isso garante que a atualização aconteça apenas após o storage ser realmente carregado.

---

### **PASSO 2.5: Alternativa Recomendada - Usar Delegate OnStorageLoaded**

**No `WBP_Storage` → Event Construct:**

Além de conectar `OnItemMoved`, também conecte `OnStorageLoaded`:

```
Event Construct
  ↓
... (código existente) ...
  ├─ Assign On Storage Loaded
  │     └─ Event: OnStorageLoaded_Event (Custom Event)
  └─ Assign On Item Moved
        └─ Event: OnItemMovedFromGameInstance_Event (Custom Event)
```

**No `OnItemMovedFromGameInstance_Event`:**

```
Custom Event: OnItemMovedFromGameInstance_Event
  ↓
Branch (FromSlot >= 50 OR ToSlot >= 50)
  ├─ TRUE:
  │   └─ Get Game Instance → Cast to Umbra Game Instance → Load Storage
  │       (O UpdateAllSlotsVisual será chamado no OnStorageLoaded_Event)
  │
  └─ FALSE: (nada)
```

**No `OnStorageLoaded_Event` (Custom Event já existente):**

```
Custom Event: OnStorageLoaded_Event
  ↓
Update All Slots Visual  ← ATUALIZAR VISUAL APÓS CARREGAR
```

**Vantagens desta abordagem:**
- ✅ Não precisa de `Delay`
- ✅ Garante que `UpdateAllSlotsVisual` só é chamado após o storage ser realmente carregado
- ✅ Funciona tanto para `LoadStorage` manual quanto para `LoadStorage` após movimento

---

### **PASSO 3: Verificar se `OnItemMoved()` (BlueprintImplementableEvent) está implementado**

**No `WBP_Storage` → Functions → Override → OnItemMoved:**

```
Event: On Item Moved (BlueprintImplementableEvent)
  ↓
Print String: "OnItemMoved() chamado!"  ← DEBUG
  ↓
Update All Slots Visual  ← ATUALIZAR VISUAL
  ↓
Load Storage  ← RECARREGAR DADOS
```

**Como fazer:**
1. Vá para **Functions** → **Override** → **On Item Moved**
2. Se não aparecer, compile o C++ primeiro
3. Adicione `Update All Slots Visual`
4. Adicione `Load Storage`

**IMPORTANTE:** Este evento é chamado quando `MoveItemToStorage` ou `MoveItemFromStorage` são bem-sucedidos. O `OnItemMovedFromGameInstance_Event` é chamado quando `MoveItem` (do GameInstance) é usado (incluindo Storage → Storage).

---

### **PASSO 4: Verificar a função `UpdateAllSlotsVisual`**

**No `WBP_Storage` → Functions → UpdateAllSlotsVisual:**

```
Function: Update All Slots Visual
  ↓
Print String: "UpdateAllSlotsVisual chamado!"  ← DEBUG
  ↓
Get Storage Data (self)  ← Obtém array de slots do storage
  ↓
ForEachLoop (Storage Data)
  Loop Body:
    ├─ Break Umbra Inventory Slot
    │     └─ UmbraInventorySlot: Array Element
    ├─ Get Slot Index (do Break)
    ├─ Subtract (Slot Index - 50)  ← Converter de 50-149 para 0-99
    ├─ Get Array Item (StorageSlotWidgets, Index: resultado do Subtract)
    ├─ Is Valid? (do Array Item)
    │   ├─ TRUE:
    │   │   ├─ Cast to WBP Inventory Slot
    │   │   │     └─ Object: Output (Get Array Item)
    │   │   ├─ then (Cast bem-sucedido):
    │   │   │   ├─ Set Slot Data
    │   │   │   │     └─ New Slot Data: Array Element (do ForEachLoop)
    │   │   │   └─ Update Slot Visual
    │   │   │
    │   │   └─ CastFailed: (nada)
    │   │
    │   └─ FALSE: (nada)
```

**IMPORTANTE:** 
- Use `StorageSlotWidgets` (Array de `WBP_InventorySlot`), não `SlotWidgets`
- O `Slot Index` do storage no banco é 50-149, mas no array `StorageSlotWidgets` é 0-99
- Por isso, subtraia 50 do `Slot Index` antes de usar no `Get Array Item`

---

## 🔍 **DEBUG:**

### **1. Verificar se `OnItemMovedFromGameInstance_Event` está sendo chamado:**

Adicione `Print String` no início do evento. Deve aparecer no log quando mover um item dentro do storage.

### **2. Verificar se `UpdateAllSlotsVisual` está sendo chamado:**

Adicione `Print String` no início da função. Deve aparecer no log quando `OnItemMovedFromGameInstance_Event` for chamado e o movimento envolver storage.

### **3. Verificar se `LoadStorage` está sendo chamado:**

O `LoadStorage` já tem logs no C++. Verifique se aparecem no log quando um item é movido.

### **4. Verificar índices:**

Adicione logs para verificar:
- `FromSlot` e `ToSlot` recebidos no `OnItemMovedFromGameInstance_Event`
- Se o `Branch` está detectando corretamente movimentos do storage (FromSlot >= 50 OR ToSlot >= 50)

---

## 📝 **RESUMO:**

1. ✅ **Conectar delegate `OnItemMoved` do GameInstance no `WBP_Storage`**
2. ✅ **Criar Custom Event `OnItemMovedFromGameInstance_Event`**
3. ✅ **Verificar se movimento envolve storage (FromSlot >= 50 OR ToSlot >= 50)**
4. ✅ **Chamar `UpdateAllSlotsVisual` e `LoadStorage` se for movimento do storage**
5. ✅ **Verificar se `OnItemMoved()` (BlueprintImplementableEvent) está implementado**
6. ✅ **Verificar se `UpdateAllSlotsVisual` está usando `StorageSlotWidgets` e convertendo índices corretamente**

---

## ⚠️ **NOTA IMPORTANTE:**

O `OnItemMoved()` (BlueprintImplementableEvent) é chamado quando:
- `MoveItemToStorage` é bem-sucedido
- `MoveItemFromStorage` é bem-sucedido

O `OnItemMovedFromGameInstance_Event` (Custom Event conectado ao delegate) é chamado quando:
- `MoveItem` (do GameInstance) é bem-sucedido (incluindo Storage → Storage)

**Ambos devem chamar `UpdateAllSlotsVisual` e `LoadStorage` para garantir que a UI seja atualizada corretamente.**

---

## 📝 **CHECKLIST FINAL:**

Antes de testar, verifique:

- [ ] `OnItemMoved` delegate do `GameInstance` está conectado no `Event Construct` do `WBP_Storage`
- [ ] `OnItemMovedFromGameInstance_Event` (Custom Event) está criado e implementado
- [ ] `OnItemMovedFromGameInstance_Event` verifica se o movimento envolve storage (FromSlot >= 50 OR ToSlot >= 50)
- [ ] `OnItemMovedFromGameInstance_Event` chama `LoadStorage` do `GameInstance` quando movimento envolve storage
- [ ] `OnStorageLoaded_Event` chama `UpdateAllSlotsVisual` (alternativa recomendada)
- [ ] OU `OnItemMovedFromGameInstance_Event` chama `UpdateAllSlotsVisual` após `Delay` (alternativa simples)
- [ ] `OnItemMoved()` (BlueprintImplementableEvent) está implementado e chama `UpdateAllSlotsVisual`
- [ ] `UpdateAllSlotsVisual` está usando `StorageSlotWidgets` (não `SlotWidgets`)
- [ ] `UpdateAllSlotsVisual` está convertendo índices corretamente (Slot Index - 50)

---

## 🐛 **TROUBLESHOOTING:**

### **Problema: `UpdateAllSlotsVisual` ainda não é chamado**

1. **Verifique se o delegate está conectado:**
   - Adicione `Print String` no `OnItemMovedFromGameInstance_Event`
   - Deve aparecer no log quando mover um item dentro do storage

2. **Verifique se o Branch está funcionando:**
   - Adicione `Print String` no caminho TRUE do Branch
   - Deve aparecer quando `FromSlot >= 50 OR ToSlot >= 50`

3. **Verifique se `LoadStorage` está sendo chamado:**
   - Verifique os logs do C++ (`[UmbraGameInstance] LoadStorage`)
   - Deve aparecer quando mover um item dentro do storage

### **Problema: Ícones não aparecem após mover**

1. **Verifique se `UpdateAllSlotsVisual` está usando o array correto:**
   - Deve usar `StorageSlotWidgets`, não `SlotWidgets`
   - Deve converter índices (Slot Index - 50)

2. **Verifique se `SetSlotData` está sendo chamado:**
   - Adicione logs na função `UpdateAllSlotsVisual`
   - Deve chamar `SetSlotData` para cada slot

3. **Verifique se `UpdateSlotVisual` está sendo chamado:**
   - Adicione logs na função `UpdateSlotVisual` do `WBP_InventorySlot`
   - Deve ser chamado após `SetSlotData`

