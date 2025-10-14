#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Umbra {
namespace Core {

/**
 * @brief Sistema de logging assíncrono com níveis e rotação
 * 
 * Wrapper para spdlog com configuração padrão e suporte a
 * múltiplos sinks (console + arquivo com rotação).
 */
class Logger {
 public:
  enum class Level {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    CRITICAL = 4
  };
  
  static Logger& getInstance();
  
  /**
   * @brief Inicializa o logger
   * @param logFile Caminho para arquivo de log
   * @param maxSize Tamanho máximo do arquivo (bytes)
   * @param maxFiles Número máximo de arquivos rotacionados
   */
  void initialize(const std::string& logFile = "logs/umbra_server.log",
                  size_t maxSize = 1024 * 1024 * 10,  // 10MB
                  size_t maxFiles = 5);
  
  /**
   * @brief Define nível mínimo de log
   * @param level Nível mínimo
   */
  void setLevel(Level level);
  
  // Métodos de logging
  void debug(const std::string& message);
  void info(const std::string& message);
  void warn(const std::string& message);
  void error(const std::string& message);
  void critical(const std::string& message);
  
  // Template para logging formatado
  template<typename... Args>
  void debug(const std::string& format, Args... args);
  
  template<typename... Args>
  void info(const std::string& format, Args... args);
  
  template<typename... Args>
  void warn(const std::string& format, Args... args);
  
  template<typename... Args>
  void error(const std::string& format, Args... args);
  
  template<typename... Args>
  void critical(const std::string& format, Args... args);
  
  /**
   * @brief Força flush dos logs para disco
   */
  void flush();

 private:
  Logger() = default;
  ~Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  
  std::shared_ptr<spdlog::logger> logger_;
  bool initialized_ = false;
};

// Template implementations
template<typename... Args>
void Logger::debug(const std::string& format, Args... args) {
  if (logger_) {
    logger_->debug(format, args...);
  }
}

template<typename... Args>
void Logger::info(const std::string& format, Args... args) {
  if (logger_) {
    logger_->info(format, args...);
  }
}

template<typename... Args>
void Logger::warn(const std::string& format, Args... args) {
  if (logger_) {
    logger_->warn(format, args...);
  }
}

template<typename... Args>
void Logger::error(const std::string& format, Args... args) {
  if (logger_) {
    logger_->error(format, args...);
  }
}

template<typename... Args>
void Logger::critical(const std::string& format, Args... args) {
  if (logger_) {
    logger_->critical(format, args...);
  }
}

}  // namespace Core
}  // namespace Umbra

