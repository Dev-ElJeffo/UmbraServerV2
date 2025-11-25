# 🏗️ Arquitetura do Sistema de Inventário no Unreal Engine

**Data**: 14/11/2024  
**Status**: APIs funcionais ✅ | C++ compilado ✅ | Blueprint UI ⏳

---

## 📊 Visão Geral da Arquitetura

```
┌─────────────────────────────────────────────────────────────────┐
│                    ARQUITETURA DO INVENTÁRIO                     │
└─────────────────────────────────────────────────────────────────┘

┌──────────────────────┐
│   MySQL Database     │ ← Armazena itens persistentes
└──────────┬───────────┘
           │
           ↓
┌──────────────────────┐
│   PHP REST API       │ ← GET/POST endpoints (6 APIs)
└──────────┬───────────┘
           │ HTTP Requests
           ↓
┌────────────────────────────────────────────────────────────────┐
│  UmbraGameInstance (C++)                                       │
│  - Singleton global (1 instância por sessão)                   │
│  - Gerencia estado do inventário na memória                    │
│  - Faz requisições HTTP via VaRest                             │
│  - Dispara Delegates quando inventário muda                    │
│  ✅ JÁ IMPLEMENTADO E COMPILADO                                │
└────────────┬───────────────────────────────────────────────────┘
             │ Delegates (eventos)
             ↓
┌────────────────────────────────────────────────────────────────┐
│  Widget Blueprints (UI)                                        │
│  ├─ WBP_InventorySlot (1 item individual)                     │
│  └─ WBP_Inventory (grid 5x10 = 50 slots)                      │
│  - Escuta Delegates do GameInstance                           │
│  - Atualiza visualmente quando inventário muda                │
│  - Permite drag & drop                                         │
│  ⏳ PRECISA CRIAR (próximo passo)                              │
└────────────┬───────────────────────────────────────────────────┘
             │
             ↓
┌────────────────────────────────────────────────────────────────┐
│  Character Blueprint (BP_ThirdPersonCharacter)                 │
│  - Cria e controla o Widget de Inventário                     │
│  - Input Action (tecla "I") para abrir/fechar                 │
│  - Chama funções do GameInstance (LoadInventory, etc)         │
│  ⏳ PRECISA CONFIGURAR                                         │
└────────────────────────────────────────────────────────────────┘
```

---

## 🎯 Componentes do Sistema

### 1️⃣ **UmbraGameInstance (C++)** ✅ JÁ EXISTE

**O que é**: Classe C++ singleton que existe **durante toda a sessão do jogo**.

**Onde está**:
- Código: `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h/.cpp`
- Configurado em: `Project Settings > Maps & Modes > Game Instance Class`

**Características**:
- ✅ **Singleton global** - Existe 1 única instância por sessão
- ✅ **Persiste entre levels** - Não é destruído ao trocar de mapa
- ✅ **Não vai no level** - É gerenciado automaticamente pela engine
- ✅ **Acessível de qualquer Blueprint** via `Get Game Instance`

**Funções principais** (já implementadas):
```cpp
// Carregar inventário do servidor
UFUNCTION(BlueprintCallable)
void LoadInventory();

// Adicionar item
UFUNCTION(BlueprintCallable)
void AddItem(int32 ItemTemplateID, int32 Quantity, int32 SlotIndex);

// Equipar item
UFUNCTION(BlueprintCallable)
void EquipItem(int32 InventoryID);

// Query: obter item de um slot
UFUNCTION(BlueprintPure)
FUmbraInventorySlot GetInventorySlotByIndex(int32 SlotIndex);
```

**Delegates** (eventos):
```cpp
// Dispara quando inventário é carregado
UPROPERTY(BlueprintAssignable)
FOnInventoryLoaded OnInventoryLoaded;

// Dispara quando item é adicionado
UPROPERTY(BlueprintAssignable)
FOnItemAdded OnItemAdded;

// Dispara quando item é equipado
UPROPERTY(BlueprintAssignable)
FOnItemEquipped OnItemEquipped;
```

**❓ Precisa criar BP_UmbraGameInstance?**
- ❌ **NÃO!** A classe C++ já é suficiente
- ✅ Ela já está configurada no projeto
- ✅ Acesse via `Get Game Instance > Cast To UmbraGameInstance`

---

### 2️⃣ **Widget Blueprints (UI)** ⏳ PRECISA CRIAR

**O que são**: Interfaces visuais que o jogador vê e interage.

**Onde criar**: `Content Browser > Right Click > User Interface > Widget Blueprint`

#### 📦 **WBP_InventorySlot** (1 slot individual)

**Função**: Representa **1 item** visual no inventário.

**Estrutura**:
```
WBP_InventorySlot
├─ Border (fundo do slot)
│  ├─ Image (ícone do item)
│  ├─ TextBlock (quantidade "x20")
│  ├─ ProgressBar (durabilidade)
│  └─ Border (borda colorida por raridade)
```

**Variáveis**:
```cpp
// Dados do slot
UPROPERTY(BlueprintReadWrite)
FUmbraInventorySlot SlotData;

// Índice do slot (0-49)
UPROPERTY(BlueprintReadWrite)
int32 SlotIndex;
```

**Eventos**:
- `On Mouse Button Down` → Iniciar drag
- `On Drop` → Receber item arrastado
- `On Mouse Enter` → Mostrar tooltip

---

#### 🎒 **WBP_Inventory** (Grid completo)

**Função**: Gerencia todos os 50 slots + lógica de inventário.

**Estrutura**:
```
WBP_Inventory (Canvas Panel)
├─ Border (fundo escuro semi-transparente)
│  └─ VerticalBox
│     ├─ TextBlock "Inventário" (título)
│     ├─ UniformGridPanel (5 colunas × 10 linhas = 50 slots)
│     │  ├─ WBP_InventorySlot (slot 0)
│     │  ├─ WBP_InventorySlot (slot 1)
│     │  ├─ ... (48 slots)
│     │  └─ WBP_InventorySlot (slot 49)
│     └─ TextBlock (info: peso, slots usados, etc)
```

**Eventos importantes**:
1. **Construct** (quando widget é criado):
   ```cpp
   Event Construct
   ├─ Get Game Instance
   ├─ Cast to UmbraGameInstance
   ├─ Bind Event "OnInventoryLoaded"
   ├─ Bind Event "OnItemAdded"
   ├─ Bind Event "OnItemEquipped"
   └─ Call "LoadInventory" (carregar do servidor)
   ```

2. **OnInventoryLoaded** (quando dados chegam):
   ```cpp
   OnInventoryLoaded
   └─ For Each Slot (0 a 49)
      ├─ Get Inventory Slot By Index
      └─ Update Visual (ícone, quantidade, durabilidade)
   ```

3. **OnItemAdded** (quando novo item é adicionado):
   ```cpp
   OnItemAdded (SlotIndex)
   └─ Update Slot Visual (SlotIndex)
   ```

---

### 3️⃣ **Character Blueprint** ⏳ PRECISA CONFIGURAR

**Onde está**: `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`

**O que adicionar**:

#### A) **Variável para armazenar o Widget**:
```cpp
UPROPERTY(BlueprintReadWrite)
UUserWidget* InventoryWidget;

UPROPERTY(BlueprintReadOnly)
bool bIsInventoryOpen = false;
```

#### B) **Input Action** (tecla "I"):
```cpp
Input Action "OpenInventory" (Key: I)
├─ Branch (Is Inventory Open?)
│  ├─ True:
│  │  ├─ Remove From Parent (InventoryWidget)
│  │  ├─ Set Input Mode Game Only
│  │  ├─ Set Show Mouse Cursor = False
│  │  └─ Set bIsInventoryOpen = False
│  └─ False:
│     ├─ Create Widget (WBP_Inventory)
│     ├─ Add to Viewport
│     ├─ Set Input Mode UI Only
│     ├─ Set Show Mouse Cursor = True
│     └─ Set bIsInventoryOpen = True
```

#### C) **BeginPlay** (opcional - carregar inventário ao spawnar):
```cpp
Event BeginPlay
├─ Delay (2 segundos) // Esperar conexão WebSocket
├─ Get Game Instance
├─ Cast to UmbraGameInstance
└─ Call "LoadInventory"
```

---

## 🗂️ Estrutura de Pastas Recomendada

```
Content/
├─ Blueprints/
│  └─ BP_ThirdPersonCharacter (já existe)
│
├─ UI/
│  ├─ Inventory/
│  │  ├─ WBP_Inventory          ← Widget principal
│  │  ├─ WBP_InventorySlot      ← Widget de 1 slot
│  │  └─ WBP_ItemTooltip        ← Tooltip ao passar mouse
│  │
│  └─ Textures/
│     ├─ T_SlotBackground       ← Textura do fundo do slot
│     ├─ T_SlotBorder_Common    ← Borda verde para comum
│     ├─ T_SlotBorder_Rare      ← Borda azul para raro
│     └─ T_SlotBorder_Epic      ← Borda roxa para épico
│
└─ Input/
   └─ IA_OpenInventory          ← Input Action para tecla I
```

---

## 🔄 Fluxo de Dados (Como Funciona)

### Cenário 1: Jogador Abre o Inventário

```
1. Jogador pressiona "I"
   ↓
2. Character Blueprint
   - Cria WBP_Inventory
   - Adiciona ao viewport
   - Muda input mode para UI
   ↓
3. WBP_Inventory (Event Construct)
   - Bind nos Delegates do GameInstance
   - Chama LoadInventory()
   ↓
4. UmbraGameInstance (C++)
   - Faz requisição HTTP GET para API
   - Aguarda resposta
   ↓
5. API PHP
   - Valida JWT token
   - Query no MySQL
   - Retorna JSON com inventário
   ↓
6. UmbraGameInstance (C++)
   - Parseia JSON
   - Popula array CurrentInventory
   - Dispara Delegate "OnInventoryLoaded"
   ↓
7. WBP_Inventory (Escuta OnInventoryLoaded)
   - Loop pelos 50 slots
   - Para cada slot:
     * Pega dados com GetInventorySlotByIndex(i)
     * Atualiza visual (ícone, quantidade, etc)
   ↓
8. Jogador vê inventário atualizado na tela!
```

---

### Cenário 2: Jogador Arrasta Item para Outro Slot

```
1. Jogador clica e arrasta item do slot 5 para slot 12
   ↓
2. WBP_InventorySlot (slot 5)
   - On Mouse Button Down → Cria DragDropOperation
   ↓
3. WBP_InventorySlot (slot 12)
   - On Drop → Recebe DragDropOperation
   - Chama: GameInstance->MoveItem(inventoryID, 12)
   ↓
4. UmbraGameInstance (C++)
   - Faz requisição HTTP POST para API
   - Body: {token, inventory_id, target_slot_index: 12}
   ↓
5. API PHP (move_item.php)
   - Valida token
   - Atualiza slot_index no MySQL
   - Retorna sucesso
   ↓
6. UmbraGameInstance (C++)
   - Atualiza array CurrentInventory local
   - Dispara Delegate "OnItemMoved"
   ↓
7. WBP_Inventory (Escuta OnItemMoved)
   - Atualiza visual dos slots 5 e 12
   ↓
8. Item aparece no novo slot visualmente!
```

---

## 🎯 Próximos Passos Práticos

### **PASSO 1**: Verificar GameInstance ✅

1. Abra o Unreal Editor
2. `Edit > Project Settings`
3. `Maps & Modes > Game Instance Class`
4. Deve estar: `UmbraGameInstance` ✅

Se não estiver, selecione `UmbraGameInstance` na lista.

---

### **PASSO 2**: Criar Input Action para Inventário

1. `Content Browser > Input`
2. Abra `IMC_Default` (ou seu Input Mapping Context)
3. Adicione novo Input Action:
   - Nome: `IA_OpenInventory`
   - Key: `I` (keyboard)
4. Salve

---

### **PASSO 3**: Criar WBP_InventorySlot (Widget de 1 slot)

1. `Content Browser > Right Click > User Interface > Widget Blueprint`
2. Nome: `WBP_InventorySlot`
3. Salve em: `Content/UI/Inventory/`

**Hierarquia visual** (próximo passo detalhado):
```
Canvas Panel
└─ Border (64x64)
   └─ Overlay
      ├─ Image (ícone do item)
      ├─ TextBlock (quantidade)
      └─ ProgressBar (durabilidade)
```

---

### **PASSO 4**: Criar WBP_Inventory (Widget principal)

1. `Content Browser > Right Click > User Interface > Widget Blueprint`
2. Nome: `WBP_Inventory`
3. Salve em: `Content/UI/Inventory/`

**Hierarquia visual** (próximo passo detalhado):
```
Canvas Panel
└─ Border (fundo)
   └─ VerticalBox
      ├─ TextBlock "Inventário"
      └─ UniformGridPanel (5×10)
```

---

### **PASSO 5**: Configurar Character Blueprint

1. Abra `BP_ThirdPersonCharacter`
2. Event Graph
3. Adicione lógica de Input para tecla "I"

---

## 📋 Resumo: O Que Já Existe vs. O Que Falta

### ✅ **JÁ EXISTE E FUNCIONA**

| Componente | Status |
|------------|--------|
| MySQL Database | ✅ 17 itens templates |
| PHP REST API (6 endpoints) | ✅ Testado e funcional |
| C++ UmbraGameInstance | ✅ Compilado (970+ linhas) |
| Delegates C++ | ✅ 11 eventos prontos |
| Game Instance configurado | ✅ Em Project Settings |

### ⏳ **PRECISA CRIAR (Blueprint UI)**

| Componente | Status |
|------------|--------|
| WBP_InventorySlot | ⏳ Criar visual + lógica |
| WBP_Inventory | ⏳ Criar grid 5×10 |
| Input Action (tecla I) | ⏳ Adicionar em IMC |
| Character: abrir/fechar UI | ⏳ Event Graph |
| Drag & Drop | ⏳ Implementar |
| Tooltips | ⏳ Implementar |

---

## 💡 Conceitos Importantes

### **Game Instance vs. Actor vs. Widget**

| Tipo | Lifecycle | Acesso | Uso |
|------|-----------|--------|-----|
| **Game Instance** | Sessão inteira | Global | Dados persistentes |
| **Actor/Character** | Spawn → Destroy | No level | Entidades do jogo |
| **Widget** | Create → Remove | Viewport | UI visual |

**Analogia**:
- **Game Instance** = Banco de dados na memória (singleton)
- **Character** = Controle remoto (chama funções)
- **Widget** = Tela da TV (mostra visualmente)

---

### **Por Que Não Criar BP_UmbraGameInstance?**

**Resposta**: A classe C++ **já é um Blueprint**!

Quando você faz `Cast To UmbraGameInstance`, você está acessando a classe C++ diretamente do Blueprint. As funções marcadas com `UFUNCTION(BlueprintCallable)` aparecem automaticamente.

**Você só criaria um Blueprint derivado se** precisasse:
- Adicionar variáveis específicas de Blueprint
- Sobrescrever comportamento padrão
- Adicionar lógica visual no Event Graph

**Para inventário**: Não é necessário! A classe C++ tem tudo.

---

## 🚀 Está Pronto para Começar?

**Próxima etapa**: Criar os Widgets visuais (WBP_InventorySlot e WBP_Inventory).

Quer que eu crie um **guia passo a passo COM SCREENSHOTS** de como montar o WBP_InventorySlot no Editor Visual do Unreal?

Ou prefere que eu explique primeiro a lógica de Drag & Drop?

---

**Dúvidas?** Pergunte! Essa é a parte mais visual e divertida! 🎨

