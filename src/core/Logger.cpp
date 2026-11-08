#include "Logger.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <filesystem>

namespace Umbra {
namespace Core {

namespace {
// Fila do logger assíncrono. Cheia → descarta as mais antigas (nunca bloqueia o jogo).
constexpr size_t kAsyncQueueSize = 32768;
}  // namespace

Logger& Logger::getInstance() {
  // O registry do spdlog precisa ser construído ANTES do Logger para ser destruído
  // DEPOIS dele: ~Logger drena a fila assíncrona e chama spdlog::shutdown().
  spdlog::details::registry::instance();
  static Logger instance;
  return instance;
}

Logger::~Logger() {
  if (logger_) {
    logger_->flush();
    logger_.reset();
    spdlog::shutdown();  // drena a fila assíncrona e encerra a thread de escrita
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
    if (logPath.has_parent_path()) {
      std::filesystem::create_directories(logPath.parent_path());
    }

    // Console: INFO por padrão (DEBUG/TRACE só no arquivo). Arquivo: segue setLevel().
    consoleSink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink_->set_level(consoleEnabled_ ? consoleLevel_ : spdlog::level::off);

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      logFile, maxSize, maxFiles);
    file_sink->set_level(spdlog::level::trace);

    // Logger assíncrono: a thread chamadora só formata e enfileira.
    spdlog::init_thread_pool(kAsyncQueueSize, 1);
    std::vector<spdlog::sink_ptr> sinks{consoleSink_, file_sink};
    logger_ = std::make_shared<spdlog::async_logger>(
        "umbra", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest);

    // Set pattern
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    logger_->set_level(spdlog::level::debug);

    // Antes não havia política de flush: o arquivo ficava minutos atrasado e as
    // últimas linhas antes de um crash se perdiam.
    logger_->flush_on(spdlog::level::warn);

    // Register as default logger (também registra no registry → flush_every alcança)
    spdlog::set_default_logger(logger_);
    spdlog::flush_every(std::chrono::seconds(1));

    initialized_ = true;
    logger_->info("Logger initialized successfully");
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
  }
}

spdlog::level::level_enum Logger::toSpd(Level level) {
  switch (level) {
    case Level::TRACE: return spdlog::level::trace;
    case Level::DEBUG: return spdlog::level::debug;
    case Level::INFO: return spdlog::level::info;
    case Level::WARN: return spdlog::level::warn;
    case Level::ERROR: return spdlog::level::err;
    case Level::CRITICAL: return spdlog::level::critical;
  }
  return spdlog::level::info;
}

Logger::Level Logger::parseLevel(const std::string& level) {
  std::string up = level;
  std::transform(up.begin(), up.end(), up.begin(),
                 [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  if (up == "TRACE") return Level::TRACE;
  if (up == "DEBUG") return Level::DEBUG;
  if (up == "WARN" || up == "WARNING") return Level::WARN;
  if (up == "ERROR") return Level::ERROR;
  if (up == "CRITICAL") return Level::CRITICAL;
  return Level::INFO;
}

void Logger::setLevel(Level level) {
  if (!logger_) {
    return;
  }
  logger_->set_level(toSpd(level));
}

void Logger::setConsoleLevel(Level level) {
  consoleLevel_ = toSpd(level);
  if (consoleSink_ && consoleEnabled_) {
    consoleSink_->set_level(consoleLevel_);
  }
}

void Logger::setConsoleEnabled(bool enabled) {
  consoleEnabled_ = enabled;
  if (consoleSink_) {
    consoleSink_->set_level(enabled ? consoleLevel_ : spdlog::level::off);
  }
}

void Logger::applyConfig(const std::string& level, bool consoleOutput) {
  setLevel(parseLevel(level));
  setConsoleEnabled(consoleOutput);
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

