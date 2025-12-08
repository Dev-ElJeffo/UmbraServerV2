# 📋 **GUIA: Setar Manager no WBP_CreateCharacter**

## 🎯 **OBJETIVO**

Configurar a variável `Manager` no widget `WBP_CreateCharacter` para que ela aponte para o `BP_CharacterCreationManager` do level.

---

## 📝 **PASSO A PASSO**

### **PASSO 1: Criar Variável Manager no Widget**

1. Abra `WBP_CreateCharacter` no Unreal Editor
2. No **My Blueprint** (painel esquerdo), clique em **Variables** → **+ Variable**
3. **Nome da variável:** `Manager`
4. **Tipo:** `Object Reference` → **Object Class:** `BP_CharacterCreationManager` (ou `Umbra Character Creation Manager`)
5. **Category:** "References" (opcional, para organização)
6. **Editable:** ✅ (marcado)
7. **Instance Editable:** ✅ (marcado) - permite editar no editor
8. **Expose on Spawn:** ❌ (desmarcado)
9. **Save Game:** ❌ (desmarcado)

---

### **PASSO 2: Setar Manager no Event Construct**

**No Event Graph do `WBP_CreateCharacter`:**

```
[Event Construct]
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    • Return Value: ManagerActor
    ↓
[Is Valid]
    • Object: ManagerActor
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [SET] Manager = ManagerActor
        • Target: Self
        • Manager: ManagerActor
    ↓
    [Print String] (opcional, para debug)
        • In String: "Manager setado com sucesso!"
        • bPrintToScreen: true
    ↓ (False)
    [Print String] (opcional, para debug)
        • In String: "ERRO: Manager não encontrado!"
        • bPrintToScreen: true
        • Text Color: (255, 0, 0) ← Vermelho
```

---

### **PASSO 3: Estrutura Completa do Event Construct**

**Se você já tem outras coisas no `Event Construct`, adicione no início:**

```
[Event Construct]
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[SET] Manager = (resultado do Get Actor of Class)
    ↓
[Continue com o resto do Event Construct...]
```

---

## 🔧 **ALTERNATIVA: Setar Manager no Level Blueprint**

**Se preferir setar no Level Blueprint (recomendado se o Manager é spawnado no level):**

### **No Level Blueprint → Event BeginPlay:**

```
[Event BeginPlay]
    ↓
[Spawn Actor] BP_CharacterCreationManager
    • Location: (0, 0, 0)
    • Return Value: ManagerActor
    ↓
[Create Widget] WBP_CreateCharacter
    • Return Value: CreateCharacterWidget
    ↓
[SET] Manager
    • Target: CreateCharacterWidget
    • Manager: ManagerActor
    ↓
[Add to Viewport]
    • Target: CreateCharacterWidget
```

**OU se o Manager já existe no level:**

```
[Event BeginPlay]
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    • Return Value: ManagerActor
    ↓
[Create Widget] WBP_CreateCharacter
    • Return Value: CreateCharacterWidget
    ↓
[SET] Manager
    • Target: CreateCharacterWidget
    • Manager: ManagerActor
    ↓
[Add to Viewport]
    • Target: CreateCharacterWidget
```

---

## ✅ **VERIFICAÇÃO**

### **Teste 1: Verificar se Manager Está Setado**

**Adicione um Print String no botão `BTN_Return`:**

```
[BTN_Return OnClicked]
    ↓
[Get] Manager
    ↓
[Is Valid]
    • Object: Manager
    ↓
[Branch]
    • Condition: (Is Valid)
    ↓ (True)
    [Print String]
        • In String: "Manager é válido!"
        • bPrintToScreen: true
        • Text Color: (0, 255, 0) ← Verde
    ↓ (False)
    [Print String]
        • In String: "ERRO: Manager é nullptr!"
        • bPrintToScreen: true
        • Text Color: (255, 0, 0) ← Vermelho
```

**Se aparecer "Manager é válido!", está funcionando! ✅**

---

## 🔍 **TROUBLESHOOTING**

### **Problema 1: Manager é nullptr**

**Causas possíveis:**
- ❌ O `BP_CharacterCreationManager` não existe no level
- ❌ O `Event Construct` não está sendo executado
- ❌ O `Get Actor of Class` não está encontrando o Manager

**Soluções:**
1. Verifique se o `BP_CharacterCreationManager` está no level (World Outliner)
2. Se não estiver, adicione via **Place Actors** ou spawn no Level Blueprint
3. Verifique se o `Event Construct` está conectado corretamente

---

### **Problema 2: Manager não persiste**

**Causa:**
- O Manager pode estar sendo destruído ou não está sendo encontrado

**Solução:**
- Use `Get Actor of Class` no `Event Construct` em vez de salvar referência estática
- Ou garanta que o Manager é spawnado antes do widget ser criado

---

### **Problema 3: Manager está setado mas ResetCharacterCreation não funciona**

**Verifique:**
1. ✅ O Manager está válido? (use `Is Valid`)
2. ✅ O `CreateCharacterWidget` está conectado? (deve ser `Self`)
3. ✅ A função `ClearCharacterCreationList` existe?
4. ✅ O `VB_InfoandCreate` existe?

---

## 📌 **NOTAS IMPORTANTES**

1. **O Manager deve existir no level** antes do widget ser criado
2. **Se o Manager for spawnado dinamicamente**, use o Level Blueprint para setar
3. **Se o Manager já existe no level**, use `Get Actor of Class` no `Event Construct`
4. **Sempre verifique com `Is Valid`** antes de usar o Manager

---

## 🎯 **RESUMO RÁPIDO**

**Método 1 (Recomendado - Event Construct):**
```
Event Construct
    → Get Actor of Class (BP_CharacterCreationManager)
    → Is Valid
    → Branch (True)
        → SET Manager = (resultado)
```

**Método 2 (Level Blueprint):**
```
Event BeginPlay
    → Spawn/Create Manager
    → Create Widget (WBP_CreateCharacter)
    → SET Manager no widget
    → Add to Viewport
```

---

**Fim do Guia**

