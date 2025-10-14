#pragma once

#include "LoadBalancer.hpp"
#include "auth/JWTManager.hpp"
#include <memory>
#include <string>

namespace Umbra {
namespace Gateway {

class GatewayServer {
 public:
  struct Config {
    uint16_t port = 9000;
    std::string jwtSecret;
    uint32_t rateLimitPerSecond = 100;
  };
  
  GatewayServer(const Config& config);
  ~GatewayServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  
  LoadBalancer& getLoadBalancer();

 private:
  Config config_;
  std::unique_ptr<LoadBalancer> loadBalancer_;
  std::unique_ptr<Auth::JWTManager> jwtManager_;
  bool running_;
  
  void handleConnection(uint32_t clientId);
};

}  // namespace Gateway
}  // namespace Umbra

