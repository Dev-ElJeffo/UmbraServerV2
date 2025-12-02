# 🔧 Correção: Equipment Slot do Colar do Mestre

## ❌ **Problema:**
O item "Colar do Mestre" está com `equipment_slot = 'amulet'` quando deveria ser `'necklace'`.

## ✅ **Solução:**

### **PASSO 1: Atualizar o ENUM da tabela (se necessário)**

Execute o script:
```
www/umbra_api/scripts/atualizar_enum_equipment_slot.sql
```

Este script:
- Adiciona `'necklace'` ao ENUM da coluna `equipment_slot` na tabela `item_templates`
- Inclui todos os slots: `none`, `head`, `chest`, `legs`, `feet`, `hands`, `main_hand`, `off_hand`, `ring`, `amulet`, **`necklace`**, `earring`, `bracelet`, `mount`

### **PASSO 2: Corrigir o equipment_slot do item**

Execute o script:
```
CORRIGIR_EQUIPMENT_SLOT_NECKLACE.sql
```

Este script:
- Atualiza o `equipment_slot` do item "Colar do Mestre" de `'amulet'` para `'necklace'`
- Verifica se a atualização foi bem-sucedida

## 📋 **Ordem de Execução:**

1. ✅ Execute `www/umbra_api/scripts/atualizar_enum_equipment_slot.sql` (se o ENUM ainda não incluir 'necklace')
2. ✅ Execute `CORRIGIR_EQUIPMENT_SLOT_NECKLACE.sql`

## 🎯 **Resultado Esperado:**

Após executar os scripts, o item "Colar do Mestre" deve ter:
- `equipment_slot = 'necklace'`
- Ser reconhecido corretamente pelo C++ (`ParseEquipmentSlot`)
- Aparecer no slot correto no `WBP_CharacterInfo` (se o Blueprint estiver configurado)

## ⚠️ **Nota:**

Se você já executou `atualizar_enum_equipment_slot.sql` anteriormente (antes desta atualização), execute-o novamente para incluir `'necklace'` no ENUM.

