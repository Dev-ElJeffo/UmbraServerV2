#pragma once
/**
 * Umbra Eternum - Skill System Types
 * Definições de tipos e structs para o sistema de skills
 * Server-Side Authoritative Combat System
 */

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Combat {

// Forward declarations
struct CharacterState;
struct SkillData;
struct ActiveBuff;
struct DotInstance;
struct CombatResult;

// ============================================================================
// ENUMS
// ============================================================================

enum class SkillType : uint8_t {
    ACTIVE = 1,
    PASSIVE = 2,
    BUFF = 3,
    DEBUFF = 4,
    AURA = 5,
    ULTIMATE = 6,
    REACTION = 7,
    DOT = 8,
    HOT = 9
};

enum class TargetType : uint8_t {
    SELF = 1,
    ENEMY = 2,
    ALLY = 3,
    AREA = 4,
    PARTY = 5,
    AREA_ALLY = 6
};

enum class Element : uint8_t {
    PHYSICAL = 1,
    SHADOW = 2,
    FIRE = 3,
    HOLY = 4,
    POISON = 5,
    ICE = 6,
    LIGHTNING = 7,
    ARCANE = 8
};

enum class ScalingStat : uint8_t {
    PHYS_ATK = 1,
    MAG_ATK = 2,
    HEALTH = 3,
    DEFENSE = 4,
    NONE = 5
};

/** School de dano: qual ATK/DEF usar no CombatCalculator (independente do element de resistência). */
enum class DamageType : uint8_t {
    PHYSICAL = 0,
    MAGIC = 1,
    TRUE = 2
};

inline bool isTrueDamage(DamageType t) {
    return t == DamageType::TRUE;
}

/** PHYSICAL usa phys ATK/DEF; MAGIC usa mag; TRUE não usa esta helper (sem DEF). */
inline bool damageTypeUsesPhysicalAttack(DamageType t) {
    return t == DamageType::PHYSICAL;
}

inline bool damageTypeUsesMagicAttack(DamageType t) {
    return t == DamageType::MAGIC;
}

inline DamageType parseDamageTypeString(const std::string& raw) {
    if (raw == "MAGIC" || raw == "magic" || raw == "1") return DamageType::MAGIC;
    if (raw == "TRUE" || raw == "true" || raw == "2") return DamageType::TRUE;
    return DamageType::PHYSICAL;
}

enum class ResourceType : uint8_t {
    MANA = 0,
    HEALTH = 1,
    STAMINA = 2,
    NONE = 3
};

enum class EffectType : uint8_t {
    DAMAGE,
    HEAL,
    SHIELD,
    BUFF_STAT,
    DEBUFF_STAT,
    DOT,
    HOT,
    CLEANSE,
    DISPEL,
    STUN,
    SILENCE,
    SLOW,
    ROOT,
    KNOCKBACK,
    TAUNT,
    STEALTH,
    INVULNERABLE,
    LIFESTEAL,
    MANASTEAL,
    SUMMON,
    TELEPORT,
    EXECUTE,
    REFLECT,
    COOLDOWN_RESET,
    RESOURCE_RESTORE,
    INTERRUPT  // chance de cancelar o cast em curso do alvo (fase B); sempre no fim do enum
};

enum class BuffType : uint8_t {
    BUFF,
    DEBUFF,
    AURA,
    DOT,
    HOT,
    SHIELD
};

enum class CombatResultType : uint8_t {
    HIT,
    MISS,
    DODGE,
    BLOCK,
    CRITICAL,
    RESIST
};

// ============================================================================
// STATS STRUCTURE
// ============================================================================

struct CharacterStats {
    // Base attributes
    int32_t strength = 10;
    int32_t dexterity = 10;
    int32_t vitality = 10;
    int32_t intelligence = 10;
    int32_t luck = 10;
    
    // Derived stats
    int32_t maxHealth = 100;
    int32_t currentHealth = 100;
    int32_t maxMana = 100;
    int32_t currentMana = 100;
    int32_t maxStamina = 100;
    int32_t currentStamina = 100;
    
    // Combat stats
    int32_t physicalAttack = 0;
    int32_t magicAttack = 0;
    int32_t physicalDefense = 0;
    int32_t magicDefense = 0;
    int32_t accuracy = 0;
    int32_t dodge = 0;
    int32_t criticalChance = 0;      // 0-100
    int32_t criticalDamage = 150;    // 150 = 1.5x
    int32_t criticalResistance = 0;
    int32_t doubleAttackRate = 0;    // 0-100
    int32_t doubleAttackResistance = 0;
    int32_t movementSpeed = 100;     // 100 = base
    int32_t stunResist = 0;          // 0-100 vs STUN
    int32_t silenceResist = 0;
    int32_t rootResist = 0;
    int32_t slowResist = 0;
    int32_t stunChance = 0;          // 0-100 extra chance to apply STUN
    int32_t silenceChance = 0;
    int32_t rootChance = 0;
    int32_t slowChance = 0;
    
    // Resistances (percentual 0-100)
    int32_t physicalRes = 0;
    int32_t shadowRes = 0;
    int32_t fireRes = 0;
    int32_t holyRes = 0;
    int32_t poisonRes = 0;
    int32_t iceRes = 0;
    int32_t lightningRes = 0;
    int32_t arcaneRes = 0;
    
    // Modifiers
    int32_t damageReduction = 0;          // Flat reduction
    int32_t damageReductionPercent = 0;   // Incoming damage % reduction (0-90)
    int32_t healingBonus = 0;        // Percentual
    int32_t ignoreDodge = 0;         // 0-100: % da esquiva do alvo ignorada no teste de acerto
    int32_t buffDurationBonus = 0;   // % extra de duração nos buffs que este personagem aplica
    int32_t lifestealPercent = 0;    // % do dano causado (skill/ataque básico) que volta como HP
    int32_t threatModifier = 100;    // 100 = base
    int32_t pvpDamageModifier = 100; // 100 = base
    
    // Get resistance by element
    int32_t getResistance(Element element) const {
        switch (element) {
            case Element::PHYSICAL: return physicalRes;
            case Element::SHADOW: return shadowRes;
            case Element::FIRE: return fireRes;
            case Element::HOLY: return holyRes;
            case Element::POISON: return poisonRes;
            case Element::ICE: return iceRes;
            case Element::LIGHTNING: return lightningRes;
            case Element::ARCANE: return arcaneRes;
            default: return 0;
        }
    }
};

// ============================================================================
// CHARACTER STATE (Complete snapshot for combat)
// ============================================================================

struct CharacterState {
    uint64_t playerId = 0;
    uint64_t classId = 0;
    int32_t level = 1;
    std::string characterName;
    
    CharacterStats baseStats;
    CharacterStats buffedStats;  // Stats after applying buffs
    
    // Position
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    std::string zoneId;
    
    // Status flags
    bool isAlive = true;
    bool isStunned = false;
    bool isSilenced = false;
    bool isRooted = false;
    bool isInvulnerable = false;
    bool isStealthed = false;
    bool isInCombat = false;
    bool isPvPEnabled = false;
    
    // Active effects
    std::vector<ActiveBuff> activeBuffs;
    std::vector<DotInstance> activeDots;

    // Rank aprendido por skill (skill_id -> current_rank), carregado no warm do
    // estado para que loadSkillRank leia do cache e não faça SELECT no hot path.
    std::unordered_map<uint32_t, uint8_t> skillRanks;
    
    // Shield (absorb damage)
    int32_t currentShield = 0;
    int32_t maxShield = 0;
    
    // Timestamp
    std::chrono::steady_clock::time_point lastUpdate;
    
    // Helper to get effective stat value
    int32_t getEffectiveStat(const std::string& statName) const;
    
    // Check if can use skill
    bool canUseSkill() const {
        return isAlive && !isStunned && !isSilenced;
    }
    
    // Check if can move
    bool canMove() const {
        return isAlive && !isStunned && !isRooted;
    }
};

// ============================================================================
// SKILL EFFECT
// ============================================================================

struct SkillEffect {
    uint32_t effectId = 0;
    uint8_t effectOrder = 1;
    EffectType effectType = EffectType::DAMAGE;
    std::string targetStat;
    int32_t valueFlat = 0;
    int16_t valuePercent = 0;
    uint32_t durationMs = 0;
    uint32_t tickIntervalMs = 1000;
    uint8_t chancePercent = 100;
    /** 0-100: reduz a resistência de CC do alvo neste efeito. */
    uint8_t resistPenetration = 0;
    /** 0 = herda SkillData::target; demais valores usam TargetType. */
    uint8_t targetOverride = 0;
    /** -1 = herda SkillData::includeCaster, 0 = exclui, 1 = inclui. */
    int8_t includeCaster = -1;
    /** Crescimento por rank em décimos de % (125 = 12,5%/rank). -1 = herda a skill/padrão. */
    int16_t rankGrowthTenths = -1;
    nlohmann::json conditions;
};

// ============================================================================
// PROGRESSÃO POR RANK (R1)
// valor(r) = base × (1 + crescimento × (r − 1))
// ============================================================================

/** Como o rank escala um efeito: valores (dano/cura/stat) ou controle (duração + chance). */
enum class RankGrowthKind : uint8_t { None, Value, Control };

inline RankGrowthKind rankGrowthKindOf(EffectType t) {
    switch (t) {
        case EffectType::DAMAGE:
        case EffectType::HEAL:
        case EffectType::SHIELD:
        case EffectType::DOT:
        case EffectType::HOT:
        case EffectType::RESOURCE_RESTORE:
        case EffectType::EXECUTE:
        case EffectType::LIFESTEAL:
        case EffectType::MANASTEAL:
        case EffectType::BUFF_STAT:
        case EffectType::DEBUFF_STAT:
        case EffectType::SLOW:
        case EffectType::REFLECT:
            return RankGrowthKind::Value;
        case EffectType::STUN:
        case EffectType::ROOT:
        case EffectType::SILENCE:
        case EffectType::KNOCKBACK:
        case EffectType::TAUNT:
        case EffectType::INTERRUPT:
            return RankGrowthKind::Control;
        default:
            return RankGrowthKind::None;
    }
}

/** Crescimento padrão por rank, em décimos de %: 125 = +12,5%/rank (+50% no rank 5). */
inline int32_t defaultRankGrowthTenths(EffectType t) {
    switch (t) {
        case EffectType::DAMAGE:
        case EffectType::HEAL:
        case EffectType::SHIELD:
        case EffectType::DOT:
        case EffectType::HOT:
        case EffectType::RESOURCE_RESTORE:
        case EffectType::EXECUTE:
        case EffectType::LIFESTEAL:
        case EffectType::MANASTEAL:
            return 125;
        case EffectType::BUFF_STAT:
        case EffectType::DEBUFF_STAT:
        case EffectType::SLOW:
        case EffectType::REFLECT:
            return 100;
        default:
            return rankGrowthKindOf(t) == RankGrowthKind::Control ? 100 : 0;
    }
}

/** Chance extra por rank nos efeitos de controle com chance < 100. */
constexpr int32_t kRankControlChancePerRank = 5;

/** Escala v pelo rank (sinal preservado: debuff negativo cresce em módulo). */
inline int32_t scaleByRankTenths(int32_t v, int32_t growthTenths, uint8_t rank) {
    if (rank <= 1 || growthTenths <= 0 || v == 0) return v;
    const double factor = 1.0 + static_cast<double>(growthTenths) * (rank - 1) / 1000.0;
    return static_cast<int32_t>(std::lround(static_cast<double>(v) * factor));
}

/** Crescimento efetivo do efeito: efeito > skill > padrão do tipo. */
inline int32_t resolveRankGrowthTenths(const SkillEffect& e, int32_t skillGrowthTenths) {
    if (e.rankGrowthTenths >= 0) return e.rankGrowthTenths;
    if (skillGrowthTenths >= 0) return skillGrowthTenths;
    return defaultRankGrowthTenths(e.effectType);
}

/** Aplica o rank em um efeito base (não usar em extras de rank, que já valem para o rank). */
inline void applyRankToEffect(SkillEffect& e, uint8_t rank, int32_t skillGrowthTenths,
                              int32_t durationBonusMs) {
    const uint8_t r = rank < 1 ? 1 : rank;
    const int32_t g = resolveRankGrowthTenths(e, skillGrowthTenths);
    switch (rankGrowthKindOf(e.effectType)) {
        case RankGrowthKind::Value:
            e.valueFlat = scaleByRankTenths(e.valueFlat, g, r);
            e.valuePercent = static_cast<int16_t>(std::clamp(
                scaleByRankTenths(e.valuePercent, g, r), -32768, 32767));
            break;
        case RankGrowthKind::Control:
            e.durationMs = static_cast<uint32_t>(
                std::max(0, scaleByRankTenths(static_cast<int32_t>(e.durationMs), g, r)));
            if (g > 0 && e.chancePercent > 0 && e.chancePercent < 100) {
                e.chancePercent = static_cast<uint8_t>(
                    std::min(100, e.chancePercent + kRankControlChancePerRank * (r - 1)));
            }
            break;
        default:
            break;
    }
    // duration_bonus_ms da linha do rank: soma em todo efeito que já tem duração.
    if (durationBonusMs > 0 && e.durationMs > 0) {
        e.durationMs += static_cast<uint32_t>(durationBonusMs);
    }
}

// ============================================================================
// SKILL RANK SCALING (skill_rank_scaling)
// ============================================================================

struct SkillRankScaling {
    uint8_t rank = 1;
    int16_t powerCoefBonus = 0;
    int16_t resourceCostBonus = 0;
    int32_t cooldownReductionMs = 0;
    int32_t durationBonusMs = 0;
    std::vector<SkillEffect> extraEffects;
};

// ============================================================================
// SKILL DATA (Loaded from database)
// ============================================================================

struct SkillData {
    uint32_t skillId = 0;
    std::string skillKey;
    std::string skillName;
    uint64_t classId = 0;
    uint8_t skillOrder = 1;
    uint8_t requiredLevel = 1;
    uint8_t skillCost = 1;
    uint8_t maxRank = 5;
    
    SkillType type = SkillType::ACTIVE;
    TargetType target = TargetType::ENEMY;
    Element element = Element::PHYSICAL;
    ScalingStat scalingStat = ScalingStat::PHYS_ATK;
    DamageType damageType = DamageType::PHYSICAL;
    
    // Attribute scaling (0-100)
    uint8_t strScaling = 0;
    uint8_t dexScaling = 0;
    uint8_t vitScaling = 0;
    uint8_t intScaling = 0;
    uint8_t lckScaling = 0;
    
    // Power coefficients (100 = 1.0x)
    uint16_t powerCoef = 100;
    uint16_t secondaryCoef = 0;
    
    // Resource
    ResourceType resourceType = ResourceType::MANA;
    uint16_t resourceCost = 0;
    uint8_t resourceCostPercent = 0;
    
    // Timing
    uint32_t cooldownMs = 0;
    uint32_t castTimeMs = 0;
    uint32_t durationMs = 0;
    
    // Range
    uint16_t rangeMin = 0;
    uint16_t rangeMax = 100;
    uint16_t areaRadius = 0;
    
    // Flags
    bool isStackable = false;
    uint8_t maxStacks = 1;
    bool canCrit = true;
    bool ignoresDefense = false;
    bool isInterrupt = false;
    bool requiresTarget = true;
    bool canMoveWhileCasting = false;
    bool includeCaster = false;
    /** Runtime (não vem do DB): força crítico neste cálculo (conditions.guaranteed_crit). */
    bool forceCrit = false;
    /** skills.rank_growth_percent em décimos de % (-1 = padrão por tipo de efeito; 0 = não escala). */
    int16_t rankGrowthTenths = -1;

    // Modifiers
    int16_t threatModifier = 100;
    uint8_t pvpModifier = 100;
    
    // Visual
    std::string iconPath;
    std::string castAnimPath;
    std::string vfxKey;
    std::string vfxPath;
    std::string hitVfxPath;
    std::string buffVfxPath;
    std::string sfxKey;
    std::string description;
    std::string tooltipTemplate;
    
    // Effects
    std::vector<SkillEffect> effects;
    std::vector<std::string> serverTags;
    std::vector<SkillRankScaling> rankScalings;

    const SkillRankScaling* findRankScaling(uint8_t rank) const {
        for (const auto& row : rankScalings) {
            if (row.rank == rank) return &row;
        }
        return nullptr;
    }

    /** Crescimento do power_coef (mesmo do DAMAGE): skill > padrão 12,5%/rank. */
    int32_t powerCoefGrowthTenths() const {
        return rankGrowthTenths >= 0 ? rankGrowthTenths
                                     : defaultRankGrowthTenths(EffectType::DAMAGE);
    }

    /**
     * power_coef efetivo (R1): base × (1 + crescimento × (rank − 1)) + power_coef_bonus.
     * Antes eram +10%/rank implícitos SOMADOS ao bônus da tabela (contagem dupla).
     */
    uint16_t getEffectivePowerCoef(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        int32_t v = scaleByRankTenths(static_cast<int32_t>(powerCoef), powerCoefGrowthTenths(), r);
        if (const SkillRankScaling* row = findRankScaling(r)) {
            v += static_cast<int32_t>(row->powerCoefBonus);
        }
        return static_cast<uint16_t>(std::clamp(v, 0, 65535));
    }

    /**
     * Fator extra do power_coef_bonus sobre o DAMAGE já escalado pelo rank
     * (1.0 sem bônus). Os efeitos de buildEffectsForRank já trazem o crescimento.
     */
    float getRankBonusFactor(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        const SkillRankScaling* row = findRankScaling(r);
        if (!row || row->powerCoefBonus == 0 || powerCoef == 0) return 1.0f;
        const int32_t scaled =
            scaleByRankTenths(static_cast<int32_t>(powerCoef), powerCoefGrowthTenths(), r);
        if (scaled <= 0) return 1.0f;
        return std::max(0.0f, static_cast<float>(scaled + row->powerCoefBonus) /
                                  static_cast<float>(scaled));
    }

    uint16_t getEffectiveResourceCost(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        int32_t cost = static_cast<int32_t>(resourceCost);
        if (const SkillRankScaling* row = findRankScaling(r)) {
            cost += static_cast<int32_t>(row->resourceCostBonus);
        }
        return static_cast<uint16_t>(std::max(0, cost));
    }

    uint32_t getEffectiveCooldownMs(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        int64_t cd = static_cast<int64_t>(cooldownMs);
        if (const SkillRankScaling* row = findRankScaling(r)) {
            cd -= static_cast<int64_t>(row->cooldownReductionMs);
        }
        return static_cast<uint32_t>(std::max<int64_t>(0, cd));
    }

    uint32_t getEffectiveDurationMs(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        int64_t dur = static_cast<int64_t>(durationMs);
        if (const SkillRankScaling* row = findRankScaling(r)) {
            dur += static_cast<int64_t>(row->durationBonusMs);
        }
        return static_cast<uint32_t>(std::max<int64_t>(0, dur));
    }

    /**
     * Efeitos base + extra_effects dos ranks <= rank (unlock cumulativo).
     * Um extra com o mesmo tipo + target_stat de um extra de rank anterior SUBSTITUI
     * o anterior (progressão 5% -> 7% -> 10%), em vez de somar todos os ranks.
     */
    std::vector<SkillEffect> buildEffectsForRank(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        std::vector<SkillEffect> out = effects;
        const size_t baseCount = out.size();
        // R1: efeitos BASE crescem com o rank; os extras abaixo já são o valor do próprio rank.
        {
            const SkillRankScaling* current = findRankScaling(r);
            const int32_t durationBonus = current ? current->durationBonusMs : 0;
            for (auto& e : out) applyRankToEffect(e, r, rankGrowthTenths, durationBonus);
        }
        uint8_t order = static_cast<uint8_t>(out.size() + 1);
        std::vector<const SkillRankScaling*> rows;
        for (const auto& row : rankScalings) {
            if (row.rank >= 1 && row.rank <= r) rows.push_back(&row);
        }
        std::sort(rows.begin(), rows.end(),
                  [](const SkillRankScaling* a, const SkillRankScaling* b) { return a->rank < b->rank; });
        for (const SkillRankScaling* row : rows) {
            for (auto eff : row->extraEffects) {
                bool replaced = false;
                for (size_t i = baseCount; i < out.size(); ++i) {
                    if (out[i].effectType == eff.effectType && out[i].targetStat == eff.targetStat) {
                        eff.effectOrder = out[i].effectOrder;
                        out[i] = std::move(eff);
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) {
                    eff.effectOrder = order++;
                    out.push_back(std::move(eff));
                }
            }
        }
        return out;
    }
    
    // Check if skill has tag
    bool hasTag(const std::string& tag) const {
        return std::find(serverTags.begin(), serverTags.end(), tag) != serverTags.end();
    }
};

// ============================================================================
// RESOLUÇÃO DE ALVO POR EFEITO
// ============================================================================

/** Efeito que favorece quem recebe (buff, cura, escudo...). */
inline bool isBeneficialEffectType(EffectType type) {
    switch (type) {
        case EffectType::HEAL:
        case EffectType::HOT:
        case EffectType::SHIELD:
        case EffectType::BUFF_STAT:
        case EffectType::CLEANSE:
        case EffectType::STEALTH:
        case EffectType::INVULNERABLE:
        case EffectType::REFLECT:
        case EffectType::COOLDOWN_RESET:
        case EffectType::RESOURCE_RESTORE:
            return true;
        default:
            return false;
    }
}

/**
 * Alvo efetivo de um efeito: target_override explícito vence; sem override, efeito
 * benéfico numa skill hostil (ENEMY/AREA) vai para o caster (ex.: golpe que dá
 * +double attack em quem bate). Demais herdam o alvo da skill.
 */
inline TargetType resolveEffectTarget(const SkillData& skill, const SkillEffect& effect) {
    if (effect.targetOverride > 0) return static_cast<TargetType>(effect.targetOverride);
    if ((skill.target == TargetType::ENEMY || skill.target == TargetType::AREA) &&
        isBeneficialEffectType(effect.effectType)) {
        return TargetType::SELF;
    }
    return skill.target;
}

// ============================================================================
// ACTIVE BUFF
// ============================================================================

struct ActiveBuff {
    uint64_t buffId = 0;
    uint64_t targetPlayerId = 0;
    uint64_t sourcePlayerId = 0;
    uint32_t skillId = 0;
    uint32_t effectId = 0;
    
    BuffType buffType = BuffType::BUFF;
    uint8_t currentStacks = 1;
    int32_t valueSnapshot = 0;
    
    std::chrono::steady_clock::time_point startedAt;
    std::chrono::steady_clock::time_point expiresAt;
    std::chrono::steady_clock::time_point lastTickAt;
    
    bool isPermanent = false;
    
    // Snapshot of source stats at cast time (for DOT/HOT)
    nlohmann::json snapshotJson;
    
    // Effect details
    std::string affectedStat;
    int32_t flatBonus = 0;
    int16_t percentBonus = 0;
    
    bool isExpired() const {
        if (isPermanent) return false;
        return std::chrono::steady_clock::now() >= expiresAt;
    }
    
    int64_t getRemainingMs() const {
        if (isPermanent) return -1;
        auto now = std::chrono::steady_clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(expiresAt - now);
        return remaining.count();
    }
};

// ============================================================================
// DOT INSTANCE
// ============================================================================

struct DotInstance {
    uint64_t dotId = 0;
    uint64_t targetPlayerId = 0;
    uint64_t sourcePlayerId = 0;
    uint32_t skillId = 0;
    uint32_t effectId = 0;
    
    enum class DotType { DAMAGE, HEAL, MANA } dotType = DotType::DAMAGE;
    Element element = Element::PHYSICAL;
    
    int32_t tickValue = 0;
    uint32_t tickIntervalMs = 1000;
    uint8_t ticksRemaining = 0;
    
    std::chrono::steady_clock::time_point startedAt;
    std::chrono::steady_clock::time_point nextTickAt;
    std::chrono::steady_clock::time_point expiresAt;
    
    // Snapshot of source stats at cast time
    nlohmann::json snapshotJson;
    
    bool isExpired() const {
        return std::chrono::steady_clock::now() >= expiresAt;
    }
    
    bool shouldTick() const {
        return ticksRemaining > 0 && std::chrono::steady_clock::now() >= nextTickAt;
    }
};

// ============================================================================
// SKILL USE REQUEST
// ============================================================================

struct SkillUseRequest {
    std::string requestId;
    uint64_t sourcePlayerId = 0;
    uint32_t skillId = 0;
    uint8_t skillRank = 1;
    
    // Target
    std::optional<uint64_t> targetPlayerId;
    std::optional<uint64_t> targetMobId;
    
    // Position (for area skills)
    std::optional<float> targetX;
    std::optional<float> targetY;
    std::optional<float> targetZ;
    
    // Timestamp
    int64_t clientTimestamp = 0;
    int64_t serverTimestamp = 0;
    
    // Validation status
    bool isValid = false;
    std::string validationError;
};

// ============================================================================
// COMBAT RESULT
// ============================================================================

struct DamageBreakdown {
    int32_t baseDamage = 0;
    int32_t scalingBonus = 0;
    int32_t buffMultiplier = 100;      // 100 = 1.0x
    int32_t critMultiplier = 100;      // 150 = 1.5x
    int32_t pvpModifier = 100;         // 100 = 1.0x
    int32_t defenseReduction = 0;
    int32_t resistanceReduction = 0;
    int32_t shieldAbsorbed = 0;
    int32_t finalDamage = 0;
    int32_t overkill = 0;
};

struct CombatResult {
    std::string requestId;
    uint64_t sourcePlayerId = 0;
    uint64_t targetPlayerId = 0;
    uint32_t skillId = 0;
    
    CombatResultType resultType = CombatResultType::HIT;
    bool isCritical = false;
    bool isDoubleAttack = false;
    bool targetKilled = false;
    
    // Damage/Heal info
    int32_t rawValue = 0;
    int32_t finalValue = 0;
    Element element = Element::PHYSICAL;
    
    DamageBreakdown breakdown;
    
    // Effects applied
    std::vector<ActiveBuff> buffsApplied;
    std::vector<DotInstance> dotsApplied;
    
    // Resource changes
    int32_t sourceResourceCost = 0;
    int32_t sourceManaChange = 0;
    int32_t sourceHealthChange = 0;
    
    // Threat generated
    int32_t threatGenerated = 0;
    
    // Timestamp
    int64_t timestamp = 0;
    
    // Serialize for client/logging
    nlohmann::json toJson() const;
};

// ============================================================================
// THREAT ENTRY
// ============================================================================

struct ThreatEntry {
    uint64_t playerId = 0;
    std::string mobInstanceId;
    uint32_t threatValue = 0;
    std::chrono::steady_clock::time_point lastUpdate;
};

// ============================================================================
// COOLDOWN ENTRY
// ============================================================================

struct CooldownEntry {
    uint64_t playerId = 0;
    uint32_t skillId = 0;
    std::chrono::steady_clock::time_point startedAt;
    std::chrono::steady_clock::time_point expiresAt;
    
    bool isReady() const {
        return std::chrono::steady_clock::now() >= expiresAt;
    }
    
    int64_t getRemainingMs() const {
        auto now = std::chrono::steady_clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(expiresAt - now);
        return std::max(static_cast<int64_t>(0), static_cast<int64_t>(remaining.count()));
    }
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

inline std::string skillTypeToString(SkillType type) {
    switch (type) {
        case SkillType::ACTIVE: return "ACTIVE";
        case SkillType::PASSIVE: return "PASSIVE";
        case SkillType::BUFF: return "BUFF";
        case SkillType::DEBUFF: return "DEBUFF";
        case SkillType::AURA: return "AURA";
        case SkillType::ULTIMATE: return "ULTIMATE";
        case SkillType::REACTION: return "REACTION";
        case SkillType::DOT: return "DOT";
        case SkillType::HOT: return "HOT";
        default: return "UNKNOWN";
    }
}

inline std::string elementToString(Element element) {
    switch (element) {
        case Element::PHYSICAL: return "PHYSICAL";
        case Element::SHADOW: return "SHADOW";
        case Element::FIRE: return "FIRE";
        case Element::HOLY: return "HOLY";
        case Element::POISON: return "POISON";
        case Element::ICE: return "ICE";
        case Element::LIGHTNING: return "LIGHTNING";
        case Element::ARCANE: return "ARCANE";
        default: return "UNKNOWN";
    }
}

} // namespace Combat
} // namespace Umbra
