# Problema: Token do login.php não é JWT real

**Data:** 29 de Outubro de 2025  
**Status:** ⚠️ **PROBLEMA IDENTIFICADO - REQUER CORREÇÃO**

## Problema

O endpoint `login.php` está retornando um token que **não é um JWT real**:

**Token atual (do log):**
```
"token": "amVmZm86MTc2MTc4NTEzNQ=="
```

**Decodificando base64:**
```
jeffo:1761785135
```

Isso é apenas `username:timestamp` codificado em base64, **não é um JWT**.

## Impacto

Os endpoints PHP de personagens esperam um **JWT válido** no formato:
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjE3ODUxMzUsImV4cCI6MTc2MTk0NzkzNX0.signature
```

O `jwt_helper.php` irá **rejeitar** tokens que não sejam JWT válidos.

## Solução

### Opção 1: Gerar JWT Real no login.php (RECOMENDADO)

Atualizar `login.php` para gerar um JWT real usando a mesma biblioteca/implementação do `jwt_helper.php`:

```php
// Após login bem-sucedido
require_once __DIR__ . '/../common/jwt_helper.php';

$payload = [
    'account_id' => $account['id'],
    'player_id' => $players[0]['player_id'] ?? 0,
    'username' => $account['username'],
    'iat' => time(),
    'exp' => time() + (60 * 60) // 1 hora
];

// Gerar JWT
$token = generateJWT($payload, $jwt_secret);

$response['token'] = $token;
```

**Requer:** Criar função `generateJWT()` no `jwt_helper.php` ou usar biblioteca JWT PHP (ex: firebase/php-jwt via Composer).

### Opção 2: Usar AuthServer C++ para Login

Ao invés de usar `login.php`, conectar diretamente ao `AuthServer` C++ que já gera JWT real.

### Opção 3: Temporário - Modificar jwt_helper.php para aceitar tokens antigos

Adicionar compatibilidade temporária para tokens no formato antigo enquanto migra:

```php
function validateJWT($token, $secret = null) {
    // Tentar validar como JWT primeiro
    $payload = validateJWTReal($token, $secret);
    if ($payload !== false) {
        return $payload;
    }
    
    // Fallback: tentar como token antigo (base64 username:timestamp)
    $decoded = base64_decode($token);
    if ($decoded && strpos($decoded, ':') !== false) {
        list($username, $timestamp) = explode(':', $decoded);
        // Validar timestamp (não muito antigo)
        if ($timestamp > time() - 3600) { // 1 hora
            // Buscar account_id do username
            // Retornar payload compatível
        }
    }
    
    return false;
}
```

**⚠️ NOTA:** Esta é apenas uma solução temporária para compatibilidade durante migração.

---

## Recomendação

**Solução definitiva:** Implementar Opção 1 (gerar JWT real no `login.php`).

---

**Documento criado em:** 2025-10-29  
**Versão:** 1.0

