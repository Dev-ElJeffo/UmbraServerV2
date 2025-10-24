#include "ConfigManager.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>

namespace Umbra {
namespace Core {

ConfigManager& ConfigManager::getInstance() {
  static ConfigManager instance;
  return instance;
}

bool ConfigManager::loadConfig(const std::string& filepath) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      Logger::getInstance().error("Failed to open config file: " + filepath);
      return false;
    }
    
    file >> config_;
    filepath_ = filepath;
    
    Logger::getInstance().info("Config loaded successfully: " + filepath);
    return true;
  } catch (const nlohmann::json::exception& e) {
    Logger::getInstance().error("JSON parse error: " + std::string(e.what()));
    return false;
  }
}

bool ConfigManager::hasKey(const std::string& key) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto value = navigateToKey(key);
    return !value.is_null();
  } catch (const std::exception&) {
    return false;
  }
}

bool ConfigManager::reload() {
  if (filepath_.empty()) {
    Logger::getInstance().warn("Cannot reload: no config file loaded");
    return false;
  }
  
  return loadConfig(filepath_);
}

const nlohmann::json& ConfigManager::getJson() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return config_;
}

bool ConfigManager::validate(const nlohmann::json& schema) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // TODO: Implement JSON schema validation
  // For now, just check if config is not empty
  return !config_.empty();
}

nlohmann::json ConfigManager::navigateToKey(const std::string& key) const {
  nlohmann::json current = config_;
  std::istringstream ss(key);
  std::string token;
  
  while (std::getline(ss, token, '.')) {
    if (!current.contains(token)) {
      return nlohmann::json();
    }
    current = current[token];
  }
  
  return current;
}

}  // namespace Core
}  // namespace Umbra

