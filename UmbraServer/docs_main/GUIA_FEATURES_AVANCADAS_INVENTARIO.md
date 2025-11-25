# 🚀 GUIA: FEATURES AVANÇADAS + SISTEMA DE ARMAZÉM

## ✅ **FEATURES IMPLEMENTADAS:**

### **1. Sistema de Stacking (Empilhamento)**
- ✅ `CanStackWith()` - Verifica se pode empilhar
- ✅ `GetMaxStackSize()` - Tamanho máximo da pilha
- ✅ `TryStackItems()` - Empilha automaticamente

### **2. Sistema de Split (Dividir Pilha)**
- ✅ `SplitStack()` - Divide pilha ao meio ou quantidade específica
- ✅ Callbacks `OnStackSplit` e `OnItemsStacked`

### **3. Sistema de Validação de Tipo**
- ✅ `ValidateItemType()` - Valida tipo do item
- ✅ `AcceptedItemTypes` - Lista de tipos aceitos

### **4. Sistema de Armazém (Storage)**
- ✅ `UmbraStorageWidget` - Widget do armazém (100 slots)
- ✅ `UmbraStorageActor` - Actor para interagir
- ✅ Drag & Drop entre inventário e armazém
- ✅ `DepositAll()` e `WithdrawAll()`

---

## 📁 **ARQUIVOS CRIADOS:**

```
UmbraEternumUE/Source/UmbraEternumUE/
├── UI/
│   ├── UmbraInventorySlotWidget.h/.cpp    (ATUALIZADO + Features)
│   ├── UmbraStorageWidget.h/.cpp          (NOVO)
│   └── UmbraItemDragDropOperation.h/.cpp  (Existente)
└── Actors/
    └── UmbraStorageActor.h/.cpp           (NOVO)
```

---

## 🎮 **1. SISTEMA DE STACKING**

### **Como Funciona:**

**Automaticamente empilha itens do mesmo tipo:**

```cpp
// Consumíveis: Max 99
// Recursos/Materiais: Max 999
// Equipamentos: Max 1 (não empilha)
```

### **No Blueprint:**

**OnDrop do WBP_InventorySlot:**

Antes de chamar `Process Item Drop`, o sistema já tenta empilhar automaticamente se:
- ✅ São o mesmo item
- ✅ Slot de destino não está cheio
- ✅ Item é stackable

**Callbacks Opcionais:**

```blueprint
[Event On Items Stacked]
    │ Added Amount (int32)
    │ New Total (int32)
    │
    └─> [Play Sound: Stack]
        └─> [Update Visual]
```

---

## 🔪 **2. SISTEMA DE SPLIT (DIVIDIR PILHA)**

### **Como Usar:**

**Shift + Drag para dividir:**

```blueprint
[On Drag Detected]
    │
    ├─> [Is Shift Pressed?] (Branch)
    │       │
    │       ├─ TRUE ──> [Split Stack]
    │       │               │ Split Amount: 0 (divide ao meio)
    │       │               │ Return: NewSlotData
    │       │               │
    │       │               └─> [Create Item Drag with Split Data]
    │       │
    │       └─ FALSE ─> [Create Item Drag Operation] (normal)
```

**Passo a passo:**

1. Detecta se Shift está pressionado
2. Chama `Split Stack` (C++)
3. Cria drag com os dados divididos
4. Ao dropar, cria novo slot com a quantidade

**Callback Opcional:**

```blueprint
[Event On Stack Split]
    │ Original Amount (int32)
    │ New Amount (int32)
    │
    └─> [Play Sound: Split]
        └─> [Update Visual]
```

---

## 🎯 **3. SISTEMA DE VALIDAÇÃO DE TIPO**

### **Para Slots Específicos:**

**Exemplo: Slot de Arma**

No `WBP_WeaponSlot`:

1. Parent Class: `UmbraInventorySlotWidget`
2. No Designer, configure:
   - `Accepted Item Types`: ["weapon", "sword", "bow"]

**No OnDrop:**

```blueprint
[On Drop]
    │
    ├─> [Get Source Item Type]
    │       │
    │       └─> [Validate Item Type] ◄─── FUNÇÃO C++!
    │               │ Item Type: (do source)
    │               │ Return: bool
    │               │
    │               ├─ TRUE ──> [Process Item Drop]
    │               │
    │               └─ FALSE ─> [On Drop Failed]
    │                           │ Reason: "Este slot só aceita armas"
```

---

## 🏦 **4. SISTEMA DE ARMAZÉM (COMPLETO)**

### **Arquitetura:**

```
Player Inventory (30 slots)
    ↕️ Drag & Drop
Storage/Warehouse (100 slots)
```

### **A. Criar o Widget do Armazém:**

**1. Criar WBP_Storage:**

- File → New → Widget Blueprint
- Nome: `WBP_Storage`
- Parent Class: `UmbraStorageWidget`

**2. Design:**

```
Canvas Panel
├── Border_Background
├── Text_StorageName ("Armazém")
├── Text_Capacity ("50/100")
├── Uniform Grid (10x10 = 100 slots)
│   └── WBP_InventorySlot (x100)
├── Button_DepositAll
├── Button_WithdrawAll
└── Button_Close
```

**3. Event Graph:**

```blueprint
[Event Construct]
    │
    └─> [Load Storage] ◄─── FUNÇÃO C++!
            │ (Carrega dados do armazém)
            │
            └─> [Create 100 Slots]
                └─> [Populate Uniform Grid]
```

**Callbacks Opcionais:**

```blueprint
[Event On Storage Loaded]
    │ Used Slots (int32)
    │ Total Slots (int32)
    │
    └─> [Update Capacity Text]
        │ Text: "{UsedSlots}/{TotalSlots}"

[Event On Item Deposited]
    │ Item Data (UmbraInventorySlot)
    │
    └─> [Play Sound: Deposit]

[Event On Item Withdrawn]
    │ Item Data (UmbraInventorySlot)
    │
    └─> [Play Sound: Withdraw]

[Event On Storage Full]
    │
    └─> [Show Message: "Armazém cheio!"]
```

---

### **B. Criar o Actor do Armazém:**

**1. Criar BP_StorageChest:**

- File → New → Blueprint Class
- Parent: `UmbraStorageActor`
- Nome: `BP_StorageChest`

**2. Configurar Components:**

```
BP_StorageChest
├── MeshComponent (baú, cofre, NPC)
└── InteractionBox (área de interação)
```

**3. Configurar Properties:**

- `Storage Widget Class`: WBP_Storage
- `Storage ID`: 1 (único)
- `Storage Name`: "Banco Central"
- `Max Interaction Distance`: 200
- `Requires Key`: false (ou true + RequiredKeyItemID)

**4. Implementar Eventos:**

```blueprint
[Event On Storage Opened]
    │ Player Controller
    │
    └─> [Play Animation: ChestOpen]
        └─> [Play Sound: ChestOpen]

[Event On Storage Closed]
    │
    └─> [Play Animation: ChestClose]
        └─> [Play Sound: ChestClose]

[Event On Access Denied]
    │ Reason (String)
    │
    └─> [Show Message: Reason]
        └─> [Play Sound: Error]
```

---

### **C. Abrir o Armazém no Mundo:**

**No PlayerController ou Character:**

```blueprint
[Input Action: Interact (E)]
    │
    ├─> [Line Trace]
    │       │ Distance: 200
    │       │
    │       └─> [Cast to UmbraStorageActor]
    │               │
    │               └─> [Open Storage] ◄─── FUNÇÃO C++!
    │                       │ Player Controller: self
    │                       │
    │                       └─> Success!
```

Ou **automaticamente ao entrar na área:**

```blueprint
[On Interaction Box Begin Overlap]
    │
    └─> [Open Storage Automatically]
```

---

### **D. Drag & Drop entre Inventário e Armazém:**

**Mesma lógica do drag normal!**

Os slots do armazém usam `UmbraInventorySlotWidget`, então o drag & drop já funciona automaticamente! 🎉

**Validações automáticas:**
- ✅ Inventário → Armazém (se armazém tem espaço)
- ✅ Armazém → Inventário (se inventário tem espaço)
- ✅ Armazém → Armazém (reorganizar)
- ✅ Inventário → Inventário (reorganizar)

---

### **E. Botões Especiais:**

**Depositar Tudo:**

```blueprint
[Button_DepositAll: OnClicked]
    │
    └─> [Deposit All] ◄─── FUNÇÃO C++!
            │ Return: ItemsDeposited (int32)
            │
            └─> [Show Message: "{ItemsDeposited} itens depositados"]
```

**Retirar Tudo:**

```blueprint
[Button_WithdrawAll: OnClicked]
    │
    └─> [Withdraw All] ◄─── FUNÇÃO C++!
            │ Return: ItemsWithdrawn (int32)
            │
            └─> [Show Message: "{ItemsWithdrawn} itens retirados"]
```

---

## 📊 **RESUMO DAS FUNÇÕES C++:**

### **UmbraInventorySlotWidget:**

| Função | Descrição | Quando Usar |
|--------|-----------|-------------|
| `CanStackWith()` | Verifica se pode empilhar | OnDrop (automático) |
| `GetMaxStackSize()` | Tamanho máximo da pilha | Validação |
| `SplitStack()` | Divide pilha | Shift + Drag |
| `ValidateItemType()` | Valida tipo | Slots específicos |
| `TryStackItems()` | Empilha itens | OnDrop (interno) |

### **UmbraStorageWidget:**

| Função | Descrição | Quando Usar |
|--------|-----------|-------------|
| `LoadStorage()` | Carrega armazém | Event Construct |
| `MoveItemToStorage()` | Move para armazém | Drag & Drop |
| `MoveItemFromStorage()` | Move do armazém | Drag & Drop |
| `DepositAll()` | Deposita tudo | Botão |
| `WithdrawAll()` | Retira tudo | Botão |
| `GetUsedSlots()` | Slots ocupados | UI |
| `GetFreeSlots()` | Slots livres | UI |
| `IsFull()` | Está cheio? | Validação |

### **UmbraStorageActor:**

| Função | Descrição | Quando Usar |
|--------|-----------|-------------|
| `OpenStorage()` | Abre armazém | Interação |
| `CloseStorage()` | Fecha armazém | Botão Close |
| `CanPlayerOpenStorage()` | Pode abrir? | Validação |
| `HasRequiredKey()` | Tem key? | Validação |

---

## 🔥 **COMPILAR:**

```powershell
cd D:\UmbraServerV2\UmbraEternumUE
.\compile_drag_drop.bat
```

---

## ✅ **CHECKLIST COMPLETO:**

### **Stacking:**
- [ ] Itens do mesmo tipo empilham automaticamente
- [ ] Max stack correto (consumíveis: 99, recursos: 999)
- [ ] Callbacks visuais funcionando

### **Split:**
- [ ] Shift + Drag divide pilha
- [ ] Quantidade dividida correta
- [ ] Visual atualiza

### **Validação:**
- [ ] Slots específicos só aceitam tipos corretos
- [ ] Mensagem de erro ao tentar dropar tipo errado

### **Armazém:**
- [ ] WBP_Storage criado (100 slots)
- [ ] BP_StorageChest criado e configurado
- [ ] Armazém abre ao interagir
- [ ] Drag & Drop funciona entre inventário e armazém
- [ ] Depositar Tudo funciona
- [ ] Retirar Tudo funciona
- [ ] UI mostra capacidade (50/100)
- [ ] Fecha automaticamente ao se afastar

---

## 🎯 **RESULTADO FINAL:**

Com todas essas features, você terá:

✅ **Stacking automático** (itens se empilham)
✅ **Split de pilhas** (Shift + Drag)
✅ **Slots específicos** (arma, armadura, etc)
✅ **Armazém de 100 slots** (banco seguro)
✅ **Drag & Drop universal** (funciona em tudo)
✅ **Botões especiais** (Depositar/Retirar Tudo)
✅ **Validações robustas** (menos bugs)
✅ **Performance otimizada** (tudo em C++)

**SEU SISTEMA DE INVENTÁRIO ESTARÁ COMPLETO E PROFISSIONAL! 🎮✨**

---

## 📚 **PRÓXIMOS PASSOS:**

1. Compile o C++
2. Crie WBP_Storage
3. Crie BP_StorageChest
4. Coloque no mundo
5. Teste! 🎮

**Veja também:**
- `GUIA_CPP_DRAG_DROP_INVENTARIO.md` - Drag do inventário
- `GUIA_CPP_DRAG_DROP_ITENS.md` - Drag de itens
- `RESUMO_SOLUCAO_CPP_COMPLETA.md` - Visão geral

---

**SISTEMA DE INVENTÁRIO COMPLETO! 🚀🔥**

