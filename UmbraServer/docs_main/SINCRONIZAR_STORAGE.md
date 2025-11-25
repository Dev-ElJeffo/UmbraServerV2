# 🔧 SINCRONIZAR STORAGE - Corrigir Itens Faltando

## 🎯 **PROBLEMA:**

Itens no storage (slots 50-149 em `player_inventory`) podem não ter entrada correspondente em `player_storage`, fazendo com que não apareçam no `get_storage.php`.

---

## ✅ **SOLUÇÃO:**

### **1. Execute o script de sincronização:**

**Opção A: Via HTML (mais fácil)**

Crie um arquivo HTML temporário:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Sync Storage</title>
</head>
<body>
    <h1>Sincronizar Storage</h1>
    <button onclick="syncStorage()">Sincronizar</button>
    <div id="result"></div>
    
    <script>
        async function syncStorage() {
            const token = prompt("Cole seu JWT token:");
            if (!token) return;
            
            const response = await fetch('http://localhost/umbra_api/api/storage/sync_storage.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ token: token })
            });
            
            const data = await response.json();
            document.getElementById('result').innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
        }
    </script>
</body>
</html>
```

**Opção B: Via cURL (terminal)**

```bash
curl -X POST http://localhost/umbra_api/api/storage/sync_storage.php \
  -H "Content-Type: application/json" \
  -d '{"token":"SEU_JWT_TOKEN_AQUI"}'
```

**Opção C: Via MySQL (direto no banco)**

```sql
-- Criar entradas em player_storage para itens que estão em slots 50-149 mas não têm entrada
INSERT INTO player_storage (player_id, inventory_id, slot_index)
SELECT i.player_id, i.inventory_id, i.slot_index
FROM player_inventory i
LEFT JOIN player_storage s ON s.inventory_id = i.inventory_id AND s.player_id = i.player_id
WHERE i.player_id = 1  -- Substitua pelo seu player_id
  AND i.slot_index >= 50
  AND i.slot_index < 150
  AND s.storage_id IS NULL;

-- Atualizar entradas existentes onde o slot_index está desatualizado
UPDATE player_storage s
INNER JOIN player_inventory i ON s.inventory_id = i.inventory_id
SET s.slot_index = i.slot_index
WHERE s.player_id = 1  -- Substitua pelo seu player_id
  AND i.slot_index >= 50
  AND i.slot_index < 150
  AND s.slot_index != i.slot_index;
```

---

## 📝 **APÓS SINCRONIZAR:**

1. **Feche e reabra o storage no jogo**
2. **Os itens devem aparecer corretamente**

---

## ⚠️ **NOTA:**

Após esta sincronização, o `move_item.php` atualizado vai manter `player_storage` sincronizado automaticamente em movimentos futuros.

