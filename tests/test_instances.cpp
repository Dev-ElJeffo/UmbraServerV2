// test_instances.cpp — etapa 4: funções puras do InstanceManager e opcodes 134-136 / spawn trailing.
#include <gtest/gtest.h>

#include "zone/InstanceManager.hpp"
#include "zone/MovementProtocol.hpp"

using namespace Umbra::Zone;

TEST(InstanceSlots, AllocatesLowestFreeSlot) {
  EXPECT_EQ(allocateInstanceSlot({}, 5), 0u);
  EXPECT_EQ(allocateInstanceSlot({0, 1, 3}, 5), 2u);
  EXPECT_EQ(allocateInstanceSlot({1, 2}, 5), 0u);
}

TEST(InstanceSlots, ReturnsNoSlotWhenFull) {
  EXPECT_EQ(allocateInstanceSlot({0, 1, 2}, 3), kNoInstanceSlot);
  EXPECT_EQ(allocateInstanceSlot({}, 0), kNoInstanceSlot);
}

TEST(InstanceOrigin, CopiesAreSpacedOnY) {
  InstanceTemplateDef tpl;
  tpl.originX = 500000.f;
  tpl.originY = 1000.f;
  tpl.originZ = -200.f;
  tpl.slotSpacing = 200000.f;
  float x = 0.f, y = 0.f, z = 0.f;
  computeInstanceCopyOrigin(tpl, 0, x, y, z);
  EXPECT_FLOAT_EQ(x, 500000.f);
  EXPECT_FLOAT_EQ(y, 1000.f);
  EXPECT_FLOAT_EQ(z, -200.f);
  computeInstanceCopyOrigin(tpl, 3, x, y, z);
  EXPECT_FLOAT_EQ(y, 601000.f);
}

TEST(InstanceOrigin, CopiesNeverShareAoiCells) {
  // AOI = 3x3 células de 10000: cópias precisam ficar a mais de 30000 uma da outra.
  InstanceTemplateDef tpl;
  tpl.slotSpacing = 1000.f;  // valor inválido do Manager
  clampInstanceTemplate(tpl);
  EXPECT_GE(tpl.slotSpacing, 30000.f);
}

TEST(InstanceClose, ClosesWhenEmptyLongEnough) {
  EXPECT_FALSE(shouldCloseInstance(false, 0, 120, false));
  EXPECT_FALSE(shouldCloseInstance(true, 119000, 120, false));
  EXPECT_TRUE(shouldCloseInstance(true, 120000, 120, false));
}

TEST(InstanceClose, ClosesWhenTimedOutEvenWithPlayers) {
  EXPECT_TRUE(shouldCloseInstance(false, 0, 120, true));
}

TEST(InstanceTemplate, ClampsManagerValues) {
  InstanceTemplateDef tpl;
  tpl.minLevel = 0;
  tpl.maxPlayers = 0;
  tpl.emptyCloseSeconds = 0;
  tpl.maxCopies = 1000;
  tpl.timeLimitMinutes = 99999;
  clampInstanceTemplate(tpl);
  EXPECT_EQ(tpl.minLevel, 1u);
  EXPECT_EQ(tpl.maxPlayers, 5u);
  EXPECT_EQ(tpl.emptyCloseSeconds, 120u);
  EXPECT_EQ(tpl.maxCopies, 200u);
  EXPECT_EQ(tpl.timeLimitMinutes, 1440u);
}

TEST(InstanceProtocol, EnterRequestRoundTrip) {
  const auto data = encodeInstanceEnterRequest(1234);
  ASSERT_EQ(data.size(), 7u);
  EXPECT_EQ(data[0], 134);
  uint32_t npcId = 0;
  uint8_t action = 0;
  uint8_t value = 0;
  ASSERT_TRUE(decodeInstanceEnterRequest(data, npcId, action, value));
  EXPECT_EQ(npcId, 1234u);
  EXPECT_EQ(action, static_cast<uint8_t>(InstanceLobbyAction::Open));
  // Abrir exige o portal; pronto/cancelar não.
  EXPECT_FALSE(decodeInstanceEnterRequest(encodeInstanceEnterRequest(0), npcId, action, value));
  ASSERT_TRUE(decodeInstanceEnterRequest(encodeInstanceEnterRequest(0, InstanceLobbyAction::Ready), npcId, action, value));
  EXPECT_EQ(action, static_cast<uint8_t>(InstanceLobbyAction::Ready));
  // Frame curto (sem ação) = Open.
  auto shortFrame = data;
  shortFrame.resize(5);  // sem ação nem valor
  ASSERT_TRUE(decodeInstanceEnterRequest(shortFrame, npcId, action, value));
  EXPECT_EQ(action, static_cast<uint8_t>(InstanceLobbyAction::Open));
  // Ação inválida.
  auto bad = data;
  bad[5] = 9;
  EXPECT_FALSE(decodeInstanceEnterRequest(bad, npcId, action, value));
}

TEST(InstanceProtocol, LobbyStateRoundTrip) {
  InstanceLobbyStatePayload p;
  p.state = static_cast<uint8_t>(InstanceLobbyStateKind::Open);
  p.lobbyId = 3;
  p.templateId = 1;
  p.minLevel = 10;
  p.maxPlayers = 5;
  p.partyRequired = 1;
  p.timeLimitMinutes = 30;
  p.expiresInSeconds = 118;
  p.joinExisting = 0;
  p.name = "Cripta de teste";
  p.description = "Uma cripta antiga.";
  p.reason = "Ana abriu a sala.";
  p.members.push_back({11, 12, 1, 0, "Ana"});
  p.members.push_back({12, 8, 0, 1, "Bruno"});
  const auto data = encodeInstanceLobbyState(p);
  EXPECT_EQ(data[0], 137);
  InstanceLobbyStatePayload out;
  ASSERT_TRUE(decodeInstanceLobbyState(data, out));
  EXPECT_EQ(out.lobbyId, 3u);
  EXPECT_EQ(out.minLevel, 10);
  EXPECT_EQ(out.timeLimitMinutes, 30);
  EXPECT_EQ(out.expiresInSeconds, 118);
  EXPECT_EQ(out.description, p.description);
  EXPECT_EQ(out.reason, p.reason);
  ASSERT_EQ(out.members.size(), 2u);
  EXPECT_EQ(out.members[0].name, "Ana");
  EXPECT_EQ(out.members[0].ready, 1);
  EXPECT_EQ(out.members[1].level, 8);
  EXPECT_EQ(out.members[1].flags, 1);
}

TEST(InstanceLobby, AllReadyNeedsEveryoneReadyAndLevel) {
  const std::vector<uint32_t> members{1, 2};
  std::unordered_map<uint32_t, bool> ready{{1, true}, {2, false}};
  std::unordered_map<uint32_t, uint32_t> levels{{1, 10}, {2, 10}};
  EXPECT_FALSE(lobbyAllReady(members, ready, levels, 5));
  ready[2] = true;
  EXPECT_TRUE(lobbyAllReady(members, ready, levels, 5));
  levels[2] = 3;  // abaixo do mínimo
  EXPECT_FALSE(lobbyAllReady(members, ready, levels, 5));
  EXPECT_FALSE(lobbyAllReady({}, ready, levels, 1));
}

TEST(InstanceProtocol, StateRoundTrip) {
  InstanceStatePayload p;
  p.state = static_cast<uint8_t>(InstanceStateKind::Entered);
  p.instanceId = 7;
  p.templateId = 2;
  p.originX = 500000.f;
  p.originY = 200000.f;
  p.originZ = 0.f;
  p.posX = 500100.f;
  p.posY = 200050.f;
  p.posZ = 120.f;
  p.yaw = 90.f;
  p.expiresAtUnix = 1800000000u;
  p.mapPath = "/Game/Maps/Instances/Lvl_Inst_Test.Lvl_Inst_Test";
  p.name = "Cripta de teste";
  p.reason = "";
  const auto data = encodeInstanceState(p);
  EXPECT_EQ(data[0], 135);
  InstanceStatePayload out;
  ASSERT_TRUE(decodeInstanceState(data, out));
  EXPECT_EQ(out.state, p.state);
  EXPECT_EQ(out.instanceId, 7u);
  EXPECT_EQ(out.templateId, 2u);
  EXPECT_FLOAT_EQ(out.originY, 200000.f);
  EXPECT_FLOAT_EQ(out.posX, 500100.f);
  EXPECT_FLOAT_EQ(out.yaw, 90.f);
  EXPECT_EQ(out.expiresAtUnix, 1800000000u);
  EXPECT_EQ(out.mapPath, p.mapPath);
  EXPECT_EQ(out.name, p.name);
}

TEST(InstanceProtocol, LeaveRequestIsOneByte) {
  const auto data = encodeInstanceLeaveRequest();
  ASSERT_EQ(data.size(), 1u);
  EXPECT_EQ(data[0], 136);
}

TEST(InstanceProtocol, NpcSpawnPortalIsTrailing) {
  NpcSpawnPayload p;
  p.npcId = 10;
  p.templateId = 3;
  p.npcName = "Portal";
  p.flags = 0x08;
  p.portalInstanceTemplateId = 4;
  auto data = encodeNpcSpawnNotify(p);
  NpcSpawnPayload out;
  ASSERT_TRUE(decodeNpcSpawnNotify(data, out));
  EXPECT_EQ(out.portalInstanceTemplateId, 4u);
  // Frame antigo (sem o campo): decoder tolera e fica 0.
  data.resize(data.size() - 4);
  NpcSpawnPayload old;
  ASSERT_TRUE(decodeNpcSpawnNotify(data, old));
  EXPECT_EQ(old.portalInstanceTemplateId, 0u);
}
