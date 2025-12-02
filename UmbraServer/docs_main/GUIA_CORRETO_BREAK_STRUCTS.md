# GUIA CORRETO: COMO QUEBRAR OS STRUCTS NO BLUEPRINT

## ✅ ESTRUTURA CORRETA

Você tem:
- `Break ItemTemplate` → saída `Stats` (tipo `FUmbraItemStats`)
- Precisa quebrar esse `Stats` para obter `Base`, `Combat`, `Bonus`, `Legacy`

---

## 📋 PASSO A PASSO CORRETO

### PASSO 1: Quebrar ItemTemplate

1. No `SetTooltipData`, você já deve ter um nó `Break ItemTemplate`
2. Este nó tem uma saída chamada `Stats` (tipo `FUmbraItemStats`)
3. **NÃO** conecte essa saída diretamente ao `Break FUmbraItemStatsBase`

### PASSO 2: Quebrar Stats (FUmbraItemStats)

**IMPORTANTE:** Você precisa quebrar o `Stats` primeiro para obter `Base`, `Combat`, `Bonus`, `Legacy`.

1. **Arraste a saída `Stats` do `Break ItemTemplate`:**
   - Clique e segure no pino `Stats` (saída do `Break ItemTemplate`)
   - Arraste para um espaço vazio no gráfico
   - Solte o mouse

2. **No menu que aparecer:**
   - Procure por "Break Struct" ou "Break FUmbraItemStats"
   - Selecione "Break FUmbraItemStats" (struct completo)
   - Isso criará um nó `Break FUmbraItemStats`

3. **Conectar:**
   - A saída `Stats` do `Break ItemTemplate` → entrada do `Break FUmbraItemStats`
   - Agora o `Break FUmbraItemStats` deve mostrar as saídas: `Base`, `Combat`, `Bonus`, `Legacy`

### PASSO 3: Quebrar Base (FUmbraItemStatsBase)

Agora sim, você pode quebrar o `Base`:

1. **Arraste a saída `Base` do `Break FUmbraItemStats`:**
   - Clique e segure no pino `Base` (saída do `Break FUmbraItemStats`)
   - Arraste para um espaço vazio no gráfico
   - Solte o mouse

2. **No menu que aparecer:**
   - Procure por "Break Struct" ou "Break FUmbraItemStatsBase"
   - Selecione "Break FUmbraItemStatsBase"
   - Isso criará um nó `Break FUmbraItemStatsBase`

3. **Conectar:**
   - A saída `Base` do `Break FUmbraItemStats` → entrada do `Break FUmbraItemStatsBase`
   - Agora o `Break FUmbraItemStatsBase` deve mostrar: `Strength`, `Dexterity`, `Intelligence`, `Vitality`, `Luck`

### PASSO 4: Quebrar Combat (FUmbraItemStatsCombat)

1. **Arraste a saída `Combat` do `Break FUmbraItemStats`:**
   - Clique e segure no pino `Combat` (saída do `Break FUmbraItemStats`)
   - Arraste para um espaço vazio no gráfico
   - Solte o mouse

2. **No menu que aparecer:**
   - Procure por "Break Struct" ou "Break FUmbraItemStatsCombat"
   - Selecione "Break FUmbraItemStatsCombat"
   - Isso criará um nó `Break FUmbraItemStatsCombat`

3. **Conectar:**
   - A saída `Combat` do `Break FUmbraItemStats` → entrada do `Break FUmbraItemStatsCombat`
   - Agora o `Break FUmbraItemStatsCombat` deve mostrar: `PhysicalAttack`, `MagicAttack`, `Critical`, etc.

### PASSO 5: Quebrar Bonus (FUmbraItemStatsBonus)

1. **Arraste a saída `Bonus` do `Break FUmbraItemStats`:**
   - Clique e segure no pino `Bonus` (saída do `Break FUmbraItemStats`)
   - Arraste para um espaço vazio no gráfico
   - Solte o mouse

2. **No menu que aparecer:**
   - Procure por "Break Struct" ou "Break FUmbraItemStatsBonus"
   - Selecione "Break FUmbraItemStatsBonus"
   - Isso criará um nó `Break FUmbraItemStatsBonus`

3. **Conectar:**
   - A saída `Bonus` do `Break FUmbraItemStats` → entrada do `Break FUmbraItemStatsBonus`
   - Agora o `Break FUmbraItemStatsBonus` deve mostrar: `HealthBonus`, `ManaBonus`, `Movement`

---

## 🎯 DIAGRAMA VISUAL CORRETO

```
┌─────────────────────────┐
│  Break ItemTemplate     │
│  (FUmbraItemTemplate)   │
├─────────────────────────┤
│  Stats ────────────┐     │
│  ItemName          │     │
│  ItemID            │     │
│  ...               │     │
└────────────────────┼─────┘
                     │
                     ▼
        ┌──────────────────────────┐
        │  Break FUmbraItemStats   │
        │  (FUmbraItemStats)       │
        ├──────────────────────────┤
        │  Base ────────────┐       │
        │  Combat ──────────┤       │
        │  Bonus ───────────┤       │
        │  Legacy ──────────┤       │
        └───────────────────┼───────┘
                            │
                            ▼
                ┌───────────────────────────┐
                │  Break FUmbraItemStatsBase│
                │  (FUmbraItemStatsBase)    │
                ├───────────────────────────┤
                │  Strength                 │
                │  Dexterity                │
                │  Intelligence             │
                │  Vitality                 │
                │  Luck                     │
                └───────────────────────────┘
```

---

## ⚠️ PROBLEMA: Stats não se conecta ao Break FUmbraItemStatsBase

**Isso está CORRETO!** Você não deve conectar `Stats` diretamente ao `Break FUmbraItemStatsBase`.

**Fluxo correto:**
1. `Break ItemTemplate` → `Stats` (tipo `FUmbraItemStats`)
2. `Stats` → `Break FUmbraItemStats` → `Base` (tipo `FUmbraItemStatsBase`)
3. `Base` → `Break FUmbraItemStatsBase` → `Strength`, `Dexterity`, etc.

---

## 🔧 MÉTODO ALTERNATIVO (se não aparecer o menu)

Se ao arrastar `Stats` não aparecer o menu para criar `Break FUmbraItemStats`:

1. **Criar o nó manualmente:**
   - Clicar com botão direito no espaço vazio
   - Digitar "Break" na busca
   - Procurar por "FUmbraItemStats"
   - Selecionar "Break FUmbraItemStats"

2. **Conectar manualmente:**
   - Pegar a saída `Stats` do `Break ItemTemplate`
   - Conectar na entrada do `Break FUmbraItemStats`

3. **Repetir para Base, Combat, Bonus:**
   - Criar `Break FUmbraItemStatsBase` manualmente
   - Conectar `Base` (de `Break FUmbraItemStats`) na entrada
   - Fazer o mesmo para `Combat` e `Bonus`

---

## ✅ CHECKLIST

Verifique se você tem:

- [ ] `Break ItemTemplate` com saída `Stats`
- [ ] `Break FUmbraItemStats` conectado ao `Stats`
- [ ] `Break FUmbraItemStats` mostrando saídas: `Base`, `Combat`, `Bonus`, `Legacy`
- [ ] `Break FUmbraItemStatsBase` conectado ao `Base`
- [ ] `Break FUmbraItemStatsBase` mostrando: `Strength`, `Dexterity`, `Intelligence`, `Vitality`, `Luck`
- [ ] `Break FUmbraItemStatsCombat` conectado ao `Combat`
- [ ] `Break FUmbraItemStatsBonus` conectado ao `Bonus`

---

## 🎯 RESUMO RÁPIDO

**Fluxo completo:**
```
Break ItemTemplate
  └─ Stats → Break FUmbraItemStats
                ├─ Base → Break FUmbraItemStatsBase → Strength, Dexterity, etc.
                ├─ Combat → Break FUmbraItemStatsCombat → PhysicalAttack, Critical, etc.
                └─ Bonus → Break FUmbraItemStatsBonus → HealthBonus, ManaBonus, Movement
```

**NÃO faça:**
```
Break ItemTemplate
  └─ Stats → Break FUmbraItemStatsBase ❌ (NÃO FUNCIONA!)
```

**FAÇA:**
```
Break ItemTemplate
  └─ Stats → Break FUmbraItemStats
                └─ Base → Break FUmbraItemStatsBase ✅ (FUNCIONA!)
```

