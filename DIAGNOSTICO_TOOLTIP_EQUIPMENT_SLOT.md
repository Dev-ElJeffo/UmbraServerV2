# 🔍 DIAGNÓSTICO: Tooltip não aparece no WBP_EquipmentSlot

## 🎯 **PROBLEMA:**

O tooltip funciona no `WBP_InventorySlot` e `WBP_StorageSlot`, mas não aparece no `WBP_EquipmentSlot`, mesmo usando o mesmo código.

---

## 🔍 **POSSÍVEIS CAUSAS:**

### **1. Z-Order do WBP_CharacterInfo**

O `WBP_EquipmentSlot` está dentro de um `UniformGridPanel` no `WBP_CharacterInfo`. O `WBP_CharacterInfo` pode estar com um Z-Order menor que o tooltip, mas ainda assim bloqueando a visualização.

**VERIFICAÇÃO:**
- Abra `WBP_CharacterInfo` no Designer
- Verifique o **Z-Order** do widget principal
- Se for maior que 999, o tooltip pode estar atrás

**SOLUÇÃO:**
- Aumente o Z-Order do tooltip para um valor maior (ex: 1000 ou 9999)
- Ou verifique se o `WBP_CharacterInfo` tem **"Is Volatile"** ou **"Render Transform"** que pode estar afetando

---

### **2. Clipping do UniformGridPanel**

O `UniformGridPanel` pode estar cortando o tooltip se ele sair dos limites do painel.

**VERIFICAÇÃO:**
- No Designer do `WBP_CharacterInfo`, selecione o `UniformGridPanel` que contém os slots
- Verifique se há **"Clipping"** ativado
- Verifique se há **"Render Transform"** ou **"Clipping Box"**

**SOLUÇÃO:**
- Desative **"Clipping"** no `UniformGridPanel` (se estiver ativado)
- Ou mude o `UniformGridPanel` para um `Canvas Panel` (permite widgets fora dos limites)

---

### **3. OnMouseEnter não está sendo disparado**

O `OnMouseEnter` pode não estar sendo chamado no `WBP_EquipmentSlot` devido a algum widget pai bloqueando os eventos de mouse.

**VERIFICAÇÃO:**
1. Adicione um `Print String` logo no início do `OnMouseEnter` do `WBP_EquipmentSlot`
2. Passe o mouse sobre um slot equipado
3. Se o log não aparecer, o `OnMouseEnter` não está sendo disparado

**SOLUÇÃO:**
- Verifique se há algum widget pai (no `WBP_CharacterInfo`) que está bloqueando eventos de mouse
- Verifique se o `WBP_EquipmentSlot` tem **"Is Enabled"** = true
- Verifique se o `WBP_EquipmentSlot` tem **"Visibility"** = Visible

---

### **4. Widget está sendo criado, mas não visível**

O widget pode estar sendo criado, mas não aparecendo devido a problemas de visibilidade ou posicionamento.

**VERIFICAÇÃO:**
1. Adicione `Print String` após cada etapa:
   - Após `Create Widget`: "Tooltip criado!"
   - Após `Set Tooltip Data`: "Tooltip data setada!"
   - Após `Add To Viewport`: "Tooltip adicionado ao viewport!"
   - Após `Set Position In Viewport`: "Tooltip posicionado!"

2. Adicione um `Print String` com os valores de X e Y após `Make Vector2D`:
   - "Tooltip Position: X=[X], Y=[Y]"

3. Verifique se o `WBP_ItemTooltip` tem **"Visibility"** = Visible no Designer

**SOLUÇÃO:**
- Se os logs aparecerem, mas o tooltip não, pode ser um problema de Z-Order ou clipping
- Se os logs não aparecerem, o problema é na execução do código

---

### **5. WBP_CharacterInfo está bloqueando o tooltip**

O `WBP_CharacterInfo` pode estar com um Z-Order muito alto ou com configurações que bloqueiam widgets filhos.

**VERIFICAÇÃO:**
- No Designer do `WBP_CharacterInfo`, verifique:
  - **Z-Order** do widget principal
  - **"Is Volatile"** (deve ser false)
  - **"Render Transform"** (deve estar vazio)
  - **"Clipping"** (deve ser "Inherit" ou "Clip to Bounds")

**SOLUÇÃO:**
- Se o Z-Order do `WBP_CharacterInfo` for maior que 999, aumente o Z-Order do tooltip
- Se houver problemas de clipping, desative ou ajuste

---

### **6. ItemTooltipWidget está sendo limpo imediatamente**

O `OnMouseLeave` pode estar sendo chamado imediatamente após `OnMouseEnter`, limpando o tooltip antes de aparecer.

**VERIFICAÇÃO:**
1. Adicione um `Print String` no `OnMouseLeave` do `WBP_EquipmentSlot`
2. Se o log aparecer logo após `OnMouseEnter`, o `OnMouseLeave` está sendo chamado muito cedo

**SOLUÇÃO:**
- Adicione um pequeno delay antes de limpar o tooltip no `OnMouseLeave`
- Ou verifique se há algum widget que está interceptando o mouse

---

## ✅ **CHECKLIST DE VERIFICAÇÃO:**

1. [ ] `OnMouseEnter` está sendo disparado? (adicionar `Print String`)
2. [ ] O widget está sendo criado? (adicionar `Print String` após `Create Widget`)
3. [ ] O `Set Tooltip Data` está sendo chamado? (adicionar `Print String`)
4. [ ] O `Add To Viewport` está sendo executado? (adicionar `Print String`)
5. [ ] O `Set Position In Viewport` está recebendo valores válidos? (adicionar `Print String` com X e Y)
6. [ ] O Z-Order do tooltip (999) é maior que o Z-Order do `WBP_CharacterInfo`?
7. [ ] O `UniformGridPanel` no `WBP_CharacterInfo` tem clipping desativado?
8. [ ] O `WBP_ItemTooltip` tem **Visibility** = Visible no Designer?
9. [ ] O `OnMouseLeave` está sendo chamado imediatamente? (adicionar `Print String`)

---

## 🎯 **SOLUÇÃO RÁPIDA (TESTE):**

Tente aumentar o Z-Order do tooltip para **9999** e verifique se aparece:

```
[Add To Viewport]
  ├─ Target: ItemTooltipWidget
  └─ Z Order: 9999  ← Aumentar de 999 para 9999
```

Se ainda não aparecer, o problema é outro (clipping, eventos de mouse, etc.).

---

## 📝 **PRÓXIMOS PASSOS:**

1. Adicione os `Print String` sugeridos
2. Execute o jogo e passe o mouse sobre um slot equipado
3. Envie os logs para análise
4. Verifique as configurações do `WBP_CharacterInfo` e `UniformGridPanel`

