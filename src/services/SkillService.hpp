#pragma once
/**
 * Umbra Eternum - Skill Service
 * Gerenciamento de skills, buffs, DOTs e cooldowns
 * Server-Side Authoritative
 */

#include "SkillTypes.hpp"
#include "CombatCalculator.hpp"
#include "../database/MySQLConnector.hpp"
#include "../core/Logger.hpp"

#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <functional>

namespace Umbra {
namespace Combat {

// Callback types
using SkillResultCallback = std::function<void(const CombatResult&)>;
using BuffAppliedCallback = std::function<void(uint64_t targetId, const ActiveBuff&)>;
using BuffExpiredCallback = std::function<void(uint64_t targetId, uint64_t buffId)>;
using DotTickCallback = std::function<void(uint64_t targetId, const DotInstance&, int32_t value)>;

/**
 * SkillService
 * Serviço central de gerenciamento de skills
 */
class SkillService {
public:
    SkillService(std::shared_ptr<Database::MySQLConnector> db);
    ~SkillService();
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    /**
     * Carrega todas as skills do banco de dados
     */
    bool loadSkillsFromDatabase();
    bool loadNpcSkillsFromDatabase();
    
    /**
     * Recarrega skills (hot reload para balanceamento)
     */
    bool reloadSkills();
    
    // ========================================================================
    // SKILL DATA ACCESS
    // ========================================================================
    
    /**
     * Obtém dados de uma skill pelo ID
     */
    const SkillData* getSkillData(uint32_t skillId) const;
    
    /**
     * Obtém dados de uma skill pela key
     */
    const SkillData* getSkillDataByKey(const std::string& skillKey) const;
    
    /**
     * Obtém todas as skills de uma classe
     */
    std::vector<const SkillData*> getSkillsByClass(uint64_t classId) const;
    const SkillData* getNpcSkillData(uint32_t npcSkillId) const;
    
    // ========================================================================
    // SKILL VALIDATION
    // ========================================================================
    
    /**
     * Valida se jogador pode usar uma skill
     */
    struct ValidationResult {
        bool isValid = false;
        std::string errorCode;
        std::string errorMessage;
    };
    
    ValidationResult validateSkillUse(
        const CharacterState& source,
        const SkillUseRequest& request
    );
    
    /**
     * Verifica se skill está em cooldown
     */
    bool isSkillOnCooldown(uint64_t playerId, uint32_t skillId);
    
    /**
     * Obtém tempo restante de cooldown
     */
    int64_t getCooldownRemainingMs(uint64_t playerId, uint32_t skillId);
    
    // ========================================================================
    // COOLDOWN MANAGEMENT
    // ========================================================================
    
    /**
     * Registra cooldown de uma skill
     */
    void startCooldown(uint64_t playerId, uint32_t skillId, uint32_t cooldownMs);
    
    /**
     * Reseta cooldown de uma skill
     */
    void resetCooldown(uint64_t playerId, uint32_t skillId);
    
    /**
     * Reseta todos os cooldowns de um jogador
     */
    void resetAllCooldowns(uint64_t playerId);
    
    /**
     * Limpa cooldowns expirados
     */
    void cleanupExpiredCooldowns();
    
    // ========================================================================
    // BUFF MANAGEMENT
    // ========================================================================
    
    /**
     * Aplica buff a um jogador. Retorna buff_id (DB) ou 0 se falhou.
     */
    uint64_t applyBuff(
        uint64_t targetPlayerId,
        uint64_t sourcePlayerId,
        uint32_t skillId,
        const SkillEffect& effect,
        const CharacterState& sourceState
    );
    
    /**
     * Remove buff específico
     */
    void removeBuff(uint64_t playerId, uint64_t buffId);
    
    /**
     * Remove todos os buffs de uma skill
     */
    void removeBuffsBySkill(uint64_t playerId, uint32_t skillId);
    
    /**
     * Obtém buffs ativos de um jogador
     */
    std::vector<ActiveBuff> getActiveBuffs(uint64_t playerId) const;
    
    /**
     * Processa expiração de buffs (poll DB). Retorna entradas removidas.
     */
    struct BuffExpirationEntry {
        uint64_t targetPlayerId = 0;
        uint64_t buffId = 0;
        uint32_t skillId = 0;
        uint8_t buffType = 0;
    };

    std::vector<BuffExpirationEntry> processBuffExpirations();
    
    /**
     * Calcula stats com buffs aplicados
     */
    CharacterStats calculateBuffedStats(
        uint64_t playerId,
        const CharacterStats& baseStats
    ) const;
    
    // ========================================================================
    // DOT/HOT MANAGEMENT
    // ========================================================================
    
    /**
     * Aplica DOT a um jogador
     */
    void applyDot(
        uint64_t targetPlayerId,
        uint64_t sourcePlayerId,
        uint32_t skillId,
        const SkillEffect& effect,
        const CharacterState& sourceState
    );
    
    /**
     * Remove DOT específico
     */
    void removeDot(uint64_t playerId, uint64_t dotId);
    
    /**
     * Obtém DOTs ativos de um jogador
     */
    std::vector<DotInstance> getActiveDots(uint64_t playerId) const;
    
    /**
     * Processa ticks de DOT (chamar no game loop)
     * Retorna lista de (playerId, dotId, tickValue) para aplicar dano
     */
    struct DotTickResult {
        uint64_t targetPlayerId;
        uint64_t dotId;
        uint32_t skillId;
        int32_t tickValue;
        bool isDamage; // true = damage, false = heal
    };
    
    std::vector<DotTickResult> processDotTicks(
        const std::unordered_map<uint64_t, CharacterState>& playerStates
    );
    
    // ========================================================================
    // THREAT MANAGEMENT
    // ========================================================================
    
    /**
     * Adiciona threat a um mob
     */
    void addThreat(
        const std::string& mobInstanceId,
        uint64_t playerId,
        int32_t threatValue
    );
    
    /**
     * Obtém jogador com maior threat
     */
    uint64_t getHighestThreatPlayer(const std::string& mobInstanceId) const;
    
    /**
     * Obtém lista de threat ordenada
     */
    std::vector<ThreatEntry> getThreatList(const std::string& mobInstanceId) const;
    
    /**
     * Limpa threat de um mob
     */
    void clearThreat(const std::string& mobInstanceId);
    
    /**
     * Limpa threat de um jogador em todos os mobs
     */
    void clearPlayerThreat(uint64_t playerId);
    
    // ========================================================================
    // CALLBACKS
    // ========================================================================
    
    void setSkillResultCallback(SkillResultCallback callback) { skillResultCallback_ = callback; }
    void setBuffAppliedCallback(BuffAppliedCallback callback) { buffAppliedCallback_ = callback; }
    void setBuffExpiredCallback(BuffExpiredCallback callback) { buffExpiredCallback_ = callback; }
    void setDotTickCallback(DotTickCallback callback) { dotTickCallback_ = callback; }
    
    // ========================================================================
    // TICK (Call from game loop)
    // ========================================================================
    
    /**
     * Processa tick do sistema de skills
     * Chamar a cada frame/tick do servidor
     */
    void tick(const std::unordered_map<uint64_t, CharacterState>& playerStates);

private:
    std::shared_ptr<Database::MySQLConnector> db_;
    
    // Skill data cache (read-heavy, rarely written)
    mutable std::shared_mutex skillDataMutex_;
    std::unordered_map<uint32_t, SkillData> skillDataById_;
    std::unordered_map<std::string, uint32_t> skillIdByKey_;
    std::unordered_map<uint64_t, std::vector<uint32_t>> skillIdsByClass_;
    std::unordered_map<uint32_t, SkillData> npcSkillById_;
    
    // Active cooldowns
    mutable std::shared_mutex cooldownMutex_;
    std::unordered_map<uint64_t, std::unordered_map<uint32_t, CooldownEntry>> playerCooldowns_;
    
    // Active buffs
    mutable std::shared_mutex buffMutex_;
    std::unordered_map<uint64_t, std::vector<ActiveBuff>> playerBuffs_;
    uint64_t nextBuffId_ = 1;
    
    // Active DOTs
    mutable std::shared_mutex dotMutex_;
    std::unordered_map<uint64_t, std::vector<DotInstance>> playerDots_;
    uint64_t nextDotId_ = 1;
    
    // Threat tables
    mutable std::shared_mutex threatMutex_;
    std::unordered_map<std::string, std::vector<ThreatEntry>> mobThreat_;
    
    // Callbacks
    SkillResultCallback skillResultCallback_;
    BuffAppliedCallback buffAppliedCallback_;
    BuffExpiredCallback buffExpiredCallback_;
    DotTickCallback dotTickCallback_;
    
    // Helpers
    SkillEffect parseEffectFromJson(const nlohmann::json& json);
    std::vector<SkillEffect> parseEffectsFromJson(const std::string& jsonStr);
    std::vector<std::string> parseTagsFromJson(const std::string& jsonStr);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline SkillService::SkillService(std::shared_ptr<Database::MySQLConnector> db)
    : db_(db) {
}

inline SkillService::~SkillService() {
}

inline const SkillData* SkillService::getSkillData(uint32_t skillId) const {
    std::shared_lock<std::shared_mutex> lock(skillDataMutex_);
    auto it = skillDataById_.find(skillId);
    return it != skillDataById_.end() ? &it->second : nullptr;
}

inline const SkillData* SkillService::getNpcSkillData(uint32_t npcSkillId) const {
    std::shared_lock<std::shared_mutex> lock(skillDataMutex_);
    auto it = npcSkillById_.find(npcSkillId);
    return it != npcSkillById_.end() ? &it->second : nullptr;
}

inline const SkillData* SkillService::getSkillDataByKey(const std::string& skillKey) const {
    std::shared_lock<std::shared_mutex> lock(skillDataMutex_);
    auto it = skillIdByKey_.find(skillKey);
    if (it == skillIdByKey_.end()) return nullptr;
    auto dataIt = skillDataById_.find(it->second);
    return dataIt != skillDataById_.end() ? &dataIt->second : nullptr;
}

inline std::vector<const SkillData*> SkillService::getSkillsByClass(uint64_t classId) const {
    std::shared_lock<std::shared_mutex> lock(skillDataMutex_);
    std::vector<const SkillData*> result;
    
    auto it = skillIdsByClass_.find(classId);
    if (it != skillIdsByClass_.end()) {
        for (uint32_t skillId : it->second) {
            auto dataIt = skillDataById_.find(skillId);
            if (dataIt != skillDataById_.end()) {
                result.push_back(&dataIt->second);
            }
        }
    }
    
    return result;
}

inline bool SkillService::isSkillOnCooldown(uint64_t playerId, uint32_t skillId) {
    std::shared_lock<std::shared_mutex> lock(cooldownMutex_);
    
    auto playerIt = playerCooldowns_.find(playerId);
    if (playerIt == playerCooldowns_.end()) return false;
    
    auto skillIt = playerIt->second.find(skillId);
    if (skillIt == playerIt->second.end()) return false;
    
    return !skillIt->second.isReady();
}

inline int64_t SkillService::getCooldownRemainingMs(uint64_t playerId, uint32_t skillId) {
    std::shared_lock<std::shared_mutex> lock(cooldownMutex_);
    
    auto playerIt = playerCooldowns_.find(playerId);
    if (playerIt == playerCooldowns_.end()) return 0;
    
    auto skillIt = playerIt->second.find(skillId);
    if (skillIt == playerIt->second.end()) return 0;
    
    return skillIt->second.getRemainingMs();
}

inline void SkillService::startCooldown(uint64_t playerId, uint32_t skillId, uint32_t cooldownMs) {
    std::unique_lock<std::shared_mutex> lock(cooldownMutex_);
    
    CooldownEntry entry;
    entry.playerId = playerId;
    entry.skillId = skillId;
    entry.startedAt = std::chrono::steady_clock::now();
    entry.expiresAt = entry.startedAt + std::chrono::milliseconds(cooldownMs);
    
    playerCooldowns_[playerId][skillId] = entry;
}

inline void SkillService::resetCooldown(uint64_t playerId, uint32_t skillId) {
    std::unique_lock<std::shared_mutex> lock(cooldownMutex_);
    
    auto playerIt = playerCooldowns_.find(playerId);
    if (playerIt != playerCooldowns_.end()) {
        playerIt->second.erase(skillId);
    }
}

inline void SkillService::resetAllCooldowns(uint64_t playerId) {
    std::unique_lock<std::shared_mutex> lock(cooldownMutex_);
    playerCooldowns_.erase(playerId);
}

inline void SkillService::cleanupExpiredCooldowns() {
    std::unique_lock<std::shared_mutex> lock(cooldownMutex_);
    
    for (auto& [playerId, cooldowns] : playerCooldowns_) {
        for (auto it = cooldowns.begin(); it != cooldowns.end();) {
            if (it->second.isReady()) {
                it = cooldowns.erase(it);
            } else {
                ++it;
            }
        }
    }
}

inline std::vector<ActiveBuff> SkillService::getActiveBuffs(uint64_t playerId) const {
    std::shared_lock<std::shared_mutex> lock(buffMutex_);
    
    auto it = playerBuffs_.find(playerId);
    if (it == playerBuffs_.end()) return {};
    
    std::vector<ActiveBuff> result;
    for (const auto& buff : it->second) {
        if (!buff.isExpired()) {
            result.push_back(buff);
        }
    }
    return result;
}

inline std::vector<DotInstance> SkillService::getActiveDots(uint64_t playerId) const {
    std::shared_lock<std::shared_mutex> lock(dotMutex_);
    
    auto it = playerDots_.find(playerId);
    if (it == playerDots_.end()) return {};
    
    std::vector<DotInstance> result;
    for (const auto& dot : it->second) {
        if (!dot.isExpired()) {
            result.push_back(dot);
        }
    }
    return result;
}

inline void SkillService::addThreat(
    const std::string& mobInstanceId,
    uint64_t playerId,
    int32_t threatValue
) {
    std::unique_lock<std::shared_mutex> lock(threatMutex_);
    
    auto& threatList = mobThreat_[mobInstanceId];
    
    // Find existing entry
    for (auto& entry : threatList) {
        if (entry.playerId == playerId) {
            entry.threatValue += threatValue;
            entry.lastUpdate = std::chrono::steady_clock::now();
            return;
        }
    }
    
    // Add new entry
    ThreatEntry entry;
    entry.playerId = playerId;
    entry.mobInstanceId = mobInstanceId;
    entry.threatValue = threatValue;
    entry.lastUpdate = std::chrono::steady_clock::now();
    threatList.push_back(entry);
}

inline uint64_t SkillService::getHighestThreatPlayer(const std::string& mobInstanceId) const {
    std::shared_lock<std::shared_mutex> lock(threatMutex_);
    
    auto it = mobThreat_.find(mobInstanceId);
    if (it == mobThreat_.end() || it->second.empty()) return 0;
    
    uint64_t highestPlayer = 0;
    uint32_t highestThreat = 0;
    
    for (const auto& entry : it->second) {
        if (entry.threatValue > highestThreat) {
            highestThreat = entry.threatValue;
            highestPlayer = entry.playerId;
        }
    }
    
    return highestPlayer;
}

inline std::vector<ThreatEntry> SkillService::getThreatList(const std::string& mobInstanceId) const {
    std::shared_lock<std::shared_mutex> lock(threatMutex_);
    
    auto it = mobThreat_.find(mobInstanceId);
    if (it == mobThreat_.end()) return {};
    
    auto result = it->second;
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.threatValue > b.threatValue;
    });
    
    return result;
}

inline void SkillService::clearThreat(const std::string& mobInstanceId) {
    std::unique_lock<std::shared_mutex> lock(threatMutex_);
    mobThreat_.erase(mobInstanceId);
}

inline void SkillService::clearPlayerThreat(uint64_t playerId) {
    std::unique_lock<std::shared_mutex> lock(threatMutex_);
    
    for (auto& [mobId, threatList] : mobThreat_) {
        threatList.erase(
            std::remove_if(threatList.begin(), threatList.end(),
                [playerId](const ThreatEntry& e) { return e.playerId == playerId; }),
            threatList.end()
        );
    }
}

} // namespace Combat
} // namespace Umbra
