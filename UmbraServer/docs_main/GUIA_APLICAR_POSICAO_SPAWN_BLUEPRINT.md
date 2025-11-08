# 🎯 **GUIA PRÁTICO: Aplicar Posição do Banco no Spawn do Character**

## 🔴 **PROBLEMA ATUAL:**

O Character está spawnando na posição padrão `(-320, 710, 92)` em vez da posição salva no banco `(721, 1786, 92)`.

**Logs confirmam:**
- ✅ C++ atualiza o array corretamente: `Position: 721.00, 1786.00, 92.00`
- ❌ Servidor recebe posição padrão: `Location: (-320.000000, 710.000000, 92.000000)`

---

## ✅ **SOLUÇÃO: Implementar no Blueprint do Character**

### **OPÇÃO 1: Usar Delegate `OnCharacterSelected` (RECOMENDADO)**

Esta é a melhor opção porque garante que a posição seja aplicada **após** o personagem ser selecionado.

#### **Blueprint: `BP_ThirdPersonCharacter` (ou Character Principal)**

#### **PASSO 1.1: Criar Binding do Delegate**

1. **No `Event Graph`**, adicione:

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

2. **O Event criado terá um parâmetro `SelectedPlayer` (tipo `UmbraPlayerData`)**

#### **PASSO 1.2: Implementar Lógica no Event**

```
OnCharacterSelectedEvent (Custom Event)
  ↓
[SelectedPlayer] → Break UmbraPlayerData
  ↓
Get Position (FVector)
  ↓
Print String: "🔵 Aplicando posição do banco: " + ToString(Position.X) + ", " + ToString(Position.Y) + ", " + ToString(Position.Z)
  ↓
Not Equal (Vector): Position != (0, 0, 0)
  ↓
Branch
  ├─ True: Position válida
  │   ↓
  │   Set Actor Location
  │     - Target: Self
  │     - New Location: Position
  │     - bTeleport: True ← IMPORTANTE!
  │   ↓
  │   Print String: "✅ Posição aplicada com sucesso!"
  │
  └─ False: Position inválida
      ↓
      Print String: "⚠️ Posição inválida (0,0,0), usando padrão"
```

---

### **OPÇÃO 2: Verificar no BeginPlay com Delay**

Se o Character já está spawnado quando o personagem é selecionado, use esta opção:

#### **Blueprint: `BP_ThirdPersonCharacter`**

#### **PASSO 2.1: Adicionar Delay no BeginPlay**

```
Event BeginPlay
  ↓
Delay: 0.5 segundos ← Aguardar SelectCharacter completar
  ↓
Get Game Instance
  ↓
Cast to UmbraGameInstance
  ↓
Has Active Character? ← Verificar se há personagem selecionado
  ↓
Branch
  ├─ True: Personagem selecionado
  │   ↓
  │   Get Active Character
  │     ↓
  │   Break UmbraPlayerData
  │     ↓
  │   Get Position (FVector)
  │     ↓
  │   Print String: "🔵 Posição obtida: " + ToString(Position.X) + ", " + ToString(Position.Y) + ", " + ToString(Position.Z)
  │     ↓
  │   Not Equal (Vector): Position != (0, 0, 0)
  │     ↓
  │   Branch
  │     ├─ True: Set Actor Location (Position, bTeleport=True)
  │     └─ False: Usar posição padrão
  │
  └─ False: Ainda não há personagem selecionado
      ↓
      Print String: "⚠️ Personagem ainda não selecionado, usando posição padrão"
```

---

### **OPÇÃO 3: Verificar Periodicamente (Timer)**

Se as opções acima não funcionarem, use um Timer para verificar periodicamente:

#### **PASSO 3.1: Criar Variável**

1. **Criar variável booleana:** `bPositionApplied` (padrão: `False`)

#### **PASSO 3.2: Implementar Timer**

```
Event BeginPlay
  ↓
Set Timer by Function Name
  - Function Name: "CheckAndApplyPosition"
  - Time: 0.1 segundos
  - Looping: True
```

#### **PASSO 3.3: Criar Custom Event**

```
CheckAndApplyPosition (Custom Event)
  ↓
[Se bPositionApplied == True] → Branch → [True] → Return ← Pular se já aplicou
  ↓
Get Game Instance → Cast to UmbraGameInstance
  ↓
Has Active Character?
  ↓
Branch
  ├─ True: Personagem selecionado
  │   ↓
  │   Get Active Character → Break UmbraPlayerData → Get Position
  │     ↓
  │   Not Equal (Vector): Position != (0, 0, 0)
  │     ↓
  │   Branch
  │     ├─ True: Position válida
  │     │   ↓
  │     │   Set Actor Location (Position, bTeleport=True)
  │     │   ↓
  │     │   Set bPositionApplied = True ← Marcar como aplicado
  │     │   ↓
  │     │   Clear Timer: "CheckAndApplyPosition" ← Parar timer
  │     │   ↓
  │     │   Print String: "✅ Posição aplicada!"
  │     │
  │     └─ False: Continuar verificando
  │
  └─ False: Continuar verificando
```

---

## 🎯 **RECOMENDAÇÃO FINAL:**

**Use a OPÇÃO 1 (Delegate `OnCharacterSelected`)** porque:
- ✅ Garante que a posição seja aplicada no momento certo
- ✅ Não depende de delays ou timers
- ✅ Mais eficiente e confiável

---

## 📋 **ESTRUTURA COMPLETA DA OPÇÃO 1:**

### **1. No `Event Graph` do Character:**

```
Event BeginPlay
  ↓
Get Game Instance
  ↓
Cast to UmbraGameInstance
  ↓
[Do Cast Result] → Bind Event to OnCharacterSelected
  ↓
[Create Event] → OnCharacterSelectedEvent
```

### **2. Implementar `OnCharacterSelectedEvent`:**

```
OnCharacterSelectedEvent (Custom Event)
  Parâmetro: SelectedPlayer (UmbraPlayerData)
  ↓
Break UmbraPlayerData (SelectedPlayer)
  ↓
Get Position (FVector)
  ↓
Print String: "🔵 [Character] Aplicando posição: X=" + ToString(Position.X) + " Y=" + ToString(Position.Y) + " Z=" + ToString(Position.Z)
  ↓
Not Equal (Vector): Position != (0, 0, 0)
  ↓
Branch
  ├─ True: Position válida
  │   ↓
  │   Set Actor Location
  │     - Target: Self
  │     - New Location: Position
  │     - bTeleport: True ← CRÍTICO!
  │   ↓
  │   Print String: "✅ [Character] Posição aplicada com sucesso!"
  │
  └─ False: Position inválida
      ↓
      Print String: "⚠️ [Character] Posição inválida (0,0,0)"
```

---

## 🧪 **TESTE:**

1. **Implementar a OPÇÃO 1** no Blueprint do Character
2. **Compilar e executar**
3. **Fazer login e selecionar personagem**
4. **Verificar logs:**
   ```
   LogBlueprintUserMessages: 🔵 [Character] Aplicando posição: X=721.00 Y=1786.00 Z=92.00
   LogBlueprintUserMessages: ✅ [Character] Posição aplicada com sucesso!
   ```
5. **Verificar no jogo:** O Character deve spawnar em `(721, 1786, 92)`

---

## ⚠️ **IMPORTANTE:**

- **`bTeleport: True`** é **CRÍTICO** no `Set Actor Location`! Sem isso, o Character pode não se mover se houver colisão.
- Se o Character já estiver spawnado quando o personagem é selecionado, o delegate ainda funcionará e aplicará a posição.

---

**Status:** 📝 **AGUARDANDO IMPLEMENTAÇÃO NO BLUEPRINT**

