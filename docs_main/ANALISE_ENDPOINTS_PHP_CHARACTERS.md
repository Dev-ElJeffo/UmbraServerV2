# Análise Completa: Endpoints PHP de Personagens vs Sistema C++

**Data:** 29 de Outubro de 2025  
**Status:** ⚠️ **REQUER CORREÇÕES CRÍTICAS**

## Resumo Executivo

Os endpoints PHP de personagens (`list_characters.php`, `create_character.php`, `select_character.php`, `delete_character.php`) **EXISTEM e estão funcionalmente completos**, porém apresentam **problemas críticos de segurança e integração** com o sistema C++.

---

## 1. Análise de Segurança (CRÍTICO)

### ❌ **PROBLEMA CRÍTICO: Falta de Autenticação JWT**

Todos os endpoints PHP **NÃO validam tokens JWT** antes de processar requisições. Eles apenas confiam no `account_id` fornecido pelo cliente, o que permite:

- **Acesso não autorizado** a dados de outras contas
- **Modificação/Exclusão** de personagens de outras contas
- **Bypass completo** do sistema de autenticação

**Exemplo do problema:**
```php
// list_characters.php - Linha 53
$account_id = intval($data['account_id']); // ❌ Aceita qualquer account_id sem validação!
```

**Solução necessária:**
```php
// Deve validar JWT token antes de processar
$token = $data['token'] ?? $_SERVER['HTTP_AUTHORIZATION'] ?? '';
if (!validateJWT($token)) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

$payload = decodeJWT($token);
$account_id = $payload['account_id']; // ✅ Usar account_id do token, não do cliente
```

### ✅ **Validações Existentes (Parcialmente Adequadas)**

1. **Verificação de propriedade** (`delete_character.php`, `select_character.php`):
   ```php
   // Linha 64 - delete_character.php
   $stmt = $pdo->prepare("SELECT id FROM players WHERE id = ? AND account_id = ?");
   ```
   ✅ **Correto**: Verifica que o personagem pertence à conta antes de deletar/selecionar.

2. **Validação de entrada** (`create_character.php`):
   ```php
   // Linhas 66-79
   - Valida comprimento do nome (3-20 caracteres)
   - Valida formato do nome (apenas alfanuméricos e underscore)
   - Verifica se conta existe
   - Verifica se nome já existe
   - Verifica limite de 5 personagens
   ```
   ✅ **Correto**: Validações adequadas de entrada.

---

## 2. Compatibilidade de Campos com PlayerDAO C++

### ✅ **Campos do Banco - COMPATÍVEIS**

Comparando os campos usados nos endpoints PHP com o `PlayerDAO::parsePlayerFromQuery()`:

**PlayerDAO C++ (21 campos):**
```cpp
SELECT id, account_id, character_name, level, experience,
       pos_x, pos_y, pos_z, current_zone,
       health, max_health, mana, max_mana, stamina, max_stamina,
       strength, dexterity, intelligence, vitality,
       created_at, last_played_at
```

**Endpoints PHP:**
- ✅ `list_characters.php`: Usa todos os 21 campos corretamente
- ✅ `create_character.php`: Insere todos os campos necessários (exceto `last_played_at` que é NULL inicialmente)
- ✅ `select_character.php`: Retorna todos os 21 campos
- ✅ `delete_character.php`: Usa `id` e `account_id` corretamente

**Conclusão:** Os campos são **100% compatíveis** com o `PlayerDAO` C++.

### ✅ **Formato JSON de Retorno - COMPATÍVEL**

O formato JSON retornado pelos endpoints PHP é compatível com o que o cliente UE5 espera:

```json
{
  "success": true,
  "player": {
    "player_id": 1,
    "account_id": 4,
    "character_name": "ElJeffo",
    "level": 1,
    "experience": 0,
    "current_zone": "Tutorial",
    "position": {
      "x": 0.0,
      "y": 0.0,
      "z": 0.0
    },
    "stats": {
      "health": 100,
      "max_health": 100,
      "mana": 50,
      "max_mana": 50,
      "stamina": 100,
      "max_stamina": 100,
      "strength": 10,
      "dexterity": 10,
      "intelligence": 10,
      "vitality": 10
    },
    "created_at": "2025-10-29 20:00:00",
    "last_login": "2025-10-29 21:00:00"
  }
}
```

✅ **Compatível** com `Player` struct em C++ e formato esperado pelo UE5.

---

## 3. Análise Detalhada por Endpoint

### 3.1. `list_characters.php`

**Funcionalidade:** Lista todos os personagens de uma conta

**Status:** ⚠️ **Funcional mas inseguro**

**Pontos Positivos:**
- ✅ Query correta: todos os 21 campos
- ✅ Ordenação correta: `ORDER BY last_played_at DESC, created_at DESC`
- ✅ Formatação JSON adequada
- ✅ Verifica se conta existe antes de listar

**Pontos Negativos:**
- ❌ **CRÍTICO**: Não valida JWT token
- ❌ Aceita `account_id` do cliente sem verificação
- ❌ Permite acesso a personagens de outras contas

**Compatibilidade com C++:**
```cpp
// PlayerDAO C++ - equivalente
auto players = playerDAO_->getPlayersByAccountId(accountId);
// Retorna std::vector<Player> com mesmos dados
```
✅ **Campos e estrutura 100% compatíveis**

---

### 3.2. `create_character.php`

**Funcionalidade:** Cria novo personagem

**Status:** ⚠️ **Funcional mas inseguro**

**Pontos Positivos:**
- ✅ Validações completas (nome, comprimento, formato, duplicação)
- ✅ Verifica limite de 5 personagens
- ✅ Insere todos os campos necessários com valores padrão corretos:
  ```php
  level = 1
  experience = 0
  pos_x, pos_y, pos_z = 0.0
  current_zone = 'Tutorial'
  health, max_health = 100
  mana, max_mana = 50
  stamina, max_stamina = 100
  strength, dexterity, intelligence, vitality = 10
  ```
- ✅ Retorna dados completos do personagem criado

**Pontos Negativos:**
- ❌ **CRÍTICO**: Não valida JWT token
- ❌ Aceita `account_id` do cliente
- ❌ Permite criar personagens em contas de outros usuários

**Compatibilidade com C++:**
```cpp
// PlayerDAO C++ - equivalente
Database::Player player;
player.accountId = accountId;
player.characterName = characterName;
// ... outros campos com valores padrão
uint64_t playerId = playerDAO_->createPlayer(player);
```
✅ **Comportamento idêntico ao C++**

---

### 3.3. `select_character.php`

**Funcionalidade:** Seleciona um personagem e atualiza `last_played_at`

**Status:** ⚠️ **Funcional mas inseguro**

**Pontos Positivos:**
- ✅ Verifica ownership: `WHERE id = ? AND account_id = ?`
- ✅ Atualiza `last_played_at = NOW()` corretamente
- ✅ Retorna dados completos do personagem

**Pontos Negativos:**
- ❌ **CRÍTICO**: Não valida JWT token
- ❌ Aceita `account_id` e `player_id` do cliente
- ⚠️ Embora verifique ownership, ainda permite tentativas de acesso não autorizadas

**Compatibilidade com C++:**
```cpp
// PlayerDAO C++ - equivalente
auto player = playerDAO_->getPlayerById(playerId);
if (player && player->accountId == accountId) {
    playerDAO_->updateLastPlayed(playerId);
}
```
✅ **Lógica compatível, mas falta validação de token**

---

### 3.4. `delete_character.php`

**Funcionalidade:** Deleta um personagem

**Status:** ⚠️ **Funcional mas inseguro**

**Pontos Positivos:**
- ✅ Verifica ownership antes de deletar
- ✅ Feedback adequado com nome do personagem deletado

**Pontos Negativos:**
- ❌ **CRÍTICO**: Não valida JWT token
- ❌ Aceita `account_id` e `player_id` do cliente
- ⚠️ Embora verifique ownership, ainda permite tentativas maliciosas

**Compatibilidade com C++:**
```cpp
// PlayerDAO C++ - equivalente
bool deleted = playerDAO_->deletePlayer(playerId);
// Note: C++ também deveria verificar ownership
```
✅ **Lógica compatível**

---

## 4. Integração com AuthServer C++

### ❌ **PROBLEMA: PHP não usa AuthServer C++**

Os endpoints PHP **acessam o banco diretamente** via PDO, **ignorando completamente** o `AuthServer` C++ e o sistema de autenticação JWT.

**Arquitetura atual (insegura):**
```
Cliente UE5 → PHP API → MySQL (direto)
```

**Arquitetura recomendada:**
```
Cliente UE5 → PHP API → AuthServer C++ (via TCP/HTTP) → MySQL
                    ↓
                Valida JWT Token
```

### Opções de Integração

**Opção 1: PHP valida JWT gerado pelo AuthServer C++**
- PHP precisa implementar validação JWT com a mesma chave secreta do C++
- PHP valida token antes de processar requisições
- ✅ Mantém endpoints PHP funcionais
- ✅ Adiciona segurança necessária

**Opção 2: PHP chama AuthServer C++ via TCP/HTTP**
- PHP faz requisição ao AuthServer C++ para validar token
- PHP faz requisição ao AuthServer C++ para operações de personagem
- ✅ Centraliza lógica de autenticação
- ❌ Requer comunicação entre PHP e C++
- ❌ Mais complexo

**Opção 3: Migrar endpoints para C++ (Gateway/Auth Server)**
- Endpoints C++ no Gateway/Auth Server
- PHP apenas para frontend/dashboard
- ✅ Segurança nativa
- ✅ Performance melhor
- ❌ Requer reimplementação

---

## 5. Comparação de Parsing

### PlayerDAO C++ vs Endpoints PHP

**Campos parseados (ambos 21 campos):**

| Campo | PlayerDAO C++ | PHP Endpoints | Compatível? |
|-------|--------------|---------------|-------------|
| id | ✅ `row[0]` | ✅ `id` | ✅ |
| account_id | ✅ `row[1]` | ✅ `account_id` | ✅ |
| character_name | ✅ `row[2]` | ✅ `character_name` | ✅ |
| level | ✅ `row[3]` | ✅ `level` | ✅ |
| experience | ✅ `row[4]` | ✅ `experience` | ✅ |
| pos_x | ✅ `row[5]` | ✅ `pos_x` | ✅ |
| pos_y | ✅ `row[6]` | ✅ `pos_y` | ✅ |
| pos_z | ✅ `row[7]` | ✅ `pos_z` | ✅ |
| current_zone | ✅ `row[8]` | ✅ `current_zone` | ✅ |
| health | ✅ `row[9]` | ✅ `health` | ✅ |
| max_health | ✅ `row[10]` | ✅ `max_health` | ✅ |
| mana | ✅ `row[11]` | ✅ `mana` | ✅ |
| max_mana | ✅ `row[12]` | ✅ `max_mana` | ✅ |
| stamina | ✅ `row[13]` | ✅ `stamina` | ✅ |
| max_stamina | ✅ `row[14]` | ✅ `max_stamina` | ✅ |
| strength | ✅ `row[15]` | ✅ `strength` | ✅ |
| dexterity | ✅ `row[16]` | ✅ `dexterity` | ✅ |
| intelligence | ✅ `row[17]` | ✅ `intelligence` | ✅ |
| vitality | ✅ `row[18]` | ✅ `vitality` | ✅ |
| created_at | ✅ `row[19]` | ✅ `created_at` | ✅ |
| last_played_at | ✅ `row[20]` (pode ser NULL) | ✅ `last_played_at` | ✅ |

**Conclusão:** ✅ **100% compatível** - Mesmos campos, mesma ordem, mesmo formato.

---

## 6. Problemas Identificados

### 🔴 **CRÍTICOS (Segurança)**

1. **Falta de autenticação JWT** em todos os endpoints
2. **Aceitação de `account_id` do cliente** sem validação
3. **Possibilidade de acesso não autorizado** a dados de outras contas
4. **Bypass do sistema de autenticação C++**

### 🟡 **MÉDIOS (Funcionalidade)**

1. **Inconsistência entre sistemas PHP e C++**
   - PHP acessa banco diretamente
   - C++ usa PlayerDAO e AuthServer
   - Não há sincronização entre sistemas

2. **Duplicação de lógica**
   - Lógica de validação duplicada entre PHP e C++
   - Risco de divergências futuras

### 🟢 **MENORES (Melhorias)**

1. **Logging limitado** nos endpoints PHP
2. **Tratamento de erros** poderia ser mais detalhado
3. **Rate limiting** não implementado

---

## 7. Recomendações

### **PRIORIDADE ALTA (Segurança)**

1. **Implementar validação JWT em todos os endpoints PHP**
   ```php
   // Criar função helper
   function validateJWTRequest($data) {
       $token = $data['token'] ?? $_SERVER['HTTP_AUTHORIZATION'] ?? '';
       if (empty($token)) {
           return ['valid' => false, 'error' => 'Token não fornecido'];
       }
       
       // Validar com mesma chave secreta do AuthServer C++
       $jwt_secret = getenv('JWT_SECRET') ?? 'UmbraEternum2025SecretKey123456789';
       $payload = JWT::decode($token, $jwt_secret, ['HS256']);
       
       return ['valid' => true, 'payload' => $payload];
   }
   ```

2. **Usar `account_id` do token JWT, não do cliente**
   ```php
   $validation = validateJWTRequest($data);
   if (!$validation['valid']) {
       http_response_code(401);
       echo json_encode(['success' => false, 'message' => $validation['error']]);
       exit;
   }
   
   $account_id = $validation['payload']['account_id']; // ✅ Do token, não do cliente
   ```

3. **Adicionar verificação de ownership em todos os endpoints**
   - Já existe em `select_character.php` e `delete_character.php`
   - Adicionar em `list_characters.php` (usando token JWT)

### **PRIORIDADE MÉDIA (Integração)**

4. **Criar biblioteca PHP compartilhada para validação JWT**
   - Reutilizar em todos os endpoints
   - Garantir consistência

5. **Documentar processo de autenticação**
   - Como gerar tokens (via AuthServer C++ ou PHP)
   - Como validar tokens nos endpoints PHP
   - Fluxo completo de autenticação

### **PRIORIDADE BAIXA (Melhorias)**

6. **Adicionar logging detalhado**
7. **Implementar rate limiting**
8. **Adicionar métricas/monitoramento**

---

## 8. Conclusão

### ✅ **Pontos Positivos**

- ✅ Endpoints **funcionalmente completos**
- ✅ **100% compatíveis** com PlayerDAO C++ (mesmos campos)
- ✅ Validações de entrada adequadas
- ✅ Formato JSON compatível com UE5
- ✅ Queries SQL corretas e eficientes

### ❌ **Pontos Críticos**

- ❌ **FALTA AUTENTICAÇÃO JWT** (vulnerabilidade crítica)
- ❌ Aceita dados do cliente sem validação
- ❌ Não integrado com AuthServer C++
- ❌ Permite acesso não autorizado

### 📋 **Status Final**

| Aspecto | Status | Nota |
|--------|--------|------|
| Funcionalidade | ✅ Completo | 10/10 |
| Compatibilidade com C++ | ✅ Total | 10/10 |
| Segurança | ❌ Crítico | 0/10 |
| Integração | ❌ Inexistente | 0/10 |
| **TOTAL** | ⚠️ **INCOMPLETO** | **5/10** |

### 🎯 **Próximos Passos Recomendados**

1. **URGENTE**: Implementar validação JWT em todos os endpoints PHP
2. **URGENTE**: Usar `account_id` do token, não do cliente
3. **MÉDIO**: Criar biblioteca PHP compartilhada para JWT
4. **MÉDIO**: Adicionar testes de segurança
5. **BAIXO**: Melhorar logging e tratamento de erros

---

## 9. Arquivos para Correção

- `c:\wamp64\www\umbra_api\api\character\list_characters.php`
- `c:\wamp64\www\umbra_api\api\character\create_character.php`
- `c:\wamp64\www\umbra_api\api\character\select_character.php`
- `c:\wamp64\www\umbra_api\api\character\delete_character.php`

**Criar novo arquivo:**
- `c:\wamp64\www\umbra_api\api\common\jwt_helper.php` (biblioteca compartilhada)

---

**Documento criado em:** 2025-10-29  
**Última atualização:** 2025-10-29  
**Versão:** 1.0

