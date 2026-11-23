// test_companion_loot.cpp — regras do pet de coleta (filtro, raio) e frames 124-126.
#include "zone/CompanionService.hpp"
#include "zone/MovementProtocol.hpp"

#include <gtest/gtest.h>

using namespace Umbra::Zone;

TEST(CompanionLootTests, RarityRank) {
  EXPECT_EQ(companionRarityRank("common"), 0);
  EXPECT_EQ(companionRarityRank("uncommon"), 1);
  EXPECT_EQ(companionRarityRank("rare"), 2);
  EXPECT_EQ(companionRarityRank("epic"), 3);
  EXPECT_EQ(companionRarityRank("legendary"), 4);
  EXPECT_EQ(companionRarityRank(""), 0);
}

TEST(CompanionLootTests, FilterByRarityAndGold) {
  PetLootRule rule;
  rule.minRarity = 2;
  rule.lootGold = false;
  EXPECT_FALSE(companionShouldTakeSlot(rule, 0, 1));
  EXPECT_TRUE(companionShouldTakeSlot(rule, 0, 2));
  EXPECT_TRUE(companionShouldTakeSlot(rule, 0, 4));
  EXPECT_FALSE(companionShouldTakeSlot(rule, 1, 0));
  rule.lootGold = true;
  EXPECT_TRUE(companionShouldTakeSlot(rule, 1, 0));
}

TEST(CompanionLootTests, RadiusIsXY) {
  EXPECT_TRUE(companionWithinRadius(0.f, 0.f, 1000.f, 0.f, 1500.f));
  EXPECT_TRUE(companionWithinRadius(0.f, 0.f, 1500.f, 0.f, 1500.f));
  EXPECT_FALSE(companionWithinRadius(0.f, 0.f, 1200.f, 1200.f, 1500.f));
  EXPECT_FALSE(companionWithinRadius(0.f, 0.f, 0.f, 0.f, 0.f));
}

TEST(CompanionLootTests, RequestRoundTrip) {
  CompanionRequestPayload in;
  in.action = 1;
  in.companionId = 42;
  in.minRarity = 3;
  in.flags = 0;
  const auto bytes = encodeCompanionRequest(in);
  ASSERT_EQ(bytes.size(), 8u);
  CompanionRequestPayload out;
  ASSERT_TRUE(decodeCompanionRequest(bytes, out));
  EXPECT_EQ(out.action, 1);
  EXPECT_EQ(out.companionId, 42u);
  EXPECT_EQ(out.minRarity, 3);
  EXPECT_EQ(out.flags, 0);
}

TEST(CompanionLootTests, StateRoundTrip) {
  CompanionStatePayload in;
  in.ownerId = 7;
  in.slot = 0;
  in.companionId = 1;
  in.active = 1;
  in.scale = 0.5f;
  in.followDistance = 180.f;
  in.hoverHeight = 80.f;
  in.name = "Fada Coletora";
  in.meshPath = "/Game/Pets/SM_Fairy.SM_Fairy";
  const auto bytes = encodeCompanionState(in);
  CompanionStatePayload out;
  ASSERT_TRUE(decodeCompanionState(bytes, out));
  EXPECT_EQ(out.ownerId, 7u);
  EXPECT_EQ(out.active, 1);
  EXPECT_FLOAT_EQ(out.scale, 0.5f);
  EXPECT_FLOAT_EQ(out.hoverHeight, 80.f);
  EXPECT_EQ(out.name, "Fada Coletora");
  EXPECT_EQ(out.meshPath, "/Game/Pets/SM_Fairy.SM_Fairy");
  EXPECT_TRUE(out.idleAnimPath.empty());
}

TEST(CompanionLootTests, LootCollectedRoundTrip) {
  CompanionLootCollectedPayload in;
  in.ownerId = 9;
  in.corpseId = 123456789012ULL;
  in.x = 10.f;
  in.y = -20.f;
  in.z = 30.f;
  in.playerGold = 5000;
  in.goldCollected = 25;
  in.inventoryChanged = 1;
  in.items.push_back({136, 2});
  const auto bytes = encodeCompanionLootCollected(in);
  ASSERT_EQ(bytes.size(), 39u + 8u);
  CompanionLootCollectedPayload out;
  ASSERT_TRUE(decodeCompanionLootCollected(bytes, out));
  EXPECT_EQ(out.corpseId, 123456789012ULL);
  EXPECT_EQ(out.playerGold, 5000);
  EXPECT_EQ(out.goldCollected, 25u);
  ASSERT_EQ(out.items.size(), 1u);
  EXPECT_EQ(out.items[0].itemTemplateId, 136u);
  EXPECT_EQ(out.items[0].quantity, 2u);
}
