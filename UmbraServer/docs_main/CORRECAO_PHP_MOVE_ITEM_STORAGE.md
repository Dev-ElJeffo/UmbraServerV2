# 🔴 CORREÇÃO CRÍTICA: PHP Rejeitando Movimentos Storage → Storage

## 🎯 **PROBLEMA IDENTIFICADO:**

O PHP está retornando erro **"slot_index inválido (deve ser 0-49)"** quando tentamos mover um item dentro do storage (Storage → Storage).

**Log do erro:**
```
LogTemp: [UmbraGameInstance] 🔄 [AUDIT] Tentando mover item - Player: 1, InventoryID: 6, TargetSlotIndex: 130
LogVaRest: Response (400): {"success":false,"message":"slot_index inválido (deve ser 0-49)"}
```

**Causa:** O arquivo `move_item.php` no servidor está **desatualizado** e ainda tem a validação antiga que só aceita 0-49.

---

## ✅ **SOLUÇÃO:**

### **PASSO 1: Verificar o arquivo PHP no servidor**

O arquivo `www/umbra_api/api/inventory/move_item.php` **deve** ter esta validação:

```php
// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}
```

**❌ VALIDAÇÃO ANTIGA (ERRADA):**
```php
if ($target_slot_index < 0 || $target_slot_index >= 50) {  // ❌ Só aceita 0-49
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-49)']);
    exit;
}
```

**✅ VALIDAÇÃO CORRETA (ATUAL):**
```php
if ($target_slot_index < 0 || $target_slot_index >= 150) {  // ✅ Aceita 0-149
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}
```

---

## 📋 **AÇÃO NECESSÁRIA:**

### **1. Atualizar o arquivo PHP no servidor:**

**Arquivo:** `www/umbra_api/api/inventory/move_item.php`

**Localização da validação:** Linha 61-66

**Substitua:**
```php
// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-49 para inventário ou 50-149 para storage)']);
    exit;
}
```

**Por:**
```php
// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}
```

**Nota:** A validação em si já está correta (`>= 150`), mas a mensagem de erro pode estar desatualizada. O importante é garantir que o arquivo no servidor aceite valores de 0-149.

---

## 🔍 **VERIFICAÇÃO:**

Após atualizar o arquivo PHP no servidor:

1. **Teste movendo um item dentro do storage:**
   - Deve aceitar `target_slot_index` de 50-149
   - Não deve retornar erro "slot_index inválido"

2. **Verifique os logs:**
   - O log deve mostrar `TargetSlotIndex: 130` (ou outro valor 50-149)
   - A resposta deve ser `success: true`

---

## 📝 **RESUMO:**

- ✅ **Código C++:** Está correto, converte índices corretamente (0-99 → 50-149)
- ✅ **Código PHP local:** Está correto, aceita 0-149
- ❌ **Código PHP no servidor:** Está desatualizado, precisa ser atualizado

**Ação:** Atualizar o arquivo `move_item.php` no servidor para aceitar `target_slot_index` de 0-149.

