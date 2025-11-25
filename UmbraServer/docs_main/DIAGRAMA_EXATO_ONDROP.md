# 📊 DIAGRAMA EXATO: Onde Implementar Cada Parte

## 🎯 **ESTRUTURA ATUAL DO SEU CÓDIGO:**

```
OnDrop
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓ (then)
IsValid (Cast)
  ↓ (then)
Branch (Is Valid? Cast)
  ├─ [TRUE]
  │   └─ Get Source Slot Widget
  │       └─ IsValid (Source Slot Widget)
  │           └─ Branch (Is Valid? Source)
  │               ├─ [TRUE]
  │               │   └─ Get Slot Data (Source)
  │               │       └─ Break Umbra Inventory Slot
  │               │           └─ Branch (Inventory ID > 0?)
  │               │               ├─ [TRUE]
  │               │               │   └─ K2Node_Knot_6
  │               │               │       └─ K2Node_IfThenElse_7 (Branch Destino é Storage?)
  │               │               │           ├─ [then] → K2Node_IfThenElse_8 (Branch Origem é Storage?)
  │               │               │           │   ├─ [then] → (Origem = Storage)
  │               │               │           │   │   └─ K2Node_IfThenElse_7 [else] → K2Node_Knot_7
  │               │               │           │   │       └─ [📍 IMPLEMENTAÇÃO 1: Storage → Inventário]
  │               │               │           │   │
  │               │               │           │   └─ [else] → (Origem = Inventário)
  │               │               │           │       └─ [📍 IMPLEMENTAÇÃO 2 e 3: Inventário → Storage/Inventário]
  │               │               │           │
  │               │               │           └─ [else] → (Não usado)
  │               │               │
  │               │               └─ [FALSE] → (Não conectado)
  │               │
  │               └─ [FALSE] → (Não conectado)
  │
  └─ [FALSE] → Return Unhandled
```

---

## 📍 **LOCALIZAÇÃO EXATA DAS IMPLEMENTAÇÕES:**

### **📍 IMPLEMENTAÇÃO 1: Storage → Inventário**

**Localização:** Após `K2Node_Knot_7` (que vem do `else` do `K2Node_IfThenElse_7`)

**Cenário:** 
- Origem = Storage (`K2Node_IfThenElse_8` [then] = TRUE)
- Destino = Inventário (`K2Node_IfThenElse_7` [else] = FALSE)

**Conexão inicial:**
```
K2Node_IfThenElse_7 [else] → K2Node_Knot_7 [InputPin]
K2Node_Knot_7 [OutputPin] → [📍 AQUI COMEÇA A IMPLEMENTAÇÃO 1]
```

**O que implementar:**
1. Get Parent Storage Widget (Source)
2. IsValid (WBP_Storage - Source)
3. Get Game Instance
4. Cast to Umbra Game Instance
5. Get Storage ID By Inventory ID
6. Greater (Storage ID > 0?)
7. Branch (Storage ID > 0?)
8. Get Slot Index (Destino)
9. Move Item From Storage
10. Branch (Return Value)
11. Return Handled / Return Unhandled

**Ver guia completo:** `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 1"

---

### **📍 IMPLEMENTAÇÃO 2: Inventário → Storage**

**Localização:** Após `Branch_DestinoIsStorage` [TRUE] (quando origem é Inventário)

**Cenário:** 
- Origem = Inventário (`K2Node_IfThenElse_8` [else] = FALSE)
- Destino = Storage (`Branch_DestinoIsStorage` [TRUE] = TRUE)

**Conexão inicial:**
```
K2Node_IfThenElse_8 [else] → GetParentStorageWidget_Destino [execute]
GetParentStorageWidget_Destino [then] → IsValid_Destino [execute]
IsValid_Destino [then] → Branch_DestinoIsStorage [execute]
Branch_DestinoIsStorage [TRUE] → [📍 AQUI COMEÇA A IMPLEMENTAÇÃO 2]
```

**O que implementar:**
1. Get Parent Storage Widget (Destino) - Para obter WBP_Storage
2. IsValid (WBP_Storage - Destino)
3. Get Slot Index (Destino)
4. Move Item To Storage
5. Branch (Return Value)
6. Return Handled / Return Unhandled

**Ver guia completo:** `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 2"

---

### **📍 IMPLEMENTAÇÃO 3: Inventário → Inventário**

**Localização:** Após `Branch_DestinoIsStorage` [FALSE] (quando origem é Inventário)

**Cenário:** 
- Origem = Inventário (`K2Node_IfThenElse_8` [else] = FALSE)
- Destino = Inventário (`Branch_DestinoIsStorage` [FALSE] = FALSE)

**Conexão inicial:**
```
K2Node_IfThenElse_8 [else] → GetParentStorageWidget_Destino [execute]
GetParentStorageWidget_Destino [then] → IsValid_Destino [execute]
IsValid_Destino [then] → Branch_DestinoIsStorage [execute]
Branch_DestinoIsStorage [FALSE] → [📍 AQUI COMEÇA A IMPLEMENTAÇÃO 3]
```

**O que implementar:**
1. Process Item Drop
2. Return Handled

**Ver guia completo:** `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 3"

---

## 🔍 **ANÁLISE DO SEU CÓDIGO ATUAL:**

### **✅ O QUE JÁ ESTÁ CONECTADO:**

1. **K2Node_IfThenElse_8 (Branch Origem é Storage?):**
   - ✅ `execute` conectado ao `then` do `K2Node_IfThenElse_7`
   - ✅ `Condition` conectado ao `ReturnValue` do `K2Node_CallFunction_17` (IsValid - Origem)
   - ✅ `then` conectado ao `execute` do `K2Node_IfThenElse_7` (mas isso está errado!)
   - ❌ `else` **NÃO está conectado** (precisa conectar!)

2. **K2Node_IfThenElse_7 (Branch Destino é Storage?):**
   - ✅ `execute` conectado ao `K2Node_Knot_6`
   - ✅ `Condition` conectado ao `ReturnValue` do `K2Node_CallFunction_16` (IsValid - Destino)
   - ✅ `then` conectado ao `execute` do `K2Node_IfThenElse_8`
   - ✅ `else` conectado ao `K2Node_Knot_7`

3. **K2Node_IfThenElse_9 (Branch Destino é Storage? - Outro):**
   - ✅ `execute` conectado ao `K2Node_Knot_7`
   - ✅ `Condition` conectado ao `ReturnValue` do `K2Node_CallFunction_18` (IsValid - Destino)
   - ❌ `then` **NÃO está conectado** (precisa conectar!)
   - ❌ `else` **NÃO está conectado** (precisa conectar!)

---

## ❌ **PROBLEMAS NO SEU CÓDIGO:**

### **PROBLEMA 1: K2Node_IfThenElse_8 [then] está conectado errado**

**Atualmente:**
```
K2Node_IfThenElse_8 [then] → K2Node_IfThenElse_7 [execute]
```

**Isso está ERRADO!** O `then` do `K2Node_IfThenElse_8` significa "Origem = Storage", mas você está conectando ao `execute` do `K2Node_IfThenElse_7`, que verifica o destino.

**Correto:**
- O `then` do `K2Node_IfThenElse_8` deve estar conectado ao `else` do `K2Node_IfThenElse_7` (que já está conectado ao `K2Node_Knot_7`)
- Mas você já tem isso! O problema é que o `then` do `K2Node_IfThenElse_8` está conectado ao `execute` do `K2Node_IfThenElse_7`, o que não faz sentido.

**Na verdade, analisando melhor:**
- O `K2Node_IfThenElse_7` verifica se o **destino** é Storage
- O `K2Node_IfThenElse_8` verifica se a **origem** é Storage
- O `then` do `K2Node_IfThenElse_8` está conectado ao `execute` do `K2Node_IfThenElse_7`, o que significa que quando a origem é Storage, ele verifica o destino novamente.

**Isso está correto!** Mas falta implementar o que fazer em cada caso.

---

### **PROBLEMA 2: K2Node_IfThenElse_8 [else] não está conectado**

**Atualmente:**
```
K2Node_IfThenElse_8 [else] → (NÃO CONECTADO)
```

**Precisa conectar:**
- Após o `else` do `K2Node_IfThenElse_8`, você precisa verificar o destino (Storage ou Inventário)
- Isso é feito com `GetParentStorageWidget_Destino` → `IsValid_Destino` → `Branch_DestinoIsStorage`

---

### **PROBLEMA 3: K2Node_IfThenElse_9 não está sendo usado**

**Atualmente:**
- `K2Node_IfThenElse_9` está conectado ao `K2Node_Knot_7`, mas não está sendo usado corretamente
- Parece ser uma duplicação do `K2Node_IfThenElse_7`

**Solução:**
- Você pode remover o `K2Node_IfThenElse_9` ou usá-lo para verificar o destino quando a origem é Inventário

---

## 🔧 **CORREÇÕES NECESSÁRIAS:**

### **CORREÇÃO 1: Conectar K2Node_IfThenElse_8 [else]**

**Após o `else` do `K2Node_IfThenElse_8`:**

1. **Get Parent Storage Widget (Destino):**
   - Arraste um nó "Self" (clique com botão direito → "Self")
   - Arraste o pino azul do `Self`
   - Digite "Get Parent Storage Widget"
   - **Conectar:**
     - **`Target` (azul):** Conecte ao `Self` (azul)
     - **`execute` (branco):** Conecte ao `else` (branco) do `K2Node_IfThenElse_8`

2. **IsValid (Destino):**
   - Arraste o `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
   - Digite "Is Valid?"
   - **Conectar:**
     - **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `Get Parent Storage Widget (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `Get Parent Storage Widget (Destino)`

3. **Branch (Destino é Storage?):**
   - Arraste o `ReturnValue` (bool, verde) do `IsValid (Destino)`
   - Digite "Branch"
   - **Conectar:**
     - **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `IsValid (Destino)`
     - **`execute` (branco):** Conecte ao `then` (branco) do `IsValid (Destino)`

4. **Agora você tem dois caminhos:**
   - **`Branch (Destino é Storage?)` [TRUE]:** Implementação 2 (Inventário → Storage)
   - **`Branch (Destino é Storage?)` [FALSE]:** Implementação 3 (Inventário → Inventário)

---

### **CORREÇÃO 2: Implementar Storage → Inventário**

**Após o `K2Node_Knot_7` (que vem do `else` do `K2Node_IfThenElse_7`):**

Siga o guia: `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 1"

---

### **CORREÇÃO 3: Implementar Inventário → Storage**

**Após o `Branch (Destino é Storage?)` [TRUE] quando origem é Inventário:**

Siga o guia: `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 2"

---

### **CORREÇÃO 4: Implementar Inventário → Inventário**

**Após o `Branch (Destino é Storage?)` [FALSE] quando origem é Inventário:**

Siga o guia: `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` → Seção "IMPLEMENTAÇÃO 3"

---

## 📊 **RESUMO VISUAL DAS CONEXÕES:**

```
K2Node_IfThenElse_6 [then]
  └─ K2Node_Knot_6
      └─ K2Node_IfThenElse_7 [execute]
          ├─ [then] → K2Node_IfThenElse_8 [execute]
          │   ├─ [then] → (Origem = Storage)
          │   │   └─ K2Node_IfThenElse_7 [else] → K2Node_Knot_7
          │   │       └─ [📍 IMPLEMENTAÇÃO 1: Storage → Inventário]
          │   │
          │   └─ [else] → (Origem = Inventário)  ← CONECTAR AQUI!
          │       └─ GetParentStorageWidget_Destino
          │           └─ IsValid_Destino
          │               └─ Branch_DestinoIsStorage
          │                   ├─ [TRUE] → [📍 IMPLEMENTAÇÃO 2: Inventário → Storage]
          │                   └─ [FALSE] → [📍 IMPLEMENTAÇÃO 3: Inventário → Inventário]
          │
          └─ [else] → (Não usado neste cenário)
```

---

## ✅ **CHECKLIST DE CORREÇÃO:**

- [ ] Conectar `K2Node_IfThenElse_8` [else] ao `GetParentStorageWidget_Destino` [execute]
- [ ] Implementar Storage → Inventário (após `K2Node_Knot_7`)
- [ ] Implementar Inventário → Storage (após `Branch_DestinoIsStorage` [TRUE])
- [ ] Implementar Inventário → Inventário (após `Branch_DestinoIsStorage` [FALSE])

---

**Siga este diagrama e o guia `GUIA_EXATO_IMPLEMENTACAO_ONDROP.md` para implementar cada parte!** 🚀

