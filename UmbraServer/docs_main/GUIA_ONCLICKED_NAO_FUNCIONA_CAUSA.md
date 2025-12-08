# 🔍 **POR QUE OnClicked NÃO FUNCIONA**

## ❌ **CAUSA REAL**

O `OnClicked` do `PrimitiveComponent` **NÃO funciona quando há Widget aberto**, mesmo que o widget não esteja bloqueando visualmente. O Unreal Engine prioriza eventos de UI sobre eventos de componentes quando há widgets na tela.

---

## ✅ **SOLUÇÃO SEM TICK: Usar Input Action**

### **PASSO 1: Criar Input Action**

1. No **Content Browser**, vá para a pasta de Input
2. Crie um **Input Action** chamado `IA_SelectClass`
3. **Value Type:** `Digital (bool)`

### **PASSO 2: Criar Input Mapping Context**

1. Crie ou edite o **Input Mapping Context** usado no nível
2. Adicione o mapeamento:
   - **Action:** `IA_SelectClass`
   - **Key:** `Left Mouse Button`

### **PASSO 3: No BP_Class_Placeholder**

**No Event BeginPlay:**

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Get Enhanced Input Local Player Subsystem]
    • Target: (Player Controller)
    ↓
[Add Mapping Context]
    • Mapping Context: (seu IMC)
    • Priority: 0
```

**Criar função `HandleSelectClassInput`:**

```
[HandleSelectClassInput] (Custom Event)
    • Action Value (Input Action Value)
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Call Function: Get Clicked Actor]
    • Player Controller: (Player Controller)
    • Out Hit Actor: (variável local)
    • Return Value: bWasClicked
    ↓
[Branch]
    • Condition: bWasClicked
    ↓ (True)
    [Equal] (Object)
        • A: Out Hit Actor
        • B: (Self)
        ↓
    [Branch]
        • Condition: (Equal result)
        ↓ (True)
        [Get] ClassID
            ↓
        [Call Function: SelectClass]
            • ClassID: ClassID
```

**No Event BeginPlay, adicionar binding:**

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    ↓
[Get Enhanced Input Component]
    • Target: (Self)
    ↓
[Bind Action]
    • Action: IA_SelectClass
    • Event: Triggered
    • Target: (Self)
    • Function: HandleSelectClassInput
```

---

## 🎯 **RESUMO**

**Por que OnClicked não funciona:**
- Widgets na tela bloqueiam eventos de componentes
- Unreal prioriza eventos de UI

**Solução sem Tick:**
- Usar Input Action (Enhanced Input)
- Bind no BeginPlay
- Chamar GetClickedActor quando o Input Action disparar

**Event-driven, sem polling, sem Tick.**

---

**Fim do Guia**

