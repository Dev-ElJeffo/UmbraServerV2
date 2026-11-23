-- =============================================================================
-- Padrão de descrição das skills (docs_main/GUIA_SKILLS_DESCRICAO_UE561.md)
--
-- description      = RESUMO: uma frase (até ~90 caracteres), sem números, dizendo o que a
--                    skill faz. Os números ficam na lista de efeitos, gerada pelo UE a
--                    partir do effects_json (sempre corretos e já com o rank).
-- tooltip_template = NOTA opcional (regra que os efeitos não explicam). Os templates
--                    antigos liam secondary_coef em vez do efeito real e ficam NULL.
--
-- Backup antes de alterar: tabela skills_desc_backup (skill_id, description, tooltip_template).
-- Desfazer:
--   UPDATE skills s JOIN skills_desc_backup b ON b.skill_id = s.skill_id
--      SET s.description = b.description, s.tooltip_template = b.tooltip_template;
--
-- Idempotente: o backup só é criado na primeira execução; os UPDATEs podem rodar de novo.
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS skills_desc_backup AS
  SELECT skill_id, description, tooltip_template FROM skills;

-- Ataques básicos (todas as classes).
UPDATE skills SET description = 'Ataque básico com a arma. Sempre disponível e sem custo de mana.',
  tooltip_template = NULL WHERE is_basic_attack = 1;

-- -----------------------------------------------------------------------------
-- Barbarian
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Golpe brutal que fere o alvo e acelera seus próximos ataques.', tooltip_template = NULL WHERE skill_key = 'BARB_RUIN_STRIKE';
UPDATE skills SET description = 'Pele endurecida que reforça sua defesa física e a resistência a críticos.', tooltip_template = NULL WHERE skill_key = 'BARB_HARDENED_SKIN';
UPDATE skills SET description = 'Avança contra o alvo, golpeia e pode interromper o que ele conjura.', tooltip_template = NULL WHERE skill_key = 'BARB_WILD_CHARGE';
UPDATE skills SET description = 'Ferida, sua fúria desperta e seus golpes críticos ficam mais frequentes.', tooltip_template = NULL WHERE skill_key = 'BARB_HOT_BLOOD';
UPDATE skills SET description = 'Grito de guerra que fortalece o ataque dos aliados próximos.', tooltip_template = NULL WHERE skill_key = 'BARB_WAR_CRY';
UPDATE skills SET description = 'Pisa com força: fere e desacelera quem está ao seu redor.', tooltip_template = NULL WHERE skill_key = 'BARB_STOMP';
UPDATE skills SET description = 'Corte profundo que fere o alvo e o deixa sangrando.', tooltip_template = NULL WHERE skill_key = 'BARB_RENDING_CUT';
UPDATE skills SET description = 'Arremessa uma corrente que fere, puxa o inimigo até você e o desacelera.', tooltip_template = NULL WHERE skill_key = 'BARB_CHAIN_THROW';
UPDATE skills SET description = 'Ignora a dor e recupera parte da sua vida na hora.', tooltip_template = NULL WHERE skill_key = 'BARB_INSTINCT_RECOVERY';
UPDATE skills SET description = 'Reação: ao sofrer um golpe crítico, contra-ataca na mesma hora.', tooltip_template = NULL WHERE skill_key = 'BARB_PRIMAL_RETALIATION';
UPDATE skills SET description = 'Troca defesa por fúria: você ataca em dobro com mais frequência.', tooltip_template = NULL WHERE skill_key = 'BARB_CRIMSON_FRENZY';
UPDATE skills SET description = 'Corpo forjado em batalha que aumenta sua vida máxima.', tooltip_template = NULL WHERE skill_key = 'BARB_BRUTAL_RESISTANCE';
UPDATE skills SET description = 'Golpe pesado que fere o alvo e quebra a postura dele, reduzindo a esquiva.', tooltip_template = NULL WHERE skill_key = 'BARB_CRUSHING_IMPACT';
UPDATE skills SET description = 'Instinto de caça que aumenta sua precisão e sua esquiva.', tooltip_template = NULL WHERE skill_key = 'BARB_RELENTLESS_HUNT';
UPDATE skills SET description = 'Vira um avatar de guerra: mais ataque, velocidade e ataques duplos.', tooltip_template = NULL WHERE skill_key = 'BARB_AVATAR_RUIN';

-- -----------------------------------------------------------------------------
-- Templar
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Golpe consagrado que fere o alvo e o queima com luz sagrada.', tooltip_template = NULL WHERE skill_key = 'TEMP_HOLY_STRIKE';
UPDATE skills SET description = 'Voto de proteção que reforça sua defesa física e mágica.', tooltip_template = NULL WHERE skill_key = 'TEMP_VOW_SHIELD';
UPDATE skills SET description = 'Provoca os inimigos ao redor para atacarem você e reduz o dano recebido.', tooltip_template = NULL WHERE skill_key = 'TEMP_CHALLENGE';
UPDATE skills SET description = 'Raio de luz que fere o alvo e enfraquece a defesa mágica dele.', tooltip_template = NULL WHERE skill_key = 'TEMP_ASH_LIGHT';
UPDATE skills SET description = 'Aura que protege o grupo contra golpes críticos.', tooltip_template = NULL WHERE skill_key = 'TEMP_GUARDIAN_AURA';
UPDATE skills SET description = 'Postura de guarda que reduz o dano que você recebe.', tooltip_template = NULL WHERE skill_key = 'TEMP_GUARDIAN_STEP';
UPDATE skills SET description = 'Golpe de condenação que causa muito dano a um único alvo.', tooltip_template = NULL WHERE skill_key = 'TEMP_JUDGMENT';
UPDATE skills SET description = 'Ergue um escudo rúnico que absorve dano.', tooltip_template = NULL WHERE skill_key = 'TEMP_RUNIC_BARRIER';
UPDATE skills SET description = 'Prece rápida de restauração que recupera vida.', tooltip_template = NULL WHERE skill_key = 'TEMP_FIELD_PRAYER';
UPDATE skills SET description = 'Marca o inimigo: enfraquece os ataques dele e pode queimá-lo aos poucos.', tooltip_template = NULL WHERE skill_key = 'TEMP_VOW_MARK';
UPDATE skills SET description = 'Fé inabalável que aumenta sua vida máxima.', tooltip_template = NULL WHERE skill_key = 'TEMP_DEVOUT_RESISTANCE';
UPDATE skills SET description = 'Reação: quando um aliado leva dano, você devolve parte dele ao agressor.', tooltip_template = NULL WHERE skill_key = 'TEMP_INTERPOSITION';
UPDATE skills SET description = 'Chama sagrada que fortalece o ataque físico e mágico do grupo.', tooltip_template = NULL WHERE skill_key = 'TEMP_VOW_FLAME';
UPDATE skills SET description = 'Escudo firme que reduz os ataques duplos que você sofre.', tooltip_template = NULL WHERE skill_key = 'TEMP_UNBREAKABLE_SHIELD';
UPDATE skills SET description = 'Milagre que cura e fortalece muito a defesa física e mágica.', tooltip_template = NULL WHERE skill_key = 'TEMP_ASH_MIRACLE';

-- -----------------------------------------------------------------------------
-- DarkMage
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Rajada de névoa carmesim que fere o alvo à distância.', tooltip_template = NULL WHERE skill_key = 'DMAGE_CRIMSON_MIST';
UPDATE skills SET description = 'Marca o inimigo e enfraquece a defesa mágica dele.', tooltip_template = NULL WHERE skill_key = 'DMAGE_DIMENSIONAL_MARK';
UPDATE skills SET description = 'Correntes de sombra que ferem o alvo e o prendem no lugar.', tooltip_template = NULL WHERE skill_key = 'DMAGE_SHADOW_CHAINS';
UPDATE skills SET description = 'Eco arcano que faz seus ataques se repetirem com mais frequência.', tooltip_template = NULL WHERE skill_key = 'DMAGE_ARCANE_ECHO';
UPDATE skills SET description = 'Selo que rompe o alvo por dentro, causando dano contínuo.', tooltip_template = NULL WHERE skill_key = 'DMAGE_RUPTURE_SEAL';
UPDATE skills SET description = 'Mente treinada que aumenta sua mana máxima.', tooltip_template = NULL WHERE skill_key = 'DMAGE_DEEP_MIND';
UPDATE skills SET description = 'Orbe corrompido que explode e fere todos os inimigos na área.', tooltip_template = NULL WHERE skill_key = 'DMAGE_CORRUPTION_ORB';
UPDATE skills SET description = 'Véu protetor que aumenta sua resistência a críticos.', tooltip_template = NULL WHERE skill_key = 'DMAGE_VEIL_REFLECTION';
UPDATE skills SET description = 'Drena o alvo: causa dano e recupera vida com parte dele.', tooltip_template = NULL WHERE skill_key = 'DMAGE_CRIMSON_DRAIN';
UPDATE skills SET description = 'Concentra energia rubi e aumenta seu ataque mágico.', tooltip_template = NULL WHERE skill_key = 'DMAGE_RUBY_CONVERGENCE';
UPDATE skills SET description = 'Distorce a realidade do alvo: ele erra mais e fica mais lento.', tooltip_template = NULL WHERE skill_key = 'DMAGE_REALITY_TWIST';
UPDATE skills SET description = 'Atravessa uma fenda e reaparece à frente.', tooltip_template = NULL WHERE skill_key = 'DMAGE_RIFT_STEP';
UPDATE skills SET description = 'Colapso arcano que causa muito dano e aumenta sua chance de crítico.', tooltip_template = NULL WHERE skill_key = 'DMAGE_ARCANE_COLLAPSE';
UPDATE skills SET description = 'Onda do vazio que fere e empurra os inimigos ao seu redor.', tooltip_template = NULL WHERE skill_key = 'DMAGE_VOID_WAVE';
UPDATE skills SET description = 'Abre um portal devastador que fere todos os inimigos na área.', tooltip_template = NULL WHERE skill_key = 'DMAGE_MINOR_PORTAL';

-- -----------------------------------------------------------------------------
-- Alchemist
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Arremessa um frasco ácido que fere o alvo e o corrói aos poucos.', tooltip_template = NULL WHERE skill_key = 'ALCH_ACID_FLASK';
UPDATE skills SET description = 'Elixir que aumenta a força, a inteligência e o crítico do aliado.', tooltip_template = NULL WHERE skill_key = 'ALCH_OFFENSIVE_ELIXIR';
UPDATE skills SET description = 'Tônico que cura os aliados próximos.', tooltip_template = NULL WHERE skill_key = 'ALCH_FIELD_TONIC';
UPDATE skills SET description = 'Elixir que aumenta a vitalidade, a vida e a defesa do aliado.', tooltip_template = NULL WHERE skill_key = 'ALCH_DEFENSIVE_ELIXIR';
UPDATE skills SET description = 'Nuvem de gás que fere e desacelera os inimigos na área.', tooltip_template = NULL WHERE skill_key = 'ALCH_PARALYZING_GAS';
UPDATE skills SET description = 'Bomba que explode em chamas e queima os inimigos na área.', tooltip_template = NULL WHERE skill_key = 'ALCH_INCENDIARY_BOMB';
UPDATE skills SET description = 'Névoa que cura os aliados ao seu redor.', tooltip_template = NULL WHERE skill_key = 'ALCH_RESTORING_MIST';
UPDATE skills SET description = 'Revestimento que reforça a defesa física e mágica do aliado.', tooltip_template = NULL WHERE skill_key = 'ALCH_PROTECTIVE_COAT';
UPDATE skills SET description = 'Estimulante que deixa o aliado mais rápido e com mais ataques duplos.', tooltip_template = NULL WHERE skill_key = 'ALCH_COMBAT_STIMULANT';
UPDATE skills SET description = 'Neutraliza venenos e maldições, removendo os efeitos negativos.', tooltip_template = NULL WHERE skill_key = 'ALCH_NEUTRALIZER';
UPDATE skills SET description = 'Ampola que envolve o aliado num escudo que absorve dano.', tooltip_template = NULL WHERE skill_key = 'ALCH_STABILIZATION_VIAL';
UPDATE skills SET description = 'Catalisador que restaura a mana do aliado aos poucos.', tooltip_template = NULL WHERE skill_key = 'ALCH_MANA_CATALYST';
UPDATE skills SET description = 'Bomba que fere e desorienta os inimigos, reduzindo precisão e crítico.', tooltip_template = NULL WHERE skill_key = 'ALCH_DISRUPTIVE_BOMB';
UPDATE skills SET description = 'Preparo cuidadoso que faz os seus buffs durarem mais.', tooltip_template = NULL WHERE skill_key = 'ALCH_ADVANCED_PREP';
UPDATE skills SET description = 'Protocolo de emergência que cura o grupo e reforça as defesas.', tooltip_template = NULL WHERE skill_key = 'ALCH_SURVIVAL_PROTOCOL';

-- -----------------------------------------------------------------------------
-- Assassin
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Golpe das sombras: crítico garantido saindo da furtividade.', tooltip_template = NULL WHERE skill_key = 'ASSN_SHADOW_STRIKE';
UPDATE skills SET description = 'Some nas sombras e se move mais rápido; atacar revela você.', tooltip_template = NULL WHERE skill_key = 'ASSN_INVISIBLE_STEP';
UPDATE skills SET description = 'Golpe preciso que fere o alvo e aumenta sua precisão.', tooltip_template = NULL WHERE skill_key = 'ASSN_PRECISE_BLADE';
UPDATE skills SET description = 'Dois cortes rápidos seguidos no mesmo alvo.', tooltip_template = NULL WHERE skill_key = 'ASSN_DOUBLE_ATTACK';
UPDATE skills SET description = 'Corta o tendão do alvo, ferindo e deixando-o lento.', tooltip_template = NULL WHERE skill_key = 'ASSN_TENDON_CUT';
UPDATE skills SET description = 'Ferida profunda que faz o alvo sangrar por vários segundos.', tooltip_template = NULL WHERE skill_key = 'ASSN_DEEP_BLEED';
UPDATE skills SET description = 'Reação: ao ser atacado, você se esquiva do golpe seguinte.', tooltip_template = NULL WHERE skill_key = 'ASSN_DEADLY_REFLEXES';
UPDATE skills SET description = 'Instinto noturno que aumenta sua chance de crítico.', tooltip_template = NULL WHERE skill_key = 'ASSN_NIGHT_HUNTER';
UPDATE skills SET description = 'Recupera vida com parte do dano que você causa.', tooltip_template = NULL WHERE skill_key = 'ASSN_SHADOW_RECOVERY';
UPDATE skills SET description = 'Bomba de fumaça que silencia e cega os inimigos ao seu redor.', tooltip_template = NULL WHERE skill_key = 'ASSN_SMOKE_BOMB';
UPDATE skills SET description = 'Golpe finalizador que causa dano extra em alvos com pouca vida.', tooltip_template = NULL WHERE skill_key = 'ASSN_EXECUTION';
UPDATE skills SET description = 'Olhar de predador: seus golpes ignoram parte da esquiva do alvo.', tooltip_template = NULL WHERE skill_key = 'ASSN_PREDATOR_EYE';
UPDATE skills SET description = 'Move-se entre os golpes: mais velocidade e mais ataques duplos.', tooltip_template = NULL WHERE skill_key = 'ASSN_STEP_BETWEEN';
UPDATE skills SET description = 'Sangue frio que aumenta sua resistência a críticos.', tooltip_template = NULL WHERE skill_key = 'ASSN_COLD_NERVES';
UPDATE skills SET description = 'O golpe perfeito: um ataque devastador que sempre é crítico.', tooltip_template = NULL WHERE skill_key = 'ASSN_PERFECT_KILL';

-- -----------------------------------------------------------------------------
-- Martial
-- -----------------------------------------------------------------------------
UPDATE skills SET description = 'Corte técnico que fere o alvo e aumenta sua precisão.', tooltip_template = NULL WHERE skill_key = 'MONK_TECH_CUT';
UPDATE skills SET description = 'Guarda alta que reforça sua defesa física.', tooltip_template = NULL WHERE skill_key = 'MONK_HIGH_GUARD';
UPDATE skills SET description = 'Sequência de golpes que fere o alvo e aumenta seus ataques duplos.', tooltip_template = NULL WHERE skill_key = 'MONK_DISCIPLINE_SEQUENCE';
UPDATE skills SET description = 'Passo rápido para se reposicionar: mais velocidade por instantes.', tooltip_template = NULL WHERE skill_key = 'MONK_TACTICAL_STEP';
UPDATE skills SET description = 'Postura treinada que aumenta sua chance de crítico.', tooltip_template = NULL WHERE skill_key = 'MONK_PRECISE_STANCE';
UPDATE skills SET description = 'Disciplina que aumenta sua resistência a críticos.', tooltip_template = NULL WHERE skill_key = 'MONK_IRON_DISCIPLINE';
UPDATE skills SET description = 'Golpe que quebra a guarda: fere, reduz a defesa e atordoa o alvo.', tooltip_template = NULL WHERE skill_key = 'MONK_BREAKER_STRIKE';
UPDATE skills SET description = 'Reação: ao se esquivar, contra-ataca na mesma hora.', tooltip_template = NULL WHERE skill_key = 'MONK_RIPOSTE';
UPDATE skills SET description = 'Respiração controlada que recupera parte da sua vida.', tooltip_template = NULL WHERE skill_key = 'MONK_COMBAT_BREATH';
UPDATE skills SET description = 'Chute giratório que fere e empurra os inimigos ao seu redor.', tooltip_template = NULL WHERE skill_key = 'MONK_SPINNING_KICK';
UPDATE skills SET description = 'Rasteira que fere o alvo e o prende no lugar.', tooltip_template = NULL WHERE skill_key = 'MONK_SWEEP';
UPDATE skills SET description = 'Golpe finalizador: dano extra em alvos feridos e cura com o dano.', tooltip_template = NULL WHERE skill_key = 'MONK_MARTIAL_EXECUTION';
UPDATE skills SET description = 'Entra no ritmo da luta: mais ataques duplos e mais velocidade.', tooltip_template = NULL WHERE skill_key = 'MONK_WAR_RHYTHM';
UPDATE skills SET description = 'Mente blindada que aumenta sua defesa mágica.', tooltip_template = NULL WHERE skill_key = 'MONK_MENTAL_ARMOR';
UPDATE skills SET description = 'Forma de veterano que fortalece ataque, defesa, precisão e esquiva.', tooltip_template = NULL WHERE skill_key = 'MONK_VETERAN_FORM';

-- Conferência: skills sem resumo ou com número no resumo (deve voltar vazio).
SELECT skill_id, skill_key, description FROM skills
 WHERE description IS NULL OR description = '' OR description REGEXP '[0-9]';
