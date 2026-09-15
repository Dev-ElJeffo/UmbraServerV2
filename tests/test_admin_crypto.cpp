#include "admin/AdminCrypto.hpp"
#include <iostream>

int main() {
  using Umbra::Admin::hmacEquals;
  using Umbra::Admin::hmacSha256Hex;
  const auto a = hmacSha256Hex("secret", "nonce");
  const auto b = hmacSha256Hex("secret", "nonce");
  if (!hmacEquals(a, b)) {
    std::cerr << "equal hmac failed\n";
    return 1;
  }
  if (hmacEquals(a, hmacSha256Hex("other", "nonce"))) {
    std::cerr << "mismatch should fail\n";
    return 1;
  }
  if (hmacEquals("", "")) {
    std::cerr << "empty should fail\n";
    return 1;
  }
  std::cout << "OK\n";
  return 0;
}
