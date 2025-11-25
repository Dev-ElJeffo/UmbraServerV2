# 🏦 REFERÊNCIA RÁPIDA: SISTEMA DE ARMAZÉM

## ⚡ **START RÁPIDO (5 minutos):**

### **1. Compilar C++:**
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
.\compile_drag_drop.bat
```

### **2. Criar Widget:**
1. New → Widget Blueprint
2. Nome: `WBP_Storage`
3. Parent: `UmbraStorageWidget`
4. Design: Uniform Grid 10x10

### **3. Criar Actor:**
1. New → Blueprint Class
2. Parent: `UmbraStorageActor`
3. Nome: `BP_StorageChest`
4. Storage Widget Class: WBP_Storage

### **4. Colocar no Mundo:**
1. Arraste BP_StorageChest para o level
2. Pressione E para interagir
3. **PRONTO!** ✅

---

## 📊 **FUNÇÕES C++ PRINCIPAIS:**

### **UmbraStorageWidget:**

```cpp
// Carrega o armazém
LoadStorage()

// Move item do inventário para armazém
MoveItemToStorage(InventoryID, SlotIndex)

// Move item do armazém para inventário
MoveItemFromStorage(StorageID, SlotIndex)

// Deposita todos os itens do inventário
DepositAll() → int32 (quantidade depositada)

// Retira todos os itens do armazém
WithdrawAll() → int32 (quantidade retirada)

// Obtém slots ocupados
GetUsedSlots() → int32

// Obtém slots livres
GetFreeSlots() → int32

// Verifica se está cheio
IsFull() → bool
```

### **UmbraStorageActor:**

```cpp
// Abre o armazém para um jogador
OpenStorage(PlayerController) → bool

// Fecha o armazém
CloseStorage()

// Verifica se pode abrir
CanPlayerOpenStorage(PlayerController) → bool

// Verifica se tem a chave necessária
HasRequiredKey(PlayerController) → bool
```

---

## 🎮 **BLUEPRINT EVENT GRAPH:**

### **WBP_Storage:**

```blueprint
[Event Construct]
    │
    └─> [Load Storage]
            │
            └─> [Create 100 Slots]
                    │
                    └─> [For Loop: 0 to 99]
                            │
                            └─> [Create WBP_InventorySlot]
                                    │
                                    └─> [Add Child to Uniform Grid]
```

### **BP_StorageChest:**

```blueprint
[Input Action: Interact]
    │
    └─> [Open Storage]
            │ Player Controller: self
            │
            └─> Success!

// OU Automático:

[On Interaction Box Begin Overlap]
    │
    └─> [Open Storage]
```

---

## 🎯 **CALLBACKS OPCIONAIS:**

### **WBP_Storage:**

```blueprint
[Event On Storage Loaded]
    │ Used Slots (int32)
    │ Total Slots (int32)
    │
    └─> [Update UI Text]
        │ "{UsedSlots}/{TotalSlots}"

[Event On Item Deposited]
    │ Item Data (struct)
    │
    └─> [Play Sound: Deposit]
        └─> [Spawn Particle: Gold Sparkle]

[Event On Item Withdrawn]
    │ Item Data (struct)
    │
    └─> [Play Sound: Withdraw]
        └─> [Spawn Particle: Item Poof]

[Event On Storage Full]
    │
    └─> [Show Message: "Armazém cheio!"]
        └─> [Play Sound: Error]
```

### **BP_StorageChest:**

```blueprint
[Event On Storage Opened]
    │ Player Controller
    │
    └─> [Play Animation: ChestOpen]
        └─> [Play Sound: ChestCreak]

[Event On Storage Closed]
    │
    └─> [Play Animation: ChestClose]
        └─> [Play Sound: ChestClose]

[Event On Access Denied]
    │ Reason (String)
    │
    └─> [Show Message: Reason]
        └─> [Play Sound: Locked]
```

---

## 🔧 **CONFIGURAÇÕES:**

### **UmbraStorageWidget:**
- `Max Storage Slots`: 100 (padrão)

### **UmbraStorageActor:**
- `Storage Widget Class`: WBP_Storage
- `Storage ID`: 1 (único)
- `Storage Name`: "Armazém"
- `Max Interaction Distance`: 200
- `Requires Key`: false
- `Required Key Item ID`: 0

---

## ⚡ **BOTÕES ESPECIAIS:**

### **Depositar Tudo:**

```blueprint
[Button_DepositAll: OnClicked]
    │
    └─> [Deposit All]
            │ Return: ItemsDeposited
            │
            └─> [Show Message]
                │ Text: "{ItemsDeposited} itens depositados"
```

### **Retirar Tudo:**

```blueprint
[Button_WithdrawAll: OnClicked]
    │
    └─> [Withdraw All]
            │ Return: ItemsWithdrawn
            │
            └─> [Show Message]
                │ Text: "{ItemsWithdrawn} itens retirados"
```

---

## 🔥 **FEATURES:**

✅ **100 Slots** de armazenamento
✅ **Drag & Drop** funciona automaticamente
✅ **Empilhamento** automático
✅ **Dividir Pilhas** (Shift + Drag)
✅ **Depositar/Retirar Tudo** com 1 clique
✅ **Sistema de Chave** opcional
✅ **Fecha automaticamente** ao se afastar
✅ **UI de Capacidade** (50/100)

---

## 🐛 **TROUBLESHOOTING:**

### **Armazém não abre:**
- Verifique se `Storage Widget Class` está configurado
- Verifique se o jogador está dentro da `Interaction Box`
- Verifique se `Can Player Open Storage` retorna true

### **Drag & Drop não funciona:**
- Verifique se `WBP_InventorySlot` tem Parent: `UmbraInventorySlotWidget`
- Verifique se compilou o C++

### **Itens não aparecem:**
- Verifique se `Load Storage` foi chamado
- Verifique os logs: `LogTemp`

---

## 📚 **DOCUMENTAÇÃO COMPLETA:**

Veja: `GUIA_FEATURES_AVANCADAS_INVENTARIO.md`

---

## ✅ **CHECKLIST:**

- [ ] C++ compilado
- [ ] WBP_Storage criado
- [ ] BP_StorageChest criado
- [ ] Colocado no mundo
- [ ] Abre ao interagir
- [ ] Drag & Drop funciona
- [ ] Depositar Tudo funciona
- [ ] Retirar Tudo funciona
- [ ] UI mostra capacidade
- [ ] Fecha ao se afastar

---

**ARMAZÉM PRONTO! 🏦✨**

**Tempo de implementação: ~5 minutos** ⚡

