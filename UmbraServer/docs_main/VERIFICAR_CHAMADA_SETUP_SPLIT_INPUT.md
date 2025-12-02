# 🔍 VERIFICAR: Como Setup Split Input Está Sendo Chamado

## ❌ **PROBLEMA:**

A função `Setup Split Input` está correta, mas pode estar sendo chamada incorretamente ou o `SourceSlot` está chegando como `None`.

---

## ✅ **VERIFICAÇÃO 1: Como a Função Está Sendo Chamada**

### **PASSO 1: Abrir WBP_InventorySlot → OnDragDetected**

1. **Abra o Blueprint `WBP_InventorySlot`**
2. **No Event Graph, encontre o evento `OnDragDetected`**
3. **Procure pela chamada de `Setup Split Input`**

**Você deve ver algo assim:**

```
[OnDragDetected]
  ↓
[Branch] (Is Shift Down?)
  ├─ TRUE: (Shift pressionado)
  │   ├─ [Get Slot Data] (self)
  │   ├─ [Break Umbra Inventory Slot]
  │   │     └─ Quantity
  │   ├─ [Branch] (Quantity > 1)
  │   │   ├─ TRUE:
  │   │   │   ├─ [Create Widget] (WBP_SplitInput)
  │   │   │   │     └─ Return Value: SplitInputWidget
  │   │   │   ├─ [Setup Split Input] ← ✅ VERIFICAR AQUI!
  │   │   │   │     └─ Target: SplitInputWidget
  │   │   │   │     └─ Source Slot: ??? ← ✅ DEVE SER self!
  │   │   │   │     └─ Item Quantity: Quantity
```

---

## ✅ **VERIFICAÇÃO 2: Conectar Source Slot Corretamente**

### **O pin `Source Slot` do `Setup Split Input` DEVE estar conectado a:**

**`self` (do WBP_InventorySlot)**

**COMO VERIFICAR:**
1. **Encontre o nó `Setup Split Input`**
2. **Veja o pin `Source Slot`**
3. **Ele DEVE estar conectado a `self` (do WBP_InventorySlot)**

**SE NÃO ESTIVER CONECTADO:**
1. **Arraste do pin `self` (do WBP_InventorySlot)**
2. **Conecte ao pin `Source Slot` do `Setup Split Input`**

---

## ✅ **VERIFICAÇÃO 3: Adicionar Validação na Função**

### **Adicionar Branch para Verificar se SourceSlot é Válido**

**No `WBP_SplitInput` → `Setup Split Input`:**

**ADICIONE NO INÍCIO DA FUNÇÃO:**

```
[Setup Split Input]
  ├─ Input: SourceSlot
  ├─ Input: ItemQuantity
  ↓
[Is Valid] ← ✅ ADICIONAR ESTE NÓ!
  └─ Object: SourceSlot (PARÂMETRO)
  └─ Return Value: (Boolean)
  ↓
[Branch] ← ✅ ADICIONAR ESTE NÓ!
  ├─ Condition: (Is Valid Return Value)
  ├─ TRUE: (SourceSlot é válido - CONTINUAR)
  │   └─ [Set Source Slot Widget] ← CONTINUAR COM O CÓDIGO ATUAL
  │       └─ Value: SourceSlot (PARÂMETRO)
  │       └─ then → [Set Current Quantity]
  │           └─ ... resto do código ...
  │
  └─ FALSE: (SourceSlot é None - ERRO!)
      └─ [Print String] "Erro: SourceSlot é None!"
      └─ [Return] ← SAIR DA FUNÇÃO
```

**COMO OBTER:**
1. **Right Click no Event Graph**
2. **Digite "Is Valid"**
3. **Selecione "Is Valid"**
4. **Conecte:**
   - `Object`: `SourceSlot` (PARÂMETRO da função)
   - `Return Value` → `Condition` de um `Branch`

---

## ✅ **VERIFICAÇÃO 4: Adicionar Validação no Get Slot Data**

### **Adicionar Branch após Get Slot Data**

**APÓS o `Get Slot Data`, ADICIONE:**

```
[Get Slot Data]
  └─ Target: SourceSlot (PARÂMETRO)
  └─ Return Value: SlotData
  ↓
[Break Umbra Inventory Slot]
  └─ Input: SlotData
  └─ Quantity ← ✅ VERIFICAR SE É > 0
  ↓
[Branch] ← ✅ ADICIONAR ESTE NÓ!
  ├─ Condition: Quantity > 0
  ├─ TRUE: (Quantidade válida - CONTINUAR)
  │   └─ ... resto do código ...
  │
  └─ FALSE: (Quantidade inválida - ERRO!)
      └─ [Print String] "Erro: Quantidade inválida!"
      └─ [Return] ← SAIR DA FUNÇÃO
```

---

## ✅ **VERIFICAÇÃO 5: Verificar Logs no Unreal**

### **Adicionar Print String para Debug**

**ADICIONE NO INÍCIO DA FUNÇÃO:**

```
[Setup Split Input]
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "Setup Split Input chamado"
  └─ Text Color: Yellow
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "SourceSlot: [valor]"
  └─ Text Color: Yellow
  └─ (Use Format Text com SourceSlot convertido para String)
  ↓
[Print String] ← ✅ ADICIONAR PARA DEBUG
  └─ In String: "ItemQuantity: [valor]"
  └─ Text Color: Yellow
  └─ (Use Format Text com ItemQuantity)
```

**ISSO VAI MOSTRAR NO LOG:**
- Se a função está sendo chamada
- Qual o valor de `SourceSlot`
- Qual o valor de `ItemQuantity`

---

## 🎯 **RESUMO DAS VERIFICAÇÕES:**

1. ✅ **Verificar se `SourceSlot` está conectado a `self` em `OnDragDetected`**
2. ✅ **Adicionar `Is Valid` no início da função**
3. ✅ **Adicionar `Branch` após `Get Slot Data`**
4. ✅ **Adicionar `Print String` para debug**

---

## ⚠️ **PROBLEMA MAIS COMUM:**

**O `SourceSlot` está sendo passado como `None` quando `Setup Split Input` é chamado.**

**CAUSAS POSSÍVEIS:**
1. **Não está conectado em `OnDragDetected`**
2. **Está conectado a algo errado (não é `self`)**
3. **O widget `WBP_SplitInput` está sendo criado antes de setar a variável**

---

**PRONTO! 🎉**

