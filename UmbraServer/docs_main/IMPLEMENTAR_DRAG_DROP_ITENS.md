# 🎯 IMPLEMENTAR DRAG & DROP DE ITENS

**OBJETIVO:** Fazer os itens arrastarem quando clicados, sem arrastar o inventário inteiro.

---

## ✅ **O QUE JÁ ESTÁ FEITO:**

1. ✅ **C++ ignora cliques em slots** - Inventário só arrasta em áreas vazias
2. ✅ **Função `CreateItemDragOperation`** - Já existe no C++
3. ✅ **Função `RequestMoveItem`** - Já existe no C++

---

## 🔧 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **1. NO WBP_InventorySlot - Event Graph:**

#### **A. Override `On Mouse Button Down`:**

```
On Mouse Button Down
  ↓
Get Slot Data (self)
  ↓
Break Umbra Inventory Slot
  ↓
Branch (Inventory ID > 0?)  ← Só arrasta se TEM item!
  ├─ TRUE:
  │   └─ Detect Drag If Pressed
  │         ├─ Pointer Event: (do On Mouse Button Down)
  │         ├─ Widget Reference: self
  │         └─ Drag Key: Left Mouse Button
  │       → Return Value: Conecte ao RETURN NODE
  │       → IMPORTANTE: O Return Node deve retornar HANDLED!
  │
  └─ FALSE:
      └─ Unhandled (Return Node vazio)  ← Permite que o pai capture
```

**CRÍTICO:** O `Return Value` do `Return Node` deve ser **HANDLED** (não Unhandled) quando detecta drag!

#### **B. Override `On Drag Detected`:**

```
On Drag Detected
  ↓
Create Item Drag Operation (self)  ← Função C++
  ↓
Return (Operation): Conecte a saída ao Return Node
```

#### **C. Override `On Drop`:**

```
On Drop
  ↓
Cast to Umbra Item Drag Drop Operation (do Operation)
  ↓
Branch (Cast sucesso?)
  ├─ TRUE:
  │   ├─ Get Dragged Slot Index (do Cast)
  │   ├─ Get Slot Index (self)
  │   ├─ Request Move Item (self)
  │   │     ├─ Source Slot Index: Dragged Slot Index
  │   │     └─ Target Slot Index: Slot Index
  │   └─ Return TRUE (handled)
  │
  └─ FALSE:
      └─ Return FALSE (unhandled)
```

---

## 📋 **CHECKLIST:**

- [ ] WBP_InventorySlot tem `On Mouse Button Down` override
- [ ] Verifica `Inventory ID > 0` antes de arrastar
- [ ] WBP_InventorySlot tem `On Drag Detected` override
- [ ] Chama `Create Item Drag Operation`
- [ ] WBP_InventorySlot tem `On Drop` override
- [ ] Faz Cast para `Umbra Item Drag Drop Operation`
- [ ] Chama `Request Move Item` com índices corretos
- [ ] Compilou sem erros
- [ ] Testou: Clicar em item = arrasta item
- [ ] Testou: Clicar em área vazia = arrasta inventário

---

## 🎮 **RESULTADO ESPERADO:**

✅ **Clicar em item com mouse** → Arrasta o item  
✅ **Clicar em área vazia** → Arrasta o inventário inteiro  
✅ **Soltar item em outro slot** → Move o item

---

**IMPLEMENTE OS 3 OVERRIDES NO WBP_InventorySlot!** 🚀

