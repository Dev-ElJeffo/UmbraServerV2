#include <gtest/gtest.h>
#include "zone/ZoneServer.hpp"
#include "zone/PlayerManager.hpp"
#include "zone/EntitySystem.hpp"
#include "database/Models.hpp"

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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

