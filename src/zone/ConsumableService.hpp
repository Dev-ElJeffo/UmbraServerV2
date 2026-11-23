#pragma once
// ConsumableService.hpp — uso de consumível decidido pela zone (opcodes 127/128) e o elemental,
// que usa poções sozinho pelas regras do jogador (opcodes 129/130).
// Tudo roda numa thread própria: inventário/cooldown no DB sem travar o tick nem o recv.

#include "zone/MovementProtocol.hpp"
#include "zone/CompanionService.hpp"
#include "database/MySQLConnector.hpp"
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;
class CombatCoreEngine;

/** Consumível usável pela zone (item_templates com item_type = consumable, exceto selos). */
struct ConsumableDef {
  uint32_t itemTemplateId = 0;
  std::string name;
  std::string subtype;  // chave do cooldown compartilhado (ex.: health_potion)
  std::string iconPath;
  int32_t healthRestore = 0;
  int32_t manaRestore = 0;
  std::string buffKey;  // ex.: strength_buff (vazio = sem buff)
  int32_t buffValue = 0;
  uint32_t buffDurationMs = 0;
  uint32_t cooldownMs = 5000;
};

/** stats_json → efeitos (mesmas chaves do use_item.php). false = item sem efeito usável. */
bool parseConsumableStats(const std::string& statsJson, ConsumableDef& def);

/** auto_rules_json ↔ regras. Entradas inválidas são descartadas no parse. */
bool parseElementalRulesJson(const std::string& json, std::vector<ElementalRuleWire>& out);
std::string elementalRulesToJson(const std::vector<ElementalRuleWire>& rules);

/** A regra combina com o item? (vida→cura vida, mana→cura mana, buff ausente→item com buff). */
bool elementalRuleMatchesItem(const ElementalRuleWire& rule, const ConsumableDef& def);

/** A regra dispara com estes vitals? buffActive só vale para BuffMissing. */
bool elementalRuleFires(const ElementalRuleWire& rule, int32_t hp, int32_t maxHp, int32_t mp,
                        int32_t maxMp, bool buffActive);

class ConsumableService {
public:
  ConsumableService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movementServer,
                    CombatCoreEngine* combat, CompanionService* companions);
  ~ConsumableService();

  /** (Re)carrega os consumíveis de item_templates. Boot e admin reload_consumables. */
  void loadTemplates();
  size_t templateCount() const;

  void start();
  void stop();

  /** Opcode 127 (inbound worker): só enfileira. */
  void enqueueUse(uint32_t playerId, uint32_t inventoryId, uint32_t itemTemplateId);
  /** Opcode 129 (inbound worker): só enfileira. */
  void enqueueSetRules(uint32_t playerId, const ElementalRulesSetPayload& payload);

  /** CompanionService: elemental invocado (regras do DB) / recolhido. */
  void onElementalActivated(uint32_t playerId, const CompanionTemplateDef& tpl, const std::string& rulesJson);
  void onElementalDeactivated(uint32_t playerId, uint32_t companionId);
  /** Saiu da zone: limpa caches (cooldown/buffs são recarregados do DB no próximo uso). */
  void onPlayerGone(uint32_t playerId);

private:
  struct Job {
    enum class Kind : uint8_t { Use, SetRules } kind = Kind::Use;
    uint32_t playerId = 0;
    uint32_t inventoryId = 0;
    uint32_t itemTemplateId = 0;
    ElementalRulesSetPayload rules;
  };

  struct ActiveElemental {
    uint32_t companionId = 0;
    std::string name;
    uint8_t maxRules = 2;
    uint32_t checkMs = 500;
    int64_t nextCheckAtMs = 0;
    std::vector<ElementalRuleWire> rules;
    int64_t backoffUntilMs[kElementalMaxRulesCap] = {0, 0, 0, 0};
  };

  struct PlayerConsumableState {
    bool loaded = false;
    std::unordered_map<std::string, int64_t> cooldownBySubtype;
    std::unordered_map<std::string, int64_t> buffExpiresByKey;
  };

  void workerLoop();
  void processJob(const Job& job);
  void scanElementals(int64_t nowMs);
  ConsumableUseStatus useItem(uint32_t playerId, uint32_t inventoryId, uint32_t itemTemplateId,
                              ConsumableSource source, std::string* outItemName = nullptr);
  void handleSetRules(uint32_t playerId, const ElementalRulesSetPayload& payload);
  void ensurePlayerLoaded(uint32_t playerId);
  bool findDef(uint32_t itemTemplateId, ConsumableDef& out) const;
  void sendResult(uint32_t playerId, const ConsumableUseResultPayload& result);
  void sendRulesState(uint32_t playerId, const ElementalRulesStatePayload& state);
  static int64_t nowMs();

  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  CombatCoreEngine* combat_ = nullptr;
  CompanionService* companions_ = nullptr;

  mutable std::mutex defsMu_;
  std::unordered_map<uint32_t, ConsumableDef> defs_;

  // Só a thread do worker mexe nestes (sem lock).
  std::unordered_map<uint32_t, PlayerConsumableState> players_;

  std::mutex elemMu_;
  std::unordered_map<uint32_t, ActiveElemental> elementals_;
  std::unordered_set<uint32_t> goneQueue_;  // onPlayerGone vindo de outra thread

  std::mutex queueMu_;
  std::condition_variable queueCv_;
  std::deque<Job> queue_;
  bool stop_ = false;
  std::thread worker_;
};

}  // namespace Zone
}  // namespace Umbra
