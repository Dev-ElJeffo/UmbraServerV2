# 🎯 **GUIA ESPECÍFICO: Onde Adicionar Lógica no OnWSConnected**

## 📍 **LOCALIZAÇÃO EXATA:**

No `BP_NetMovementClient` → `Event Graph` → Evento `OnWSConnected` (Custom Event)

---

## 🔍 **FLUXO ATUAL DO OnWSConnected:**

Analisando o XML fornecido, o fluxo atual é:

```
OnWSConnected (Custom Event)
  ↓
Set ExpectedPlayerID (do GetActivePlayerID)
  ↓
Set IsConnected = true
  ↓
Print String: "WebSocket Connected!"
  ↓
Set Timer (SendMoveUpdate)
  ↓
Set SendTimerHandle
  ↓
Print String: "Timer interval set to: ..."
```

---

## ✅ **ONDE ADICIONAR:**

**Adicione a lógica APÓS o `Print String: "WebSocket Connected!"` e ANTES do `Set Timer`.**

### **ESTRUTURA VISUAL:**

```
OnWSConnected (Custom Event)
  ↓
Set ExpectedPlayerID
  ↓
Set IsConnected = true
  ↓
Print String: "WebSocket Connected!"
  ↓
[✅ ADICIONAR AQUI - NOVA LÓGICA]
  ↓
Set Timer (SendMoveUpdate)
  ↓
Set SendTimerHandle
  ↓
Print String: "Timer interval set to: ..."
```

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Adicionar Delay**

Após o `Print String: "WebSocket Connected!"`:

1. **Clique direito** → Busque: `Delay`
2. **Selecione:** `Delay` (da categoria `Utilities`)
3. **Duration:** `0.5` segundos
4. **Conecte:** Do `then` do `Print String` → `execute` do `Delay`

### **PASSO 2: Obter Character (Pawn)**

Após o `Delay`:

1. **Clique direito** → Busque: `Get First Player Controller`
2. **Selecione:** `Get First Player Controller` (da categoria `Game`)
3. **Conecte:** Do `then` do `Delay` → `Get First Player Controller`
4. **Do `Return Value`** → Arraste → Busque: `Get Pawn`
5. **Selecione:** `Get Pawn` (da categoria `Pawn`)
6. **Do `Return Value`** → Arraste → Busque: `Is Valid`
7. **Selecione:** `Is Valid` (da categoria `Utilities`)
8. **Adicione `Branch`:**
   - Do `Return Value` (Boolean) do `Is Valid` → Arraste → `Branch`
   - **Condition:** Conectado automaticamente

### **PASSO 3: Obter Posição do Banco**

No caminho `True` do `Branch` (Character válido):

1. **Clique direito** → Busque: `Get Game Instance`
2. **Selecione:** `Get Game Instance` (da categoria `Game`)
3. **Do `Return Value`** → Arraste → Busque: `Cast to UmbraGameInstance`
4. **Selecione:** `Cast to UmbraGameInstance`
5. **Conecte:** Do `then` do `Branch` → `execute` do `Cast`
6. **Do `As Umbra Game Instance`** → Arraste → Busque: `Has Active Character`
7. **Selecione:** `Has Active Character` (da categoria `Character`)
8. **Adicione `Branch`:**
   - Do `Return Value` (Boolean) → Arraste → `Branch`
   - **Condition:** Conectado automaticamente

### **PASSO 4: Aplicar Posição**

No caminho `True` do segundo `Branch` (Personagem selecionado):

1. **Do `As Umbra Game Instance`** → Arraste → Busque: `Get Active Character`
2. **Selecione:** `Get Active Character`
3. **Do `Return Value`** → Arraste → Busque: `Break UmbraPlayerData`
4. **Selecione:** `Break UmbraPlayerData` (da categoria `Struct`)
5. **Conecte:** `Return Value` do `Get Active Character` → `In Struct` do `Break`
6. **Do `Position` (FVector)** → Arraste → Busque: `Not Equal (Vector)`
7. **Selecione:** `Not Equal (Vector)` (da categoria `Vector`)
8. **Configure:**
   - **A:** Conecte `Position` (do Break)
   - **B:** Crie um `Make Vector` com valores `(0, 0, 0)`
9. **Adicione `Branch`:**
   - Do `Return Value` (Boolean) → Arraste → `Branch`
   - **Condition:** Conectado automaticamente

### **PASSO 5: Set Actor Location**

No caminho `True` do terceiro `Branch` (Position válida):

1. **Do `Return Value` do `Get Pawn`** (do Passo 2) → Arraste → Busque: `Set Actor Location`
2. **Selecione:** `Set Actor Location` (da categoria `Transform`)
3. **Configure:**
   - **Target:** Conecte o `Return Value` do `Get Pawn` (do Passo 2)
   - **New Location:** Conecte `Position` (do `Break UmbraPlayerData`)
   - **bTeleport:** `True` ← **CRÍTICO!**
4. **Adicione `Print String`:**
   - Do `then` do `Set Actor Location` → `Print String`
   - **InString:** `"✅ Posição do banco aplicada: X=" + ToString(Position.X) + " Y=" + ToString(Position.Y) + " Z=" + ToString(Position.Z)`

### **PASSO 6: Conectar ao Fluxo Original**

Após o `Print String` da posição aplicada:

1. **Conecte o `then`** do último `Print String` → **`execute` do `Set Timer`** (que já existe no fluxo original)

---

## 🎯 **ESTRUTURA COMPLETA VISUAL:**

```
OnWSConnected
  ↓
Set ExpectedPlayerID
  ↓
Set IsConnected = true
  ↓
Print String: "WebSocket Connected!"
  ↓
Delay: 0.5 segundos ← NOVO
  ↓
Get First Player Controller → Get Pawn → Is Valid? → Branch
  ↓ [True]
Get Game Instance → Cast to UmbraGameInstance → Has Active Character? → Branch
  ↓ [True]
Get Active Character → Break UmbraPlayerData → Get Position
  ↓
Position != (0,0,0)? → Branch
  ↓ [True]
Set Actor Location (Pawn, Position, bTeleport=True) ← NOVO
  ↓
Print String: "✅ Posição aplicada!" ← NOVO
  ↓
[Conectar aqui ao Set Timer original]
  ↓
Set Timer (SendMoveUpdate)
  ↓
Set SendTimerHandle
  ↓
Print String: "Timer interval set to: ..."
```

---

## ⚠️ **IMPORTANTE:**

1. **`bTeleport: True`** é **CRÍTICO** no `Set Actor Location`!
2. **Delay de 0.5 segundos** garante que o Character já spawnou
3. **Conecte o fluxo** de volta ao `Set Timer` original para não quebrar a lógica existente

---

## 🧪 **TESTE:**

Após implementar, você deve ver nos logs:

```
LogBlueprintUserMessages: WebSocket Connected!
LogBlueprintUserMessages: ✅ Posição do banco aplicada: X=721.00 Y=1786.00 Z=92.00
LogBlueprintUserMessages: Timer interval set to: ...
```

E o Character deve estar na posição correta `(721, 1786, 92)`.

---

**Status:** 📝 **PRONTO PARA IMPLEMENTAÇÃO**

