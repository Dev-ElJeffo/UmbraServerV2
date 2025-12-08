# 📋 **GUIA: Como Criar o Map ClassIDToPlaceholderClass**

## 🎯 **OBJETIVO**

Criar e preencher um Map que mapeia ClassID (Integer) → PlaceholderClass (Actor Class Reference) no Blueprint.

---

## 📝 **PASSO 1: Criar a Variável Map**

**No `WBP_CreateCharacter`:**

1. Abra o Blueprint `WBP_CreateCharacter`
2. **My Blueprint** → **Variables** → **"+"** (adicionar variável)
3. **Nome:** `ClassIDToPlaceholderClass`
4. **Tipo:** Clique em **"Variable Type"** → procure **"Map"**
5. **Key Type:** Selecione **"Integer"**
6. **Value Type:** Selecione **"Actor Class Reference"**
7. **Category:** "Placeholders" (opcional, para organização)
8. **Compile** e **Save**

**Resultado:**
- Variável criada: `ClassIDToPlaceholderClass` (Map<Integer, Actor Class Reference>)

---

## 📝 **PASSO 2: Preencher o Map no Event Construct**

**No `Event Construct` do `WBP_CreateCharacter`:**

### **2.1. Limpar o Map (Opcional, mas recomendado)**

```
[Event Construct]
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Clear Map]
    • Target: ClassIDToPlaceholderClass
```

**Como fazer:**
1. Arraste a variável `ClassIDToPlaceholderClass` para o Event Graph
2. Selecione **"Get ClassIDToPlaceholderClass"**
3. Arraste do pin de saída
4. Digite: **"Clear Map"**
5. Selecione **"Clear Map"** (da categoria Map)
6. Conecte o pin de saída do **Get** ao pin **"Target"** do **Clear Map**

---

### **2.2. Adicionar Entrada 1: ClassID 1 → BP_Barbarian_Placeholder**

```
[Clear Map] (do passo anterior)
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Add to Map]
    • Target: ClassIDToPlaceholderClass
    • Key: 1 (digite manualmente ou use Make Literal Int)
    • Value: BP_Barbarian_Placeholder (arraste do Content Browser)
```

**Como fazer:**

1. **Arraste a variável `ClassIDToPlaceholderClass`** novamente para o Event Graph
2. Selecione **"Get ClassIDToPlaceholderClass"**
3. **Arraste do pin de saída** do Get
4. Digite: **"Add to Map"**
5. Selecione **"Add to Map"** (da categoria Map)
6. Conecte o pin de saída do **Get** ao pin **"Target"** do **Add to Map**

7. **Para o Key (ClassID 1):**
   - Clique com botão direito no pin **"Key"** do **Add to Map**
   - Selecione **"Convert to Literal"** OU
   - Digite **"Make Literal Int"** e conecte ao pin Key
   - Defina o valor como **1**

8. **Para o Value (BP_Barbarian_Placeholder):**
   - **Content Browser** → Navegue até o Blueprint `BP_Barbarian_Placeholder`
   - **Arraste o Blueprint** do Content Browser para o Event Graph
   - Selecione **"Get Class"** (ou use o pin de saída diretamente se for Actor Class Reference)
   - Conecte ao pin **"Value"** do **Add to Map**

   **⚠️ IMPORTANTE:** Se o Blueprint não aparecer como "Actor Class Reference" automaticamente:
   - Arraste o Blueprint para o Event Graph
   - Selecione **"Get Class"** no menu
   - O **"Get Class"** retorna um tipo genérico, então você pode precisar fazer:
     - Arraste o Blueprint → **"Get Class"** → Conecte ao pin **"Value"**
     - OU use **"Load Class from Asset Path"** com o caminho completo

---

### **2.3. Adicionar Entrada 2: ClassID 2 → BP_Templar_Placeholder**

```
[Add to Map] (ClassID 1)
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Add to Map]
    • Target: ClassIDToPlaceholderClass
    • Key: 2
    • Value: BP_Templar_Placeholder
```

**Repita o mesmo processo:**
- Arraste `ClassIDToPlaceholderClass` → **Get**
- Arraste do Get → **Add to Map**
- Key: **2** (Make Literal Int)
- Value: Arraste `BP_Templar_Placeholder` do Content Browser → **Get Class** → Conecte ao Value

---

### **2.4. Adicionar Todas as Outras Entradas**

**Repita para todos os seus ClassIDs:**

- ClassID 3 → BP_Assassin_Placeholder
- ClassID 4 → BP_Cleric_Placeholder
- ClassID 5 → BP_DarkMage_Placeholder
- ClassID 6 → BP_Monk_Placeholder
- (adicione todos os seus)

**Estrutura final no Event Construct:**

```
[Event Construct]
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Clear Map]
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Add to Map] (Key: 1, Value: BP_Barbarian_Placeholder)
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Add to Map] (Key: 2, Value: BP_Templar_Placeholder)
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Add to Map] (Key: 3, Value: BP_Assassin_Placeholder)
    ↓
... (continue para todos)
    ↓
[Get All Actors of Class] (resto do código)
```

---

## 📝 **PASSO 3: Alternativa - Preencher no Default Value (Mais Simples)**

**Se você preferir preencher diretamente no Editor (sem código):**

1. **My Blueprint** → **Variables** → Selecione `ClassIDToPlaceholderClass`
2. **Details Panel** → **Default Value**
3. Clique em **"+"** para adicionar uma entrada
4. **Key:** Digite o ClassID (ex: 1)
5. **Value:** Clique no dropdown → **"Select Asset"** → Procure `BP_Barbarian_Placeholder`
6. Repita para todas as entradas

**Vantagem:** Não precisa de código no Event Construct!

---

## ✅ **VERIFICAÇÃO**

**Para verificar se o Map está preenchido:**

1. Adicione um **Print String** após o último **Add to Map**
2. Use **"Get Map Keys"** para obter todos os ClassIDs
3. Use **"Get Map Length"** para ver quantas entradas existem

```
[Add to Map] (último)
    ↓
[Get] ClassIDToPlaceholderClass
    ↓
[Get Map Length]
    • Target: ClassIDToPlaceholderClass
    ↓
[Print String]
    • In String: "Map tem " + (Length como String) + " entradas"
```

---

## ⚠️ **PROBLEMAS COMUNS**

### **Problema 1: "Get Class" não conecta ao Value**

**Solução:**
- Use **"Load Class from Asset Path"** em vez de "Get Class"
- Path completo: `/Game/Caminho/BP_Barbarian_Placeholder.BP_Barbarian_Placeholder_C`

### **Problema 2: Blueprint não aparece no Content Browser**

**Solução:**
- Verifique se o Blueprint está compilado
- Verifique se está na pasta correta
- Use **"Load Class from Asset Path"** com o caminho completo

### **Problema 3: Map está vazio quando uso**

**Solução:**
- Verifique se o **Event Construct** está sendo executado
- Adicione **Print String** após cada **Add to Map** para debug
- Verifique se não está limpando o Map depois de preencher

---

**Fim do Guia**

