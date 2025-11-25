# 🎯 GUIA COMPLETO: Drag & Drop Inventário ↔ Storage

## 📑 **ÍNDICE:**

1. [Análise do Código C++](#análise-do-código-c)
2. [Estratégia de Implementação](#estratégia-de-implementação)
3. [PARTE 1: WBP_Storage - Configuração Inicial](#parte-1-wbp_storage---configuração-inicial)
4. [PARTE 2: WBP_InventorySlot - OnDrop para Storage](#parte-2-wbp_inventoryslot---ondrop-para-storage)
5. [PARTE 3: Como Obter Referência ao WBP_Storage](#parte-3-como-obter-referência-ao-wbp_storage-no-ondrop)
6. [PARTE 4: Implementação Detalhada do OnDrop](#parte-4-implementação-detalhada-do-ondrop)
7. [PARTE 5: Implementar MoveItemToStorage e MoveItemFromStorage](#parte-5-implementar-moveitemtostorage-e-moveitemfromstorage)
8. [PARTE 6: Exemplo Completo de OnDrop](#parte-6-exemplo-completo-de-ondrop-no-wbp_inventoryslot)
9. [PARTE 7: Resumo e Checklist](#parte-7-resumo-e-checklist)

---

## 📚 **ANÁLISE DO CÓDIGO C++:**

### **FUNÇÕES DISPONÍVEIS NO `UUmbraStorageWidget`:**

1. **`LoadStorage()`** - Carrega o armazém do servidor
2. **`SetStorageData(TArray<FUmbraInventorySlot>)`** - Define os dados do armazém
3. **`GetStorageData()`** - Obtém os dados do armazém
4. **`GetUsedSlots()`** - Retorna quantos slots estão ocupados
5. **`GetFreeSlots()`** - Retorna quantos slots estão livres
6. **`IsFull()`** - Verifica se o armazém está cheio
7. **`MoveItemToStorage(int32 InventoryItemID, int32 TargetSlotIndex)`** - Move item do inventário para o armazém
8. **`MoveItemFromStorage(int32 StorageItemID, int32 TargetSlotIndex)`** - Move item do armazém para o inventário
9. **`DepositAll()`** - Deposita todos os itens possíveis
10. **`WithdrawAll()`** - Retira todos os itens possíveis
11. **`ClearStorage()`** - Limpa o armazém

### **EVENTOS BLUEPRINT (BlueprintImplementableEvent):**

1. **`OnStorageLoaded(int32 UsedSlots, int32 TotalSlots)`** - Quando o armazém é carregado
2. **`OnItemDeposited(FUmbraInventorySlot ItemData)`** - Quando um item é depositado
3. **`OnItemWithdrawn(FUmbraInventorySlot ItemData)`** - Quando um item é retirado
4. **`OnStorageFull()`** - Quando o armazém fica cheio

### **PROPRIEDADES IMPORTANTES:**

- **`StorageSlots`** - Array de `FUmbraInventorySlot` (100 slots, índices 50-149)
- **`MaxStorageSlots`** - Capacidade máxima (padrão: 100)
- **`StorageID`** - ID único do armazém
- **`bIsLoaded`** - Flag se o armazém está carregado

### **PROPRIEDADE NO `UUmbraInventorySlotWidget`:**

- **`ParentStorageWidget`** - Referência ao `WBP_Storage` se o slot pertence ao armazém (já implementado)

---

## 🎯 **ESTRATÉGIA DE IMPLEMENTAÇÃO:**

### **CONCEITO CHAVE:**

- **Slots do Inventário:** Índices 0-49
- **Slots do Storage:** Índices 50-149
- **Identificação de Origem:** Usar `ParentStorageWidget` para determinar se o slot é do inventário ou do storage

### **DIAGRAMA DA ESTRUTURA:**

```
WBP_Inventory (50 slots)
├─ SlotWidgets[0-49] (Array de WBP_InventorySlot)
│  ├─ WBP_InventorySlot[0]
│  │  └─ ParentStorageWidget = None  ← Pertence ao inventário
│  ├─ WBP_InventorySlot[1]
│  │  └─ ParentStorageWidget = None
│  └─ ... (até slot 49)
│
└─ Event Construct
   └─ Load Inventory
   └─ OnInventoryLoaded_Event
      └─ CreateInventorySlots
      └─ UpdateAllSlotsVisual

WBP_Storage (100 slots)
├─ SlotWidgets[0-99] (Array de WBP_InventorySlot)
│  ├─ WBP_InventorySlot[0] → SlotIndex = 50
│  │  └─ ParentStorageWidget = WBP_Storage  ← Pertence ao storage
│  ├─ WBP_InventorySlot[1] → SlotIndex = 51
│  │  └─ ParentStorageWidget = WBP_Storage
│  └─ ... (até slot 99, SlotIndex = 149)
│
└─ Event Construct
   └─ Load Storage
   └─ OnStorageLoaded_Event
      └─ CreateStorageSlots
      └─ UpdateAllSlotsVisual

WBP_InventorySlot (OnDrop)
├─ Verifica ParentStorageWidget da origem
├─ Verifica ParentStorageWidget do destino
└─ Decide ação:
   ├─ Origem = Inventário, Destino = Storage
   │  └─ MoveItemToStorage (WBP_Storage)
   │
   ├─ Origem = Storage, Destino = Inventário
   │  └─ MoveItemFromStorage (WBP_Storage)
   │
   └─ Origem = Inventário, Destino = Inventário
      └─ ProcessItemDrop (lógica existente)
```

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

---

## **PARTE 1: WBP_Storage - Configuração Inicial**

### **PASSO 1.1: Criar Variáveis no WBP_Storage**

**No `WBP_Storage` (My Blueprint → Variables):**

1. **`SlotWidgets`** (Array de `WBP_InventorySlot`)
   - Tipo: `Array` de `WBP Inventory Slot`
   - Instance Editable: `False`
   - Tooltip: "Array de 100 slots do armazém"

2. **`MyGameInstance`** (Object Reference)
   - Tipo: `Umbra Game Instance`
   - Instance Editable: `False`
   - Tooltip: "Referência ao Game Instance"

3. **`UniformGridPanel_Storage`** (Object Reference)
   - Tipo: `Uniform Grid Panel`
   - Instance Editable: `False`
   - Tooltip: "Grid panel para os slots do armazém"

---

### **PASSO 1.2: Event Construct do WBP_Storage**

**No `WBP_Storage` → Event Graph → Event Construct:**

```
Event Construct
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
Is Valid? (As Umbra Game Instance)
  ├─ TRUE:
  │   ├─ Set MyGameInstance = As Umbra Game Instance
  │   └─ Load Storage  ← Chama a função C++
  │
  └─ FALSE:
      Print String "Game Instance inválido!"
```

**Como fazer:**
1. Adicione `Event Construct`
2. Adicione `Get Game Instance`
3. Adicione `Cast to Umbra Game Instance`
4. Conecte o `Return Value` de `Get Game Instance` ao `Object` pin do `Cast`
5. Adicione `Is Valid?` e conecte `As Umbra Game Instance` ao `Object` pin
6. No caminho `TRUE`, adicione `Set MyGameInstance` e `Load Storage`
7. Conecte `As Umbra Game Instance` ao `Target` de `Set MyGameInstance`
8. Conecte `self` ao `Target` de `Load Storage`

---

### **PASSO 1.3: Função CreateStorageSlots**

**No `WBP_Storage` → Functions → CreateStorageSlots:**

```
CreateStorageSlots (Function)
  ↓
Clear Array (SlotWidgets)
  ↓
Remove All Children (UniformGridPanel_Storage)
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Create Widget (WBP_InventorySlot)
    │     └─ Owning Player: Get Owning Player
    ├─ Set Slot Index
    │     └─ New Slot Index: Index + 50  ← CRÍTICO! Storage usa índices 50-149
    ├─ Set Parent Storage Widget
    │     └─ Parent Storage Widget: self (WBP_Storage)
    ├─ Add to Array (SlotWidgets)
    └─ Add Child to Uniform Grid (UniformGridPanel_Storage)
          └─ Content: Created Widget
          └─ Column: Index % 10  ← 10 colunas
          └─ Row: Index / 10     ← 10 linhas
```

**Como fazer:**
1. Crie a função `CreateStorageSlots`
2. Adicione `Clear Array` para `SlotWidgets`
3. Adicione `Remove All Children` para `UniformGridPanel_Storage`
4. Adicione `For Loop` (First: 0, Last: 99)
5. No `Loop Body`:
   - Adicione `Create Widget` (WBP_InventorySlot)
   - Adicione `Set Slot Index` (função do C++) com `Index + 50`
   - Adicione `Set Parent Storage Widget` (função do C++) com `self`
   - Adicione `Add to Array` para `SlotWidgets`
   - Adicione `Add Child to Uniform Grid` para `UniformGridPanel_Storage`
   - Use `Index % 10` para Column e `Index / 10` para Row

---

### **PASSO 1.4: Event OnStorageLoaded**

**No `WBP_Storage` → Event Graph → Override Functions → OnStorageLoaded:**

```
OnStorageLoaded (BlueprintImplementableEvent)
  ├─ Input: UsedSlots (Integer)
  └─ Input: TotalSlots (Integer)
  ↓
Create Storage Slots  ← Cria os 100 slots
  ↓
Update All Slots Visual  ← Atualiza os slots com dados
```

**Como fazer:**
1. Vá para **Functions** → **Override** → **OnStorageLoaded**
2. Adicione `Create Storage Slots`
3. Adicione `Update All Slots Visual` (função que você criará)

---

### **PASSO 1.5: Função UpdateAllSlotsVisual**

**No `WBP_Storage` → Functions → UpdateAllSlotsVisual:**

```
UpdateAllSlotsVisual (Function)
  ↓
Get Storage Data (self)  ← Obtém array de slots do storage
  ↓
ForEachLoop (Storage Data)
  Loop Body:
    ├─ Break Umbra Inventory Slot
    │     └─ UmbraInventorySlot: Array Element
    ├─ Get Slot Index (do Break)
    ├─ Get Array Item (SlotWidgets, Index: Slot Index - 50)  ← Ajusta para índice 0-99
    ├─ Cast to WBP Inventory Slot
    │     └─ Object: Output (Get Array Item)
    ├─ then (Cast bem-sucedido):
    │   ├─ Set Slot Data
    │   │     └─ New Slot Data: Array Element (do ForEachLoop)
    │   └─ Update Slot Visual
    │
    └─ CastFailed: (nada)
```

**Como fazer:**
1. Crie a função `UpdateAllSlotsVisual`
2. Adicione `Get Storage Data` (função C++)
3. Adicione `ForEachLoop` com `Storage Data`
4. No `Loop Body`:
   - Adicione `Break Umbra Inventory Slot` com `Array Element`
   - Adicione `Get Slot Index` do `Break`
   - Adicione `Subtract` (Slot Index - 50) para ajustar para índice 0-99
   - Adicione `Get Array Item` (SlotWidgets, Index: resultado do Subtract)
   - Adicione `Cast to WBP Inventory Slot`
   - No `then` do Cast:
     - Adicione `Set Slot Data` com `Array Element`
     - Adicione `Update Slot Visual`

---

## **PARTE 2: WBP_InventorySlot - OnDrop para Storage**

### **PASSO 2.1: Modificar OnDrop no WBP_InventorySlot**

**No `WBP_InventorySlot` → Event Graph → Override Functions → OnDrop:**

**ESTRUTURA ATUAL (manter):**
```
OnDrop
  ├─ Input: MyGeometry (Geometry)
  ├─ Input: PointerEvent (Pointer Event)
  └─ Input: Operation (Drag Drop Operation)
  ↓
Cast to Umbra Item Drag Drop Operation
  └─ Object: Operation
  ↓
then (Cast bem-sucedido):
  ├─ Get Source Slot Widget
  ├─ Is Valid? (Source Slot Widget)
  │   ├─ TRUE:
  │   │   ├─ Get Slot Data (Source Slot Widget)
  │   │   ├─ Break Umbra Inventory Slot
  │   │   │     └─ Slot Index
  │   │   ├─ Get Slot Data (self)
  │   │   ├─ Break Umbra Inventory Slot (self)
  │   │   │     └─ Slot Index
  │   │   │
  │   │   └─ BRANCH: Verificar origem e destino
  │   │
  │   └─ FALSE:
  │       Return Unhandled
```

**ADICIONAR LÓGICA DE STORAGE:**

```
BRANCH: Verificar origem e destino
  ↓
Get Parent Storage Widget (Source Slot Widget)  ← Verifica origem
  ↓
Is Valid? (Parent Storage Widget da origem)
  ├─ TRUE: Origem = STORAGE
  │   └─ Get Parent Storage Widget (self)  ← Verifica destino
  │       └─ Is Valid? (Parent Storage Widget do destino)
  │           ├─ TRUE: Destino = STORAGE
  │           │   └─ (Mover dentro do storage - não implementar agora)
  │           │
  │           └─ FALSE: Destino = INVENTÁRIO
  │               └─ Move Item From Storage
  │                     └─ Storage Item ID: InventoryID (do Source)
  │                     └─ Target Slot Index: SlotIndex (do self)
  │
  └─ FALSE: Origem = INVENTÁRIO
      └─ Get Parent Storage Widget (self)  ← Verifica destino
          └─ Is Valid? (Parent Storage Widget do destino)
              ├─ TRUE: Destino = STORAGE
              │   └─ Move Item To Storage
              │         └─ Inventory Item ID: InventoryID (do Source)
              │         └─ Target Slot Index: SlotIndex (do self) - 50  ← Ajusta para índice 0-99
              │
              └─ FALSE: Destino = INVENTÁRIO
                  └─ (Mover dentro do inventário - lógica existente)
                  └─ Process Item Drop (Source Slot Widget)
```

**Como fazer:**

1. **Mantenha toda a lógica existente** até o `BRANCH`

2. **Adicione verificação de origem:**
   - Adicione `Get Parent Storage Widget` do `Source Slot Widget`
   - Adicione `Is Valid?` para verificar se a origem é storage

3. **Se origem = STORAGE:**
   - Verifique o destino (self)
   - Se destino = INVENTÁRIO:
     - Adicione `Move Item From Storage` (função C++ do `WBP_Storage`)
     - **PROBLEMA:** Como obter referência ao `WBP_Storage`?
     - **SOLUÇÃO:** Use `Get Parent Storage Widget` do `Source Slot Widget` para obter o `WBP_Storage`

4. **Se origem = INVENTÁRIO:**
   - Verifique o destino (self)
   - Se destino = STORAGE:
     - Adicione `Move Item To Storage` (função C++ do `WBP_Storage`)
     - **PROBLEMA:** Como obter referência ao `WBP_Storage`?
     - **SOLUÇÃO:** Use `Get Parent Storage Widget` do `self` para obter o `WBP_Storage`

---

## **PARTE 3: Como Obter Referência ao WBP_Storage no OnDrop**

### **PROBLEMA:**

No `OnDrop` do `WBP_InventorySlot`, você precisa chamar `MoveItemToStorage` ou `MoveItemFromStorage`, mas essas funções pertencem ao `WBP_Storage`, não ao `WBP_InventorySlot`.

### **SOLUÇÃO:**

Use `Get Parent Storage Widget` para obter a referência ao `WBP_Storage`.

### **CENÁRIOS:**

#### **CENÁRIO 1: Origem = STORAGE, Destino = INVENTÁRIO**

```
Get Parent Storage Widget (Source Slot Widget)  ← Obtém WBP_Storage da origem
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   └─ Move Item From Storage
  │         └─ Target: WBP_Storage (do Get acima)
  │         └─ Storage Item ID: InventoryID (do Source)
  │         └─ Target Slot Index: SlotIndex (do self, 0-49)
  │
  └─ FALSE:
      Return Unhandled
```

#### **CENÁRIO 2: Origem = INVENTÁRIO, Destino = STORAGE**

```
Get Parent Storage Widget (self)  ← Obtém WBP_Storage do destino
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Subtract (SlotIndex do self - 50)  ← Converte 50-149 para 0-99
  │   └─ Move Item To Storage
  │         └─ Target: WBP_Storage (do Get acima)
  │         └─ Inventory Item ID: InventoryID (do Source)
  │         └─ Target Slot Index: Resultado do Subtract (0-99)
  │
  └─ FALSE:
      Return Unhandled
```

### **IMPORTANTE:**

- **`Get Parent Storage Widget`** retorna a referência ao `WBP_Storage` que contém o slot
- Se o slot pertence ao inventário, retorna `None`
- Se o slot pertence ao storage, retorna o `WBP_Storage`

---

## **PARTE 4: Implementação Detalhada do OnDrop**

### **PASSO 3.1: Estrutura Completa do OnDrop**

**No `WBP_InventorySlot` → OnDrop:**

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
  │   │   │     └─ Inventory ID, Slot Index
  │   │   ├─ Get Slot Data (self)
  │   │   ├─ Break Umbra Inventory Slot (self)
  │   │   │     └─ Slot Index
  │   │   │
  │   │   └─ VERIFICAÇÃO DE ORIGEM E DESTINO:
  │   │
  │   │       Get Parent Storage Widget (Source Slot Widget)
  │   │       ↓
  │   │       Is Valid? (Parent Storage Widget da origem)
  │   │       ├─ TRUE: Origem = STORAGE
  │   │       │   ├─ Get Parent Storage Widget (self)
  │   │       │   ├─ Is Valid? (Parent Storage Widget do destino)
  │   │       │   │   ├─ TRUE: Destino = STORAGE
  │   │       │   │   │   └─ (Não implementar - mover dentro do storage)
  │   │       │   │   │
  │   │       │   │   └─ FALSE: Destino = INVENTÁRIO
  │   │       │   │       ├─ Get Parent Storage Widget (Source Slot Widget)  ← WBP_Storage
  │   │       │   │       ├─ Move Item From Storage
  │   │       │   │       │     └─ Target: WBP_Storage (do Get acima)
  │   │       │   │       │     └─ Storage Item ID: InventoryID (do Source)
  │   │       │   │       │     └─ Target Slot Index: SlotIndex (do self)
  │   │       │   │       └─ Branch (Return Value)
  │   │       │   │           ├─ TRUE: Return Handled
  │   │       │   │           └─ FALSE: Return Unhandled
  │   │       │   │
  │   │       │   └─ (nada se destino = storage)
  │   │       │
  │   │       └─ FALSE: Origem = INVENTÁRIO
  │   │           ├─ Get Parent Storage Widget (self)
  │   │           ├─ Is Valid? (Parent Storage Widget do destino)
  │   │           │   ├─ TRUE: Destino = STORAGE
  │   │           │   │   ├─ Get Parent Storage Widget (self)  ← WBP_Storage
  │   │           │   │   ├─ Subtract (SlotIndex do self - 50)  ← Ajusta para índice 0-99
  │   │           │   │   ├─ Move Item To Storage
  │   │           │   │   │     └─ Target: WBP_Storage (do Get acima)
  │   │           │   │   │     └─ Inventory Item ID: InventoryID (do Source)
  │   │           │   │   │     └─ Target Slot Index: Resultado do Subtract
  │   │           │   │   └─ Branch (Return Value)
  │   │           │   │       ├─ TRUE: Return Handled
  │   │           │   │       └─ FALSE: Return Unhandled
  │   │           │   │
  │   │           │   └─ FALSE: Destino = INVENTÁRIO
  │   │           │       └─ (Lógica existente de mover dentro do inventário)
  │   │           │       └─ Process Item Drop (Source Slot Widget)
  │   │           │       └─ Return Handled
  │   │           │
  │   │           └─ (nada se origem = inventário e destino = storage)
  │   │
  │   └─ FALSE:
  │       Return Unhandled
  │
  └─ CastFailed:
      Return Unhandled
```

---

## **PARTE 5: Implementar MoveItemToStorage e MoveItemFromStorage**

### **ANÁLISE:**

As funções `MoveItemToStorage` e `MoveItemFromStorage` no C++ estão marcadas como `TODO`. Você tem duas opções:

### **PROBLEMA IDENTIFICADO:**

A função `MoveItem` do `GameInstance` tem uma validação que limita `TargetSlotIndex` a **0-49** (apenas inventário). Não pode ser usada diretamente para storage (50-149).

### **SOLUÇÃO: Implementar Funções Blueprint que Chamam a API Diretamente**

**No `WBP_Storage`, crie funções Blueprint que fazem chamadas HTTP diretas:**

#### **Função: MoveItemToStorageBlueprint**

```
MoveItemToStorageBlueprint (Function)
  ├─ Input: InventoryItemID (Integer)
  └─ Input: TargetSlotIndex (Integer)  ← Índice 0-99 do storage
  ↓
Get MyGameInstance
  ↓
Get Current Token (GameInstance)
  ↓
Create Request (VaRest)
  └─ URL: "http://localhost/umbra_api/api/storage/move_to_storage.php"
  └─ Verb: "POST"
  ↓
Construct Va Rest Json Object
  ├─ Set String Field: "token" = Current Token
  ├─ Set Number Field: "inventory_id" = InventoryItemID
  └─ Set Number Field: "target_slot_index" = TargetSlotIndex + 50  ← Converte para 50-149
  ↓
Set Request Object
  ↓
Execute Process Request
  ↓
On Request Complete (Bind Event)
  └─ Branch (success)
      ├─ TRUE:
      │   ├─ Load Storage  ← Recarrega o storage
      │   └─ Load Inventory (GameInstance)  ← Recarrega o inventário
      │
      └─ FALSE:
          Print String "Falha ao mover item para storage"
```

#### **Função: MoveItemFromStorageBlueprint**

```
MoveItemFromStorageBlueprint (Function)
  ├─ Input: StorageItemID (Integer)
  └─ Input: TargetSlotIndex (Integer)  ← Índice 0-49 do inventário
  ↓
Get MyGameInstance
  ↓
Get Current Token (GameInstance)
  ↓
Create Request (VaRest)
  └─ URL: "http://localhost/umbra_api/api/storage/move_from_storage.php"
  └─ Verb: "POST"
  ↓
Construct Va Rest Json Object
  ├─ Set String Field: "token" = Current Token
  ├─ Set Number Field: "storage_item_id" = StorageItemID
  └─ Set Number Field: "target_slot_index" = TargetSlotIndex  ← Já está no range 0-49
  ↓
Set Request Object
  ↓
Execute Process Request
  ↓
On Request Complete (Bind Event)
  └─ Branch (success)
      ├─ TRUE:
      │   ├─ Load Storage  ← Recarrega o storage
      │   └─ Load Inventory (GameInstance)  ← Recarrega o inventário
      │
      └─ FALSE:
          Print String "Falha ao mover item do storage"
```

### **IMPLEMENTAÇÃO DETALHADA DAS FUNÇÕES BLUEPRINT:**

#### **PASSO A PASSO: MoveItemToStorageBlueprint**

**No `WBP_Storage` → Functions → MoveItemToStorageBlueprint:**

1. **Criar a função:**
   - Nome: `MoveItemToStorageBlueprint`
   - Inputs: `InventoryItemID` (Integer), `TargetSlotIndex` (Integer)

2. **Obter GameInstance e Token:**
   ```
   Get MyGameInstance
   ↓
   Get Current Token (GameInstance)
   ```

3. **Criar requisição VaRest:**
   ```
   Get Va Rest Subsystem
   ↓
   Construct Va Rest Request JSON
     └─ URL: "http://localhost/umbra_api/api/storage/move_to_storage.php"
     └─ Verb: "POST"
   ```

4. **Criar JSON Object:**
   ```
   Construct Va Rest Json Object
   ↓
   Set String Field
     └─ Field Name: "token"
     └─ String Value: Current Token
   ↓
   Set Number Field
     └─ Field Name: "inventory_id"
     └─ Number Value: InventoryItemID
   ↓
   Add (TargetSlotIndex + 50)  ← Converte 0-99 para 50-149
   ↓
   Set Number Field
     └─ Field Name: "target_slot_index"
     └─ Number Value: Resultado do Add
   ```

5. **Configurar e executar requisição:**
   ```
   Set Request Object (VaRest Request)
     └─ Request Object: JSON Object criado
   ↓
   Bind Event to OnRequestComplete
     └─ Custom Event: OnMoveToStorageComplete
   ↓
   Bind Event to OnRequestFail
     └─ Custom Event: OnMoveToStorageFail
   ↓
   Execute Process Request
   ```

6. **Criar Custom Events para callbacks:**
   ```
   OnMoveToStorageComplete (Custom Event)
     ├─ Input: Request (VaRest Request JSON)
     ↓
     Get Response Object
     ↓
     Get Bool Field ("success")
     ↓
     Branch (success)
       ├─ TRUE:
       │   ├─ Load Storage (self)
       │   └─ Load Inventory (MyGameInstance)
       │
       └─ FALSE:
           Get String Field ("message")
           Print String (message)
   ```

#### **PASSO A PASSO: MoveItemFromStorageBlueprint**

**Similar ao acima, mas:**
- URL: `"http://localhost/umbra_api/api/storage/move_from_storage.php"`
- Field: `"storage_item_id"` (não `"inventory_id"`)
- `TargetSlotIndex` já está no range 0-49 (não precisa adicionar 50)

---

### **ALTERNATIVA: Modificar C++ MoveItem para Aceitar 50-149**

**Se você quiser usar a função `MoveItem` existente, modifique o C++:**

**No `UmbraGameInstance.cpp`, linha ~1603:**

**ANTES:**
```cpp
if (TargetSlotIndex < 0 || TargetSlotIndex >= 50)
```

**DEPOIS:**
```cpp
if (TargetSlotIndex < 0 || TargetSlotIndex >= 150)  // Aceita 0-149 (inventário + storage)
```

**E modifique a API `move_item.php` para aceitar índices 50-149 e mover para a tabela `player_storage`.**

**RECOMENDAÇÃO:** Use a solução de criar funções Blueprint que chamam APIs específicas de storage (mais limpo e separado).

---

## **PARTE 6: Exemplo Completo de OnDrop no WBP_InventorySlot**

### **CÓDIGO COMPLETO DO OnDrop:**

**IMPORTANTE:** Este é o código completo que você deve implementar no `WBP_InventorySlot` → `OnDrop`.

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
  │   │       │   └─ VERIFICAÇÃO DE DESTINO:
  │   │       │       Get Parent Storage Widget (self)
  │   │       │       ↓
  │   │       │       Is Valid? (Parent Storage Widget do destino)
  │   │       │       ├─ TRUE: Destino = STORAGE
  │   │       │       │   └─ Return Unhandled  ← Não implementar mover dentro do storage
  │   │       │       │
  │   │       │       └─ FALSE: Destino = INVENTÁRIO
  │   │       │           ├─ Get Parent Storage Widget (Source Slot Widget)  ← WBP_Storage
  │   │       │           ├─ Is Valid? (WBP_Storage)
  │   │       │           ├─ TRUE:
  │   │       │           │   ├─ Move Item From Storage
  │   │       │           │   │     └─ Target: WBP_Storage
  │   │       │           │   │     └─ Storage Item ID: InventoryID (do Source)
  │   │       │           │   │     └─ Target Slot Index: SlotIndex (do self, 0-49)
  │   │       │           │   ├─ Branch (Return Value)
  │   │       │           │   │   ├─ TRUE:
  │   │       │           │   │   │   ├─ Load Storage (WBP_Storage)
  │   │       │           │   │   │   ├─ Load Inventory (GameInstance)
  │   │       │           │   │   │   └─ Return Handled
  │   │       │           │   │   └─ FALSE:
  │   │       │           │   │       └─ Return Unhandled
  │   │       │           │   └─ FALSE:
  │   │       │           │       └─ Return Unhandled
  │   │       │
  │   │       └─ FALSE: Origem = INVENTÁRIO
  │   │           └─ VERIFICAÇÃO DE DESTINO:
  │   │               Get Parent Storage Widget (self)
  │   │               ↓
  │   │               Is Valid? (Parent Storage Widget do destino)
  │   │               ├─ TRUE: Destino = STORAGE
  │   │               │   ├─ Get Parent Storage Widget (self)  ← WBP_Storage
  │   │               │   ├─ Is Valid? (WBP_Storage)
  │   │               │   ├─ TRUE:
  │   │               │   │   ├─ Get Slot Index (self)
  │   │               │   │   ├─ Subtract (SlotIndex - 50)  ← Converte 50-149 para 0-99
  │   │               │   │   ├─ Move Item To Storage
  │   │               │   │   │     └─ Target: WBP_Storage
  │   │               │   │   │     └─ Inventory Item ID: InventoryID (do Source)
  │   │               │   │   │     └─ Target Slot Index: Resultado do Subtract (0-99)
  │   │               │   │   ├─ Branch (Return Value)
  │   │               │   │   │   ├─ TRUE:
  │   │               │   │   │   │   ├─ Load Storage (WBP_Storage)
  │   │               │   │   │   │   ├─ Load Inventory (GameInstance)
  │   │               │   │   │   │   └─ Return Handled
  │   │               │   │   │   └─ FALSE:
  │   │               │   │   │       └─ Return Unhandled
  │   │               │   │   └─ FALSE:
  │   │               │   │       └─ Return Unhandled
  │   │               │   │
  │   │               └─ FALSE: Destino = INVENTÁRIO
  │   │                   └─ (Lógica existente de mover dentro do inventário)
  │   │                   └─ Process Item Drop (Source Slot Widget)
  │   │                   └─ Return Handled
  │   │
  │   └─ FALSE:
  │       Return Unhandled
  │
  └─ CastFailed:
      Return Unhandled
```

---

## **PARTE 7: Resumo e Checklist**

### **CHECKLIST DE IMPLEMENTAÇÃO:**

#### **WBP_Storage:**
- [ ] Criar variável `SlotWidgets` (Array de WBP_InventorySlot)
- [ ] Criar variável `MyGameInstance` (Umbra Game Instance)
- [ ] Criar variável `UniformGridPanel_Storage` (Uniform Grid Panel)
- [ ] Implementar `Event Construct` (obter GameInstance, chamar LoadStorage)
- [ ] Criar função `CreateStorageSlots` (criar 100 slots, índices 50-149)
- [ ] Criar função `UpdateAllSlotsVisual` (atualizar slots com dados)
- [ ] Implementar `OnStorageLoaded` (chamar CreateStorageSlots e UpdateAllSlotsVisual)

#### **WBP_InventorySlot:**
- [ ] Modificar `OnDrop` para verificar `ParentStorageWidget`
- [ ] Adicionar lógica para detectar origem (inventário ou storage)
- [ ] Adicionar lógica para detectar destino (inventário ou storage)
- [ ] Implementar chamada a `MoveItemToStorage` quando origem = inventário e destino = storage
- [ ] Implementar chamada a `MoveItemFromStorage` quando origem = storage e destino = inventário

#### **C++ (Opcional - se necessário):**
- [ ] Implementar API para mover item do inventário para storage
- [ ] Implementar API para mover item do storage para inventário
- [ ] Modificar `MoveItem` no `GameInstance` para aceitar índices 50-149

---

## **NOTAS IMPORTANTES:**

1. **Índices do Storage:** 
   - **No Blueprint (arrays):** Use `SlotIndex - 50` para converter 50-149 → 0-99
   - **Na API:** Use `SlotIndex` diretamente (50-149)

2. **ParentStorageWidget:** 
   - Esta propriedade é CRUCIAL para identificar se um slot pertence ao storage ou ao inventário
   - Se `ParentStorageWidget` é válido → slot pertence ao storage
   - Se `ParentStorageWidget` é `None` → slot pertence ao inventário

3. **Obter Referência ao WBP_Storage:**
   - Use `Get Parent Storage Widget` do slot de origem ou destino
   - Isso retorna a referência ao `WBP_Storage` que contém o slot

4. **Validação:** 
   - Sempre valide se `ParentStorageWidget` é válido antes de chamar funções do storage
   - Sempre valide se o `WBP_Storage` é válido antes de chamar `MoveItemToStorage` ou `MoveItemFromStorage`

5. **Atualização Visual:** 
   - Após mover um item, você precisa atualizar os visuais de ambos os widgets (inventário e storage)
   - Chame `Load Storage` no `WBP_Storage`
   - Chame `Load Inventory` no `GameInstance`

6. **Funções C++ MoveItemToStorage/MoveItemFromStorage:**
   - Atualmente estão marcadas como `TODO` no C++
   - Você DEVE implementar as funções Blueprint que chamam a API diretamente
   - OU modificar o C++ para implementar essas funções

---

## **PRÓXIMOS PASSOS:**

### **ORDEM DE IMPLEMENTAÇÃO:**

1. **WBP_Storage - Configuração Básica:**
   - [ ] Criar variáveis (`SlotWidgets`, `MyGameInstance`, `UniformGridPanel_Storage`)
   - [ ] Implementar `Event Construct`
   - [ ] Criar função `CreateStorageSlots`
   - [ ] Criar função `UpdateAllSlotsVisual`
   - [ ] Implementar `OnStorageLoaded`

2. **WBP_Storage - Funções de Movimento:**
   - [ ] Criar função `MoveItemToStorageBlueprint` (chama API)
   - [ ] Criar função `MoveItemFromStorageBlueprint` (chama API)
   - [ ] Testar essas funções manualmente

3. **WBP_InventorySlot - OnDrop:**
   - [ ] Modificar `OnDrop` para verificar `ParentStorageWidget`
   - [ ] Adicionar lógica de detecção de origem e destino
   - [ ] Implementar chamadas a `MoveItemToStorage` e `MoveItemFromStorage`

4. **Testes:**
   - [ ] Testar criar slots do storage
   - [ ] Testar carregar dados do storage
   - [ ] Testar drag and drop do inventário para storage
   - [ ] Testar drag and drop do storage para inventário

5. **APIs (se necessário):**
   - [ ] Criar `/api/storage/move_to_storage.php`
   - [ ] Criar `/api/storage/move_from_storage.php`
   - [ ] Criar `/api/storage/get_storage.php` (se não existir)

---

## **RESUMO FINAL:**

### **O QUE FAZER EM CADA WBP:**

#### **WBP_Storage:**
- Criar slots (100 slots, índices 50-149)
- Carregar dados do storage
- Atualizar visuais dos slots
- Funções para mover itens (chamando API)

#### **WBP_InventorySlot:**
- Modificar `OnDrop` para detectar origem e destino
- Chamar funções do storage quando necessário
- Manter lógica existente de mover dentro do inventário

#### **WBP_Inventory:**
- Nenhuma mudança necessária (usa a mesma lógica)

---

## **DICAS FINAIS:**

1. **Use `ParentStorageWidget` para identificar origem/destino**
2. **Sempre ajuste índices:** Storage usa 50-149, mas arrays usam 0-99
3. **Valide tudo:** Sempre verifique se widgets são válidos antes de usar
4. **Atualize ambos:** Após mover, atualize inventário E storage
5. **Teste incrementalmente:** Implemente uma parte, teste, depois continue

