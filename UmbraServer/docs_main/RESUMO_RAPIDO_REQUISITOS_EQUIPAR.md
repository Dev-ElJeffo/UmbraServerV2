# ⚡ RESUMO RÁPIDO: Requisitos para Equipar Itens

## ✅ **REQUISITOS:**

1. ✅ **`equipment_slot` != 'none'**
2. ✅ **`player_level >= required_level`** (ou `required_level = NULL/0` = nível 1)
3. ✅ **`is_equipped = FALSE`**
4. ✅ **Item pertence ao jogador**

---

## 🔍 **VERIFICAÇÃO RÁPIDA:**

Execute no MySQL Workbench:

```sql
-- Ver itens e status
SELECT 
    it.item_id,
    it.item_name,
    it.equipment_slot,
    it.required_level,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ NÃO EQUIPÁVEL'
        WHEN it.required_level IS NULL OR it.required_level = 0 THEN '⚠️ SEM NÍVEL'
        ELSE '✅ OK'
    END AS status
FROM item_templates it
ORDER BY it.item_id;
```

---

## 🔧 **CORREÇÃO RÁPIDA:**

1. **Atualizar ENUM (se necessário):**
   ```sql
   -- Execute: atualizar_enum_equipment_slot.sql
   ```

2. **Corrigir itens:**
   ```sql
   -- Execute: corrigir_itens_para_equipar.sql
   ```

3. **Verificar nível do jogador:**
   ```sql
   SELECT id, level FROM players WHERE id = 1;
   UPDATE players SET level = 1 WHERE level IS NULL OR level = 0;
   ```

---

## 📘 **REFERÊNCIAS:**

- **Diagnóstico Completo:** `DIAGNOSTICO_REQUISITOS_EQUIPAR_ITENS.md`
- **Script de Verificação:** `verificar_requisitos_equipar_itens.sql`
- **Script de Correção:** `corrigir_itens_para_equipar.sql`
- **Atualizar ENUM:** `atualizar_enum_equipment_slot.sql`
- **Teste HTML:** `test_equip_item.html`

