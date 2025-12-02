# 🔧 COMO CONECTAR SELECT PARA GETRARITYCOLOR

## 🎯 **ENTENDENDO O NÓ SELECT:**

O nó **Select** tem:
- ✅ **1 pin de ENTRADA:** "Select" (ou "Index") - recebe o valor que será usado para selecionar
- ✅ **Vários pins de SAÍDA:** Um para cada opção (Common, Uncommon, Rare, Epic, Legendary)
- ✅ **1 pin de SAÍDA FINAL:** Retorna o valor selecionado

---

## 📋 **PASSO A PASSO CORRETO:**

### **PASSO 1: Criar a Função GetRarityColor**

1. **Functions** → **+ (Add Function)**
2. Nome: `GetRarityColor`
3. **Input:** `Rarity` (type: `EUmbraItemRarity`)
4. **Output:** `ReturnValue` (type: `Linear Color`)
5. Marque como **Pure**

---

### **PASSO 2: Adicionar Nó Select**

1. Clique com botão direito no Graph da função
2. Digite: `Select`
3. Selecione o nó **Select**

**IMPORTANTE:** O Select aparecerá com apenas um pin de entrada chamado **"Select"** (ou "Index").

---

### **PASSO 3: Configurar Select no Details**

1. Selecione o nó **Select**
2. No painel **Details** (lado direito):
   - **Index Pin Type:** `EUmbraItemRarity` ← MUDAR AQUI
   - **Option Pin Type:** `Linear Color` ← MUDAR AQUI

**APÓS CONFIGURAR:**
- O pin de entrada "Select" aceitará `EUmbraItemRarity`
- Aparecerão 5 pins de saída (Common, Uncommon, Rare, Epic, Legendary)
- Aparecerá um pin de saída final (Return Value)

---

### **PASSO 4: CONECTAR RARITY AO SELECT**

**ESTE É O PASSO IMPORTANTE:**

1. **Arraste** o pin **Rarity** (input da função) → **Pin "Select"** do nó Select
   - O pin "Select" está na **ESQUERDA** do nó Select
   - É o pin de **ENTRADA** (seta apontando para dentro)

**VISUAL:**
```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity) ──────────┐
                                          │
                                          ▼
                                    [Select]
                                      ↑
                                      │
                            Pin "Select" (ENTRADA)
```

---

### **PASSO 5: Criar Make Linear Color para Cada Raridade**

Para cada pin de saída do Select (Common, Uncommon, Rare, Epic, Legendary):

1. Clique com botão direito → `Make Linear Color`
2. Configure os valores RGB
3. Conecte a **SAÍDA** do `Make Linear Color` ao pin correspondente do Select

**EXEMPLO PARA COMMON:**
```
[Make Linear Color]
  ├─ R: 0.6
  ├─ G: 0.6
  ├─ B: 0.6
  ├─ A: 1.0
  └─ Output (Linear Color) ──→ [Select] ← Pin "Common"
```

**REPITA PARA TODAS AS 5 RARIDADES:**
- Common → Pin "Common" do Select
- Uncommon → Pin "Uncommon" do Select
- Rare → Pin "Rare" do Select
- Epic → Pin "Epic" do Select
- Legendary → Pin "Legendary" do Select

---

### **PASSO 6: Conectar Select ao Return Node**

1. Conecte o pin de **SAÍDA FINAL** do Select (Return Value) ao pin **Return Value** do **Return Node**

**VISUAL:**
```
[Select]
  └─ Return Value (Linear Color) ──→ [Return Node]
                                        └─ Return Value
```

---

## 📊 **ESTRUTURA COMPLETA:**

```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity) ──────────┐
                                          │
                                          ▼
                                    [Select]
                                      ↑
                                      │
                            Pin "Select" (ENTRADA) ← Conectado aqui!
                                      │
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
                            [Select] Return Value
                                      │
                                      ▼
                            [Return Node]
                              └─ Return Value
```

---

## 🔍 **DETALHES IMPORTANTES:**

### **Pin de ENTRADA do Select:**
- Nome: **"Select"** ou **"Index"**
- Tipo: `EUmbraItemRarity` (após configurar)
- Localização: **ESQUERDA** do nó (seta apontando para dentro)
- **CONECTE AQUI:** O pin `Rarity` da função

### **Pins de SAÍDA do Select:**
- Common, Uncommon, Rare, Epic, Legendary
- Localização: **DIREITA** do nó (setas apontando para fora)
- **CONECTE AQUI:** Os `Make Linear Color`

### **Pin de SAÍDA FINAL do Select:**
- Nome: **"Return Value"** ou apenas a saída final
- Tipo: `Linear Color`
- Localização: **DIREITA** do nó (seta apontando para fora)
- **CONECTE AQUI:** O pin `Return Value` do Return Node

---

## ✅ **RESUMO:**

1. ✅ **Rarity** (input da função) → **Pin "Select"** do Select (ENTRADA)
2. ✅ **Make Linear Color** (cada um) → **Pin correspondente** do Select (Common, Uncommon, etc.)
3. ✅ **Select Return Value** → **Return Node Return Value** (SAÍDA)

---

## 🎨 **CORES DAS RARIDADES:**

| Raridade | RGB |
|----------|-----|
| Common | `(0.6, 0.6, 0.6, 1)` |
| Uncommon | `(0, 1, 0, 1)` |
| Rare | `(0, 0.5, 1, 1)` |
| Epic | `(0.7, 0, 1, 1)` |
| Legendary | `(1, 0.5, 0, 1)` |

---

## ✅ **PRONTO!**

Agora você sabe como conectar corretamente o Select!

