# 🎯 GUIA COMPLETO: Conectar Delegates OnItemEquipped e OnItemUnequipped

## ✅ **OBJETIVO:**

Conectar os delegates `OnItemEquipped` e `OnItemUnequipped` do `UmbraGameInstance` no `WBP_CharacterInfo` para atualizar automaticamente os slots de equipamento quando um item for equipado ou desequipado.

---

## 📋 **PRÉ-REQUISITOS:**

1. ✅ `WBP_CharacterInfo` criado
2. ✅ Função `Update Equipment Slots` implementada
3. ✅ Função `Load Character Info` disponível no Game Instance
4. ✅ Delegates `OnItemEquipped` e `OnItemUnequipped` existem no C++

---

## 🔧 **PARTE 1: Criar os Eventos no WBP_CharacterInfo**

### **1.1 Criar Evento OnItemEquipped_Event**

**PASSO A PASSO:**

1. No `WBP_CharacterInfo`, abra o **Event Graph**
2. Clique direito → **Custom Event**
3. Nome: `OnItemEquipped_Event`
4. No painel **Details** do evento:
   - Clique em **Inputs** → **+ (Add Input)**
   - Nome: `Inventory ID`
   - Tipo: `Integer`
   - Clique em **Inputs** → **+ (Add Input)** novamente
   - Nome: `Equipment Slot`
   - Tipo: `EUmbraEquipmentSlot` (Enum)

**ESTRUTURA DO EVENTO:**
```
[OnItemEquipped_Event]
  ├─ Inventory ID: (Integer)
  └─ Equipment Slot: (EUmbraEquipmentSlot)
```

---

### **1.2 Criar Evento OnItemUnequipped_Event**

**PASSO A PASSO:**

1. No `WBP_CharacterInfo`, abra o **Event Graph**
2. Clique direito → **Custom Event**
3. Nome: `OnItemUnequipped_Event`
4. No painel **Details** do evento:
   - Clique em **Inputs** → **+ (Add Input)**
   - Nome: `Inventory ID`
   - Tipo: `Integer`
   - Clique em **Inputs** → **+ (Add Input)** novamente
   - Nome: `Equipment Slot`
   - Tipo: `EUmbraEquipmentSlot` (Enum)

**ESTRUTURA DO EVENTO:**
```
[OnItemUnequipped_Event]
  ├─ Inventory ID: (Integer)
  └─ Equipment Slot: (EUmbraEquipmentSlot)
```

---

## 🔧 **PARTE 2: Implementar a Lógica dos Eventos**

### **2.1 Implementar OnItemEquipped_Event**

**LÓGICA:**
Quando um item é equipado, precisamos recarregar as informações do personagem para atualizar os slots:

```
[OnItemEquipped_Event]
  ├─ Inventory ID: (Integer)
  └─ Equipment Slot: (EUmbraEquipmentSlot)
       │
       ▼
[Get Game Instance]
  └─ Game Instance
       │
       ▼
[Cast to Umbra Game Instance]
  ├─ Success
  │    │
  │    ▼
  │  [Load Character Info]
  │    └─ Target: As Umbra Game Instance
  │
  └─ Fail: [Log Error]
```

**COMO CRIAR:**
1. No `OnItemEquipped_Event`, arraste o pin `exec` (seta branca)
2. Digite: `Get Game Instance`
3. Conecte o `Return Value` (Game Instance) a um `Cast to Umbra Game Instance`
4. No pin `Success` do Cast:
   - Digite: `Load Character Info`
   - Conecte `As Umbra Game Instance` ao `Target` de `Load Character Info`
5. No pin `Fail` do Cast:
   - Digite: `Print String` ou `Log Error`
   - Mensagem: "Erro ao carregar Game Instance"

---

### **2.2 Implementar OnItemUnequipped_Event**

**LÓGICA:**
Mesma lógica do `OnItemEquipped_Event`:

```
[OnItemUnequipped_Event]
  ├─ Inventory ID: (Integer)
  └─ Equipment Slot: (EUmbraEquipmentSlot)
       │
       ▼
[Get Game Instance]
  └─ Game Instance
       │
       ▼
[Cast to Umbra Game Instance]
  ├─ Success
  │    │
  │    ▼
  │  [Load Character Info]
  │    └─ Target: As Umbra Game Instance
  │
  └─ Fail: [Log Error]
```

**COMO CRIAR:**
1. Mesmo processo do `OnItemEquipped_Event`
2. Copie a lógica e cole no `OnItemUnequipped_Event`

---

## 🔧 **PARTE 3: Conectar os Delegates no Event Construct**

### **3.1 Estrutura Completa do Event Construct**

**NO EVENT CONSTRUCT DO WBP_CharacterInfo:**

```
[Event Construct]
  ↓
[Get Game Instance]
  └─ Game Instance
       │
       ▼
[Cast to Umbra Game Instance]
  ├─ Success ───────────────────────────────────────────────┐
  │                                                         │
  │  [Assign On Item Equipped]                             │
  │    ├─ Target: As Umbra Game Instance                   │
  │    └─ Event: OnItemEquipped_Event                      │
  │         │                                               │
  │         ▼                                               │
  │  [Assign On Item Unequipped]                           │
  │    ├─ Target: As Umbra Game Instance                   │
  │    └─ Event: OnItemUnequipped_Event                    │
  │                                                         │
  └─────────────────────────────────────────────────────────┘
  │
  └─ Fail: [Print String]
        └─ In String: "Erro ao conectar delegates"
```

---

### **3.2 Passo a Passo Detalhado**

**PASSO 1: Obter Game Instance**

1. No `Event Construct` do `WBP_CharacterInfo`
2. Arraste o pin `exec` (seta branca)
3. Digite: `Get Game Instance`
4. O nó `Get Game Instance` será criado

**PASSO 2: Cast para Umbra Game Instance**

1. Arraste o pin `Return Value` (Game Instance) do `Get Game Instance`
2. Digite: `Cast to Umbra Game Instance`
3. Conecte `Return Value` ao pin `Object` do Cast
4. O Cast terá dois pins de saída: `Success` e `Fail`

**PASSO 3: Conectar OnItemEquipped**

1. No pin `Success` do Cast, arraste para criar um novo nó
2. Digite: `Assign On Item Equipped`
3. **IMPORTANTE:** Este nó conecta o delegate do Game Instance ao evento do widget
4. Configure:
   - **Target:** Arraste `As Umbra Game Instance` (do Cast) e conecte ao pin `Target`
   - **Event:** Arraste o evento `OnItemEquipped_Event` (criado na Parte 1) e conecte ao pin `Event`

**COMO OBTER O NÓ "Assign On Item Equipped":**
- Arraste `As Umbra Game Instance` (do Cast)
- Digite: `Assign On Item Equipped`
- O nó será criado automaticamente

**COMO CONECTAR O EVENTO:**
- Arraste o evento `OnItemEquipped_Event` (do painel "My Blueprint" ou do Event Graph)
- Conecte ao pin `Event` do nó `Assign On Item Equipped`

**PASSO 4: Conectar OnItemUnequipped**

1. Após o `Assign On Item Equipped`, arraste o pin `exec` de saída
2. Digite: `Assign On Item Unequipped`
3. Configure:
   - **Target:** `As Umbra Game Instance` (mesmo do passo anterior)
   - **Event:** `OnItemUnequipped_Event` (criado na Parte 1)

**PASSO 5: Tratamento de Erro (Opcional)**

1. No pin `Fail` do Cast, arraste para criar um novo nó
2. Digite: `Print String`
3. Configure:
   - **In String:** "Erro ao conectar delegates de equipamento"

---

## 🔧 **PARTE 4: Verificar se OnCharacterInfoLoaded Atualiza os Slots**

### **4.1 Garantir que Update Equipment Slots é Chamado**

**NO EVENTO OnCharacterInfoLoaded_Event:**

Certifique-se de que `Update Equipment Slots` está sendo chamado:

```
[OnCharacterInfoLoaded_Event]
  └─ Character Info: (FUmbraCharacterInfo)
       │
       ▼
[Update Equipment Slots]
  └─ Character Info: Character Info
```

**COMO VERIFICAR:**
1. Abra o `OnCharacterInfoLoaded_Event` no `WBP_CharacterInfo`
2. Verifique se há uma chamada para `Update Equipment Slots`
3. Se não houver, adicione:
   - Arraste o pin `Character Info` do evento
   - Digite: `Update Equipment Slots`
   - Conecte `Character Info` ao parâmetro `Character Info` da função

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

- [ x] Criar evento `OnItemEquipped_Event` com parâmetros `Inventory ID` e `Equipment Slot`
- [x ] Criar evento `OnItemUnequipped_Event` com parâmetros `Inventory ID` e `Equipment Slot`
- [x ] Implementar lógica em `OnItemEquipped_Event` para chamar `Load Character Info`
- [x ] Implementar lógica em `OnItemUnequipped_Event` para chamar `Load Character Info`
- [x ] No `Event Construct`, obter `Game Instance` e fazer `Cast to Umbra Game Instance`
- [x ] Conectar `Assign On Item Equipped` ao `OnItemEquipped_Event`
- [ x] Conectar `Assign On Item Unequipped` ao `OnItemUnequipped_Event`
- [ x] Verificar se `Update Equipment Slots` é chamado no `OnCharacterInfoLoaded_Event`

---

## 🎯 **FLUXO COMPLETO:**

```
1. Item é equipado/desequipado
   ↓
2. Game Instance dispara delegate (OnItemEquipped ou OnItemUnequipped)
   ↓
3. Delegate conectado chama evento no WBP_CharacterInfo
   ↓
4. Evento chama Load Character Info
   ↓
5. Load Character Info carrega dados atualizados
   ↓
6. OnCharacterInfoLoaded_Event é disparado
   ↓
7. Update Equipment Slots é chamado
   ↓
8. Slots são atualizados visualmente
```

---

## ✅ **TESTE:**

1. Abra o Character Info (C)
2. Equipe um item (arrastando ou duplo clique)
3. Os slots devem atualizar automaticamente
4. Desequipe um item
5. Os slots devem atualizar automaticamente novamente

---

## 🎯 **PRONTO!**

Agora os slots de equipamento serão atualizados automaticamente quando itens forem equipados ou desequipados! 🎉

