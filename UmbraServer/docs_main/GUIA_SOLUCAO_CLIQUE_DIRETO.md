# ✅ **SOLUÇÃO: Detectar Clique Diretamente (C++)**

## ❌ **PROBLEMA**

O `OnClicked` do `Collision_Box` não está funcionando.

---

## ✅ **SOLUÇÃO: Usar Função C++ IsActorClicked**

A classe `AUmbraCharacterCreationManager` agora tem uma função estática `IsActorClicked` que detecta cliques usando LineTrace.

---

## 🔧 **IMPLEMENTAÇÃO NO BLUEPRINT**

### **PASSO 1: No BP_Class_Placeholder**

**Remova o `OnClicked` do `Collision_Box` e use um Timer ou Tick:**

**Opção A: Usar Event Tick (mais simples):**

```
[Event Tick]
    • Delta Seconds
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Is Input Key Down]
    • Key: Left Mouse Button
    ↓
[Branch]
    • Condition: (Is Input Key Down)
    ↓ (True)
    [Call Function: Is Actor Clicked]
        • Target: (Self - BP_CharacterCreationManager)
        • Player Controller: (Player Controller)
        • Target Actor: (Self)
        • Return Value: (bWasClicked - Boolean)
        ↓
    [Branch]
        • Condition: bWasClicked
        ↓ (True)
        [Get] ClassID
            ↓
        [Call Function: SelectClass]
            • ClassID: ClassID
```

**Opção B: Usar Timer (mais eficiente):**

No `Event BeginPlay`:

```
[Event BeginPlay]
    ↓
[Set Timer by Function Name]
    • Function Name: "CheckClick"
    • Time: 0.1 (verificar a cada 0.1 segundos)
    • Looping: true
```

Criar função `CheckClick`:

```
[CheckClick]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Is Input Key Down]
    • Key: Left Mouse Button
    ↓
[Branch]
    • Condition: (Is Input Key Down)
    ↓ (True)
    [Get All Actors of Class]
        • Actor Class: BP_CharacterCreationManager
        ↓
    [Get] (primeiro elemento do array)
        ↓
    [Call Function: Is Actor Clicked]
        • Target: (Manager obtido)
        • Player Controller: (Player Controller)
        • Target Actor: (Self)
        • Return Value: bWasClicked
        ↓
    [Branch]
        • Condition: bWasClicked
        ↓ (True)
        [Get] ClassID
            ↓
        [Call Function: SelectClass]
            • ClassID: ClassID
```

---

## ⚠️ **IMPORTANTE**

**A função `IsActorClicked` é estática**, então você pode chamá-la de qualquer lugar:

```
[Call Function: Is Actor Clicked]
    • Target: (qualquer instância de BP_CharacterCreationManager ou deixar vazio)
    • Player Controller: (Player Controller)
    • Target Actor: (Actor que quer verificar)
    • Return Value: (Boolean - true se foi clicado)
```

---

## 🎯 **RESUMO**

1. **Compile o C++**
2. **No `BP_Class_Placeholder`**, remova o `OnClicked` do `Collision_Box`
3. **Use `Is Actor Clicked`** no Tick ou Timer
4. **Quando retornar true**, chame `SelectClass`

**Pronto! Funciona direto, sem depender do `OnClicked` do componente.**

---

**Fim do Guia**

