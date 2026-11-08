#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Umbra {
namespace Core {

/**
 * @brief Sistema de logging assíncrono com níveis e rotação
 *
 * Wrapper para spdlog: console + arquivo com rotação, escrita em thread própria
 * (a thread do jogo só enfileira; nunca espera I/O de disco/console). Fila cheia
 * descarta as mensagens mais antigas em vez de bloquear.
 * Flush: imediato em WARN+ e periódico (1s) para o resto.
 */
class Logger {
 public:
  enum class Level {
    TRACE = -1,  // por frame / por pacote (movimento, queries vazias): só em investigação
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

  /**
   * @brief Nível mínimo do console (padrão INFO). O arquivo segue setLevel().
   * Console do Windows é lento e trava com seleção de texto (QuickEdit).
   */
  void setConsoleLevel(Level level);

  /** @brief Liga/desliga o console (config logging.console_output). */
  void setConsoleEnabled(bool enabled);

  /**
   * @brief Aplica logging.level ("TRACE".."CRITICAL") e logging.console_output da config.
   * Nível desconhecido → INFO.
   */
  void applyConfig(const std::string& level, bool consoleOutput);

  /** @brief Converte "TRACE"/"DEBUG"/"INFO"/"WARN"/"ERROR"/"CRITICAL" (padrão INFO). */
  static Level parseLevel(const std::string& level);

  // Métodos de logging
  void debug(const std::string& message);
  void info(const std::string& message);
  void warn(const std::string& message);
  void error(const std::string& message);
  void critical(const std::string& message);
  
  // Template para logging formatado
  template<typename... Args>
  void trace(const std::string& format, Args... args);

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
  std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consoleSink_;
  spdlog::level::level_enum consoleLevel_ = spdlog::level::info;
  bool consoleEnabled_ = true;
  bool initialized_ = false;

  static spdlog::level::level_enum toSpd(Level level);
};

// Template implementations
template<typename... Args>
void Logger::trace(const std::string& format, Args... args) {
  if (logger_) {
    logger_->trace(format, args...);
  }
}

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

