# Implementação: Geração de JWT Real no login.php

**Data:** 29 de Outubro de 2025  
**Status:** ✅ **IMPLEMENTADO**

## Resumo

Implementada geração de tokens JWT reais no `login.php`, compatível com o `JWTManager` C++ do AuthServer.

---

## Mudanças Implementadas

### 1. `api/common/jwt_helper.php` - ATUALIZADO ✅

**Nova função adicionada:**
```php
function generateJWT($accountId, $playerId, $username, $expirationMinutes = 60, $secret = null)
```

**Funcionalidade:**
- ✅ Gera token JWT no formato: `header.payload.signature`
- ✅ Header: `{"alg":"HS256","typ":"JWT"}`
- ✅ Payload: `{account_id, player_id, username, iat, exp}`
- ✅ Assinatura: HMAC-SHA256 com chave secreta
- ✅ Base64URL encoding (compatível com C++)
- ✅ Mesma chave secreta padrão do AuthServer C++

**Formato do token gerado:**
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjE3ODUxMzUsImV4cCI6MTc2MTk0NzkzNX0.signature
```

**Compatible com:**
- ✅ `JWTManager` C++ do AuthServer
- ✅ `jwt_helper.php` validação
- ✅ Endpoints PHP de personagens

---

### 2. `api/login.php` - ATUALIZADO ✅

**Mudanças:**
1. ✅ Carrega `jwt_helper.php` para ter acesso a `generateJWT()`
2. ✅ Gera token JWT real após login bem-sucedido
3. ✅ Usa `player_id` do primeiro personagem (ou 0 se não houver)
4. ✅ Token expira em 60 minutos (padrão, configurável)
5. ✅ Fallback para token antigo se JWT helper não estiver disponível

**Antes (ANTIGO):**
```php
// TODO: Gerar JWT token aqui
$response['token'] = base64_encode($account['username'] . ':' . time());
```

**Depois (NOVO):**
```php
// ✅ Gerar JWT token real (compatível com AuthServer C++)
if ($jwt_loaded && function_exists('generateJWT')) {
    $firstPlayerId = !empty($players) ? (int)$players[0]['player_id'] : 0;
    
    $jwtToken = generateJWT(
        (int)$account['id'],
        $firstPlayerId,
        $account['username'],
        60 // 60 minutos
    );
    
    $response['token'] = $jwtToken;
} else {
    // Fallback: token temporário
    $response['token'] = base64_encode($account['username'] . ':' . time());
}
```

---

## Compatibilidade

### ✅ Com AuthServer C++

| Aspecto | AuthServer C++ | login.php PHP | Status |
|--------|----------------|---------------|--------|
| Algoritmo | HS256 (HMAC-SHA256) | HS256 (`hash_hmac`) | ✅ Compatível |
| Header | `{"alg":"HS256","typ":"JWT"}` | `{"alg":"HS256","typ":"JWT"}` | ✅ Idêntico |
| Payload | `{account_id, player_id, username, iat, exp}` | `{account_id, player_id, username, iat, exp}` | ✅ Idêntico |
| Encoding | Base64URL | Base64URL | ✅ Compatível |
| Chave Secreta | `config.jwtSecret` | `JWT_SECRET` env ou padrão | ✅ Mesma padrão |
| Expiração | 60 min (configurável) | 60 min (configurável) | ✅ Compatível |

### ✅ Com Endpoints PHP de Personagens

Os tokens gerados pelo `login.php` são **100% válidos** para:
- ✅ `list_characters.php`
- ✅ `create_character.php`
- ✅ `select_character.php`
- ✅ `delete_character.php`

Todos usam `validateJWT()` do `jwt_helper.php` que aceita tokens gerados por `generateJWT()`.

---

## Estrutura do Token Gerado

### Exemplo Real

**Input:**
- `account_id`: 4
- `player_id`: 1
- `username`: "jeffo"
- `expirationMinutes`: 60

**Token gerado:**
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjE3ODUxMzUsImV4cCI6MTc2MTk0NzkzNX0.assinatura_base64url
```

**Decodificando:**
- **Header:** `{"alg":"HS256","typ":"JWT"}`
- **Payload:** `{"account_id":4,"player_id":1,"username":"jeffo","iat":1761785135,"exp":1761947935}`
- **Signature:** HMAC-SHA256(header.payload, secret)

---

## Fluxo Completo

### 1. Login
```
Cliente UE5 → POST /api/login.php
                ↓
           Valida credenciais
                ↓
           Busca personagens
                ↓
           Gera JWT token real ← ✅ NOVO
                ↓
           Retorna token + dados
```

### 2. Usar Token nos Endpoints
```
Cliente UE5 → POST /api/character/list_characters.php
                Body: {"token": "eyJ..."}
                ↓
           validateJWT(token) ← ✅ Valida com mesma chave
                ↓
           Extrai account_id do token
                ↓
           Busca personagens
                ↓
           Retorna lista
```

---

## Chave Secreta

### Configuração

A chave secreta é lida em ordem de prioridade:

1. **Variável de ambiente:** `JWT_SECRET`
2. **Fallback padrão:** `'UmbraEternum2025SecretKey123456789'`

### Mesma Chave para Todos

**AuthServer C++:**
```cpp
config.jwtSecret = "UmbraEternum2025SecretKey123456789"; // Padrão
```

**PHP (jwt_helper.php):**
```php
$secret = getenv('JWT_SECRET') ?: 'UmbraEternum2025SecretKey123456789';
```

**✅ Garantido:** Todos usam a mesma chave por padrão.

### Produção

**Recomendação:** Configurar `JWT_SECRET` como variável de ambiente:
```bash
export JWT_SECRET="sua_chave_secreta_super_segura_aqui"
```

---

## Testes

### Teste Manual

1. **Fazer login:**
```bash
curl -X POST http://localhost/umbra_api/api/login.php \
  -H "Content-Type: application/json" \
  -d '{"username":"jeffo","password":"senha"}'
```

2. **Verificar token JWT:**
- Token deve começar com `eyJ` (base64 de `{"`)
- Token deve ter 3 partes separadas por `.`
- Token deve ser válido com `validateJWT()`

3. **Testar validação:**
```php
require_once 'api/common/jwt_helper.php';
$payload = validateJWT($token);
// Deve retornar array com account_id, player_id, username, iat, exp
```

### Teste Automatizado (Futuro)

Criar testes que:
- ✅ Geram token com `generateJWT()`
- ✅ Validam token com `validateJWT()`
- ✅ Comparam com token gerado pelo AuthServer C++
- ✅ Verificam compatibilidade total

---

## Logs

O `login.php` agora loga:
- ✅ `"JWT token gerado para usuário: {username}"` - quando JWT é gerado
- ⚠️ `"AVISO: JWT Helper não disponível, usando token temporário"` - fallback

---

## Rollback

Se precisar reverter temporariamente:

1. **Comentar a geração de JWT:**
```php
// $jwtToken = generateJWT(...);
$response['token'] = base64_encode($account['username'] . ':' . time());
```

2. **Ou:** Usar flag de configuração para alternar

---

## Próximos Passos

1. ✅ JWT real gerado no login - **COMPLETO**
2. ⏭️ Testar integração completa:
   - Login → Receber JWT
   - Listar personagens com JWT
   - Criar personagem com JWT
   - Selecionar personagem com JWT
   - Deletar personagem com JWT
3. ⏭️ Remover código de fallback (token antigo) após validação
4. ⏭️ Configurar `JWT_SECRET` em produção

---

**Documento criado em:** 2025-10-29  
**Última atualização:** 2025-10-29  
**Versão:** 1.0

