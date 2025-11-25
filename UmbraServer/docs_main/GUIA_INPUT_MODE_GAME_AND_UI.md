# 🎮 GUIA: INPUT MODE - GAME AND UI

## 🎯 **OBJETIVO:**

Permitir que o jogador **movimente seu personagem** enquanto o inventário/armazém está aberto, mantendo a interação com a UI.

---

## ❌ **PROBLEMA:**

**Input Mode: UI Only**
- ✅ Mouse funciona na UI
- ❌ Jogador não consegue se mover
- ❌ Não pode usar habilidades
- ❌ Gameplay bloqueado

**Input Mode: Game Only**
- ✅ Jogador se move normalmente
- ❌ Mouse não funciona na UI
- ❌ Não pode clicar em botões

---

## ✅ **SOLUÇÃO:**

**Input Mode: Game And UI**
- ✅ Mouse funciona na UI
- ✅ Jogador pode se mover (WASD)
- ✅ Pode usar habilidades
- ✅ Gameplay continua ativo!

---

## 🔧 **IMPLEMENTAÇÃO:**

### **JÁ IMPLEMENTADO NO C++! ✅**

O sistema já configura automaticamente o Input Mode correto quando:
- Inventário é aberto
- Armazém é aberto
- Inventário é fechado
- Armazém é fechado

---

## 📊 **ONDE ESTÁ IMPLEMENTADO:**

### **1. UmbraInventoryWidget.cpp**

**Ao abrir o inventário:**

```cpp
// NativeConstruct() ou função de abrir
APlayerController* PC = GetOwningPlayer();
if (PC)
{
    PC->SetShowMouseCursor(true);
    
    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(this->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    
    PC->SetInputMode(InputMode);
}
```

**Ao fechar o inventário:**

```cpp
// Função de fechar
APlayerController* PC = GetOwningPlayer();
if (PC)
{
    PC->SetShowMouseCursor(false);
    
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
}
```

### **2. UmbraStorageActor.cpp**

**Ao abrir o armazém:**

```cpp
// OpenStorage()
PlayerController->SetShowMouseCursor(true);

FInputModeGameAndUI InputMode;
InputMode.SetWidgetToFocus(StorageWidgetInstance->TakeWidget());

PlayerController->SetInputMode(InputMode);
```

**Ao fechar o armazém:**

```cpp
// CloseStorage()
if (InteractingPlayer)
{
    InteractingPlayer->SetShowMouseCursor(false);
    
    FInputModeGameOnly InputMode;
    InteractingPlayer->SetInputMode(InputMode);
}
```

---

## 🎮 **COMO USAR (BLUEPRINT):**

Se você quiser configurar manualmente em outro widget:

### **Ao Abrir Widget:**

```blueprint
[Event Construct]
    │
    └─> [Get Player Controller]
            │
            ├─> [Set Show Mouse Cursor]
            │       │ bShow: True
            │
            └─> [Set Input Mode Game And UI]
                    │ Widget to Focus: Self
                    │ Lock Mouse: Do Not Lock
```

### **Ao Fechar Widget:**

```blueprint
[Event on Button Close: OnClicked]
    │
    ├─> [Remove From Parent]
    │
    └─> [Get Player Controller]
            │
            ├─> [Set Show Mouse Cursor]
            │       │ bShow: False
            │
            └─> [Set Input Mode Game Only]
```

---

## 🔍 **DIFERENÇAS:**

| Input Mode | Mouse Cursor | WASD Move | UI Clicável | Atalhos (Q, E, R) |
|------------|-------------|-----------|-------------|-------------------|
| **Game Only** | ❌ | ✅ | ❌ | ✅ |
| **UI Only** | ✅ | ❌ | ✅ | ❌ |
| **Game And UI** | ✅ | ✅ | ✅ | ✅ |

**Recomendado: Game And UI** 🏆

---

## 🎯 **COMPORTAMENTO ESPERADO:**

### **Com Input Mode: Game And UI:**

1. Jogador pressiona `I` para abrir inventário
2. ✅ Inventário abre
3. ✅ Mouse aparece
4. ✅ Jogador pode clicar nos slots
5. ✅ Jogador pode arrastar itens
6. ✅ Jogador pode apertar WASD e se mover
7. ✅ Jogador pode usar habilidades (Q, E, R)
8. Jogador pressiona `X` (botão fechar)
9. ✅ Inventário fecha
10. ✅ Mouse desaparece
11. ✅ Gameplay volta ao normal

**MESMA COISA PARA O ARMAZÉM!**

---

## 🐛 **TROUBLESHOOTING:**

### **Problema: Jogador não consegue se mover:**
- ❌ Input Mode está em **UI Only**
- ✅ Altere para **Game And UI**

### **Problema: Mouse não aparece:**
- ❌ `Set Show Mouse Cursor` não foi chamado
- ✅ Adicione `Set Show Mouse Cursor: True` ao abrir

### **Problema: UI não responde a cliques:**
- ❌ Input Mode está em **Game Only**
- ✅ Altere para **Game And UI**

### **Problema: Após fechar, mouse continua visível:**
- ❌ `Set Show Mouse Cursor: False` não foi chamado ao fechar
- ✅ Adicione ao evento de fechar

---

## 🔥 **BEST PRACTICES:**

### **1. Sempre configure ao abrir:**
```cpp
SetShowMouseCursor(true);
SetInputMode(FInputModeGameAndUI);
```

### **2. Sempre restaure ao fechar:**
```cpp
SetShowMouseCursor(false);
SetInputMode(FInputModeGameOnly);
```

### **3. Use SetLockMouseToViewportBehavior:**
```cpp
FInputModeGameAndUI InputMode;
InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
```
Permite que o mouse saia da viewport (multi-monitor).

### **4. SetWidgetToFocus (opcional):**
```cpp
InputMode.SetWidgetToFocus(WidgetInstance->TakeWidget());
```
Define qual widget recebe foco automático.

---

## ✅ **CHECKLIST:**

**No seu sistema:**
- [x] ✅ Input Mode configurado no `UmbraInventoryWidget` (C++)
- [x] ✅ Input Mode configurado no `UmbraStorageActor` (C++)
- [x] ✅ Mouse aparece ao abrir
- [x] ✅ Mouse desaparece ao fechar
- [x] ✅ Jogador pode se mover com UI aberta
- [x] ✅ Jogador pode clicar na UI
- [x] ✅ Gameplay continua ativo

**SISTEMA PERFEITO! 🏆**

---

## 📚 **REFERÊNCIAS:**

- [Unreal Engine Docs: Input Mode](https://docs.unrealengine.com/5.3/en-US/API/Runtime/Engine/GameFramework/FInputModeGameAndUI/)
- `UmbraInventoryWidget.cpp` - Implementação no inventário
- `UmbraStorageActor.cpp` - Implementação no armazém

---

## 🎮 **RESULTADO FINAL:**

**Jogador pode:**
- ✅ Mover personagem (WASD)
- ✅ Olhar ao redor (Mouse)
- ✅ Usar habilidades (Q, E, R)
- ✅ Clicar na UI (inventário/armazém)
- ✅ Arrastar itens
- ✅ Fechar com ESC ou X

**GAMEPLAY FLUÍDO E RESPONSIVO! 🔥**

---

**IMPLEMENTAÇÃO COMPLETA! ✅**

