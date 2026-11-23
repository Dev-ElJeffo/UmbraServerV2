// test_quest_events.cpp — Quests v2: evento × objetivo, contagem e frames 131/132.
#include "zone/QuestProgressService.hpp"
#include "zone/MovementProtocol.hpp"

#include <gtest/gtest.h>

using namespace Umbra::Zone;

namespace {
QuestEvent ev(QuestObjectiveKind kind, uint32_t key = 0, uint32_t level = 0, int32_t amount = 1) {
  QuestEvent e;
  e.kind = kind;
  e.key = key;
  e.level = level;
  e.amount = amount;
  return e;
}
}  // namespace

TEST(QuestEventTests, KindFromString) {
  EXPECT_EQ(questKindFromString("kill"), QuestObjectiveKind::Kill);
  EXPECT_EQ(questKindFromString("kill_any"), QuestObjectiveKind::KillAny);
  EXPECT_EQ(questKindFromString("earn_gold"), QuestObjectiveKind::EarnGold);
  EXPECT_EQ(questKindFromString("collect"), QuestObjectiveKind::Other);  // PHP
}

TEST(QuestEventTests, KillSingleAndMultiTarget) {
  auto one = parseQuestObjectiveParams(QuestObjectiveKind::Kill, R"({"npc_template_id": 6, "required_count": 3})");
  EXPECT_EQ(one.required, 3);
  EXPECT_TRUE(questEventMatches(QuestObjectiveKind::Kill, one, ev(QuestObjectiveKind::Kill, 6), 1));
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::Kill, one, ev(QuestObjectiveKind::Kill, 7), 1));

  auto multi = parseQuestObjectiveParams(QuestObjectiveKind::Kill, R"({"npc_template_ids": [6, 9], "required_count": 5})");
  EXPECT_TRUE(questEventMatches(QuestObjectiveKind::Kill, multi, ev(QuestObjectiveKind::Kill, 9), 1));
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::Kill, multi, ev(QuestObjectiveKind::Kill, 8), 1));
  // Tipo de evento diferente nunca casa.
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::Kill, one, ev(QuestObjectiveKind::KillAny, 6), 1));
}

TEST(QuestEventTests, KillAnyLevelRangeAndZone) {
  auto p = parseQuestObjectiveParams(QuestObjectiveKind::KillAny,
                                     R"({"min_level": 5, "max_level": 10, "zone_id": 2, "required_count": 20})");
  QuestEvent e = ev(QuestObjectiveKind::KillAny, 99, 7);
  e.zoneId = 2;
  EXPECT_TRUE(questEventMatches(QuestObjectiveKind::KillAny, p, e, 2));
  e.level = 4;
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::KillAny, p, e, 2));
  e.level = 11;
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::KillAny, p, e, 2));
  e.level = 7;
  e.zoneId = 3;
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::KillAny, p, e, 3));
}

TEST(QuestEventTests, SkillAndConsumable) {
  auto anySkill = parseQuestObjectiveParams(QuestObjectiveKind::UseSkill, R"({"required_count": 10})");
  EXPECT_TRUE(questEventMatches(QuestObjectiveKind::UseSkill, anySkill, ev(QuestObjectiveKind::UseSkill, 42), 1));
  auto oneSkill = parseQuestObjectiveParams(QuestObjectiveKind::UseSkill, R"({"skill_id": 42})");
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::UseSkill, oneSkill, ev(QuestObjectiveKind::UseSkill, 41), 1));

  auto bySubtype = parseQuestObjectiveParams(QuestObjectiveKind::UseConsumable, R"({"item_subtype": "health_potion"})");
  QuestEvent c = ev(QuestObjectiveKind::UseConsumable, 7);
  c.subtype = "health_potion";
  EXPECT_TRUE(questEventMatches(QuestObjectiveKind::UseConsumable, bySubtype, c, 1));
  c.subtype = "mana_potion";
  EXPECT_FALSE(questEventMatches(QuestObjectiveKind::UseConsumable, bySubtype, c, 1));
}

TEST(QuestEventTests, CountsGoldAndLevel) {
  auto gold = parseQuestObjectiveParams(QuestObjectiveKind::EarnGold, R"({"amount": 500})");
  EXPECT_EQ(gold.required, 500);
  EXPECT_EQ(questApplyEventCount(QuestObjectiveKind::EarnGold, 450, 500, ev(QuestObjectiveKind::EarnGold, 0, 0, 120)), 500);
  EXPECT_EQ(questApplyEventCount(QuestObjectiveKind::EarnGold, 0, 500, ev(QuestObjectiveKind::EarnGold, 0, 0, 120)), 120);

  auto lvl = parseQuestObjectiveParams(QuestObjectiveKind::ReachLevel, R"({"level": 10})");
  EXPECT_EQ(lvl.required, 10);
  EXPECT_EQ(questApplyEventCount(QuestObjectiveKind::ReachLevel, 6, 10, ev(QuestObjectiveKind::ReachLevel, 0, 8)), 8);
  EXPECT_EQ(questApplyEventCount(QuestObjectiveKind::ReachLevel, 8, 10, ev(QuestObjectiveKind::ReachLevel, 0, 12)), 10);
  EXPECT_EQ(questApplyEventCount(QuestObjectiveKind::Kill, 2, 3, ev(QuestObjectiveKind::Kill, 6)), 3);
}

TEST(QuestEventTests, Frames131And132) {
  uint32_t questId = 0;
  ASSERT_TRUE(decodeQuestStateChanged(encodeQuestStateChanged(17), questId));
  EXPECT_EQ(questId, 17u);
  uint32_t itemId = 0;
  ASSERT_TRUE(decodeQuestInteract(encodeQuestInteract(17, 55), questId, itemId));
  EXPECT_EQ(itemId, 55u);
  EXPECT_FALSE(decodeQuestInteract(encodeQuestInteract(0, 55), questId, itemId));
}

TEST(QuestEventTests, Frame133GuildMission) {
  GuildMissionProgressPayload in;
  in.guildMissionId = 12;
  in.progress = 40;
  in.required = 60;
  in.status = 0;
  const auto bytes = encodeGuildMissionProgress(in);
  ASSERT_EQ(bytes.size(), 14u);
  GuildMissionProgressPayload out;
  ASSERT_TRUE(decodeGuildMissionProgress(bytes, out));
  EXPECT_EQ(out.guildMissionId, 12u);
  EXPECT_EQ(out.progress, 40u);
  EXPECT_EQ(out.required, 60u);
}
