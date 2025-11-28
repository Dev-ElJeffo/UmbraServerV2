# 🖱️ GUIA DETALHADO: Como Verificar Left Mouse Button com Equal (Key Key)

## 🎯 **OBJETIVO:**

Verificar se o botão clicado no `OnMouseButtonDoubleClick` é o **Left Mouse Button**.

---

## 📋 **PASSO A PASSO COMPLETO:**

### **PASSO 1: Criar o Evento OnMouseButtonDoubleClick**

1. No `WBP_EquipmentSlot`, abra o **Event Graph**
2. Clique direito no espaço vazio
3. Procure por: **"Override"** ou **"OnMouseButtonDoubleClick"**
4. Selecione: **`OnMouseButtonDoubleClick`**

**Você terá automaticamente:**
- `Geometry` (FGeometry)
- `MouseEvent` (FPointerEvent)
- `Button` (EKeys) ← **ESTE É O QUE PRECISAMOS!**

---

### **PASSO 2: Obter o Nó Equal (Key Key)**

**OPÇÃO A: Pelo Context Menu (Recomendado)**

1. Arraste o pin `Button` para fora (ou clique direito próximo ao pin)
2. No menu de busca, digite: **`Equal`**
3. Você verá várias opções:
   - `Equal (Key Key)` ← **ESTE É O CORRETO!**
   - `Equal (Bool Bool)`
   - `Equal (Int Int)`
   - etc.
4. Selecione: **`Equal (Key Key)`**

**OPÇÃO B: Pelo Painel de Nós**

1. Clique direito no espaço vazio
2. Digite: **`Equal Key`**
3. Selecione: **`Equal (Key Key)`**

---

### **PASSO 3: Conectar o Button ao Equal**

1. **Conecte o pin `Button`** (do `OnMouseButtonDoubleClick`) ao pin **`A`** do `Equal (Key Key)`

**COMO FAZER:**
- Clique e arraste do pin `Button` até o pin `A` do `Equal`
- OU clique no pin `Button` e depois no pin `A`

---

### **PASSO 4: Obter Left Mouse Button**

**OPÇÃO A: Make Literal Key (Recomendado)**

1. Clique direito no espaço vazio
2. Digite: **`Make Literal Key`**
3. Selecione: **`Make Literal Key`**
4. No pin **`Value`**, clique no dropdown
5. Procure por: **`Left Mouse Button`** ou **`EKeys::LeftMouseButton`**
6. Selecione: **`Left Mouse Button`**

**OPÇÃO B: EKeys Enum (Alternativa)**

1. Clique direito no espaço vazio
2. Digite: **`EKeys`**
3. Selecione: **`EKeys`**
4. Expanda o enum e procure: **`Left Mouse Button`**
5. Arraste para o pin **`B`** do `Equal`

**OPÇÃO C: Digitar Diretamente (Mais Rápido)**

1. Clique no pin **`B`** do `Equal (Key Key)`
2. No campo de texto, digite: **`Left Mouse Button`**
3. Pressione Enter

---

### **PASSO 5: Conectar ao Branch**

1. Arraste o pin **`Return Value`** (bool) do `Equal (Key Key)`
2. Digite: **`Branch`**
3. Selecione: **`Branch`**
4. Conecte o `Return Value` ao pin **`Condition`** do `Branch`

**OU:**

1. Arraste o pin **`Return Value`** do `Equal`
2. Conecte diretamente ao pin **`Condition`** de um `Branch` existente

---

## 📊 **ESTRUTURA VISUAL:**

```
┌─────────────────────────────────────────────────────────────┐
│  [OnMouseButtonDoubleClick]                                  │
│    ├─ Geometry: (FGeometry)                                │
│    ├─ MouseEvent: (FPointerEvent)                          │
│    └─ Button: (EKeys) ───────────────────┐                  │
│                                          │                   │
│                                          ▼                   │
│  ┌─────────────────────────────────────────────┐           │
│  │  [Equal (Key Key)]                          │           │
│  │    ├─ A: Button ────────────────────────────┼──────────┐ │
│  │    ├─ B: Left Mouse Button ─────────────────┼────────┐ │ │
│  │    │                                        │        │ │ │
│  │    │  [Make Literal Key]                    │        │ │ │
│  │    │    └─ Value: Left Mouse Button ─────────┘        │ │ │
│  │    │                                                  │ │ │
│  │    └─ Return Value: (bool) ──────────────────────────┼─┼─┘ │
│  └──────────────────────────────────────────────────────┘ │ │
│                                                           │ │
│                                                           ▼ │
│  ┌─────────────────────────────────────────────┐         │ │
│  │  [Branch]                                    │         │ │
│  │    ├─ Condition: (bool) ────────────────────┘         │ │
│  │    ├─ True ───────────────────────────────────────────┐ │
│  │    │                                                   │ │
│  │    │  [AQUI VAI SUA LÓGICA DE DESEQUIPAR]            │ │
│  │    │                                                   │ │
│  │    └─ False                                           │ │
│  │                                                         │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔍 **DETALHES IMPORTANTES:**

### **1. Tipo do Pin Button:**

O pin `Button` do `OnMouseButtonDoubleClick` é do tipo **`EKeys`** (enum).

**EKeys** é um enum que representa todas as teclas e botões do mouse:
- `Left Mouse Button`
- `Right Mouse Button`
- `Middle Mouse Button`
- `Space Bar`
- `Enter`
- etc.

---

### **2. Equal (Key Key) vs Outros Equal:**

**❌ ERRADO:**
- `Equal (Bool Bool)` - Não funciona!
- `Equal (Int Int)` - Não funciona!
- `Equal (Byte Byte)` - Pode funcionar, mas não é o ideal

**✅ CORRETO:**
- `Equal (Key Key)` - **ESTE É O CORRETO!**

---

### **3. Make Literal Key:**

O nó `Make Literal Key` permite criar um valor literal do tipo `EKeys`.

**COMO USAR:**
1. Crie o nó `Make Literal Key`
2. No pin `Value`, clique no dropdown
3. Procure por `Left Mouse Button`
4. Selecione

**OU:**

1. Crie o nó `Make Literal Key`
2. Clique no pin `Value`
3. Digite: `Left Mouse Button`
4. Pressione Enter

---

### **4. Alternativa: Usar EKeys Enum Diretamente**

Se preferir, você pode usar o enum `EKeys` diretamente:

1. Clique direito → Digite: **`EKeys`**
2. Selecione: **`EKeys`**
3. Expanda e procure: **`Left Mouse Button`**
4. Conecte ao pin `B` do `Equal`

**MAS:** `Make Literal Key` é mais simples e direto!

---

## ✅ **EXEMPLO COMPLETO:**

### **CÓDIGO BLUEPRINT:**

```
[OnMouseButtonDoubleClick]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  └─ Button: (EKeys)
       │
       ▼
  [Equal (Key Key)]
    ├─ A: Button ──────────────────────────┐
    │                                      │
    │  [Make Literal Key]                  │
    │    └─ Value: Left Mouse Button ─────┘
    │
    └─ Return Value: (bool)
         │
         ▼
      [Branch]
        ├─ Condition: (bool)
        ├─ True ───────────────────────────┐
        │                                   │
        │  [Get Equipped Item]              │
        │    └─ Equipped Item              │
        │         │                         │
        │         ▼                         │
        │  [Break Umbra Inventory Slot]    │
        │    └─ Item Template ID           │
        │         │                         │
        │         ▼                         │
        │  [Greater] → Item Template ID > 0 │
        │    │                              │
        │    ├─ TRUE:                       │
        │    │    │                         │
        │    │    ▼                         │
        │    │  [Unequip Item]              │
        │    │    └─ Inventory ID           │
        │    │                              │
        │    └─ FALSE: [Unhandled]         │
        │                                   │
        └─ False: [Unhandled]
```

---

## 🎯 **RESUMO RÁPIDO:**

1. ✅ **Crie** `OnMouseButtonDoubleClick`
2. ✅ **Arraste** o pin `Button`
3. ✅ **Digite** `Equal` e selecione `Equal (Key Key)`
4. ✅ **Conecte** `Button` ao pin `A`
5. ✅ **Crie** `Make Literal Key`
6. ✅ **Selecione** `Left Mouse Button` no dropdown
7. ✅ **Conecte** ao pin `B` do `Equal`
8. ✅ **Conecte** `Return Value` a um `Branch`

---

## ⚠️ **ERROS COMUNS:**

### **ERRO 1: Usar Equal (Bool Bool) ou Equal (Int Int)**

**❌ ERRADO:**
```
[Button] → [Equal (Bool Bool)] → [Branch]
```

**✅ CORRETO:**
```
[Button] → [Equal (Key Key)] → [Branch]
```

---

### **ERRO 2: Não Conectar o Make Literal Key**

**❌ ERRADO:**
```
[Equal (Key Key)]
  ├─ A: Button
  └─ B: (não conectado)
```

**✅ CORRETO:**
```
[Equal (Key Key)]
  ├─ A: Button
  └─ B: [Make Literal Key] → Left Mouse Button
```

---

### **ERRO 3: Usar String ao Invés de Key**

**❌ ERRADO:**
```
[Button] → [Equal (String String)] → "Left Mouse Button"
```

**✅ CORRETO:**
```
[Button] → [Equal (Key Key)] → [Make Literal Key] → Left Mouse Button
```

---

## 🎉 **PRONTO!**

Agora você sabe exatamente como verificar se é `Left Mouse Button` usando `Equal (Key Key)`! 🎯

