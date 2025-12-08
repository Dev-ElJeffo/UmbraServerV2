# 🎯 **GUIA: Configurar CameraActor do Nível como Câmera Principal**

## 📋 **OBJETIVO**

Fazer com que o `CameraActor` do nível seja usado como câmera principal quando o nível abre, em vez da câmera do Character.

---

## ✅ **SOLUÇÃO: Função C++ no Player Controller**

A classe `AUmbraEternumUEPlayerController` já possui duas funções:

1. **`SetLevelCameraActor`** - Configura um Actor específico como câmera
2. **`FindAndSetLevelCameraActor`** - Encontra automaticamente o `CameraActor` no nível

---

## 🔧 **OPÇÃO 1: Usar no Blueprint (Recomendado)**

### **PASSO 1: Criar/Editar Game Mode Blueprint**

1. **No Content Browser**, encontre ou crie o Game Mode usado no nível `Lvl_Character_Creation`
2. **Abra o Blueprint**

### **PASSO 2: Implementar no Event BeginPlay**

No **Event Graph** do Game Mode:

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Cast to Umbra Player Controller]
    ↓ (Success)
    [Call Function: Find And Set Level Camera Actor]
        • Target: (cast result)
        • Blend Time: 0.0 (instantâneo) ou 1.0 (transição suave)
```

**Pronto!** A câmera será configurada automaticamente ao abrir o nível.

---

## 🔧 **OPÇÃO 2: Usar no BP_CharacterCreationManager**

Se você quiser configurar no `BP_CharacterCreationManager`:

### **PASSO 1: No Event BeginPlay do BP_CharacterCreationManager**

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Cast to Umbra Player Controller]
    ↓ (Success)
    [Get All Actors of Class]
        • Actor Class: Actor
        • Out Actors: (variável local)
        ↓
    [For Each Loop]
        • Array: Out Actors
        ↓ Loop Body
        • Array Element: (Actor)
        ↓
    [Get Actor Name]
        • Target: Array Element
        ↓
    [Equal] (String)
        • A: (Actor Name)
        • B: "CameraActor"
        ↓
    [Branch]
        • Condition: (Equal result)
        ↓ (True)
        [Call Function: Set Level Camera Actor]
            • Target: (Player Controller)
            • Camera Actor: Array Element
            • Blend Time: 0.0
        [Break]
```

---

## 🔧 **OPÇÃO 3: Configurar Manualmente (Mais Controle)**

Se você já tem uma referência ao `CameraActor`:

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Cast to Umbra Player Controller]
    ↓ (Success)
    [Get] CameraActor (sua variável)
        ↓
    [Call Function: Set Level Camera Actor]
        • Target: (Player Controller)
        • Camera Actor: CameraActor
        • Blend Time: 0.0 (ou 1.0 para transição suave)
```

---

## ⚙️ **PARÂMETROS**

### **FindAndSetLevelCameraActor**
- `Blend Time` (Float) - Tempo de transição suave
  - `0.0` = Instantâneo (padrão)
  - `1.0` = Transição suave de 1 segundo

### **SetLevelCameraActor**
- `Camera Actor` (Actor Reference) - O Actor que será usado como câmera
- `Blend Time` (Float) - Tempo de transição suave (padrão: 0.0)

---

## ✅ **VERIFICAÇÃO**

Após implementar:

1. **Compile o C++** (se ainda não compilou)
2. **Abra o nível `Lvl_Character_Creation`**
3. **Pressione Play**
4. **A câmera deve estar na posição do `CameraActor`**

---

## ⚠️ **IMPORTANTE: Não Afeta Outros Níveis**

**✅ SEGURO:** As funções só são executadas quando você as chama explicitamente no Blueprint.

- ❌ **NÃO são chamadas automaticamente** no `BeginPlay` do Player Controller
- ❌ **NÃO afetam outros níveis** que não chamarem essas funções
- ✅ **Outros níveis continuam funcionando normalmente** com a câmera padrão do Character

**Se quiser garantir ainda mais, adicione uma verificação de nome do nível:**

```
[Event BeginPlay]
    ↓
[Get Level Name]
    ↓
[Contains] (String)
    • Search In: (Level Name)
    • Substring: "Character_Creation"
    ↓
[Branch]
    • Condition: (Contains result)
    ↓ (True - só neste nível)
    [Get Player Controller]
        • Player Index: 0
        ↓
    [Cast to Umbra Player Controller]
        ↓ (Success)
        [Call Function: Find And Set Level Camera Actor]
            • Target: (cast result)
            • Blend Time: 0.0
```

Assim, a função só será chamada em níveis que contenham "Character_Creation" no nome.

---

## 🎯 **RESUMO**

**Método mais simples (OPÇÃO 1):**
1. No Game Mode Blueprint
2. Event BeginPlay
3. Get Player Controller → Cast to Umbra Player Controller
4. Call Function: Find And Set Level Camera Actor
5. **Pronto!**

A função C++ encontra automaticamente o `CameraActor` no nível e configura como câmera principal.

---

**Fim do Guia**

