#pragma once
// #region agent log
// Instrumentação temporária (debug session f24ed2). Remover após verificação.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

namespace Umbra {
namespace Zone {

/** Logger NDJSON assíncrono: o hot path NÃO faz I/O de disco.
 *  Evidência: write síncrono no Proxmox media 2.7–3.7s e congelava pumpInbound
 *  (lateMs 900–2917 com silence_before_any_event ~3s), mesmo após try_lock
 *  (só protegia as outras threads; quem ganhava o lock ainda bloqueava). */
inline void agentDebugLog(const char* hypothesisId, const char* location, const char* message,
                          const std::string& dataJson = "{}", const char* runId = "post-fix") {
  struct LoggerState {
    std::mutex qMu;
    std::condition_variable cv;
    std::deque<std::string> q;
    std::atomic<bool> running{true};
    std::atomic<bool> started{false};
    std::thread worker;
    std::atomic<uint32_t> dropped{0};

    void start() {
      bool expected = false;
      if (!started.compare_exchange_strong(expected, true)) return;
      worker = std::thread([this]() {
        std::ofstream f;
        // CRÍTICO no Proxmox: NÃO escrever no disco da VM (/dev/loop0 com r_await~270ms).
        // dirty_expire_centisecs=3000 → flush a cada 30s → sleep_until(3ms) virava 2.6–3.0s
        // (medido: sleepMs dominante, pump/update=0). /tmp é tmpfs.
        f.open("/tmp/debug-f24ed2.log", std::ios::app);
        if (!f.is_open()) {
          f.open("/root/UmbraServerV2/debug-f24ed2.log", std::ios::app);
        }
        if (!f.is_open()) {
          f.open("debug-f24ed2.log", std::ios::app);
        }
        while (true) {
          std::string line;
          {
            std::unique_lock<std::mutex> lock(qMu);
            cv.wait_for(lock, std::chrono::milliseconds(100), [this]() {
              return !q.empty() || !running.load(std::memory_order_relaxed);
            });
            if (q.empty()) {
              if (!running.load(std::memory_order_relaxed)) break;
              continue;
            }
            line = std::move(q.front());
            q.pop_front();
          }
          if (f.is_open() && !line.empty()) {
            f << line;
          }
        }
      });
    }

    ~LoggerState() {
      running.store(false, std::memory_order_relaxed);
      cv.notify_all();
      if (worker.joinable()) worker.join();
    }
  };

  static LoggerState state;
  state.start();

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
  std::string line;
  line.reserve(256 + dataJson.size());
  line += "{\"sessionId\":\"f24ed2\",\"runId\":\"";
  line += runId;
  line += "\",\"hypothesisId\":\"";
  line += hypothesisId;
  line += "\",\"location\":\"";
  line += location;
  line += "\",\"message\":\"";
  line += message;
  line += "\",\"data\":";
  line += dataJson.empty() ? "{}" : dataJson;
  line += ",\"timestamp\":";
  line += std::to_string(ms);
  line += "}\n";

  {
    std::lock_guard<std::mutex> lock(state.qMu);
    // Bound: se o disco do Proxmox atrasar, descarta em vez de crescer sem limite.
    if (state.q.size() >= 2000) {
      state.dropped.fetch_add(1, std::memory_order_relaxed);
      return;
    }
    state.q.push_back(std::move(line));
  }
  state.cv.notify_one();
}

inline int64_t agentNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace Zone
}  // namespace Umbra
// #endregion
