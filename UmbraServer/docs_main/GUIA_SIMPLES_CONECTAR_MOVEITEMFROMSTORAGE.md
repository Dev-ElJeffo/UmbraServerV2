# 🔧 GUIA SIMPLES: Conectar ParentStorageWidget ao MoveItemFromStorage

## 🎯 **O QUE PRECISA SER FEITO:**

Conectar o `ParentStorageWidget` do slot de **ORIGEM** ao `Target` (self) do `MoveItemFromStorage`.

---

## 📍 **ONDE ADICIONAR:**

No Blueprint `WBP_InventorySlot`, no evento `OnDrop`, na seção **"ORIGIN = STORAGE"**.

**Localização exata:** Após o nó `Branch` (Storage ID > 0?), antes do nó `MoveItemFromStorage`.

---

## 🔨 **PASSO A PASSO:**

### **PASSO 1: Encontrar o nó "Get Source Slot Widget"**

1. No Blueprint `OnDrop`, procure pelo nó chamado **"Get Source Slot Widget"**
2. Este nó retorna o widget do slot de origem (o slot de onde você está arrastando o item)

---

### **PASSO 2: Criar o nó "Get Parent Storage Widget"**

1. **Clique com o botão direito** no espaço vazio do Blueprint (próximo ao `MoveItemFromStorage`)
2. **Digite:** `Get Parent Storage Widget`
3. **Selecione:** `Get Parent Storage Widget` (de `Umbra Inventory Slot Widget`)
4. Um novo nó aparecerá com:
   - **Target** (pino azul à esquerda)
   - **Return Value** (pino azul à direita)

---

### **PASSO 3: Conectar "Get Source Slot Widget" ao "Get Parent Storage Widget"**

1. Encontre o nó **"Get Source Slot Widget"**
2. Arraste o pino **"Return Value"** (azul, à direita) do **"Get Source Slot Widget"**
3. Conecte ao pino **"Target"** (azul, à esquerda) do **"Get Parent Storage Widget"**

**Visual:**
```
[Get Source Slot Widget] → Return Value (azul) → Target (azul) → [Get Parent Storage Widget]
```

---

### **PASSO 4: Conectar "Get Parent Storage Widget" ao "MoveItemFromStorage"**

1. Encontre o nó **"MoveItemFromStorage"** (ou **"Move Item From Storage"**)
2. Este nó tem um pino **"Target"** (ou **"self"**) na parte superior esquerda (azul) - **ESTÁ VAZIO**
3. Arraste o pino **"Return Value"** (azul, à direita) do **"Get Parent Storage Widget"**
4. Conecte ao pino **"Target"** (azul, superior esquerda) do **"MoveItemFromStorage"**

**Visual:**
```
[Get Parent Storage Widget] → Return Value (azul) → Target (azul) → [MoveItemFromStorage]
```

---

## 📊 **DIAGRAMA VISUAL COMPLETO:**

```
┌─────────────────────────┐
│ Get Source Slot Widget  │
│                         │
│  Return Value (azul) ────┼──┐
└─────────────────────────┘  │
                              │
                              ▼
┌─────────────────────────┐  │
│ Get Parent Storage      │  │
│ Widget                  │  │
│                         │  │
│  Target (azul) ←────────┘  │
│  Return Value (azul) ────┐  │
└─────────────────────────┘  │
                              │
                              ▼
┌─────────────────────────┐  │
│ MoveItemFromStorage     │  │
│                         │  │
│  Target (azul) ←────────┘  │
│  StorageItemID           │
│  TargetSlotIndex          │
└─────────────────────────┘
```

---

## ✅ **VERIFICAÇÃO:**

Após fazer as conexões, verifique se:

1. ✅ O nó **"Get Parent Storage Widget"** existe
2. ✅ O **"Target"** do **"Get Parent Storage Widget"** está conectado ao **"Return Value"** do **"Get Source Slot Widget"**
3. ✅ O **"Return Value"** do **"Get Parent Storage Widget"** está conectado ao **"Target"** do **"MoveItemFromStorage"**
4. ✅ O **"MoveItemFromStorage"** tem o **"execute"** conectado ao **"then"** do **"Branch"** (Storage ID > 0?)

---

## 🎯 **RESUMO EM 3 PASSOS:**

1. **Criar** o nó `Get Parent Storage Widget`
2. **Conectar** `Get Source Slot Widget` → `Get Parent Storage Widget` (Return Value → Target)
3. **Conectar** `Get Parent Storage Widget` → `MoveItemFromStorage` (Return Value → Target)

---

**Pronto! Com essas conexões, o MoveItemFromStorage saberá qual widget de storage chamar.** 🚀

