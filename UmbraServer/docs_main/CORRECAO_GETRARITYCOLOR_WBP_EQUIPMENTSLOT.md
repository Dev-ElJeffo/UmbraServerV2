# 🔧 CORREÇÃO: GetRarityColor no WBP_EquipmentSlot

## 🐛 **PROBLEMA:**

A função `GetRarityColor` está no `WBP_ItemTooltip` e pede um **Target**, mas no `WBP_EquipmentSlot` não faz sentido ter uma referência ao `WBP_ItemTooltip` apenas para obter a cor.

---

## ✅ **SOLUÇÃO:**

**Criar a função `GetRarityColor` diretamente no `WBP_EquipmentSlot`.**

Isso é melhor porque:
- ✅ Não precisa de target/referência externa
- ✅ Função pura (não precisa de estado)
- ✅ Pode ser reutilizada dentro do próprio widget
- ✅ Mais simples e direto

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Criar Função GetRarityColor**

1. **Abra o `WBP_EquipmentSlot`**
2. **Functions** → **+ (Add Function)**
3. Nome: `GetRarityColor`
4. **Output:** `ReturnValue` (type: `Linear Color`)
5. **Marque como Pure:**
   - Selecione o **Entry Node** da função (nó inicial)
   - No painel **Details**, procure **Pure** ou **Pure Function**
   - **Marque como Pure** (checkbox)
   - Isso remove o pin "then" (exec) - não é necessário!

**IMPORTANTE:** Funções puras não têm pins de execução. Elas são chamadas diretamente pelos valores.

### **PASSO 1.5: ADICIONAR INPUT RARITY (MUITO IMPORTANTE!)**

**APÓS criar a função, você DEVE adicionar o input:**

1. No painel **Details** (lado direito), procure a seção **Inputs**
2. Clique em **+ (Add Input)**
3. Configure:
   - **Name:** `Rarity`
   - **Type:** `EUmbraItemRarity` (enum)
   - **Default Value:** (deixe vazio ou `Common`)

**IMPORTANTE:** O input `Rarity` deve aparecer no **Entry Node** da função (o nó inicial, à esquerda).

**SE NÃO ADICIONAR O INPUT, VOCÊ NÃO CONSEGUIRÁ CONECTAR O RARITY!**

---

### **PASSO 2: Adicionar Nó Select**

1. Clique com botão direito no Graph da função
2. Digite: `Select`
3. Selecione o nó **Select**

---

### **PASSO 3: Configurar Select**

1. Selecione o nó **Select**
2. No painel **Details**:
   - **Index Pin Type:** `EUmbraItemRarity`
   - **Option Pin Type:** `Linear Color`

Agora o Select terá 5 pins de saída (um para cada raridade).

---

### **PASSO 4: Conectar Rarity ao Select**

**IMPORTANTE:** 
- O Select tem um pin de **ENTRADA** chamado **"Index"** na **ESQUERDA** do nó
- Você precisa conectar o pin **Rarity** (do Entry Node da função) ao pin **"Index"** do Select

**COMO FAZER:**
1. No **Entry Node** da função (nó inicial à esquerda), você verá o pin **Rarity** (input)
2. **Arraste** o pin **Rarity** → **Pin "Index"** do nó Select
   - O pin "Index" está na **ESQUERDA** do Select (seta apontando para dentro)
   - É o pin de **ENTRADA** do Select

**SE O PIN RARITY NÃO APARECER NO ENTRY NODE:**
- Você esqueceu de adicionar o Input na função!
- Volte ao **PASSO 1.5** e adicione o input `Rarity`

**VISUAL:**
```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity) ──────────┐
                                          │
                                          ▼
                                    [Select]
                                      ↑
                                      │
                            Pin "Select" (ENTRADA) ← Conecte aqui!
```

---

### **PASSO 5: Criar Make Linear Color para Cada Raridade**

Para cada pin de raridade no Select, crie um nó **Make Linear Color**:

#### **5.1 - Common (Cinza):**
1. Clique com botão direito → `Make Linear Color`
2. Configure:
   - **R:** `0.6`
   - **G:** `0.6`
   - **B:** `0.6`
   - **A:** `1.0`
3. Conecte ao pin **Common** do Select

#### **5.2 - Uncommon (Verde):**
- **R:** `0.0`
- **G:** `1.0`
- **B:** `0.0`
- **A:** `1.0`

#### **5.3 - Rare (Azul):**
- **R:** `0.0`
- **G:** `0.5`
- **B:** `1.0`
- **A:** `1.0`

#### **5.4 - Epic (Roxo):**
- **R:** `0.7`
- **G:** `0.0`
- **B:** `1.0`
- **A:** `1.0`

#### **5.5 - Legendary (Laranja):**
- **R:** `1.0`
- **G:** `0.5`
- **B:** `0.0`
- **A:** `1.0`

---

### **PASSO 6: Conectar ao Return Node**

1. Conecte a saída do **Select** ao pin **Return Value** do **Return Node**

---

## 📊 **ESTRUTURA FINAL:**

```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity)
       │
       ▼
[Select]
  ├─ Index Pin Type: EUmbraItemRarity
  ├─ Option Pin Type: Linear Color
  ├─ Select: Rarity ← Conectado
  │
  ├─ Common: [Make Linear Color] (0.6, 0.6, 0.6, 1.0) ← Conectado
  ├─ Uncommon: [Make Linear Color] (0.0, 1.0, 0.0, 1.0) ← Conectado
  ├─ Rare: [Make Linear Color] (0.0, 0.5, 1.0, 1.0) ← Conectado
  ├─ Epic: [Make Linear Color] (0.7, 0.0, 1.0, 1.0) ← Conectado
  └─ Legendary: [Make Linear Color] (1.0, 0.5, 0.0, 1.0) ← Conectado
       │
       ▼
[Return Node]
  └─ Return Value: Select Output ← Conectado
```

---

## 🔧 **USAR NA FUNÇÃO Update Slot Visual:**

Agora você pode usar a função sem precisar de target:

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
[Get Rarity Color] ← Função do próprio widget
  └─ Rarity: Rarity (do Break Umbra Item Template) ← CONECTE AQUI!
  └─ Return Value: (Linear Color) ← NÃO PRECISA DE TARGET!
       ↓
[Set Brush Color] → Border_Slot
  └─ Color: Return Value
```

**IMPORTANTE:** 
- O pin **Rarity** do `Break Umbra Item Template` deve ser conectado ao pin **Rarity** (input) da função `GetRarityColor`
- Se o pin não aparecer, você esqueceu de adicionar o Input na função!

---

## ✅ **RESULTADO:**

- ✅ Função `GetRarityColor` criada no `WBP_EquipmentSlot`
- ✅ Não precisa de target
- ✅ Função pura (melhor performance)
- ✅ **Sem pin de execução** (não precisa conectar "then")
- ✅ Pode ser usada diretamente em `Update Slot Visual`

---

## ⚠️ **SOBRE O EXEC PIN:**

**Se a função tiver um pin "then" (exec):**
- ✅ **Pode deixar desconectado** - funciona normalmente
- ✅ A função retorna o valor mesmo sem exec pin conectado
- ✅ **Mas o ideal é marcar como Pure** para remover o exec pin

**Funções puras são melhores porque:**
- ✅ Mais simples (sem gerenciar exec pins)
- ✅ Melhor performance
- ✅ Podem ser usadas em qualquer lugar (até em cálculos)

---

## 🎨 **CORES DAS RARIDADES (REFERÊNCIA):**

| Raridade | Cor | RGB |
|----------|-----|-----|
| Common | Cinza | `(0.6, 0.6, 0.6, 1)` |
| Uncommon | Verde | `(0, 1, 0, 1)` |
| Rare | Azul | `(0, 0.5, 1, 1)` |
| Epic | Roxo | `(0.7, 0, 1, 1)` |
| Legendary | Laranja | `(1, 0.5, 0, 1)` |

---

## ✅ **PRONTO!**

Agora você pode usar `GetRarityColor` no `Update Slot Visual` sem precisar de target!

