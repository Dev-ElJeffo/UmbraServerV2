# 🖱️ GUIA: OnMouseButtonDoubleClick (MELHOR QUE OnMouseButtonDown!)

## ✅ **VANTAGENS DO OnMouseButtonDoubleClick:**

1. ✅ **Mais Simples** - Não precisa verificar `Click Count` manualmente
2. ✅ **Mais Limpo** - Código mais direto e fácil de entender
3. ✅ **Mais Eficiente** - O Unreal Engine detecta automaticamente o double click
4. ✅ **Menos Código** - Menos nós no Blueprint

---

## 🔧 **COMO USAR:**

### **PASSO 1: Criar o Evento**

1. No `WBP_EquipmentSlot`, Event Graph
2. Clique direito → **Override** → `OnMouseButtonDoubleClick`
3. Você terá:
   - `exec` (execução)
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent) ← **ESTE É O QUE PRECISAMOS!**

**⚠️ IMPORTANTE:** O `OnMouseButtonDoubleClick` **NÃO** tem um pin `Button` direto!

---

### **PASSO 2: Obter o Botão do Mouse**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Get Mouse Button`**
3. Selecione: **`Get Mouse Button`**
4. Você terá:
   - `Return Value` (EKeys) ← **O BOTÃO!**

---

### **PASSO 3: Verificar se é Left Mouse Button**

**OPÇÃO A: Switch on EKeys (RECOMENDADO - Mais Simples!)**

1. Arraste o `Return Value` (EKeys) do `Get Mouse Button`
2. Digite: **`Switch on EKeys`**
3. Selecione: **`Switch on EKeys`**
4. Conecte o `Return Value` ao pin `Select` (ou `Key`)
5. No `Switch on EKeys`, adicione um case:
   - Clique em **"Add Pin"** ou **"+"**
   - Selecione: **`Left Mouse Button`**
6. Conecte o pin `Left Mouse Button` (exec) à sua lógica

**OPÇÃO B: Equal (Key Key)**

1. Arraste o `Return Value` (EKeys) do `Get Mouse Button`
2. Digite: **`Equal`** e selecione **`Equal (Key Key)`**
3. Conecte o `Return Value` ao pin `A`
4. Para o pin `B`:
   - Clique direito → Digite: **`EKeys`**
   - Selecione: **`EKeys`** (enum)
   - Expanda e procure: **`Left Mouse Button`**
   - Arraste para o pin `B` do `Equal`
5. Conecte o `Return Value` (bool) a um `Branch`

**📘 Veja o guia completo:** `GUIA_CORRETO_ONMOUSEBUTTONDOUBLECLICK.md`

---

### **PASSO 3: Verificar se Tem Item Equipado**

1. Arraste a variável `EquippedItem`
2. Digite: `Break Umbra Inventory Slot`
3. Conecte `EquippedItem` ao pin de entrada
4. Expanda e mostre o pin `Item Template ID`
5. Digite: `Greater (Int Int)`
6. Conecte:
   - **A:** `Item Template ID`
   - **B:** `0` (Make Literal Int)

---

### **PASSO 4: Desequipar o Item**

1. Se `Item Template ID > 0`:
   - `Get Game Instance`
   - `Cast to Umbra Game Instance`
   - `Unequip Item`
     - Target: Game Instance
     - Inventory ID: `Inventory ID` (do Break Umbra Inventory Slot)
   - `Handled` (ou `Make Literal Bool` true se retornar bool)

---

## 📊 **ESTRUTURA COMPLETA:**

### **OnMouseButtonDoubleClick (WBP_EquipmentSlot):**

```
[OnMouseButtonDoubleClick]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ├─ Button: (EKeys)
  ↓
[Equal (Key Key)] ← Verificar se é Left Mouse Button
  ├─ A: Button
  ├─ B: Left Mouse Button
  └─ Return Value: (bool)
       │
       ├─ TRUE ───────────────────────────────────────────────┐
       │                                                      │
       │  [Get Equipped Item] ← Variável                      │
       │    └─ Equipped Item: (FUmbraInventorySlot)           │
       │         │                                            │
       │         ▼                                            │
       │  [Break Umbra Inventory Slot]                        │
       │    ├─ Inventory ID: (int)                            │
       │    └─ Item Template ID: (int)                        │
       │         │                                            │
       │         ▼                                            │
       │  [Greater (Int Int)]                                 │
       │    ├─ A: Item Template ID                            │
       │    ├─ B: 0                                           │
       │    └─ Return Value: (bool)                           │
       │         │                                            │
       │         ├─ TRUE ──────────────────────────────────┐  │
       │         │                                        │  │
       │         │  [Get Game Instance]                   │  │
       │         │    └─ Game Instance                     │  │
       │         │         │                              │  │
       │         │         ▼                              │  │
       │         │  [Cast to Umbra Game Instance]          │  │
       │         │    └─ Success                          │  │
       │         │         │                              │  │
       │         │         ▼                              │  │
       │         │  [Unequip Item]                        │  │
       │         │    ├─ Target: Game Instance             │  │
       │         │    └─ Inventory ID: Inventory ID       │  │
       │         │         │                              │  │
       │         │         ▼                              │  │
       │         │  [Handled] ← OU Make Literal Bool true │  │
       │         │                                        │  │
       │         └──────────────────────────────────────┘  │
       │                                                      │
       └──────────────────────────────────────────────────────┘
```

---

## 🔧 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Criar o Evento**

1. No `WBP_EquipmentSlot`, Event Graph
2. Clique direito → **Override** → `OnMouseButtonDoubleClick`
3. Você terá automaticamente:
   - `Geometry` (FGeometry)
   - `MouseEvent` (FPointerEvent)
   - `Button` (EKeys)

---

### **PASSO 2: Verificar se é Left Mouse Button**

1. Arraste o pin `Button`
2. Digite: `Equal (Key Key)` ou `Equal (Byte Byte)`
3. Conecte:
   - **A:** `Button`
   - **B:** `Left Mouse Button` (ou use `EKeys::LeftMouseButton`)

**COMO OBTER "Left Mouse Button":**
- Clique direito → Digite: `Make Literal Key` ou `EKeys`
- Selecione `Left Mouse Button`

---

### **PASSO 3: Verificar se Tem Item Equipado**

1. Arraste a variável `EquippedItem`
2. Digite: `Break Umbra Inventory Slot`
3. Conecte `EquippedItem` ao pin de entrada
4. Expanda e mostre:
   - `Inventory ID` ← Para desequipar
   - `Item Template ID` ← Para validar

---

### **PASSO 4: Validar Item Template ID**

1. Arraste o pin `Item Template ID` do Break
2. Digite: `Greater (Int Int)`
3. Conecte:
   - **A:** `Item Template ID`
   - **B:** `0` (Make Literal Int)
4. Conecte o `Return Value` a um `Branch`

---

### **PASSO 5: Desequipar o Item**

1. Se `Item Template ID > 0`:
   - `Get Game Instance`
   - `Cast to Umbra Game Instance`
   - `Unequip Item`
     - Target: Game Instance
     - Inventory ID: `Inventory ID` (do Break)
   - `Handled` (ou `Make Literal Bool` true)

---

## 📊 **COMPARAÇÃO:**

### **❌ ANTES (OnMouseButtonDown com Click Count):**

```
[OnMouseButtonDown]
  ↓
[Get Mouse Button]
  ↓
[Is Valid]
  ↓
[Get Click Count] ← VERIFICAÇÃO MANUAL
  ↓
[Equal] → Click Count == 2
  ↓
[Unequip Item]
```

### **✅ AGORA (OnMouseButtonDoubleClick):**

```
[OnMouseButtonDoubleClick]
  ↓
[Equal] → Button == Left Mouse Button
  ↓
[Is Valid] → Item Template ID > 0
  ↓
[Unequip Item] ← DIRETO!
```

**MUITO MAIS SIMPLES!** 🎉

---

## ⚠️ **IMPORTANTE:**

### **OnMouseButtonDoubleClick:**
- ✅ É disparado **automaticamente** quando há um double click
- ✅ Não precisa verificar `Click Count` manualmente
- ✅ Mais eficiente e limpo

### **OnMouseButtonDown:**
- ❌ É disparado em **qualquer** clique (single ou double)
- ❌ Precisa verificar `Click Count` manualmente
- ❌ Mais código e mais complexo

---

## ✅ **RESUMO:**

1. ✅ Use **`OnMouseButtonDoubleClick`** ao invés de `OnMouseButtonDown`
2. ✅ Verifique se é **Left Mouse Button**
3. ✅ Verifique se tem **Item Equipado** (Item Template ID > 0)
4. ✅ Chame **`Unequip Item`** com o `Inventory ID`
5. ✅ Retorne **`Handled`** (ou `true` se retornar bool)

---

## 🎯 **PRONTO!**

Use `OnMouseButtonDoubleClick` - é muito mais simples e direto! 🎉

