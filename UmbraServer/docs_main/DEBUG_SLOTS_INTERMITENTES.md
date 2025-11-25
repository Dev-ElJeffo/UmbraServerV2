# 🔍 DEBUG: Slots Intermitentes - Diagnóstico Real

**PROBLEMA:**
- Slots vazios eram brancos, agora estão cinza
- Funcionamento intermitente
- Alguns itens entram, outros não

---

## 🔍 **ADICIONE ESTES LOGS NO `OnInventoryLoaded_Event`:**

### **1. No início do For Loop:**

```
For Loop (0-49)
  Loop Body:
    ├─ Print String: "=== Slot X ===" (X = Index)
    ├─ Get Game Instance...
    ├─ Get Inventory Slot By Index
    │     └─ Return Value → Print String: "bFound: X" (X = ReturnValue)
    │     └─ Out Slot → Break Umbra Inventory Slot
    │         └─ Inventory ID → Print String: "InventoryID: X" (X = InventoryID)
    │         └─ Slot Index → Print String: "SlotIndex: X" (X = SlotIndex)
```

### **2. No Branch (TRUE):**

```
Branch (TRUE)
  ├─ Print String: "Caminho TRUE - Slot tem item"
  ├─ Get Array Item (SlotWidgets, Index)
  ├─ Is Valid? (do Array Item)
  │   ├─ FALSE: Print String: "ERRO: SlotWidget é NULL!"
  │   └─ TRUE:
  │       ├─ Print String: "SlotWidget válido, atualizando..."
  │       ├─ Set Slot Data...
  │       └─ Update Slot Visual...
  │           └─ Print String: "UpdateSlotVisual chamado"
```

### **3. No Branch (FALSE):**

```
Branch (FALSE)
  ├─ Print String: "Caminho FALSE - Slot vazio"
  ├─ Get Array Item (SlotWidgets, Index)
  ├─ Is Valid? (do Array Item)
  │   ├─ FALSE: Print String: "ERRO: SlotWidget é NULL!"
  │   └─ TRUE:
  │       ├─ Print String: "SlotWidget válido, limpando..."
  │       ├─ Make Umbra Inventory Slot...
  │       ├─ Set Slot Data...
  │       └─ Update Slot Visual...
  │           └─ Print String: "UpdateSlotVisual chamado (vazio)"
```

---

## 📋 **O QUE VERIFICAR NOS LOGS:**

1. **Todos os 50 slots estão sendo processados?**
   - Deve ver "=== Slot 0 ===" até "=== Slot 49 ==="

2. **`bFound` está correto?**
   - Slots com itens devem mostrar `bFound: true`
   - Slots vazios devem mostrar `bFound: false`

3. **`SlotWidget` é válido?**
   - Se aparecer "ERRO: SlotWidget é NULL!", esse é o problema

4. **`UpdateSlotVisual` está sendo chamado?**
   - Deve ver "UpdateSlotVisual chamado" para cada slot

5. **Dados estão corretos?**
   - `InventoryID` deve ser > 0 para slots com itens
   - `SlotIndex` deve corresponder ao índice do loop

---

## 🎯 **ENVIE OS LOGS:**

Depois de adicionar os logs, execute o jogo e mova um item. Envie:
1. Os logs do console (especialmente quando move um item)
2. Qual slot específico está falhando (se houver)
3. O que aparece nos logs para os slots que não funcionam

Com esses logs, vou identificar exatamente onde está o problema.

---

**ADICIONE OS LOGS E ENVIE O RESULTADO!** 🔍

