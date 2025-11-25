# 🔧 DEBUG: DROP NÃO FUNCIONA EM SLOT VAZIO

**PROBLEMA:** Drag & drop funciona quando solta em slot com item (troca), mas **NÃO funciona em slot vazio**.

---

## 🔍 **ANÁLISE:**

O problema provavelmente está em uma destas áreas:

1. **`CanAcceptItem` retorna `false` para slots vazios** (improvável, mas possível)
2. **`ProcessItemDrop` falha silenciosamente** quando o slot está vazio
3. **`RequestMoveItem` não encontra o `InventoryID`** ou falha na API
4. **`OnDrop` no Blueprint não está sendo chamado** para slots vazios

---

## 🛠️ **PASSO 1: ADICIONAR LOGS DE DEBUG ESPECÍFICOS**

### **A. No `OnDrop` - Verificar se está sendo chamado:**

```
On Drop
  ↓
Print String: "OnDrop chamado!"  ← DEBUG
  ↓
Get Slot Data (self)  ← ADICIONAR!
  ↓
Break Umbra Inventory Slot  ← ADICIONAR!
  ↓
Print String: (Inventory ID)  ← DEBUG - Ver se é 0 (vazio)
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓
IsValid
  ↓
Branch
  ├─ TRUE:
  │   └─ Print String: "Cast válido!"  ← DEBUG
  │   └─ Get Source Slot Widget
  │   └─ IsValid
  │   └─ Branch
  │       ├─ TRUE:
  │       │   └─ Print String: "Source Slot válido!"  ← DEBUG
  │       │   └─ Process Item Drop
  │       │       └─ Print String: (Return Value)  ← DEBUG
  │       │
  │       └─ FALSE:
  │           └─ Print String: "Source Slot inválido!"  ← DEBUG
  │
  └─ FALSE:
      └─ Print String: "Cast inválido!"  ← DEBUG
```

---

## 🛠️ **PASSO 2: VERIFICAR `CanAcceptItem` NO C++**

O código do C++ mostra que `CanAcceptItem` retorna `true` para slots vazios. Mas vamos verificar se está sendo chamado corretamente.

**Adicione logs no C++ (opcional, se tiver acesso):**

No arquivo `UmbraInventorySlotWidget.cpp`, linha 83-96, já existem logs. Verifique no Output Log se aparece:

```
[UmbraInventorySlotWidget] CanAcceptItem - Slot vazio, pode aceitar
```

**Se NÃO aparecer este log:**
- O `CanAcceptItem` não está sendo chamado
- Ou o slot não está sendo reconhecido como vazio

---

## 🛠️ **PASSO 3: VERIFICAR SE O SLOT ESTÁ REALMENTE VAZIO**

### **Problema comum: Slot parece vazio, mas tem dados**

O slot pode ter `InventoryID = 0` mas ainda ter `ItemTemplateID > 0` (ou vice-versa), fazendo o `IsSlotEmpty()` retornar `false`.

**Verificação no Blueprint:**

```
On Drop
  ↓
Get Slot Data (self)
  ↓
Break Umbra Inventory Slot
  ↓
Print String: (Inventory ID)  ← Deve ser 0 para slot vazio
  ↓
Print String: (Item Template ID)  ← Deve ser 0 para slot vazio
```

**Se algum dos valores NÃO for 0:**
- O slot não está realmente vazio
- Limpe o slot antes de testar

---

## 🛠️ **PASSO 4: VERIFICAR `ProcessItemDrop`**

### **Possível problema: `ProcessItemDrop` retorna `false` silenciosamente**

Adicione logs antes e depois de chamar `ProcessItemDrop`:

```
Process Item Drop
  ↓
Print String: "ProcessItemDrop chamado!"  ← DEBUG
  ↓
Print String: (Return Value)  ← DEBUG - Ver se é true ou false
```

**Se o `ReturnValue` for `false`:**
- Verifique os logs do C++ no Output Log
- Procure por mensagens de erro do `ProcessItemDrop`

---

## 🛠️ **PASSO 5: VERIFICAR `RequestMoveItem`**

### **Possível problema: `InventoryID` não encontrado**

O `RequestMoveItem` procura o `InventoryID` no `CurrentInventory` do GameInstance. Se não encontrar, retorna sem fazer nada.

**Verificação:**

1. Adicione logs no `OnDrop` para ver o `InventoryID` do item sendo arrastado:

```
Get Source Slot Widget
  ↓
Get Slot Data (do Source Slot Widget)
  ↓
Break Umbra Inventory Slot
  ↓
Print String: (Inventory ID)  ← DEBUG - Ver o ID do item arrastado
```

2. Verifique se este `InventoryID` existe no `CurrentInventory` do GameInstance

---

## 🛠️ **PASSO 6: CORREÇÃO ESPECÍFICA - VERIFICAR `IsSlotEmpty`**

### **Problema possível: `IsSlotEmpty` retorna `false` incorretamente**

O `IsSlotEmpty` verifica:
```cpp
return SlotData.InventoryID <= 0 || SlotData.ItemTemplateID <= 0;
```

**Se o slot tem `InventoryID = 0` mas `ItemTemplateID > 0`:**
- O slot NÃO é considerado vazio
- O `CanAcceptItem` pode retornar `false`

**Correção no Blueprint:**

Antes de chamar `ProcessItemDrop`, verifique explicitamente se o slot está vazio:

```
On Drop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓
IsValid
  ↓
Branch
  ├─ TRUE:
  │   └─ Get Source Slot Widget
  │   └─ IsValid
  │   └─ Branch
  │       ├─ TRUE:
  │       │   └─ Get Slot Data (self)  ← ADICIONAR!
  │       │   └─ Break Umbra Inventory Slot  ← ADICIONAR!
  │       │   └─ Branch (Inventory ID == 0?)  ← ADICIONAR!
  │       │       ├─ TRUE (slot vazio):
  │       │       │   └─ Print String: "Slot vazio, processando drop..."  ← DEBUG
  │       │       │   └─ Process Item Drop
  │       │       │
  │       │       └─ FALSE (slot com item):
  │       │           └─ Print String: "Slot com item, processando drop..."  ← DEBUG
  │       │           └─ Process Item Drop
```

---

## 🔍 **VERIFICAÇÕES ADICIONAIS:**

### **1. Verificar se o slot está habilitado:**

No **Designer** do `WBP_InventorySlot`:
1. Selecione o widget raiz
2. No **Details Panel**, verifique:
   - **Is Enabled**: ✅ Marcado
   - **Visibility**: **Visible**

### **2. Verificar se há `OnDrop` implementado:**

1. No **Event Graph** do `WBP_InventorySlot`
2. Verifique se o override `On Drop` existe
3. Se não existir, adicione via **Functions** → **Override** → **On Drop**

### **3. Verificar logs do C++:**

No **Output Log**, procure por:

```
[UmbraInventorySlotWidget] CanAcceptItem - Slot vazio, pode aceitar
[UmbraInventorySlotWidget] ProcessItemDrop - Movendo item do slot X para slot Y
[UmbraInventorySlotWidget] RequestMoveItem - InventoryID: X, From: Y, To: Z
```

**Se algum destes logs NÃO aparecer:**
- Identifique qual etapa está falhando
- Aplique a correção correspondente

---

## 📋 **CHECKLIST DE DEBUG:**

- [ ] `OnDrop` tem `Print String` no início
- [ ] `OnDrop` verifica se o slot está vazio (`Inventory ID == 0`)
- [ ] `OnDrop` tem `Print String` antes de `ProcessItemDrop`
- [ ] `OnDrop` tem `Print String` com o `ReturnValue` do `ProcessItemDrop`
- [ ] `Get Slot Data` foi adicionado para verificar se o slot está vazio
- [ ] Logs do C++ foram verificados no Output Log
- [ ] Slot está habilitado no Designer
- [ ] Compilou sem erros
- [ ] Testou e verificou os logs

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Adicione os logs de debug** conforme o Passo 1 e 6
2. **Teste o drop em um slot vazio** e verifique:
   - Se "OnDrop chamado!" aparece
   - Se "Slot vazio, processando drop..." aparece
   - Qual é o valor do `ReturnValue` do `ProcessItemDrop`
   - Quais logs do C++ aparecem no Output Log
3. **Envie os logs** que aparecem no Output Log
4. **Com base nos logs**, identificaremos o problema exato

---

**ADICIONE OS LOGS E TESTE!** 🚀

