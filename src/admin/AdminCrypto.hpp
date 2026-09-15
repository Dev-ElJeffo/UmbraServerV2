#pragma once

#include <string>

namespace Umbra {
namespace Admin {

/** HMAC-SHA256 em hexadecimal (lowercase). */
std::string hmacSha256Hex(const std::string& key, const std::string& message);

/** Comparação em tempo constante de strings hex HMAC. */
bool hmacEquals(const std::string& expected, const std::string& actual);

/** Gera nonce aleatório hex (32 chars). */
std::string generateNonce();

}  // namespace Admin
}  // namespace Umbra
