# 🎯 **GUIA: Criar Função FindPlaceholderByClassID no Blueprint**

## 📋 **OBJETIVO**

Criar a função `FindPlaceholderByClassID` no Blueprint `BP_CharacterCreationManager` que busca um placeholder pelo seu `ClassID`.

---

## ✅ **PASSO A PASSO COMPLETO**

### **PASSO 1: Criar a Função**

1. **Abra o Blueprint `BP_CharacterCreationManager`**
2. No painel **My Blueprint** (lado esquerdo), encontre a seção **Functions**
3. Clique no botão **+ Function** (ou botão direito → **New Function**)
4. Nomeie como: `FindPlaceholderByClassID`
5. A função será criada e aberta automaticamente no Event Graph

### **PASSO 2: Configurar Inputs e Outputs**

1. **Com a função aberta**, no painel **Details** (lado direito):
   - Se não aparecer, selecione o nó **Function Entry** (nó roxo no início da função)

2. **Adicionar Input:**
   - No painel **Details**, encontre a seção **Inputs**
   - Clique no botão **+ Input**
   - Nomeie como: `ClassID`
   - Tipo: **Integer**
   - ✅ Marque **Expose on Spawn** como **false** (não precisa)

3. **Adicionar Output:**
   - No painel **Details**, encontre a seção **Outputs**
   - Clique no botão **+ Output**
   - Nomeie como: `FoundPlaceholder`
   - Tipo: **BP_Class_Placeholder** (Object Reference)
     - Se não aparecer, use: **Object Reference** e depois mude para `BP_Class_Placeholder`
   - ✅ Marque **Expose on Spawn** como **false**

### **PASSO 3: Verificar Variável AllPlaceholders**

Antes de implementar, certifique-se de que a variável `AllPlaceholders` existe:

1. No painel **My Blueprint** → **Variables**
2. Procure por `AllPlaceholders`
3. Se **NÃO existir**, crie:
   - Clique em **+ Variable**
   - Nomeie como: `AllPlaceholders`
   - Tipo: **Array** → **Array of BP_Class_Placeholder** (Object Reference)
   - ✅ Marque **Instance Editable** como **false**
   - ✅ Marque **Expose on Spawn** como **false**

### **PASSO 4: Implementar a Lógica**

No **Event Graph** da função `FindPlaceholderByClassID`:

```
[FindPlaceholderByClassID] (Function Entry - nó roxo)
    • ClassID (input - laranja)
    ↓ (exec - branco)
[Get] AllPlaceholders
    ↓
[For Each Loop]
    • Array: AllPlaceholders (conecte aqui)
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder - saída automática)
    • Array Index: (opcional - não precisa)
    ↓
[Get] ClassID (do Array Element)
    ↓
[Equal] (Integer)
    • A: ClassID (do Get)
    • B: ClassID (input da função)
    ↓
[Branch]
    • Condition: (resultado do Equal)
    ↓ (True)
    [SET] FoundPlaceholder = Array Element
    [Break] (do For Each Loop)
    ↓ (False)
    [Continue Loop] (volta para próximo item)
```

**Passo a Passo Detalhado:**

#### **4.1. Adicionar Get AllPlaceholders**

1. No Event Graph da função
2. Arraste do pino **exec** (branco) do nó `FindPlaceholderByClassID`
3. Digite: `Get AllPlaceholders` ou arraste a variável `AllPlaceholders` do painel **My Blueprint**
4. Selecione: **Get AllPlaceholders**

#### **4.2. Adicionar For Each Loop**

1. Arraste do pino de saída de `Get AllPlaceholders`
2. Digite: `For Each Loop`
3. Selecione: **For Each Loop**
4. Conecte:
   - **Array:** Conecte o pino de saída de `Get AllPlaceholders` ao pino **Array** do `For Each Loop`

#### **4.3. Adicionar Get ClassID do Array Element**

1. No **Loop Body** do `For Each Loop`
2. Arraste do pino **Array Element** (saída do `For Each Loop`)
3. Digite: `Get ClassID` ou `Get`
4. Selecione: **Get ClassID**
   - ⚠️ Se não aparecer, você precisa expor a variável `ClassID` no `BP_Class_Placeholder`:
     - Abra `BP_Class_Placeholder`
     - No painel **My Blueprint** → **Variables**
     - Selecione `ClassID`
     - No **Details**, marque **Instance Editable** como **true**
     - Ou marque **Public** como **true**

#### **4.4. Adicionar Comparação (Equal)**

1. Arraste do pino de saída de `Get ClassID`
2. Digite: `Equal` ou `==`
3. Selecione: **Equal (Integer)**
4. Conecte:
   - **A:** Conecte o pino de saída de `Get ClassID`
   - **B:** Conecte o pino `ClassID` (input da função)

#### **4.5. Adicionar Branch**

1. Arraste do pino **exec** do `For Each Loop` (dentro do Loop Body)
2. Digite: `Branch`
3. Selecione: **Branch**
4. Conecte:
   - **Condition:** Conecte o pino de saída do `Equal`

#### **4.6. Adicionar SET FoundPlaceholder (True Branch)**

1. Arraste do pino **True** (verde) do `Branch`
2. Digite: `SET` ou `Set FoundPlaceholder`
3. Selecione: **SET FoundPlaceholder**
4. Conecte:
   - **Target:** Deixe vazio (ou `Self`)
   - **FoundPlaceholder:** Conecte o pino **Array Element** (do `For Each Loop`)

#### **4.7. Adicionar Break (True Branch)**

1. Arraste do pino **exec** (saída) do `SET FoundPlaceholder`
2. Digite: `Break`
3. Selecione: **Break** (deve aparecer como "Break For Each Loop")
4. Isso interrompe o loop quando encontra o placeholder

#### **4.8. Adicionar Continue (False Branch)**

1. Arraste do pino **False** (vermelho) do `Branch`
2. Digite: `Continue`
3. Selecione: **Continue** (deve aparecer como "Continue For Each Loop")
4. Isso continua o loop para o próximo item

### **PASSO 5: Conectar Output**

1. No nó **Function Entry** (roxo), você verá o pino **FoundPlaceholder** (output)
2. Este pino já está conectado automaticamente à variável `FoundPlaceholder`
3. Quando você faz `SET FoundPlaceholder`, o valor é automaticamente retornado

**⚠️ IMPORTANTE:** Se o output não estiver funcionando:
- Verifique se o output `FoundPlaceholder` está configurado no **Details**
- Certifique-se de que o `SET FoundPlaceholder` está sendo executado antes do `Break`

---

## 🔍 **ESTRUTURA VISUAL COMPLETA**

```
[FindPlaceholderByClassID] (Function Entry)
    • ClassID (input)
    ↓
[Get] AllPlaceholders
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
    [Get] ClassID (do Array Element)
        ↓
    [Equal] (Integer)
        • A: ClassID (do Get)
        • B: ClassID (input)
        ↓
    [Branch]
        • Condition: (Equal result)
        ↓
        ├─ True:
        │   ↓
        │   [SET] FoundPlaceholder = Array Element
        │   ↓
        │   [Break] (sai do loop)
        │
        └─ False:
            ↓
            [Continue] (próximo item)
```

---

## ⚠️ **PROBLEMAS COMUNS E SOLUÇÕES**

### **Problema 1: Não consigo acessar Get ClassID do Array Element**

**Solução:**
1. Abra o Blueprint `BP_Class_Placeholder`
2. No painel **My Blueprint** → **Variables**
3. Selecione `ClassID`
4. No **Details**, marque:
   - ✅ **Instance Editable:** true
   - ✅ **Public:** true (opcional, mas ajuda)

### **Problema 2: For Each Loop não aparece**

**Solução:**
- Digite: `For Each` ou `Loop`
- Certifique-se de que está no **Event Graph** (não no **Construction Script**)

### **Problema 3: Output não retorna valor**

**Solução:**
- Verifique se o output `FoundPlaceholder` está configurado no **Details** do nó **Function Entry**
- Certifique-se de que o `SET FoundPlaceholder` está sendo executado
- O output retorna automaticamente o último valor setado

### **Problema 4: Array Element não é do tipo correto**

**Solução:**
- Verifique se `AllPlaceholders` é do tipo **Array of BP_Class_Placeholder**
- Se for **Array of Actor**, você precisa fazer **Cast to BP_Class_Placeholder** antes de usar

---

## 📝 **ALTERNATIVA: Usar Get All Actors of Class**

Se você não quiser usar a variável `AllPlaceholders`, pode buscar diretamente:

```
[FindPlaceholderByClassID]
    • ClassID (input)
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado do Get All Actors)
    ↓ Loop Body
    • Array Element: (Actor)
    ↓
[Cast to BP_Class_Placeholder]
    • Object: Array Element
    ↓ (Success)
[Get] ClassID (do cast result)
    ↓
[Equal] (Integer)
    • A: ClassID (do Get)
    • B: ClassID (input)
    ↓
[Branch]
    • Condition: (Equal result)
    ↓ (True)
    [SET] FoundPlaceholder = (cast result)
    [Break]
```

---

## ✅ **CHECKLIST**

- [ ] Função `FindPlaceholderByClassID` criada
- [ ] Input `ClassID` (Integer) adicionado
- [ ] Output `FoundPlaceholder` (BP_Class_Placeholder Reference) adicionado
- [ ] Variável `AllPlaceholders` existe e está populada
- [ ] `For Each Loop` implementado
- [ ] `Get ClassID` do Array Element funciona
- [ ] Comparação `Equal` implementada
- [ ] `Branch` implementado
- [ ] `SET FoundPlaceholder` no True branch
- [ ] `Break` após o SET
- [ ] `Continue` no False branch
- [ ] Função compila sem erros

---

**Fim do Guia**

