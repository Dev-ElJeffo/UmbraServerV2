# ✅ **SOLUÇÃO DIRETA: Buscar Cada Placeholder pelo ClassID**

## 🎯 **OBJETIVO**

Para cada ClassID (1, 2, 3, 4, 5, 6), buscar o placeholder correspondente no nível e salvar seu Transform.

---

## ✅ **SOLUÇÃO SIMPLES**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderTransforms
    ↓
[For Loop] (1 a 6)
    ↓ Loop Body
    [Get All Actors of Class]
        • Actor Class: BP_Class_Placeholder
        ↓
    [For Each Loop]
        • Array: (Out Actors)
        ↓ Loop Body
        [Get ClassID] (do Array Element)
        ↓
        [Equal] (Integer)
            • A: (ClassID)
            • B: (Loop Index)
        ↓
        [Branch] (True)
            ↓
        [Add to Array] PlaceholderClassIDs
            • New Item: (Loop Index)
        ↓
        [Get Actor Location]
            • Target: (Array Element)
        ↓
        [Get Actor Rotation]
            • Target: (Array Element)
        ↓
        [Get Actor Scale]
            • Target: (Array Element)
        ↓
        [Make Transform]
            • Location: (Actor Location)
            • Rotation: (Actor Rotation)
            • Scale: (Actor Scale)
        ↓
        [Add to Array] PlaceholderTransforms
            • New Item: (Make Transform)
        ↓
        [Break] ← Conecta ao pin Break do For Each Loop
    ↓
[Completed] (do For Loop)
    ↓
[RESTANTE DO EVENT CONSTRUCT]
```

---

## 📋 **PASSO A PASSO**

### **1. No Event Construct, após limpar os arrays:**

**Adicione um `For Loop` de 1 a 6**

### **2. Dentro do For Loop:**

**Adicione `Get All Actors of Class` (BP_Class_Placeholder)**

### **3. Após Get All Actors:**

**Adicione `For Each Loop` (normal, não precisa ser "with Break" se você não tiver)**

**OU se tiver `For Each Loop with Break`:**

**Dentro do Loop Body:**
- `Get ClassID` do Array Element
- `Equal` (ClassID == Loop Index)
- `Branch` (True)
- `Add to Array` PlaceholderClassIDs (Loop Index)
- `Get Actor Location` (Target = Array Element)
- `Get Actor Rotation` (Target = Array Element)
- `Get Actor Scale` (Target = Array Element)
- `Make Transform` (Location, Rotation, Scale)
- `Add to Array` PlaceholderTransforms (Make Transform)
- **Se usar `For Each Loop with Break`**: conecte o `then` do último `Add to Array` ao pin `Break` do `For Each Loop with Break`

### **4. Após o For Loop:**

**O `Completed` do `For Loop` conecta ao resto do `Event Construct`**

---

## ⚠️ **IMPORTANTE**

**Se você usar `For Each Loop with Break`:**
- O pin `Break` é uma **ENTRADA** (input)
- Conecte o `then` do último `Add to Array` ao pin `Break`
- O `Completed` do `For Each Loop with Break` **NÃO conecta a nada**

**Se você usar `For Each Loop` normal:**
- Não há como fazer Break, então o loop vai continuar
- Mas isso não importa, porque você só adiciona quando o ClassID corresponde

---

## ✅ **VERIFICAÇÃO**

**Após fazer isso:**
1. ✅ `PlaceholderClassIDs` terá [1, 2, 3, 4, 5, 6]
2. ✅ `PlaceholderTransforms` terá os Transforms na mesma ordem
3. ✅ Cada Transform terá a Location, Rotation e Scale corretas do placeholder no nível

---

**FIM DA SOLUÇÃO**

