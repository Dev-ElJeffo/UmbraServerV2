# 🖱️ GUIA CORRETO: OnMouseButtonDoubleClick (Como Obter o Botão do Mouse)

## ✅ **CORREÇÃO IMPORTANTE:**

O `OnMouseButtonDoubleClick` **NÃO** tem um pin `Button` direto!

**O que ele TEM:**
- `exec` (execução)
- `In My Geometry` (FGeometry)
- `In Mouse Event` (FPointerEvent) ← **ESTE É O QUE PRECISAMOS!**

---

## 🔧 **COMO OBTER O BOTÃO DO MOUSE:**

### **PASSO 1: Criar o Evento OnMouseButtonDoubleClick**

1. No `WBP_EquipmentSlot`, abra o **Event Graph**
2. Clique direito → **Override** → `OnMouseButtonDoubleClick`
3. Você terá:
   - `exec` (execução)
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent) ← **ESTE!**

---

### **PASSO 2: Obter o Botão do MouseEvent**

**OPÇÃO A: Get Mouse Button (Recomendado)**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. No menu de busca, digite: **`Get Mouse Button`**
3. Selecione: **`Get Mouse Button`**
4. Você terá:
   - `Return Value` (EKeys) ← **O BOTÃO!**

**OPÇÃO B: Break Pointer Event**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Break Pointer Event`**
3. Selecione: **`Break Pointer Event`**
4. Expanda e procure por: **`Mouse Button`** ou **`Button`**
5. Use esse pin

**⚠️ NOTA:** `Get Mouse Button` é mais direto e simples!

---

### **PASSO 3: Verificar se é Left Mouse Button**

**OPÇÃO A: Usar Switch on EKeys (Mais Simples - RECOMENDADO!)**

1. Arraste o `Return Value` (EKeys) do `Get Mouse Button`
2. Digite: **`Switch on EKeys`**
3. Selecione: **`Switch on EKeys`**
4. Conecte o `Return Value` ao pin `Select` (ou `Key`)
5. No `Switch on EKeys`, adicione um case:
   - Clique em **"Add Pin"** ou **"+"**
   - Selecione: **`Left Mouse Button`**
6. Conecte o pin `Left Mouse Button` (exec) à sua lógica

**OPÇÃO B: Usar Equal (Key Key)**

1. Arraste o `Return Value` (EKeys) do `Get Mouse Button`
2. Digite: **`Equal`**
3. Selecione: **`Equal (Key Key)`** ← **IMPORTANTE: Key Key, não Bool!**
4. Conecte o `Return Value` ao pin `A`
5. Para o pin `B`, você precisa criar um valor `Left Mouse Button`:
   - Clique direito → Digite: **`EKeys`**
   - Selecione: **`EKeys`** (enum)
   - Expanda e procure: **`Left Mouse Button`**
   - Arraste para o pin `B` do `Equal`
6. Conecte o `Return Value` (bool) a um `Branch`

**⚠️ NOTA:** `Switch on EKeys` é mais simples e limpo!

---

## 📊 **ESTRUTURA COMPLETA (OPÇÃO A - Switch on EKeys):**

```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Get Mouse Button] ← Do MouseEvent
    └─ Return Value: (EKeys)
         │
         ▼
  [Switch on EKeys]
    ├─ Select: (EKeys) ← Conectado ao Return Value
    ├─ Left Mouse Button (exec) ───────────────────────┐
    │                                                   │
    │  [Get Equipped Item]                              │
    │    └─ Equipped Item: (FUmbraInventorySlot)       │
    │         │                                         │
    │         ▼                                         │
    │  [Break Umbra Inventory Slot]                    │
    │    ├─ Inventory ID: (int)                         │
    │    └─ Item Template ID: (int)                     │
    │         │                                         │
    │         ▼                                         │
    │  [Greater (Int Int)]                              │
    │    ├─ A: Item Template ID                         │
    │    ├─ B: 0                                        │
    │    └─ Return Value: (bool)                         │
    │         │                                         │
    │         ├─ TRUE ───────────────────────────────┐  │
    │         │                                      │  │
    │         │  [Get Game Instance]                │  │
    │         │    └─ Game Instance                 │  │
    │         │         │                            │  │
    │         │         ▼                            │  │
    │         │  [Cast to Umbra Game Instance]       │  │
    │         │    └─ Success                        │  │
    │         │         │                            │  │
    │         │         ▼                            │  │
    │         │  [Unequip Item]                      │  │
    │         │    ├─ Target: Game Instance           │  │
    │         │    └─ Inventory ID: Inventory ID     │  │
    │         │                                      │  │
    │         └─ FALSE: [Unhandled]                 │  │
    │                                                   │
    └─ Default (exec) → [Unhandled] ← Outros botões
```

---

## 📊 **ESTRUTURA COMPLETA (OPÇÃO B - Equal Key Key):**

```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Get Mouse Button] ← Do MouseEvent
    └─ Return Value: (EKeys)
         │
         ▼
  [Equal (Key Key)]
    ├─ A: Return Value (EKeys) ───────────────┐
    │                                          │
    │  [EKeys] (enum)                          │
    │    └─ Left Mouse Button ────────────────┘
    │
    └─ Return Value: (bool)
         │
         ▼
      [Branch]
        ├─ Condition: (bool)
        ├─ True ───────────────────────────────┐
        │                                      │
        │  [Get Equipped Item]                 │
        │    └─ Equipped Item                  │
        │         │                            │
        │         ▼                            │
        │  [Break Umbra Inventory Slot]        │
        │    └─ Item Template ID               │
        │         │                            │
        │         ▼                            │
        │  [Greater] → Item Template ID > 0   │
        │    │                                 │
        │    ├─ TRUE:                          │
        │    │    │                            │
        │    │    ▼                            │
        │    │  [Unequip Item]                 │
        │    │                                  │
        │    └─ FALSE: [Unhandled]            │
        │                                      │
        └─ False: [Unhandled] ← Não é Left Mouse Button
```

---

## 🔍 **DETALHES IMPORTANTES:**

### **1. Get Mouse Button:**

O nó `Get Mouse Button` extrai o botão do `FPointerEvent`.

**COMO OBTER:**
1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Get Mouse Button`**
3. Selecione: **`Get Mouse Button`**

**RETORNA:**
- `Return Value` (EKeys) - O botão que foi clicado

---

### **2. Switch on EKeys vs Equal (Key Key):**

**Switch on EKeys (Recomendado):**
- ✅ Mais simples
- ✅ Mais limpo
- ✅ Fácil de adicionar outros botões (Right Mouse Button, etc.)
- ✅ Não precisa criar valores literais

**Equal (Key Key):**
- ✅ Mais flexível
- ❌ Precisa criar `EKeys` enum para comparar
- ❌ Mais nós no Blueprint

---

### **3. EKeys Enum:**

Para usar `Equal (Key Key)`, você precisa criar um valor `EKeys`:

1. Clique direito → Digite: **`EKeys`**
2. Selecione: **`EKeys`** (enum)
3. Expanda o enum
4. Procure: **`Left Mouse Button`**
5. Arraste para o pin `B` do `Equal`

**OU:**

1. Clique no pin `B` do `Equal (Key Key)`
2. No dropdown, procure: **`Left Mouse Button`**
3. Selecione

---

## ✅ **RESUMO RÁPIDO:**

### **MÉTODO 1: Switch on EKeys (RECOMENDADO)**

1. ✅ `OnMouseButtonDoubleClick` → `In Mouse Event`
2. ✅ `Get Mouse Button` → `Return Value` (EKeys)
3. ✅ `Switch on EKeys` → `Select` (conectado ao Return Value)
4. ✅ Adicionar case: `Left Mouse Button`
5. ✅ Conectar à lógica de desequipar

### **MÉTODO 2: Equal (Key Key)**

1. ✅ `OnMouseButtonDoubleClick` → `In Mouse Event`
2. ✅ `Get Mouse Button` → `Return Value` (EKeys)
3. ✅ `Equal (Key Key)` → `A` (conectado ao Return Value)
4. ✅ `EKeys` enum → `Left Mouse Button` → `B`
5. ✅ `Branch` → `Condition` (conectado ao Return Value)
6. ✅ Conectar à lógica de desequipar

---

## 🎯 **RECOMENDAÇÃO:**

**Use `Switch on EKeys`** - é mais simples, limpo e fácil de expandir para outros botões!

---

## 🎉 **PRONTO!**

Agora você sabe como obter o botão do mouse corretamente do `OnMouseButtonDoubleClick`! 🎯

