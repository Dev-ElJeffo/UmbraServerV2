# 🔧 CORREÇÃO CRÍTICA: MoveItemFromStorage Target

## ❌ **PROBLEMA:**

O `self` (Target) do `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage) **NÃO está conectado**.

**Por que isso é crítico:**
- `MoveItemFromStorage` é uma função do `UUmbraStorageWidget`
- Ela precisa saber qual widget de storage chamar
- Sem o `Target` conectado, a função não funcionará

---

## ✅ **SOLUÇÃO:**

Precisamos obter o `ParentStorageWidget` do slot de origem (Source Slot Widget) e conectar ao `self` do `MoveItemFromStorage`.

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Adicionar GetParentStorageWidget**

**Localização:** Após `K2Node_IfThenElse_StorageToInv_2` [then], antes do `MoveItemFromStorage`

**Como fazer:**
1. Encontre o `K2Node_CallFunction_12` (GetSourceSlotWidget)
2. Arraste o `ReturnValue` (azul) do `GetSourceSlotWidget`
3. Digite "Get Parent Storage Widget" no menu de busca
4. Selecione `Get Parent Storage Widget` (de `UmbraInventorySlotWidget`)
5. Conecte o `Target` (azul) do `GetParentStorageWidget` ao `ReturnValue` (azul) do `GetSourceSlotWidget`

**Visualização:**
```
K2Node_CallFunction_12 (GetSourceSlotWidget)
  └─ [ReturnValue] (azul)
      └─ GetParentStorageWidget [Target] (azul)
          └─ [ReturnValue] (azul)
```

---

### **PASSO 2: Conectar ao self do MoveItemFromStorage**

**Localização:** `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage)

**Como fazer:**
1. Encontre o `K2Node_CallFunction_StorageToInv_6` (MoveItemFromStorage)
2. Localize o pino `self` (Target) (azul) - está vazio/desconectado
3. Conecte o `ReturnValue` (azul) do `GetParentStorageWidget` ao `self` (azul) do `MoveItemFromStorage`

**Visualização:**
```
GetParentStorageWidget
  └─ [ReturnValue] (azul)
      └─ K2Node_CallFunction_StorageToInv_6 (MoveItemFromStorage)
          └─ [self] (Target) (azul)
```

---

## 📊 **DIAGRAMA COMPLETO:**

```
K2Node_IfThenElse_StorageToInv_2 [then] (exec)
  │
  ├─→ K2Node_CallFunction_StorageToInv_6 [execute] (MoveItemFromStorage)
  │
  └─→ GetParentStorageWidget
      ├─ [Target] (azul) ← K2Node_CallFunction_12 [ReturnValue] (GetSourceSlotWidget)
      └─ [ReturnValue] (azul) → K2Node_CallFunction_StorageToInv_6 [self] (Target) (azul)
```

---

## ✅ **VERIFICAÇÃO:**

Após a correção, verifique se:

1. ✅ `GetParentStorageWidget` está presente
2. ✅ `GetParentStorageWidget` [Target] está conectado ao `ReturnValue` do `GetSourceSlotWidget`
3. ✅ `GetParentStorageWidget` [ReturnValue] está conectado ao `self` (Target) do `MoveItemFromStorage`
4. ✅ `MoveItemFromStorage` [execute] está conectado ao `then` do `K2Node_IfThenElse_StorageToInv_2`

---

## 🎯 **RESULTADO ESPERADO:**

Após essa correção:
- O `MoveItemFromStorage` saberá qual widget de storage chamar
- A função será executada no widget de storage correto
- O item será movido do storage para o inventário corretamente

---

**Com essa correção, o Storage → Inventário deve funcionar!** 🚀

