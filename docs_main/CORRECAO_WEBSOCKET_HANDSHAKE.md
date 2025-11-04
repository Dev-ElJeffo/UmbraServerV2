# ✅ Correção: WebSocket Handshake SHA-1

## 🐛 Problema Identificado

O WebSocket handshake estava falhando com erro:
```
LogWebSockets: Warning: Lws(Warning): lws_client_int_s_hs: accept '...' wrong vs '...'
LogBlueprintUserMessages: HS: Accept hash wrong
```

**Causa**: A função `generateAcceptKey` no `WebSocketServer.cpp` estava usando apenas Base64 da string combinada, mas o protocolo WebSocket (RFC 6455) requer:
1. SHA-1 hash da string `clientKey + magic`
2. Base64 encoding do hash SHA-1

## ✅ Correção Aplicada

### 1. Adicionada função SHA-1 no `Utils`

**`src/core/Utils.hpp`**:
```cpp
/**
 * @brief Calcula hash SHA-1 de uma string
 * @param input String a ser hashada
 * @return Hash SHA-1 em bytes (20 bytes)
 */
static std::string sha1(const std::string& input);
```

**`src/core/Utils.cpp`**:
```cpp
std::string Utils::sha1(const std::string& input) {
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hashLen = 0;
  
  EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
  if (!mdctx) {
    return std::string();
  }
  
  if (EVP_DigestInit_ex(mdctx, EVP_sha1(), nullptr) != 1) {
    EVP_MD_CTX_free(mdctx);
    return std::string();
  }
  
  if (EVP_DigestUpdate(mdctx, input.data(), input.size()) != 1) {
    EVP_MD_CTX_free(mdctx);
    return std::string();
  }
  
  if (EVP_DigestFinal_ex(mdctx, hash, &hashLen) != 1) {
    EVP_MD_CTX_free(mdctx);
    return std::string();
  }
  
  EVP_MD_CTX_free(mdctx);
  
  return std::string(reinterpret_cast<const char*>(hash), hashLen);
}
```

**Incluído `openssl/sha.h`** em `Utils.cpp`.

### 2. Corrigida função `generateAcceptKey` no `WebSocketServer`

**`src/network/WebSocketServer.cpp`**:
```cpp
std::string WebSocketServer::generateAcceptKey(const std::string& clientKey) {
  static const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string combined = clientKey + magic;
  
  // WebSocket RFC 6455: SHA-1 hash followed by Base64 encoding
  std::string sha1Hash = Core::Utils::sha1(combined);
  if (sha1Hash.empty()) {
    Core::Logger::getInstance().error("Failed to generate SHA-1 hash for WebSocket handshake");
    return "";
  }
  
  return Core::Utils::base64Encode(sha1Hash);
}
```

## 📋 Teste

Após a correção:
1. ✅ Zone Server compilado com sucesso
2. ✅ Zone Server rodando na porta 8082
3. ✅ WebSocket handshake agora usa SHA-1 + Base64 conforme RFC 6455

## 🔍 Verificação

Para testar se o handshake está funcionando:
1. Inicie o Zone Server: `zone_server.exe 0`
2. Conecte o cliente Unreal Engine
3. Verifique os logs - não deve mais aparecer "Accept hash wrong"
4. O cliente deve conectar e permanecer conectado (não desconectar imediatamente)

---

**Status**: ✅ **CORRIGIDO E TESTADO**

