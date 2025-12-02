# 🔍 DIAGNÓSTICO: Requisitos para Equipar Itens

## ✅ **REQUISITOS PARA EQUIPAR UM ITEM:**

Baseado no código PHP `equip_item.php`, um item pode ser equipado se:

1. ✅ **`equipment_slot` NÃO é 'none'** (linha 88)
   - O item precisa ter um slot de equipamento válido
   - Valores válidos: `head`, `chest`, `legs`, `feet`, `hands`, `main_hand`, `off_hand`, `ring`, `amulet`

2. ✅ **Jogador tem nível suficiente** (linha 103)
   - `player_level >= required_level`
   - Se `required_level` for NULL ou 0, será considerado como 1

3. ✅ **Item não está já equipado** (linha 115)
   - `is_equipped` deve ser `FALSE`

4. ✅ **Item pertence ao jogador** (linha 70)
   - O `inventory_id` deve existir e pertencer ao `player_id` do token

---

## 🔍 **VERIFICAÇÕES NECESSÁRIAS:**

### **1. Verificar Estrutura da Tabela `item_templates`**

Execute no MySQL Workbench:
```sql
DESCRIBE item_templates;
```

**Campos necessários:**
- `equipment_slot` (ENUM) - deve ter valores válidos
- `required_level` (INT) - nível mínimo para usar o item

---

### **2. Verificar Itens no Banco**

Execute o script: `verificar_requisitos_equipar_itens.sql`

**O que verifica:**
- Itens com `equipment_slot = 'none'` → ❌ Não podem ser equipados
- Itens sem `required_level` → ⚠️ Precisam ter nível definido
- Itens no inventário que não podem ser equipados
- Itens no inventário com nível insuficiente

---

### **3. Verificar Nível do Jogador**

```sql
SELECT id, character_name, level FROM players WHERE id = 1;
```

**Problema comum:**
- Jogador com `level = NULL` ou `level = 0`
- **Solução:** Definir `level = 1` (nível mínimo)

---

## 🔧 **CORREÇÕES POSSÍVEIS:**

### **OPÇÃO 1: Executar Script de Correção Automática**

Execute o script: `corrigir_itens_para_equipar.sql`

**O que faz:**
1. Define `equipment_slot` baseado em `item_type` e `item_subtype`
2. Define `required_level = 1` para itens sem nível
3. Define `level = 1` para jogadores sem nível

---

### **OPÇÃO 2: Corrigir Manualmente**

**Para cada item que não pode ser equipado:**

```sql
-- Exemplo: Definir um item como equipável na cabeça
UPDATE item_templates
SET equipment_slot = 'head',
    required_level = 1
WHERE item_id = 1;
```

**Valores válidos para `equipment_slot` (conforme C++):**
- `none` - Não equipável
- `head` - Cabeça
- `chest` - Torso/Armadura
- `legs` - Pernas
- `feet` - Pés/Botas
- `hands` - Mãos/Luvas
- `main_hand` - Mão Principal (Arma)
- `off_hand` - Mão Secundária (Escudo)
- `ring` - Anel
- `amulet` - Colar/Amuleto
- `earring` - Brinco ⚠️ **Pode não estar no ENUM do MySQL**
- `bracelet` - Bracelete ⚠️ **Pode não estar no ENUM do MySQL**
- `mount` - Montaria ⚠️ **Pode não estar no ENUM do MySQL**

**⚠️ IMPORTANTE:** O ENUM da tabela `item_templates` pode não incluir `earring`, `bracelet`, `mount`. 

**Solução:** Execute o script `atualizar_enum_equipment_slot.sql` para atualizar o ENUM e incluir todos os slots.

---

## 📋 **CHECKLIST DE DIAGNÓSTICO:**

1. [ ] Executar `verificar_requisitos_equipar_itens.sql` no MySQL Workbench
2. [ ] Verificar se o ENUM inclui todos os slots (executar `atualizar_enum_equipment_slot.sql` se necessário)
3. [ ] Verificar se há itens com `equipment_slot = 'none'`
4. [ ] Verificar se há itens sem `required_level`
5. [ ] Verificar nível do jogador (deve ser >= 1)
6. [ ] Verificar se os itens no inventário pertencem ao jogador correto
7. [ ] Executar `corrigir_itens_para_equipar.sql` se necessário
8. [ ] Testar equipar um item novamente

---

## 🎯 **PROBLEMAS COMUNS E SOLUÇÕES:**

### **Problema 1: "Este item não pode ser equipado"**

**Causa:** `equipment_slot = 'none'` ou `NULL`

**Solução:**
```sql
UPDATE item_templates
SET equipment_slot = 'main_hand'  -- ou outro slot válido
WHERE item_id = X;
```

---

### **Problema 2: "Nível insuficiente"**

**Causa:** `player_level < required_level`

**Solução:**
```sql
-- Opção A: Aumentar nível do jogador
UPDATE players SET level = 10 WHERE id = 1;

-- Opção B: Reduzir nível requerido do item
UPDATE item_templates SET required_level = 1 WHERE item_id = X;
```

---

### **Problema 3: Item não aparece no inventário**

**Causa:** Item não foi adicionado ao inventário do jogador

**Solução:**
```sql
-- Verificar se o item está no inventário
SELECT * FROM player_inventory WHERE player_id = 1;

-- Se não estiver, adicionar
INSERT INTO player_inventory (player_id, item_template_id, slot_index, quantity)
VALUES (1, X, 0, 1);
```

---

## 🔍 **SCRIPT DE DIAGNÓSTICO RÁPIDO:**

Execute este SQL para ver todos os problemas de uma vez:

```sql
-- Ver todos os itens e seus status
SELECT 
    it.item_id,
    it.item_name,
    it.equipment_slot,
    it.required_level,
    p.level AS player_level,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ NÃO EQUIPÁVEL'
        WHEN p.level < it.required_level THEN CONCAT('⚠️ NÍVEL INSUFICIENTE (Requer: ', it.required_level, ')')
        ELSE '✅ PODE SER EQUIPADO'
    END AS status
FROM item_templates it
CROSS JOIN players p
WHERE p.id = 1  -- ID do jogador
ORDER BY it.item_id;
```

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Execute `verificar_requisitos_equipar_itens.sql`** no MySQL Workbench
   - Analise os resultados
   - Identifique itens com problemas

2. **Execute `atualizar_enum_equipment_slot.sql`** (se necessário)
   - Atualiza o ENUM para incluir `earring`, `bracelet`, `mount`
   - ⚠️ **IMPORTANTE:** Faça backup antes de executar!

3. **Execute `corrigir_itens_para_equipar.sql`** (se necessário)
   - Corrige itens com `equipment_slot = 'none'`
   - Define `required_level` para itens sem nível
   - Define `level = 1` para jogadores sem nível

4. **Teste equipar um item novamente**
   - Abra o Character Info (C)
   - Tente equipar um item
   - Verifique os logs do Unreal Engine se não funcionar

5. **Se ainda não funcionar:**
   - Verifique os logs do Unreal Engine (Output Log)
   - Verifique os logs do PHP (se houver)
   - Teste a API diretamente com um cliente HTTP (Postman, etc.)

---

## 📘 **REFERÊNCIAS:**

- **Script de Verificação:** `verificar_requisitos_equipar_itens.sql`
- **Script de Correção:** `corrigir_itens_para_equipar.sql`
- **API PHP:** `www/umbra_api/api/inventory/equip_item.php`

