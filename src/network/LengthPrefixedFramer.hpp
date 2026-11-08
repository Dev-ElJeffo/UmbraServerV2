#pragma once
// LengthPrefixedFramer.hpp — remonta mensagens TCP com prefixo de tamanho (4 bytes big-endian).
//
// Contrato do cliente UE (UmbraTCPClient::SendMessage): [len:u32 BE][payload len bytes].
// O SocketServer entrega pedaços crus de recv() (sem fronteira de mensagem); este framer
// acumula por conexão e só libera mensagens completas.
//
// Compatibilidade: o modo é decidido pelo 1º byte da conexão.
//  - 0x00  → Framed (tamanho <= kMaxFrameSize < 16 MB ⇒ byte alto sempre 0).
//  - outro → Legacy: cada pedaço é repassado como está (JSON cru / Base64 de scripts antigos).
// JSON começa com '{' e Base64 nunca contém 0x00, então a detecção não é ambígua.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Umbra {
namespace Network {

class LengthPrefixedFramer {
 public:
  enum class Mode { Unknown, Framed, Legacy };
  enum class Result { Ok, InvalidFrameSize };

  static constexpr uint32_t kDefaultMaxFrameSize = 64 * 1024;

  explicit LengthPrefixedFramer(uint32_t maxFrameSize = kDefaultMaxFrameSize)
      : maxFrameSize_(maxFrameSize) {}

  /**
   * @brief Alimenta bytes recebidos e extrai mensagens completas.
   * @param out Recebe os payloads completos (sem o prefixo), na ordem de chegada.
   * @return InvalidFrameSize se o tamanho for 0 ou > máximo (conexão deve ser fechada).
   */
  Result feed(const uint8_t* data, size_t size, std::vector<std::vector<uint8_t>>& out) {
    if (size == 0) return Result::Ok;

    if (mode_ == Mode::Unknown) {
      mode_ = (data[0] == 0x00) ? Mode::Framed : Mode::Legacy;
    }

    if (mode_ == Mode::Legacy) {
      out.emplace_back(data, data + size);
      return Result::Ok;
    }

    buffer_.insert(buffer_.end(), data, data + size);

    size_t offset = 0;
    while (buffer_.size() - offset >= 4) {
      const uint32_t frameSize = (static_cast<uint32_t>(buffer_[offset]) << 24) |
                                 (static_cast<uint32_t>(buffer_[offset + 1]) << 16) |
                                 (static_cast<uint32_t>(buffer_[offset + 2]) << 8) |
                                 static_cast<uint32_t>(buffer_[offset + 3]);
      if (frameSize == 0 || frameSize > maxFrameSize_) {
        buffer_.clear();
        return Result::InvalidFrameSize;
      }
      if (buffer_.size() - offset - 4 < frameSize) break;

      const auto begin = buffer_.begin() + static_cast<std::ptrdiff_t>(offset + 4);
      out.emplace_back(begin, begin + frameSize);
      offset += 4 + frameSize;
    }

    if (offset > 0) {
      buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(offset));
    }
    return Result::Ok;
  }

  Mode mode() const { return mode_; }
  bool isFramed() const { return mode_ == Mode::Framed; }
  size_t pendingBytes() const { return buffer_.size(); }

  /** @brief Monta [len:u32 BE][payload] — mesmo formato que o cliente envia. */
  static std::vector<uint8_t> encode(const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame;
    frame.reserve(payload.size() + 4);
    const auto len = static_cast<uint32_t>(payload.size());
    frame.push_back(static_cast<uint8_t>((len >> 24) & 0xFF));
    frame.push_back(static_cast<uint8_t>((len >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(len & 0xFF));
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
  }

 private:
  uint32_t maxFrameSize_;
  Mode mode_ = Mode::Unknown;
  std::vector<uint8_t> buffer_;
};

}  // namespace Network
}  // namespace Umbra
