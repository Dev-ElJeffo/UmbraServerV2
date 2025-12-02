# 🔧 GUIA: Atualizar Tooltip para Mostrar Stats dos Itens

## 🎯 **OBJETIVO:**

Atualizar a função `UpdateStats` em `WBP_ItemTooltip` para exibir corretamente os stats que vêm do banco de dados através do `ItemTemplate.Stats`.

---

## 📋 **PROBLEMA ATUAL:**

A função `UpdateStats` estava usando `Break Umbra Item Stats`, que retornava apenas campos antigos. Agora o `FUmbraItemStats` foi atualizado para incluir TODOS os campos do banco de dados:

**Atributos Base:**
- `Strength`, `Dexterity`, `Intelligence`, `Vitality`, `Luck`

**Stats de Combate:**
- `PhysicalAttack`, `MagicAttack`, `PhysicalDefense`, `MagicDefense`
- `Accuracy`, `Dodge`, `Critical`, `Resistance`, `DoubleAttackRate`

**Bônus:**
- `HealthBonus`, `ManaBonus`, `Movement`

**Campos Legados (compatibilidade):**
- `Damage`, `AttackSpeed`, `Defense`, `FireDamage`, `SpeedBonus`
- `HealthRestore`, `ManaRestore`, `StrengthBuff`, `Duration`

Precisamos atualizar o tooltip para exibir todos esses campos de forma organizada.

---

## ✅ **SOLUÇÃO:**

Atualizar a função `UpdateStats` para verificar quais stats têm valores > 0 e exibi-los de forma organizada.

---

## 🔧 **IMPLEMENTAÇÃO:**

### **PASSO 1: Modificar a Função `UpdateStats`**

**LOCALIZAÇÃO:** `WBP_ItemTooltip` → Functions → `UpdateStats`

**ESTRUTURA ATUAL:**
- Recebe `Stats` (FUmbraItemStats)
- Faz `Break Umbra Item Stats`
- Verifica se `Damage > 0` e exibe
- Verifica se `Defense > 0` e exibe
- Exibe `Value` (mas isso vem do ItemTemplate, não do Stats)

**ESTRUTURA NOVA:**

```
[UpdateStats]
  ├─ Input: Stats (FUmbraItemStats)
  ↓
[Break Umbra Item Stats]
  ├─ Atributos: Strength, Dexterity, Intelligence, Vitality, Luck
  ├─ Combate: PhysicalAttack, MagicAttack, PhysicalDefense, MagicDefense
  ├─ Combate: Accuracy, Dodge, Critical, Resistance, DoubleAttackRate
  ├─ Bônus: HealthBonus, ManaBonus, Movement
  └─ Legados: Damage, AttackSpeed, Defense, FireDamage, SpeedBonus, etc.
       ↓
═══════════════════════════════════════════════════════════
🔴 SEÇÃO 1: Atributos Base
═══════════════════════════════════════════════════════════
       ↓
[Branch] (Strength > 0?)
  ├─ True: [Format Text] "Força: +{0}" → [Set Text] (Text_Strength)
  └─ False: [Set Text] (Text_Strength) → "" (vazio)
       ↓
[Branch] (Dexterity > 0?)
  ├─ True: [Format Text] "Destreza: +{0}" → [Set Text] (Text_Dexterity)
  └─ False: [Set Text] (Text_Dexterity) → "" (vazio)
       ↓
[Branch] (Intelligence > 0?)
  ├─ True: [Format Text] "Inteligência: +{0}" → [Set Text] (Text_Intelligence)
  └─ False: [Set Text] (Text_Intelligence) → "" (vazio)
       ↓
[Branch] (Vitality > 0?)
  ├─ True: [Format Text] "Vitalidade: +{0}" → [Set Text] (Text_Vitality)
  └─ False: [Set Text] (Text_Vitality) → "" (vazio)
       ↓
[Branch] (Luck > 0?)
  ├─ True: [Format Text] "Sorte: +{0}" → [Set Text] (Text_Luck)
  └─ False: [Set Text] (Text_Luck) → "" (vazio)
       ↓
═══════════════════════════════════════════════════════════
🟠 SEÇÃO 2: Stats de Combate
═══════════════════════════════════════════════════════════
       ↓
[Branch] (PhysicalAttack > 0?)
  ├─ True: [Format Text] "Ataque Físico: +{0}" → [Set Text] (Text_PhysicalAttack)
  └─ False: [Set Text] (Text_PhysicalAttack) → "" (vazio)
       ↓
[Branch] (MagicAttack > 0?)
  ├─ True: [Format Text] "Ataque Mágico: +{0}" → [Set Text] (Text_MagicAttack)
  └─ False: [Set Text] (Text_MagicAttack) → "" (vazio)
       ↓
[Branch] (PhysicalDefense > 0?)
  ├─ True: [Format Text] "Defesa Física: +{0}" → [Set Text] (Text_PhysicalDefense)
  └─ False: [Set Text] (Text_PhysicalDefense) → "" (vazio)
       ↓
[Branch] (MagicDefense > 0?)
  ├─ True: [Format Text] "Defesa Mágica: +{0}" → [Set Text] (Text_MagicDefense)
  └─ False: [Set Text] (Text_MagicDefense) → "" (vazio)
       ↓
[Branch] (Accuracy > 0?)
  ├─ True: [Format Text] "Precisão: +{0}" → [Set Text] (Text_Accuracy)
  └─ False: [Set Text] (Text_Accuracy) → "" (vazio)
       ↓
[Branch] (Dodge > 0?)
  ├─ True: [Format Text] "Esquiva: +{0}" → [Set Text] (Text_Dodge)
  └─ False: [Set Text] (Text_Dodge) → "" (vazio)
       ↓
[Branch] (Critical > 0?)
  ├─ True: [Format Text] "Crítico: +{0}" → [Set Text] (Text_Critical)
  └─ False: [Set Text] (Text_Critical) → "" (vazio)
       ↓
[Branch] (Resistance > 0?)
  ├─ True: [Format Text] "Resistência: +{0}" → [Set Text] (Text_Resistance)
  └─ False: [Set Text] (Text_Resistance) → "" (vazio)
       ↓
[Branch] (DoubleAttackRate > 0?)
  ├─ True: [Format Text] "Taxa de Ataque Duplo: +{0}" → [Set Text] (Text_DoubleAttackRate)
  └─ False: [Set Text] (Text_DoubleAttackRate) → "" (vazio)
       ↓
═══════════════════════════════════════════════════════════
🟢 SEÇÃO 3: Bônus de Vida/Mana/Movimento
═══════════════════════════════════════════════════════════
       ↓
[Branch] (HealthBonus > 0?)
  ├─ True: [Format Text] "Bônus de Vida: +{0}" → [Set Text] (Text_HealthBonus)
  └─ False: [Set Text] (Text_HealthBonus) → "" (vazio)
       ↓
[Branch] (ManaBonus > 0?)
  ├─ True: [Format Text] "Bônus de Mana: +{0}" → [Set Text] (Text_ManaBonus)
  └─ False: [Set Text] (Text_ManaBonus) → "" (vazio)
       ↓
[Branch] (Movement > 0?)
  ├─ True: [Format Text] "Movimento: +{0}" → [Set Text] (Text_Movement)
  └─ False: [Set Text] (Text_Movement) → "" (vazio)
```

---

### **PASSO 2: Chamar `UpdateStats` em `SetTooltipData`**

**LOCALIZAÇÃO:** `WBP_ItemTooltip` → Functions → `SetTooltipData`

**ADICIONAR APÓS:** Todas as configurações de nome, descrição, raridade, etc.

```
[SetTooltipData]
  ├─ ... (configurações existentes)
  ↓
[Break Umbra Inventory Slot]
  └─ ItemTemplate
       ↓
[Break Umbra Item Template]
  └─ Stats
       ↓
[UpdateStats]
  └─ Stats: Stats (do Break Umbra Item Template)
```

---

## 📝 **NOTAS IMPORTANTES:**

1. **TextBlocks Adicionais:**
   - Você precisará criar novos `TextBlock` widgets no Designer para cada stat:
     - **Atributos:** `Text_Strength`, `Text_Dexterity`, `Text_Intelligence`, `Text_Vitality`, `Text_Luck`
     - **Combate:** `Text_PhysicalAttack`, `Text_MagicAttack`, `Text_PhysicalDefense`, `Text_MagicDefense`
     - **Combate:** `Text_Accuracy`, `Text_Dodge`, `Text_Critical`, `Text_Resistance`, `Text_DoubleAttackRate`
     - **Bônus:** `Text_HealthBonus`, `Text_ManaBonus`, `Text_Movement`
     - **Legados (opcional):** `Text_AttackSpeed`, `Text_FireDamage`, `Text_HealthRestore`, `Text_ManaRestore`, `Text_StrengthBuff`

2. **Visibilidade:**
   - Configure os TextBlocks para `Visibility: Collapsed` por padrão
   - Use `Set Visibility: Visible` quando o stat for > 0
   - Use `Set Visibility: Collapsed` quando o stat for 0

3. **Formatação de Números:**
   - Para `int32` (todos os novos campos são inteiros):
     - Use `Format Text` com `{0}` diretamente
     - Exemplo: `"Força: +{0}"` onde `{0}` é `Strength`
   - Para `float` (campos legados como AttackSpeed, FireDamage, StrengthBuff, Duration):
     - Use `Format Text` com `{0}` e formate com 1-2 casas decimais se necessário

4. **Ordem de Exibição:**
   - Organize os TextBlocks no Designer na ordem desejada:
     1. **Atributos Base** (Strength, Dexterity, Intelligence, Vitality, Luck)
     2. **Stats de Combate** (PhysicalAttack, MagicAttack, PhysicalDefense, MagicDefense, Accuracy, Dodge, Critical, Resistance, DoubleAttackRate)
     3. **Bônus** (HealthBonus, ManaBonus, Movement)
     4. **Legados** (opcional, se necessário)

---

## 🎨 **EXEMPLO DE LAYOUT NO DESIGNER:**

```
[WBP_ItemTooltip]
  ├─ Text_ItemName (já existe)
  ├─ Text_Description (já existe)
  ├─ Text_Rarity (já existe)
  │
  ├─ ──── ATRIBUTOS BASE ────
  ├─ Text_Strength (NOVO)
  ├─ Text_Dexterity (NOVO)
  ├─ Text_Intelligence (NOVO)
  ├─ Text_Vitality (NOVO)
  ├─ Text_Luck (NOVO)
  │
  ├─ ──── STATS DE COMBATE ────
  ├─ Text_PhysicalAttack (NOVO)
  ├─ Text_MagicAttack (NOVO)
  ├─ Text_PhysicalDefense (NOVO)
  ├─ Text_MagicDefense (NOVO)
  ├─ Text_Accuracy (NOVO)
  ├─ Text_Dodge (NOVO)
  ├─ Text_Critical (NOVO)
  ├─ Text_Resistance (NOVO)
  ├─ Text_DoubleAttackRate (NOVO)
  │
  ├─ ──── BÔNUS ────
  ├─ Text_HealthBonus (NOVO)
  ├─ Text_ManaBonus (NOVO)
  ├─ Text_Movement (NOVO)
  │
  └─ Text_Value (já existe)
```

---

## ✅ **RESUMO:**

1. ✅ Modificar `UpdateStats` para verificar todos os campos de `FUmbraItemStats`
2. ✅ Criar TextBlocks adicionais no Designer para stats que não existem
3. ✅ Usar `Format Text` para formatar os valores
4. ✅ Controlar visibilidade baseado em valores > 0
5. ✅ Chamar `UpdateStats` em `SetTooltipData` passando `ItemTemplate.Stats`

---

## 🔍 **VERIFICAÇÃO:**

Após implementar, verifique:
- ✅ Stats com valor > 0 aparecem no tooltip
- ✅ Stats com valor 0 não aparecem (ou aparecem vazios)
- ✅ Formatação de números está correta
- ✅ Ordem de exibição está organizada
- ✅ Tooltip atualiza corretamente ao passar o mouse sobre diferentes itens

