# ✅ SOLUÇÃO SIMPLES - OnDrop Storage

## 🎯 **LÓGICA SIMPLIFICADA:**

Em vez de verificar `SlotIndex < 50` ou `>= 50`, vamos usar `ParentStorageWidget`:
- Se `ParentStorageWidget` é válido (não é None) → o slot pertence ao **armazém**
- Se `ParentStorageWidget` é None → o slot pertence ao **inventário**

---

## 📋 **ESTRUTURA DO ONDROP (SIMPLIFICADA):**

```
OnDrop Entry
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → IsValid (Cast) → Branch
  └─ CastFailed → Return false
  ↓
Get Source Slot Widget
  ↓
IsValid (Source Slot Widget) → Branch
  ├─ TRUE → [LÓGICA PRINCIPAL]
  └─ FALSE → Return false
  ↓
[LÓGICA PRINCIPAL - 3 CASOS SIMPLES]
```

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Obter Source Slot Widget**

1. Adicione o nó **"Get Source Slot Widget"**
   - **Target:** Conecte ao **Cast (As Umbra Item Drag Drop Operation)**
   - **Return Value:** Vai ser usado nas próximas etapas

---

### **PASSO 2: Verificar de onde vem o item (ORIGEM)**

1. Adicione **"Get ParentStorageWidget"**
   - **Target:** Conecte ao **Return Value** de "Get Source Slot Widget"
   - **Return Value:** Vai indicar se o item vem do armazém ou inventário

2. Adicione **"Is Valid?"**
   - **Object:** Conecte ao **Return Value** de "Get ParentStorageWidget" (do Source)
   - **Return Value:** TRUE = vem do armazém, FALSE = vem do inventário

3. Adicione um **Branch**
   - **Condition:** Conecte ao **Return Value** de "Is Valid?"
   - **then:** Item vem do ARMAZÉM
   - **else:** Item vem do INVENTÁRIO

---

### **PASSO 3: Verificar para onde vai o item (DESTINO)**

1. Adicione **"Get ParentStorageWidget"** (outro nó)
   - **Target:** Conecte a **self** (o slot de destino)
   - **Return Value:** Vai indicar se o destino é armazém ou inventário

2. Adicione **"Is Valid?"** (outro nó)
   - **Object:** Conecte ao **Return Value** de "Get ParentStorageWidget" (do self)
   - **Return Value:** TRUE = destino é armazém, FALSE = destino é inventário

3. Adicione um **Branch**
   - **Condition:** Conecte ao **Return Value** de "Is Valid?" (do self)
   - **then:** Destino é ARMAZÉM
   - **else:** Destino é INVENTÁRIO

---

### **PASSO 4: Combinar as verificações (4 CASOS)**

Agora você tem 2 Branches:
- **Branch 1:** Origem (armazém ou inventário)
- **Branch 2:** Destino (armazém ou inventário)

Isso cria 4 caminhos possíveis:

#### **CASO 1: Inventário → Inventário**
- **Branch 1:** else (origem = inventário)
- **Branch 2:** else (destino = inventário)
- **Ação:** Use **"Process Item Drop"**
  - **Target:** self
  - **DraggedSlotWidget:** Get Source Slot Widget (Return Value)

#### **CASO 2: Inventário → Armazém**
- **Branch 1:** else (origem = inventário)
- **Branch 2:** then (destino = armazém)
- **Ação:** Use **"Move Item To Storage"**
  - **Target:** Get ParentStorageWidget (do self) → Return Value
  - **InventoryItemID:** Get Slot Data (Source) → Break → InventoryID
  - **TargetSlotIndex:** Get Slot Data (self) → Break → SlotIndex

#### **CASO 3: Armazém → Inventário**
- **Branch 1:** then (origem = armazém)
- **Branch 2:** else (destino = inventário)
- **Ação:** Use **"Move Item From Storage"**
  - **Target:** Get ParentStorageWidget (do Source) → Return Value
  - **StorageItemID:** Get Slot Data (Source) → Break → InventoryID
  - **TargetSlotIndex:** Get Slot Data (self) → Break → SlotIndex

#### **CASO 4: Armazém → Armazém**
- **Branch 1:** then (origem = armazém)
- **Branch 2:** then (destino = armazém)
- **Ação:** Use **"Process Item Drop"**
  - **Target:** self
  - **DraggedSlotWidget:** Get Source Slot Widget (Return Value)

---

## 🎨 **ESTRUTURA VISUAL SIMPLIFICADA:**

```
OnDrop
  ↓
Cast → IsValid → Branch
  ↓
Get Source Slot Widget
  ↓
[VERIFICAR ORIGEM]
Get ParentStorageWidget (Source) → Is Valid? → Branch
  ├─ then: Origem = ARMAZÉM
  └─ else: Origem = INVENTÁRIO
  ↓
[VERIFICAR DESTINO]
Get ParentStorageWidget (self) → Is Valid? → Branch
  ├─ then: Destino = ARMAZÉM
  └─ else: Destino = INVENTÁRIO
  ↓
[4 CAMINHOS]
  ├─ Origem=INV, Destino=INV → Process Item Drop
  ├─ Origem=INV, Destino=ARM → Move Item To Storage
  ├─ Origem=ARM, Destino=INV → Move Item From Storage
  └─ Origem=ARM, Destino=ARM → Process Item Drop
```

---

## ✅ **VANTAGENS DESTA ABORDAGEM:**

1. ✅ **Não precisa verificar SlotIndex** com < 50 ou >= 50
2. ✅ **Lógica clara:** Usa `ParentStorageWidget` para identificar origem e destino
3. ✅ **Fácil de entender:** 2 verificações simples (origem e destino)
4. ✅ **4 casos claros:** Cada combinação tem uma ação específica

---

## 📝 **RESUMO:**

1. Verifique se **origem** tem `ParentStorageWidget` válido
2. Verifique se **destino** tem `ParentStorageWidget` válido
3. Use os 4 caminhos para decidir qual função chamar

**MUITO MAIS SIMPLES QUE VERIFICAR SlotIndex!**

