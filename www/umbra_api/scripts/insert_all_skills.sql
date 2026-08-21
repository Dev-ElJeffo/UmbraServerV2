-- ============================================================================
-- UMBRA ETERNUM - INSERÇÃO DE TODAS AS 90 SKILLS
-- 6 Classes x 15 Skills cada
-- ============================================================================
-- Versão: 1.0.0
-- Data: 2026-02-21
-- ============================================================================

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ============================================================================
-- Níveis de desbloqueio das skills (distribuição 1-50)
-- Skill 1: Level 1    | Skill 6: Level 18   | Skill 11: Level 38
-- Skill 2: Level 4    | Skill 7: Level 22   | Skill 12: Level 42
-- Skill 3: Level 7    | Skill 8: Level 26   | Skill 13: Level 45
-- Skill 4: Level 10   | Skill 9: Level 30   | Skill 14: Level 48
-- Skill 5: Level 14   | Skill 10: Level 34  | Skill 15: Level 50 (Ultimate)
-- ============================================================================

-- ============================================================================
-- BARBARIAN (class_id = 1) — Árvore: Fúria da Sobrevivente
-- Arquétipo: Bruiser ofensiva, dano físico alto, resistência bruta
-- Escala com: Strength + Vitality
-- ============================================================================

INSERT INTO `skills` (
    `skill_key`, `skill_name`, `class_id`, `skill_order`, `required_level`, `skill_cost`, `max_rank`,
    `type_id`, `target_id`, `element_id`, `scaling_stat_id`,
    `str_scaling`, `dex_scaling`, `vit_scaling`, `int_scaling`, `lck_scaling`,
    `power_coef`, `secondary_coef`,
    `resource_type`, `resource_cost`, `resource_cost_percent`,
    `cooldown_ms`, `cast_time_ms`, `duration_ms`,
    `range_min`, `range_max`, `area_radius`,
    `is_stackable`, `max_stacks`, `can_crit`, `ignores_defense`, `is_interrupt`, `requires_target`, `can_move_while_casting`,
    `threat_modifier`, `pvp_modifier`,
    `icon_path`, `vfx_key`, `sfx_key`,
    `description`, `tooltip_template`,
    `server_tags`, `effects_json`
) VALUES

-- 1. Golpe da Ruína (Level 1)
('BARB_RUIN_STRIKE', 'Golpe da Ruína', 1, 1, 1, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
60, 0, 0, 0, 0, -- STR 60%
180, 25, -- power 1.8x, secondary 0.25 (double atk chance)
'MANA', 15, 0,
3000, 500, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
110, 100,
'Skills/Barbarian/T_Ruin_Strike', 'VFX_Heavy_Slash', 'SFX_Heavy_Impact',
'Um ataque descendente brutal que quebra defesas pela força pura.',
'Causa {damage} de dano físico. {secondary}% de chance de Double Attack.',
'["melee", "single_target", "high_damage"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 180}, {"type": "BUFF_STAT", "target_stat": "double_attack_chance", "value_percent": 25, "duration_ms": 0, "chance_percent": 100}, {"type": "STUN", "duration_ms": 2000, "chance_percent": 100}]'),

-- 2. Pele Endurecida (Level 4)
('BARB_HARDENED_SKIN', 'Pele Endurecida', 1, 2, 4, 1, 5,
2, 1, 1, 4, -- PASSIVE, SELF, PHYSICAL, DEFENSE
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Hardened_Skin', NULL, NULL,
'A carne calejada absorve impacto como couro de guerra.',
'+{value} Defesa Física permanente.',
'["passive", "defensive", "permanent"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_defense", "value_flat": 50, "value_percent": 15}]'),

-- 3. Investida Selvagem (Level 7)
('BARB_WILD_CHARGE', 'Investida Selvagem', 1, 3, 7, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
40, 20, 0, 0, 0, -- STR 40%, DEX 20%
140, 0,
'MANA', 25, 0,
8000, 0, 3000,
0, 300, 0,
0, 1, 1, 0, 0, 1, 1,
100, 100,
'Skills/Barbarian/T_Wild_Charge', 'VFX_Charge', 'SFX_Rush',
'A bárbara avança como uma avalanche viva.',
'Avança até o alvo causando {damage} de dano. +{speed}% Movement Speed por {duration}s.',
'["melee", "gap_closer", "mobility"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 140}, {"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 30, "duration_ms": 3000}]'),

-- 4. Sangue Quente (Level 10)
('BARB_HOT_BLOOD', 'Sangue Quente', 1, 4, 10, 1, 5,
2, 1, 1, 1, -- PASSIVE, SELF, PHYSICAL, PHYS_ATK
0, 0, 0, 0, 40, -- LCK 40%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Hot_Blood', NULL, NULL,
'Ferimentos despertam a fúria latente.',
'+{value}% Critical quando Health abaixo de 30%.',
'["passive", "offensive", "low_health_trigger"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_chance", "value_percent": 25, "conditions_json": {"health_below_percent": 30}}]'),

-- 5. Rugido de Guerra (Level 14)
('BARB_WAR_CRY', 'Rugido de Guerra', 1, 5, 14, 1, 5,
3, 5, 1, 1, -- BUFF, PARTY, PHYSICAL, PHYS_ATK
50, 0, 0, 0, 0, -- STR 50%
0, 0,
'MANA', 40, 0,
30000, 1000, 15000,
0, 0, 500,
0, 1, 0, 0, 0, 0, 0,
150, 100,
'Skills/Barbarian/T_War_Cry', 'VFX_War_Shout', 'SFX_Roar',
'Um grito que endurece a coragem dos vivos.',
'+{value}% Phys Atk para aliados próximos por {duration}s.',
'["buff", "party", "aoe_ally"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_attack", "value_percent": 20, "duration_ms": 15000}]'),

-- 6. Ossos de Ferro (Level 18)
('BARB_IRON_BONES', 'Ossos de Ferro', 1, 6, 18, 1, 5,
2, 1, 1, 4, -- PASSIVE, SELF, PHYSICAL, DEFENSE
0, 0, 45, 0, 0, -- VIT 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Iron_Bones', NULL, NULL,
'Seus ossos foram quebrados — e reconstruídos mais fortes.',
'+{value} Critical Resistance permanente.',
'["passive", "defensive", "anti_crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_resistance", "value_flat": 30, "value_percent": 10}]'),

-- 7. Corte Dilacerante (Level 22)
('BARB_RENDING_CUT', 'Corte Dilacerante', 1, 7, 22, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
55, 0, 0, 0, 0, -- STR 55%
130, 40, -- power 1.3x, secondary 0.4 (bleed)
'MANA', 30, 0,
6000, 300, 8000,
0, 150, 0,
1, 3, 1, 0, 0, 1, 0,
100, 100,
'Skills/Barbarian/T_Rending_Cut', 'VFX_Slash_Bleed', 'SFX_Slash',
'A lâmina rasga carne e esperança.',
'Causa {damage} de dano e aplica Sangramento: {dot} dano/s por {duration}s.',
'["melee", "dot", "bleed"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 130}, {"type": "DOT", "target_stat": "health", "value_percent": 40, "duration_ms": 8000, "tick_interval_ms": 2000}]'),

-- 8. Passo da Caçadora (Level 26)
('BARB_HUNTER_STEP', 'Passo da Caçadora', 1, 8, 26, 1, 5,
2, 1, 1, 5, -- PASSIVE, SELF, PHYSICAL, NONE
0, 40, 0, 0, 0, -- DEX 40%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Hunter_Step', NULL, NULL,
'Movimento instintivo de predadora.',
'+{value} Dodge permanente.',
'["passive", "defensive", "evasion"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_flat": 25, "value_percent": 8}]'),

-- 9. Recuperação Instintiva (Level 30)
('BARB_INSTINCT_RECOVERY', 'Recuperação Instintiva', 1, 9, 30, 1, 5,
1, 1, 1, 3, -- ACTIVE, SELF, PHYSICAL, HEALTH
0, 0, 60, 0, 0, -- VIT 60%
0, 0,
'MANA', 50, 0,
45000, 1500, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
50, 100,
'Skills/Barbarian/T_Instinct_Recovery', 'VFX_Self_Heal', 'SFX_Breath',
'Respiração profunda e dor ignorada.',
'Recupera {heal}% do Health máximo.',
'["heal", "self", "survival"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 25}]'),

-- 10. Retaliação Primitiva (Level 34)
('BARB_PRIMAL_RETALIATION', 'Retaliação Primitiva', 1, 10, 34, 1, 5,
7, 1, 1, 1, -- REACTION, SELF, PHYSICAL, PHYS_ATK
50, 0, 0, 0, 0, -- STR 50%
120, 0,
'NONE', 0, 0,
10000, 0, 0,
0, 150, 0,
0, 1, 1, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Primal_Retaliation', 'VFX_Counter', 'SFX_Counter_Hit',
'Cada golpe sofrido cobra preço.',
'Ao receber crítico, contra-ataca causando {damage} de dano.',
'["reaction", "counter", "trigger_on_crit_received"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 120, "conditions_json": {"trigger": "on_crit_received"}}]'),

-- 11. Frenesi Carmesim (Level 38)
('BARB_CRIMSON_FRENZY', 'Frenesi Carmesim', 1, 11, 38, 1, 5,
3, 1, 1, 1, -- BUFF, SELF, PHYSICAL, PHYS_ATK
55, 0, 0, 0, 0, -- STR 55%
0, 0,
'MANA', 35, 0,
25000, 500, 12000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
120, 100,
'Skills/Barbarian/T_Crimson_Frenzy', 'VFX_Frenzy', 'SFX_Rage',
'Troca proteção por massacre.',
'+{attack}% Double Attack, -{defense}% Defesa por {duration}s.',
'["buff", "self", "risk_reward"]',
'[{"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 35, "duration_ms": 12000}, {"type": "DEBUFF_STAT", "target_stat": "physical_defense", "value_percent": -20, "duration_ms": 12000}]'),

-- 12. Resistência Brutal (Level 42)
('BARB_BRUTAL_RESISTANCE', 'Resistência Brutal', 1, 12, 42, 1, 5,
2, 1, 1, 3, -- PASSIVE, SELF, PHYSICAL, HEALTH
0, 0, 55, 0, 0, -- VIT 55%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Brutal_Resistance', NULL, NULL,
'Sobrevive onde outros tombam.',
'+{value} Health Máximo permanente.',
'["passive", "defensive", "health"]',
'[{"type": "BUFF_STAT", "target_stat": "max_health", "value_flat": 200, "value_percent": 12}]'),

-- 13. Impacto Esmagador (Level 45)
('BARB_CRUSHING_IMPACT', 'Impacto Esmagador', 1, 13, 45, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
60, 0, 0, 0, 0, -- STR 60%
150, 0,
'MANA', 40, 0,
12000, 800, 6000,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Barbarian/T_Crushing_Impact', 'VFX_Ground_Slam', 'SFX_Heavy_Impact',
'Golpe que quebra postura.',
'Causa {damage} de dano e reduz Dodge do alvo em {debuff}% por {duration}s.',
'["melee", "debuff", "armor_break"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 150}, {"type": "DEBUFF_STAT", "target_stat": "dodge", "value_percent": -30, "duration_ms": 6000}]'),

-- 14. Caçada Implacável (Level 48)
('BARB_RELENTLESS_HUNT', 'Caçada Implacável', 1, 14, 48, 1, 5,
2, 1, 1, 1, -- PASSIVE, SELF, PHYSICAL, PHYS_ATK
0, 45, 0, 0, 0, -- DEX 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Barbarian/T_Relentless_Hunt', NULL, NULL,
'A presa não escapa duas vezes.',
'+{value} Accuracy permanente.',
'["passive", "offensive", "accuracy"]',
'[{"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 40, "value_percent": 10}]'),

-- 15. Avatar da Ruína (Level 50 - ULTIMATE)
('BARB_AVATAR_RUIN', 'Avatar da Ruína', 1, 15, 50, 1, 5,
6, 1, 1, 1, -- ULTIMATE, SELF, PHYSICAL, PHYS_ATK
50, 0, 30, 0, 20, -- STR 50%, VIT 30%, LCK 20%
0, 0,
'MANA', 100, 0,
180000, 2000, 20000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
200, 80,
'Skills/Barbarian/T_Avatar_Ruin', 'VFX_Ultimate_Barbarian', 'SFX_Ultimate_Roar',
'A fúria vira entidade de guerra.',
'Por {duration}s: +{atk}% Phys Atk, +{speed}% Speed, +{double}% Double Attack.',
'["ultimate", "self_buff", "transformation"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_attack", "value_percent": 40, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 25, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 30, "duration_ms": 20000}]'),

-- ============================================================================
-- DARK MAGE (class_id = 3) — Árvore: Véu Rubro
-- Arquétipo: caster ofensiva / controle / drenagem
-- Escala com: Intelligence + Luck
-- ============================================================================

-- 1. Rajada de Névoa Carmesim (Level 1)
('DMAGE_CRIMSON_MIST', 'Rajada de Névoa Carmesim', 3, 1, 1, 1, 5,
1, 2, 2, 2, -- ACTIVE, ENEMY, SHADOW, MAG_ATK
0, 0, 0, 60, 0, -- INT 60%
170, 0,
'MANA', 20, 0,
4000, 800, 0,
0, 400, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/DarkMage/T_Crimson_Mist', 'VFX_Shadow_Bolt', 'SFX_Dark_Cast',
'Dispara energia da Neblina Vermelha que corrói a alma.',
'Causa {damage} de dano Shadow.',
'["ranged", "magic", "shadow"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 170}]'),

-- 2. Marca Dimensional (Level 4)
('DMAGE_DIMENSIONAL_MARK', 'Marca Dimensional', 3, 2, 4, 1, 5,
4, 2, 2, 2, -- DEBUFF, ENEMY, SHADOW, MAG_ATK
0, 0, 0, 55, 0, -- INT 55%
0, 0,
'MANA', 25, 0,
10000, 600, 10000,
0, 350, 0,
0, 1, 0, 0, 0, 1, 0,
80, 100,
'Skills/DarkMage/T_Dimensional_Mark', 'VFX_Mark', 'SFX_Mark_Apply',
'Selo que enfraquece o tecido mágico do inimigo.',
'Reduz Magic Defense do alvo em {value}% por {duration}s.',
'["debuff", "magic_pen", "control"]',
'[{"type": "DEBUFF_STAT", "target_stat": "magic_defense", "value_percent": -25, "duration_ms": 10000}]'),

-- 3. Véu Sombrio (Level 7)
('DMAGE_SHADOW_VEIL', 'Véu Sombrio', 3, 3, 7, 1, 5,
3, 1, 2, 5, -- BUFF, SELF, SHADOW, NONE
0, 40, 0, 0, 0, -- DEX 40%
0, 0,
'MANA', 30, 0,
20000, 500, 8000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Shadow_Veil', 'VFX_Shadow_Aura', 'SFX_Shadow_Whisper',
'O corpo oscila entre planos.',
'+{value}% Dodge por {duration}s.',
'["buff", "defensive", "evasion"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_percent": 30, "duration_ms": 8000}]'),

-- 4. Eco Arcano (Level 10)
('DMAGE_ARCANE_ECHO', 'Eco Arcano', 3, 4, 10, 1, 5,
2, 1, 8, 2, -- PASSIVE, SELF, ARCANE, MAG_ATK
0, 0, 0, 0, 45, -- LCK 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Arcane_Echo', NULL, NULL,
'Magia reverbera além do primeiro impacto.',
'{value}% de chance de Double Magic Attack.',
'["passive", "offensive", "double_cast"]',
'[{"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 15}]'),

-- 5. Selo de Ruptura (Level 14)
('DMAGE_RUPTURE_SEAL', 'Selo de Ruptura', 3, 5, 14, 1, 5,
8, 2, 2, 2, -- DOT, ENEMY, SHADOW, MAG_ATK
0, 0, 0, 55, 0, -- INT 55%
50, 80,
'MANA', 35, 0,
8000, 700, 12000,
0, 350, 0,
1, 3, 0, 0, 0, 1, 0,
90, 100,
'Skills/DarkMage/T_Rupture_Seal', 'VFX_DoT_Shadow', 'SFX_Seal',
'Energia dimensional rasga lentamente.',
'Aplica {damage} de dano Shadow a cada {tick}s por {duration}s.',
'["dot", "shadow", "magic"]',
'[{"type": "DOT", "target_stat": "health", "value_percent": 80, "duration_ms": 12000, "tick_interval_ms": 3000}]'),

-- 6. Mente Profunda (Level 18)
('DMAGE_DEEP_MIND', 'Mente Profunda', 3, 6, 18, 1, 5,
2, 1, 8, 5, -- PASSIVE, SELF, ARCANE, NONE
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Deep_Mind', NULL, NULL,
'Intelecto expandido pelo Véu.',
'+{value} Mana Máximo permanente.',
'["passive", "resource", "mana"]',
'[{"type": "BUFF_STAT", "target_stat": "max_mana", "value_flat": 150, "value_percent": 15}]'),

-- 7. Orbe da Corrupção (Level 22)
('DMAGE_CORRUPTION_ORB', 'Orbe da Corrupção', 3, 7, 22, 1, 5,
1, 4, 2, 2, -- ACTIVE, AREA, SHADOW, MAG_ATK
0, 0, 0, 60, 0, -- INT 60%
145, 0,
'MANA', 45, 0,
10000, 1200, 0,
0, 350, 200,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/DarkMage/T_Corruption_Orb', 'VFX_AoE_Shadow', 'SFX_Explosion_Dark',
'Explosão de energia carmesim instável.',
'Causa {damage} de dano Shadow em área.',
'["aoe", "magic", "shadow"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 145}]'),

-- 8. Reflexo do Véu (Level 26)
('DMAGE_VEIL_REFLECTION', 'Reflexo do Véu', 3, 8, 26, 1, 5,
2, 1, 2, 5, -- PASSIVE, SELF, SHADOW, NONE
0, 0, 0, 0, 40, -- LCK 40%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Veil_Reflection', NULL, NULL,
'Previsão arcana de impactos.',
'+{value} Critical Resistance permanente.',
'["passive", "defensive", "anti_crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_resistance", "value_flat": 25, "value_percent": 8}]'),

-- 9. Dreno Carmesim (Level 30)
('DMAGE_CRIMSON_DRAIN', 'Dreno Carmesim', 3, 9, 30, 1, 5,
1, 2, 2, 2, -- ACTIVE, ENEMY, SHADOW, MAG_ATK
0, 0, 0, 55, 0, -- INT 55%
120, 40,
'MANA', 40, 0,
12000, 800, 0,
0, 300, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/DarkMage/T_Crimson_Drain', 'VFX_Drain', 'SFX_Soul_Drain',
'Rouba vitalidade pela ruptura planar.',
'Causa {damage} de dano e cura {heal}% do dano causado.',
'["magic", "lifesteal", "sustain"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 120}, {"type": "LIFESTEAL", "value_percent": 40}]'),

-- 10. Convergência Rubicina (Level 34)
('DMAGE_RUBY_CONVERGENCE', 'Convergência Rubicina', 3, 10, 34, 1, 5,
3, 1, 8, 2, -- BUFF, SELF, ARCANE, MAG_ATK
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 35, 0,
25000, 600, 12000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Ruby_Convergence', 'VFX_Magic_Buff', 'SFX_Power_Up',
'Amplifica foco em cristal arcano.',
'+{value}% Magic Attack por {duration}s.',
'["buff", "offensive", "magic_power"]',
'[{"type": "BUFF_STAT", "target_stat": "magic_attack", "value_percent": 30, "duration_ms": 12000}]'),

-- 11. Torção da Realidade (Level 38)
('DMAGE_REALITY_TWIST', 'Torção da Realidade', 3, 11, 38, 1, 5,
4, 2, 8, 2, -- DEBUFF, ENEMY, ARCANE, MAG_ATK
0, 0, 0, 0, 45, -- LCK 45%
0, 0,
'MANA', 30, 0,
15000, 500, 8000,
0, 350, 0,
0, 1, 0, 0, 0, 1, 0,
80, 100,
'Skills/DarkMage/T_Reality_Twist', 'VFX_Distortion', 'SFX_Warp',
'Distorce percepção inimiga.',
'Reduz Accuracy do alvo em {value}% por {duration}s.',
'["debuff", "control", "blind"]',
'[{"type": "DEBUFF_STAT", "target_stat": "accuracy", "value_percent": -25, "duration_ms": 8000}]'),

-- 12. Passo Entre Fendas (Level 42)
('DMAGE_RIFT_STEP', 'Passo Entre Fendas', 3, 12, 42, 1, 5,
1, 1, 8, 5, -- ACTIVE, SELF, ARCANE, NONE
0, 40, 0, 0, 0, -- DEX 40%
0, 0,
'MANA', 25, 0,
15000, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
100, 100,
'Skills/DarkMage/T_Rift_Step', 'VFX_Teleport', 'SFX_Blink',
'Deslocamento dimensional curto.',
'Teleporta instantaneamente até {range} unidades.',
'["mobility", "teleport", "escape"]',
'[{"type": "TELEPORT", "value_flat": 500}]'),

-- 13. Colapso Arcano (Level 45)
('DMAGE_ARCANE_COLLAPSE', 'Colapso Arcano', 3, 13, 45, 1, 5,
1, 2, 8, 2, -- ACTIVE, ENEMY, ARCANE, MAG_ATK
0, 0, 0, 50, 30, -- INT 50%, LCK 30%
200, 0,
'MANA', 60, 0,
18000, 1500, 0,
0, 400, 0,
0, 1, 1, 0, 0, 1, 0,
120, 100,
'Skills/DarkMage/T_Arcane_Collapse', 'VFX_Arcane_Burst', 'SFX_Collapse',
'Colapso local do tecido mágico.',
'Causa {damage} de dano Arcane massivo. Alto crítico.',
'["burst", "magic", "high_crit"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 200}]'),

-- 14. Afinidade da Névoa (Level 48)
('DMAGE_MIST_AFFINITY', 'Afinidade da Névoa', 3, 14, 48, 1, 5,
2, 1, 2, 4, -- PASSIVE, SELF, SHADOW, DEFENSE
0, 0, 0, 45, 0, -- INT 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/DarkMage/T_Mist_Affinity', NULL, NULL,
'Resistência à corrupção.',
'+{value} Magic Defense permanente.',
'["passive", "defensive", "magic_resist"]',
'[{"type": "BUFF_STAT", "target_stat": "magic_defense", "value_flat": 40, "value_percent": 12}]'),

-- 15. Abertura do Portal Menor (Level 50 - ULTIMATE)
('DMAGE_MINOR_PORTAL', 'Abertura do Portal Menor', 3, 15, 50, 1, 5,
6, 4, 2, 2, -- ULTIMATE, AREA, SHADOW, MAG_ATK
0, 0, 0, 55, 25, -- INT 55%, LCK 25%
250, 0,
'MANA', 120, 0,
180000, 2500, 0,
0, 400, 300,
0, 1, 1, 0, 0, 1, 0,
150, 80,
'Skills/DarkMage/T_Minor_Portal', 'VFX_Ultimate_DarkMage', 'SFX_Portal_Open',
'Rasga o espaço por um instante.',
'Causa {damage} de dano Shadow massivo em área.',
'["ultimate", "aoe", "devastation"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 250}]'),

-- ============================================================================
-- MONK/MARCIAL (class_id = 6) — Árvore: Doutrina da Lâmina Cinzenta
-- Arquétipo: combatente técnico equilibrado, precisão, anti-erro
-- Escala com: Dexterity + Strength
-- ============================================================================

-- 1. Corte Técnico (Level 1)
('MONK_TECH_CUT', 'Corte Técnico', 6, 1, 1, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
0, 55, 0, 0, 0, -- DEX 55%
150, 20,
'MANA', 15, 0,
3000, 400, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Monk/T_Tech_Cut', 'VFX_Precise_Slash', 'SFX_Swift_Cut',
'Um golpe treinado para atingir pontos vulneráveis com eficiência fria.',
'Causa {damage} de dano com +{accuracy} Accuracy.',
'["melee", "precise", "single_target"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 150}, {"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 20, "duration_ms": 0}]'),

-- 2. Guarda Alta (Level 4)
('MONK_HIGH_GUARD', 'Guarda Alta', 6, 2, 4, 1, 5,
3, 1, 1, 4, -- BUFF, SELF, PHYSICAL, DEFENSE
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'MANA', 20, 0,
15000, 300, 8000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_High_Guard', 'VFX_Defense_Stance', 'SFX_Guard',
'Postura defensiva clássica das escolas de guerra pré-queda.',
'+{value}% Physical Defense por {duration}s.',
'["buff", "defensive", "stance"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 25, "duration_ms": 8000}]'),

-- 3. Sequência Disciplinada (Level 7)
('MONK_DISCIPLINE_SEQUENCE', 'Sequência Disciplinada', 6, 3, 7, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
0, 50, 0, 0, 0, -- DEX 50%
130, 30,
'MANA', 25, 0,
5000, 500, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Monk/T_Discipline_Sequence', 'VFX_Multi_Strike', 'SFX_Rapid_Hits',
'Cadeia de golpes calculados sem desperdício de movimento.',
'Causa {damage} de dano com {double}% chance de Double Attack.',
'["melee", "combo", "double_attack"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 130}, {"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 30, "duration_ms": 0}]'),

-- 4. Passo Tático (Level 10)
('MONK_TACTICAL_STEP', 'Passo Tático', 6, 4, 10, 1, 5,
3, 1, 1, 5, -- BUFF, SELF, PHYSICAL, NONE
0, 45, 0, 0, 0, -- DEX 45%
0, 0,
'MANA', 15, 0,
12000, 0, 4000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
100, 100,
'Skills/Monk/T_Tactical_Step', 'VFX_Speed_Boost', 'SFX_Dash',
'Reposicionamento preciso no caos do campo de batalha.',
'+{value}% Movement Speed por {duration}s.',
'["mobility", "buff", "reposition"]',
'[{"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 40, "duration_ms": 4000}]'),

-- 5. Postura Precisa (Level 14)
('MONK_PRECISE_STANCE', 'Postura Precisa', 6, 5, 14, 1, 5,
2, 1, 1, 1, -- PASSIVE, SELF, PHYSICAL, PHYS_ATK
0, 0, 0, 0, 45, -- LCK 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Precise_Stance', NULL, NULL,
'Técnica refinada aumenta o impacto de acertos perfeitos.',
'+{value} Critical Attack permanente.',
'["passive", "offensive", "crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_chance", "value_flat": 20, "value_percent": 8}]'),

-- 6. Disciplina de Ferro (Level 18)
('MONK_IRON_DISCIPLINE', 'Disciplina de Ferro', 6, 6, 18, 1, 5,
2, 1, 1, 4, -- PASSIVE, SELF, PHYSICAL, DEFENSE
0, 0, 45, 0, 0, -- VIT 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Iron_Discipline', NULL, NULL,
'Controle mental reduz vulnerabilidade a golpes decisivos.',
'+{value} Critical Resistance permanente.',
'["passive", "defensive", "anti_crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_resistance", "value_flat": 25, "value_percent": 8}]'),

-- 7. Golpe Quebrador (Level 22)
('MONK_BREAKER_STRIKE', 'Golpe Quebrador', 6, 7, 22, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
45, 0, 0, 0, 0, -- STR 45%
160, 0,
'MANA', 30, 0,
8000, 600, 6000,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Monk/T_Breaker_Strike', 'VFX_Armor_Break', 'SFX_Armor_Crack',
'Ataque direcionado para romper guarda.',
'Causa {damage} de dano e reduz Physical Defense em {debuff}% por {duration}s.',
'["melee", "armor_break", "debuff"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 160}, {"type": "DEBUFF_STAT", "target_stat": "physical_defense", "value_percent": -20, "duration_ms": 6000}]'),

-- 8. Ripostar (Level 26)
('MONK_RIPOSTE', 'Ripostar', 6, 8, 26, 1, 5,
7, 1, 1, 1, -- REACTION, SELF, PHYSICAL, PHYS_ATK
0, 50, 0, 0, 0, -- DEX 50%
110, 0,
'NONE', 0, 0,
8000, 0, 0,
0, 150, 0,
0, 1, 1, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Riposte', 'VFX_Counter_Slash', 'SFX_Parry',
'Transformar defesa em punição imediata.',
'Ao esquivar, contra-ataca causando {damage} de dano.',
'["reaction", "counter", "trigger_on_dodge"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 110, "conditions_json": {"trigger": "on_dodge"}}]'),

-- 9. Respiração de Combate (Level 30)
('MONK_COMBAT_BREATH', 'Respiração de Combate', 6, 9, 30, 1, 5,
1, 1, 1, 3, -- ACTIVE, SELF, PHYSICAL, HEALTH
0, 0, 55, 0, 0, -- VIT 55%
0, 0,
'MANA', 40, 0,
30000, 1000, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
50, 100,
'Skills/Monk/T_Combat_Breath', 'VFX_Heal_Self', 'SFX_Deep_Breath',
'Técnica respiratória usada por veteranos para se manter lutando.',
'Recupera {heal}% do Health máximo.',
'["heal", "self", "recovery"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 20}]'),

-- 10. Foco de Batalha (Level 34)
('MONK_BATTLE_FOCUS', 'Foco de Batalha', 6, 10, 34, 1, 5,
3, 1, 1, 1, -- BUFF, SELF, PHYSICAL, PHYS_ATK
0, 50, 0, 0, 0, -- DEX 50%
0, 0,
'MANA', 25, 0,
20000, 400, 10000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Battle_Focus', 'VFX_Focus_Aura', 'SFX_Focus',
'Atenção total elimina margem de erro.',
'+{value} Accuracy alto por {duration}s.',
'["buff", "offensive", "accuracy"]',
'[{"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 50, "value_percent": 20, "duration_ms": 10000}]'),

-- 11. Defesa Rotacional (Level 38)
('MONK_ROTATION_DEFENSE', 'Defesa Rotacional', 6, 11, 38, 1, 5,
3, 1, 1, 5, -- BUFF, SELF, PHYSICAL, NONE
0, 50, 0, 0, 0, -- DEX 50%
0, 0,
'MANA', 30, 0,
18000, 300, 5000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Rotation_Defense', 'VFX_Spin_Dodge', 'SFX_Whoosh',
'Movimento circular que dificulta ser atingido.',
'+{value}% Dodge por {duration}s.',
'["buff", "defensive", "evasion"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_percent": 35, "duration_ms": 5000}]'),

-- 12. Execução Marcial (Level 42)
('MONK_MARTIAL_EXECUTION', 'Execução Marcial', 6, 12, 42, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
40, 0, 0, 0, 0, -- STR 40%
180, 50,
'MANA', 45, 0,
14000, 800, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Monk/T_Martial_Execution', 'VFX_Execute', 'SFX_Final_Blow',
'Finalização técnica sem hesitação.',
'Causa {damage} de dano. +{bonus}% dano se alvo abaixo de 30% HP.',
'["melee", "execute", "finisher"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 180}, {"type": "EXECUTE", "value_percent": 50, "conditions_json": {"target_health_below_percent": 30}}]'),

-- 13. Ritmo de Guerra (Level 45)
('MONK_WAR_RHYTHM', 'Ritmo de Guerra', 6, 13, 45, 1, 5,
3, 1, 1, 1, -- BUFF, SELF, PHYSICAL, PHYS_ATK
0, 55, 0, 0, 0, -- DEX 55%
0, 0,
'MANA', 40, 0,
30000, 500, 15000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_War_Rhythm', 'VFX_Combat_Flow', 'SFX_Rhythm',
'Entra no fluxo perfeito de combate.',
'+{double}% Double Attack, +{speed}% Speed por {duration}s.',
'["buff", "offensive", "combo"]',
'[{"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 25, "duration_ms": 15000}, {"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 20, "duration_ms": 15000}]'),

-- 14. Armadura Mental (Level 48)
('MONK_MENTAL_ARMOR', 'Armadura Mental', 6, 14, 48, 1, 5,
2, 1, 1, 4, -- PASSIVE, SELF, PHYSICAL, DEFENSE
0, 0, 30, 30, 0, -- VIT 30%, INT 30%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Monk/T_Mental_Armor', NULL, NULL,
'Treino contra terror e magia hostil.',
'+{value} Magic Defense permanente.',
'["passive", "defensive", "magic_resist"]',
'[{"type": "BUFF_STAT", "target_stat": "magic_defense", "value_flat": 35, "value_percent": 10}]'),

-- 15. Forma do Veterano (Level 50 - ULTIMATE)
('MONK_VETERAN_FORM', 'Forma do Veterano', 6, 15, 50, 1, 5,
6, 1, 1, 1, -- ULTIMATE, SELF, PHYSICAL, PHYS_ATK
40, 40, 30, 0, 0, -- STR 40%, DEX 40%, VIT 30%
0, 0,
'MANA', 100, 0,
180000, 1500, 20000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
150, 80,
'Skills/Monk/T_Veteran_Form', 'VFX_Ultimate_Monk', 'SFX_Ultimate_Focus',
'Aplica todas as doutrinas de guerra de uma vez.',
'Por {duration}s: +{atk}% Phys Atk, +{def}% Defense, +{acc} Accuracy, +{dodge} Dodge.',
'["ultimate", "buff", "balanced"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_attack", "value_percent": 30, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 25, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 40, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "dodge", "value_flat": 30, "duration_ms": 20000}]'),

-- ============================================================================
-- TEMPLAR (class_id = 2) — Árvore: Juramento das Cinzas
-- Arquétipo: defensora sagrada, proteção, mitigação, cura moderada
-- Escala com: Vitality + Strength + Intelligence
-- ============================================================================

-- 1. Golpe Consagrado (Level 1)
('TEMP_HOLY_STRIKE', 'Golpe Consagrado', 2, 1, 1, 1, 5,
1, 2, 4, 1, -- ACTIVE, ENEMY, HOLY, PHYS_ATK
35, 0, 0, 35, 0, -- STR 35%, INT 35%
145, 0,
'MANA', 18, 0,
4000, 500, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
120, 100,
'Skills/Templar/T_Holy_Strike', 'VFX_Holy_Slash', 'SFX_Holy_Impact',
'Ataque carregado com runas das Cinzas.',
'Causa {damage} de dano híbrido (Físico + Holy).',
'["melee", "hybrid", "holy"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 145}]'),

-- 2. Escudo de Voto (Level 4)
('TEMP_VOW_SHIELD', 'Escudo de Voto', 2, 2, 4, 1, 5,
3, 1, 4, 4, -- BUFF, SELF, HOLY, DEFENSE
0, 0, 55, 0, 0, -- VIT 55%
0, 0,
'MANA', 25, 0,
18000, 400, 10000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Vow_Shield', 'VFX_Holy_Shield', 'SFX_Shield_Up',
'Escudo energizado por juramento ritual.',
'+{value}% Physical Defense por {duration}s.',
'["buff", "defensive", "shield"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 30, "duration_ms": 10000}]'),

-- 3. Égide Espiritual (Level 7)
('TEMP_SPIRIT_AEGIS', 'Égide Espiritual', 2, 3, 7, 1, 5,
3, 1, 4, 4, -- BUFF, SELF, HOLY, DEFENSE
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 25, 0,
18000, 400, 10000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Spirit_Aegis', 'VFX_Magic_Barrier', 'SFX_Barrier',
'Proteção contra corrupção dimensional.',
'+{value}% Magic Defense por {duration}s.',
'["buff", "defensive", "magic_resist"]',
'[{"type": "BUFF_STAT", "target_stat": "magic_defense", "value_percent": 30, "duration_ms": 10000}]'),

-- 4. Luz das Cinzas (Level 10)
('TEMP_ASH_LIGHT', 'Luz das Cinzas', 2, 4, 10, 1, 5,
1, 2, 4, 2, -- ACTIVE, ENEMY, HOLY, MAG_ATK
0, 0, 0, 55, 0, -- INT 55%
160, 30,
'MANA', 30, 0,
6000, 800, 0,
0, 300, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Templar/T_Ash_Light', 'VFX_Holy_Beam', 'SFX_Holy_Blast',
'Energia purificadora rara.',
'Causa {damage} de dano Holy. +{bonus}% dano contra criaturas da Neblina.',
'["ranged", "holy", "anti_shadow"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 160}, {"type": "DAMAGE", "value_percent": 30, "conditions_json": {"target_type": "shadow_creature"}}]'),

-- 5. Aura do Guardião (Level 14)
('TEMP_GUARDIAN_AURA', 'Aura do Guardião', 2, 5, 14, 1, 5,
5, 5, 4, 4, -- AURA, PARTY, HOLY, DEFENSE
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'MANA', 0, 5,
60000, 1000, 0,
0, 0, 300,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Guardian_Aura', 'VFX_Aura_Protection', 'SFX_Aura_Hum',
'Presença protetora constante.',
'+{value} Critical Resistance para aliados na área.',
'["aura", "party", "defensive"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_resistance", "value_flat": 20, "value_percent": 10}]'),

-- 6. Passo Guardião (Level 18)
('TEMP_GUARDIAN_STEP', 'Passo Guardião', 2, 6, 18, 1, 5,
3, 1, 4, 4, -- BUFF, SELF, HOLY, DEFENSE
0, 0, 55, 0, 0, -- VIT 55%
0, 0,
'MANA', 30, 0,
15000, 300, 6000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
150, 100,
'Skills/Templar/T_Guardian_Step', 'VFX_Holy_Charge', 'SFX_Heavy_Step',
'Avança absorvendo impacto.',
'-{value}% dano recebido por {duration}s.',
'["buff", "defensive", "damage_reduction"]',
'[{"type": "BUFF_STAT", "target_stat": "damage_reduction", "value_percent": 20, "duration_ms": 6000}]'),

-- 7. Julgamento (Level 22)
('TEMP_JUDGMENT', 'Julgamento', 2, 7, 22, 1, 5,
1, 2, 4, 1, -- ACTIVE, ENEMY, HOLY, PHYS_ATK
0, 0, 0, 0, 50, -- LCK 50%
170, 0,
'MANA', 35, 0,
10000, 700, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Templar/T_Judgment', 'VFX_Holy_Judgment', 'SFX_Condemn',
'Golpe de condenação ritual.',
'Causa {damage} de dano Holy com alto crítico.',
'["melee", "holy", "high_crit"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 170}]'),

-- 8. Barreira Rúnica (Level 26)
('TEMP_RUNIC_BARRIER', 'Barreira Rúnica', 2, 8, 26, 1, 5,
3, 1, 4, 3, -- BUFF, SELF, HOLY, HEALTH
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 40, 0,
25000, 800, 10000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Runic_Barrier', 'VFX_Rune_Shield', 'SFX_Rune_Activate',
'Selo defensivo projetado.',
'Cria escudo que absorve {value} de dano por {duration}s.',
'["shield", "absorb", "defensive"]',
'[{"type": "SHIELD", "value_flat": 300, "value_percent": 20, "duration_ms": 10000}]'),

-- 9. Oração de Campo (Level 30)
('TEMP_FIELD_PRAYER', 'Oração de Campo', 2, 9, 30, 1, 5,
1, 3, 4, 3, -- ACTIVE, ALLY, HOLY, HEALTH
0, 0, 40, 40, 0, -- VIT 40%, INT 40%
0, 0,
'MANA', 45, 0,
12000, 1200, 0,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
150, 100,
'Skills/Templar/T_Field_Prayer', 'VFX_Heal_Single', 'SFX_Prayer',
'Prece curta de restauração.',
'Cura {heal}% do Health máximo do aliado.',
'["heal", "single_target", "support"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 25}]'),

-- 10. Marca do Juramento (Level 34)
('TEMP_VOW_MARK', 'Marca do Juramento', 2, 10, 34, 1, 5,
4, 2, 4, 2, -- DEBUFF, ENEMY, HOLY, MAG_ATK
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 30, 0,
15000, 600, 10000,
0, 300, 0,
0, 1, 0, 0, 0, 1, 0,
200, 100,
'Skills/Templar/T_Vow_Mark', 'VFX_Holy_Mark', 'SFX_Mark',
'Marca quem deve ser contido.',
'Reduz Attack do alvo em {value}% por {duration}s.',
'["debuff", "control", "weaken"]',
'[{"type": "DEBUFF_STAT", "target_stat": "physical_attack", "value_percent": -20, "duration_ms": 10000}, {"type": "DEBUFF_STAT", "target_stat": "magic_attack", "value_percent": -20, "duration_ms": 10000}]'),

-- 11. Resistência Devota (Level 38)
('TEMP_DEVOUT_RESISTANCE', 'Resistência Devota', 2, 11, 38, 1, 5,
2, 1, 4, 3, -- PASSIVE, SELF, HOLY, HEALTH
0, 0, 55, 0, 0, -- VIT 55%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Devout_Resistance', NULL, NULL,
'Corpo fortalecido pela fé marcial.',
'+{value} Health Máximo permanente.',
'["passive", "defensive", "health"]',
'[{"type": "BUFF_STAT", "target_stat": "max_health", "value_flat": 250, "value_percent": 15}]'),

-- 12. Interposição (Level 42)
('TEMP_INTERPOSITION', 'Interposição', 2, 12, 42, 1, 5,
7, 3, 4, 4, -- REACTION, ALLY, HOLY, DEFENSE
0, 0, 60, 0, 0, -- VIT 60%
0, 0,
'NONE', 0, 0,
30000, 0, 0,
0, 200, 0,
0, 1, 0, 0, 0, 1, 0,
300, 100,
'Skills/Templar/T_Interposition', 'VFX_Protect_Ally', 'SFX_Block',
'Proteção sacrificial.',
'Redireciona {value}% do dano de um aliado para si.',
'["reaction", "tank", "redirect"]',
'[{"type": "REFLECT", "value_percent": 50, "conditions_json": {"trigger": "ally_damaged", "redirect_to_self": true}}]'),

-- 13. Chama do Voto (Level 45)
('TEMP_VOW_FLAME', 'Chama do Voto', 2, 13, 45, 1, 5,
3, 5, 4, 1, -- BUFF, PARTY, HOLY, PHYS_ATK
45, 0, 0, 0, 0, -- STR 45%
0, 0,
'MANA', 50, 0,
35000, 1000, 15000,
0, 0, 400,
0, 1, 0, 0, 0, 0, 0,
120, 100,
'Skills/Templar/T_Vow_Flame', 'VFX_Holy_Empower', 'SFX_Fire_Holy',
'Convoca fervor de guerra.',
'+{value}% Physical Attack para aliados por {duration}s.',
'["buff", "party", "offensive"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_attack", "value_percent": 20, "duration_ms": 15000}]'),

-- 14. Escudo Inquebrável (Level 48)
('TEMP_UNBREAKABLE_SHIELD', 'Escudo Inquebrável', 2, 14, 48, 1, 5,
2, 1, 4, 4, -- PASSIVE, SELF, HOLY, DEFENSE
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Templar/T_Unbreakable_Shield', NULL, NULL,
'Defesa contra cadeias de golpes.',
'+{value} Double Attack Resistance permanente.',
'["passive", "defensive", "anti_double"]',
'[{"type": "BUFF_STAT", "target_stat": "double_attack_resistance", "value_flat": 30, "value_percent": 12}]'),

-- 15. Milagre das Cinzas (Level 50 - ULTIMATE)
('TEMP_ASH_MIRACLE', 'Milagre das Cinzas', 2, 15, 50, 1, 5,
6, 6, 4, 3, -- ULTIMATE, AREA_ALLY, HOLY, HEALTH
0, 0, 45, 40, 0, -- VIT 45%, INT 40%
0, 0,
'MANA', 120, 0,
180000, 2500, 0,
0, 0, 400,
0, 1, 0, 0, 0, 0, 0,
200, 80,
'Skills/Templar/T_Ash_Miracle', 'VFX_Ultimate_Templar', 'SFX_Miracle',
'Ritual maior templário.',
'Cura {heal}% do Health de aliados em área e concede +{def}% Defense por {duration}s.',
'["ultimate", "heal", "aoe_ally"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 35}, {"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 25, "duration_ms": 15000}, {"type": "BUFF_STAT", "target_stat": "magic_defense", "value_percent": 25, "duration_ms": 15000}]'),

-- ============================================================================
-- CLERIC/ALCHEMIST (class_id = 4) — Árvore: Fórmulas de Guerra (SUPORTE)
-- Arquétipo: suporte pleno — buffs, cura, mitigação, preparo
-- Escala com: Intelligence + Luck + Vitality
-- ============================================================================

-- 1. Tônico de Campo (Level 1)
('ALCH_FIELD_TONIC', 'Tônico de Campo', 4, 1, 1, 1, 5,
1, 3, 5, 3, -- ACTIVE, ALLY, POISON (alchemical), HEALTH
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 20, 0,
6000, 600, 0,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
150, 100,
'Skills/Alchemist/T_Field_Tonic', 'VFX_Heal_Potion', 'SFX_Potion',
'Tratamento rápido com reagentes instáveis.',
'Cura {heal}% do Health máximo do aliado.',
'["heal", "single_target", "support"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 20}]'),

-- 2. Elixir de Força (Level 4)
('ALCH_STRENGTH_ELIXIR', 'Elixir de Força', 4, 2, 4, 1, 5,
3, 3, 5, 5, -- BUFF, ALLY, POISON, NONE
50, 0, 0, 0, 0, -- STR 50% (concedido)
0, 0,
'MANA', 25, 0,
20000, 500, 60000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Strength_Elixir', 'VFX_Buff_Red', 'SFX_Drink',
'Mistura que estimula fibras musculares.',
'+{value} Strength para aliado por {duration}s.',
'["buff", "ally", "strength"]',
'[{"type": "BUFF_STAT", "target_stat": "strength", "value_flat": 30, "value_percent": 15, "duration_ms": 60000}]'),

-- 3. Soro de Precisão (Level 7)
('ALCH_PRECISION_SERUM', 'Soro de Precisão', 4, 3, 7, 1, 5,
3, 3, 5, 5, -- BUFF, ALLY, POISON, NONE
0, 50, 0, 0, 0, -- DEX 50% (concedido)
0, 0,
'MANA', 25, 0,
20000, 500, 60000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Precision_Serum', 'VFX_Buff_Green', 'SFX_Drink',
'Ajusta reflexos e foco.',
'+{value} Dexterity e Accuracy para aliado por {duration}s.',
'["buff", "ally", "dexterity"]',
'[{"type": "BUFF_STAT", "target_stat": "dexterity", "value_flat": 25, "value_percent": 12, "duration_ms": 60000}, {"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 20, "duration_ms": 60000}]'),

-- 4. Composto Revigorante (Level 10)
('ALCH_VIGOR_COMPOUND', 'Composto Revigorante', 4, 4, 10, 1, 5,
3, 3, 5, 5, -- BUFF, ALLY, POISON, NONE
0, 0, 50, 0, 0, -- VIT 50% (concedido)
0, 0,
'MANA', 25, 0,
20000, 500, 60000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Vigor_Compound', 'VFX_Buff_Orange', 'SFX_Drink',
'Endurece resistência corporal.',
'+{value} Vitality e Max Health para aliado por {duration}s.',
'["buff", "ally", "vitality"]',
'[{"type": "BUFF_STAT", "target_stat": "vitality", "value_flat": 25, "value_percent": 12, "duration_ms": 60000}, {"type": "BUFF_STAT", "target_stat": "max_health", "value_flat": 100, "duration_ms": 60000}]'),

-- 5. Infusão Cognitiva (Level 14)
('ALCH_COGNITIVE_INFUSION', 'Infusão Cognitiva', 4, 5, 14, 1, 5,
3, 3, 8, 5, -- BUFF, ALLY, ARCANE, NONE
0, 0, 0, 50, 0, -- INT 50% (concedido)
0, 0,
'MANA', 25, 0,
20000, 500, 60000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Cognitive_Infusion', 'VFX_Buff_Blue', 'SFX_Drink',
'Clareza mental alquímica.',
'+{value} Intelligence e Magic Attack para aliado por {duration}s.',
'["buff", "ally", "intelligence"]',
'[{"type": "BUFF_STAT", "target_stat": "intelligence", "value_flat": 25, "value_percent": 12, "duration_ms": 60000}, {"type": "BUFF_STAT", "target_stat": "magic_attack", "value_flat": 20, "duration_ms": 60000}]'),

-- 6. Destilado da Fortuna (Level 18)
('ALCH_FORTUNE_DISTILLATE', 'Destilado da Fortuna', 4, 6, 18, 1, 5,
3, 3, 8, 5, -- BUFF, ALLY, ARCANE, NONE
0, 0, 0, 0, 50, -- LCK 50% (concedido)
0, 0,
'MANA', 30, 0,
25000, 500, 60000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Fortune_Distillate', 'VFX_Buff_Gold', 'SFX_Drink',
'Catalisador probabilístico raro.',
'+{value} Luck e Critical Chance para aliado por {duration}s.',
'["buff", "ally", "luck"]',
'[{"type": "BUFF_STAT", "target_stat": "luck", "value_flat": 25, "value_percent": 15, "duration_ms": 60000}, {"type": "BUFF_STAT", "target_stat": "critical_chance", "value_flat": 15, "duration_ms": 60000}]'),

-- 7. Névoa Restauradora (Level 22)
('ALCH_RESTORING_MIST', 'Névoa Restauradora', 4, 7, 22, 1, 5,
1, 6, 5, 3, -- ACTIVE, AREA_ALLY, POISON, HEALTH
0, 0, 0, 55, 0, -- INT 55%
0, 0,
'MANA', 50, 0,
18000, 1000, 0,
0, 0, 300,
0, 1, 0, 0, 0, 0, 0,
180, 100,
'Skills/Alchemist/T_Restoring_Mist', 'VFX_Heal_AoE', 'SFX_Mist',
'Vapores curativos.',
'Cura {heal}% do Health de aliados em área.',
'["heal", "aoe_ally", "support"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 18}]'),

-- 8. Revestimento Protetor (Level 26)
('ALCH_PROTECTIVE_COAT', 'Revestimento Protetor', 4, 8, 26, 1, 5,
3, 3, 5, 4, -- BUFF, ALLY, POISON, DEFENSE
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'MANA', 35, 0,
22000, 600, 30000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Protective_Coat', 'VFX_Buff_Shield', 'SFX_Apply',
'Camada química protetora.',
'+{phys}% Physical Defense e +{mag}% Magic Defense para aliado por {duration}s.',
'["buff", "ally", "defensive"]',
'[{"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 20, "duration_ms": 30000}, {"type": "BUFF_STAT", "target_stat": "magic_defense", "value_percent": 20, "duration_ms": 30000}]'),

-- 9. Estimulante de Combate (Level 30)
('ALCH_COMBAT_STIMULANT', 'Estimulante de Combate', 4, 9, 30, 1, 5,
3, 3, 5, 5, -- BUFF, ALLY, POISON, NONE
0, 45, 0, 0, 0, -- DEX 45%
0, 0,
'MANA', 35, 0,
25000, 500, 20000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Combat_Stimulant', 'VFX_Speed_Buff', 'SFX_Inject',
'Acelera respostas neurais.',
'+{speed}% Movement Speed e +{double}% Double Attack para aliado por {duration}s.',
'["buff", "ally", "offensive"]',
'[{"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 25, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 20, "duration_ms": 20000}]'),

-- 10. Neutralizador (Level 34)
('ALCH_NEUTRALIZER', 'Neutralizador', 4, 10, 34, 1, 5,
1, 3, 5, 5, -- ACTIVE, ALLY, POISON, NONE
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 40, 0,
15000, 300, 0,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Neutralizer', 'VFX_Cleanse', 'SFX_Cleanse',
'Antídoto universal parcial.',
'Remove até {value} debuffs do aliado.',
'["cleanse", "support", "dispel"]',
'[{"type": "CLEANSE", "value_flat": 3}]'),

-- 11. Ampola de Estabilização (Level 38)
('ALCH_STABILIZATION_VIAL', 'Ampola de Estabilização', 4, 11, 38, 1, 5,
3, 3, 5, 3, -- BUFF, ALLY, POISON, HEALTH
0, 0, 50, 0, 0, -- VIT 50%
0, 0,
'MANA', 45, 0,
20000, 500, 8000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Stabilization_Vial', 'VFX_Shield_Ally', 'SFX_Shield',
'Gel reativo absorvedor.',
'Cria escudo que absorve {value} de dano no aliado por {duration}s.',
'["shield", "ally", "absorb"]',
'[{"type": "SHIELD", "value_flat": 350, "value_percent": 25, "duration_ms": 8000}]'),

-- 12. Catalisador de Mana (Level 42)
('ALCH_MANA_CATALYST', 'Catalisador de Mana', 4, 12, 42, 1, 5,
1, 3, 8, 5, -- ACTIVE, ALLY, ARCANE, NONE
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'MANA', 30, 0,
30000, 600, 10000,
0, 250, 0,
0, 1, 0, 0, 0, 1, 0,
100, 100,
'Skills/Alchemist/T_Mana_Catalyst', 'VFX_Mana_Restore', 'SFX_Magic',
'Recarrega fluxo arcano.',
'Regenera {value}% do Mana máximo do aliado ao longo de {duration}s.',
'["resource", "ally", "mana"]',
'[{"type": "HOT", "target_stat": "mana", "value_percent": 30, "duration_ms": 10000, "tick_interval_ms": 2000}]'),

-- 13. Bomba Disruptiva (Level 45)
('ALCH_DISRUPTIVE_BOMB', 'Bomba Disruptiva', 4, 13, 45, 1, 5,
4, 4, 5, 2, -- DEBUFF, AREA, POISON, MAG_ATK
0, 0, 0, 0, 50, -- LCK 50%
60, 0,
'MANA', 45, 0,
20000, 800, 8000,
0, 300, 200,
0, 1, 0, 0, 0, 1, 0,
80, 100,
'Skills/Alchemist/T_Disruptive_Bomb', 'VFX_Smoke_Bomb', 'SFX_Explosion',
'Explosão química cegante.',
'Causa {damage} de dano e reduz Accuracy e Critical em {debuff}% dos inimigos por {duration}s.',
'["debuff", "aoe", "control"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 60}, {"type": "DEBUFF_STAT", "target_stat": "accuracy", "value_percent": -25, "duration_ms": 8000}, {"type": "DEBUFF_STAT", "target_stat": "critical_chance", "value_percent": -20, "duration_ms": 8000}]'),

-- 14. Preparação Antecipada (Level 48)
('ALCH_ADVANCED_PREP', 'Preparação Antecipada', 4, 14, 48, 1, 5,
2, 1, 5, 5, -- PASSIVE, SELF, POISON, NONE
0, 0, 0, 50, 0, -- INT 50%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Alchemist/T_Advanced_Prep', NULL, NULL,
'Planejamento alquímico.',
'Buffs aplicados duram {value}% mais tempo.',
'["passive", "support", "buff_duration"]',
'[{"type": "BUFF_STAT", "target_stat": "buff_duration", "value_percent": 25}]'),

-- 15. Protocolo de Sobrevivência (Level 50 - ULTIMATE)
('ALCH_SURVIVAL_PROTOCOL', 'Protocolo de Sobrevivência', 4, 15, 50, 1, 5,
6, 5, 5, 3, -- ULTIMATE, PARTY, POISON, HEALTH
0, 0, 40, 45, 25, -- VIT 40%, INT 45%, LCK 25%
0, 0,
'MANA', 130, 0,
180000, 2000, 0,
0, 0, 500,
0, 1, 0, 0, 0, 0, 0,
250, 80,
'Skills/Alchemist/T_Survival_Protocol', 'VFX_Ultimate_Alchemist', 'SFX_Ultimate_Support',
'Sequência completa de suporte.',
'Cura {heal}% do Health de aliados e concede +{def}% Defense, +{res} Resistances por {duration}s.',
'["ultimate", "party", "full_support"]',
'[{"type": "HEAL", "target_stat": "health", "value_percent": 30}, {"type": "BUFF_STAT", "target_stat": "physical_defense", "value_percent": 25, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "magic_defense", "value_percent": 25, "duration_ms": 20000}, {"type": "BUFF_STAT", "target_stat": "all_resistance", "value_flat": 20, "duration_ms": 20000}]'),

-- ============================================================================
-- ASSASSIN (class_id = 5) — Árvore: Doutrina do Silêncio Carmesim
-- Arquétipo: burst, crítico, mobilidade, eliminação
-- Escala com: Dexterity + Luck
-- ============================================================================

-- 1. Golpe nas Sombras (Level 1)
('ASSN_SHADOW_STRIKE', 'Golpe nas Sombras', 5, 1, 1, 1, 5,
1, 2, 2, 1, -- ACTIVE, ENEMY, SHADOW, PHYS_ATK
0, 0, 0, 0, 55, -- LCK 55%
160, 0,
'MANA', 15, 0,
4000, 300, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
80, 100,
'Skills/Assassin/T_Shadow_Strike', 'VFX_Stealth_Hit', 'SFX_Swift_Stab',
'Ataque invisível inicial.',
'Causa {damage} de dano com alto crítico.',
'["melee", "crit", "opener"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 160}]'),

-- 2. Passo Invisível (Level 4)
('ASSN_INVISIBLE_STEP', 'Passo Invisível', 5, 2, 4, 1, 5,
3, 1, 2, 5, -- BUFF, SELF, SHADOW, NONE
0, 50, 0, 0, 0, -- DEX 50%
0, 0,
'MANA', 20, 0,
15000, 0, 5000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
100, 100,
'Skills/Assassin/T_Invisible_Step', 'VFX_Stealth', 'SFX_Vanish',
'Movimento fora da linha de visão.',
'+{value}% Dodge por {duration}s.',
'["buff", "evasion", "stealth"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_percent": 40, "duration_ms": 5000}]'),

-- 3. Lâmina Precisa (Level 7)
('ASSN_PRECISE_BLADE', 'Lâmina Precisa', 5, 3, 7, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
0, 55, 0, 0, 0, -- DEX 55%
140, 30,
'MANA', 18, 0,
3500, 250, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Assassin/T_Precise_Blade', 'VFX_Quick_Slash', 'SFX_Sharp_Cut',
'Ajuste fino de golpe.',
'Causa {damage} de dano com +{accuracy} Accuracy.',
'["melee", "precise", "accurate"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 140}, {"type": "BUFF_STAT", "target_stat": "accuracy", "value_flat": 30, "duration_ms": 0}]'),

-- 4. Ataque Duplo (Level 10)
('ASSN_DOUBLE_ATTACK', 'Ataque Duplo', 5, 4, 10, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
0, 55, 0, 0, 0, -- DEX 55%
110, 0,
'MANA', 22, 0,
5000, 300, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Assassin/T_Double_Attack', 'VFX_Dual_Strike', 'SFX_Double_Hit',
'Sequência rápida.',
'Ataca duas vezes causando {damage} de dano cada.',
'["melee", "multi_hit", "combo"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 110}, {"type": "DAMAGE", "target_stat": "health", "value_percent": 110}]'),

-- 5. Corte de Tendão (Level 14)
('ASSN_TENDON_CUT', 'Corte de Tendão', 5, 5, 14, 1, 5,
1, 2, 1, 1, -- ACTIVE, ENEMY, PHYSICAL, PHYS_ATK
0, 50, 0, 0, 0, -- DEX 50%
120, 0,
'MANA', 25, 0,
10000, 400, 6000,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Assassin/T_Tendon_Cut', 'VFX_Cripple', 'SFX_Slice',
'Imobiliza a presa.',
'Causa {damage} de dano e reduz Movement Speed em {slow}% por {duration}s.',
'["melee", "slow", "control"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 120}, {"type": "SLOW", "value_percent": -40, "duration_ms": 6000}]'),

-- 6. Sangramento Profundo (Level 18)
('ASSN_DEEP_BLEED', 'Sangramento Profundo', 5, 6, 18, 1, 5,
8, 2, 1, 1, -- DOT, ENEMY, PHYSICAL, PHYS_ATK
0, 0, 0, 0, 50, -- LCK 50%
40, 70,
'MANA', 28, 0,
12000, 500, 10000,
0, 150, 0,
1, 3, 0, 0, 0, 1, 0,
100, 100,
'Skills/Assassin/T_Deep_Bleed', 'VFX_Bleed_Heavy', 'SFX_Deep_Cut',
'Ferida que não fecha.',
'Aplica sangramento: {dot} dano a cada {tick}s por {duration}s.',
'["dot", "bleed", "sustained"]',
'[{"type": "DOT", "target_stat": "health", "value_percent": 70, "duration_ms": 10000, "tick_interval_ms": 2000}]'),

-- 7. Reflexos Mortais (Level 22)
('ASSN_DEADLY_REFLEXES', 'Reflexos Mortais', 5, 7, 22, 1, 5,
7, 1, 2, 5, -- REACTION, SELF, SHADOW, NONE
0, 55, 0, 0, 0, -- DEX 55%
0, 0,
'NONE', 0, 0,
15000, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Assassin/T_Deadly_Reflexes', 'VFX_Dodge_Flash', 'SFX_Whoosh',
'Desvio instantâneo.',
'Esquiva automaticamente do próximo ataque.',
'["reaction", "evasion", "survival"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_percent": 100, "duration_ms": 500, "conditions_json": {"trigger": "on_attack_received", "max_uses": 1}}]'),

-- 8. Caçador Noturno (Level 26)
('ASSN_NIGHT_HUNTER', 'Caçador Noturno', 5, 8, 26, 1, 5,
2, 1, 2, 1, -- PASSIVE, SELF, SHADOW, PHYS_ATK
0, 0, 0, 0, 50, -- LCK 50%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Assassin/T_Night_Hunter', NULL, NULL,
'Especialista em morte rápida.',
'+{value} Critical Attack permanente.',
'["passive", "offensive", "crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_chance", "value_flat": 25, "value_percent": 10}]'),

-- 9. Recuperação Sombria (Level 30)
('ASSN_SHADOW_RECOVERY', 'Recuperação Sombria', 5, 9, 30, 1, 5,
2, 1, 2, 3, -- PASSIVE, SELF, SHADOW, HEALTH
0, 0, 40, 0, 0, -- VIT 40%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Assassin/T_Shadow_Recovery', NULL, NULL,
'Roubo de vitalidade.',
'Ataques curam {value}% do dano causado.',
'["passive", "lifesteal", "sustain"]',
'[{"type": "LIFESTEAL", "value_percent": 8}]'),

-- 10. Véu de Fumaça (Level 34)
('ASSN_SMOKE_VEIL', 'Véu de Fumaça', 5, 10, 34, 1, 5,
3, 1, 2, 5, -- BUFF, SELF, SHADOW, NONE
0, 55, 0, 0, 0, -- DEX 55%
0, 0,
'MANA', 30, 0,
25000, 0, 4000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
100, 100,
'Skills/Assassin/T_Smoke_Veil', 'VFX_Smoke_Cloud', 'SFX_Smoke',
'Desaparecimento tático.',
'+{value}% Dodge muito alto por {duration}s.',
'["buff", "evasion", "escape"]',
'[{"type": "BUFF_STAT", "target_stat": "dodge", "value_percent": 60, "duration_ms": 4000}]'),

-- 11. Execução (Level 38)
('ASSN_EXECUTION', 'Execução', 5, 11, 38, 1, 5,
1, 2, 2, 1, -- ACTIVE, ENEMY, SHADOW, PHYS_ATK
0, 0, 0, 0, 55, -- LCK 55%
200, 80,
'MANA', 50, 0,
20000, 600, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
100, 100,
'Skills/Assassin/T_Execution', 'VFX_Execute_Shadow', 'SFX_Execute',
'Finalização precisa.',
'Causa {damage} de dano. +{bonus}% dano se alvo abaixo de 25% HP.',
'["melee", "execute", "finisher"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 200}, {"type": "EXECUTE", "value_percent": 80, "conditions_json": {"target_health_below_percent": 25}}]'),

-- 12. Olho do Predador (Level 42)
('ASSN_PREDATOR_EYE', 'Olho do Predador', 5, 12, 42, 1, 5,
3, 1, 2, 1, -- BUFF, SELF, SHADOW, PHYS_ATK
0, 50, 0, 0, 0, -- DEX 50%
0, 0,
'MANA', 35, 0,
30000, 300, 10000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Assassin/T_Predator_Eye', 'VFX_Eye_Glow', 'SFX_Focus',
'Nada escapa.',
'Ataques ignoram {value}% do Dodge inimigo por {duration}s.',
'["buff", "offensive", "anti_dodge"]',
'[{"type": "BUFF_STAT", "target_stat": "ignore_dodge", "value_percent": 50, "duration_ms": 10000}]'),

-- 13. Passo Entre Golpes (Level 45)
('ASSN_STEP_BETWEEN', 'Passo Entre Golpes', 5, 13, 45, 1, 5,
3, 1, 2, 5, -- BUFF, SELF, SHADOW, NONE
0, 55, 0, 0, 0, -- DEX 55%
0, 0,
'MANA', 35, 0,
25000, 0, 8000,
0, 0, 0,
0, 1, 0, 0, 0, 0, 1,
100, 100,
'Skills/Assassin/T_Step_Between', 'VFX_Phase_Move', 'SFX_Blink',
'Reposicionamento letal.',
'+{speed}% Movement Speed e +{double}% Double Attack por {duration}s.',
'["buff", "mobility", "offensive"]',
'[{"type": "BUFF_STAT", "target_stat": "movement_speed", "value_percent": 35, "duration_ms": 8000}, {"type": "BUFF_STAT", "target_stat": "double_attack_rate", "value_percent": 25, "duration_ms": 8000}]'),

-- 14. Nervos Frios (Level 48)
('ASSN_COLD_NERVES', 'Nervos Frios', 5, 14, 48, 1, 5,
2, 1, 2, 4, -- PASSIVE, SELF, SHADOW, DEFENSE
0, 0, 45, 0, 0, -- VIT 45%
0, 0,
'NONE', 0, 0,
0, 0, 0,
0, 0, 0,
0, 1, 0, 0, 0, 0, 0,
100, 100,
'Skills/Assassin/T_Cold_Nerves', NULL, NULL,
'Controle emocional absoluto.',
'+{value} Critical Resistance permanente.',
'["passive", "defensive", "anti_crit"]',
'[{"type": "BUFF_STAT", "target_stat": "critical_resistance", "value_flat": 30, "value_percent": 10}]'),

-- 15. Assassinato Perfeito (Level 50 - ULTIMATE)
('ASSN_PERFECT_KILL', 'Assassinato Perfeito', 5, 15, 50, 1, 5,
6, 2, 2, 1, -- ULTIMATE, ENEMY, SHADOW, PHYS_ATK
0, 50, 0, 0, 40, -- DEX 50%, LCK 40%
280, 0,
'MANA', 100, 0,
180000, 500, 0,
0, 150, 0,
0, 1, 1, 0, 0, 1, 0,
120, 80,
'Skills/Assassin/T_Perfect_Kill', 'VFX_Ultimate_Assassin', 'SFX_Ultimate_Strike',
'Execução sem erro.',
'Sequência de ataques garantindo crítico. Causa {damage} de dano com 100% Critical.',
'["ultimate", "burst", "guaranteed_crit"]',
'[{"type": "DAMAGE", "target_stat": "health", "value_percent": 280, "conditions_json": {"guaranteed_crit": true}}]');

-- ============================================================================
-- ÍNDICES ADICIONAIS PARA PERFORMANCE
-- ============================================================================

-- Índice para busca de skills por classe e nível
CREATE INDEX IF NOT EXISTS idx_skills_class_level ON skills (class_id, required_level);

-- Índice para busca de skills habilitadas
CREATE INDEX IF NOT EXISTS idx_skills_enabled ON skills (is_enabled, class_id);

-- ============================================================================
-- TRIGGERS PARA MANUTENÇÃO
-- ============================================================================

DELIMITER //

-- Trigger para criar registro de skill points quando jogador é criado
DROP TRIGGER IF EXISTS trg_create_skill_points//
CREATE TRIGGER trg_create_skill_points
AFTER INSERT ON players
FOR EACH ROW
BEGIN
    INSERT INTO player_skill_points (player_id, total_points_earned, points_spent, points_available)
    VALUES (NEW.id, NEW.level * 3, 0, NEW.level * 3);
    
    -- Criar slots vazios na skillbar
    INSERT INTO player_skillbar (player_id, slot_index)
    SELECT NEW.id, n
    FROM (
        SELECT 0 AS n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4
        UNION SELECT 5 UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9
        UNION SELECT 10 UNION SELECT 11 UNION SELECT 12 UNION SELECT 13 UNION SELECT 14
        UNION SELECT 15 UNION SELECT 16 UNION SELECT 17 UNION SELECT 18 UNION SELECT 19
    ) AS slots;
    
    -- Criar registro de resistências
    INSERT INTO player_resistances (player_id)
    VALUES (NEW.id);
END//

-- Trigger para atualizar skill points quando jogador sobe de nível
DROP TRIGGER IF EXISTS trg_update_skill_points_on_level//
CREATE TRIGGER trg_update_skill_points_on_level
AFTER UPDATE ON players
FOR EACH ROW
BEGIN
    IF NEW.level > OLD.level THEN
        UPDATE player_skill_points
        SET total_points_earned = NEW.level * 3,
            points_available = (NEW.level * 3) - points_spent
        WHERE player_id = NEW.id;
    END IF;
END//

DELIMITER ;

SET FOREIGN_KEY_CHECKS = 1;

-- ============================================================================
-- VERIFICAÇÃO FINAL
-- ============================================================================

SELECT 
    c.class_name,
    COUNT(s.skill_id) as total_skills,
    MIN(s.required_level) as min_level,
    MAX(s.required_level) as max_level
FROM skills s
JOIN classes c ON s.class_id = c.class_id
GROUP BY c.class_id, c.class_name
ORDER BY c.class_id;

-- Deve retornar 6 classes com 15 skills cada, níveis 1-50

-- ============================================================================
-- FIM DA INSERÇÃO DE SKILLS
-- ============================================================================
