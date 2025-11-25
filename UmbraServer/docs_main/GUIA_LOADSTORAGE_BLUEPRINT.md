# 📦 GUIA: LoadStorageBlueprint - Passo a Passo

## ⚠️ **ATENÇÃO:**

Este guia foi atualizado! Agora existe uma função C++ `LoadStorage()` no `UmbraGameInstance` que faz tudo automaticamente.

**Recomendação:** Use o guia `GUIA_SIMPLES_LOADSTORAGE_BLUEPRINT.md` que é muito mais simples!

---

## 🎯 **OBJETIVO (MÉTODO ANTIGO - NÃO RECOMENDADO):**

Criar uma função Blueprint que carrega os itens do storage do servidor manualmente usando VaRest.

**⚠️ Este método é mais complexo e não é necessário! Use `LoadStorage()` do GameInstance.**

---

## 📋 **PASSO 1: Criar Função LoadStorageBlueprint (MÉTODO ANTIGO)**

### **No `WBP_Storage`:**

1. **Abra o Blueprint `WBP_Storage`**
2. **Vá para Functions → Add Function**
3. **Nome:** `LoadStorageBlueprint`
4. **Category:** "Storage"

---

## 📋 **PASSO 2: Implementar a Função**

### **NÓS NECESSÁRIOS:**

```
LoadStorageBlueprint
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then:
  ├─ Get Current Token (Umbra Game Instance)
  ├─ Get Va Rest Subsystem
  ├─ Construct Va Rest Request Ext
  │     └─ Verb: POST
  │     └─ Content Type: JSON
  ├─ Make Literal String
  │     └─ Value: "http://localhost/umbra_api/api/storage/get_storage.php"
  ├─ Set URL (VaRest Request)
  │     └─ URL: String acima
  ├─ Construct Va Rest Json Object
  ├─ Set String Field
  │     └─ Field Name: "token"
  │     └─ String Value: Current Token
  ├─ Set Request Object (VaRest Request)
  │     └─ Request Object: JSON Object
  ├─ Bind Event to OnRequestComplete
  │     └─ Custom Event: OnStorageLoadComplete
  ├─ Bind Event to OnRequestFail
  │     └─ Custom Event: OnStorageLoadFail
  └─ Execute Process Request
```

---

## 📋 **PASSO 3: Criar Custom Event OnStorageLoadComplete**

### **No `WBP_Storage` → Event Graph:**

1. **Clique com botão direito → Add Custom Event**
2. **Nome:** `OnStorageLoadComplete`
3. **Input:** `Request` (VaRest Request JSON)

### **IMPLEMENTAÇÃO:**

```
OnStorageLoadComplete
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
  │   │
  │   └─ ForEachLoop (storage array)
  │       Loop Body:
  │         ├─ Get Array Element
  │         ├─ Get Object Field ("storage_id")
  │         ├─ Get Object Field ("inventory_id")
  │         ├─ Get Object Field ("slot_index")
  │         ├─ Subtract (slot_index - 50)  ← Converte para 0-99
  │         ├─ Get Object Field ("item_template_id")
  │         ├─ Get Object Field ("quantity")
  │         ├─ Get Object Field ("durability")
  │         ├─ Get Object Field ("is_equipped")
  │         │
  │         └─ ATUALIZAR SLOT:
  │             Get Array Item (StorageSlots, Index: Resultado do Subtract)
  │             ↓
  │             Cast to WBP Inventory Slot
  │             ↓
  │             then:
  │               ├─ Make Umbra Inventory Slot
  │               │     ├─ Inventory ID: inventory_id
  │               │     ├─ Slot Index: Resultado do Subtract
  │               │     ├─ Item Template ID: item_template_id
  │               │     ├─ Quantity: quantity
  │               │     ├─ Durability: durability
  │               │     └─ Is Equipped: is_equipped
  │               ├─ Set Slot Data (do Cast)
  │               │     └─ New Slot Data: Struct acima
  │               └─ Update Slot Visual (do Cast)
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

## 📋 **PASSO 4: Criar Custom Event OnStorageLoadFail**

### **No `WBP_Storage` → Event Graph:**

1. **Clique com botão direito → Add Custom Event**
2. **Nome:** `OnStorageLoadFail`
3. **Input:** `Request` (VaRest Request JSON)

### **IMPLEMENTAÇÃO:**

```
OnStorageLoadFail
  ├─ Input: Request (VaRest Request JSON)
  ↓
Print String "Falha ao carregar storage"
```

---

## 📋 **PASSO 5: Chamar LoadStorageBlueprint no Event Construct**

### **No `WBP_Storage` → Event Graph → Event Construct:**

**Adicione após `CreateStorageSlots`:**

```
Event Construct
  ↓
Create Storage Slots
  ↓
Load Storage Blueprint  ← Adicionar esta chamada
```

---

## 🔧 **DETALHES IMPORTANTES:**

### **1. Como Obter "Get Object Field":**

- Clique com botão direito → **"Get Object Field"**
- Conecte o `Array Element` do `ForEachLoop` ao `Object` pin
- Digite o nome do campo (ex: "storage_id", "inventory_id")

### **2. Conversão de Índices:**

- API retorna `slot_index` no range 50-149
- Storage Blueprint usa índices 0-99
- Use `Subtract (slot_index - 50)` para converter

### **3. Estrutura do JSON da API:**

```json
{
  "success": true,
  "storage": [
    {
      "storage_id": 1,
      "inventory_id": 7,
      "slot_index": 50,
      "item_template_id": 13,
      "quantity": 10,
      "durability": 100.0,
      "is_equipped": false,
      ...
    }
  ]
}
```

### **4. Atualizar Slots Vazios:**

**Após o `ForEachLoop`, limpe todos os slots que não foram atualizados:**

```
ForEachLoop Completed
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Get Array Item (StorageSlots, Index: Index)
    ├─ Cast to WBP Inventory Slot
    ├─ then:
    │   ├─ Get Slot Data
    │   ├─ Break Umbra Inventory Slot
    │   │     └─ Inventory ID
    │   ├─ Branch (Inventory ID > 0)
    │   │   ├─ TRUE: (já tem item, pular)
    │   │   │
    │   │   └─ FALSE: (slot vazio, garantir que está limpo)
    │   │       ├─ Clear Slot
    │   │       └─ Update Slot Visual
```

---

## ⚠️ **PROBLEMAS COMUNS:**

### **PROBLEMA 1: "Get Object Field" não encontra o campo**

**Solução:** Certifique-se de que:
- O `Array Element` está conectado ao `Object` pin
- O nome do campo está correto (exatamente como na API)
- O campo existe no JSON (verifique a resposta da API)

### **PROBLEMA 2: Slots não aparecem visualmente**

**Solução:** Certifique-se de que:
- `Set Slot Data` está sendo chamado
- `Update Slot Visual` está sendo chamado após `Set Slot Data`
- O `SlotIndex` está correto (0-99)

### **PROBLEMA 3: Índices incorretos**

**Solução:** Sempre use `Subtract (slot_index - 50)` para converter de 50-149 para 0-99

---

## 🧪 **TESTE:**

1. Abra o jogo
2. Interaja com o baú de storage
3. **Deve:** Storage abrir e carregar itens automaticamente
4. Verifique logs para erros
5. Confirme que os itens aparecem nos slots corretos

---

## 📝 **RESUMO:**

1. ✅ Criar função `LoadStorageBlueprint`
2. ✅ Criar Custom Event `OnStorageLoadComplete`
3. ✅ Criar Custom Event `OnStorageLoadFail`
4. ✅ Processar array `storage` da resposta
5. ✅ Atualizar cada slot do storage
6. ✅ Chamar `LoadStorageBlueprint` no `Event Construct`

