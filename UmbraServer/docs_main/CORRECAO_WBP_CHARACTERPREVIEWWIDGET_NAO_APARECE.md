# 🔧 CORREÇÃO: WBP_CharacterPreviewWidget não aparece no personagem

## ❌ PROBLEMA

O widget 3D `WBP_CharacterPreviewWidget` não está sendo mostrado acima dos personagens no level.

## ✅ SOLUÇÃO

Verificar e configurar corretamente:
1. O Widget Component no `BP_CharacterPreview`
2. A função `SetPreviewActor` no `WBP_CharacterPreviewWidget`
3. A visibilidade do Widget Component

---

## 📋 PASSO A PASSO

### **1. Verificar Widget Component no BP_CharacterPreview**

1. **Abra o `BP_CharacterPreview`** no Editor
2. **Vá para a aba "Components"**
3. **Procure por um componente chamado "Widget Component"** (ou similar)
4. **Se NÃO existir:** Vá para o passo 2
5. **Se existir:** Vá para o passo 3

---

### **2. Criar Widget Component**

1. **No `BP_CharacterPreview` → Components → + Add Component**
2. **Digite "Widget"** → Selecione **Widget Component**
3. **Renomeie para:** `CharacterWidget` (ou mantenha o nome padrão)
4. **Configure as propriedades:**
   - **Widget Class:** `WBP_CharacterPreviewWidget`
   - **Space:** World
   - **Draw Size:** X = 400, Y = 200
   - **Widget Location:** X = 0, Y = 0, Z = 200 (acima do personagem)
   - **Visibility:** ✅ Visible
   - **Tick Mode:** Always Tick
5. **Compile o Blueprint**

---

### **3. Verificar Configuração do Widget Component**

**No Widget Component, verifique:**

1. **Widget Class:**
   - Deve estar configurado para `WBP_CharacterPreviewWidget`
   - Se estiver vazio ou None, selecione `WBP_CharacterPreviewWidget`

2. **Space:**
   - Deve estar como **World** (não Screen)

3. **Draw Size:**
   - X: 400
   - Y: 200
   - (Ajuste conforme necessário)

4. **Widget Location:**
   - X: 0
   - Y: 0
   - Z: 200 (ou outro valor positivo para ficar acima do personagem)

5. **Visibility:**
   - Deve estar marcado como **Visible**

6. **Compile o Blueprint**

---

### **4. Verificar se WBP_CharacterPreviewWidget existe**

1. **Content Browser** → Procure por `WBP_CharacterPreviewWidget`
2. **Se NÃO existir:** Vá para o passo 5
3. **Se existir:** Vá para o passo 6

---

### **5. Criar WBP_CharacterPreviewWidget**

1. **Content Browser** → **User Interface → Widget Blueprint**
2. **Nome:** `WBP_CharacterPreviewWidget`
3. **Parent Class:** User Widget
4. **Clique em "Create"**

**No Designer:**

```
Canvas Panel
└─ Vertical Box
    └─ Text Block: "TXT_CharacterName"
        • Font Size: 24
        • Color: White
        • Text: "Character Name"
```

**Variáveis:**

1. **Variables → + Variable**
2. **Nome:** `PreviewActor`
3. **Tipo:** **BP_CharacterPreview** (Object Reference)
4. **Access Specifier:** Private
5. **Compile**

---

### **6. Criar Função SetPreviewActor no WBP_CharacterPreviewWidget**

1. **No `WBP_CharacterPreviewWidget` → Functions → + New Function**
2. **Nome da função:** `SetPreviewActor`
3. **Configurações:**
   - **Access Specifier:** Public
   - **Pure:** ❌ FALSE
   - **Call In Editor:** ❌ FALSE

### **7. Adicionar Input**

1. **Na aba "Details" da função, em "Inputs", clique em "+"**
2. **Nome:** `Actor`
3. **Tipo:** **BP_CharacterPreview** (Object Reference)
4. **Clique em "Compile"**

---

### **8. Implementar a Função SetPreviewActor**

**No Graph da função `SetPreviewActor`:**

```
[SetPreviewActor]
    • Input: Actor (BP_CharacterPreview)
    ↓
[Set] PreviewActor (variável) = Actor (input)
    ↓
[Is Valid?] PreviewActor
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Get CharacterData] (do PreviewActor)
        • Target: PreviewActor
        ↓
        [Break Struct] CharacterData
        • Struct: (CharacterData obtido)
        ↓
        [Set Text] TXT_CharacterName
        • Text: (CharacterName - do Break Struct)
```

---

### **9. Passo a Passo Detalhado no Blueprint**

#### **PASSO 1: Set PreviewActor (variável)**

1. **Arraste do exec pin da função** → Digite "Set PreviewActor" → Selecione **Set PreviewActor**
2. **Conecte o exec pin da função** ao **exec pin de Set PreviewActor**
3. **Conecte o Actor (input)** ao **PreviewActor (entrada de Set)**

#### **PASSO 2: Is Valid?**

1. **Arraste do exec pin de Set PreviewActor** → Digite "Is Valid" → Selecione **Is Valid?**
2. **Conecte o PreviewActor (variável)** ao **Object de Is Valid?**
3. **Arraste do Is Valid (saída booleana)** → Digite "Branch" → Selecione **Branch**
4. **Conecte o exec pin de Set PreviewActor** ao **exec pin de Branch**

#### **PASSO 3: Se FALSE (PreviewActor inválido)**

1. **Arraste do False (saída do Branch)** → Digite "Print String" → Selecione **Print String**
2. **No campo "In String"**, digite: `"PreviewActor é inválido!"`

#### **PASSO 4: Se TRUE (PreviewActor válido)**

1. **Arraste do True (saída do Branch)** → Digite "Get CharacterData" → Selecione **Get CharacterData**
2. **Conecte o PreviewActor (variável)** ao **Target de Get CharacterData**
3. **Arraste do exec pin de Get CharacterData** → Digite "Break Umbra Player Data" → Selecione **Break Umbra Player Data**
4. **Conecte o CharacterData (de Get CharacterData)** ao **CharacterData (entrada de Break)**
5. **Arraste do exec pin de Break** → Digite "Set Text" → Selecione **Set Text (Text Block)**
6. **Conecte o TXT_CharacterName (variável do widget)** ao **Target de Set Text**
7. **Conecte o CharacterName (do Break)** ao **Text de Set Text**

---

### **10. Verificar se TXT_CharacterName existe**

1. **No Designer do `WBP_CharacterPreviewWidget`**, verifique se existe um Text Block chamado `TXT_CharacterName`
2. **Se NÃO existir:**
   - Adicione um **Text Block** no Designer
   - Renomeie para `TXT_CharacterName`
   - Marque como **"Is Variable" = TRUE**
3. **Compile o widget**

---

### **11. Verificar se a função SetCharacterData existe (Opcional)**

O C++ também tenta chamar `SetCharacterData` no widget 3D. Se você quiser usar essa função:

1. **Crie uma função `SetCharacterData`** no `WBP_CharacterPreviewWidget`
2. **Input:** `CharacterData` (Umbra Player Data)
3. **Implemente similar ao `SetPreviewActor`**, mas usando o `CharacterData` diretamente do input

**Isso é opcional**, pois o `SetPreviewActor` já obtém o `CharacterData` do `PreviewActor`.

---

### **12. Verificar se SetupPreviewWidget está sendo chamado**

O C++ chama `SetupPreviewWidget` automaticamente quando você spawna os previews via `SpawnAllCharacterPreviews`. 

**Verifique os logs:**
- Se você ver `"✅ Widget 3D configurado com CharacterData para preview X"`, está funcionando
- Se você ver `"⚠️ Widget Component não encontrado no preview X"`, o Widget Component não está configurado

---

## ✅ VERIFICAÇÃO

1. **Compile o C++** (já foi atualizado)
2. **Compile o Blueprint** `BP_CharacterPreview`
3. **Compile o Blueprint** `WBP_CharacterPreviewWidget`
4. **Teste no jogo:**
   - Os personagens devem aparecer no level
   - ✅ O widget 3D deve aparecer acima de cada personagem
   - ✅ O nome do personagem deve estar visível no widget

---

## 🔍 LOGS ESPERADOS

Se tudo estiver funcionando, você verá nos logs:

```
[UmbraCharacterSelectionManager] ✅ Preview spawnado: Nome (ID: X) em ...
[UmbraCharacterSelectionManager] ✅ Widget 3D configurado com CharacterData para preview X
```

---

## ⚠️ NOTA IMPORTANTE

1. O **Widget Component** deve estar configurado no `BP_CharacterPreview`
2. O **Widget Class** deve ser `WBP_CharacterPreviewWidget`
3. O **Space** deve ser **World** (não Screen)
4. A função `SetPreviewActor` **DEVE existir** no `WBP_CharacterPreviewWidget` e **DEVE ser Public**

---

## 🐛 TROUBLESHOOTING

### **Erro: "Widget Component não encontrado no preview"**

**Causa:** O Widget Component não existe ou não está sendo encontrado.

**Solução:**
1. Verifique se o Widget Component existe no `BP_CharacterPreview`
2. Verifique se o nome do componente está correto
3. Recompile o Blueprint `BP_CharacterPreview`

### **Widget não aparece visualmente**

**Causa:** O Widget Component pode estar invisível ou mal posicionado.

**Solução:**
1. Verifique se **Visibility** está marcado como **Visible**
2. Verifique se **Widget Location Z** é positivo (ex: 200)
3. Verifique se **Draw Size** não é muito pequeno
4. Verifique se **Space** está como **World** (não Screen)

### **Erro: "SetPreviewActor não encontrado no widget"**

**Causa:** A função não foi criada ou está com nome diferente.

**Solução:**
1. Verifique se a função se chama exatamente `SetPreviewActor` (case-sensitive)
2. Verifique se a função é **Public** (não Private)
3. Recompile o Blueprint `WBP_CharacterPreviewWidget`

### **Nome do personagem não aparece**

**Causa:** O Text Block não está sendo preenchido ou não existe.

**Solução:**
1. Verifique se `TXT_CharacterName` existe no Designer
2. Verifique se `TXT_CharacterName` está marcado como **"Is Variable" = TRUE**
3. Adicione logs dentro de `SetPreviewActor` para verificar se está sendo executada
4. Verifique se o `CharacterData` está sendo obtido corretamente do `PreviewActor`

---

## ✅ RESUMO

Agora o `WBP_CharacterPreviewWidget` deve:
- ✅ Aparecer acima de cada personagem no level
- ✅ Mostrar o nome do personagem
- ✅ Ser configurado automaticamente quando os previews são spawnados

**Tudo funcionando corretamente!**

