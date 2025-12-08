# 📋 **GUIA: Criar Função ClearCharacterCreationList no WBP_CreateCharacter**

## 🎯 **OBJETIVO**

Criar a função `ClearCharacterCreationList` no widget `WBP_CreateCharacter` que limpa o VBox `VB_InfoandCreate`, similar ao que foi feito no `WBP_CharacterSelection` com `ClearCharacterList`.

---

## 📝 **PASSO A PASSO**

### **PASSO 1: Abrir WBP_CreateCharacter**

1. No **Content Browser**, navegue até `WBP_CreateCharacter`
2. **Duplo clique** para abrir o widget

---

### **PASSO 2: Criar a Função ClearCharacterCreationList**

1. No **My Blueprint** (painel esquerdo), clique em **Functions** → **+ Function**
2. Nome da função: `ClearCharacterCreationList`
3. **Category:** "Character Creation" (opcional, para organização)

---

### **PASSO 3: Implementar a Função**

**No Event Graph, abra a função `ClearCharacterCreationList`:**

```
[ClearCharacterCreationList] (Function Entry)
    ↓
[Get] VB_InfoandCreate
    ↓
[Is Valid]
    • Object: VB_InfoandCreate
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [Clear Children]
        • Target: VB_InfoandCreate
```

**Estrutura completa:**

```
ClearCharacterCreationList
    ↓
[Get] VB_InfoandCreate
    ↓
[Is Valid]
    • Object: VB_InfoandCreate
    ↓
[Branch]
    • Condition: (Is Valid)
    ├─→ [TRUE] → [Clear Children]
    │              • Target: VB_InfoandCreate
    │              ↓
    │              [Print String] (opcional, para debug)
    │                  • In String: "VB_InfoandCreate limpo!"
    │                  • bPrintToScreen: true
    └─→ [FALSE] → [Print String] (opcional, para debug)
                      • In String: "ERRO: VB_InfoandCreate é nullptr!"
                      • bPrintToScreen: true
```

---

## ✅ **RESULTADO**

A função `ClearCharacterCreationList` agora:
- ✅ Limpa todos os widgets filhos do `VB_InfoandCreate`
- ✅ É chamada automaticamente pelo C++ quando `ResetCharacterCreation` é executado
- ✅ Funciona exatamente como `ClearCharacterList` no `WBP_CharacterSelection`

---

## 🔧 **COMO USAR**

**No Blueprint, quando quiser resetar a criação de personagem:**

### **No Event Graph do WBP_CreateCharacter:**

```
[Botão Return ou Cancel] (OnClicked)
    ↓
[Play Sound 2D] (opcional)
    ↓
[Get] Manager (variável do widget)
    ↓
[Is Valid]
    • Object: Manager
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [Call Function: Reset Character Creation]
        • Target: Manager
        • Create Character Widget: Self ← IMPORTANTE! Use "Self" aqui
        • Placeholder Class: BP_Class_Placeholder (ou a classe base)
        • Spacing: 300.0
```

### **⚠️ IMPORTANTE: CreateCharacterWidget**

**No pino `CreateCharacterWidget` da função `ResetCharacterCreation`:**

1. **Arraste do pino de execução** (ou de qualquer nó)
2. **Digite "Self"** ou **"Get Self"**
3. **Selecione "Get Self"** (retorna o próprio widget)
4. **Conecte o "Return Value" ao pino `CreateCharacterWidget`**

**OU:**

1. **Clique com botão direito** no pino `CreateCharacterWidget`
2. **Selecione "Split Struct Pin"** (se necessário)
3. **Digite "Self"** no campo de busca
4. **Selecione "Get Self"** e conecte

**A função `ClearCharacterCreationList` será chamada automaticamente pelo C++!**

---

## 📌 **NOTAS IMPORTANTES**

1. **VB_InfoandCreate deve existir** no widget `WBP_CreateCharacter`
2. **A função não precisa de parâmetros** - ela apenas limpa o VBox
3. **O C++ tenta chamar essa função primeiro**, se não encontrar, usa reflection como fallback
4. **`CreateCharacterWidget` deve receber `Self`** - como você está dentro do próprio widget `WBP_CreateCharacter`, use `Get Self` para passar a referência do próprio widget

## 🔍 **SOLUÇÃO PARA O PROBLEMA**

**Se o botão não está fazendo nada, verifique:**

1. ✅ **`CreateCharacterWidget` está conectado?** → Deve conectar `Self` (Get Self)
2. ✅ **`Manager` está válido?** → Verifique se a variável `Manager` está setada no widget
3. ✅ **A função `ClearCharacterCreationList` existe?** → Verifique se foi criada no widget
4. ✅ **`VB_InfoandCreate` existe?** → Verifique se o VBox está no widget

**Exemplo visual da conexão:**

```
[Get Self]
    ↓ Return Value
[Reset Character Creation]
    • Create Character Widget: (conectado aqui)
```

---

**Fim do Guia**

