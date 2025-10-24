#include <gtest/gtest.h>
#include "auth/JWTManager.hpp"
#include "auth/SessionManager.hpp"

using namespace Umbra::Auth;

class JWTManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    jwtManager = new JWTManager("test_secret_key");
  }
  
  void TearDown() override {
    delete jwtManager;
  }
  
  JWTManager* jwtManager;
};

TEST_F(JWTManagerTest, GenerateToken) {
  std::string token = jwtManager->generateToken(1, 1, "testuser", 60);
  
  EXPECT_FALSE(token.empty());
  EXPECT_TRUE(token.find(".") != std::string::npos);
}

TEST_F(JWTManagerTest, ValidateToken) {
  std::string token = jwtManager->generateToken(1, 1, "testuser", 60);
  auto payload = jwtManager->validateToken(token);
  
  ASSERT_TRUE(payload.has_value());
  EXPECT_EQ(payload->accountId, 1);
  EXPECT_EQ(payload->playerId, 1);
  EXPECT_EQ(payload->username, "testuser");
}

TEST_F(JWTManagerTest, RevokeToken) {
  std::string token = jwtManager->generateToken(1, 1, "testuser", 60);
  
  EXPECT_TRUE(jwtManager->revokeToken(token));
  EXPECT_TRUE(jwtManager->isTokenRevoked(token));
  
  auto payload = jwtManager->validateToken(token);
  EXPECT_FALSE(payload.has_value());
}

class SessionManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    sessionManager = new SessionManager();
  }
  
  void TearDown() override {
    delete sessionManager;
  }
  
  SessionManager* sessionManager;
};

TEST_F(SessionManagerTest, CreateSession) {
  std::string token = sessionManager->createSession(1, 1, "127.0.0.1", 60);
  
  EXPECT_FALSE(token.empty());
}

TEST_F(SessionManagerTest, ValidateSession) {
  std::string token = sessionManager->createSession(1, 1, "127.0.0.1", 60);
  auto session = sessionManager->validateSession(token);
  
  ASSERT_TRUE(session.has_value());
  EXPECT_EQ(session->accountId, 1);
  EXPECT_EQ(session->playerId, 1);
  EXPECT_EQ(session->ipAddress, "127.0.0.1");
}

TEST_F(SessionManagerTest, InvalidateSession) {
  std::string token = sessionManager->createSession(1, 1, "127.0.0.1", 60);
  
  EXPECT_TRUE(sessionManager->invalidateSession(token));
  
  auto session = sessionManager->validateSession(token);
  EXPECT_FALSE(session.has_value());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

