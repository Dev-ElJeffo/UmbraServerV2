# 📊 GUIA: Exibir Quantidade de Itens no Inventário

## 🎯 **OBJETIVO:**

Criar um widget de texto que exibe a quantidade de itens ocupados no inventário (ex: "8/50" ou "Slots: 8/50").

---

## 📋 **PASSO A PASSO:**

### **PASSO 1: Adicionar Widget de Texto no Designer**

1. **Abra o Blueprint `WBP_Inventory`**
2. **Vá para o Designer** (painel visual)
3. **Adicione um `Text Block`** (ou `Text`):
   - No **Palette**, procure por **"Text Block"**
   - Arraste para o **Hierarchy** (geralmente no final do inventário, como footer)
   - **Nome sugerido:** `Text_ItemCount` ou `Text_SlotsInfo`

4. **Posicione o texto:**
   - Coloque-o na parte inferior do inventário (footer)
   - Ajuste tamanho, cor e fonte conforme necessário

5. **Configure o texto inicial:**
   - No **Details Panel**, em **Content** → **Text**, defina: `"Slots: 0/50"` (ou deixe vazio)

---

### **PASSO 2: Criar Função para Atualizar o Texto**

1. **No `WBP_Inventory`, vá para Functions:**
   - **My Blueprint** → **Functions** → **"+" (Add)**

2. **Nome da Função:** `UpdateItemCount`

3. **Implemente a lógica:**

```
UpdateItemCount (Function)
  ↓
Get MyGameInstance (ou Get Game Instance → Cast to Umbra Game Instance)
  ↓
Get Occupied Slots Count (do Game Instance)  ← FUNÇÃO JÁ EXISTE!
  ↓
Format Text
  └─ Format: "Slots: {0}/50"
  └─ {0}: Get Occupied Slots Count (Return Value)
  ↓
Get Text_ItemCount (ou Text_SlotsInfo)
  ↓
Set Text (Text_ItemCount)
  └─ In Text: Format Text (Return Value)
```

---

### **PASSO 3: Chamar a Função Quando o Inventário For Carregado**

**No Event `OnInventoryLoaded_Event`:**

1. **Após atualizar todos os slots, adicione:**

```
OnInventoryLoaded_Event
  ↓
(loop para atualizar slots...)
  ↓
Update Item Count  ← ADICIONE AQUI!
```

**Como fazer:**
- No final do `OnInventoryLoaded_Event`, adicione o nó `Update Item Count`
- Conecte o `then` do último nó do loop ao `execute` de `Update Item Count`

---

### **PASSO 4: Chamar a Função Quando Itens Forem Movidos**

**No Event `OnItemMoved_Event`:**

1. **Após `Load Inventory`, adicione:**

```
OnItemMoved_Event
  ↓
Refresh Inventory
  ↓
Load Inventory
  ↓
Update Item Count  ← ADICIONE AQUI!
```

**Como fazer:**
- Após o `then` de `Load Inventory`, adicione `Update Item Count`
- Isso garante que a contagem seja atualizada após mover um item

---

### **PASSO 5: Chamar a Função Quando Itens Forem Adicionados/Removidos (Opcional)**

**Se você tiver eventos `OnItemAdded_Event` ou `OnItemRemoved_Event`:**

1. **Adicione `Update Item Count` no final desses eventos também**

---

## 🔧 **IMPLEMENTAÇÃO DETALHADA:**

### **1. Criar a Função `UpdateItemCount`:**

**Passo a passo:**

1. **Adicione `Get MyGameInstance`:**
   - Arraste a variável `MyGameInstance` para o Event Graph
   - Ou use `Get Game Instance` → `Cast to Umbra Game Instance`

2. **Adicione `Get Occupied Slots Count`:**
   - Arraste `Get Occupied Slots Count` do `MyGameInstance` para o Event Graph
   - Ou procure por **"Get Occupied Slots Count"** na busca
   - Isso retorna um `Integer` com a quantidade de slots ocupados

3. **Adicione `Format Text`:**
   - Procure por **"Format Text"**
   - No **Format** pin, digite: `"Slots: {0}/50"` (ou `"Itens: {0}/50"`)
   - Conecte o `Return Value` do `Get Occupied Slots Count` ao `{0}` pin (ou `Arg0`)

5. **Adicione `Get Text_ItemCount`:**
   - Arraste o widget `Text_ItemCount` do **Hierarchy** para o Event Graph

6. **Adicione `Set Text`:**
   - Procure por **"Set Text"**
   - Conecte o `Get Text_ItemCount` ao `Target` pin
   - Conecte o `Return Value` do `Format Text` ao `In Text` pin

---

### **2. Conectar no `OnInventoryLoaded_Event`:**

**Estrutura:**

```
OnInventoryLoaded_Event
  ↓
(Primeiro loop: limpar todos os slots)
  ↓
(Segundo loop: atualizar slots com itens)
  ↓
Update Item Count  ← ADICIONE AQUI!
```

**Como fazer:**
- No final do segundo loop (após `Update Slot Visual` do último item)
- Adicione o nó `Update Item Count`
- Conecte o `then` do último `Update Slot Visual` ao `execute` de `Update Item Count`

---

### **3. Conectar no `OnItemMoved_Event`:**

**Estrutura:**

```
OnItemMoved_Event
  ↓
Refresh Inventory
  ↓
Load Inventory
  ↓
Update Item Count  ← ADICIONE AQUI!
```

**Como fazer:**
- Após o `then` de `Load Inventory`, adicione `Update Item Count`
- Isso garante que a contagem seja atualizada após a API responder

---

## 📝 **EXEMPLO DE FORMATO DE TEXTO:**

Você pode personalizar o formato do texto:

- `"Slots: {0}/50"` → "Slots: 8/50"
- `"Itens: {0}/50"` → "Itens: 8/50"
- `"{0}/50 slots ocupados"` → "8/50 slots ocupados"
- `"Inventário: {0}/50"` → "Inventário: 8/50"

---

## ⚠️ **IMPORTANTE:**

1. **A contagem usa `GetOccupiedSlotsCount()`:**
   - Esta função retorna `CurrentInventory.Num()` (número de itens no inventário)
   - Cada item ocupa 1 slot (mesmo que tenha quantidade > 1)
   - A função já existe no `UmbraGameInstance`, então use `Get Occupied Slots Count` diretamente

2. **A função deve ser chamada após `Load Inventory`:**
   - Isso garante que `CurrentInventory` esteja atualizado

3. **Se você quiser mostrar slots vazios também:**
   - Use: `"Slots: {0}/50"` onde `{0}` é `50 - Array Length`
   - Ou: `"Slots livres: {0}/50"`

---

## 🧪 **TESTE:**

1. Abra o inventário
2. **O texto deve mostrar:** "Slots: 8/50" (ou a quantidade atual)
3. Mova um item
4. **O texto deve atualizar** após a API responder
5. Adicione um item
6. **O texto deve atualizar** para "Slots: 9/50"

---

## 📝 **RESUMO:**

1. ✅ Adicione um `Text Block` no Designer do `WBP_Inventory`
2. ✅ Crie a função `UpdateItemCount` que:
   - Obtém `Get Occupied Slots Count` do `GameInstance`
   - Formata o texto com `Format Text`
   - Atualiza o `Text Block`
3. ✅ Chame `UpdateItemCount` em:
   - `OnInventoryLoaded_Event` (após atualizar slots)
   - `OnItemMoved_Event` (após `Load Inventory`)

