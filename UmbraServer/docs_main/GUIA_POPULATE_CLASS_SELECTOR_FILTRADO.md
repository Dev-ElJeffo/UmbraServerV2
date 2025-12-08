# 🎯 **GUIA: Filtrar PopulateClassSelector para Mostrar Apenas Classe Selecionada**

## 📋 **OBJETIVO**

Modificar a função `PopulateClassSelector` em `WBP_CreateCharacter` para exibir apenas as informações da classe selecionada, em vez de todas as classes.

---

## ✅ **SOLUÇÃO**

Adicionar um parâmetro `ClassID` à função e filtrar o loop para criar o widget apenas quando o `ClassID` corresponder.

---

## 🔧 **PASSO 1: Adicionar Parâmetro ClassID à Função**

1. **Abra o Blueprint `WBP_CreateCharacter`**
2. **Encontre a função `PopulateClassSelector`**
3. **Clique com botão direito na função** → **Add Input** → **Integer**
4. **Renomeie o pin para `ClassID`**

---

## 🔧 **PASSO 2: Modificar o Loop para Filtrar**

No **For Each Loop** dentro de `PopulateClassSelector`:

### **ANTES DO LOOP:**
```
[PopulateClassSelector]
    • ClassID (input) ← NOVO PARÂMETRO
    ↓
[Clear Children]
    • Target: VBox_ClassList
    ↓
[For Each Loop]
    • Array: ClassesArray
```

### **DENTRO DO LOOP BODY:**

Adicione uma verificação **ANTES** de criar o widget:

```
[For Each Loop] (LoopBody)
    • Array Element: (FUmbraClassData)
    ↓
[Break Struct]
    • Struct: Array Element
    ↓
[Get] ClassID (do Break Struct)
    ↓
[Equal] (Integer)
    • A: ClassID (do Break Struct)
    • B: ClassID (input da função)
    ↓
[Branch]
    • Condition: (Equal result)
    ↓ (True) ← Só executa se o ClassID corresponder
    [Print String] (opcional, para debug)
        • In String: "Classe encontrada: {ClassName}"
    ↓
    [Create Widget]
        • Class: WBP_ClassSelectorItem
        • Owning Player: (Get Player Controller)
    ↓
    [Call Function: Set Class Data]
        • Target: (Widget criado)
        • New Class Data: Array Element
    ↓
    [Add Child to Vertical Box]
        • Target: VBox_ClassList
        • Content: (Widget criado)
    ↓ (False)
    [Continue] ← Pula para próxima iteração se não corresponder
```

---

## 📝 **ESTRUTURA COMPLETA DA FUNÇÃO**

```
[PopulateClassSelector]
    • ClassID (input)
    ↓
[Clear Children]
    • Target: VBox_ClassList
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ LoopBody
    [Break Struct]
        • Struct: Array Element
    ↓
    [Get] ClassID (do Break Struct)
    ↓
    [Equal] (Integer)
        • A: ClassID (do Break Struct)
        • B: ClassID (input)
    ↓
    [Branch]
        • Condition: (Equal result)
        ↓ (True)
        [Create Widget]
            • Class: WBP_ClassSelectorItem
            • Owning Player: (Get Player Controller)
        ↓
        [Call Function: Set Class Data]
            • Target: (Widget criado)
            • New Class Data: Array Element
        ↓
        [Add Child to Vertical Box]
            • Target: VBox_ClassList
            • Content: (Widget criado)
        ↓ (False)
        [Continue] (ou simplesmente não conecte nada)
```

---

## 🔧 **PASSO 3: Atualizar Chamadas da Função**

Agora, quando chamar `PopulateClassSelector`, você precisa passar o `ClassID`:

### **No Event Graph ou em outras funções:**

```
[On Clicked] (BTN_Class1)
    ↓
[PopulateClassSelector]
    • ClassID: 1
```

### **Ou usando Get Selected Class ID:**

```
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Selected Class ID]
    ↓
[PopulateClassSelector]
    • ClassID: (Selected Class ID)
```

---

## ✅ **RESULTADO ESPERADO**

- Quando `PopulateClassSelector` for chamado com `ClassID = 1`, apenas o widget da classe Barbarian será criado
- Quando chamado com `ClassID = 2`, apenas o widget da classe Templar será criado
- E assim por diante...

---

## 🎯 **EXEMPLO DE USO NO SelectClassByID**

Se você tem uma função `SelectClassByID` que já seleciona a classe:

```
[SelectClassByID]
    • ClassID: 1
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Call Function: Select Class]
    • ClassID: 1
    ↓
[PopulateClassSelector] ← ADICIONE AQUI
    • ClassID: 1
```

---

## 📌 **NOTAS IMPORTANTES**

1. **Clear Children**: Mantenha o `Clear Children` no início para limpar widgets anteriores
2. **Break Struct**: Use `Break Struct` para acessar o `ClassID` do `Array Element`
3. **Branch**: O `Branch` garante que apenas a classe correspondente seja processada
4. **Continue**: Opcional, mas ajuda a deixar o código mais claro

---

## 🔍 **DEBUG**

Se quiser verificar se está funcionando, adicione um `Print String` antes do `Branch`:

```
[Get] ClassID (do Break Struct)
    ↓
[Print String]
    • In String: "Verificando ClassID: {ClassID}"
    • {ClassID}: (conecte o ClassID do Break Struct)
    ↓
[Equal] (Integer)
```

Isso mostrará no log quais classes estão sendo verificadas.

