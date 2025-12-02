# SOLUÇÃO: SETTOOLTIPDATA COM SEQUENCE

## 🎯 PROBLEMA

Você colocou tudo no `SetTooltipData` e quando um stat é 0, os outros não aparecem.

**SOLUÇÃO:** Usar `Sequence` para processar cada stat **INDEPENDENTEMENTE**.

---

## ✅ IMPLEMENTAÇÃO NO SETTOOLTIPDATA

### ESTRUTURA CORRETA:

```
Entry → (outras configurações: nome, descrição, etc.)
    │
    └─→ Sequence (para processar stats em paralelo)
         │
         ├─→ Exec 0 → Strength → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 1 → Dexterity → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 2 → Intelligence → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 3 → Vitality → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 4 → Luck → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 5 → PhysicalAttack → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 6 → MagicAttack → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 7 → PhysicalDefense → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 8 → MagicDefense → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 9 → Accuracy → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 10 → Dodge → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 11 → Critical → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 12 → CriticalResistance → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 13 → DoubleAttackRate → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 14 → DoubleAttackResistance → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 15 → HealthBonus → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         ├─→ Exec 16 → ManaBonus → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
         │
         └─→ Exec 17 → Movement → Format Text → Set Text → Greater_IntInt → Branch → Set Visibility
```

**Cada `Exec` processa UM stat COMPLETAMENTE, sem depender dos outros.**

---

## 📋 PASSOS CONCRETOS

### 1. No `SetTooltipData`, após todas as outras configurações:

1. **Criar um nó `Sequence`**
   - Botão direito → "Sequence"
   - Ou arrastar do painel de busca

2. **Conectar o `Then` final das outras configurações ao `Sequence`**

### 2. Para cada stat:

1. **Pegar o valor do Break:**
   - Exemplo: `Strength` do `Break FUmbraItemStatsBase`

2. **Conectar a um `Exec` diferente do `Sequence`:**
   - `Exec 0` para Strength
   - `Exec 1` para Dexterity
   - `Exec 2` para Intelligence
   - E assim por diante...

3. **Em cada `Exec`, criar a cadeia completa:**
   ```
   Valor → Format Text → Set Text (TextBlock)
   Valor → Greater_IntInt (comparar com 0) → Branch
                                              │
                                  ┌───────────┴───────────┐
                                  │                       │
                            Branch True            Branch False
                            (valor > 0)            (valor <= 0)
                                  │                       │
                            Set Visibility         Set Visibility
                            (Visible)              (Collapsed)
                            (Widget)               (Widget)
   ```

---

## 🎯 EXEMPLO CONCRETO: STRENGTH

### No `SetTooltipData`:

1. **Quebrar os structs:**
   ```
   Break ItemTemplate → Stats → Break FUmbraItemStats → Base → Break FUmbraItemStatsBase
   ```

2. **Criar Sequence:**
   ```
   (última configuração anterior) → Then → Sequence
   ```

3. **Conectar Strength ao Exec 0:**
   ```
   Sequence → Exec 0 → Strength (do Break Base)
                              │
                              ├─→ Format Text → Set Text (Text_Strength)
                              │
                              └─→ Greater_IntInt (0) → Branch
                                                      │
                                          ┌───────────┴───────────┐
                                          │                       │
                                    Branch True            Branch False
                                    (Strength > 0)         (Strength <= 0)
                                          │                       │
                                    Set Visibility         Set Visibility
                                    (Visible)              (Collapsed)
                                    (Widget_Strength)      (Widget_Strength)
   ```

4. **Repetir para cada stat em um Exec diferente:**
   - `Exec 1` → Dexterity
   - `Exec 2` → Intelligence
   - `Exec 3` → Vitality
   - `Exec 4` → Luck
   - `Exec 5` → PhysicalAttack (do Break Combat)
   - E assim por diante...

---

## ⚠️ IMPORTANTE

### NÃO faça isso:

**ERRADO:**
```
Strength → Format Text → Set Text
    │
    └─→ Then → Dexterity → Format Text → Set Text
                    │
                    └─→ Then → Intelligence → ...
```

**Isso cria uma sequência onde se um falha, os outros não executam.**

### FAÇA isso:

**CORRETO:**
```
Sequence
  ├─→ Exec 0 → Strength → Format Text → Set Text → Verificação → Set Visibility
  ├─→ Exec 1 → Dexterity → Format Text → Set Text → Verificação → Set Visibility
  └─→ Exec 2 → Intelligence → Format Text → Set Text → Verificação → Set Visibility
```

**Cada Exec é independente. Se Strength = 0, ele fica oculto, mas Dexterity e Intelligence ainda são processados.**

---

## 🔍 VERIFICAÇÃO

### Verifique se:

- [ ] Você tem um `Sequence` no `SetTooltipData`?
- [ ] Cada stat está em um `Exec` diferente do `Sequence`?
- [ ] Não há conexões `Then` entre stats diferentes?
- [ ] Cada stat tem sua própria verificação de visibilidade?
- [ ] A verificação de um stat não afeta outros stats?

---

## 🎯 RESUMO

1. **No `SetTooltipData`, após outras configurações, criar um `Sequence`**
2. **Para cada stat, conectar a um `Exec` diferente do `Sequence`**
3. **Em cada `Exec`, criar a cadeia completa: Format Text → Set Text → Verificação → Set Visibility**
4. **Nenhum stat depende de outro**

**Isso DEVE funcionar.**

