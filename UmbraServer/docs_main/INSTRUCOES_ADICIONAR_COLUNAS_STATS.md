# 📋 INSTRUÇÕES: Adicionar Colunas de Stats Faltantes

## ❌ **PROBLEMA IDENTIFICADO:**

A tabela `players` no banco de dados **NÃO possui** as seguintes colunas que a API está tentando buscar:
- `agility`
- `constitution`
- `luck`

Isso causará erro na API `get_character_info.php`.

---

## ✅ **SOLUÇÃO:**

### **OPÇÃO 1: Executar Script SQL (Recomendado)**

1. **Abra o MySQL Workbench**
2. **Conecte-se ao banco `umbra_eternum`**
3. **Execute o script:** `www/umbra_api/scripts/add_character_stats_columns_simple.sql`

**OU execute manualmente:**

```sql
USE umbra_eternum;

-- Adicionar coluna agility
ALTER TABLE players
ADD COLUMN agility INT UNSIGNED DEFAULT 10 AFTER vitality;

-- Adicionar coluna constitution
ALTER TABLE players
ADD COLUMN constitution INT UNSIGNED DEFAULT 10 AFTER agility;

-- Adicionar coluna luck
ALTER TABLE players
ADD COLUMN luck INT UNSIGNED DEFAULT 10 AFTER constitution;

-- Atualizar valores existentes
UPDATE players SET agility = 10 WHERE agility IS NULL;
UPDATE players SET constitution = 10 WHERE constitution IS NULL;
UPDATE players SET luck = 10 WHERE luck IS NULL;
```

**NOTA:** Se alguma coluna já existir, você receberá um erro `Duplicate column name`, mas pode ignorar e continuar com as outras.

---

### **OPÇÃO 2: API Já Corrigida (Temporária)**

A API PHP já foi corrigida para usar `COALESCE` e retornar valor padrão (10) caso as colunas não existam.

**MAS RECOMENDO ADICIONAR AS COLUNAS** para que os valores sejam salvos corretamente no banco.

---

## 🔍 **VERIFICAÇÃO:**

Após executar o script, verifique se as colunas foram adicionadas:

```sql
SELECT 
    COLUMN_NAME, 
    DATA_TYPE, 
    COLUMN_DEFAULT, 
    IS_NULLABLE
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('agility', 'constitution', 'luck')
ORDER BY ORDINAL_POSITION;
```

**Deve retornar 3 linhas:**
- `agility` - INT UNSIGNED - DEFAULT 10
- `constitution` - INT UNSIGNED - DEFAULT 10
- `luck` - INT UNSIGNED - DEFAULT 10

---

## 📝 **ARQUIVOS CRIADOS:**

1. ✅ `www/umbra_api/scripts/add_character_stats_columns.sql` - Script completo com verificação
2. ✅ `www/umbra_api/scripts/add_character_stats_columns_simple.sql` - Script simples (recomendado)
3. ✅ `www/umbra_api/api/character/get_character_info.php` - Corrigido para usar COALESCE

---

## ✅ **PRONTO!**

Após executar o script SQL, a API funcionará corretamente e os valores serão salvos no banco de dados.

