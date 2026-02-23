#pragma once
/**
 * Umbra Eternum - Combat Calculator
 * Sistema de cálculo de dano server-side authoritative
 * 
 * PRINCÍPIO FUNDAMENTAL:
 * - Cliente nunca calcula dano
 * - Cliente nunca decide crítico
 * - Cliente nunca aplica buff
 * - Servidor é a única fonte de verdade
 */

#include "SkillTypes.hpp"
#include <random>
#include <mutex>
#include <functional>

namespace Umbra {
namespace Combat {

/**
 * CombatCalculator
 * Engine de cálculo de combate centralizado
 * Thread-safe com RNG server-side
 */
class CombatCalculator {
public:
    // Singleton
    static CombatCalculator& getInstance() {
        static CombatCalculator instance;
        return instance;
    }
    
    // ========================================================================
    // FÓRMULAS DE DANO
    // ========================================================================
    
    /**
     * Calcula dano físico completo
     * 
     * Damage = (BaseStat × power_coef) × (1 + AttributeScaling) × BuffMultipliers × CritMultiplier × PvPModifier - DefenseReduction
     * 
     * @param attacker Estado do atacante
     * @param defender Estado do defensor
     * @param skill Dados da skill usada
     * @param skillRank Rank atual da skill
     * @param isPvP Se é combate PvP
     * @return Breakdown completo do dano
     */
    DamageBreakdown calculatePhysicalDamage(
        const CharacterState& attacker,
        const CharacterState& defender,
        const SkillData& skill,
        uint8_t skillRank,
        bool isPvP = false
    );
    
    /**
     * Calcula dano mágico completo
     */
    DamageBreakdown calculateMagicDamage(
        const CharacterState& attacker,
        const CharacterState& defender,
        const SkillData& skill,
        uint8_t skillRank,
        bool isPvP = false
    );
    
    /**
     * Calcula cura
     */
    int32_t calculateHeal(
        const CharacterState& healer,
        const CharacterState& target,
        const SkillData& skill,
        uint8_t skillRank
    );
    
    /**
     * Calcula valor de shield
     */
    int32_t calculateShieldValue(
        const CharacterState& caster,
        const SkillData& skill,
        uint8_t skillRank
    );
    
    // ========================================================================
    // FÓRMULAS DE HIT/MISS/CRIT
    // ========================================================================
    
    /**
     * Calcula chance de acerto
     * HitChance = BaseHit + (Accuracy - Dodge)
     * Clamped entre 5% e 95%
     */
    int32_t calculateHitChance(
        const CharacterState& attacker,
        const CharacterState& defender
    );
    
    /**
     * Rola hit (server-side RNG)
     */
    bool rollHit(int32_t hitChance);
    
    /**
     * Calcula chance de crítico
     * CritChance = AttackerCrit - TargetCritResist
     * Clamped entre 0% e 80%
     */
    int32_t calculateCritChance(
        const CharacterState& attacker,
        const CharacterState& defender
    );
    
    /**
     * Rola crítico (server-side RNG)
     */
    bool rollCrit(int32_t critChance);
    
    /**
     * Calcula chance de double attack
     */
    int32_t calculateDoubleAttackChance(
        const CharacterState& attacker,
        const CharacterState& defender
    );
    
    /**
     * Rola double attack
     */
    bool rollDoubleAttack(int32_t chance);
    
    // ========================================================================
    // FÓRMULAS DE REDUÇÃO
    // ========================================================================
    
    /**
     * Calcula redução de defesa
     * Reduction = Defense / (Defense + 100)
     * Resultado entre 0 e 0.9 (máximo 90% redução)
     */
    float calculateDefenseReduction(int32_t defense);
    
    /**
     * Calcula redução de resistência elemental
     * Cada 1% de resistência = 1% redução de dano
     * Capped em 75%
     */
    float calculateResistanceReduction(int32_t resistance);
    
    // ========================================================================
    // DOT/HOT CALCULATION
    // ========================================================================
    
    /**
     * Calcula valor do tick de DOT
     * Usa snapshot do momento do cast
     */
    int32_t calculateDotTickValue(
        const DotInstance& dot,
        const CharacterState& target
    );
    
    /**
     * Calcula valor do tick de HOT
     */
    int32_t calculateHotTickValue(
        const DotInstance& hot,
        const CharacterState& target
    );
    
    // ========================================================================
    // THREAT CALCULATION
    // ========================================================================
    
    /**
     * Calcula threat gerado
     * Threat = Damage × 1.0 (DPS)
     * HealThreat = Heal × 0.5 (Healer)
     * Multiplicado por tank modifier se aplicável
     */
    int32_t calculateThreat(
        int32_t value,
        bool isHeal,
        int32_t threatModifier,
        bool isTaunt = false
    );
    
    // ========================================================================
    // LIFESTEAL / MANASTEAL
    // ========================================================================
    
    int32_t calculateLifesteal(int32_t damageDealt, int32_t lifestealPercent);
    int32_t calculateManasteal(int32_t damageDealt, int32_t manastealPercent);
    
    // ========================================================================
    // BUFF/DEBUFF CALCULATION
    // ========================================================================
    
    /**
     * Aplica modificadores de buff a uma stat
     * Separado em: base + flat bonus, depois * percent bonus
     */
    int32_t applyBuffModifiers(
        int32_t baseStat,
        const std::vector<ActiveBuff>& buffs,
        const std::string& statName
    );
    
    /**
     * Calcula stats finais com todos os buffs aplicados
     */
    CharacterStats calculateBuffedStats(
        const CharacterStats& baseStats,
        const std::vector<ActiveBuff>& buffs
    );
    
    // ========================================================================
    // EXECUTE BONUS
    // ========================================================================
    
    /**
     * Calcula bônus de execute (dano extra em alvos com HP baixo)
     */
    float calculateExecuteBonus(
        const CharacterState& target,
        int32_t executeThreshold,  // % de HP
        int32_t executeBonusPercent
    );
    
    // ========================================================================
    // SCALING CALCULATION
    // ========================================================================
    
    /**
     * Calcula bônus de scaling por atributo
     * Cada ponto de atributo acima de 10 dá +1% do scaling
     */
    float calculateAttributeScaling(
        const CharacterState& character,
        const SkillData& skill
    );
    
    // ========================================================================
    // COMBAT RESULT GENERATION
    // ========================================================================
    
    /**
     * Processa uso de skill completo e gera resultado
     */
    CombatResult processSkillUse(
        const SkillUseRequest& request,
        CharacterState& source,
        CharacterState& target,
        const SkillData& skill
    );
    
    /**
     * Processa skill de área
     */
    std::vector<CombatResult> processAreaSkill(
        const SkillUseRequest& request,
        CharacterState& source,
        std::vector<CharacterState*>& targets,
        const SkillData& skill
    );

private:
    CombatCalculator();
    ~CombatCalculator() = default;
    CombatCalculator(const CombatCalculator&) = delete;
    CombatCalculator& operator=(const CombatCalculator&) = delete;
    
    // RNG thread-safe
    std::mt19937 rng_;
    std::mutex rngMutex_;
    
    // Roll random value between 0-99
    int32_t roll100();
    
    // Roll random value in range
    int32_t rollRange(int32_t min, int32_t max);
    
    // Clamp value
    template<typename T>
    T clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }
    
    // Constants
    static constexpr int32_t MIN_HIT_CHANCE = 5;
    static constexpr int32_t MAX_HIT_CHANCE = 95;
    static constexpr int32_t MAX_CRIT_CHANCE = 80;
    static constexpr int32_t MAX_DEFENSE_REDUCTION = 90;
    static constexpr int32_t MAX_RESISTANCE = 75;
    static constexpr int32_t BASE_CRIT_MULTIPLIER = 150; // 1.5x
    static constexpr float PVP_DAMAGE_REDUCTION = 0.7f;  // 30% redução em PvP
};

// ============================================================================
// IMPLEMENTATION (inline for header-only)
// ============================================================================

inline CombatCalculator::CombatCalculator() {
    // Initialize RNG with random seed
    std::random_device rd;
    rng_.seed(rd());
}

inline int32_t CombatCalculator::roll100() {
    std::lock_guard<std::mutex> lock(rngMutex_);
    std::uniform_int_distribution<int32_t> dist(0, 99);
    return dist(rng_);
}

inline int32_t CombatCalculator::rollRange(int32_t min, int32_t max) {
    std::lock_guard<std::mutex> lock(rngMutex_);
    std::uniform_int_distribution<int32_t> dist(min, max);
    return dist(rng_);
}

inline float CombatCalculator::calculateDefenseReduction(int32_t defense) {
    // Defense / (Defense + 100), capped at 90%
    if (defense <= 0) return 0.0f;
    float reduction = static_cast<float>(defense) / (defense + 100.0f);
    return std::min(reduction, MAX_DEFENSE_REDUCTION / 100.0f);
}

inline float CombatCalculator::calculateResistanceReduction(int32_t resistance) {
    // 1% per resistance point, capped at 75%
    return std::min(resistance, MAX_RESISTANCE) / 100.0f;
}

inline int32_t CombatCalculator::calculateHitChance(
    const CharacterState& attacker,
    const CharacterState& defender
) {
    int32_t baseHit = 80; // 80% base hit chance
    int32_t accuracyBonus = attacker.buffedStats.accuracy;
    int32_t dodgePenalty = defender.buffedStats.dodge;
    
    int32_t hitChance = baseHit + accuracyBonus - dodgePenalty;
    return clamp(hitChance, MIN_HIT_CHANCE, MAX_HIT_CHANCE);
}

inline bool CombatCalculator::rollHit(int32_t hitChance) {
    return roll100() < hitChance;
}

inline int32_t CombatCalculator::calculateCritChance(
    const CharacterState& attacker,
    const CharacterState& defender
) {
    int32_t critChance = attacker.buffedStats.criticalChance - defender.buffedStats.criticalResistance;
    return clamp(critChance, 0, MAX_CRIT_CHANCE);
}

inline bool CombatCalculator::rollCrit(int32_t critChance) {
    return roll100() < critChance;
}

inline int32_t CombatCalculator::calculateDoubleAttackChance(
    const CharacterState& attacker,
    const CharacterState& defender
) {
    int32_t chance = attacker.buffedStats.doubleAttackRate - defender.buffedStats.doubleAttackResistance;
    return clamp(chance, 0, 100);
}

inline bool CombatCalculator::rollDoubleAttack(int32_t chance) {
    return roll100() < chance;
}

inline int32_t CombatCalculator::calculateThreat(
    int32_t value,
    bool isHeal,
    int32_t threatModifier,
    bool isTaunt
) {
    if (isTaunt) {
        return value * 10; // Taunt generates massive threat
    }
    
    float baseMultiplier = isHeal ? 0.5f : 1.0f;
    float modifierMultiplier = threatModifier / 100.0f;
    
    return static_cast<int32_t>(value * baseMultiplier * modifierMultiplier);
}

inline int32_t CombatCalculator::calculateLifesteal(int32_t damageDealt, int32_t lifestealPercent) {
    return (damageDealt * lifestealPercent) / 100;
}

inline int32_t CombatCalculator::calculateManasteal(int32_t damageDealt, int32_t manastealPercent) {
    return (damageDealt * manastealPercent) / 100;
}

inline float CombatCalculator::calculateExecuteBonus(
    const CharacterState& target,
    int32_t executeThreshold,
    int32_t executeBonusPercent
) {
    float healthPercent = (static_cast<float>(target.buffedStats.currentHealth) / 
                          target.buffedStats.maxHealth) * 100.0f;
    
    if (healthPercent <= executeThreshold) {
        return 1.0f + (executeBonusPercent / 100.0f);
    }
    return 1.0f;
}

inline float CombatCalculator::calculateAttributeScaling(
    const CharacterState& character,
    const SkillData& skill
) {
    float totalScaling = 0.0f;
    const auto& stats = character.buffedStats;
    
    // Cada ponto acima de 10 contribui com o scaling%
    if (skill.strScaling > 0 && stats.strength > 10) {
        totalScaling += ((stats.strength - 10) * skill.strScaling) / 100.0f / 100.0f;
    }
    if (skill.dexScaling > 0 && stats.dexterity > 10) {
        totalScaling += ((stats.dexterity - 10) * skill.dexScaling) / 100.0f / 100.0f;
    }
    if (skill.vitScaling > 0 && stats.vitality > 10) {
        totalScaling += ((stats.vitality - 10) * skill.vitScaling) / 100.0f / 100.0f;
    }
    if (skill.intScaling > 0 && stats.intelligence > 10) {
        totalScaling += ((stats.intelligence - 10) * skill.intScaling) / 100.0f / 100.0f;
    }
    if (skill.lckScaling > 0 && stats.luck > 10) {
        totalScaling += ((stats.luck - 10) * skill.lckScaling) / 100.0f / 100.0f;
    }
    
    return totalScaling;
}

inline DamageBreakdown CombatCalculator::calculatePhysicalDamage(
    const CharacterState& attacker,
    const CharacterState& defender,
    const SkillData& skill,
    uint8_t skillRank,
    bool isPvP
) {
    DamageBreakdown breakdown;
    
    // 1. Base damage from stat
    int32_t baseStat = attacker.buffedStats.physicalAttack;
    breakdown.baseDamage = baseStat;
    
    // 2. Apply power coefficient with rank
    uint16_t effectivePowerCoef = skill.getEffectivePowerCoef(skillRank);
    int32_t scaledDamage = (baseStat * effectivePowerCoef) / 100;
    
    // 3. Apply attribute scaling
    float attrScaling = calculateAttributeScaling(attacker, skill);
    breakdown.scalingBonus = static_cast<int32_t>(scaledDamage * attrScaling);
    scaledDamage += breakdown.scalingBonus;
    
    // 4. Buff multipliers are already in buffedStats
    breakdown.buffMultiplier = 100;
    
    // 5. Critical
    int32_t critChance = calculateCritChance(attacker, defender);
    bool isCrit = skill.canCrit && rollCrit(critChance);
    breakdown.critMultiplier = isCrit ? attacker.buffedStats.criticalDamage : 100;
    scaledDamage = (scaledDamage * breakdown.critMultiplier) / 100;
    
    // 6. PvP modifier
    breakdown.pvpModifier = isPvP ? static_cast<int32_t>(PVP_DAMAGE_REDUCTION * 100) : 100;
    if (isPvP) {
        scaledDamage = (scaledDamage * skill.pvpModifier) / 100;
        scaledDamage = static_cast<int32_t>(scaledDamage * PVP_DAMAGE_REDUCTION);
    }
    
    // 7. Defense reduction (if not ignoring defense)
    if (!skill.ignoresDefense) {
        float defReduction = calculateDefenseReduction(defender.buffedStats.physicalDefense);
        breakdown.defenseReduction = static_cast<int32_t>(scaledDamage * defReduction);
        scaledDamage -= breakdown.defenseReduction;
    }
    
    // 8. Resistance reduction
    float resReduction = calculateResistanceReduction(defender.buffedStats.getResistance(skill.element));
    breakdown.resistanceReduction = static_cast<int32_t>(scaledDamage * resReduction);
    scaledDamage -= breakdown.resistanceReduction;
    
    // 9. Flat damage reduction
    scaledDamage -= defender.buffedStats.damageReduction;
    
    // 10. Shield absorption
    if (defender.currentShield > 0) {
        breakdown.shieldAbsorbed = std::min(defender.currentShield, scaledDamage);
        scaledDamage -= breakdown.shieldAbsorbed;
    }
    
    // Minimum 1 damage
    breakdown.finalDamage = std::max(1, scaledDamage);
    
    // Overkill calculation
    if (breakdown.finalDamage > defender.buffedStats.currentHealth) {
        breakdown.overkill = breakdown.finalDamage - defender.buffedStats.currentHealth;
    }
    
    return breakdown;
}

inline DamageBreakdown CombatCalculator::calculateMagicDamage(
    const CharacterState& attacker,
    const CharacterState& defender,
    const SkillData& skill,
    uint8_t skillRank,
    bool isPvP
) {
    DamageBreakdown breakdown;
    
    // Same formula but with magic stats
    int32_t baseStat = attacker.buffedStats.magicAttack;
    breakdown.baseDamage = baseStat;
    
    uint16_t effectivePowerCoef = skill.getEffectivePowerCoef(skillRank);
    int32_t scaledDamage = (baseStat * effectivePowerCoef) / 100;
    
    float attrScaling = calculateAttributeScaling(attacker, skill);
    breakdown.scalingBonus = static_cast<int32_t>(scaledDamage * attrScaling);
    scaledDamage += breakdown.scalingBonus;
    
    breakdown.buffMultiplier = 100;
    
    int32_t critChance = calculateCritChance(attacker, defender);
    bool isCrit = skill.canCrit && rollCrit(critChance);
    breakdown.critMultiplier = isCrit ? attacker.buffedStats.criticalDamage : 100;
    scaledDamage = (scaledDamage * breakdown.critMultiplier) / 100;
    
    breakdown.pvpModifier = isPvP ? static_cast<int32_t>(PVP_DAMAGE_REDUCTION * 100) : 100;
    if (isPvP) {
        scaledDamage = (scaledDamage * skill.pvpModifier) / 100;
        scaledDamage = static_cast<int32_t>(scaledDamage * PVP_DAMAGE_REDUCTION);
    }
    
    if (!skill.ignoresDefense) {
        float defReduction = calculateDefenseReduction(defender.buffedStats.magicDefense);
        breakdown.defenseReduction = static_cast<int32_t>(scaledDamage * defReduction);
        scaledDamage -= breakdown.defenseReduction;
    }
    
    float resReduction = calculateResistanceReduction(defender.buffedStats.getResistance(skill.element));
    breakdown.resistanceReduction = static_cast<int32_t>(scaledDamage * resReduction);
    scaledDamage -= breakdown.resistanceReduction;
    
    scaledDamage -= defender.buffedStats.damageReduction;
    
    if (defender.currentShield > 0) {
        breakdown.shieldAbsorbed = std::min(defender.currentShield, scaledDamage);
        scaledDamage -= breakdown.shieldAbsorbed;
    }
    
    breakdown.finalDamage = std::max(1, scaledDamage);
    
    if (breakdown.finalDamage > defender.buffedStats.currentHealth) {
        breakdown.overkill = breakdown.finalDamage - defender.buffedStats.currentHealth;
    }
    
    return breakdown;
}

inline int32_t CombatCalculator::calculateHeal(
    const CharacterState& healer,
    const CharacterState& target,
    const SkillData& skill,
    uint8_t skillRank
) {
    int32_t baseStat;
    
    switch (skill.scalingStat) {
        case ScalingStat::MAG_ATK:
            baseStat = healer.buffedStats.magicAttack;
            break;
        case ScalingStat::HEALTH:
            baseStat = target.buffedStats.maxHealth;
            break;
        default:
            baseStat = healer.buffedStats.magicAttack;
    }
    
    uint16_t effectivePowerCoef = skill.getEffectivePowerCoef(skillRank);
    int32_t healAmount = (baseStat * effectivePowerCoef) / 100;
    
    float attrScaling = calculateAttributeScaling(healer, skill);
    healAmount += static_cast<int32_t>(healAmount * attrScaling);
    
    // Apply healing bonus
    healAmount = (healAmount * (100 + target.buffedStats.healingBonus)) / 100;
    
    // Cap at missing health
    int32_t missingHealth = target.buffedStats.maxHealth - target.buffedStats.currentHealth;
    return std::min(healAmount, missingHealth);
}

inline int32_t CombatCalculator::calculateShieldValue(
    const CharacterState& caster,
    const SkillData& skill,
    uint8_t skillRank
) {
    int32_t baseStat;
    
    switch (skill.scalingStat) {
        case ScalingStat::MAG_ATK:
            baseStat = caster.buffedStats.magicAttack;
            break;
        case ScalingStat::HEALTH:
            baseStat = caster.buffedStats.maxHealth;
            break;
        case ScalingStat::DEFENSE:
            baseStat = caster.buffedStats.physicalDefense + caster.buffedStats.magicDefense;
            break;
        default:
            baseStat = caster.buffedStats.magicAttack;
    }
    
    uint16_t effectivePowerCoef = skill.getEffectivePowerCoef(skillRank);
    int32_t shieldValue = (baseStat * effectivePowerCoef) / 100;
    
    float attrScaling = calculateAttributeScaling(caster, skill);
    shieldValue += static_cast<int32_t>(shieldValue * attrScaling);
    
    return shieldValue;
}

inline int32_t CombatCalculator::calculateDotTickValue(
    const DotInstance& dot,
    const CharacterState& target
) {
    // DOT usa snapshot do cast, não recalcula
    int32_t tickDamage = dot.tickValue;
    
    // Aplicar resistência do alvo atual
    float resReduction = calculateResistanceReduction(
        target.buffedStats.getResistance(dot.element)
    );
    tickDamage -= static_cast<int32_t>(tickDamage * resReduction);
    
    return std::max(1, tickDamage);
}

inline int32_t CombatCalculator::calculateHotTickValue(
    const DotInstance& hot,
    const CharacterState& target
) {
    int32_t tickHeal = hot.tickValue;
    
    // Apply healing bonus
    tickHeal = (tickHeal * (100 + target.buffedStats.healingBonus)) / 100;
    
    // Cap at missing health
    int32_t missingHealth = target.buffedStats.maxHealth - target.buffedStats.currentHealth;
    return std::min(tickHeal, missingHealth);
}

inline int32_t CombatCalculator::applyBuffModifiers(
    int32_t baseStat,
    const std::vector<ActiveBuff>& buffs,
    const std::string& statName
) {
    int32_t flatBonus = 0;
    int32_t percentBonus = 0; // Accumulated percent bonus
    
    for (const auto& buff : buffs) {
        if (buff.isExpired()) continue;
        if (buff.affectedStat != statName) continue;
        
        flatBonus += buff.flatBonus * buff.currentStacks;
        percentBonus += buff.percentBonus * buff.currentStacks;
    }
    
    // Apply: (base + flat) * (1 + percent/100)
    int32_t withFlat = baseStat + flatBonus;
    int32_t final = (withFlat * (100 + percentBonus)) / 100;
    
    return std::max(0, final);
}

} // namespace Combat
} // namespace Umbra
