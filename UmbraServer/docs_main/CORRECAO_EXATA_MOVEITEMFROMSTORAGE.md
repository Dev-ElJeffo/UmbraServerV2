# 🔧 CORREÇÃO EXATA: Conectar ParentStorageWidget ao MoveItemFromStorage

## 📊 **ANÁLISE DO CÓDIGO ATUAL:**

Você JÁ tem:
- `K2Node_VariableGet_3` que obtém `ParentStorageWidget` do slot de **ORIGEM** (via `K2Node_Knot_8` → `GetSourceSlotWidget`)
- Mas esse valor está sendo usado apenas para verificar se o **DESTINO** é storage (no `K2Node_IfThenElse_8`)

**PROBLEMA:**
- O `self` (Target) do `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage) **NÃO está conectado**
- Você precisa conectar o `ParentStorageWidget` do slot de **ORIGEM** ao `self` do `MoveItemFromStorage`

---

## ✅ **SOLUÇÃO SIMPLES:**

Você JÁ tem o `K2Node_VariableGet_3` que obtém o `ParentStorageWidget` do slot de origem. Você só precisa:

1. **Criar uma NOVA conexão** do `ReturnValue` do `K2Node_VariableGet_3` ao `self` do `K2Node_CallFunction_StorageToInv_6`

**MAS ATENÇÃO:** O `K2Node_VariableGet_3` está sendo usado para verificar o destino. Então você precisa de **DOIS** `GetParentStorageWidget`:
- Um para verificar o **DESTINO** (já existe, mas está errado)
- Um para conectar ao `MoveItemFromStorage` (do slot de **ORIGEM**)

---

## 📋 **PASSO A PASSO EXATO:**

### **PASSO 1: Criar GetParentStorageWidget do Slot de ORIGEM**

**Localização:** Após `K2Node_IfThenElse_StorageToInv_2` [then], antes do `MoveItemFromStorage`

**Como fazer:**
1. Encontre o `K2Node_CallFunction_12` (GetSourceSlotWidget)
2. Arraste o `ReturnValue` (azul) do `GetSourceSlotWidget`
3. Digite "Get Parent Storage Widget" ou "Parent Storage Widget"
4. Selecione `Get Parent Storage Widget` (de `UmbraInventorySlotWidget`)
5. Conecte o `Target` (azul) do `GetParentStorageWidget` ao `ReturnValue` (azul) do `GetSourceSlotWidget`

**OU** (mais simples):
1. Encontre o `K2Node_Knot_8` (que já tem o `ReturnValue` do `GetSourceSlotWidget`)
2. Arraste o `OutputPin` (azul) do `K2Node_Knot_8`
3. Digite "Get Parent Storage Widget"
4. Conecte o `Target` (azul) ao `OutputPin` (azul) do `K2Node_Knot_8`

**Código Visual:**
```
K2Node_Knot_8 [OutputPin] (azul)
  └─ GetParentStorageWidget [Target] (azul)
      └─ [ReturnValue] (azul)
```

---

### **PASSO 2: Conectar ao self do MoveItemFromStorage**

**Localização:** `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage)

**Como fazer:**
1. Encontre o `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage)
2. Localize o pino `self` (Target) (azul) - está vazio/desconectado
3. Conecte o `ReturnValue` (azul) do `GetParentStorageWidget` (criado no Passo 1) ao `self` (azul) do `MoveItemFromStorage`

**Código Visual:**
```
GetParentStorageWidget [ReturnValue] (azul)
  └─ K2Node_CallFunction_StorageToInv_6 [self] (Target) (azul)
```

---

## 📊 **DIAGRAMA COMPLETO:**

```
K2Node_IfThenElse_StorageToInv_2 [then] (exec)
  │
  ├─→ K2Node_CallFunction_StorageToInv_6 [execute] (MoveItemFromStorage)
  │
  └─→ GetParentStorageWidget (NOVO)
      ├─ [Target] (azul) ← K2Node_Knot_8 [OutputPin] (azul)
      └─ [ReturnValue] (azul) → K2Node_CallFunction_StorageToInv_6 [self] (Target) (azul)
```

---

## ⚠️ **IMPORTANTE:**

Você JÁ tem um `K2Node_VariableGet_3` que obtém `ParentStorageWidget`, mas ele está sendo usado para verificar o **DESTINO**. 

Para o `MoveItemFromStorage`, você precisa do `ParentStorageWidget` do slot de **ORIGEM**, então:

- **OPÇÃO 1:** Criar um NOVO `GetParentStorageWidget` usando o `K2Node_Knot_8` (slot de origem)
- **OPÇÃO 2:** Reutilizar o `K2Node_VariableGet_3`, mas criar uma NOVA conexão do seu `ReturnValue` ao `self` do `MoveItemFromStorage`

**Recomendação:** Use a **OPÇÃO 1** para evitar confusão, pois o `K2Node_VariableGet_3` está sendo usado para outra coisa.

---

## ✅ **VERIFICAÇÃO FINAL:**

Após a correção, verifique se:

1. ✅ Existe um `GetParentStorageWidget` que recebe o `ReturnValue` do `GetSourceSlotWidget` (via `K2Node_Knot_8`)
2. ✅ O `ReturnValue` desse `GetParentStorageWidget` está conectado ao `self` (Target) do `MoveItemFromStorage`
3. ✅ O `MoveItemFromStorage` [execute] está conectado ao `then` do `K2Node_IfThenElse_StorageToInv_2`

---

**Com essa correção, o Storage → Inventário deve funcionar!** 🚀

