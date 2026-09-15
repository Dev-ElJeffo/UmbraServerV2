#include <gtest/gtest.h>
#include "database/MySQLConnector.hpp"
#include "database/Models.hpp"
#include "core/Logger.hpp"
#include <cstdlib>
#include <string>

using namespace Umbra::Database;

namespace {

void ensureTestLogger() {
  static bool once = false;
  if (!once) {
    Umbra::Core::Logger::getInstance().initialize("logs/test_db.log", 1024 * 1024, 2);
    once = true;
  }
}

}  // namespace

class DatabaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ensureTestLogger();
    MySQLConnector::Config config;
    config.host = "localhost";
    config.database = "umbra_eternum_test";
    config.autoReconnect = false;
    connector = new MySQLConnector(config);
  }

  void TearDown() override {
    connector->disconnect();
    delete connector;
  }

  MySQLConnector* connector;
};

TEST_F(DatabaseTest, Connection) {
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

/**
 * Integração opcional (UMBRA_RUN_MYSQL_TESTS=1): grava DAMAGE/DOUBLE/REACTION.
 * Credenciais: UMBRA_DB_USER / UMBRA_DB_PASSWORD (default root / vazio).
 */
TEST(CombatLogEnumTest, InsertDamageDoubleReaction) {
  if (!std::getenv("UMBRA_RUN_MYSQL_TESTS")) {
    GTEST_SKIP() << "Defina UMBRA_RUN_MYSQL_TESTS=1 para rodar integração MySQL";
  }
  ensureTestLogger();

  MySQLConnector::Config config;
  config.host = "localhost";
  config.port = 3306;
  config.database = "umbra_eternum";
  config.username = "root";
  config.password = "";
  config.autoReconnect = false;
  config.poolSize = 2;
  config.connectionTimeout = 5;
  if (const char* u = std::getenv("UMBRA_DB_USER")) {
    config.username = u;
  }
  if (const char* p = std::getenv("UMBRA_DB_PASSWORD")) {
    config.password = p;
  }

  MySQLConnector connector(config);
  if (!connector.connect()) {
    GTEST_SKIP() << "MySQL umbra_eternum indisponível";
  }

  const char* types[] = {"DAMAGE", "DOUBLE", "REACTION"};
  for (const char* action : types) {
    const std::string sql =
        std::string("INSERT INTO combat_log (source_player_id, target_player_id, skill_id, "
                    "action_type, value, is_critical, overkill, zone_id) VALUES (0, 0, 0, '") +
        action + "', 1, 0, 0, 0)";
    const bool ok = connector.execute(sql);
    EXPECT_TRUE(ok) << "Falha ao inserir action_type=" << action
                    << " — rode www/umbra_api/scripts/alter_combat_log_action_type.sql";
  }

  connector.execute(
      "DELETE FROM combat_log WHERE source_player_id = 0 AND target_player_id = 0 AND skill_id = 0 "
      "AND value = 1 AND zone_id = '0'");
  connector.disconnect();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
