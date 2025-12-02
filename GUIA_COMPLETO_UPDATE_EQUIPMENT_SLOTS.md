# 🎯 GUIA COMPLETO: Implementar Função Update Equipment Slots

## ✅ **OBJETIVO:**

Criar a função `Update Equipment Slots` no `WBP_CharacterInfo` que:
- Recebe `FUmbraCharacterInfo` como parâmetro
- Extrai os itens equipados do `EquippedItems` (TMap)
- Atualiza cada slot de equipamento (`WBP_EquipmentSlot`) com o item correspondente
- Limpa slots que não têm itens equipados

---

## 📋 **PRÉ-REQUISITOS:**

1. ✅ `WBP_CharacterInfo` criado
2. ✅ `WBP_EquipmentSlot` criado com função `Update Slot Visual` e `Clear Slot`
3. ✅ Variáveis de slots criadas no `WBP_CharacterInfo` (`Slot_Head`, `Slot_Chest`, etc.)
4. ✅ Função `CreateEquipmentSlots` implementada no `WBP_CharacterInfo`
5. ✅ `FUmbraCharacterInfo` contém `EquippedItems` (TMap<EUmbraEquipmentSlot, FUmbraInventorySlot>)

---

## ⚠️ **IMPORTANTE: Onde Criar as Funções**

**TODAS as funções deste guia devem ser criadas no `WBP_CharacterInfo`:**

- ✅ `Update Equipment Slots` → **`WBP_CharacterInfo`**
- ✅ `Clear All Equipment Slots` → **`WBP_CharacterInfo`**

**POR QUÊ?**
- As variáveis `Slot_Head`, `Slot_Chest`, etc. são variáveis do `WBP_CharacterInfo`
- As funções `Update Slot Visual` e `Clear Slot` são funções do `WBP_EquipmentSlot`, mas são **chamadas** a partir do `WBP_CharacterInfo`
- O `WBP_CharacterInfo` é o widget "pai" que gerencia todos os slots de equipamento

---

## 🔧 **PARTE 1: Criar a Função Update Equipment Slots**

### **1.1 Criar a Função**

**⚠️ IMPORTANTE:** Esta função é criada no **`WBP_CharacterInfo`**, não no `WBP_EquipmentSlot`!

1. **No `WBP_CharacterInfo`**, abra o **Event Graph**
2. Clique direito → **Function** → Nome: `Update Equipment Slots`
3. Adicione o parâmetro de entrada:
   - **Input:** `Character Info` (Type: `Umbra Character Info`)

---

## 🔧 **PARTE 2: Estrutura da Função**

### **2.1 Break Character Info**

**PASSO 1: Quebrar a estrutura**

```
[Update Equipment Slots Entry]
  └─ Character Info: (FUmbraCharacterInfo)
       │
       ▼
[Break Umbra Character Info]
  ├─ Character Info: Character Info
  └─ Equipped Items: (TMap<EUmbraEquipmentSlot, FUmbraInventorySlot>)
```

**COMO CRIAR:**
1. Arraste o pin `Character Info` da função
2. Digite: `Break Umbra Character Info`
3. Conecte `Character Info` ao pin de entrada do `Break`
4. Expanda o `Break` para mostrar todos os campos
5. **IMPORTANTE:** Localize o campo `Equipped Items` (TMap)

---

### **2.2 Limpar Todos os Slots Primeiro**

**PASSO 2: Limpar todos os slots antes de atualizar**

Antes de atualizar com os itens equipados, precisamos limpar todos os slots para garantir que slots vazios sejam exibidos corretamente.

```
[Break Umbra Character Info]
  └─ Equipped Items: (TMap)
       │
       ▼
[Clear All Equipment Slots] ← Função auxiliar (criar depois)
```

**OU, limpar cada slot individualmente:**

```
[Clear All Equipment Slots]
  ↓
  [Clear Slot] → Slot_Head
  [Clear Slot] → Slot_Chest
  [Clear Slot] → Slot_Hands
  [Clear Slot] → Slot_Feet
  [Clear Slot] → Slot_MainHand
  [Clear Slot] → Slot_OffHand
  [Clear Slot] → Slot_Ring
  [Clear Slot] → Slot_Amulet
  [Clear Slot] → Slot_Earring
  [Clear Slot] → Slot_Bracelet
  [Clear Slot] → Slot_Mount
```

**COMO CRIAR:**
1. Para cada variável de slot (`Slot_Head`, `Slot_Chest`, etc.):
   - Arraste a variável → Selecione `Get Slot_Head` (ou o slot correspondente)
   - Arraste o pin → Digite: `Clear Slot`
   - Conecte `Get Slot_Head` ao `Target` de `Clear Slot`

---

### **2.3 Iterar sobre Equipped Items**

**PASSO 3: Usar ForEach para iterar sobre o TMap**

```
[Break Umbra Character Info]
  └─ Equipped Items: (TMap)
       │
       ▼
[ForEach Loop (TMap)] ← Nó especial para TMap
  ├─ Map: Equipped Items
  ├─ Loop Body (exec)
  ├─ Key: Equipment Slot (EUmbraEquipmentSlot)
  └─ Value: Inventory Slot (FUmbraInventorySlot)
```

**COMO CRIAR:**
1. Arraste o pin `Equipped Items` (TMap)
2. Digite: `ForEach Loop` ou `ForEach`
3. **IMPORTANTE:** O Unreal Engine tem um nó especial para iterar sobre TMap
4. Configure:
   - **Map:** `Equipped Items`
   - O nó automaticamente criará pins `Key` e `Value`
   - `Key` será do tipo `EUmbraEquipmentSlot`
   - `Value` será do tipo `FUmbraInventorySlot`

**⚠️ NOTA:** Se o nó `ForEach Loop` não aparecer diretamente para TMap, você pode usar:
- `Get Keys` para obter um array de chaves
- `ForEach Loop` normal para iterar sobre as chaves
- `Find` para obter o valor de cada chave

---

### **2.4 Switch on Equipment Slot**

**PASSO 4: Usar Switch para cada slot**

Dentro do loop, use um `Switch on EUmbraEquipmentSlot` para determinar qual slot atualizar:

```
[ForEach Loop (TMap)]
  ├─ Loop Body (exec)
  ├─ Key: Equipment Slot (EUmbraEquipmentSlot)
  └─ Value: Inventory Slot (FUmbraInventorySlot)
       │
       ▼
[Switch on EUmbraEquipmentSlot]
  ├─ Select: Equipment Slot (Key)
  │
  ├─ Head: [Update Slot Visual] → Slot_Head
  │         ├─ Target: Get Slot_Head
  │         └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Chest: [Update Slot Visual] → Slot_Chest
  │          ├─ Target: Get Slot_Chest
  │          └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Hands: [Update Slot Visual] → Slot_Hands
  │          ├─ Target: Get Slot_Hands
  │          └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Feet: [Update Slot Visual] → Slot_Feet
  │        ├─ Target: Get Slot_Feet
  │        └─ Item Slot: Inventory Slot (Value)
  │
  ├─ MainHand: [Update Slot Visual] → Slot_MainHand
  │             ├─ Target: Get Slot_MainHand
  │             └─ Item Slot: Inventory Slot (Value)
  │
  ├─ OffHand: [Update Slot Visual] → Slot_OffHand
  │            ├─ Target: Get Slot_OffHand
  │            └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Ring: [Update Slot Visual] → Slot_Ring
  │        ├─ Target: Get Slot_Ring
  │        └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Amulet: [Update Slot Visual] → Slot_Amulet
  │           ├─ Target: Get Slot_Amulet
  │           └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Earring: [Update Slot Visual] → Slot_Earring
  │            ├─ Target: Get Slot_Earring
  │            └─ Item Slot: Inventory Slot (Value)
  │
  ├─ Bracelet: [Update Slot Visual] → Slot_Bracelet
  │              ├─ Target: Get Slot_Bracelet
  │              └─ Item Slot: Inventory Slot (Value)
  │
  └─ Mount: [Update Slot Visual] → Slot_Mount
            ├─ Target: Get Slot_Mount
            └─ Item Slot: Inventory Slot (Value)
```

**COMO CRIAR O SWITCH:**
1. Clique direito → Digite: `Switch on EUmbraEquipmentSlot`
2. Conecte o pin `Key` (Equipment Slot) ao pin `Select` do Switch
3. O Switch automaticamente criará um pin de saída para cada valor do enum
4. Para cada pin de saída (Head, Chest, etc.):
   - Arraste a variável correspondente (`Get Slot_Head`, etc.)
   - Arraste o pin → Digite: `Update Slot Visual`
   - Conecte `Get Slot_Head` ao `Target` de `Update Slot Visual`
   - Conecte `Value` (Inventory Slot) ao pin `Item Slot` de `Update Slot Visual`

---

## 🔧 **PARTE 3: Método SIMPLES com Função C++ Helper**

**✅ SOLUÇÃO MELHOR:** Use a função C++ `GetEquippedItemsArray` que converte o TMap em um array simples!

**VANTAGENS:**
- ✅ Usa `ForEach Loop` normal (sobre array)
- ✅ Muito mais simples e limpo
- ✅ Não precisa de 11 nós `Find` repetidos

**COMO FUNCIONA:**
1. A função C++ `GetEquippedItemsArray` converte o TMap em um array de structs
2. Cada struct contém o `EquipmentSlot` e o `InventorySlot`
3. Você usa um `ForEach Loop` normal sobre o array
4. Dentro do loop, usa `Switch on EUmbraEquipmentSlot` para atualizar cada slot

### **3.1 Estrutura com GetEquippedItemsArray**

```
[Update Equipment Slots Entry]
  └─ Character Info: (FUmbraCharacterInfo)
       │
       ├─→ [Clear All Equipment Slots]
       │    (limpar todos os slots primeiro)
       │
       └─→ [Get Game Instance]
            └─ Game Instance
                 │
                 ▼
            [Cast to Umbra Game Instance]
              └─ Success
                   │
                   ▼
            [Get Equipped Items Array]
              ├─ Target: Game Instance
              ├─ Character Info: Character Info
              └─ Return Value: Array of Umbra Equipped Item Entry
                   │
                   ▼
            [ForEach Loop] ← Loop normal sobre array!
              ├─ Array: Return Value
              ├─ Loop Body (exec)
              └─ Array Element: Equipped Item Entry
                   │
                   ▼
            [Break Umbra Equipped Item Entry]
              ├─ Equipment Slot: (EUmbraEquipmentSlot)
              └─ Inventory Slot: (FUmbraInventorySlot)
                   │
                   ▼
            [Switch on EUmbraEquipmentSlot]
              ├─ Select: Equipment Slot
              │
              ├─ Head: [Update Slot Visual] → Slot_Head
              │         ├─ Target: Get Slot_Head
              │         └─ Item Slot: Inventory Slot
              │
              ├─ Chest: [Update Slot Visual] → Slot_Chest
              ├─ Hands: [Update Slot Visual] → Slot_Hands
              ├─ Feet: [Update Slot Visual] → Slot_Feet
              ├─ MainHand: [Update Slot Visual] → Slot_MainHand
              ├─ OffHand: [Update Slot Visual] → Slot_OffHand
              ├─ Ring: [Update Slot Visual] → Slot_Ring
              ├─ Amulet: [Update Slot Visual] → Slot_Amulet
              ├─ Earring: [Update Slot Visual] → Slot_Earring
              ├─ Bracelet: [Update Slot Visual] → Slot_Bracelet
              └─ Mount: [Update Slot Visual] → Slot_Mount
```

**COMO CRIAR:**
1. Arraste `Character Info` → Digite: `Get Game Instance`
2. Arraste `Game Instance` → Digite: `Cast to Umbra Game Instance`
3. Arraste `As Umbra Game Instance` → Digite: `Get Equipped Items Array`
4. Conecte `Character Info` ao pin `Character Info` de `Get Equipped Items Array`
5. Conecte `Return Value` (Array) a um `ForEach Loop` normal
6. Dentro do loop:
   - Arraste `Array Element` → Digite: `Break Umbra Equipped Item Entry`
   - Conecte `Equipment Slot` ao `Switch on EUmbraEquipmentSlot`
   - Conecte `Inventory Slot` aos `Update Slot Visual` de cada case do Switch

---

## 🔧 **PARTE 3.1: Método Alternativo (Sem Função Helper - NÃO RECOMENDADO)**

**⚠️ NOTA:** Se você não quiser usar a função helper C++, pode usar este método que verifica cada slot individualmente (muito verboso):

### **3.1 Verificar Cada Slot Individualmente**

Como não podemos iterar sobre o TMap, vamos verificar cada slot um por um usando `Find`:

```
[Break Umbra Character Info]
  └─ Equipped Items: (TMap)
       │
       ├─────────────────────────────────────────────────────────────┐
       │                                                             │
       ▼                                                             ▼
[Find] → Head                    [Find] → Chest
  ├─ Target: Equipped Items        ├─ Target: Equipped Items
  ├─ Key: Head (EUmbraEquipmentSlot) ├─ Key: Chest (EUmbraEquipmentSlot)
  └─ Return Value: Inventory Slot   └─ Return Value: Inventory Slot
       │                                 │
       ▼                                 ▼
[Is Valid] → Return Value          [Is Valid] → Return Value
  ├─ True: [Update Slot Visual] → Slot_Head  ├─ True: [Update Slot Visual] → Slot_Chest
  └─ False: (slot vazio, já foi limpo)      └─ False: (slot vazio, já foi limpo)
       │                                 │
       ▼                                 ▼
[Find] → Hands                    [Find] → Feet
  ├─ Target: Equipped Items        ├─ Target: Equipped Items
  ├─ Key: Hands                    ├─ Key: Feet
  └─ Return Value                   └─ Return Value
       │                                 │
       ▼                                 ▼
[Is Valid]                          [Is Valid]
  ├─ True: [Update Slot Visual] → Slot_Hands ├─ True: [Update Slot Visual] → Slot_Feet
  └─ False: (slot vazio)            └─ False: (slot vazio)
       │                                 │
       ▼                                 ▼
[Find] → MainHand                  [Find] → OffHand
  ├─ Target: Equipped Items        ├─ Target: Equipped Items
  ├─ Key: MainHand                 ├─ Key: OffHand
  └─ Return Value                   └─ Return Value
       │                                 │
       ▼                                 ▼
[Is Valid]                          [Is Valid]
  ├─ True: [Update Slot Visual] → Slot_MainHand ├─ True: [Update Slot Visual] → Slot_OffHand
  └─ False: (slot vazio)            └─ False: (slot vazio)
       │                                 │
       ▼                                 ▼
[Find] → Ring                       [Find] → Amulet
  ├─ Target: Equipped Items        ├─ Target: Equipped Items
  ├─ Key: Ring                     ├─ Key: Amulet
  └─ Return Value                   └─ Return Value
       │                                 │
       ▼                                 ▼
[Is Valid]                          [Is Valid]
  ├─ True: [Update Slot Visual] → Slot_Ring ├─ True: [Update Slot Visual] → Slot_Amulet
  └─ False: (slot vazio)            └─ False: (slot vazio)
       │                                 │
       ▼                                 ▼
[Find] → Earring                    [Find] → Bracelet
  ├─ Target: Equipped Items        ├─ Target: Equipped Items
  ├─ Key: Earring                  ├─ Key: Bracelet
  └─ Return Value                   └─ Return Value
       │                                 │
       ▼                                 ▼
[Is Valid]                          [Is Valid]
  ├─ True: [Update Slot Visual] → Slot_Earring ├─ True: [Update Slot Visual] → Slot_Bracelet
  └─ False: (slot vazio)            └─ False: (slot vazio)
       │                                 │
       ▼                                 ▼
[Find] → Mount
  ├─ Target: Equipped Items
  ├─ Key: Mount
  └─ Return Value
       │
       ▼
[Is Valid]
  ├─ True: [Update Slot Visual] → Slot_Mount
  └─ False: (slot vazio)
```

**COMO CRIAR:**
1. Arraste `Equipped Items` (TMap) do `Break Umbra Character Info`
2. Digite: `Find`
3. Configure:
   - **Target:** `Equipped Items`
   - **Key:** Selecione o slot específico (ex: `Head`, `Chest`, etc.)
     - Para obter o valor do enum, clique no pin `Key` → Selecione o valor do enum (ex: `Head`)
4. Conecte o `Return Value` (Inventory Slot) a um `Is Valid`
5. Se `Is Valid` for `True`, chame `Update Slot Visual` no slot correspondente
6. Repita para todos os 11 slots (Head, Chest, Hands, Feet, MainHand, OffHand, Ring, Amulet, Earring, Bracelet, Mount)

**COMO OBTER O VALOR DO ENUM PARA O KEY:**
1. Clique no pin `Key` do nó `Find`
2. No dropdown, selecione o valor do enum (ex: `Head`, `Chest`, etc.)
3. Ou use `Make Literal EUmbraEquipmentSlot` e configure o valor

---

## 🔧 **PARTE 4: Função Auxiliar - Clear All Equipment Slots**

### **4.1 Onde Criar a Função**

**⚠️ IMPORTANTE:** A função `Clear All Equipment Slots` deve ser criada no **`WBP_CharacterInfo`**, não no `WBP_EquipmentSlot`!

**POR QUÊ?**
- As variáveis `Slot_Head`, `Slot_Chest`, etc. são variáveis do `WBP_CharacterInfo`
- A função `Clear Slot` é uma função do `WBP_EquipmentSlot`, mas ela é chamada a partir do `WBP_CharacterInfo`
- A função auxiliar `Clear All Equipment Slots` é apenas uma função de conveniência dentro do `WBP_CharacterInfo` para limpar todos os slots de uma vez

### **4.1.1 Erro Comum: Conectar Equipped Items ao Target**

**❌ ERRO:**
```
Erro: "Mapa de Enumeração EUmbraEquipmentSlots para Umbra Inventory Slot 
Estruturas não é compatível com Self Referência de Objeto."
```

**CAUSA:**
- Tentar conectar `Equipped Items` (TMap) ao pin `Target` de `Clear All Equipment Slots`
- O pin `Target` espera um `Object Reference` (self), não um TMap

**✅ SOLUÇÃO:**
1. **Remova a conexão** entre `Equipped Items` e o pin `Target` de `Clear All Equipment Slots`
2. **Deixe o pin `Target` vazio** (ou conecte a `self` se necessário)
3. `Equipped Items` será usado **DEPOIS**, na função `Update Equipment Slots`, no `ForEach Loop`

### **4.2 Criar Função Auxiliar**

**ONDE:** No `WBP_CharacterInfo` → Event Graph

**NOME:** `Clear All Equipment Slots`

**⚠️ IMPORTANTE:** Esta função **NÃO recebe parâmetros**! Ela apenas limpa todos os slots.

**❌ ERRO COMUM:**
- **NÃO** conecte `Equipped Items` ao pin `Target` de `Clear All Equipment Slots`!
- O pin `Target` deve ser deixado como `self` (ou vazio) - ele já é automaticamente o próprio `WBP_CharacterInfo`
- `Equipped Items` será usado **DEPOIS**, na função `Update Equipment Slots`, não aqui!

**✅ CORRETO:**
- O pin `Target` de `Clear All Equipment Slots` deve ser `self` (ou deixar vazio)
- A função apenas limpa os slots, não precisa de nenhum parâmetro

**LÓGICA:**
```
[Clear All Equipment Slots]
  (Target: self - deixe vazio ou conecte a "self")
  ↓
  [Is Valid] → Slot_Head
  ├─ True: [Clear Slot] → Slot_Head
  │         ├─ Target: Get Slot_Head
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Chest
  ├─ True: [Clear Slot] → Slot_Chest
  │         ├─ Target: Get Slot_Chest
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Hands
  ├─ True: [Clear Slot] → Slot_Hands
  │         ├─ Target: Get Slot_Hands
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Feet
  ├─ True: [Clear Slot] → Slot_Feet
  │         ├─ Target: Get Slot_Feet
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_MainHand
  ├─ True: [Clear Slot] → Slot_MainHand
  │         ├─ Target: Get Slot_MainHand
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_OffHand
  ├─ True: [Clear Slot] → Slot_OffHand
  │         ├─ Target: Get Slot_OffHand
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Ring
  ├─ True: [Clear Slot] → Slot_Ring
  │         ├─ Target: Get Slot_Ring
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Amulet
  ├─ True: [Clear Slot] → Slot_Amulet
  │         ├─ Target: Get Slot_Amulet
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Earring
  ├─ True: [Clear Slot] → Slot_Earring
  │         ├─ Target: Get Slot_Earring
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Bracelet
  ├─ True: [Clear Slot] → Slot_Bracelet
  │         ├─ Target: Get Slot_Bracelet
  │         └─ (Clear Slot é função do WBP_EquipmentSlot)
  │
  [Is Valid] → Slot_Mount
  └─ True: [Clear Slot] → Slot_Mount
            ├─ Target: Get Slot_Mount
            └─ (Clear Slot é função do WBP_EquipmentSlot)
```

**COMO CRIAR:**
1. **No `WBP_CharacterInfo`**, abra o **Event Graph**
2. Clique direito → **Function** → Nome: `Clear All Equipment Slots`
3. Para cada slot (11 slots):
   - Arraste a variável (`Get Slot_Head`, `Get Slot_Chest`, etc.)
   - Digite: `Is Valid`
   - Conecte `Get Slot_Head` ao `Target` de `Is Valid`
   - Arraste o pin `True` → Digite: `Clear Slot`
   - **IMPORTANTE:** `Clear Slot` é uma função do `WBP_EquipmentSlot`
   - Conecte `Get Slot_Head` ao `Target` de `Clear Slot`
   - O `Clear Slot` será chamado no widget `WBP_EquipmentSlot` correspondente

---

## 🔧 **PARTE 5: Estrutura Completa da Função**

### **5.1 Fluxo Completo**

```
[Update Equipment Slots Entry]
  └─ Character Info: (FUmbraCharacterInfo)
       │
       ▼
[Break Umbra Character Info]
  └─ Equipped Items: (TMap)
       │
       ├─────────────────────────────────────┐
       │                                     │
       ▼                                     ▼
[Clear All Equipment Slots]    [Find] → Head
  (Target: self - sem parâmetros) ├─ Target: Equipped Items
  (limpar todos os slots)         ├─ Key: Head
                                  └─ Return Value: Inventory Slot
                                           │
                                           ▼
                                  [Is Valid]
                                    ├─ True: [Update Slot Visual] → Slot_Head
                                    └─ False: (slot vazio, já foi limpo)
                                           │
                                           ▼
                                  [Find] → Chest
                                    ├─ Target: Equipped Items
                                    ├─ Key: Chest
                                    └─ Return Value
                                         │
                                         ▼
                                    [Is Valid]
                                      ├─ True: [Update Slot Visual] → Slot_Chest
                                      └─ False: (slot vazio)
                                         │
                                         ▼
                                    [Find] → Hands
                                      ├─ Target: Equipped Items
                                      ├─ Key: Hands
                                      └─ Return Value
                                           │
                                           ▼
                                      [Is Valid]
                                        ├─ True: [Update Slot Visual] → Slot_Hands
                                        └─ False: (slot vazio)
                                           │
                                           ▼
                                      ... (repetir para todos os 11 slots)
```

**⚠️ IMPORTANTE:**
- `Clear All Equipment Slots` **NÃO recebe parâmetros** - o pin `Target` deve ser `self` (ou deixar vazio)
- `Equipped Items` é usado **DEPOIS**, em múltiplos nós `Find` (um para cada slot), não na função `Clear All Equipment Slots`!
- Este método verifica cada slot individualmente usando `Find` no TMap, sem precisar de loops

---

## 🔧 **PARTE 6: Chamar a Função**

### **6.1 No OnCharacterInfoLoaded_Event**

Após carregar as informações do personagem, chame `Update Equipment Slots`:

```
[OnCharacterInfoLoaded_Event]
  └─ Character Info: (FUmbraCharacterInfo)
       │
       ▼
[Update Equipment Slots]
  └─ Character Info: Character Info
```

**COMO CRIAR:**
1. No `OnCharacterInfoLoaded_Event` (criado via "Assign On Character Info Loaded")
2. Arraste o pin `Character Info`
3. Digite: `Update Equipment Slots`
4. Conecte `Character Info` ao parâmetro `Character Info` da função

---

## 📋 **DETALHAMENTO PASSO A PASSO:**

### **PASSO 1: Criar a Função**

**⚠️ IMPORTANTE:** Esta função é criada no **`WBP_CharacterInfo`**, não no `WBP_EquipmentSlot`!

1. Abra `WBP_CharacterInfo`
2. Vá para **Event Graph**
3. Clique direito → **Function** → Nome: `Update Equipment Slots`
4. No painel **Details** da função:
   - Clique em **Inputs** → **+ (Add Input)**
   - Nome: `Character Info`
   - Tipo: `Umbra Character Info` (FUmbraCharacterInfo)

---

### **PASSO 2: Break Character Info**

1. Arraste o pin `Character Info` da função
2. Digite: `Break Umbra Character Info`
3. Conecte `Character Info` ao pin de entrada
4. Expanda o `Break` para ver todos os campos
5. Localize `Equipped Items` (TMap)

---

### **PASSO 3: Criar Função Clear All Equipment Slots**

**⚠️ IMPORTANTE:** Esta função é criada no **`WBP_CharacterInfo`**, não no `WBP_EquipmentSlot`!

1. **No `WBP_CharacterInfo`**, abra o **Event Graph**
2. Clique direito → **Function** → Nome: `Clear All Equipment Slots`
3. Para cada slot (11 slots):
   - Arraste a variável (`Get Slot_Head`, `Get Slot_Chest`, etc.) - essas variáveis são do `WBP_CharacterInfo`
   - Digite: `Is Valid`
   - Conecte `Get Slot_Head` ao `Target` de `Is Valid`
   - Arraste o pin `True` → Digite: `Clear Slot`
   - **IMPORTANTE:** `Clear Slot` é uma função do `WBP_EquipmentSlot`, mas você a chama a partir do `WBP_CharacterInfo`
   - Conecte `Get Slot_Head` ao `Target` de `Clear Slot`
   - O `Clear Slot` será executado no widget `WBP_EquipmentSlot` correspondente

---

### **PASSO 4: Usar GetEquippedItemsArray (MÉTODO RECOMENDADO)**

**✅ RECOMENDADO:** Use a função C++ helper `GetEquippedItemsArray` que converte o TMap em um array simples!

1. Arraste `Character Info` → Digite: `Get Game Instance`
2. Arraste `Game Instance` → Digite: `Cast to Umbra Game Instance`
3. Arraste `As Umbra Game Instance` → Digite: `Get Equipped Items Array`
4. Conecte `Character Info` ao pin `Character Info` de `Get Equipped Items Array`
5. Conecte `Return Value` (Array of Umbra Equipped Item Entry) a um `ForEach Loop` normal
6. Dentro do loop:
   - Arraste `Array Element` → Digite: `Break Umbra Equipped Item Entry`
   - Conecte `Equipment Slot` ao `Switch on EUmbraEquipmentSlot`
   - Conecte `Inventory Slot` aos `Update Slot Visual` de cada case do Switch

---

### **PASSO 4.1: Método Alternativo (SEM Função Helper - NÃO RECOMENDADO)**

**⚠️ NOTA:** Se você não quiser usar a função helper, pode usar este método (muito verboso):

1. Arraste o pin `Equipped Items` (TMap) do `Break Umbra Character Info`
2. Digite: `Find`
3. Configure o primeiro `Find`:
   - **Target:** `Equipped Items`
   - **Key:** Clique no pin `Key` → Selecione `Head` (ou use `Make Literal EUmbraEquipmentSlot` e configure como `Head`)
4. Conecte o `Return Value` (Inventory Slot) a um `Is Valid`
5. Se `Is Valid` for `True`:
   - Arraste `Get Slot_Head` (variável do `WBP_CharacterInfo`)
   - Digite: `Update Slot Visual`
   - Conecte `Get Slot_Head` ao `Target` de `Update Slot Visual`
   - Conecte o `Return Value` do `Find` ao pin `Item Slot` de `Update Slot Visual`
6. Se `Is Valid` for `False`:
   - Não faça nada (o slot já foi limpo pela função `Clear All Equipment Slots`)
7. **Repita os passos 2-6 para todos os 11 slots:**
   - Head → Slot_Head
   - Chest → Slot_Chest
   - Hands → Slot_Hands
   - Feet → Slot_Feet
   - MainHand → Slot_MainHand
   - OffHand → Slot_OffHand
   - Ring → Slot_Ring
   - Amulet → Slot_Amulet
   - Earring → Slot_Earring
   - Bracelet → Slot_Bracelet
   - Mount → Slot_Mount

**COMO OBTER O VALOR DO ENUM PARA O KEY:**
- **Método 1:** Clique no pin `Key` do nó `Find` → No dropdown, selecione o valor do enum (ex: `Head`, `Chest`, etc.)
- **Método 2:** Use `Make Literal EUmbraEquipmentSlot` → Configure o valor → Conecte ao pin `Key` do `Find`

---

### **PASSO 5: Chamar Clear All Equipment Slots**

1. Antes de verificar os slots, chame `Clear All Equipment Slots`
2. Conecte o `exec` pin da função `Update Equipment Slots` ao `exec` pin de entrada de `Clear All Equipment Slots`
3. Conecte o `exec` pin de saída de `Clear All Equipment Slots` ao primeiro `Find` (ou ao primeiro `Is Valid`)

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

- [ ] Criar função `Update Equipment Slots` com parâmetro `Character Info`
- [ ] Criar função auxiliar `Clear All Equipment Slots`
- [ ] Implementar `Break Umbra Character Info` para obter `Equipped Items`
- [ ] Implementar `Clear All Equipment Slots` antes de verificar os slots
- [ ] Para cada slot (11 slots), criar um nó `Find` no TMap:
  - [ ] `Find` → Head → `Is Valid` → `Update Slot Visual` → Slot_Head
  - [ ] `Find` → Chest → `Is Valid` → `Update Slot Visual` → Slot_Chest
  - [ ] `Find` → Hands → `Is Valid` → `Update Slot Visual` → Slot_Hands
  - [ ] `Find` → Feet → `Is Valid` → `Update Slot Visual` → Slot_Feet
  - [ ] `Find` → MainHand → `Is Valid` → `Update Slot Visual` → Slot_MainHand
  - [ ] `Find` → OffHand → `Is Valid` → `Update Slot Visual` → Slot_OffHand
  - [ ] `Find` → Ring → `Is Valid` → `Update Slot Visual` → Slot_Ring
  - [ ] `Find` → Amulet → `Is Valid` → `Update Slot Visual` → Slot_Amulet
  - [ ] `Find` → Earring → `Is Valid` → `Update Slot Visual` → Slot_Earring
  - [ ] `Find` → Bracelet → `Is Valid` → `Update Slot Visual` → Slot_Bracelet
  - [ ] `Find` → Mount → `Is Valid` → `Update Slot Visual` → Slot_Mount
- [ ] Chamar `Update Equipment Slots` no `OnCharacterInfoLoaded_Event`

---

## 🎯 **EXEMPLO VISUAL COMPLETO:**

```
┌─────────────────────────────────────────────────────────────┐
│ [Update Equipment Slots Entry]                              │
│   └─ Character Info: (FUmbraCharacterInfo)                 │
│        │                                                     │
│        ▼                                                     │
│ ┌─────────────────────────────────────────────────────┐   │
│ │ [Break Umbra Character Info]                          │   │
│ │   └─ Equipped Items: (TMap)                           │   │
│ │        │                                               │   │
│ │        ├───────────────────┐                           │   │
│ │        │                   │                           │   │
│ │        ▼                   ▼                           │   │
│ │ [Clear All Equipment Slots]  [Find] → Head              │   │
│ │                              ├─ Target: Equipped Items  │   │
│ │                              ├─ Key: Head               │   │
│ │                              └─ Return Value           │   │
│ │                                   │                     │   │
│ │                                   ▼                     │   │
│ │                          [Is Valid]                     │   │
│ │                            ├─ True → [Update Slot Visual] → Slot_Head│
│ │                            └─ False: (slot vazio)      │   │
│ │                                   │                     │   │
│ │                                   ▼                     │   │
│ │                          [Find] → Chest                 │   │
│ │                            ├─ Target: Equipped Items    │   │
│ │                            ├─ Key: Chest                │   │
│ │                            └─ Return Value              │   │
│ │                                 │                       │   │
│ │                                 ▼                       │   │
│ │                          [Is Valid]                     │   │
│ │                            ├─ True → [Update Slot Visual] → Slot_Chest│
│ │                            └─ False: (slot vazio)       │   │
│ │                                 │                       │   │
│ │                                 ▼                       │   │
│ │                          ... (repetir para todos os 11 slots)│
│ │                            ├─ Amulet → [Update Slot Visual]││
│ │                            ├─ Earring → [Update Slot Visual]│
│ │                            ├─ Bracelet → [Update Slot Visual]│
│ │                            └─ Mount → [Update Slot Visual]││
│ └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 **PRONTO!**

Agora você tem um guia completo para implementar a função `Update Equipment Slots`! 🎉

**TESTE:**
1. Abra o Character Info (C)
2. Os slots de equipamento devem exibir os itens equipados
3. Slots vazios devem aparecer vazios (sem ícone)
4. Ao equipar/desequipar um item, os slots devem atualizar automaticamente

---

## 📘 **REFERÊNCIAS:**

- `GUIA_CRIAR_EQUIPMENT_SLOTS_WBP_CHARACTERINFO.md` - Como criar os slots
- `GUIA_COMPLETO_EQUIPAR_ITENS.md` - Sistema completo de equipamento
- `GUIA_COMPLETO_CHARACTER_INFO.md` - Widget de informações do personagem

