# 🔧 **CORREÇÃO: Usar Escala Local do RootComponent**

## ❌ **PROBLEMA**

`Get Actor Scale3D` retorna a escala do **mundo** (world scale), não a escala **local** do RootComponent. Isso causa escala incorreta no respawn.

---

## ✅ **SOLUÇÃO: Usar Get Actor Scale ou RootComponent Scale**

### **OPÇÃO 1: Usar Get Actor Scale (Recomendado)**

**Substitua o nó `Get Actor Scale3D` por `Get Actor Scale`:**

1. **Delete o nó `Get Actor Scale3D`**
2. **Adicione o nó `Get Actor Scale`** (procure por "Get Actor Scale" no menu de contexto)
3. **Conecte o `Array Element` do `For Each Loop with Break` ao `Target` do `Get Actor Scale`**
4. **Conecte o `Return Value` (Vector) do `Get Actor Scale` ao `Scale` do `Make Transform`**

**`Get Actor Scale` retorna a escala local do RootComponent, que é o que precisamos.**

---

### **OPÇÃO 2: Usar Get Root Component + Get Relative Scale3D**

**Se `Get Actor Scale` não existir ou não funcionar:**

1. **Adicione `Get Root Component`**
   - Conecte o `Array Element` ao `Target`
   - Retorna um `Scene Component`

2. **Adicione `Get Relative Scale3D`**
   - Conecte o `Return Value` do `Get Root Component` ao `Target`
   - Retorna um `Vector` (a escala local)

3. **Conecte o `Return Value` do `Get Relative Scale3D` ao `Scale` do `Make Transform`**

---

## 📋 **ESTRUTURA CORRETA**

```
[For Each Loop with Break]
    ↓ Array Element
    [Get Actor Location] ← OK
    [Get Actor Rotation] ← OK
    [Get Actor Scale] ← TROCAR AQUI (não Get Actor Scale3D!)
        ↓
    [Make Transform]
        Location: Get Actor Location
        Rotation: Get Actor Rotation
        Scale: Get Actor Scale ← ESCALA LOCAL!
        ↓
    [Add to Array] PlaceholderTransforms
```

---

## ⚠️ **IMPORTANTE**

**`Get Actor Scale3D` = Escala do mundo (pode incluir escala de pais)**
**`Get Actor Scale` = Escala local do RootComponent (o que queremos)**

---

**FIM DA CORREÇÃO**

