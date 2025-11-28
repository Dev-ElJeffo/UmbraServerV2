# 📋 ATUALIZAÇÃO DE STATS BASE DA CLASSE

## 🎯 **PROBLEMA IDENTIFICADO:**

Quando uma classe é atribuída a um player, os **stats base do player** devem ser atualizados para corresponder aos **stats base da classe**.

---

## ✅ **SOLUÇÃO IMPLEMENTADA:**

### **Scripts Criados:**

1. ✅ **`update_player_1_complete.sql`** - Script completo para player_id = 1
   - Atualiza classe, facção, stats PvP
   - **Atualiza stats base com stats da classe**

2. ✅ **`update_player_stats_from_class.sql`** - Atualiza apenas stats do player_id = 1

3. ✅ **`update_all_players_stats_from_class.sql`** - Atualiza stats de TODOS os players que têm classe

4. ✅ **`update_player_4_test_simple.sql`** - Atualizado para incluir atualização de stats

5. ✅ **`update_player_test_data.sql`** - Atualizado para incluir atualização de stats

---

## 🔧 **COMO FUNCIONA:**

### **Stats Atualizados:**

Quando um player recebe uma classe, os seguintes stats são atualizados:

- `strength` → `base_strength` da classe
- `dexterity` → `base_dexterity` da classe
- `intelligence` → `base_intelligence` da classe
- `vitality` → `base_vitality` da classe
- `luck` → `base_luck` da classe
- `max_health` → `base_health` da classe
- `health` → `base_health` da classe (atual também)
- `max_mana` → `base_mana` da classe
- `mana` → `base_mana` da classe (atual também)
- `max_stamina` → `base_stamina` da classe
- `stamina` → `base_stamina` da classe (atual também)

---

## 📋 **EXEMPLO: BARBARIAN**

**Stats Base do Barbarian:**
- Strength: 20
- Dexterity: 12
- Intelligence: 8
- Vitality: 18
- Luck: 10
- Health: 150
- Mana: 30
- Stamina: 120

**Após atualizar player_id = 1:**
- O player terá exatamente esses stats

---

## 🔧 **SCRIPTS DISPONÍVEIS:**

### **1. Atualizar Player Específico:**

```sql
-- Atualizar player_id = 1
UPDATE players p
INNER JOIN classes c ON p.class_id = c.class_id
SET 
    p.strength = c.base_strength,
    p.dexterity = c.base_dexterity,
    p.intelligence = c.base_intelligence,
    p.vitality = c.base_vitality,
    p.luck = c.base_luck,
    p.max_health = c.base_health,
    p.health = c.base_health,
    p.max_mana = c.base_mana,
    p.mana = c.base_mana,
    p.max_stamina = c.base_stamina,
    p.stamina = c.base_stamina
WHERE p.id = 1;
```

### **2. Atualizar Todos os Players:**

```sql
-- Atualizar todos os players que têm classe
UPDATE players p
INNER JOIN classes c ON p.class_id = c.class_id
SET 
    p.strength = c.base_strength,
    p.dexterity = c.base_dexterity,
    p.intelligence = c.base_intelligence,
    p.vitality = c.base_vitality,
    p.luck = c.base_luck,
    p.max_health = c.base_health,
    p.health = c.base_health,
    p.max_mana = c.base_mana,
    p.mana = c.base_mana,
    p.max_stamina = c.base_stamina,
    p.stamina = c.base_stamina
WHERE p.class_id IS NOT NULL;
```

---

## ✅ **PRÓXIMOS PASSOS:**

### **1. Executar Script para Player 1:**

Execute no MySQL Workbench:
- `www/umbra_api/scripts/update_player_1_complete.sql`

Este script:
- Atribui classe Barbarian
- Atribui facção Novarra
- Define stats PvP (50, 1000, 1)
- **Atualiza todos os stats base com stats do Barbarian**

### **2. Verificar Resultado:**

Após executar, verifique:

```sql
SELECT 
    p.strength,
    p.dexterity,
    p.intelligence,
    p.vitality,
    p.luck,
    p.max_health,
    p.max_mana,
    p.max_stamina,
    c.base_strength,
    c.base_dexterity,
    c.base_intelligence,
    c.base_vitality,
    c.base_luck,
    c.base_health,
    c.base_mana,
    c.base_stamina
FROM players p
INNER JOIN classes c ON p.class_id = c.class_id
WHERE p.id = 1;
```

**Os valores de `p.*` devem corresponder aos valores de `c.base_*`**

---

## 📝 **NOTA IMPORTANTE:**

**FUTURO:** Seria ideal criar um **TRIGGER** ou **STORED PROCEDURE** para atualizar os stats automaticamente quando `class_id` é alterado. Por enquanto, use os scripts manuais.

---

## ✅ **PRONTO!**

Execute o script `update_player_1_complete.sql` para atualizar o player_id = 1 com todos os dados, incluindo os stats base da classe Barbarian.

