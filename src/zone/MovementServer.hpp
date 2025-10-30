#pragma once

#include <unordered_map>
#include <mutex>
#include <chrono>
#include "network/WebSocketServer.hpp"
#include "zone/MovementProtocol.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

struct PlayerStateNet {
  uint32_t playerId;
  float x, y, z;
  float yaw;
  uint32_t tsMs;
};

class MovementServer {
public:
  explicit MovementServer(uint16_t port)
    : ws_(port) {}

  bool start() {
    ws_.setConnectionCallback([this](uint32_t cid, bool connected){
      if (connected) {
        Umbra::Core::Logger::getInstance().info("WS client {} connected", cid);
      } else {
        Umbra::Core::Logger::getInstance().info("WS client {} disconnected", cid);
      }
    });

    ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data){
      MovementFrame f{};
      if (!decode(data, f)) return;
      if (f.type != MovementMsgType::MoveUpdate) return;
      handleMoveUpdate(cid, f);
    });

    return ws_.start();
  }

  void stop() { ws_.stop(); }

  // snapshot a 10–20 Hz deve ser chamado externamente por um timer
  void broadcastSnapshot() {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encode(f);
      ws_.broadcastBinary(bytes);
    }
  }

  void setLimits(float maxSpeed, float maxTeleportDist, uint32_t maxDelayMs) {
    maxSpeed_ = maxSpeed; maxTeleportDist_ = maxTeleportDist; maxDelayMs_ = maxDelayMs;
  }

private:
  void handleMoveUpdate(uint32_t /*cid*/, const MovementFrame& f) {
    uint32_t nowMs = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());

    if (nowMs > f.tsMs && (nowMs - f.tsMs) > maxDelayMs_) return;

    std::lock_guard<std::mutex> lock(mu_);
    auto it = players_.find(f.playerId);
    if (it != players_.end()) {
      float dx = f.x - it->second.x;
      float dy = f.y - it->second.y;
      float dz = f.z - it->second.z;
      float dist2 = dx*dx + dy*dy + dz*dz;
      if (dist2 > maxTeleportDist_ * maxTeleportDist_) return;
      // velocidade aprox em uu/s se tsMs coerente
      float dt = (f.tsMs > it->second.tsMs) ? (f.tsMs - it->second.tsMs) / 1000.0f : 0.016f;
      float speed = (dt > 0.0f) ? std::sqrt(dist2) / dt : 0.0f;
      if (speed > maxSpeed_) return;
    }
    players_[f.playerId] = PlayerStateNet{f.playerId, f.x, f.y, f.z, f.yaw, f.tsMs};

    // broadcast imediato de state_update (além do snapshot periódico)
    MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, f.tsMs};
    ws_.broadcastBinary(encode(out));
  }

  Umbra::Network::WebSocketServer ws_;
  std::mutex mu_;
  std::unordered_map<uint32_t, PlayerStateNet> players_;
  float maxSpeed_ = 1200.0f;
  float maxTeleportDist_ = 3000.0f;
  uint32_t maxDelayMs_ = 300;
};

} // namespace Zone
} // namespace Umbra


