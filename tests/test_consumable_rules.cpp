// test_consumable_rules.cpp — consumíveis pela zone e regras do elemental (opcodes 127-130).
#include "zone/ConsumableService.hpp"
#include "zone/MovementProtocol.hpp"

#include <gtest/gtest.h>

using namespace Umbra::Zone;

namespace {
ElementalRuleWire rule(ElementalTrigger t, uint8_t threshold, uint32_t item) {
  ElementalRuleWire r;
  r.trigger = static_cast<uint8_t>(t);
  r.threshold = threshold;
  r.itemTemplateId = item;
  return r;
}
}  // namespace

TEST(ConsumableRulesTests, ParseStatsLikePhp) {
  ConsumableDef hp;
  ASSERT_TRUE(parseConsumableStats(R"({"health_restore": 150})", hp));
  EXPECT_EQ(hp.healthRestore, 150);
  EXPECT_TRUE(hp.buffKey.empty());

  ConsumableDef buff;
  ASSERT_TRUE(parseConsumableStats(R"({"duration": 300, "strength_buff": 20})", buff));
  EXPECT_EQ(buff.buffKey, "strength_buff");
  EXPECT_EQ(buff.buffValue, 20);
  EXPECT_EQ(buff.buffDurationMs, 300000u);

  ConsumableDef none;
  EXPECT_FALSE(parseConsumableStats("", none));
  EXPECT_FALSE(parseConsumableStats(R"({"strength_buff": 20})", none));  // buff sem duração
  EXPECT_FALSE(parseConsumableStats("not json", none));
}

TEST(ConsumableRulesTests, RuleFires) {
  const auto hpRule = rule(ElementalTrigger::HpBelow, 40, 7);
  EXPECT_TRUE(elementalRuleFires(hpRule, 39, 100, 50, 50, false));
  EXPECT_FALSE(elementalRuleFires(hpRule, 40, 100, 50, 50, false));
  EXPECT_FALSE(elementalRuleFires(hpRule, 0, 100, 50, 50, false));  // morto

  const auto mpRule = rule(ElementalTrigger::MpBelow, 30, 9);
  EXPECT_TRUE(elementalRuleFires(mpRule, 100, 100, 29, 100, false));
  EXPECT_FALSE(elementalRuleFires(mpRule, 100, 100, 30, 100, false));

  const auto buffRule = rule(ElementalTrigger::BuffMissing, 0, 10);
  EXPECT_TRUE(elementalRuleFires(buffRule, 100, 100, 10, 100, false));
  EXPECT_FALSE(elementalRuleFires(buffRule, 100, 100, 10, 100, true));
}

TEST(ConsumableRulesTests, RuleMatchesItem) {
  ConsumableDef hp;
  hp.healthRestore = 150;
  ConsumableDef mp;
  mp.manaRestore = 100;
  ConsumableDef buff;
  buff.buffKey = "strength_buff";
  EXPECT_TRUE(elementalRuleMatchesItem(rule(ElementalTrigger::HpBelow, 40, 7), hp));
  EXPECT_FALSE(elementalRuleMatchesItem(rule(ElementalTrigger::HpBelow, 40, 9), mp));
  EXPECT_FALSE(elementalRuleMatchesItem(rule(ElementalTrigger::HpBelow, 0, 7), hp));  // % inválido
  EXPECT_TRUE(elementalRuleMatchesItem(rule(ElementalTrigger::MpBelow, 30, 9), mp));
  EXPECT_TRUE(elementalRuleMatchesItem(rule(ElementalTrigger::BuffMissing, 0, 10), buff));
  EXPECT_FALSE(elementalRuleMatchesItem(rule(ElementalTrigger::BuffMissing, 0, 7), hp));
}

TEST(ConsumableRulesTests, RulesJsonRoundTrip) {
  std::vector<ElementalRuleWire> in{rule(ElementalTrigger::HpBelow, 40, 7), rule(ElementalTrigger::BuffMissing, 0, 10)};
  std::vector<ElementalRuleWire> out;
  ASSERT_TRUE(parseElementalRulesJson(elementalRulesToJson(in), out));
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].trigger, static_cast<uint8_t>(ElementalTrigger::HpBelow));
  EXPECT_EQ(out[0].threshold, 40);
  EXPECT_EQ(out[1].itemTemplateId, 10u);
  ASSERT_TRUE(parseElementalRulesJson("", out));
  EXPECT_TRUE(out.empty());
  ASSERT_TRUE(parseElementalRulesJson(R"([{"trigger":"xyz","threshold":10,"item_template_id":7}])", out));
  EXPECT_TRUE(out.empty());  // gatilho desconhecido descartado
}

TEST(ConsumableRulesTests, UseFramesRoundTrip) {
  ConsumableUseRequestPayload req;
  req.inventoryId = 55;
  const auto reqBytes = encodeConsumableUseRequest(req);
  ASSERT_EQ(reqBytes.size(), 9u);
  ConsumableUseRequestPayload reqOut;
  ASSERT_TRUE(decodeConsumableUseRequest(reqBytes, reqOut));
  EXPECT_EQ(reqOut.inventoryId, 55u);

  ConsumableUseResultPayload res;
  res.status = static_cast<uint8_t>(ConsumableUseStatus::Ok);
  res.source = static_cast<uint8_t>(ConsumableSource::Elemental);
  res.itemTemplateId = 7;
  res.healthRestored = 150;
  res.cooldownExpiresAtMs = 1735000000000LL;
  res.subtype = "health_potion";
  res.message = "Poção de Vida Menor";
  ConsumableUseResultPayload resOut;
  ASSERT_TRUE(decodeConsumableUseResult(encodeConsumableUseResult(res), resOut));
  EXPECT_EQ(resOut.source, 1);
  EXPECT_EQ(resOut.healthRestored, 150);
  EXPECT_EQ(resOut.cooldownExpiresAtMs, 1735000000000LL);
  EXPECT_EQ(resOut.subtype, "health_potion");
  EXPECT_EQ(resOut.message, "Poção de Vida Menor");
}

TEST(ConsumableRulesTests, RulesFramesRoundTrip) {
  ElementalRulesSetPayload set;
  set.companionId = 2;
  set.rules = {rule(ElementalTrigger::HpBelow, 40, 7), rule(ElementalTrigger::MpBelow, 30, 9)};
  ElementalRulesSetPayload setOut;
  ASSERT_TRUE(decodeElementalRulesSet(encodeElementalRulesSet(set), setOut));
  EXPECT_EQ(setOut.companionId, 2u);
  ASSERT_EQ(setOut.rules.size(), 2u);
  EXPECT_EQ(setOut.rules[1].threshold, 30);

  ElementalRulesStatePayload state;
  state.status = 0;
  state.companionId = 2;
  state.maxRules = 2;
  state.rules = set.rules;
  ElementalRulesStatePayload stateOut;
  ASSERT_TRUE(decodeElementalRulesState(encodeElementalRulesState(state), stateOut));
  EXPECT_EQ(stateOut.maxRules, 2);
  EXPECT_EQ(stateOut.rules.size(), 2u);

  // count acima do limite é recusado
  std::vector<uint8_t> bad = {static_cast<uint8_t>(MovementMsgType::ElementalRulesSet), 2, 0, 0, 0, 9};
  ElementalRulesSetPayload badOut;
  EXPECT_FALSE(decodeElementalRulesSet(bad, badOut));
}
