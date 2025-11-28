# 🚀 GUIA RÁPIDO: Tabela de Níveis

## ✅ **O QUE FOI CRIADO:**

1. ✅ Tabela `player_levels` com níveis 1-50
2. ✅ Fórmula crescente de EXP (15% por nível)
3. ✅ API atualizada para usar a tabela
4. ✅ Scripts SQL e PHP para criar/popular

---

## 📋 **PASSO A PASSO:**

### **1. Criar Tabela e Popular Dados:**

**OPÇÃO A - SQL (Recomendado):**
```sql
-- Execute no MySQL Workbench:
SOURCE D:/UmbraServerV2/www/umbra_api/scripts/create_player_levels_table_simple.sql;
```

**OPÇÃO B - PHP (Via Navegador):**
```
http://localhost/umbra_api/scripts/create_player_levels_table_php.php
```

### **2. Verificar Dados:**
```sql
SELECT level_number, exp_required, exp_for_next_level 
FROM player_levels 
WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50);
```

### **3. Testar API:**
A API `get_character_info.php` já retorna:
- `experience`: EXP atual do jogador
- `exp_for_next_level`: EXP necessário para próximo nível
- `exp_progress_percent`: Progresso (0-100%)

---

## 📊 **FÓRMULA DE EXP:**

```
Level 1->2: 1,000 EXP
Level 2->3: 1,150 EXP (1,000 * 1.15)
Level 3->4: 1,323 EXP (1,150 * 1.15)
...
Level 50: Nível máximo (sem próximo)
```

**Total de EXP para Level 50:** 6,284,904

---

## 🔧 **ESTRUTURA DA TABELA:**

```sql
player_levels:
  - level_number (1-50)
  - exp_required (EXP total acumulado)
  - exp_for_next_level (EXP para próximo nível)
  - stat_points_gained (10 por nível)
  - hp_gain, mp_gain, etc.
```

---

## ✅ **PRONTO!**

Execute o script SQL e a API já funcionará automaticamente com os valores corretos de EXP.

