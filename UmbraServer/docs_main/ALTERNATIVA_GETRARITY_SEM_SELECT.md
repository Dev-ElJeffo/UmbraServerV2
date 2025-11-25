# 🔧 ALTERNATIVA: GetRarityText e GetRarityColor SEM SELECT

## 🚨 **PROBLEMA**

O painel Details do nó `Select` não mostra "Index Pin Type", então não consegue configurar para usar enum.

---

## ✅ **SOLUÇÃO ALTERNATIVA 1: USAR SWITCH + VARIÁVEL LOCAL**

Esta é a solução que **REALMENTE FUNCIONA** com Switch!

---

## 📝 **FUNÇÃO: GetRarityText (COM SWITCH)**

### **PASSO 1: Criar a função**

1. **Functions** → **+ (Add Function)**
2. Nome: `GetRarityText`

---

### **PASSO 2: Configurar Input e Output**

**No painel Details do Entry Node:**

**Inputs:**
- Nome: `Rarity`
- Type: `EUmbraItemRarity`

**Outputs:**
- Clique em **+ Add Output**
- Type: `String`
- Nome: `Return Value` (já vem por padrão)

---

### **PASSO 3: Criar Variável Local**

**Na aba "My Blueprint" → Local Variables (dentro da função):**

1. Clique em **+ (Add Local Variable)**
2. Nome: `Result`
3. Type: `String`
4. Default Value: `""` (vazio)

---

### **PASSO 4: Adicionar nó Switch**

1. **Clique com botão direito** no Graph
2. Digite: `Switch on EUmbraItemRarity`
3. Selecione o nó

---

### **PASSO 5: Conectar o fluxo**

**Estrutura:**

```
[GetRarityText Entry]
    │ (exec out - pin branco)
    │ Rarity (enum out)
    │
    ├─> (exec) → [Switch on EUmbraItemRarity]
    │                │ Index: ← Rarity
    │                │
    │                ├─ Common (exec) ──────────────────┐
    │                ├─ Uncommon (exec) ─────────────┐  │
    │                ├─ Rare (exec) ──────────────┐  │  │
    │                ├─ Epic (exec) ───────────┐  │  │  │
    │                └─ Legendary (exec) ────┐  │  │  │  │
    │                                        │  │  │  │  │
    │                                        │  │  │  │  │
    │     ┌──────────────────────────────────┘  │  │  │  │
    │     │  ┌───────────────────────────────────┘  │  │  │
    │     │  │  ┌──────────────────────────────────────┘  │  │
    │     │  │  │  ┌─────────────────────────────────────────┘  │
    │     │  │  │  │  ┌──────────────────────────────────────────┘
    │     │  │  │  │  │
    │     ▼  ▼  ▼  ▼  ▼
    │   [Set Result]   (variável local)
    │        │ Result = "Legendary"
    │        │ Result = "Epic"
    │        │ Result = "Rare"
    │        │ Result = "Uncommon"
    │        │ Result = "Common"
    │        │
    │        └────────┬──────── (exec out de cada Set)
    │                 │
    │                 └──> [Return Node]
    │                          │ Return Value ← Result (variável)
    └─────────────────────────────────────────────────┘
```

---

### **PASSO 6: Implementar cada case**

**Para COMMON:**

1. Do pin de execução **Common** do Switch
2. Arraste para o espaço vazio
3. Digite: `Set Result` (sua variável local)
4. No valor, digite: `Common`
5. Do pin de saída (exec) do `Set Result`, conecte ao **Return Node**

**Repita para cada raridade:**

- **Uncommon:** `Set Result` = `"Uncommon"`
- **Rare:** `Set Result` = `"Rare"`
- **Epic:** `Set Result` = `"Epic"`
- **Legendary:** `Set Result` = `"Legendary"`

---

### **PASSO 7: Conectar Return Value**

No **Return Node:**
1. Arraste da variável **Result** (Get Result)
2. Conecte ao pin **Return Value** do Return Node

---

### **VISUAL COMPLETO:**

```
┏━━━━━━━━━━━━━━━━━━━━━━┓
┃ GetRarityText Entry  ┃
┗━━━━━━━━━━━━━━━━━━━━━━┛
    │ (exec)
    │ Rarity ────────────────┐
    │                        │
    ▼                        │
╔════════════════════════════│═══════╗
║ Switch on EUmbraItemRarity │       ║
║                            │       ║
║ (exec in)                  ▼       ║
║ Index ◄─────────────── Rarity      ║
╚════════════════════════════════════╝
    │
    ├─ Common ───> [Set Result] = "Common" ──────┐
    │                                             │
    ├─ Uncommon ─> [Set Result] = "Uncommon" ────┤
    │                                             │
    ├─ Rare ─────> [Set Result] = "Rare" ────────┤
    │                                             │
    ├─ Epic ─────> [Set Result] = "Epic" ────────┤
    │                                             │
    └─ Legendary ─> [Set Result] = "Legendary" ───┤
                                                  │
                                                  │ (todos convergem)
                                                  ▼
                                    ┏━━━━━━━━━━━━━━━━━━━┓
                                    ┃   Return Node     ┃
                                    ┃                   ┃
                                    ┃ Return Value ◄───── [Get Result]
                                    ┗━━━━━━━━━━━━━━━━━━━┛
```

---

## 🎨 **FUNÇÃO: GetRarityColor (COM SWITCH)**

### **PASSO 1-2: Igual ao GetRarityText**

- Função: `GetRarityColor`
- Input: `Rarity` (EUmbraItemRarity)
- Output: `Return Value` (Linear Color)

---

### **PASSO 3: Criar Variável Local**

**Local Variables:**
- Nome: `ResultColor`
- Type: `Linear Color`
- Default: `(0, 0, 0, 1)` (preto)

---

### **PASSO 4-5: Switch**

Igual ao anterior, mas agora:

```
[Switch on EUmbraItemRarity]
    │
    ├─ Common ───> [Make Linear Color] (0.6, 0.6, 0.6, 1) → [Set ResultColor]
    │
    ├─ Uncommon ─> [Make Linear Color] (0, 1, 0, 1) → [Set ResultColor]
    │
    ├─ Rare ─────> [Make Linear Color] (0, 0.5, 1, 1) → [Set ResultColor]
    │
    ├─ Epic ─────> [Make Linear Color] (0.7, 0, 1, 1) → [Set ResultColor]
    │
    └─ Legendary ─> [Make Linear Color] (1, 0.5, 0, 1) → [Set ResultColor]
                                                │
                                                └──> [Return Node]
                                                      └─ Return Value ← ResultColor
```

---

## ✅ **SOLUÇÃO ALTERNATIVA 2: USAR BRANCH (IF-ELSE)**

Se nem Switch funcionar, use **Branch** (If-Else) manual!

---

## 📝 **FUNÇÃO: GetRarityText (COM BRANCH)**

**Estrutura:**

```
[Entry] Rarity
    │
    ├─> [Equal (Enum)] Rarity == Common?
    │       │
    │       ├─ TRUE → Return "Common"
    │       │
    │       └─ FALSE → [Equal (Enum)] Rarity == Uncommon?
    │                       │
    │                       ├─ TRUE → Return "Uncommon"
    │                       │
    │                       └─ FALSE → [Equal (Enum)] Rarity == Rare?
    │                                   (continua...)
```

---

### **PASSO A PASSO:**

1. **Comparar Rarity == Common:**
   - Arraste do pin `Rarity`
   - Digite: `Equal (Enum)`
   - No segundo pin, selecione: `Common` (do dropdown)

2. **Branch:**
   - Arraste do resultado do `Equal`
   - Digite: `Branch`

3. **TRUE (é Common):**
   - Do pin `True` do Branch → `Return Node`
   - Conecte `"Common"` (Make Literal String) ao `Return Value`

4. **FALSE (não é Common):**
   - Do pin `False` do Branch → repita para `Uncommon`, `Rare`, etc.

---

### **VISUAL:**

```
[Entry] Rarity
    │
    ▼
[==] Rarity == Common?
    │
    ▼
[Branch]
    ├─ TRUE ──> [Return] "Common"
    │
    └─ FALSE ──> [==] Rarity == Uncommon?
                     │
                     ▼
                 [Branch]
                     ├─ TRUE ──> [Return] "Uncommon"
                     │
                     └─ FALSE ──> [==] Rarity == Rare?
                                      │
                                      ▼
                                  [Branch]
                                      ├─ TRUE ──> [Return] "Rare"
                                      │
                                      └─ FALSE ──> [==] Rarity == Epic?
                                                       │
                                                       ▼
                                                   [Branch]
                                                       ├─ TRUE ──> [Return] "Epic"
                                                       │
                                                       └─ FALSE ──> [Return] "Legendary"
```

---

## 🎯 **MÉTODO RECOMENDADO: SWITCH + VARIÁVEL LOCAL**

**POR QUÊ:**
- ✅ Funciona em TODAS as versões do Unreal
- ✅ Mais limpo que Branch aninhado
- ✅ Fácil de debugar

**DESVANTAGENS:**
- ❌ Mais nós que Select
- ❌ Precisa convergir todas as execuções

---

## 🔧 **DICA: CONVERGIR EXECUÇÕES**

Para unir todas as saídas do Switch no Return Node, você pode:

### **Opção 1: Conectar diretamente**
- Cada `Set Result` conecta direto ao Return Node

### **Opção 2: Usar nó Sequence (inverso)**
- Junte todas as execuções em um único ponto
- Não existe "merge execution" nativo, então pode deixar múltiplas linhas chegando no Return

**O Unreal Engine aceita múltiplas conexões de execução chegando no mesmo nó!**

---

## 🧪 **TESTAR**

No Event Graph:

```
[Event Construct]
    │
    └─> [GetRarityText]
            ├─ Rarity: Rare
            │
            └─ Return Value → [Print String]
```

**Resultado esperado:** `"Rare"`

---

## 📊 **COMPARAÇÃO DOS MÉTODOS**

| Método | Complexidade | Compatibilidade | Visual |
|--------|--------------|-----------------|--------|
| **Select** | ⭐⭐⭐⭐⭐ Fácil | Algumas versões | Limpo |
| **Switch + Variável** | ⭐⭐⭐⭐ Médio | ✅ Todas | OK |
| **Branch (If-Else)** | ⭐⭐ Difícil | ✅ Todas | Poluído |

---

## 📝 **CHECKLIST - SWITCH + VARIÁVEL**

### **GetRarityText:**
- [ ] Função criada
- [ ] Input `Rarity` (EUmbraItemRarity)
- [ ] Output `Return Value` (String)
- [ ] Variável local `Result` (String)
- [ ] Nó `Switch on EUmbraItemRarity`
- [ ] Conectado Entry (exec) → Switch (exec)
- [ ] Conectado Entry (Rarity) → Switch (Index)
- [ ] 5 nós `Set Result` (um para cada case)
- [ ] Cada `Set Result` com o texto correto
- [ ] Todos conectam ao Return Node
- [ ] `Get Result` → Return Value do Return Node
- [ ] Compila sem erros
- [ ] Testado

### **GetRarityColor:**
- [ ] Função criada
- [ ] Input `Rarity` (EUmbraItemRarity)
- [ ] Output `Return Value` (Linear Color)
- [ ] Variável local `ResultColor` (Linear Color)
- [ ] Nó `Switch on EUmbraItemRarity`
- [ ] 5 nós `Make Linear Color` (um para cada case)
- [ ] 5 nós `Set ResultColor`
- [ ] Cores corretas para cada raridade
- [ ] Todos conectam ao Return Node
- [ ] `Get ResultColor` → Return Value do Return Node
- [ ] Compila sem erros
- [ ] Testado

---

## 🎨 **EXEMPLO COMPLETO - GETRARITY TEXT**

```blueprint
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃     GetRarityText Entry        ┃
┃                                ┃
┃ (exec out) ──────┐             ┃
┃ Rarity (out) ────┼─────┐       ┃
┗━━━━━━━━━━━━━━━━━━│━━━━━│━━━━━━━┛
                   │     │
                   ▼     │
    ╔══════════════════════│══════════════╗
    ║ Switch on            │              ║
    ║ EUmbraItemRarity     │              ║
    ║                      ▼              ║
    ║ (exec in)       Index (Rarity)      ║
    ╚═════════════════════════════════════╝
         │
         ├─ Common ──> [Set Result] "Common" ───┐
         │                                       │
         ├─ Uncommon ─> [Set Result] "Uncommon" ─┤
         │                                       │
         ├─ Rare ────> [Set Result] "Rare" ─────┤
         │                                       │
         ├─ Epic ────> [Set Result] "Epic" ─────┤
         │                                       │
         └─ Legendary ─> [Set Result] "Legendary"┤
                                                 │
                                                 ▼
                                    ┌────────────────────┐
                                    │ Variável: Result   │
                                    │ (String)           │
                                    └────────┬───────────┘
                                             │ Get
                                             ▼
                                    ┏━━━━━━━━━━━━━━━━━┓
                                    ┃  Return Node    ┃
                                    ┃                 ┃
                                    ┃ Return Value ◄──┘
                                    ┗━━━━━━━━━━━━━━━━━┛
```

---

## 💡 **DICA FINAL**

Se você não conseguir usar Select, **USE SWITCH + VARIÁVEL LOCAL**.

É um pouco mais trabalhoso, mas **FUNCIONA 100%** em todas as versões do Unreal Engine!

---

**Boa sorte! 🚀**

