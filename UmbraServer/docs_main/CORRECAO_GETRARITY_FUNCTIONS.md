# 🔧 CORREÇÃO: Funções GetRarityText e GetRarityColor

## 🚨 **PROBLEMA COMUM**

Você criou um nó `Switch on EUmbraItemRarity` mas não consegue conectar `Make Literal String` porque:
- ❌ Switch tem pins de **EXECUÇÃO** (brancos)
- ❌ Make Literal String é um nó de **DADOS** (sem execução)

---

## ✅ **SOLUÇÃO CORRETA: USAR "SELECT"**

Em vez de usar `Switch`, use o nó **`Select`**!

---

## 📝 **FUNÇÃO: GetRarityText (MÉTODO CORRETO)**

### **PASSO 1: Criar a função**

1. **Functions** → **+ (Add Function)**
2. Nome: `GetRarityText`

---

### **PASSO 2: Configurar Input e Output**

**Input:**
- Nome: `Rarity`
- Type: `EUmbraItemRarity` (enum)

**Output:**
- Nome: `ReturnValue` (já vem por padrão)
- Type: `String`

Para adicionar o output:
1. Selecione o nó **GetRarityText** (Entry)
2. No painel **Details**, procure **Outputs**
3. Clique em **+ (Add Output)**
4. Type: `String`

---

### **PASSO 3: Adicionar nó SELECT**

1. **Clique com botão direito** no Graph
2. Digite: `Select`
3. Selecione **Select** (nó roxo)

O nó `Select` vai aparecer com:
- Pin **Index** (int) - IGNORE ESTE
- Pin **Option 0, Option 1, etc.** - vários pins de opções

---

### **PASSO 4: Configurar o SELECT para usar ENUM**

1. Clique no nó **Select**
2. No painel **Details**, procure **Index Pin Type**
3. Mude de `Integer` para `EUmbraItemRarity`

Agora o nó muda e mostra:
- Pin **Select** (tipo: EUmbraItemRarity)
- Pin **Common** (string)
- Pin **Uncommon** (string)
- Pin **Rare** (string)
- Pin **Epic** (string)
- Pin **Legendary** (string)
- Pin **Return Value** (string)

---

### **PASSO 5: Conectar os valores**

**Lógica:**

```
[GetRarityText Entry]
    │ Rarity (EUmbraItemRarity)
    │
    ▼
[Select] (Index Pin Type = EUmbraItemRarity)
    ├─ Select: Rarity
    ├─ Common: "Common"
    ├─ Uncommon: "Uncommon"
    ├─ Rare: "Rare"
    ├─ Epic: "Epic"
    ├─ Legendary: "Legendary"
    │
    └─ Return Value → [Return Node]
```

**Como fazer:**

1. **Conectar Rarity ao Select:**
   - Arraste do pin `Rarity` (Entry) → pin `Select` (do nó Select)

2. **Definir os textos:**
   - No pin **Common**, digite: `Common`
   - No pin **Uncommon**, digite: `Uncommon`
   - No pin **Rare**, digite: `Rare`
   - No pin **Epic**, digite: `Epic`
   - No pin **Legendary**, digite: `Legendary`

3. **Conectar ao Return:**
   - Arraste do pin **Return Value** do Select → pin **Return Value** do nó **Return**

---

### **VISUAL:**

```
┏━━━━━━━━━━━━━━━━━━━━┓
┃ GetRarityText Entry┃
┗━━━━━━━━━━━━━━━━━━━━┛
    │ Rarity (enum)
    │
    ▼
╔═══════════════════════════════════╗
║         Select (enum)             ║
║                                   ║
║ Select: ◄────── Rarity            ║
║                                   ║
║ Common:     "Common"              ║
║ Uncommon:   "Uncommon"            ║
║ Rare:       "Rare"                ║
║ Epic:       "Epic"                ║
║ Legendary:  "Legendary"           ║
║                                   ║
║ Return Value ─────────┐           ║
╚═══════════════════════│═══════════╝
                        │
                        ▼
              ┏━━━━━━━━━━━━━━┓
              ┃ Return Node  ┃
              ┗━━━━━━━━━━━━━━┛
```

---

## 🎨 **FUNÇÃO: GetRarityColor (MESMO MÉTODO)**

### **PASSO 1: Criar a função**

1. **Functions** → **+ (Add Function)**
2. Nome: `GetRarityColor`

---

### **PASSO 2: Configurar Input e Output**

**Input:**
- Nome: `Rarity`
- Type: `EUmbraItemRarity` (enum)

**Output:**
- Nome: `ReturnValue`
- Type: `Linear Color`

---

### **PASSO 3: Adicionar nó SELECT**

1. Clique com botão direito → `Select`
2. No painel **Details** do Select:
   - **Index Pin Type:** `EUmbraItemRarity`

---

### **PASSO 4: Mudar o tipo dos pins de opção**

1. Selecione o nó **Select**
2. No painel **Details**, procure **Option Pin Type**
3. Mude de `Wildcard` para `Linear Color`

Agora os pins mudam de cor (ficam coloridos).

---

### **PASSO 5: Definir as cores**

Para cada pin de raridade, você precisa criar um **Make Linear Color**:

#### **5.1 - Common (Cinza):**

1. Clique com botão direito → `Make Linear Color`
2. Configure os valores:
   - **R:** `0.6`
   - **G:** `0.6`
   - **B:** `0.6`
   - **A:** `1.0`
3. Conecte o resultado ao pin **Common** do Select

#### **5.2 - Uncommon (Verde):**

```
R: 0.0
G: 1.0
B: 0.0
A: 1.0
```

#### **5.3 - Rare (Azul):**

```
R: 0.0
G: 0.5
B: 1.0
A: 1.0
```

#### **5.4 - Epic (Roxo):**

```
R: 0.7
G: 0.0
B: 1.0
A: 1.0
```

#### **5.5 - Legendary (Laranja):**

```
R: 1.0
G: 0.5
B: 0.0
A: 1.0
```

---

### **PASSO 6: Conectar**

```
[GetRarityColor Entry]
    │ Rarity (enum)
    │
    ▼
[Select] (Index Pin Type = EUmbraItemRarity, Option Pin Type = Linear Color)
    ├─ Select: Rarity
    ├─ Common: [Make Linear Color] (0.6, 0.6, 0.6, 1)
    ├─ Uncommon: [Make Linear Color] (0, 1, 0, 1)
    ├─ Rare: [Make Linear Color] (0, 0.5, 1, 1)
    ├─ Epic: [Make Linear Color] (0.7, 0, 1, 1)
    ├─ Legendary: [Make Linear Color] (1, 0.5, 0, 1)
    │
    └─ Return Value → [Return Node]
```

---

### **VISUAL:**

```
┏━━━━━━━━━━━━━━━━━━━━┓
┃ GetRarityColor Entry┃
┗━━━━━━━━━━━━━━━━━━━━┛
    │ Rarity (enum)
    │
    ▼
╔═══════════════════════════════════════════════════════════╗
║                  Select (enum → Linear Color)             ║
║                                                           ║
║ Select: ◄────── Rarity                                    ║
║                                                           ║
║ Common: ◄────── [Make Linear Color] (0.6, 0.6, 0.6, 1)   ║
║ Uncommon: ◄──── [Make Linear Color] (0, 1, 0, 1)         ║
║ Rare: ◄──────── [Make Linear Color] (0, 0.5, 1, 1)       ║
║ Epic: ◄──────── [Make Linear Color] (0.7, 0, 1, 1)       ║
║ Legendary: ◄─── [Make Linear Color] (1, 0.5, 0, 1)       ║
║                                                           ║
║ Return Value ─────────────┐                               ║
╚═══════════════════════════│═══════════════════════════════╝
                            │
                            ▼
                  ┏━━━━━━━━━━━━━━┓
                  ┃ Return Node  ┃
                  ┗━━━━━━━━━━━━━━┛
```

---

## 🔄 **SE VOCÊ JÁ CRIOU COM SWITCH (ALTERNATIVA)**

Se você já tem o `Switch on EUmbraItemRarity` e não quer deletar, aqui está como fazer funcionar:

### **Estrutura com Switch (mais complexa):**

```
[GetRarityText Entry]
    │ Rarity
    │ (exec) ──┐
    │          │
    │          ▼
    │    ╔══════════════════════════╗
    │    ║ Switch on EUmbraItemRarity║
    │    ╚══════════════════════════╝
    │          │ Index: Rarity
    │          │
    │          ├─ Common ──> [Set ReturnValue] = "Common" ──┐
    │          ├─ Uncommon ─> [Set ReturnValue] = "Uncommon" ─┤
    │          ├─ Rare ─────> [Set ReturnValue] = "Rare" ────┤
    │          ├─ Epic ─────> [Set ReturnValue] = "Epic" ────┤
    │          └─ Legendary ─> [Set ReturnValue] = "Legendary"─┤
    │                                                          │
    └──────────────────────────────────────────────────────────┘
                                │
                                ▼
                          [Return Node]
```

**Como fazer:**

1. **Criar variável local:**
   - No painel **Local Variables** da função
   - Adicione variável: `ReturnValue` (String)

2. **Conectar execução:**
   - Entry (exec) → Switch (exec)

3. **Para cada saída do Switch:**
   - Common → `Set ReturnValue` → valor: `"Common"` → conectar à sequência final
   - Uncommon → `Set ReturnValue` → valor: `"Uncommon"` → conectar à sequência final
   - (repita para todas)

4. **Merge executions:**
   - Use vários nós **Sequence** ou conecte todos ao Return

**⚠️ ATENÇÃO:** Esta abordagem é **MUITO MAIS COMPLEXA** e não recomendada!

---

## 💡 **MÉTODO RECOMENDADO: DELETE O SWITCH, USE SELECT**

O nó **Select** foi feito exatamente para isso:
- ✅ Mais simples
- ✅ Mais limpo visualmente
- ✅ Sem necessidade de execução
- ✅ Pure function (melhor performance)

**Para deletar o Switch:**
1. Selecione o nó `Switch on EUmbraItemRarity`
2. Aperte **Delete**
3. Crie o `Select` como explicado acima

---

## 🧪 **TESTAR AS FUNÇÕES**

### **Teste GetRarityText:**

No Event Graph do `WBP_ItemTooltip`:

```
[Event Construct]
    │
    └─> [GetRarityText]
            ├─ Rarity: Common (ou Rare, Epic, etc.)
            │
            └─ Return Value → [Print String]
```

**Resultado esperado:**
```
"Common"
```

---

### **Teste GetRarityColor:**

```
[Event Construct]
    │
    └─> [GetRarityColor]
            ├─ Rarity: Rare
            │
            └─ Return Value → [Print String] (converter para string)
```

**Resultado esperado:**
```
"R=0.000000,G=0.500000,B=1.000000,A=1.000000"
```

---

## 📋 **CHECKLIST**

### **GetRarityText:**
- [ ] Função criada
- [ ] Input `Rarity` (EUmbraItemRarity)
- [ ] Output `ReturnValue` (String)
- [ ] Nó **Select** adicionado
- [ ] Index Pin Type = `EUmbraItemRarity`
- [ ] Todas as 5 raridades têm texto definido
- [ ] Conectado ao Return Node
- [ ] Compila sem erros
- [ ] Testado

### **GetRarityColor:**
- [ ] Função criada
- [ ] Input `Rarity` (EUmbraItemRarity)
- [ ] Output `ReturnValue` (Linear Color)
- [ ] Nó **Select** adicionado
- [ ] Index Pin Type = `EUmbraItemRarity`
- [ ] Option Pin Type = `Linear Color`
- [ ] 5 nós **Make Linear Color** criados
- [ ] Cores corretas para cada raridade
- [ ] Conectado ao Return Node
- [ ] Compila sem erros
- [ ] Testado

---

## 🎨 **CORES DAS RARIDADES (REFERÊNCIA)**

| Raridade | Cor | RGB |
|----------|-----|-----|
| Common | Cinza | `(0.6, 0.6, 0.6, 1)` |
| Uncommon | Verde | `(0, 1, 0, 1)` |
| Rare | Azul | `(0, 0.5, 1, 1)` |
| Epic | Roxo | `(0.7, 0, 1, 1)` |
| Legendary | Laranja | `(1, 0.5, 0, 1)` |

---

## 🚨 **ERRO COMUM: "Cannot find EUmbraItemRarity"**

Se o enum não aparece ao configurar o Select:

1. Verifique se `EUmbraItemRarity` está definido no C++:
   - Arquivo: `UmbraDataStructures.h`
   - Deve ter:
     ```cpp
     UENUM(BlueprintType)
     enum class EUmbraItemRarity : uint8
     {
         Common      UMETA(DisplayName = "Common"),
         Uncommon    UMETA(DisplayName = "Uncommon"),
         Rare        UMETA(DisplayName = "Rare"),
         Epic        UMETA(DisplayName = "Epic"),
         Legendary   UMETA(DisplayName = "Legendary")
     };
     ```

2. Recompile o C++
3. Reinicie o Unreal Editor

---

## 📝 **RESUMO**

✅ **USE SELECT, NÃO SWITCH**
- Select = nó de dados (pure function)
- Switch = nó de execução (não funciona para retornar valores simples)

✅ **CONFIGURE O SELECT:**
1. Index Pin Type = `EUmbraItemRarity`
2. Option Pin Type = `String` (para GetRarityText) ou `Linear Color` (para GetRarityColor)
3. Conecte valores a cada opção
4. Conecte Return Value ao Return Node

---

**Isso vai funcionar perfeitamente! 🚀**

