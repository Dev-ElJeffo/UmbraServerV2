# 🔧 CORREÇÃO: Tooltip não aparece no WBP_EquipmentSlot

## 🎯 **PROBLEMA:**

O tooltip não está aparecendo porque está usando `GetMousePosition` do `PlayerController`, que pode não funcionar corretamente para widgets.

## ✅ **SOLUÇÃO:**

Substituir `GetMousePosition` do `PlayerController` por `Get Mouse Position On Viewport` de `UWidgetLayoutLibrary`.

---

## 📋 **CORREÇÃO NO OnMouseEnter:**

### **1. Remover os nós atuais:**

1. **Delete** o nó `Get Player Controller` (K2Node_CallFunction_10)
2. **Delete** o nó `Get Mouse Position` (K2Node_CallFunction_7)

### **2. Adicionar o nó correto:**

1. **Procure por:** `Get Mouse Position On Viewport`
   - **Categoria:** `Widget Layout Library`
   - **Ou digite:** `GetMousePositionOnViewport`

2. **Conecte:**
   - **World Context Object:** Arraste `self` (ou deixe vazio, ele pega automaticamente)
   - **Return Value:** `Vector2D` (já retorna a posição correta)

### **3. Ajustar os nós de adição:**

Os nós `Add` (K2Node_PromotableOperator_2 e K2Node_PromotableOperator_0) devem receber:
- **A:** `X` ou `Y` do `Get Mouse Position On Viewport` (Break Vector2D)
- **B:** `10.0` (Make Literal Float)

---

## 📊 **ESTRUTURA CORRIGIDA:**

```
[OnMouseEnter]
  ↓
[Get Equipped Item]
  ↓
[Break Umbra Inventory Slot]
  └─ Item Template ID
       ↓
[Greater (Int Int)]
  ├─ A: Item Template ID
  ├─ B: 0
  └─ Return Value: (bool)
       ↓
[Branch]
  ├─ True:
  │    ↓
  │  [Create Widget]
  │    ├─ Class: WBP_ItemTooltip
  │    └─ Return Value: (WBP_ItemTooltip)
  │         ↓
  │    [Set ItemTooltipWidget] ← Variável
  │         ↓
  │    [Set Tooltip Data]
  │      ├─ Target: ItemTooltipWidget
  │      └─ In Slot Data: Equipped Item
  │           ↓
  │    [Add To Viewport]
  │      ├─ Target: ItemTooltipWidget
  │      └─ Z Order: 999
  │           ↓
  │    [Get Mouse Position On Viewport] ← NOVO!
  │      ├─ World Context Object: (deixe vazio ou self)
  │      └─ Return Value: (Vector2D)
  │           ↓
  │    [Break Vector2D] ← NOVO!
  │      ├─ X: (float)
  │      └─ Y: (float)
  │           ↓
  │    [Add (Float + Float)] (para X)
  │      ├─ A: X (do Break Vector2D)
  │      ├─ B: 10.0 (Make Literal Float)
  │      └─ Return Value: (float)
  │           ↓
  │    [Add (Float + Float)] (para Y)
  │      ├─ A: Y (do Break Vector2D)
  │      ├─ B: 10.0 (Make Literal Float)
  │      └─ Return Value: (float)
  │           ↓
  │    [Make Vector2D]
  │      ├─ X: Return Value (do Add X)
  │      ├─ Y: Return Value (do Add Y)
  │      └─ Return Value: (Vector2D)
  │           ↓
  │    [Set Position In Viewport]
  │      ├─ Target: ItemTooltipWidget
  │      ├─ Position: Return Value (do Make Vector2D)
  │      └─ b Remove DPIScale: true
  │
  └─ False:
       ↓
  (nada)
```

---

## 🔍 **DETALHES IMPORTANTES:**

### **Get Mouse Position On Viewport:**

- **Localização:** `Widget Layout Library` → `Get Mouse Position On Viewport`
- **Parâmetros:**
  - **World Context Object:** Pode deixar vazio (usa o contexto atual) ou conectar `self`
- **Retorno:** `Vector2D` (X, Y) diretamente, sem precisar de `LocationX` e `LocationY` separados

### **Break Vector2D:**

- **Localização:** `Break Vector2D` (procure por "Break" e selecione `Vector2D`)
- **Conecte:**
  - **Input:** `Return Value` de `Get Mouse Position On Viewport`
  - **Outputs:** `X` e `Y` (floats)

---

## ✅ **VERIFICAÇÕES:**

1. ✅ `Get Mouse Position On Viewport` está conectado corretamente
2. ✅ `Break Vector2D` está recebendo o `Return Value` de `Get Mouse Position On Viewport`
3. ✅ Os dois nós `Add` estão recebendo `X` e `Y` do `Break Vector2D`
4. ✅ `Make Vector2D` está recebendo os resultados dos dois `Add`
5. ✅ `Set Position In Viewport` está recebendo o `Return Value` de `Make Vector2D`

---

## 🎯 **RESULTADO ESPERADO:**

Após a correção, o tooltip deve aparecer **10 pixels** à direita e **10 pixels** abaixo da posição do mouse quando você passar sobre um item equipado.

---

## 📝 **NOTA:**

Se ainda não aparecer, verifique:
1. O `ItemTooltipWidget` está sendo criado? (adicione um `Print String` após `Create Widget`)
2. O `Set Tooltip Data` está sendo chamado? (adicione um `Print String` após `Set Tooltip Data`)
3. O `Add To Viewport` está sendo executado? (adicione um `Print String` após `Add To Viewport`)
4. O `Set Position In Viewport` está recebendo valores válidos? (adicione um `Print String` com os valores de X e Y)

