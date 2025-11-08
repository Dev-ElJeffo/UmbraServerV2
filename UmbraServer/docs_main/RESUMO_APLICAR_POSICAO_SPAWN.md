# 📋 **RESUMO EXECUTIVO: Aplicar Posição do Banco no Spawn**

## 🔴 **PROBLEMA:**

Character spawna em `(-320, 710, 92)` em vez de `(721, 1786, 92)` (posição do banco).

**Causa:** Blueprint do Character não está aplicando a posição do banco no spawn.

---

## ✅ **SOLUÇÃO RÁPIDA:**

### **No Blueprint `BP_ThirdPersonCharacter`:**

1. **No `Event Graph`**, adicione após `Event BeginPlay`:

```
Event BeginPlay
  ↓
Get Game Instance
  ↓
Cast to UmbraGameInstance
  ↓
[Do Cast Result] → Arraste → "Bind Event to OnCharacterSelected"
  ↓
[Create Event] → Nome: "OnCharacterSelectedEvent"
```

2. **Implementar o Event criado:**

```
OnCharacterSelectedEvent (Custom Event)
  Parâmetro: SelectedPlayer (UmbraPlayerData)
  ↓
Break UmbraPlayerData
  ↓
Get Position (FVector)
  ↓
Not Equal (Vector): Position != (0, 0, 0)
  ↓
Branch → True
  ↓
Set Actor Location
  - Target: Self
  - New Location: Position
  - bTeleport: True ← IMPORTANTE!
```

---

## 🎯 **POR QUE FUNCIONA:**

- O delegate `OnCharacterSelected` é disparado **após** o `SelectCharacter` completar
- Garante que a posição esteja disponível quando aplicada
- Funciona mesmo se o Character já estiver spawnado

---

## 📝 **PRÓXIMO PASSO:**

Implementar no Blueprint e testar.

