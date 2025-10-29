#include "Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <iomanip>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

namespace Umbra {
namespace Core {

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  
  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  
  return tokens;
}

std::string Utils::trim(const std::string& str) {
  auto start = std::find_if_not(str.begin(), str.end(), 
    [](unsigned char ch) { return std::isspace(ch); });
  
  auto end = std::find_if_not(str.rbegin(), str.rend(), 
    [](unsigned char ch) { return std::isspace(ch); }).base();
  
  return (start < end) ? std::string(start, end) : std::string();
}

std::string Utils::toLower(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
    [](unsigned char c) { return std::tolower(c); });
  return result;
}

std::string Utils::toUpper(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
    [](unsigned char c) { return std::toupper(c); });
  return result;
}

bool Utils::isValidEmail(const std::string& email) {
  static const std::regex pattern(
    R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
  return std::regex_match(email, pattern);
}

std::string Utils::generateRandomString(size_t length) {
  static const char charset[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  
  std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
  auto& rng = getRandomGenerator();
  
  std::string result;
  result.reserve(length);
  
  for (size_t i = 0; i < length; ++i) {
    result += charset[dist(rng)];
  }
  
  return result;
}

std::string Utils::generateUUID() {
  std::uniform_int_distribution<> dist(0, 15);
  std::uniform_int_distribution<> dist2(8, 11);
  auto& rng = getRandomGenerator();
  
  std::stringstream ss;
  ss << std::hex;
  
  for (int i = 0; i < 8; i++) ss << dist(rng);
  ss << "-";
  for (int i = 0; i < 4; i++) ss << dist(rng);
  ss << "-4";
  for (int i = 0; i < 3; i++) ss << dist(rng);
  ss << "-";
  ss << dist2(rng);
  for (int i = 0; i < 3; i++) ss << dist(rng);
  ss << "-";
  for (int i = 0; i < 12; i++) ss << dist(rng);
  
  return ss.str();
}

int64_t Utils::getCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

std::string Utils::formatTimestamp(int64_t timestamp) {
  auto time = std::chrono::system_clock::time_point(
    std::chrono::milliseconds(timestamp));
  auto time_t = std::chrono::system_clock::to_time_t(time);
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%Y-%m-%dT%H:%M:%S");
  
  return ss.str();
}

std::string Utils::hashPassword(const std::string& password, 
                                const std::string& salt) {
  // Implementação usando PBKDF2 (Password-Based Key Derivation Function 2)
  // Alternativa segura ao bcrypt, usando OpenSSL
  // Formato: $pbkdf2$iterations$salt$hash
  
  unsigned char derivedKey[32]; // SHA-256 output
  int iterations = 100000; // Número de iterações (ajustável)
  
  // Gerar salt aleatório se não fornecido
  std::string actualSalt;
  std::string saltBytes;
  
  if (salt.empty()) {
    // Gerar novo salt
    unsigned char saltBytesRaw[16];
    if (RAND_bytes(saltBytesRaw, sizeof(saltBytesRaw)) != 1) {
      // Fallback para gerador não-crypto-seguro se OpenSSL falhar
      std::mt19937& rng = getRandomGenerator();
      for (size_t i = 0; i < sizeof(saltBytesRaw); ++i) {
        saltBytesRaw[i] = static_cast<unsigned char>(rng() & 0xFF);
      }
    }
    saltBytes = std::string(reinterpret_cast<const char*>(saltBytesRaw), sizeof(saltBytesRaw));
    actualSalt = base64Encode(saltBytes);
  } else {
    // Salt fornecido - assumir que está em Base64 (formato do hash antigo) ou usar diretamente
    actualSalt = salt;
    saltBytes = base64Decode(salt);
    // Se decodificação falhar ou resultar vazio, usar salt diretamente
    if (saltBytes.empty()) {
      saltBytes = salt;
      actualSalt = base64Encode(salt);
    }
  }
  
  // Gerar hash usando PBKDF2
  if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.length()),
                         reinterpret_cast<const unsigned char*>(saltBytes.data()),
                         static_cast<int>(saltBytes.length()),
                         iterations,
                         EVP_sha256(),
                         32,
                         derivedKey) != 1) {
    // Fallback para hash simples se PBKDF2 falhar
    return hashPasswordSimple(password, actualSalt);
  }
  
  // Codificar hash em Base64
  std::string hashStr = base64Encode(std::string(reinterpret_cast<const char*>(derivedKey), 32));
  
  // Retornar formato: $pbkdf2$iterations$salt$hash
  std::stringstream ss;
  ss << "$pbkdf2$" << iterations << "$" << actualSalt << "$" << hashStr;
  return ss.str();
}

bool Utils::verifyPassword(const std::string& password, 
                           const std::string& hash) {
  // Verificar formato de hash
  if (hash.substr(0, 8) == "$pbkdf2$") {
    // Formato PBKDF2: $pbkdf2$iterations$salt$hash
    std::vector<std::string> parts = split(hash, '$');
    if (parts.size() != 5 || parts[0] != "" || parts[1] != "pbkdf2") {
      return false;
    }
    
    int iterations = std::stoi(parts[2]);
    std::string salt = parts[3];
    std::string storedHash = parts[4];
    
    // Decodificar salt
    std::string saltBytes = base64Decode(salt);
    
    // Calcular hash
    unsigned char derivedKey[32];
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.length()),
                           reinterpret_cast<const unsigned char*>(saltBytes.data()),
                           static_cast<int>(saltBytes.length()),
                           iterations,
                           EVP_sha256(),
                           32,
                           derivedKey) != 1) {
      return false;
    }
    
    // Codificar e comparar
    std::string computedHash = base64Encode(std::string(reinterpret_cast<const char*>(derivedKey), 32));
    return computedHash == storedHash;
  } else {
    // Fallback para hash simples (compatibilidade)
    std::string computed = hashPasswordSimple(password);
    return computed == hash;
  }
}

// Função auxiliar para hash simples (fallback)
std::string Utils::hashPasswordSimple(const std::string& password, 
                                      const std::string& salt) {
  std::string combined = password + salt;
  std::hash<std::string> hasher;
  size_t hash = hasher(combined);
  
  std::stringstream ss;
  ss << std::hex << hash;
  return ss.str();
}

std::string Utils::sanitizeInput(const std::string& input) {
  std::string result = input;
  
  // Replace dangerous characters
  std::vector<std::pair<std::string, std::string>> replacements = {
    {"'", "''"},
    {"\\", "\\\\"},
    {"\0", ""},
    {"\n", "\\n"},
    {"\r", "\\r"},
    {"\x1a", "\\Z"}
  };
  
  for (const auto& [from, to] : replacements) {
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
      result.replace(pos, from.length(), to);
      pos += to.length();
    }
  }
  
  return result;
}

std::string Utils::base64Encode(const std::string& input) {
  static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
  
  std::string result;
  int val = 0;
  int valb = -6;
  
  for (unsigned char c : input) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      result.push_back(base64_chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  
  if (valb > -6) {
    result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
  }
  
  while (result.size() % 4) {
    result.push_back('=');
  }
  
  return result;
}

std::string Utils::base64Decode(const std::string& input) {
  static const int base64_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
  };
  
  std::string result;
  int val = 0;
  int valb = -8;
  
  for (unsigned char c : input) {
    if (base64_table[c] == -1) break;
    val = (val << 6) + base64_table[c];
    valb += 6;
    if (valb >= 0) {
      result.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  
  return result;
}

std::mt19937& Utils::getRandomGenerator() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return gen;
}

}  // namespace Core
}  // namespace Umbra

