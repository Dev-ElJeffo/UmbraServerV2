#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>
#include <memory>
#include <cstdint>

namespace Umbra {
namespace Core {

/** @brief ms monotônicos (steady_clock) — para throttles, timeouts e medição. */
inline int64_t steadyNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

/**
 * @brief Tempo de CPU (user+kernel) consumido pela thread atual, em ms.
 * Comparado ao relógio, separa trabalho real de preempção pelo SO (thread parada
 * esperando CPU). Windows: GetThreadTimes (resolução ~15,6ms — avança por tick do
 * relógio do SO); Linux: CLOCK_THREAD_CPUTIME_ID (ns).
 */
int64_t threadCpuMs();

/**
 * @brief Limita a frequência de um log de anomalia (thread-safe, sem lock).
 *
 * Uso: `static Core::LogThrottle t(5000); if (t.allow()) Logger.warn(...)`.
 * suppressed() devolve quantos eventos foram omitidos desde o último log liberado.
 */
class LogThrottle {
 public:
  explicit LogThrottle(int64_t intervalMs) : intervalMs_(intervalMs) {}

  bool allow() {
    const int64_t now = steadyNowMs();
    int64_t last = lastMs_.load(std::memory_order_relaxed);
    if (last != 0 && now - last < intervalMs_) {
      suppressed_.fetch_add(1, std::memory_order_relaxed);
      return false;
    }
    return lastMs_.compare_exchange_strong(last, now, std::memory_order_relaxed);
  }

  /** @brief Eventos omitidos desde o último allow()==true (e zera o contador). */
  uint32_t takeSuppressed() { return suppressed_.exchange(0, std::memory_order_relaxed); }

 private:
  int64_t intervalMs_;
  std::atomic<int64_t> lastMs_{0};
  std::atomic<uint32_t> suppressed_{0};
};

/**
 * @brief Sistema de agendamento de tarefas periódicas
 * 
 * Permite agendar callbacks para execução em intervalos
 * regulares ou uma única vez após um delay.
 */
class Timer {
 public:
  using Callback = std::function<void()>;
  using Duration = std::chrono::milliseconds;
  
  Timer();
  ~Timer();
  
  /**
   * @brief Agenda tarefa para execução repetida
   * @param callback Função a ser executada
   * @param interval Intervalo entre execuções
   * @return ID da tarefa agendada
   */
  uint32_t scheduleRepeating(Callback callback, Duration interval);
  
  /**
   * @brief Agenda tarefa para execução única
   * @param callback Função a ser executada
   * @param delay Delay antes da execução
   * @return ID da tarefa agendada
   */
  uint32_t scheduleOnce(Callback callback, Duration delay);
  
  /**
   * @brief Cancela tarefa agendada
   * @param taskId ID da tarefa
   * @return true se cancelada com sucesso
   */
  bool cancel(uint32_t taskId);
  
  /**
   * @brief Inicia o timer
   */
  void start();
  
  /**
   * @brief Para o timer e cancela todas as tarefas
   */
  void stop();
  
  /**
   * @brief Verifica se o timer está rodando
   * @return true se ativo
   */
  bool isRunning() const;

 private:
  struct Task {
    uint32_t id;
    Callback callback;
    Duration interval;
    std::chrono::steady_clock::time_point nextRun;
    bool repeating;
    bool cancelled;
  };
  
  std::vector<std::shared_ptr<Task>> tasks_;
  std::mutex tasksMutex_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> workerThread_;
  std::atomic<uint32_t> nextTaskId_;
  
  void workerLoop();
  void executeTasks();
};

}  // namespace Core
}  // namespace Umbra

