#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>
#include <memory>

namespace Umbra {
namespace Core {

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

