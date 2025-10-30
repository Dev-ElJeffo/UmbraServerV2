# Documentação Completa: Autenticação JWT - Sistema Completo

**Data:** 29 de Outubro de 2025  
**Status:** ✅ **IMPLEMENTAÇÃO COMPLETA - REQUER COMPILAÇÃO UE5**

## Resumo Executivo

Implementação completa do sistema de autenticação JWT end-to-end:

1. ✅ **Backend C++ (AuthServer)** - Gera e valida JWT
2. ✅ **Backend PHP (login.php)** - Gera JWT real compatível
3. ✅ **Backend PHP (endpoints)** - Valida JWT em todos os endpoints
4. ✅ **Client Unreal** - Envia token JWT no body JSON
5. ⏭️ **REQUER:** Recompilação do projeto UE5 para ativar mudanças

---

## Fluxo Completo de Autenticação

### 1. Login (Login bem-sucedido ✅)

```
Cliente UE5
    ↓ POST /api/login.php
    ↓ {username, password}
PHP login.php
    ↓ Valida credenciais
    ↓ Busca personagens
    ↓ generateJWT() ← ✅ GERA JWT REAL
    ↓ Retorna token JWT
Cliente UE5
    ↓ Salva CurrentToken
    ✅ Token JWT válido: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

**Status:** ✅ **FUNCIONANDO** (token JWT real sendo gerado)

---

### 2. Listar Personagens (Problema: Token não enviado ⚠️)

```
Cliente UE5
    ↓ LoadCharacterList()
    ↓ POST /api/character/list_characters.php
    ↓ Body: {token: "eyJ..."} ← ✅ CÓDIGO ATUALIZADO MAS NÃO COMPILADO
    ↓
PHP list_characters.php
    ↓ validateJWTRequest()
    ↓ Token não encontrado ← ❌ PROBLEMA
    ↓ Retorna 401
```

**Problema:** 
- Código C++ atualizado ✅
- Mas **NÃO FOI RECOMPILADO** ⚠️
- Token não está sendo enviado no body JSON

**Solução:** **RECOMPILAR PROJETO UE5**

---

## Status Atual

### ✅ Funcionando

1. **Geração de JWT no login.php**
   - Token JWT real gerado: `eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...`
   - Compatível com AuthServer C++
   - Payload correto: `{account_id, player_id, username, iat, exp}`

2. **Validação JWT nos endpoints PHP**
   - `validateJWTRequest()` implementado
   - Todos os 4 endpoints validam token
   - Mensagens de erro apropriadas

3. **Client Unreal - Código atualizado**
   - `LoadCharacterList()` - Adiciona token ao body
   - `SelectCharacter()` - Adiciona token ao body
   - `CreateCharacter()` - Adiciona token ao body
   - `DeleteCharacter()` - Adiciona token ao body

### ⚠️ Requer Ação

1. **Recompilar Projeto UE5**
   - As mudanças no C++ não estão ativas
   - O log não mostra `🔐 Token adicionado ao body`
   - O endpoint recebe "Token não fornecido"

---

## Como Recompilar UE5

### Método 1: Via Unreal Editor

1. Abrir projeto no Unreal Editor
2. Clicar em **Tools → Refresh Visual Studio Project Files**
3. Clicar em **Compile** (ou pressionar `Ctrl+Alt+F11`)
4. Aguardar compilação completa
5. Reiniciar PIE (Play In Editor)

### Método 2: Via Visual Studio

1. Abrir `UmbraEternumUE.sln` no Visual Studio
2. Build → Build Solution (ou `Ctrl+Shift+B`)
3. Aguardar compilação
4. Abrir projeto no Unreal Editor
5. Testar no PIE

### Método 3: Via Command Line

```powershell
# No diretório do projeto UE5
& "C:\Program Files\Epic Games\UE_5.x\Engine\Build\BatchFiles\Build.bat" UmbraEternumUEEditor Win64 Development "D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.uproject" -WaitMutex -FromMsBuild
```

---

## Logs Esperados Após Recompilação

### Login (Já Funcionando ✅)

```
LogTemp: [UmbraGameInstance] ✅ Login bem-sucedido!
LogTemp:    Token: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9... ← ✅ JWT Real
LogTemp: [UmbraGameInstance] 💾 Token salvo
```

### LoadCharacterList (Após Recompilação ✅)

```
LogTemp: [UmbraGameInstance] 🔍 DEBUG Antes de LoadCharacterList:
LogTemp:   - CurrentToken.IsEmpty(): NÃO ← ✅ Token existe
LogTemp:   - CurrentToken.Length(): 150+ ← ✅ Token tem tamanho
LogTemp:   - CurrentToken: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9... ← ✅ Token completo
LogTemp: [UmbraGameInstance] 🔍 DEBUG LoadCharacterList:
LogTemp:   - CurrentToken.IsEmpty(): NÃO ← ✅ Token ainda existe
LogTemp: [UmbraGameInstance] 🔐 Token adicionado ao body JSON: eyJ... ← ✅ NOVO LOG
LogVaRest: Response (200): {"success":true,"players":[...]} ← ✅ SUCESSO
```

---

## Verificação de Funcionamento

### Teste 1: Login Gera JWT Real

**Status:** ✅ **FUNCIONANDO**

Do log:
```json
"token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjE3ODc3ODUsImV4cCI6MTc2MTc5MTM4NX0.signature"
```

✅ **Confirmado:** Token JWT real de 3 partes, formato correto.

### Teste 2: Endpoint Valida JWT

**Aguardando:** Recompilação UE5

Após recompilação, o endpoint deve:
- Receber token no body JSON
- Validar com `validateJWT()`
- Retornar lista de personagens

### Teste 3: Decodificar Token (Validação Manual)

Você pode validar o token manualmente decodificando:

**Header (base64URL decode):**
```json
{"alg":"HS256","typ":"JWT"}
```

**Payload (base64URL decode):**
```json
{
  "account_id": 4,
  "player_id": 1,
  "username": "jeffo",
  "iat": 1761787785,  // Timestamp
  "exp": 1761791385   // 60 minutos depois
}
```

---

## Troubleshooting

### Problema: "Token não fornecido" mesmo após recompilação

**Verificações:**
1. ✅ Código foi recompilado?
   - Verificar se log `🔐 Token adicionado ao body` aparece

2. ✅ Token está em `CurrentToken`?
   - Verificar logs `🔍 DEBUG LoadCharacterList`
   - `CurrentToken.IsEmpty()` deve ser `NÃO`
   - `CurrentToken.Length()` deve ser > 100

3. ✅ VaRest está enviando body JSON?
   - Verificar logs VaRest: `Request (json): POST ...`
   - Verificar se body contém `"token"`

4. ✅ Endpoint PHP está recebendo body?
   - Verificar logs PHP (se disponível)
   - Adicionar `error_log(json_encode($data))` no endpoint temporariamente

### Problema: Token válido mas ainda recebe 401

**Possíveis causas:**
1. Chave secreta diferente
   - AuthServer C++: `config.jwtSecret`
   - PHP: `JWT_SECRET` env ou padrão
   - **Solução:** Garantir mesma chave

2. Token expirado
   - Verificar `exp` no payload
   - Token expira em 60 minutos por padrão

3. Formato Base64URL incorreto
   - Verificar se `base64UrlEncode()` está correto
   - Comparar com token gerado pelo C++

---

## Arquivos Modificados

### Backend C++ (Main Repository)
- Nenhum (AuthServer já tinha JWT real)

### Backend PHP (WAMP - Fora do Git)
1. `c:\wamp64\www\umbra_api\api\login.php` - ✅ Atualizado
2. `c:\wamp64\www\umbra_api\api\common\jwt_helper.php` - ✅ Atualizado (função `generateJWT()`)
3. `c:\wamp64\www\umbra_api\api\character\list_characters.php` - ✅ Atualizado
4. `c:\wamp64\www\umbra_api\api\character\create_character.php` - ✅ Atualizado
5. `c:\wamp64\www\umbra_api\api\character\select_character.php` - ✅ Atualizado
6. `c:\wamp64\www\umbra_api\api\character\delete_character.php` - ✅ Atualizado

### Client Unreal (Submodule)
- `Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` - ✅ Atualizado (commits: `8fcb9cc`, `3d97c1f`)

---

## Checklist Final

- [x] ✅ Geração JWT real no `login.php`
- [x] ✅ Validação JWT nos endpoints PHP
- [x] ✅ Código C++ atualizado para enviar token
- [x] ✅ Logs de debug adicionados
- [x] ✅ Commits realizados (submodule e main)
- [ ] ⏭️ **RECOMPILAR PROJETO UE5** ← **AÇÃO NECESSÁRIA**
- [ ] ⏭️ Testar login e listar personagens após recompilação
- [ ] ⏭️ Remover logs de debug após validação

---

## Próximos Passos Imediatos

1. **URGENTE:** Recompilar projeto UE5
2. Testar login e verificar se token JWT é recebido
3. Testar listar personagens e verificar se funciona
4. Verificar logs de debug para confirmar token sendo enviado
5. Remover logs de debug após validação

---

**Documento criado em:** 2025-10-29  
**Última atualização:** 2025-10-29  
**Versão:** 1.0

