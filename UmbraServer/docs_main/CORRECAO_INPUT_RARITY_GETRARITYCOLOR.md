# 🔧 CORREÇÃO: Adicionar Input Rarity na Função GetRarityColor

## 🐛 **PROBLEMA:**

A função `GetRarityColor` não tem um **INPUT** chamado `Rarity`. Por isso você não consegue conectar o Rarity do `Break Umbra Item Template` à função.

---

## ✅ **SOLUÇÃO:**

**Adicionar o INPUT `Rarity` na função `GetRarityColor`.**

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Abrir a Função GetRarityColor**

1. Abra o `WBP_EquipmentSlot`
2. Vá para **Functions**
3. Selecione a função **GetRarityColor**

---

### **PASSO 2: Adicionar Input Rarity**

1. No painel **Details** (lado direito), procure a seção **Inputs**
2. Clique em **+ (Add Input)**
3. Configure:
   - **Name:** `Rarity`
   - **Type:** `EUmbraItemRarity` (enum)
   - **Default Value:** (deixe vazio ou `Common`)

**IMPORTANTE:** O input deve aparecer no **Entry Node** da função (o nó inicial).

---

### **PASSO 3: Conectar Rarity ao Select**

Agora você pode conectar:

1. **Arraste** o pin **Rarity** (do Entry Node da função) → **Pin "Index"** do nó Select
   - O pin "Index" está na **ESQUERDA** do Select
   - É o pin de **ENTRADA** do Select

**VISUAL:**
```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity) ──────────┐
                                          │
                                          ▼
                                    [Select]
                                      ↑
                                      │
                            Pin "Index" (ENTRADA) ← Conecte aqui!
```

---

### **PASSO 4: Usar a Função no Update Slot Visual**

Agora você pode usar a função corretamente:

```
[Update Slot Visual]
  └─ Item Slot: (FUmbraInventorySlot)
       ↓
[Break Umbra Inventory Slot]
  └─ Item Slot: Item Slot
       ↓
[Get Item Template]
  └─ Item Template: Item Template
       ↓
[Break Umbra Item Template]
  └─ Item Template: Item Template
       ↓
[Get Rarity Color] ← Função
  └─ Rarity: Rarity (do Break Umbra Item Template) ← Conecte aqui!
  └─ Return Value: (Linear Color)
       ↓
[Set Brush Color] → Border_Slot
  └─ Color: Return Value
```

---

## 🔍 **VERIFICAÇÃO:**

### **A Função Deve Ter:**

1. ✅ **Entry Node** com:
   - Pin de execução "then" (saída)
   - Pin de input **"Rarity"** (EUmbraItemRarity)

2. ✅ **Select Node** com:
   - Pin "Index" conectado ao pin "Rarity" do Entry Node
   - 5 pins de saída (Common, Uncommon, Rare, Epic, Legendary)
   - Pin "ReturnValue" (saída final)

3. ✅ **Return Node** com:
   - Pin "Return Value" conectado ao "ReturnValue" do Select

---

## 📊 **ESTRUTURA CORRETA:**

```
[GetRarityColor Entry]
  ├─ then (exec)
  └─ Rarity: (EUmbraItemRarity) ──────────┐
                                          │
                                          ▼
                                    [Select]
                                      ↑
                                      │
                            Pin "Index" ← Conectado!
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    ▼                 ▼                 ▼
        [Make Linear Color]  [Make Linear Color]  [Make Linear Color]
        (Common)              (Uncommon)          (Rare)
                    │                 │                 │
                    └─────────────────┼─────────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    ▼                 ▼                 ▼
        [Make Linear Color]  [Make Linear Color]
        (Epic)               (Legendary)
                    │                 │
                    └─────────────────┘
                                      │
                                      ▼
                            [Select] ReturnValue
                                      │
                                      ▼
                            [Return Node]
                              └─ Return Value
```

---

## ⚠️ **IMPORTANTE:**

### **Se a Função Não Tem Input:**

1. **Delete** a função atual
2. **Recrie** a função seguindo os passos:
   - Criar função `GetRarityColor`
   - **ADICIONAR INPUT** `Rarity` (EUmbraItemRarity) **ANTES** de criar o Select
   - Depois criar o Select e conectar

### **Ordem Correta:**

1. ✅ Criar função `GetRarityColor`
2. ✅ **Adicionar Input `Rarity`** ← IMPORTANTE!
3. ✅ Criar nó Select
4. ✅ Configurar Select (Index Pin Type, Option Pin Type)
5. ✅ Conectar Rarity → Select Index
6. ✅ Criar Make Linear Color para cada raridade
7. ✅ Conectar Make Linear Color → Select pins
8. ✅ Conectar Select ReturnValue → Return Node

---

## ✅ **PRONTO!**

Após adicionar o input `Rarity`, você poderá:
- ✅ Conectar o Rarity do Break Umbra Item Template à função
- ✅ Usar a função no Update Slot Visual
- ✅ Obter a cor correta baseada na raridade

