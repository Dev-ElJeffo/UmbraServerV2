#include <gtest/gtest.h>
#include "zone/ZoneServer.hpp"
#include "zone/PlayerManager.hpp"
#include "zone/EntitySystem.hpp"
#include "database/Models.hpp"
#include "zone/MovementProtocol.hpp"

using namespace Umbra::Zone;

class ZoneServerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ZoneServer::Config config;
    config.zoneId = 1;
    config.zoneName = "TestZone";
    config.maxPlayers = 100;
    
    zoneServer = new ZoneServer(config);
  }
  
  void TearDown() override {
    zoneServer->stop();
    delete zoneServer;
  }
  
  ZoneServer* zoneServer;
};

TEST_F(ZoneServerTest, StartStop) {
  EXPECT_TRUE(zoneServer->start());
  EXPECT_TRUE(zoneServer->isRunning());
  
  zoneServer->stop();
  EXPECT_FALSE(zoneServer->isRunning());
}

TEST_F(ZoneServerTest, Update) {
  zoneServer->start();
  
  EXPECT_NO_THROW(zoneServer->update(0.016f));  // ~60 FPS
}

class PlayerManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    playerManager = new PlayerManager(10);
  }
  
  void TearDown() override {
    delete playerManager;
  }
  
  PlayerManager* playerManager;
};

TEST_F(PlayerManagerTest, AddPlayer) {
  Umbra::Database::Player player;
  player.id = 1;
  player.characterName = "TestChar";
  player.posX = 0;
  player.posY = 0;
  player.posZ = 100;
  
  EXPECT_TRUE(playerManager->addPlayer(player));
  EXPECT_EQ(playerManager->getPlayerCount(), 1);
}

TEST_F(PlayerManagerTest, RemovePlayer) {
  Umbra::Database::Player player;
  player.id = 1;
  player.characterName = "TestChar";
  
  playerManager->addPlayer(player);
  EXPECT_EQ(playerManager->getPlayerCount(), 1);
  
  EXPECT_TRUE(playerManager->removePlayer(1));
  EXPECT_EQ(playerManager->getPlayerCount(), 0);
}

TEST_F(PlayerManagerTest, UpdatePosition) {
  Umbra::Database::Player player;
  player.id = 1;
  player.characterName = "TestChar";
  
  playerManager->addPlayer(player);
  
  EXPECT_TRUE(playerManager->updatePosition(1, 10.0f, 20.0f, 30.0f));
}

class EntitySystemTest : public ::testing::Test {
 protected:
  void SetUp() override {
    entitySystem = new EntitySystem();
  }
  
  void TearDown() override {
    delete entitySystem;
  }
  
  EntitySystem* entitySystem;
};

TEST_F(EntitySystemTest, SpawnEntity) {
  Entity entity;
  entity.type = "mob";
  entity.posX = 0;
  entity.posY = 0;
  entity.posZ = 0;
  entity.health = 100;
  
  uint64_t entityId = entitySystem->spawnEntity(entity);
  EXPECT_GT(entityId, 0);
  EXPECT_EQ(entitySystem->getEntityCount(), 1);
}

TEST_F(EntitySystemTest, DespawnEntity) {
  Entity entity;
  entity.type = "mob";
  
  uint64_t entityId = entitySystem->spawnEntity(entity);
  EXPECT_TRUE(entitySystem->despawnEntity(entityId));
  EXPECT_EQ(entitySystem->getEntityCount(), 0);
}

// Opcode 119 (CombatStanceState): ida/volta e frame curto sem flags.
TEST(CombatStanceProtocolTest, RoundTrip) {
  Umbra::Zone::CombatStancePayload in;
  in.playerId = 0x01020304;
  in.stance = 1;
  in.flags = Umbra::Zone::kCombatStanceFlagAuto;
  const auto bytes = Umbra::Zone::encodeCombatStanceState(in);
  ASSERT_EQ(bytes.size(), 7u);
  EXPECT_EQ(bytes[0], 119);
  Umbra::Zone::CombatStancePayload out;
  ASSERT_TRUE(Umbra::Zone::decodeCombatStanceState(bytes, out));
  EXPECT_EQ(out.playerId, in.playerId);
  EXPECT_EQ(out.stance, 1);
  EXPECT_EQ(out.flags, Umbra::Zone::kCombatStanceFlagAuto);
}

TEST(CombatStanceProtocolTest, ShortFrameDefaultsFlags) {
  std::vector<uint8_t> bytes = {119, 7, 0, 0, 0, 0};
  Umbra::Zone::CombatStancePayload out;
  ASSERT_TRUE(Umbra::Zone::decodeCombatStanceState(bytes, out));
  EXPECT_EQ(out.playerId, 7u);
  EXPECT_EQ(out.stance, 0);
  EXPECT_EQ(out.flags, 0);
  std::vector<uint8_t> tooShort = {119, 7, 0, 0};
  EXPECT_FALSE(Umbra::Zone::decodeCombatStanceState(tooShort, out));
}

// Opcode 117: trailing3 hide_body_slots (ida/volta e frame antigo sem o campo).
TEST(EquipmentVisualProtocolTest, HideBodySlotsRoundTrip) {
  std::vector<Umbra::Zone::EquippedVisualWireEntry> in(2);
  in[0].equipSlot = 2;
  in[0].meshPath = "/Game/A.A";
  in[0].flags = 1;
  in[0].refinementLevel = 7;
  in[0].hideBodySlots = "Hands,Feet";
  in[1].equipSlot = 4;
  in[1].meshPath = "/Game/B.B";
  const auto bytes = Umbra::Zone::encodePlayerEquipmentVisualUpdate(42, in);
  uint32_t playerId = 0;
  std::vector<Umbra::Zone::EquippedVisualWireEntry> out;
  ASSERT_TRUE(Umbra::Zone::decodePlayerEquipmentVisualUpdate(bytes, playerId, out));
  EXPECT_EQ(playerId, 42u);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].meshPath, "/Game/A.A");
  EXPECT_EQ(out[0].flags, 1);
  EXPECT_EQ(out[0].refinementLevel, 7);
  EXPECT_EQ(out[0].hideBodySlots, "Hands,Feet");
  EXPECT_TRUE(out[1].hideBodySlots.empty());
}

TEST(EquipmentVisualProtocolTest, LegacyFrameWithoutHideBodySlots) {
  std::vector<Umbra::Zone::EquippedVisualWireEntry> in(1);
  in[0].equipSlot = 1;
  in[0].meshPath = "/Game/C.C";
  in[0].refinementLevel = 3;
  auto bytes = Umbra::Zone::encodePlayerEquipmentVisualUpdate(5, in);
  bytes.resize(bytes.size() - 2);  // remove o trailing3 (len=0) -> frame do cliente antigo
  uint32_t playerId = 0;
  std::vector<Umbra::Zone::EquippedVisualWireEntry> out;
  ASSERT_TRUE(Umbra::Zone::decodePlayerEquipmentVisualUpdate(bytes, playerId, out));
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].refinementLevel, 3);
  EXPECT_TRUE(out[0].hideBodySlots.empty());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

