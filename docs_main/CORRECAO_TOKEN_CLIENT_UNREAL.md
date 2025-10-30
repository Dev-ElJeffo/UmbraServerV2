# Correção: Envio de Token JWT no Client Unreal

**Data:** 29 de Outubro de 2025  
**Status:** ✅ **ANÁLISE COMPLETA - PRONTO PARA IMPLEMENTAÇÃO**

## Problema Identificado

Do log do PIE:
```
LogTemp: [UmbraGameInstance] Carregando lista de personagens para account_id: 4
LogVaRest: Request (json): POST http://localhost/umbra_api/api/character/list_characters.php
LogVaRest: Response (401): {"success":false,"message":"Token não fornecido"}
```

**Causa raiz:**
1. O token JWT está sendo adicionado no header `Authorization: Bearer <token>` via `CreateRequest()`
2. MAS o VaRest pode não estar enviando o header corretamente
3. O endpoint PHP espera token no body JSON OU no header Authorization
4. O token não está sendo enviado no body JSON

## Análise do Código Atual

### `LoadCharacterList()` - Linha 455-488
```cpp
void UUmbraGameInstance::LoadCharacterList()
{
    // ...
    UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/character/list_characters.php"));
    // ...
    UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
    JsonObject->SetNumberField(TEXT("account_id"), CurrentAccount.ID); // ❌ Não necessário mais
    
    // ❌ FALTA: Adicionar token no body JSON
    Request->SetRequestObject(JsonObject);
}
```

### `CreateRequest()` - Linha 308-345
```cpp
UVaRestRequestJSON* UUmbraGameInstance::CreateRequest(...)
{
    // ...
    if (!CurrentToken.IsEmpty())
    {
        Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + CurrentToken); // ✅ Header está sendo adicionado
    }
    // ...
}
```

## Solução

### 1. Adicionar Token no Body JSON (Todos os Endpoints)

O endpoint PHP aceita token em dois lugares:
- Header: `Authorization: Bearer <token>` ✅ Já implementado
- Body JSON: `{"token": "<token>"}` ❌ FALTA IMPLEMENTAR

**Solução mais segura:** Adicionar token no body JSON também, para garantir compatibilidade.

### 2. Remover account_id do Body (Opcional mas Recomendado)

Como `account_id` agora vem do token JWT, não é necessário enviar no body. No entanto, pode ser mantido para validação adicional (o endpoint PHP verifica se corresponde ao token).

---

## Mudanças Necessárias

### 1. `LoadCharacterList()` - ATUALIZAR

**Antes:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
JsonObject->SetNumberField(TEXT("account_id"), CurrentAccount.ID);
```

**Depois:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
// Adicionar token no body JSON (garantia de funcionamento)
if (!CurrentToken.IsEmpty())
{
    JsonObject->SetStringField(TEXT("token"), CurrentToken);
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🔐 Token adicionado ao body: %s"), *CurrentToken);
}
// account_id opcional - endpoint PHP verifica se corresponde ao token
// JsonObject->SetNumberField(TEXT("account_id"), CurrentAccount.ID);
```

### 2. `SelectCharacter()` - ATUALIZAR

**Antes:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
JsonObject->SetNumberField(TEXT("player_id"), PlayerID);
JsonObject->SetNumberField(TEXT("account_id"), CurrentAccount.ID);
```

**Depois:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
JsonObject->SetNumberField(TEXT("player_id"), PlayerID);
// Adicionar token no body JSON
if (!CurrentToken.IsEmpty())
{
    JsonObject->SetStringField(TEXT("token"), CurrentToken);
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🔐 Token adicionado ao body"));
}
// account_id vem do token agora, não do body
```

### 3. `CreateCharacter()` - ATUALIZAR

**Localizar função e atualizar:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
JsonObject->SetStringField(TEXT("character_name"), CharacterName);
// Adicionar token no body JSON
if (!CurrentToken.IsEmpty())
{
    JsonObject->SetStringField(TEXT("token"), CurrentToken);
}
// Não enviar account_id - vem do token
```

### 4. `DeleteCharacter()` - ATUALIZAR (se existir)

**Similar aos outros:**
```cpp
UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
JsonObject->SetNumberField(TEXT("player_id"), PlayerID);
// Adicionar token no body JSON
if (!CurrentToken.IsEmpty())
{
    JsonObject->SetStringField(TEXT("token"), CurrentToken);
}
// Não enviar account_id - vem do token
```

---

## Estrutura das Requisições Atualizadas

### Antes (NÃO FUNCIONA)
```json
POST /api/character/list_characters.php
Headers: Authorization: Bearer <token>
Body: {
  "account_id": 4
}
```

### Depois (FUNCIONA)
```json
POST /api/character/list_characters.php
Headers: Authorization: Bearer <token>
Body: {
  "token": "<token>"
}
```

**OU apenas body (fallback se header não funcionar):**
```json
POST /api/character/list_characters.php
Body: {
  "token": "<token>"
}
```

---

## Checklist de Implementação

- [x] ✅ Atualizar `LoadCharacterList()` - adicionar token no body
- [x] ✅ Atualizar `SelectCharacter()` - adicionar token no body
- [x] ✅ Atualizar `CreateCharacter()` - adicionar token no body
- [x] ✅ Atualizar `DeleteCharacter()` - adicionar token no body
- [x] ✅ Adicionar logs de debug para verificar token sendo enviado
- [x] ✅ Remover `account_id` do body (agora vem do token)
- [ ] ⏭️ Testar todos os endpoints após mudanças (requer compilação UE5)

## Implementações Realizadas

### 1. `LoadCharacterList()` - ATUALIZADO ✅
- Adicionado: `JsonObject->SetStringField(TEXT("token"), CurrentToken);`
- Removido: `account_id` do body (agora vem do token)
- Logs de debug adicionados

### 2. `SelectCharacter()` - ATUALIZADO ✅
- Adicionado: `JsonObject->SetStringField(TEXT("token"), CurrentToken);`
- Removido: `account_id` do body
- Logs de debug adicionados

### 3. `CreateCharacter()` - ATUALIZADO ✅
- Adicionado: `JsonObject->SetStringField(TEXT("token"), CurrentToken);`
- Removido: `account_id` do body
- Logs de debug adicionados

### 4. `DeleteCharacter()` - ATUALIZADO ✅
- Adicionado: `JsonObject->SetStringField(TEXT("token"), CurrentToken);`
- Removido: `account_id` do body
- Logs de debug adicionados

---

## Nota Importante sobre o Token

Do log:
```
"token": "amVmZm86MTc2MTc4NTEzNQ=="
```

Este token parece ser base64 de `username:timestamp`, não um JWT real. O endpoint PHP JWT helper espera um token JWT no formato:
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjE3ODUxMzUsImV4cCI6MTc2MTk0NzkzNX0.signature
```

**Ação adicional necessária:**
- O `login.php` precisa gerar um JWT real ao invés de base64 simples
- OU os endpoints PHP precisam aceitar ambos os formatos temporariamente

---

## Logs de Debug Recomendados

Adicionar em cada função que envia requisição:
```cpp
UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🔐 Enviando token: %s"), *CurrentToken);
UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 📤 Body JSON: %s"), *JsonObject->EncodeJson());
```

---

**Documento criado em:** 2025-10-29  
**Versão:** 1.0

