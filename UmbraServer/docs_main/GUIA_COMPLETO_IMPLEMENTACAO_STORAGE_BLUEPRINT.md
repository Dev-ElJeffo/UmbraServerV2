# 📋 GUIA COMPLETO: Implementação Storage no Blueprint

## ✅ **STATUS:**

- ✅ APIs PHP criadas (`move_to_storage.php`, `move_from_storage.php`, `get_storage.php`)
- ✅ Funções C++ implementadas (`MoveItemToStorage`, `MoveItemFromStorage`)
- ✅ Tabela `player_storage` criada no banco de dados
- ✅ `WBP_Storage` criado com 100 slots

---

## 🎯 **OBJETIVO:**

Implementar todas as funções Blueprint necessárias para o sistema de storage funcionar completamente.

---

## 📋 **PARTE 1: Implementar LoadStorage no Blueprint**

### **✅ MÉTODO RECOMENDADO (SIMPLES):**

**Use a função C++ `LoadStorage()` do `UmbraGameInstance`!**

Veja o guia: **`GUIA_SIMPLES_LOADSTORAGE_BLUEPRINT.md`**

É muito mais simples - você só precisa:
1. Chamar `Load Storage` do `UmbraGameInstance`
2. Conectar ao delegate `On Storage Loaded`
3. Usar `Get All Storage Slots` para atualizar a UI

**Tudo automático, sem requisições manuais!**

---

### **⚠️ MÉTODO ANTIGO (COMPLEXO - NÃO RECOMENDADO):**

Se você realmente quiser fazer manualmente (não recomendado):

### **PASSO 1: Criar Função LoadStorageBlueprint**

**No `WBP_Storage` → Functions → Add Function:**

**Nome:** `LoadStorageBlueprint`

**Implementação:**

```
LoadStorageBlueprint (Function)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then:
  ├─ Is Valid? (Umbra Game Instance)
  │   ├─ TRUE:
  │   │   ├─ Get Current Token (Umbra Game Instance)
  │   │   ├─ Get Va Rest Subsystem
  │   │   ├─ Construct Va Rest Request Ext
  │   │   │     └─ Verb: POST
  │   │   │     └─ Content Type: JSON
  │   │   ├─ Make Literal String
  │   │   │     └─ Value: "http://localhost/umbra_api/api/storage/get_storage.php"
  │   │   ├─ Set URL (VaRest Request)
  │   │   │     └─ URL: String acima
  │   │   ├─ Construct Va Rest Json Object
  │   │   ├─ Set String Field
  │   │   │     └─ Field Name: "token"
  │   │   │     └─ String Value: Current Token
  │   │   ├─ Set Request Object (VaRest Request)
  │   │   │     └─ Request Object: JSON Object acima
  │   │   ├─ Bind Event to OnRequestComplete
  │   │   │     └─ Custom Event: OnStorageLoadComplete
  │   │   ├─ Bind Event to OnRequestFail
  │   │   │     └─ Custom Event: OnStorageLoadFail
  │   │   └─ Execute Process Request
  │   │
  │   └─ FALSE:
  │       Print String "GameInstance inválido"
```

---

### **PASSO 2: Criar Custom Event OnStorageLoadComplete**

**No `WBP_Storage` → Event Graph → Add Custom Event:**

**Nome:** `OnStorageLoadComplete`

**Input:** `Request` (VaRest Request JSON)

**Implementação:**

```
OnStorageLoadComplete (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Get Response Object (Request)
  ↓
Get Bool Field ("success")
  ↓
Branch (success)
  ├─ TRUE:
  │   ├─ Get Array Field ("storage")
  │   │     └─ Field Name: "storage"
  │   ├─ Get Array Length (storage array)
  │   ├─ Print String (Format Text: "Storage carregado: {0} itens", Array Length)
  │   │
  │   └─ PROCESSAR CADA ITEM:
  │       ForEachLoop (storage array)
  │         Loop Body:
  │           ├─ Get Array Element
  │           ├─ Get Object Field
  │           │     └─ Field Name: "storage_id"
  │           │     └─ Return Value: storage_id (Integer)
  │           ├─ Get Object Field
  │           │     └─ Field Name: "inventory_id"
  │           │     └─ Return Value: inventory_id (Integer)
  │           ├─ Get Object Field
  │           │     └─ Field Name: "slot_index"
  │           │     └─ Return Value: slot_index (Integer)
  │           ├─ Subtract (slot_index - 50)  ← Converte 50-149 para 0-99
  │           ├─ Get Object Field
  │           │     └─ Field Name: "item_template_id"
  │           │     └─ Return Value: item_template_id (Integer)
  │           ├─ Get Object Field
  │           │     └─ Field Name: "quantity"
  │           │     └─ Return Value: quantity (Integer)
  │           ├─ Get Object Field
  │           │     └─ Field Name: "durability"
  │           │     └─ Return Value: durability (Float)
  │           ├─ Get Object Field
  │           │     └─ Field Name: "is_equipped"
  │           │     └─ Return Value: is_equipped (Boolean)
  │           │
  │           └─ CRIAR STRUCT FUmbraInventorySlot:
  │               Make Umbra Inventory Slot
  │                 ├─ Inventory ID: inventory_id
  │                 ├─ Slot Index: Resultado do Subtract (0-99)
  │                 ├─ Item Template ID: item_template_id
  │                 ├─ Quantity: quantity
  │                 ├─ Durability: durability
  │                 ├─ Is Equipped: is_equipped
  │                 └─ (outros campos: valores padrão)
  │               │
  │               └─ ATUALIZAR SLOT NO ARRAY:
  │                   Get Array Item (StorageSlots, Index: Resultado do Subtract)
  │                   ↓
  │                   Set Slot Data (do Array Item)
  │                     └─ New Slot Data: Struct criado acima
  │                   ↓
  │                   Update Slot Visual (do Array Item)
  │
  │       ForEachLoop Completed
  │         ↓
  │       On Storage Loaded (self)
  │         └─ Used Slots: Get Used Slots (self)
  │         └─ Total Slots: Get Max Capacity (self)
  │
  └─ FALSE:
      Get String Field ("message")
      Print String (message)
```

---

### **PASSO 3: Criar Custom Event OnStorageLoadFail**

**No `WBP_Storage` → Event Graph → Add Custom Event:**

**Nome:** `OnStorageLoadFail`

**Input:** `Request` (VaRest Request JSON)

**Implementação:**

```
OnStorageLoadFail (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Print String "Falha ao carregar storage"
```

---

### **PASSO 4: Modificar Event Construct**

**No `WBP_Storage` → Event Graph → Event Construct:**

**Adicione após `CreateStorageSlots`:**

```
Event Construct
  ↓
Create Storage Slots
  ↓
Load Storage Blueprint  ← Adicionar esta chamada
```

---

## 📋 **PARTE 2: Usar Funções C++ MoveItemToStorage e MoveItemFromStorage**

### **OBJETIVO:**

As funções C++ já estão implementadas e prontas para uso. Você só precisa chamá-las do Blueprint.

---

### **COMO USAR MoveItemToStorage:**

**No Blueprint (ex: `OnDrop` do `WBP_InventorySlot`):**

```
Move Item To Storage (self - WBP_Storage)
  ├─ Inventory Item ID: inventory_id (do item arrastado)
  └─ Target Slot Index: slot_index (0-99 do storage)
  ↓
Branch (Return Value)
  ├─ TRUE:
  │   ├─ Load Storage Blueprint (self - WBP_Storage)
  │   ├─ Get Game Instance
  │   ├─ Cast to Umbra Game Instance
  │   └─ Load Inventory
  │
  └─ FALSE:
      Print String "Falha ao mover item para storage"
```

---

### **COMO USAR MoveItemFromStorage:**

**No Blueprint (ex: `OnDrop` do `WBP_InventorySlot`):**

```
Move Item From Storage (self - WBP_Storage)
  ├─ Storage Item ID: storage_id (do item no storage)
  └─ Target Slot Index: slot_index (0-49 do inventário)
  ↓
Branch (Return Value)
  ├─ TRUE:
  │   ├─ Load Storage Blueprint (self - WBP_Storage)
  │   ├─ Get Game Instance
  │   ├─ Cast to Umbra Game Instance
  │   └─ Load Inventory
  │
  └─ FALSE:
      Print String "Falha ao mover item do storage"
```

---

## 📋 **PARTE 3: Implementar OnDrop no WBP_InventorySlot**

### **OBJETIVO:**

Modificar o `OnDrop` existente para suportar drag and drop entre inventário e storage.

---

### **ESTRUTURA COMPLETA DO OnDrop:**

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
      │   ├─ Move Item From Storage (WBP_Storage)
      │   │     └─ Storage Item ID: InventoryID (do Source)
      │   │     └─ Target Slot Index: SlotIndex (do self, 0-49)
      │   ├─ Branch (Return Value)
      │   │   ├─ TRUE:
      │   │   │   ├─ Load Storage Blueprint (WBP_Storage)
      │   │   │   ├─ Get Game Instance
      │   │   │   ├─ Cast to Umbra Game Instance
      │   │   │   └─ Load Inventory
      │   │   │   └─ Return Handled
      │   │   │
      │   │   └─ FALSE:
      │   │       Print String "Falha ao mover item do storage"
      │   │       └─ Return Unhandled
      │   │
      │   └─ FALSE:
      │       Return Unhandled
```

---

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
  │   │   ├─ Move Item To Storage (WBP_Storage)
  │   │   │     └─ Inventory Item ID: InventoryID (do Source)
  │   │   │     └─ Target Slot Index: Resultado do Subtract (0-99)
  │   │   ├─ Branch (Return Value)
  │   │   │   ├─ TRUE:
  │   │   │   │   ├─ Load Storage Blueprint (WBP_Storage)
  │   │   │   │   ├─ Get Game Instance
  │   │   │   │   ├─ Cast to Umbra Game Instance
  │   │   │   │   └─ Load Inventory
  │   │   │   │   └─ Return Handled
  │   │   │   │
  │   │   │   └─ FALSE:
  │   │   │       Print String "Falha ao mover item para storage"
  │   │   │       └─ Return Unhandled
  │   │   │
  │   │   └─ FALSE:
  │   │       Return Unhandled
  │   │
  │   └─ FALSE: Destino = INVENTÁRIO
  │       └─ (Lógica existente - manter como está)
  │       └─ Process Item Drop (Source Slot Widget)
  │       └─ Return Handled
```

---

## 📋 **PARTE 4: Variáveis Necessárias no WBP_Storage**

### **VARIÁVEIS:**

1. **`MyGameInstance`** (Object Reference → Umbra Game Instance)
   - Inicializar no `Event Construct`:
     ```
     Get Game Instance
       ↓
     Cast to Umbra Game Instance
       ↓
     Set MyGameInstance
     ```

2. **`StorageSlots`** (Array of WBP Inventory Slot)
   - Criado automaticamente pelo `CreateStorageSlots`

3. **`UniformGridPanel_Storage`** (Object Reference → Uniform Grid Panel)
   - Vincular ao `Grid_StorageSlots` no Designer

---

## 📋 **PARTE 5: Função UpdateAllSlotsVisual**

### **OBJETIVO:**

Atualizar visualmente todos os slots do storage após carregar.

---

### **IMPLEMENTAÇÃO:**

```
UpdateAllSlotsVisual (Function)
  ↓
ForEachLoop (StorageSlots array)
  Loop Body:
    ├─ Get Array Element
    ├─ Cast to WBP Inventory Slot
    │     └─ Object: Array Element
    │
    └─ then (Cast successful):
        ├─ Get Slot Data (do Cast)
        ├─ Break Umbra Inventory Slot
        │     └─ Inventory ID
        ├─ Branch (Inventory ID > 0)
        │   ├─ TRUE:
        │   │   ├─ Set Slot Data (do Cast)
        │   │   │     └─ New Slot Data: Slot Data (do Get)
        │   │   └─ Update Slot Visual (do Cast)
        │   │
        │   └─ FALSE:
        │       ├─ Clear Slot (do Cast)
        │       └─ Update Slot Visual (do Cast)
        │
  ForEachLoop Completed
```

---

## 📋 **PARTE 6: Checklist de Implementação**

### **WBP_Storage:**

- [ ] Variável `MyGameInstance` criada e inicializada
- [ ] Variável `UniformGridPanel_Storage` vinculada ao Designer
- [ ] Função `CreateStorageSlots` implementada (100 slots, índices 50-149)
- [ ] Função `LoadStorageBlueprint` implementada
- [ ] Custom Event `OnStorageLoadComplete` implementado
- [ ] Custom Event `OnStorageLoadFail` implementado
- [ ] Função `UpdateAllSlotsVisual` implementada
- [ ] `Event Construct` chama `CreateStorageSlots` e `LoadStorageBlueprint`

### **WBP_InventorySlot:**

- [ ] Variável `ParentStorageWidget` criada
- [ ] Função `OnDrop` modificada para suportar storage
- [ ] Verificação de origem (inventário vs storage) implementada
- [ ] Verificação de destino (inventário vs storage) implementada
- [ ] Chamadas a `MoveItemToStorage` e `MoveItemFromStorage` implementadas

### **BP_StorageChest (Actor):**

- [ ] Collision Box configurado
- [ ] Botão de interação configurado
- [ ] Abre `WBP_Inventory` e `WBP_Storage` simultaneamente

---

## 📋 **PARTE 7: Teste Completo**

### **TESTE 1: Carregar Storage**

1. Abra o jogo
2. Interaja com o baú de storage
3. **Deve:** Storage abrir com itens carregados (se houver)

### **TESTE 2: Mover Item do Inventário para Storage**

1. Arraste um item do inventário para um slot do storage
2. **Deve:** Item sair do inventário e aparecer no storage
3. **Deve:** Ambos os widgets serem atualizados

### **TESTE 3: Mover Item do Storage para Inventário**

1. Arraste um item do storage para um slot do inventário
2. **Deve:** Item sair do storage e aparecer no inventário
3. **Deve:** Ambos os widgets serem atualizados

### **TESTE 4: Mover Item Dentro do Inventário**

1. Arraste um item dentro do inventário
2. **Deve:** Funcionar como antes (lógica existente)

---

## ⚠️ **IMPORTANTE:**

1. **Conversão de Índices:**
   - Storage usa índices 0-99 no Blueprint
   - Banco de dados usa índices 50-149
   - Sempre use `Subtract (SlotIndex - 50)` ao converter

2. **Recarregar Após Mover:**
   - Sempre chame `LoadStorageBlueprint` e `LoadInventory` após mover
   - Isso garante que ambos os widgets sejam atualizados

3. **ParentStorageWidget:**
   - Deve ser definido no `CreateStorageSlots` do `WBP_Storage`
   - Use `self` (WBP_Storage) como valor

4. **Validações:**
   - Sempre verifique `Is Valid?` antes de usar widgets
   - Sempre verifique `Return Value` das funções C++

---

## 📝 **RESUMO:**

1. ✅ Implementar `LoadStorageBlueprint` (chama API `get_storage.php`)
2. ✅ Implementar `OnStorageLoadComplete` (processa resposta e atualiza slots)
3. ✅ Modificar `OnDrop` no `WBP_InventorySlot` (suporta storage)
4. ✅ Usar funções C++ `MoveItemToStorage` e `MoveItemFromStorage`
5. ✅ Sempre recarregar ambos os widgets após mover

---

## 🔧 **PRÓXIMOS PASSOS:**

Após implementar tudo acima:

1. Teste cada funcionalidade individualmente
2. Verifique logs para erros
3. Ajuste visual se necessário
4. Implemente feedback visual (animações, sons, etc.)

