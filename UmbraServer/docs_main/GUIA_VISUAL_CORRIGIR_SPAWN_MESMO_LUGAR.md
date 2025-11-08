# 🎯 **GUIA VISUAL: Corrigir Spawn no Mesmo Lugar**

## 🚨 **PROBLEMA:**

Todos os personagens spawnam na mesma posição, causando:
- Colisão entre eles
- Eles ficam travados
- Já spawnam travados

---

## ✅ **SOLUÇÃO: Adicionar Offset por PlayerID**

### **LOCALIZAÇÃO:** `BP_NetMovementClient:OnWSConnected`

---

## 📋 **ESTRUTURA ATUAL (PROBLEMÁTICA):**

```
Not Equal (Position != 0,0,0)?
  ↓ (True)
Set Actor Location
  - NewLocation: Position (direto do Break UmbraPlayerData)
  ← PROBLEMA: Todos usam a mesma posição
```

---

## ✅ **ESTRUTURA CORRIGIDA:**

```
Not Equal (Position != 0,0,0)?
  ↓ (True)
Get Active Player ID
  ↓
Multiply (PlayerID * 200.0) → OffsetX
  ↓
Make Vector
  - X: OffsetX
  - Y: 0.0
  - Z: 0.0
  → Offset
  ↓
Add Vector
  - A: Position (do Break UmbraPlayerData)
  - B: Offset
  → FinalPosition
  ↓
Set Actor Location
  - NewLocation: FinalPosition (Position + Offset)
```

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Adicionar Get Active Player ID**

**APÓS `Not Equal (Position != 0,0,0)?` → True:**

1. **Adicionar `Get Active Player ID`:**
   - Buscar: `Get Active Player ID`
   - Target: `MyGameInstance` (variável)
   - ReturnValue: `PlayerID` (Integer)

2. **Conectar:**
   - `then` do `Not Equal` → `execute` do `Get Active Player ID`

---

### **PASSO 2: Calcular Offset X**

**APÓS `Get Active Player ID`:**

1. **Adicionar `Multiply (Double Double)`:**
   - A: `PlayerID` (do Get Active Player ID)
   - B: `200.0` (literal Double)
   - ReturnValue: `OffsetX` (Double)

2. **Conectar:**
   - `ReturnValue` do `Get Active Player ID` → `A` do `Multiply`
   - Digite `200.0` no campo `B`

---

### **PASSO 3: Criar Vector Offset**

**APÓS `Multiply`:**

1. **Adicionar `Make Vector`:**
   - X: `OffsetX` (do Multiply)
   - Y: `0.0` (literal)
   - Z: `0.0` (literal)
   - ReturnValue: `Offset` (Vector)

2. **Conectar:**
   - `ReturnValue` do `Multiply` → `X` do `Make Vector`
   - Digite `0.0` nos campos `Y` e `Z`

---

### **PASSO 4: Somar Position + Offset**

**APÓS `Make Vector`:**

1. **Adicionar `Add Vector`:**
   - A: `Position` (do Break UmbraPlayerData)
   - B: `Offset` (do Make Vector)
   - ReturnValue: `FinalPosition` (Vector)

2. **Conectar:**
   - `Position` (do Break UmbraPlayerData) → `A` do `Add Vector`
   - `ReturnValue` do `Make Vector` → `B` do `Add Vector`

---

### **PASSO 5: Aplicar Posição Final**

**APÓS `Add Vector`:**

1. **Modificar `Set Actor Location`:**
   - Desconectar `Position` (do Break UmbraPlayerData) do `NewLocation`
   - Conectar `FinalPosition` (do Add Vector) ao `NewLocation`

2. **Conectar execução:**
   - `then` do `Add Vector` → `execute` do `Set Actor Location`

---

## 📊 **DIAGRAMA COMPLETO:**

```
OnWSConnected
  ↓
Delay (0.5s)
  ↓
Get First Player Pawn Helper
  ↓
Is Valid (Pawn)?
  ↓ (True)
Has Active Character?
  ↓ (True)
Get Active Character
  ↓
Break UmbraPlayerData
  - Position: (X, Y, Z)
  ↓
Not Equal (Position != 0,0,0)?
  ├─ True:
  │    ↓
  │   Get Active Player ID
  │     - Target: MyGameInstance
  │     → PlayerID: 1, 2, 3, ...
  │    ↓
  │   Multiply (PlayerID * 200.0)
  │     - A: PlayerID
  │     - B: 200.0
  │     → OffsetX: 200.0, 400.0, 600.0, ...
  │    ↓
  │   Make Vector
  │     - X: OffsetX
  │     - Y: 0.0
  │     - Z: 0.0
  │     → Offset: (200, 0, 0), (400, 0, 0), ...
  │    ↓
  │   Add Vector
  │     - A: Position (do banco)
  │     - B: Offset
  │     → FinalPosition: Position + Offset
  │    ↓
  │   Set Actor Location
  │     - NewLocation: FinalPosition
  │
  └─ False:
       ↓
      Print String: "Posição inválida (0,0,0), usando padrão"
       ↓
      [Usar PlayerStart + Offset ou posição padrão]
```

---

## 🎯 **EXEMPLO DE RESULTADO:**

**PlayerID 1:**
- Posição salva: `(100, 200, 50)`
- Offset: `(200, 0, 0)`
- **Posição final: `(300, 200, 50)`**

**PlayerID 2:**
- Posição salva: `(100, 200, 50)` (mesma)
- Offset: `(400, 0, 0)`
- **Posição final: `(500, 200, 50)`** ← DIFERENTE!

**PlayerID 3:**
- Posição salva: `(100, 200, 50)` (mesma)
- Offset: `(600, 0, 0)`
- **Posição final: `(700, 200, 50)`** ← DIFERENTE!

---

## ⚙️ **AJUSTES OPCIONAIS:**

### **OPÇÃO 1: Offset Circular (Melhor Distribuição)**

**Em vez de `Multiply (PlayerID * 200.0)`, usar:**

```
Get Active Player ID
  ↓
Multiply (PlayerID * 60.0) → Angle (graus)
  ↓
Sin (Angle) → X Offset
Cos (Angle) → Y Offset
  ↓
Multiply (X Offset * 500.0) → X
Multiply (Y Offset * 500.0) → Y
  ↓
Make Vector (X, Y, Position.Z)
```

**Resultado:** Players spawnam em círculo ao redor da posição base.

---

### **OPÇÃO 2: Offset 2D (X e Y)**

**Em vez de apenas X, usar X e Y:**

```
Make Vector
  - X: (PlayerID * 200.0)
  - Y: (PlayerID * 200.0)
  - Z: 0.0
```

**Resultado:** Players spawnam em grid 2D.

---

## 🧪 **TESTE RÁPIDO:**

1. Conectar Client 1 (PlayerID: 1)
2. **VERIFICAR:** Deve spawnar em `Position + (200, 0, 0)`
3. Conectar Client 2 (PlayerID: 2)
4. **VERIFICAR:** Deve spawnar em `Position + (400, 0, 0)`
5. **VERIFICAR:** Não devem estar colidindo
6. **VERIFICAR:** Não devem estar travados

---

## 📝 **LOGS PARA ADICIONAR:**

```
Print String: "OnWSConnected - PlayerID: {PlayerID}, Offset: X={OffsetX}"
Print String: "OnWSConnected - Posição salva: X={PosX}, Y={PosY}, Z={PosZ}"
Print String: "OnWSConnected - Posição final: X={FinalX}, Y={FinalY}, Z={FinalZ}"
```

---

**Status:** 🚨 **URGENTE - IMPLEMENTAR OFFSET POR PLAYERID**

