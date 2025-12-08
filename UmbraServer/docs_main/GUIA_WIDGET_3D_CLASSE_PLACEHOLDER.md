# 📋 **GUIA: Widget 3D no Class Placeholder**

## 🎯 **OBJETIVO**

Adicionar um Widget 3D em cada `BP_Class_Placeholder` com nome da classe e botão "CRIAR" que seleciona o actor, faz highlight e move a câmera.

---

## 📝 **PASSO 1: Criar Widget Blueprint**

### **1.1. Criar o Widget**

1. **Content Browser** → pasta de widgets
2. **Botão direito** → **User Interface** → **Widget Blueprint**
3. Nome: `WBP_ClassPlaceholderWidget`
4. Abrir o widget

---

### **1.2. Estrutura do Widget**

**Hierarquia:**
```
Canvas Panel (Root)
└─ Vertical Box
    ├─ Text: TXT_ClassName
    └─ Button: BTN_Create
        └─ Text: "CRIAR"
```

**Configurações:**

**TXT_ClassName:**
- **Size:** Auto
- **Font Size:** 24
- **Color:** Branco
- **Justification:** Center

**BTN_Create:**
- **Size:** Width: 200, Height: 50
- **Background Color:** Azul/Verde

---

### **1.3. Variáveis do Widget**

**No "My Blueprint" → "Variables":**

1. **ClassID** (Integer)
   - Category: "Class"
   - Expose on Spawn: ✅

2. **ClassName** (Text)
   - Category: "Class"
   - Expose on Spawn: ✅

3. **PlaceholderActor** (Object Reference → BP_Class_Placeholder)
   - Category: "Class"
   - Expose on Spawn: ✅

---

### **1.4. Função SetClassData**

**Criar função `SetClassData`:**

```
[SetClassData]
    • ClassID (input - Integer)
    • ClassName (input - Text)
    • PlaceholderActor (input - BP_Class_Placeholder)
    ↓
[SET] ClassID = ClassID (input)
[SET] ClassName = ClassName (input)
[SET] PlaceholderActor = PlaceholderActor (input)
    ↓
[Set Text]
    • Target: TXT_ClassName
    • Text: ClassName (input)
```

---

### **1.5. Evento On Clicked do Botão**

**No `BTN_Create`, no "On Clicked":**

```
[On Clicked] (BTN_Create)
    ↓
[Get] PlaceholderActor
    ↓
[Is Valid]
    • Object: PlaceholderActor
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [Get] ClassID (do PlaceholderActor)
        ↓
    [Call Function: SelectClass]
        • Target: PlaceholderActor
        • ClassID: ClassID
```

---

## 📝 **PASSO 2: Adicionar Widget Component no BP_Class_Placeholder**

### **2.1. Adicionar Componente**

1. Abrir `BP_Class_Placeholder`
2. **Components** → **"Add Component"**
3. Procurar **"Widget"** → **"Widget Component"**
4. Nome: `Widget_ClassInfo`

---

### **2.2. Configurar Widget Component**

**No `Widget_ClassInfo` (Details):**

1. **Widget Class:** `WBP_ClassPlaceholderWidget`
2. **Draw Size:**
   - X: 400
   - Y: 200
3. **Pivot:**
   - X: 0.5
   - Y: 0.5
4. **Space:** `World`
5. **Widget Space:** `Screen`

---

### **2.3. Posicionar Widget**

**No `Widget_ClassInfo` (Transform):**

- **Location:**
  - X: 0
  - Y: 0
  - Z: 200 (acima do personagem)
- **Rotation:** 0, 0, 0
- **Scale:** 1, 1, 1

---

### **2.4. No Event BeginPlay do BP_Class_Placeholder**

```
[Event BeginPlay]
    ↓
[Get] Widget_ClassInfo
    ↓
[Get User Widget Object]
    • Target: Widget_ClassInfo
    ↓
[Cast to WBP_ClassPlaceholderWidget]
    • Object: (User Widget Object)
    ↓ (Success)
    [Get] ClassID
        ↓
    [Get] ClassName (variável do BP_Class_Placeholder - se não tiver, criar)
        ↓
    [Call Function: SetClassData]
        • Target: (Cast result)
        • ClassID: ClassID
        • ClassName: ClassName
        • PlaceholderActor: Self
```

**NOTA:** Se `BP_Class_Placeholder` não tiver variável `ClassName`, criar uma variável Text chamada `ClassName`.

---

## 📝 **PASSO 3: Atualizar SelectClass**

### **3.1. No BP_Class_Placeholder, função SelectClass**

```
[SelectClass]
    • ClassID (input)
    ↓
[SET] bIsSelected = true
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    • Return Value: Manager (variável local)
    ↓
[Call Function: RemoveHighlightsFromOthers]
    • Target: Manager
    • SelectedPlaceholder: Self
    ↓
[Call Function: HighlightSelected]
    ↓
[Call Function: Find Camera Actor]
    • Target: Manager
    • Return Value: CameraActor
    ↓
[Call Function: Move Camera To Selection]
    • Target: Manager
    • Target Placeholder: Self
    • Camera Actor: CameraActor
    • Duration: 1.0
    • Offset X: 200.0
    • Offset Z: 100.0
    ↓
[Broadcast] OnClassSelected
    • ClassID: ClassID
```

---

## 📝 **PASSO 4: Função HighlightSelected**

### **4.1. No BP_Class_Placeholder**

**Função `HighlightSelected`:**

```
[HighlightSelected]
    ↓
[Get] PostProcess_Highlight (ou criar componente de highlight)
    ↓
[Set Visibility]
    • Target: PostProcess_Highlight
    • New Visibility: Visible
    ↓
[Set Actor Scale]
    • Target: Self
    • New Scale: (1.1, 1.1, 1.1)
```

**OU** usar um **Post Process Volume** ou **Material** para highlight.

---

## 📝 **PASSO 5: Função RemoveHighlight**

### **5.1. No BP_Class_Placeholder**

**Função `RemoveHighlight`:**

```
[RemoveHighlight]
    ↓
[Get] PostProcess_Highlight
    ↓
[Set Visibility]
    • Target: PostProcess_Highlight
    • New Visibility: Hidden
    ↓
[Set Actor Scale]
    • Target: Self
    • New Scale: (1.0, 1.0, 1.0)
```

---

## 📝 **PASSO 6: Adicionar Variável ClassName no BP_Class_Placeholder**

### **6.1. Se não existir**

1. Abrir `BP_Class_Placeholder`
2. **My Blueprint** → **Variables** → **"+"**
3. Nome: `ClassName`
4. Tipo: **Text**
5. **Compile** e **Save**

---

## ✅ **RESUMO DO FLUXO**

1. Widget 3D aparece sobre cada placeholder com nome da classe e botão
2. Jogador clica no botão "CRIAR"
3. Widget chama `SelectClass` no placeholder
4. `SelectClass`:
   - Remove highlight dos outros (via `RemoveHighlightsFromOthers`)
   - Adiciona highlight no selecionado (via `HighlightSelected`)
   - Move a câmera (via `MoveCameraToSelection`)
   - Broadcast `OnClassSelected`
5. `BP_CharacterCreationManager` recebe o broadcast e atualiza a UI

---

## 🎯 **ORDEM DE IMPLEMENTAÇÃO**

1. ✅ Criar `WBP_ClassPlaceholderWidget` (Passo 1)
2. ✅ Adicionar `Widget Component` no `BP_Class_Placeholder` (Passo 2)
3. ✅ Configurar `Event BeginPlay` para inicializar widget (Passo 2.4)
4. ✅ Atualizar `SelectClass` (Passo 3)
5. ✅ Criar/Atualizar `HighlightSelected` (Passo 4)
6. ✅ Criar/Atualizar `RemoveHighlight` (Passo 5)
7. ✅ Adicionar variável `ClassName` se necessário (Passo 6)

---

**Fim do Guia**
