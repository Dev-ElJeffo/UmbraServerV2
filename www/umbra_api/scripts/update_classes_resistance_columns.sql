-- Script para atualizar colunas de resistência na tabela classes
-- 1. Renomear base_resistance para base_critical_resistance
-- 2. Adicionar base_double_attack_resistance
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- ============================================================================
-- PASSO 1: Renomear base_resistance para base_critical_resistance
-- ============================================================================

-- Verificar se a coluna base_resistance existe e renomeá-la
-- Se a coluna base_critical_resistance já existir, este comando falhará silenciosamente
-- (você pode verificar manualmente antes de executar)

ALTER TABLE classes
CHANGE COLUMN base_resistance base_critical_resistance INT DEFAULT 0;

-- ============================================================================
-- PASSO 2: Adicionar base_double_attack_resistance
-- ============================================================================

-- Adicionar a nova coluna base_double_attack_resistance
-- NOTA: Se a coluna já existir, este comando retornará um erro.
-- Isso é normal se você executar o script novamente - apenas ignore o erro.

ALTER TABLE classes
ADD COLUMN base_double_attack_resistance INT DEFAULT 0 AFTER base_critical_resistance;

-- ============================================================================
-- VERIFICAÇÃO
-- ============================================================================

-- Verificar a estrutura atualizada da tabela
SELECT 
    COLUMN_NAME,
    DATA_TYPE,
    IS_NULLABLE,
    COLUMN_DEFAULT,
    ORDINAL_POSITION
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'classes'
  AND COLUMN_NAME IN ('base_resistance', 'base_critical_resistance', 'base_double_attack_resistance')
ORDER BY ORDINAL_POSITION;

-- Verificar dados das classes (mostrando as colunas de resistência)
SELECT 
    class_id,
    class_name,
    base_critical_resistance,
    base_double_attack_resistance
FROM classes
ORDER BY class_id;

