# 📋 GUIA: Criar APIs de Storage

## ✅ **APIS CRIADAS:**

1. ✅ `move_to_storage.php` - Move item do inventário para o storage
2. ✅ `move_from_storage.php` - Move item do storage para o inventário
3. ✅ `get_storage.php` - Obtém todos os itens do storage

## 📋 **TABELA DO BANCO DE DADOS:**

Foi criado o script SQL `setup_storage_table.sql` para criar a tabela `player_storage`.

---

## 🔧 **PASSO 1: Criar Tabela no Banco de Dados**

### **Execute o script SQL:**

1. **Abra o MySQL Workbench ou phpMyAdmin**
2. **Selecione o banco de dados `umbra_eternum`**
3. **Execute o script `setup_storage_table.sql`**

**OU execute diretamente no MySQL:**

```sql
CREATE TABLE IF NOT EXISTS player_storage (
    storage_id INT PRIMARY KEY AUTO_INCREMENT,
    player_id BIGINT UNSIGNED NOT NULL,
    inventory_id INT NOT NULL COMMENT 'Referência ao item em player_inventory',
    slot_index INT NOT NULL COMMENT 'Índice do slot no storage (50-149)',
    
    stored_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES player_inventory(inventory_id) ON DELETE CASCADE,
    
    INDEX idx_player_storage (player_id),
    INDEX idx_slot_index (player_id, slot_index),
    
    UNIQUE KEY unique_player_storage_slot (player_id, slot_index),
    UNIQUE KEY unique_inventory_storage (inventory_id, player_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

---

## 📋 **PASSO 2: Verificar Estrutura das APIs**

### **API 1: move_to_storage.php**

**Endpoint:** `POST /api/storage/move_to_storage.php`

**Body (JSON):**
```json
{
  "token": "jwt_token",
  "inventory_id": 123,
  "target_slot_index": 50  // Índice 50-149
}
```

**Resposta de Sucesso:**
```json
{
  "success": true,
  "message": "Item movido para o storage com sucesso",
  "inventory_id": 123,
  "from_slot": 5,
  "to_slot": 50
}
```

**Validações:**
- ✅ Token JWT válido
- ✅ Item pertence ao jogador
- ✅ Item não está equipado
- ✅ `target_slot_index` está no range 50-149
- ✅ Suporta swap se o slot de destino estiver ocupado

---

### **API 2: move_from_storage.php**

**Endpoint:** `POST /api/storage/move_from_storage.php`

**Body (JSON):**
```json
{
  "token": "jwt_token",
  "storage_item_id": 123,  // storage_id da tabela player_storage
  "target_slot_index": 5   // Índice 0-49
}
```

**Resposta de Sucesso:**
```json
{
  "success": true,
  "message": "Item movido do storage para o inventário com sucesso",
  "storage_id": 123,
  "inventory_id": 456,
  "from_slot": 50,
  "to_slot": 5
}
```

**Validações:**
- ✅ Token JWT válido
- ✅ Item pertence ao jogador
- ✅ `target_slot_index` está no range 0-49
- ✅ Suporta swap se o slot de destino estiver ocupado

---

### **API 3: get_storage.php**

**Endpoint:** `POST /api/storage/get_storage.php`

**Body (JSON):**
```json
{
  "token": "jwt_token"
}
```

**Resposta de Sucesso:**
```json
{
  "success": true,
  "message": "Storage carregado com sucesso",
  "player": {
    "player_id": 1
  },
  "storage": [
    {
      "storage_id": 1,
      "inventory_id": 5,
      "slot_index": 50,
      "item_template_id": 1,
      "quantity": 1,
      ...
    }
  ],
  "total_items": 1
}
```

---

## 🔧 **PASSO 3: Atualizar LoadStorage no C++ (Opcional)**

**Se você quiser que `LoadStorage()` no C++ chame a API:**

Você pode modificar `UmbraStorageWidget.cpp` para chamar `get_storage.php` e popular `StorageSlots`.

**Por enquanto, as funções Blueprint que você criou já chamam as APIs diretamente, então não é necessário.**

---

## ⚠️ **IMPORTANTE:**

1. **Estrutura da Tabela:**
   - `player_storage` referencia `player_inventory` (não cria novos itens)
   - Quando um item é movido para o storage, seu `slot_index` em `player_inventory` é atualizado para 50-149
   - A tabela `player_storage` mantém o controle de quais itens estão no storage

2. **Índices:**
   - Inventário: 0-49
   - Storage: 50-149
   - A API valida esses ranges

3. **Swap:**
   - Ambas as APIs suportam swap (trocar itens de lugar)
   - Se o slot de destino estiver ocupado, os itens são trocados

4. **Itens Equipados:**
   - `move_to_storage.php` não permite mover itens equipados para o storage

---

## 🧪 **TESTE:**

1. Execute o script SQL para criar a tabela
2. Teste `get_storage.php` (deve retornar array vazio inicialmente)
3. Teste `move_to_storage.php` (mover item do inventário para storage)
4. Teste `move_from_storage.php` (mover item do storage para inventário)
5. Teste swap (mover para slot ocupado)

---

## 📝 **RESUMO:**

1. ✅ Execute `setup_storage_table.sql` para criar a tabela
2. ✅ APIs criadas em `www/umbra_api/api/storage/`
3. ✅ Agora você pode usar as funções Blueprint que chamam essas APIs
4. ✅ Modifique o `OnDrop` no `WBP_InventorySlot` para usar as funções Blueprint

