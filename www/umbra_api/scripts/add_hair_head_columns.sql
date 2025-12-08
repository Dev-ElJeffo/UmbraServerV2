-- Script para adicionar campos hair e head na tabela players
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Adicionar campos hair e head
ALTER TABLE players
ADD COLUMN hair INT UNSIGNED DEFAULT 0 AFTER vitality,
ADD COLUMN head INT UNSIGNED DEFAULT 0 AFTER hair;

-- Comentários para documentação
ALTER TABLE players
MODIFY COLUMN hair INT UNSIGNED DEFAULT 0 COMMENT 'ID do cabelo do personagem',
MODIFY COLUMN head INT UNSIGNED DEFAULT 0 COMMENT 'ID da cabeça do personagem';

