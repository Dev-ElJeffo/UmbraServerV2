# 🔍 **ONDE ESTÁ O INITIALIZE - GUIA VISUAL**

## 📍 **LOCALIZAÇÃO EXATA**

### **No Level Blueprint do `Lvl_CharacterSelection`:**

1. **Abra o Level Blueprint:**
   - Com o level `Lvl_CharacterSelection` aberto
   - **Blueprints → Open Level Blueprint**

2. **No Event Graph, procure por:**
   - **"Call Function: Initialize"** OU
   - **"Initialize"** (nó de função)

3. **Onde ele DEVE estar:**
   - **Após** `Get Current Players`
   - **Após** `Get Array Length`
   - **Após** `Greater` (Length > 0)
   - **Após** `Branch`
   - **Na saída TRUE do Branch**

---

## 🎯 **SE VOCÊ NÃO ENCONTROU O NÓ:**

### **Crie o nó agora:**

1. **No Event Graph do Level Blueprint:**
   - Clique com botão direito no espaço vazio
   - Digite: **"Initialize"**
   - Selecione: **"Initialize"** (deve aparecer como função do `BP_CharacterSelectionManager`)

2. **Conecte os pinos:**
   - **Target**: Conecte ao **Manager** (variável do Level Blueprint ou resultado do Spawn Actor)
   - **Player Controller**: Conecte ao **Player Controller** (de Get Player Controller)
   - **Widget**: Conecte ao **Widget** (variável do Level Blueprint ou resultado do Create Widget)

3. **Configure os parâmetros:**
   - **Preview Class**: Clique no campo → Selecione **BP_CharacterPreview**
   - **Spacing**: Digite **300.0**

---

## 📝 **GRAPH COMPLETO (Onde está o Initialize):**

```
[Event BeginPlay]
    ↓
[Delay] 0.2s
    ↓
[Get Player Controller]
    ↓
[Spawn Actor] BP_CharacterSelectionManager
    ↓
[Set Manager] ← Salvar referência
    ↓
[Create Widget] WBP_CharacterSelection
    ↓
[Set Widget] ← Salvar referência
    ↓
[Add to Viewport]
    ↓
[Set Input Mode Game And UI]
    ↓
[Set Show Mouse Cursor] TRUE
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Get Current Players]
    ↓
[Get Array Length]
    ↓
[Greater] Length > 0
    ↓
[Branch]
    ├─→ [TRUE] →
    │       ↓
    │       [Get Manager] ← Pegar variável salva
    │       ↓
    │       [Get Widget] ← Pegar variável salva
    │       ↓
    │       [Get Player Controller]
    │       ↓
    │       [Call Function: Initialize] ← AQUI ESTÁ!
    │           • Target: Manager
    │           • Player Controller: (Player Controller)
    │           • Widget: (Widget)
    │           • Preview Class: BP_CharacterPreview ← CONFIGURAR AQUI!
    │           • Spacing: 300.0 ← CONFIGURAR AQUI!
    │           ↓
    │       [Call Function: PopulateCharacterSelectButtons]
    │
    └─→ [FALSE] →
            ↓
            [Bind Event to OnCharacterListLoaded]
            ↓
            [OnCharacterListLoaded_Event] (Custom Event)
                ↓
                [Get Manager]
                ↓
                [Get Widget]
                ↓
                [Get Player Controller]
                ↓
                [Call Function: Initialize] ← OU AQUI!
                    • Target: Manager
                    • Player Controller: (Player Controller)
                    • Widget: (Widget)
                    • Preview Class: BP_CharacterPreview ← CONFIGURAR AQUI!
                    • Spacing: 300.0 ← CONFIGURAR AQUI!
```

---

## ⚠️ **SE O NÓ NÃO APARECE:**

1. **Compile o C++ primeiro!**
2. **Feche o Unreal Editor**
3. **Reabra o Unreal Editor**
4. **Abra o Level Blueprint novamente**
5. **Procure por "Initialize" novamente**

---

## ✅ **VERIFICAÇÃO RÁPIDA**

**No nó `Initialize`, você DEVE ver 5 pinos de entrada:**
- ✅ Target (Manager)
- ✅ Player Controller
- ✅ Widget
- ✅ **Preview Class** ← ESTE É O PROBLEMA!
- ✅ Spacing

**Se você NÃO ver o pino "Preview Class":**
- O C++ não foi compilado corretamente
- Ou você está usando uma versão antiga do Blueprint

---

**ESSA É A LOCALIZAÇÃO EXATA. O Initialize está no Branch TRUE ou no Custom Event OnCharacterListLoaded_Event.**

