#include "AdminCrypto.hpp"
#include "core/Utils.hpp"
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <iomanip>
#include <sstream>

namespace Umbra {
namespace Admin {

std::string hmacSha256Hex(const std::string& key, const std::string& message) {
  unsigned char digest[EVP_MAX_MD_SIZE];
  unsigned int digestLen = 0;
  HMAC(EVP_sha256(),
       key.data(), static_cast<int>(key.size()),
       reinterpret_cast<const unsigned char*>(message.data()), message.size(),
       digest, &digestLen);

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (unsigned int i = 0; i < digestLen; ++i) {
    oss << std::setw(2) << static_cast<int>(digest[i]);
  }
  return oss.str();
}

bool hmacEquals(const std::string& expected, const std::string& actual) {
  if (expected.size() != actual.size()) {
    return false;
  }
  if (expected.empty()) {
    return false;
  }
  return CRYPTO_memcmp(expected.data(), actual.data(), expected.size()) == 0;
}

std::string generateNonce() {
  return Core::Utils::generateRandomString(32);
}

}  // namespace Admin
}  // namespace Umbra
