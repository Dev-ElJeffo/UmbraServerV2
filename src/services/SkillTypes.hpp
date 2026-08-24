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
    RESOURCE_RESTORE
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
    int32_t damageReduction = 0;     // Flat reduction
    int32_t healingBonus = 0;        // Percentual
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
    nlohmann::json conditions;
};

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
    
    // Modifiers
    int16_t threatModifier = 100;
    uint8_t pvpModifier = 100;
    
    // Visual
    std::string iconPath;
    std::string vfxKey;
    std::string vfxPath;
    std::string hitVfxPath;
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

    /** power_coef efetivo: +10%/rank + power_coef_bonus da linha (se existir). */
    uint16_t getEffectivePowerCoef(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        const float multiplier = 1.0f + ((r - 1) * 0.1f);
        int32_t v = static_cast<int32_t>(std::lround(static_cast<double>(powerCoef) * multiplier));
        if (const SkillRankScaling* row = findRankScaling(r)) {
            v += static_cast<int32_t>(row->powerCoefBonus);
        }
        return static_cast<uint16_t>(std::clamp(v, 0, 65535));
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

    /** Efeitos base + extra_effects de todos os ranks <= rank (unlock cumulativo). */
    std::vector<SkillEffect> buildEffectsForRank(uint8_t rank) const {
        const uint8_t r = rank < 1 ? 1 : rank;
        std::vector<SkillEffect> out = effects;
        uint8_t order = static_cast<uint8_t>(out.size() + 1);
        for (const auto& row : rankScalings) {
            if (row.rank < 1 || row.rank > r) continue;
            for (auto eff : row.extraEffects) {
                eff.effectOrder = order++;
                out.push_back(std::move(eff));
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
