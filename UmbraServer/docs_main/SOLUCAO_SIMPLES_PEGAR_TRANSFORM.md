# ✅ **SOLUÇÃO SIMPLES: Pegar Location, Rotation e Scale Separadamente**

## 🎯 **OBJETIVO**

Pegar a posição e escala dos placeholders que estão no nível e usar no respawn.

---

## ✅ **SOLUÇÃO: Usar Get Actor Location, Rotation e Scale Separadamente**

**No `Event Construct`, substitua `Get Actor Transform` por:**

```
[For Loop] (1 a 6)
    ↓ Loop Body
    [Get All Actors of Class]
        • Actor Class: BP_Class_Placeholder
        ↓
    [For Each Loop with Break]
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
        [Break] ← Conecta ao pin Break do For Each Loop with Break
```

---

## 📋 **PASSO A PASSO**

### **1. Remover Get Actor Transform**

**Delete o nó `Get Actor Transform` que está sendo usado atualmente.**

### **2. Adicionar Get Actor Location**

**Após `Add to Array` do ClassID:**
1. **Clique com botão direito** → "Get Actor Location"
2. **Conecte o `Target` ao `Array Element` do `For Each Loop with Break`**

### **3. Adicionar Get Actor Rotation**

**Após `Get Actor Location`:**
1. **Clique com botão direito** → "Get Actor Rotation"
2. **Conecte o `Target` ao `Array Element` do `For Each Loop with Break`**

### **4. Adicionar Get Actor Scale**

**Após `Get Actor Rotation`:**
1. **Clique com botão direito** → "Get Actor Scale"
2. **Conecte o `Target` ao `Array Element` do `For Each Loop with Break`**

### **5. Adicionar Make Transform**

**Após `Get Actor Scale`:**
1. **Clique com botão direito** → "Make Transform"
2. **Conecte:**
   - `Location` → `Return Value` do `Get Actor Location`
   - `Rotation` → `Return Value` do `Get Actor Rotation`
   - `Scale` → `Return Value` do `Get Actor Scale`

### **6. Conectar ao Add to Array**

**Conecte o `Return Value` do `Make Transform` ao `New Item` do `Add to Array` do `PlaceholderTransforms`.**

### **7. Conectar o Break**

**Conecte o `then` do `Add to Array` ao pin `Break` (entrada) do `For Each Loop with Break`.**

---

## ✅ **ESTRUTURA COMPLETA**

```
[For Loop] (1 a 6)
    ↓ Loop Body
    [Get All Actors of Class]
        ↓
    [For Each Loop with Break]
        ↓ Loop Body
        [Get ClassID]
        ↓
        [Equal] (ClassID == Loop Index)
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
        [Break] ← Conecta ao pin Break do For Each Loop with Break
        ↓
    [Completed] ← NÃO CONECTA A NADA
        ↓
[Completed] (do For Loop) ← Conecta ao resto do Event Construct
```

---

## ✅ **VERIFICAÇÃO**

**Após fazer isso:**
1. ✅ Os placeholders devem spawnar nas posições corretas
2. ✅ Os placeholders devem spawnar com as escalas corretas
3. ✅ Todos os 6 placeholders devem spawnar

---

**FIM DA SOLUÇÃO**

