# 📦 GUIA SIMPLES: LoadStorage no Blueprint

## ✅ **STATUS:**

- ✅ Função `LoadStorage()` implementada no C++ (`UmbraGameInstance`)
- ✅ Delegate `OnStorageLoaded` disponível
- ✅ Array `CurrentStorage` populado automaticamente
- ✅ Função `GetAllStorageSlots()` disponível

---

## 🎯 **OBJETIVO:**

Usar a função C++ `LoadStorage()` do `UmbraGameInstance` para carregar o storage automaticamente, sem precisar fazer requisições manuais no Blueprint.

---

## 📋 **IMPLEMENTAÇÃO SIMPLES:**

### **PASSO 1: No Event Construct do WBP_Storage**

```
Event Construct
  ↓
Create Storage Slots
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then:
  └─ Load Storage  ← Função C++ automática!
```

**Pronto!** A função C++ já faz tudo:
- ✅ Cria a requisição HTTP
- ✅ Adiciona o token automaticamente
- ✅ Processa a resposta
- ✅ Popula `CurrentStorage` automaticamente
- ✅ Broadcast `OnStorageLoaded` quando completo

---

### **PASSO 2: Conectar ao Delegate OnStorageLoaded**

**No `WBP_Storage` → Event Graph:**

1. **Clique com botão direito → Add Event → Assign On Storage Loaded**
2. **Ou:** Arraste `MyGameInstance` → Event → Assign On Storage Loaded

**Implementação:**

```
On Storage Loaded (Event)
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
ForEachLoop (Storage Slots array)
  Loop Body:
    ├─ Get Array Element
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index, Inventory ID
    ├─ Get Array Item (StorageSlots, Index: Slot Index)
    ├─ Cast to WBP Inventory Slot
    ├─ then:
    │   ├─ Set Slot Data (do Cast)
    │   │     └─ New Slot Data: Array Element
    │   └─ Update Slot Visual (do Cast)
    │
  ForEachLoop Completed
    ↓
  On Storage Loaded (self)
    └─ Used Slots: Get Used Slots (self)
    └─ Total Slots: Get Max Capacity (self)
```

---

## 📋 **ALTERNATIVA: Usar GetAllStorageSlots**

**Se preferir usar a função que retorna todos os 100 slots (incluindo vazios):**

```
On Storage Loaded (Event)
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Get Array Item (GetAllStorageSlots result, Index: Index)
    ├─ Break Umbra Inventory Slot
    │     └─ Inventory ID
    ├─ Get Array Item (StorageSlots, Index: Index)
    ├─ Cast to WBP Inventory Slot
    ├─ then:
    │   ├─ Branch (Inventory ID > 0)
    │   │   ├─ TRUE:
    │   │   │   ├─ Set Slot Data (do Cast)
    │   │   │   │     └─ New Slot Data: Array Element (do GetAllStorageSlots)
    │   │   │   └─ Update Slot Visual (do Cast)
    │   │   │
    │   │   └─ FALSE:
    │   │       ├─ Clear Slot (do Cast)
    │   │       └─ Update Slot Visual (do Cast)
    │
  For Loop Completed
    ↓
  On Storage Loaded (self)
    └─ Used Slots: Get Used Slots (self)
    └─ Total Slots: Get Max Capacity (self)
```

---

## 📋 **PASSO 3: Tratar Erros (Opcional)**

**Conectar ao delegate `OnStorageLoadFailed`:**

```
On Storage Load Failed (Event)
  ├─ Input: Error Message (String)
  ↓
Print String (Error Message)
```

---

## 🔧 **VARIÁVEIS NECESSÁRIAS:**

### **No `WBP_Storage`:**

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

---

## ⚠️ **IMPORTANTE:**

1. **Não precisa fazer requisições manuais:**
   - A função C++ `LoadStorage()` já faz tudo
   - Não precisa usar VaRest diretamente no Blueprint
   - Não precisa criar JSON objects manualmente

2. **Conversão de índices:**
   - A função C++ já converte automaticamente (50-149 → 0-99)
   - `GetAllStorageSlots()` retorna índices 0-99
   - Use diretamente no Blueprint

3. **Atualizar após mover:**
   - Após `MoveItemToStorage` ou `MoveItemFromStorage`
   - Chame `Load Storage` novamente
   - O delegate `OnStorageLoaded` será chamado automaticamente

---

## 📋 **EXEMPLO COMPLETO: Event Construct**

```
Event Construct
  ↓
Create Storage Slots
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then:
  ├─ Set MyGameInstance
  ├─ Assign On Storage Loaded
  │     └─ Custom Event: OnStorageLoaded_Event
  ├─ Assign On Storage Load Failed
  │     └─ Custom Event: OnStorageLoadFailed_Event
  └─ Load Storage
```

---

## 📋 **EXEMPLO COMPLETO: OnStorageLoaded_Event**

```
OnStorageLoaded_Event (Custom Event)
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Get Array Item (GetAllStorageSlots result, Index: Index)
    ├─ Break Umbra Inventory Slot
    │     └─ Inventory ID
    ├─ Get Array Item (StorageSlots, Index: Index)
    ├─ Cast to WBP Inventory Slot
    ├─ then:
    │   ├─ Branch (Inventory ID > 0)
    │   │   ├─ TRUE:
    │   │   │   ├─ Set Slot Data (do Cast)
    │   │   │   │     └─ New Slot Data: Array Element
    │   │   │   └─ Update Slot Visual (do Cast)
    │   │   │
    │   │   └─ FALSE:
    │   │       ├─ Clear Slot (do Cast)
    │   │       └─ Update Slot Visual (do Cast)
    │
  For Loop Completed
    ↓
  On Storage Loaded (self)
    └─ Used Slots: Get Used Slots (self)
    └─ Total Slots: Get Max Capacity (self)
```

---

## 🧪 **TESTE:**

1. Abra o jogo
2. Interaja com o baú de storage
3. **Deve:** Storage abrir e carregar itens automaticamente
4. Verifique logs para confirmar que `LoadStorage()` foi chamado
5. Confirme que os itens aparecem nos slots corretos

---

## 📝 **RESUMO:**

1. ✅ Chame `Load Storage` do `UmbraGameInstance` no `Event Construct`
2. ✅ Conecte ao delegate `On Storage Loaded`
3. ✅ Use `Get All Storage Slots` para obter os dados
4. ✅ Atualize os slots visualmente no loop
5. ✅ Pronto! Tudo automático, sem requisições manuais

---

## 🎉 **VANTAGENS:**

- ✅ **Muito mais simples** - não precisa fazer requisições manuais
- ✅ **Consistente** - usa o mesmo padrão do inventário
- ✅ **Automático** - token, validações, tudo feito no C++
- ✅ **Menos erros** - menos código Blueprint = menos bugs
- ✅ **Manutenível** - mudanças na API só precisam ser feitas no C++

