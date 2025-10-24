#include "Logger.hpp"
#include <iostream>
#include <filesystem>

namespace Umbra {
namespace Core {

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
}

Logger::~Logger() {
  if (logger_) {
    logger_->flush();
  }
}

void Logger::initialize(const std::string& logFile, 
                        size_t maxSize, 
                        size_t maxFiles) {
  if (initialized_) {
    return;
  }
  
  try {
    // Create logs directory if it doesn't exist
    std::filesystem::path logPath(logFile);
    std::filesystem::create_directories(logPath.parent_path());
    
    // Create sinks
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      logFile, maxSize, maxFiles);
    file_sink->set_level(spdlog::level::debug);
    
    // Create logger with both sinks
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    logger_ = std::make_shared<spdlog::logger>("umbra", sinks.begin(), sinks.end());
    
    // Set pattern
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    logger_->set_level(spdlog::level::debug);
    
    // Register as default logger
    spdlog::set_default_logger(logger_);
    
    initialized_ = true;
    logger_->info("Logger initialized successfully");
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
  }
}

void Logger::setLevel(Level level) {
  if (!logger_) {
    return;
  }
  
  switch (level) {
    case Level::DEBUG:
      logger_->set_level(spdlog::level::debug);
      break;
    case Level::INFO:
      logger_->set_level(spdlog::level::info);
      break;
    case Level::WARN:
      logger_->set_level(spdlog::level::warn);
      break;
    case Level::ERROR:
      logger_->set_level(spdlog::level::err);
      break;
    case Level::CRITICAL:
      logger_->set_level(spdlog::level::critical);
      break;
  }
}

void Logger::debug(const std::string& message) {
  if (logger_) {
    logger_->debug(message);
  }
}

void Logger::info(const std::string& message) {
  if (logger_) {
    logger_->info(message);
  }
}

void Logger::warn(const std::string& message) {
  if (logger_) {
    logger_->warn(message);
  }
}

void Logger::error(const std::string& message) {
  if (logger_) {
    logger_->error(message);
  }
}

void Logger::critical(const std::string& message) {
  if (logger_) {
    logger_->critical(message);
  }
}

void Logger::flush() {
  if (logger_) {
    logger_->flush();
  }
}

}  // namespace Core
}  // namespace Umbra

