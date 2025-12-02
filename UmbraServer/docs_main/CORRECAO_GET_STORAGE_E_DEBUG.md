# 🔧 CORREÇÃO: get_storage.php e Sistema de Debug

## ✅ **CORREÇÕES APLICADAS:**

### **1. get_storage.php - INNER JOIN player_storage**

O arquivo `get_storage.php` foi corrigido para usar `INNER JOIN player_storage` (como você solicitou), garantindo que apenas itens registrados no storage sejam retornados:

```php
FROM player_storage s
INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
INNER JOIN item_templates t ON i.item_template_id = t.item_id
WHERE s.player_id = :player_id
  AND i.slot_index >= 50
  AND i.slot_index < 150
ORDER BY i.slot_index ASC
```

**⚠️ IMPORTANTE:** Se o storage estiver vazio, significa que:
- Não há itens em `player_storage` para este `player_id`
- OU os itens não têm `slot_index` entre 50-149
- OU o `move_item.php` não está atualizando `player_storage` corretamente

---

### **2. Logs Detalhados no C++**

Adicionei logs extremamente detalhados em `OnLoadStorageRequestComplete` para diagnosticar o problema:

- **Log da resposta JSON completa** do PHP
- **Log de cada item antes e depois de parsear**
- **Log de erros** se o array estiver vazio
- **Log dos primeiros 3 itens** após parsear

**Como ver os logs:**
1. Abra o Unreal Engine
2. Abra a janela "Output Log" (Window → Developer Tools → Output Log)
3. Filtre por "LogTemp" ou "UmbraGameInstance"
4. Procure por mensagens com `[DEBUG]` ou `📦`

---

### **3. Página HTML de Debug**

Criei `www/umbra_api/debug_storage.html` para testar o `get_storage.php` diretamente no navegador:

**Como usar:**
1. Abra `http://localhost/umbra_api/debug_storage.html` no navegador
2. Cole seu JWT token
3. Clique em "Carregar Storage"
4. Analise:
   - Quantos itens foram retornados
   - Se os itens têm `storage_id` > 0
   - Se os `slot_index` estão entre 50-149
   - O JSON completo da resposta

---

## 🔍 **DIAGNÓSTICO - O QUE VERIFICAR:**

### **PASSO 1: Verificar o Banco de Dados**

Execute no MySQL:

```sql
-- Verificar itens no storage para seu player_id
SELECT 
    s.storage_id,
    s.player_id,
    s.inventory_id,
    s.slot_index as storage_slot_index,
    i.slot_index as inventory_slot_index,
    i.item_template_id,
    t.item_name
FROM player_storage s
INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
INNER JOIN item_templates t ON i.item_template_id = t.item_id
WHERE s.player_id = SEU_PLAYER_ID
ORDER BY i.slot_index ASC;
```

**O que verificar:**
- ✅ Há registros em `player_storage`?
- ✅ Os `inventory_slot_index` estão entre 50-149?
- ✅ Os `storage_slot_index` correspondem aos `inventory_slot_index`?

---

### **PASSO 2: Testar a API Diretamente**

Use a página `debug_storage.html` ou teste via curl:

```bash
curl -X POST http://localhost/umbra_api/api/storage/get_storage.php \
  -H "Content-Type: application/json" \
  -d '{"token":"SEU_JWT_TOKEN"}'
```

**O que verificar:**
- ✅ A resposta tem `"success": true`?
- ✅ O array `"storage"` tem itens?
- ✅ Cada item tem `storage_id` > 0?
- ✅ Cada item tem `slot_index` entre 50-149?

---

### **PASSO 3: Verificar os Logs do Unreal**

Após compilar e executar:

1. Abra o Output Log no Unreal
2. Filtre por `[DEBUG]` ou `📦`
3. Procure por:
   - `Resposta completa do get_storage.php` - mostra o JSON retornado
   - `Parseando X itens do storage` - mostra quantos itens foram encontrados
   - `Item X/Y (ANTES de parsear)` - mostra o JSON de cada item
   - `Item X/Y (DEPOIS de parsear)` - mostra os dados parseados
   - `CurrentStorage tem X itens` - mostra quantos itens foram adicionados

**Se `CurrentStorage` estiver vazio:**
- Verifique se o array `"storage"` na resposta do PHP está vazio
- Verifique se há erros de parsing nos logs
- Verifique se os `slot_index` estão no formato correto (50-149)

---

### **PASSO 4: Sincronizar o Storage (se necessário)**

Se você descobrir que há itens em `player_inventory` com `slot_index` entre 50-149, mas não há em `player_storage`, use o `sync_storage.php`:

1. Abra `http://localhost/umbra_api/sync_storage.html`
2. Cole seu JWT token
3. Clique em "Sincronizar Storage"
4. Isso criará entradas em `player_storage` para itens que estão em slots 50-149

---

## 🎯 **POSSÍVEIS CAUSAS DO PROBLEMA:**

### **Causa 1: player_storage está vazio**
**Solução:** Execute `sync_storage.php` para sincronizar

### **Causa 2: move_item.php não está atualizando player_storage**
**Solução:** O `move_item.php` já foi corrigido para atualizar `player_storage`. Verifique se a versão no servidor está atualizada.

### **Causa 3: Itens não estão em slots 50-149**
**Solução:** Verifique no MySQL se os itens têm `slot_index` entre 50-149

### **Causa 4: INNER JOIN não encontra correspondências**
**Solução:** Verifique se:
- `player_storage.inventory_id` corresponde a `player_inventory.inventory_id`
- `player_storage.player_id` corresponde ao `player_id` do token
- Os itens existem em ambas as tabelas

---

## 📝 **PRÓXIMOS PASSOS:**

1. **Compile o C++** para aplicar os novos logs
2. **Teste a API** usando `debug_storage.html`
3. **Verifique o banco de dados** usando a query SQL acima
4. **Execute o sync_storage** se necessário
5. **Analise os logs do Unreal** para ver o que está sendo retornado
6. **Me envie:**
   - O resultado da query SQL
   - A resposta do `debug_storage.html`
   - Os logs do Unreal (especialmente as mensagens `[DEBUG]`)

Com essas informações, poderemos identificar exatamente onde está o problema!

