// test_dungeons.cpp — etapa 5: regras de conclusão, sorteio de recompensas, período do lockout,
// opcodes 138/139, trailing do 137 e objetivo complete_dungeon.
#include <gtest/gtest.h>

#include "zone/DungeonService.hpp"
#include "zone/InstanceManager.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/QuestProgressService.hpp"

using namespace Umbra::Zone;

TEST(DungeonCompletion, FinalBossMode) {
  // Com chefe final: só ele conclui.
  EXPECT_FALSE(dungeonCompletionReached("final_boss", 1, 2, 0, 1, 10, 20));
  EXPECT_TRUE(dungeonCompletionReached("final_boss", 1, 2, 1, 1, 10, 20));
  // Sem chefe final cadastrado: todos os chefes.
  EXPECT_FALSE(dungeonCompletionReached("final_boss", 1, 2, 0, 0, 10, 20));
  EXPECT_TRUE(dungeonCompletionReached("final_boss", 2, 2, 0, 0, 10, 20));
  // Sem chefes: todos os inimigos.
  EXPECT_FALSE(dungeonCompletionReached("final_boss", 0, 0, 0, 0, 19, 20));
  EXPECT_TRUE(dungeonCompletionReached("final_boss", 0, 0, 0, 0, 20, 20));
  // Molde sem NPCs nunca conclui sozinho.
  EXPECT_FALSE(dungeonCompletionReached("final_boss", 0, 0, 0, 0, 0, 0));
}

TEST(DungeonCompletion, AllBossesMode) {
  EXPECT_FALSE(dungeonCompletionReached("all_bosses", 2, 3, 1, 1, 10, 20));
  EXPECT_TRUE(dungeonCompletionReached("all_bosses", 3, 3, 1, 1, 10, 20));
}

TEST(DungeonRewards, FiltersByDifficultyAndChance) {
  std::mt19937 rng(42);
  std::vector<DungeonRewardEntry> entries;
  entries.push_back({10, "", 1.f, 1, 1});        // todas, sempre
  entries.push_back({11, "heroic", 1.f, 2, 2});  // só heroica
  entries.push_back({12, "", 0.f, 1, 1});        // chance 0
  auto normal = rollDungeonRewards(entries, "normal", rng);
  ASSERT_EQ(normal.size(), 1u);
  EXPECT_EQ(normal[0].itemTemplateId, 10u);
  auto heroic = rollDungeonRewards(entries, "heroic", rng);
  ASSERT_EQ(heroic.size(), 2u);
  EXPECT_EQ(heroic[1].itemTemplateId, 11u);
  EXPECT_EQ(heroic[1].quantity, 2u);
}

TEST(DungeonRewards, QuantityWithinRange) {
  std::mt19937 rng(7);
  std::vector<DungeonRewardEntry> entries{{20, "", 1.f, 3, 5}};
  for (int i = 0; i < 50; ++i) {
    auto r = rollDungeonRewards(entries, "normal", rng);
    ASSERT_EQ(r.size(), 1u);
    EXPECT_GE(r[0].quantity, 3u);
    EXPECT_LE(r[0].quantity, 5u);
  }
}

TEST(DungeonLockout, PeriodSql) {
  EXPECT_EQ(dungeonPeriodKeySql("daily"), "CURDATE()");
  EXPECT_NE(dungeonPeriodKeySql("weekly").find("WEEKDAY"), std::string::npos);
  EXPECT_EQ(dungeonPeriodKeySql("none"), "'2000-01-01'");
}

TEST(DungeonDifficulty, FallbackToNormal) {
  InstanceTemplateDef tpl;
  auto d = instanceDifficultyAt(tpl, 3);
  EXPECT_EQ(d.key, "normal");
  EXPECT_FLOAT_EQ(d.hpMult, 1.f);
  tpl.difficulties.push_back({"heroic", "Heroica", 10, 2.5f, 1.8f, 2.f});
  EXPECT_EQ(instanceDifficultyAt(tpl, 0).key, "heroic");
  EXPECT_EQ(instanceDifficultyAt(tpl, 5).key, "heroic");
}

TEST(DungeonProtocol, ProgressRoundTrip) {
  DungeonProgressPayload p;
  p.instanceId = 9;
  p.bossesKilled = 1;
  p.bossesTotal = 3;
  p.enemiesKilled = 12;
  p.enemiesTotal = 40;
  p.state = static_cast<uint8_t>(DungeonRunState::Completed);
  p.closesInSeconds = 299;
  p.difficultyName = "Heroica";
  const auto data = encodeDungeonProgress(p);
  EXPECT_EQ(data[0], 138);
  DungeonProgressPayload out;
  ASSERT_TRUE(decodeDungeonProgress(data, out));
  EXPECT_EQ(out.bossesTotal, 3);
  EXPECT_EQ(out.enemiesKilled, 12);
  EXPECT_EQ(out.state, 1);
  EXPECT_EQ(out.closesInSeconds, 299);
  EXPECT_EQ(out.difficultyName, "Heroica");
}

TEST(DungeonProtocol, CompletedRoundTrip) {
  DungeonCompletedPayload p;
  p.templateId = 1;
  p.durationSeconds = 754;
  p.rewarded = 1;
  p.exp = 1000;
  p.gold = 400;
  p.playerGold = 5000000000LL;
  p.inventoryChanged = 1;
  p.dungeonName = "Cripta de teste";
  p.difficultyName = "Heroica";
  p.items.push_back({33, 2, 0, "Poção"});
  p.items.push_back({34, 1, 1, "Espada"});
  const auto data = encodeDungeonCompleted(p);
  EXPECT_EQ(data[0], 139);
  DungeonCompletedPayload out;
  ASSERT_TRUE(decodeDungeonCompleted(data, out));
  EXPECT_EQ(out.durationSeconds, 754u);
  EXPECT_EQ(out.playerGold, 5000000000LL);
  ASSERT_EQ(out.items.size(), 2u);
  EXPECT_EQ(out.items[1].mailed, 1);
  EXPECT_EQ(out.items[1].name, "Espada");
}

TEST(DungeonProtocol, LobbyTrailingAndDifficultyAction) {
  InstanceLobbyStatePayload p;
  p.state = 1;
  p.lobbyId = 2;
  p.name = "Cripta";
  p.members.push_back({11, 20, 0, 0x02, "Ana"});
  p.isDungeon = 1;
  p.selectedDifficulty = 1;
  p.leaderPlayerId = 11;
  p.difficulties.push_back({"normal", "Normal", 1});
  p.difficulties.push_back({"heroic", "Heroica", 10});
  auto data = encodeInstanceLobbyState(p);
  InstanceLobbyStatePayload out;
  ASSERT_TRUE(decodeInstanceLobbyState(data, out));
  EXPECT_EQ(out.isDungeon, 1);
  EXPECT_EQ(out.selectedDifficulty, 1);
  EXPECT_EQ(out.leaderPlayerId, 11u);
  ASSERT_EQ(out.difficulties.size(), 2u);
  EXPECT_EQ(out.difficulties[1].minLevel, 10);
  EXPECT_EQ(out.members[0].flags & 0x02, 0x02);

  uint32_t npc = 0;
  uint8_t action = 0, value = 0;
  ASSERT_TRUE(decodeInstanceEnterRequest(encodeInstanceEnterRequest(0, InstanceLobbyAction::SetDifficulty, 1), npc,
                                         action, value));
  EXPECT_EQ(action, static_cast<uint8_t>(InstanceLobbyAction::SetDifficulty));
  EXPECT_EQ(value, 1);
}

TEST(DungeonQuest, CompleteDungeonMatches) {
  const auto kind = questKindFromString("complete_dungeon");
  EXPECT_EQ(kind, QuestObjectiveKind::CompleteDungeon);
  QuestEvent ev;
  ev.kind = QuestObjectiveKind::CompleteDungeon;
  ev.key = 1;
  ev.subtype = "heroic";
  EXPECT_TRUE(questEventMatches(kind, parseQuestObjectiveParams(kind, "{}"), ev, 0));
  EXPECT_TRUE(questEventMatches(kind, parseQuestObjectiveParams(kind, "{\"instance_template_id\":1}"), ev, 0));
  EXPECT_FALSE(questEventMatches(kind, parseQuestObjectiveParams(kind, "{\"instance_template_id\":2}"), ev, 0));
  EXPECT_FALSE(questEventMatches(kind, parseQuestObjectiveParams(kind, "{\"difficulty_key\":\"normal\"}"), ev, 0));
}
