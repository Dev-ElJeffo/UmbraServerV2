# Correções de Segurança - Endpoints PHP de Personagens

**Data:** 29 de Outubro de 2025  
**Status:** ✅ **IMPLEMENTADO**

## Resumo

Implementadas correções críticas de segurança em todos os endpoints PHP de personagens:

1. ✅ Validação JWT obrigatória em todos os endpoints
2. ✅ Uso de `account_id` do token JWT, não do cliente
3. ✅ Biblioteca JWT helper compartilhada criada

---

## Arquivos Criados

### `api/common/jwt_helper.php`
- Biblioteca compartilhada para validação JWT
- Compatível com `JWTManager` C++ (HS256, Base64URL)
- Funções:
  - `validateJWT($token, $secret)` - Valida token e retorna payload
  - `validateJWTRequest($data, $headers)` - Wrapper completo para requisições HTTP
  - `base64UrlEncode($input)` - Codificação Base64URL
  - `base64UrlDecode($input)` - Decodificação Base64URL

---

## Arquivos Modificados

### 1. `api/character/list_characters.php`

**Mudanças:**
- ✅ Adicionada validação JWT obrigatória
- ✅ `account_id` agora vem do token JWT
- ✅ Verificação adicional: se cliente fornece `account_id`, deve corresponder ao token
- ✅ Headers HTTP atualizados: `Authorization` header suportado

**Antes (INSEGURO):**
```php
$account_id = intval($data['account_id']); // ❌ Do cliente, sem validação
```

**Depois (SEGURO):**
```php
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error']]);
    exit;
}
$account_id = intval($validation['payload']['account_id']); // ✅ Do token
```

---

### 2. `api/character/create_character.php`

**Mudanças:**
- ✅ Adicionada validação JWT obrigatória
- ✅ `account_id` agora vem do token JWT
- ✅ Removida necessidade de enviar `account_id` no body (agora vem do token)
- ✅ Headers HTTP atualizados

**Antes (INSEGURO):**
```php
if (empty($data['account_id']) || empty($data['character_name'])) {
    // ❌ Requeria account_id do cliente
}
$account_id = intval($data['account_id']); // ❌ Do cliente
```

**Depois (SEGURO):**
```php
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    exit;
}
$account_id = intval($validation['payload']['account_id']); // ✅ Do token
// Agora só precisa de character_name
```

---

### 3. `api/character/select_character.php`

**Mudanças:**
- ✅ Adicionada validação JWT obrigatória
- ✅ `account_id` agora vem do token JWT
- ✅ Removida necessidade de enviar `account_id` no body
- ✅ Headers HTTP atualizados

**Antes (INSEGURO):**
```php
if (empty($data['player_id']) || empty($data['account_id'])) {
    // ❌ Requeria account_id do cliente
}
$account_id = intval($data['account_id']); // ❌ Do cliente
```

**Depois (SEGURO):**
```php
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    exit;
}
$account_id = intval($validation['payload']['account_id']); // ✅ Do token
// Agora só precisa de player_id
```

---

### 4. `api/character/delete_character.php`

**Mudanças:**
- ✅ Adicionada validação JWT obrigatória
- ✅ `account_id` agora vem do token JWT
- ✅ Removida necessidade de enviar `account_id` no body
- ✅ Headers HTTP atualizados

**Antes (INSEGURO):**
```php
if (empty($data['player_id']) || empty($data['account_id'])) {
    // ❌ Requeria account_id do cliente
}
$account_id = intval($data['account_id']); // ❌ Do cliente
```

**Depois (SEGURO):**
```php
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    exit;
}
$account_id = intval($validation['payload']['account_id']); // ✅ Do token
// Agora só precisa de player_id
```

---

## Segurança Implementada

### ✅ Proteções Adicionadas

1. **Autenticação obrigatória**
   - Todos os endpoints agora exigem token JWT válido
   - Tokens expirados são rejeitados
   - Tokens com assinatura inválida são rejeitados

2. **Autorização baseada em token**
   - `account_id` vem exclusivamente do token JWT
   - Cliente não pode modificar `account_id`
   - Previne acesso não autorizado

3. **Validação de formato**
   - Tokens com formato inválido são rejeitados
   - Base64URL decodificação robusta
   - Verificação de expiração (timestamp)

4. **Headers HTTP suportados**
   - Suporte a `Authorization: Bearer <token>` header
   - Suporte a token no JSON body (`token` field)
   - Fallback para múltiplas fontes de token

---

## Compatibilidade com JWTManager C++

### ✅ Algoritmo de Assinatura
- **C++:** HMAC-SHA256 (`HMAC(EVP_sha256(), ...)`)
- **PHP:** `hash_hmac('sha256', ...)`
- **Status:** ✅ **COMPATÍVEL**

### ✅ Formato de Token
- **C++:** `base64UrlEncode(header) + "." + base64UrlEncode(payload) + "." + base64UrlEncode(signature)`
- **PHP:** Mesmo formato
- **Status:** ✅ **COMPATÍVEL**

### ✅ Payload Estruturado
- **C++:** `{account_id, player_id, username, iat, exp}`
- **PHP:** Espera mesmo formato
- **Status:** ✅ **COMPATÍVEL**

### ✅ Chave Secreta
- **C++:** `config.jwtSecret` (padrão: "UmbraEternum2025SecretKey123456789")
- **PHP:** `getenv('JWT_SECRET') ?: 'UmbraEternum2025SecretKey123456789'`
- **Status:** ✅ **COMPATÍVEL** (usar mesma chave)

---

## Mudanças na API

### Requisições Antigas (DEPRECATED)

```json
POST /api/character/list_characters.php
{
  "account_id": 4
}
```

### Requisições Novas (OBRIGATÓRIO)

**Opção 1: Token no JSON body**
```json
POST /api/character/list_characters.php
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

**Opção 2: Token no Header (Recomendado)**
```http
POST /api/character/list_characters.php
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
Content-Type: application/json
```

### Respostas de Erro

**Token não fornecido:**
```json
{
  "success": false,
  "message": "Token não fornecido"
}
```
HTTP Status: `401 Unauthorized`

**Token inválido/expirado:**
```json
{
  "success": false,
  "message": "Token inválido ou expirado"
}
```
HTTP Status: `401 Unauthorized`

**account_id não corresponde:**
```json
{
  "success": false,
  "message": "account_id fornecido não corresponde ao token"
}
```
HTTP Status: `403 Forbidden`

---

## Testes Necessários

### ✅ Testes de Segurança

1. **Sem token:**
   - Requisição sem token → Deve retornar 401

2. **Token inválido:**
   - Token malformado → Deve retornar 401
   - Token com assinatura inválida → Deve retornar 401

3. **Token expirado:**
   - Token com `exp < time()` → Deve retornar 401

4. **Token válido:**
   - Token válido → Deve processar requisição normalmente
   - `account_id` do token usado, não do cliente

5. **Tentativa de acesso não autorizado:**
   - Token de account_id=1 usado para acessar account_id=4 → Deve retornar 403

### ⏭️ Testes Funcionais

1. **Listar personagens** com token válido
2. **Criar personagem** com token válido
3. **Selecionar personagem** com token válido
4. **Deletar personagem** com token válido (apenas próprios personagens)

---

## Próximos Passos

1. ✅ Correções de segurança implementadas
2. ⏭️ **URGENTE**: Testar todos os endpoints com tokens válidos
3. ⏭️ Atualizar clientes (UE5) para enviar tokens JWT
4. ⏭️ Documentar fluxo completo de autenticação
5. ⏭️ Considerar implementar refresh tokens
6. ⏭️ Adicionar rate limiting por token

---

## Notas Técnicas

### Compatibilidade Base64URL

O `jwt_helper.php` implementa Base64URL compatível com o C++:
- Remove padding (`=`)
- Substitui `+` por `-`
- Substitui `/` por `_`
- Decodifica corretamente adicionando padding de volta

### Verificação de Assinatura

Usa `hash_equals()` para comparação segura de strings (timing-safe), prevenindo timing attacks.

### Chave Secreta

A chave secreta é lida de:
1. Variável de ambiente `JWT_SECRET`
2. Fallback para valor padrão: `'UmbraEternum2025SecretKey123456789'`

**Recomendação:** Configurar `JWT_SECRET` em produção via variável de ambiente ou arquivo de configuração seguro.

---

**Documento criado em:** 2025-10-29  
**Última atualização:** 2025-10-29  
**Versão:** 1.0

