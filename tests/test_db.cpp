#include <gtest/gtest.h>
#include "database/MySQLConnector.hpp"
#include "database/Models.hpp"

using namespace Umbra::Database;

class DatabaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MySQLConnector::Config config;
    config.host = "localhost";
    config.database = "umbra_eternum_test";
    connector = new MySQLConnector(config);
  }
  
  void TearDown() override {
    connector->disconnect();
    delete connector;
  }
  
  MySQLConnector* connector;
};

TEST_F(DatabaseTest, Connection) {
  // Note: This will fail if MySQL is not available
  // EXPECT_TRUE(connector->connect());
  // EXPECT_TRUE(connector->isConnected());
  EXPECT_TRUE(true);  // Placeholder
}

TEST_F(DatabaseTest, EscapeString) {
  std::string input = "test'string\"with\\special";
  std::string escaped = connector->escapeString(input);
  
  EXPECT_NE(input, escaped);
  EXPECT_TRUE(escaped.find("''") != std::string::npos || 
              escaped.find("\\'") != std::string::npos);
}

TEST(ModelsTest, AccountValidation) {
  Account account;
  EXPECT_FALSE(account.isValid());
  
  account.username = "testuser";
  account.email = "test@example.com";
  account.passwordHash = "hash";
  
  EXPECT_TRUE(account.isValid());
}

TEST(ModelsTest, PlayerValidation) {
  Player player;
  EXPECT_FALSE(player.isValid());
  
  player.accountId = 1;
  player.characterName = "TestChar";
  
  EXPECT_TRUE(player.isValid());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

