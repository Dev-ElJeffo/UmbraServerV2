# GUIA VISUAL: COMO QUEBRAR OS STRUCTS NO BLUEPRINT

## ⚠️ PROBLEMA COMUM

Você está tentando criar um `Break FUmbraItemStatsBase` mas não sabe de onde vem a entrada.

**RESPOSTA:** A entrada vem do `Break ItemTemplate.Stats` → saída `Base`

---

## 📋 PASSO A PASSO VISUAL

### PASSO 1: Encontrar o `Break ItemTemplate.Stats`

1. No `SetTooltipData`, procure pelo nó que quebra `ItemTemplate.Stats`
2. Este nó deve ter MUITAS saídas (Strength, Dexterity, PhysicalAttack, etc.)
3. **AGORA** ele também deve ter 4 novas saídas: `Base`, `Combat`, `Bonus`, `Legacy`

### PASSO 2: Criar o `Break FUmbraItemStatsBase`

**MÉTODO CORRETO:**

1. **Arraste a saída `Base` do `Break ItemTemplate.Stats`:**
   - Clique e segure no pino `Base` (saída do `Break ItemTemplate.Stats`)
   - Arraste para um espaço vazio no gráfico
   - Solte o mouse

2. **O Unreal vai mostrar um menu de contexto:**
   - Procure por "Break Struct" ou "Break FUmbraItemStatsBase"
   - Se não aparecer, tente:
     - Clicar com botão direito no espaço vazio
     - Digitar "Break" na busca
     - Procurar por "FUmbraItemStatsBase"

3. **Selecionar o tipo correto:**
   - Escolha `FUmbraItemStatsBase` (não `FUmbraItemStats`)
   - Isso criará um nó `Break FUmbraItemStatsBase`

4. **Conectar:**
   - A saída `Base` do `Break ItemTemplate.Stats` deve estar conectada automaticamente
   - Se não estiver, conecte manualmente: `Base` → entrada do `Break FUmbraItemStatsBase`

### PASSO 3: Verificar se funcionou

Após criar o `Break FUmbraItemStatsBase`, você deve ver:
- **Entrada:** `FUmbraItemStatsBase Struct` (conectada ao `Base` do `Break ItemTemplate.Stats`)
- **Saídas:** `Strength`, `Dexterity`, `Intelligence`, `Vitality`, `Luck`

---

## 🔧 MÉTODO ALTERNATIVO (se o método acima não funcionar)

### Opção 1: Criar o nó manualmente

1. **Clicar com botão direito no espaço vazio**
2. **Digitar "Break" na busca**
3. **Procurar por "FUmbraItemStatsBase"**
4. **Selecionar "Break FUmbraItemStatsBase"**
5. **Conectar manualmente:**
   - Pegar a saída `Base` do `Break ItemTemplate.Stats`
   - Conectar na entrada do `Break FUmbraItemStatsBase`

### Opção 2: Verificar se o struct está disponível

Se você não consegue encontrar `FUmbraItemStatsBase`:

1. **Verificar se o projeto foi recompilado:**
   - Fechar Unreal Editor
   - Recompilar o projeto no Visual Studio
   - Abrir Unreal Editor novamente

2. **Verificar se os structs foram regenerados:**
   - Deletar `Intermediate/` e `Binaries/`
   - Regenerar arquivos de projeto
   - Recompilar

3. **Verificar no código C++:**
   - Abrir `UmbraDataStructures.h`
   - Verificar se `FUmbraItemStatsBase` tem `USTRUCT(BlueprintType)`

---

## 🎯 DIAGRAMA VISUAL

```
┌─────────────────────────────────────┐
│  Break ItemTemplate.Stats           │
│  (FUmbraItemStats)                  │
├─────────────────────────────────────┤
│  Base ────────────┐                  │
│  Combat ──────────┤                  │
│  Bonus ────────────┤                  │
│  Legacy ──────────┤                  │
│  Strength ────────┤ (DEPRECATED)     │
│  PhysicalAttack ──┤ (DEPRECATED)     │
│  ...              │                  │
└───────────────────┼──────────────────┘
                    │
                    ▼
        ┌───────────────────────────┐
        │  Break FUmbraItemStatsBase│
        │  (FUmbraItemStatsBase)     │
        ├───────────────────────────┤
        │  Strength                 │
        │  Dexterity                │
        │  Intelligence             │
        │  Vitality                 │
        │  Luck                     │
        └───────────────────────────┘
```

---

## ❌ ERROS COMUNS

### ERRO 1: Tentar criar variável

**ERRADO:**
```
Criar variável do tipo FUmbraItemStatsBase
→ Tentar conectar ao Break FUmbraItemStatsBase
→ Retorna 0 em todos os campos
```

**CORRETO:**
```
Pegar Base do Break ItemTemplate.Stats
→ Conectar ao Break FUmbraItemStatsBase
→ Funciona corretamente
```

### ERRO 2: Usar o struct errado

**ERRADO:**
```
Break FUmbraItemStats (struct completo)
→ Ainda tem problemas de serialização
```

**CORRETO:**
```
Break FUmbraItemStatsBase (struct menor)
→ Funciona corretamente
```

### ERRO 3: Não conectar corretamente

**ERRADO:**
```
Break ItemTemplate.Stats → Break FUmbraItemStatsBase
Mas conecta Strength (deprecated) em vez de Base
```

**CORRETO:**
```
Break ItemTemplate.Stats → Base → Break FUmbraItemStatsBase
Conecta Base (não Strength)
```

---

## ✅ CHECKLIST

Antes de continuar, verifique:

- [ ] O `Break ItemTemplate.Stats` tem a saída `Base`?
- [ ] O `Break FUmbraItemStatsBase` foi criado?
- [ ] A saída `Base` está conectada à entrada do `Break FUmbraItemStatsBase`?
- [ ] O `Break FUmbraItemStatsBase` mostra as saídas: Strength, Dexterity, Intelligence, Vitality, Luck?
- [ ] Os valores não são todos 0?

---

## 🔍 DEBUG: Por que está retornando 0?

Se o `Break FUmbraItemStatsBase` está retornando 0 em todos os campos:

### Verificação 1: Conexão correta?

1. Verificar se a saída `Base` do `Break ItemTemplate.Stats` está conectada
2. Verificar se está conectada na entrada correta do `Break FUmbraItemStatsBase`
3. **NÃO** conectar `Strength` (deprecated) - conectar `Base`

### Verificação 2: Struct está sendo preenchido?

1. Adicionar um `Print String` após o `Break ItemTemplate.Stats`
2. Verificar se `Base` não é vazio
3. Verificar logs do C++ para ver se `ParseItemStats` está preenchendo `Base`

### Verificação 3: Tipo correto?

1. Verificar se o `Break FUmbraItemStatsBase` está usando o tipo `FUmbraItemStatsBase`
2. **NÃO** usar `FUmbraItemStats` (struct completo)

---

## 🎯 SOLUÇÃO RÁPIDA

Se nada funcionar, tente isso:

1. **Remover todos os `Break` antigos**
2. **No `SetTooltipData`:**
   - Quebrar `ItemTemplate.Stats` → obter `Base`, `Combat`, `Bonus`
   - Criar `Break FUmbraItemStatsBase` → conectar `Base`
   - Criar `Break FUmbraItemStatsCombat` → conectar `Combat`
   - Criar `Break FUmbraItemStatsBonus` → conectar `Bonus`
3. **Usar os valores dos novos `Break`**

---

## 📞 SE AINDA NÃO FUNCIONAR

Envie:
1. Screenshot do `Break ItemTemplate.Stats` mostrando as saídas
2. Screenshot do `Break FUmbraItemStatsBase` mostrando as conexões
3. Logs do C++ mostrando os valores de `Base` após `ParseItemStats`

