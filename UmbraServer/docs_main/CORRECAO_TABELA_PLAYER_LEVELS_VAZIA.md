# 🔧 CORREÇÃO: Tabela player_levels Vazia

## 🐛 **PROBLEMAS IDENTIFICADOS:**

1. ❌ Tabela `player_levels` está vazia (0 rows)
2. ❌ Coluna `next_level_exp` não aparece na tabela `players`
3. ❌ UPDATE falhou por causa do safe update mode

---

## ✅ **SOLUÇÕES:**

### **1. Popular Tabela player_levels:**

Execute o script:
```sql
SOURCE D:/UmbraServerV2/www/umbra_api/scripts/populate_player_levels.sql;
```

Ou copie e cole o conteúdo do arquivo `populate_player_levels.sql` no MySQL Workbench e execute.

### **2. Adicionar Coluna next_level_exp:**

Execute o script:
```sql
SOURCE D:/UmbraServerV2/www/umbra_api/scripts/add_next_level_exp_column.sql;
```

Este script:
- ✅ Verifica se a coluna existe
- ✅ Adiciona a coluna se não existir
- ✅ Atualiza todos os jogadores com o EXP correto
- ✅ Usa WHERE para evitar erro de safe update mode

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Popular player_levels**
1. Abra o arquivo `populate_player_levels.sql`
2. Copie todo o conteúdo
3. Cole no MySQL Workbench
4. Execute (Ctrl+Enter ou botão Execute)

### **PASSO 2: Verificar dados inseridos**
```sql
SELECT * FROM player_levels WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50);
```

Deve retornar 8 linhas com dados.

### **PASSO 3: Adicionar coluna next_level_exp**
1. Abra o arquivo `add_next_level_exp_column.sql`
2. Copie todo o conteúdo
3. Cole no MySQL Workbench
4. Execute

### **PASSO 4: Verificar tabela players**
```sql
SELECT id, character_name, level, experience, next_level_exp FROM players LIMIT 10;
```

A coluna `next_level_exp` deve aparecer e ter valores.

---

## ✅ **RESULTADO ESPERADO:**

### **Tabela player_levels:**
- ✅ 50 linhas (níveis 1-50)
- ✅ Cada linha com `exp_required` e `exp_for_next_level` preenchidos

### **Tabela players:**
- ✅ Coluna `next_level_exp` visível
- ✅ Valores preenchidos baseados no nível do jogador

---

## 🔍 **VERIFICAÇÃO:**

Execute estas queries para verificar:

```sql
-- Verificar player_levels
SELECT COUNT(*) as total_niveis FROM player_levels;
-- Deve retornar: 50

-- Verificar alguns níveis
SELECT level_number, exp_required, exp_for_next_level 
FROM player_levels 
WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50);

-- Verificar coluna next_level_exp
SELECT id, character_name, level, experience, next_level_exp 
FROM players 
LIMIT 5;
```

---

## ✅ **PRONTO!**

Após executar os scripts, tudo deve funcionar corretamente.

