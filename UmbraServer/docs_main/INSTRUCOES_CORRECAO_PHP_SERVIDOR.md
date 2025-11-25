# ⚠️ CORREÇÃO URGENTE: Atualizar move_item.php no Servidor

## 🔴 **PROBLEMA:**

O arquivo `move_item.php` no servidor está **desatualizado** e está rejeitando movimentos Storage → Storage.

**Erro atual:**
```
Response (400): {"success":false,"message":"slot_index inválido (deve ser 0-49)"}
```

---

## ✅ **SOLUÇÃO:**

### **PASSO 1: Localizar o arquivo no servidor**

**Caminho:** `www/umbra_api/api/inventory/move_item.php`

### **PASSO 2: Verificar a linha 61-66**

O arquivo deve ter esta validação:

```php
// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}
```

### **PASSO 3: Se a validação estiver incorreta, substituir**

**❌ VALIDAÇÃO INCORRETA (ANTIGA):**
```php
if ($target_slot_index < 0 || $target_slot_index >= 50) {  // ❌ Só aceita 0-49
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-49)']);
    exit;
}
```

**✅ SUBSTITUIR POR (CORRETA):**
```php
// Validar slot index (aceita 0-49 para inventário e 50-149 para storage)
// IMPORTANTE: Esta validação permite movimentos dentro do storage (50-149)
if ($target_slot_index < 0 || $target_slot_index >= 150) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'slot_index inválido (deve ser 0-149: 0-49 para inventário, 50-149 para storage)']);
    exit;
}
```

### **PASSO 4: Salvar e testar**

Após atualizar:
1. Salve o arquivo
2. Teste movendo um item dentro do storage
3. Verifique se não retorna mais o erro "slot_index inválido (deve ser 0-49)"

---

## 📋 **VERIFICAÇÃO RÁPIDA:**

**A validação está correta se:**
- ✅ Aceita `target_slot_index` de **0-149**
- ✅ Rejeita valores `< 0` ou `>= 150`
- ✅ Mensagem de erro menciona "0-149" ou "50-149 para storage"

**A validação está incorreta se:**
- ❌ Só aceita `target_slot_index` de **0-49**
- ❌ Rejeita valores `>= 50`
- ❌ Mensagem de erro só menciona "0-49"

---

## 🔍 **ARQUIVO DE REFERÊNCIA:**

O arquivo local `www/umbra_api/api/inventory/move_item.php` já está correto e pode ser usado como referência.

**Linha 63:** `if ($target_slot_index < 0 || $target_slot_index >= 150) {`

Esta é a validação correta que aceita 0-149.

