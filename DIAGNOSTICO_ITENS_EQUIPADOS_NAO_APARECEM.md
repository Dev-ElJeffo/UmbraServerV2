# 🔍 DIAGNÓSTICO: Itens Equipados Não Aparecem nos Slots

## ❌ **PROBLEMA:**

Os itens estão marcados como `is_equipped = TRUE` no banco de dados, mas não aparecem nos slots de equipamento do `WBP_CharacterInfo`.

**Sintomas:**
- API retorna "Item já está equipado" ao tentar equipar
- Itens aparecem no inventário, não nos slots de equipamento
- Slots de equipamento estão vazios

---

## 🔍 **POSSÍVEIS CAUSAS:**

### **1. Item com `equipment_slot = 'none'` ou `NULL`**

**Problema:** Item está marcado como equipado, mas não tem slot válido.

**Verificação:**
```sql
SELECT pi.inventory_id, it.item_name, it.equipment_slot, pi.is_equipped
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);
```

**Solução:**
```sql
-- Desequipar itens com slot inválido
UPDATE player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
SET pi.is_equipped = FALSE
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);
```

---

### **2. Múltiplos Itens no Mesmo Slot**

**Problema:** Mais de um item está equipado no mesmo slot (ex: duas armas em `main_hand`).

**Verificação:**
```sql
SELECT pi.player_id, it.equipment_slot, COUNT(*) AS quantidade
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND it.equipment_slot != 'none'
GROUP BY pi.player_id, it.equipment_slot
HAVING COUNT(*) > 1;
```

**Solução:**
- Desequipar itens duplicados manualmente
- Ou executar o script de correção

---

### **3. `equipment_slot` com Valor Desconhecido**

**Problema:** `equipment_slot` não está no ENUM esperado pelo C++.

**Valores Válidos:**
- `head`, `chest`, `legs`, `feet`, `hands`
- `main_hand`, `off_hand`
- `ring`, `amulet`, `earring`, `bracelet`, `mount`

**Verificação:**
```sql
SELECT it.equipment_slot, COUNT(*) 
FROM item_templates it
WHERE it.equipment_slot NOT IN ('none', 'head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'earring', 'bracelet', 'mount')
GROUP BY it.equipment_slot;
```

---

### **4. C++ Não Está Parseando Corretamente**

**Verificação:**
1. Abra o Output Log do Unreal Engine
2. Procure por logs de `OnLoadCharacterInfoComplete`
3. Verifique se `EquippedItems` está sendo populado

**Logs Esperados:**
```
[UmbraGameInstance] ✅ Character info carregado
[UmbraGameInstance] 🗺️ EquippedItems atualizado: X itens equipados
```

**Se não aparecer:**
- Verifique se `LoadCharacterInfo()` está sendo chamado após equipar
- Verifique se o JSON da API está no formato correto

---

### **5. Widget Não Está Atualizando**

**Verificação:**
1. Abra `WBP_CharacterInfo` no Blueprint Editor
2. Verifique se `OnCharacterInfoLoaded_Event` está conectado
3. Verifique se `Update Equipment Slots` está sendo chamado
4. Verifique se `Get Equipped Items Array` está retornando dados

**Teste:**
- Adicione um `Print String` em `Update Equipment Slots` para ver quantos itens estão sendo processados

---

## 🔧 **SOLUÇÃO PASSO A PASSO:**

### **PASSO 1: Executar Script de Verificação**

Execute `verificar_itens_equipados.sql` no MySQL Workbench:

```sql
-- Ver todos os itens equipados
SELECT 
    pi.inventory_id,
    it.item_name,
    it.equipment_slot,
    pi.is_equipped
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE;
```

**Análise:**
- Se houver itens com `equipment_slot = 'none'` → **Problema 1**
- Se houver múltiplos itens no mesmo slot → **Problema 2**
- Se houver slots desconhecidos → **Problema 3**

---

### **PASSO 2: Corrigir Itens com Problemas**

**Opção A: Desequipar Todos e Reequipar**

```sql
-- Desequipar todos os itens
UPDATE player_inventory 
SET is_equipped = FALSE 
WHERE is_equipped = TRUE;
```

Depois, reequipe os itens pelo jogo.

**Opção B: Corrigir Apenas os Problemáticos**

```sql
-- Desequipar apenas itens com slot inválido
UPDATE player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
SET pi.is_equipped = FALSE
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);
```

---

### **PASSO 3: Testar API Diretamente**

Use `test_equip_item.html` ou Postman para testar:

1. **Desequipar todos os itens:**
   ```json
   POST /api/inventory/equip_item.php
   {
     "token": "seu_token",
     "inventory_id": 1,
     "equip": false
   }
   ```

2. **Equipar novamente:**
   ```json
   POST /api/inventory/equip_item.php
   {
     "token": "seu_token",
     "inventory_id": 1,
     "equip": true
   }
   ```

3. **Verificar `get_character_info.php`:**
   ```json
   POST /api/character/get_character_info.php
   {
     "token": "seu_token"
   }
   ```

   **Verifique se `equipped_items` contém o item:**
   ```json
   {
     "equipped_items": {
       "main_hand": {
         "inventory_id": 1,
         "item_name": "Espada de Ferro",
         ...
       }
     }
   }
   ```

---

### **PASSO 4: Verificar Logs do Unreal Engine**

1. Abra o Output Log
2. Equipe um item
3. Procure por:
   - `✅ [AUDIT] Item equipado com sucesso`
   - `LoadCharacterInfo()` sendo chamado
   - `OnCharacterInfoLoaded` sendo disparado
   - `EquippedItems atualizado: X itens equipados`

---

### **PASSO 5: Verificar Widget**

1. Abra `WBP_CharacterInfo` no Blueprint Editor
2. Verifique `OnCharacterInfoLoaded_Event`:
   - Deve chamar `Update Equipment Slots`
   - Deve passar `Character Info` como parâmetro

3. Verifique `Update Equipment Slots`:
   - Deve usar `Get Equipped Items Array` (do Game Instance)
   - Deve iterar sobre o array
   - Deve chamar `Update Slot Visual` para cada slot

---

## 📋 **CHECKLIST DE DIAGNÓSTICO:**

- [ ] Executar `verificar_itens_equipados.sql`
- [ ] Verificar se há itens com `equipment_slot = 'none'`
- [ ] Verificar se há múltiplos itens no mesmo slot
- [ ] Testar API diretamente (`test_equip_item.html`)
- [ ] Verificar logs do Unreal Engine
- [ ] Verificar se `LoadCharacterInfo()` está sendo chamado
- [ ] Verificar se `OnCharacterInfoLoaded_Event` está conectado
- [ ] Verificar se `Update Equipment Slots` está sendo chamado
- [ ] Verificar se `Get Equipped Items Array` retorna dados

---

## 🎯 **SOLUÇÃO RÁPIDA:**

Se quiser uma solução rápida, execute:

```sql
-- Desequipar todos os itens
UPDATE player_inventory 
SET is_equipped = FALSE 
WHERE is_equipped = TRUE;
```

Depois, reequipe os itens pelo jogo. Isso deve resolver a maioria dos problemas.

---

## 📘 **REFERÊNCIAS:**

- **Script de Verificação:** `verificar_itens_equipados.sql`
- **Correção C++:** `CORRECAO_ATUALIZAR_SLOTS_EQUIPAMENTO.md`
- **Teste HTML:** `test_equip_item.html`

