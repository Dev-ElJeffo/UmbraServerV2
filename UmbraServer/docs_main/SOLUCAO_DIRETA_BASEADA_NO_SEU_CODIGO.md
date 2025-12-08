# ✅ **SOLUÇÃO DIRETA: Baseada no Seu Código**

## 🔍 **O QUE EU VI NO SEU CÓDIGO**

No `BTN_Class1 OnClicked`, você já tem:
1. ✅ `SelectClassAndMoveCamera` (ClassID: 1)
2. ✅ `PopulateClassSelector` (ClassID: 1) ← **JÁ ESTÁ AQUI!**
3. ✅ `Set Visibility` Border_WBP_Creator → Visible ← **JÁ ESTÁ AQUI!**

**O problema é que na segunda vez não funciona.**

---

## 🎯 **SOLUÇÃO: Verificar PopulateClassSelector**

### **PASSO 1: Abrir PopulateClassSelector**

**No `WBP_CreateCharacter`:**
1. **My Blueprint** → **Functions** → `PopulateClassSelector`
2. **Abra a função**

### **PASSO 2: Verificar se Limpa o VBox**

**No início de `PopulateClassSelector`, DEVE ter:**

```
[PopulateClassSelector]
    • ClassID (input)
    ↓
[Get] VBox_ClassList
    ↓
[Clear Children] ← SE NÃO TIVER, ADICIONE!
    • Target: VBox_ClassList
    ↓
[For Each Loop]
    • Array: ClassesArray
```

**Se NÃO tiver `Clear Children`, adicione ANTES do `For Each Loop`.**

---

### **PASSO 3: Verificar se Filtra por ClassID**

**Dentro do `For Each Loop`, DEVE ter:**

```
[For Each Loop] (LoopBody)
    • Array Element: (FUmbraClassData)
    ↓
[Break Umbra Class Data]
    • Struct: Array Element
    ↓
[Get] ClassID (do Break)
    ↓
[Equal] (Integer)
    • A: ClassID (do Break)
    • B: ClassID (input da função)
    ↓
[Branch]
    • Condition: (Equal result)
    ↓ (True) ← SÓ EXECUTA SE CORRESPONDER
    [Create Widget] WBP_ClassSelectorItem
    ↓
    [Call Function: SetClassData]
    ↓
    [Get] VBox_ClassList
    ↓
    [Add Child to Vertical Box]
        • Target: VBox_ClassList
        • Content: (Widget criado)
    ↓ (False)
    [Continue] ← Pula se não corresponder
```

**Se NÃO tiver essa verificação, adicione.**

---

## 🔧 **SE AINDA NÃO FUNCIONAR: Adicionar Delay**

**Pode ser problema de timing. Adicione um pequeno delay:**

```
[SelectClassAndMoveCamera]
    • Return Value: Success
    ↓
[Branch] (Success = True)
    ↓
[Delay]
    • Duration: 0.1
    ↓
[Call Function: PopulateClassSelector]
    • ClassID: 1
    ↓
[Get] Border_WBP_Creator
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible
```

---

## 🔧 **ALTERNATIVA: Forçar Visibilidade DEPOIS**

**Adicione um segundo `Set Visibility` no final:**

```
[Set Visibility] Border_WBP_Creator → Visible
    ↓
[Delay]
    • Duration: 0.2
    ↓
[Get] Border_WBP_Creator
    ↓
[Set Visibility]
    • Target: Border_WBP_Creator
    • New Visibility: Visible ← FORÇAR NOVAMENTE
```

---

## ✅ **CHECKLIST RÁPIDO**

1. **`PopulateClassSelector` tem `Clear Children` no início?**
   - Se NÃO → Adicione

2. **`PopulateClassSelector` filtra por ClassID no loop?**
   - Se NÃO → Adicione a verificação

3. **O `Set Visibility` está sendo executado?**
   - Adicione um `Print String` após para verificar

4. **Há algum código que esconde o Border depois?**
   - Procure por outros `Set Visibility` → Collapsed

---

## 🎯 **TESTE RÁPIDO**

**Adicione logs para ver o que acontece:**

```
[PopulateClassSelector]
    • ClassID (input)
    ↓
[Print String]
    • In String: "🔥 PopulateClassSelector CHAMADO com ClassID: " + (ClassID como String)
    • bPrintToScreen: true
    • Text Color: (255, 255, 0) ← Amarelo
    ↓
[Get] VBox_ClassList
    ↓
[Clear Children]
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ LoopBody
    [Break Umbra Class Data]
        ↓
    [Get] ClassID (do Break)
        ↓
    [Equal] (Integer)
        • A: ClassID (do Break)
        • B: ClassID (input)
        ↓
    [Branch] (True)
        ↓
    [Print String]
        • In String: "✅ Classe encontrada! ClassID: " + (ClassID do Break como String)
        • bPrintToScreen: true
        • Text Color: (0, 255, 0) ← Verde
        ↓
    [Create Widget] WBP_ClassSelectorItem
        ↓
    [Add Child to Vertical Box]
        ↓
    [Print String]
        • In String: "✅ Widget adicionado ao VBox!"
        • bPrintToScreen: true
        • Text Color: (0, 255, 0) ← Verde
```

**Execute e veja quais logs aparecem na segunda vez.**

---

**Fim do Guia**

