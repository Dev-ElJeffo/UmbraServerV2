# 🎯 **GUIA DETALHADO: Branch Condition no SavePositionTimer**

## 📋 **ANÁLISE DA SUA LÓGICA:**

### **✅ O QUE ESTÁ CORRETO:**
1. ✅ `Get Game Instance` → `Cast To UmbraGameInstance` → `Set Variable: MyGameInstance`
2. ✅ `Get Active Player ID` → conectado ao `PlayerID` do `Save Player Position`
3. ✅ `Get Actor Location` → conectado ao `Position` do `Save Player Position`
4. ✅ `Current Zone` → conectado ao `CurrentZone` do `Save Player Position`

### **❌ O QUE PRECISA SER CORRIGIDO:**
1. ❌ `Branch.Condition` está desconectado (sempre executa `True`)
2. ❌ Não há validação antes de salvar

---

## 🔧 **O QUE CONECTAR AO `Condition` DO BRANCH:**

### **OPÇÃO RECOMENDADA: Validar PlayerID e Location**

**ADICIONE ESTES NÓS ANTES DO `Branch`:**

```
Set Variable: MyGameInstance
  ↓ (then)
[ADICIONAR AQUI - VALIDAÇÃO 1]
Get Active Player ID
  - Target: MyGameInstance
  → ReturnValue: PlayerID (Integer)
  ↓
Greater (Integer)
  - A: PlayerID (do Get Active Player ID)
  - B: 0 (literal Integer)
  → ReturnValue: Boolean (PlayerID > 0?)
  ↓
[ADICIONAR AQUI - VALIDAÇÃO 2]
Get Actor Location
  - Target: Self
  → ReturnValue: Location (Vector)
  ↓
Not Equal (Vector)
  - A: Location (do Get Actor Location)
  - B: (0, 0, 0) (Make Vector ou literal)
  → ReturnValue: Boolean (Location != 0,0,0?)
  ↓
[ADICIONAR AQUI - COMBINAR]
Boolean AND
  - A: PlayerID > 0 (do Greater)
  - B: Location != 0,0,0 (do Not Equal)
  → ReturnValue: Boolean (Ambas válidas?)
  ↓
[CONECTAR AQUI]
Branch
  - Condition: ReturnValue (do Boolean AND)
  ├─ True: Salvar posição ✅
  └─ False: Não salvar ❌
```

---

## 📝 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Adicionar Validação de PlayerID**

**APÓS `Set Variable: MyGameInstance`:**

1. **Adicione `Get Active Player ID`:**
   - **Target:** `MyGameInstance` (variável)
   - **ReturnValue:** Integer (PlayerID)

2. **Adicione `Greater (Integer)`:**
   - **A:** `ReturnValue` do `Get Active Player ID`
   - **B:** `0` (digite `0` no campo)
   - **ReturnValue:** Boolean (`true` se PlayerID > 0)

**COMO FAZER:**
- Arraste do `then` de `Set Variable` → Busque: `Get Active Player ID`
- Conecte `MyGameInstance` ao `Target`
- Arraste do `ReturnValue` → Busque: `Greater (Integer)`
- Conecte `ReturnValue` ao `A`
- Digite `0` no campo `B`

---

### **PASSO 2: Adicionar Validação de Location**

**APÓS `Get Active Player ID` (em paralelo):**

1. **Adicione `Get Actor Location`:**
   - **Target:** `Self` (ou deixe vazio se já está no Character)
   - **ReturnValue:** Vector (Location)

2. **Adicione `Not Equal (Vector)`:**
   - **A:** `ReturnValue` do `Get Actor Location`
   - **B:** `(0, 0, 0)` (Make Vector ou digite `0, 0, 0`)
   - **ReturnValue:** Boolean (`true` se Location != 0,0,0)

**COMO FAZER:**
- Arraste do `then` de `Get Active Player ID` → Busque: `Get Actor Location`
- Arraste do `ReturnValue` → Busque: `Not Equal (Vector)`
- Conecte `ReturnValue` ao `A`
- Para `B`, você pode:
  - Usar `Make Vector` com `X=0, Y=0, Z=0`
  - Ou deixar o valor padrão `(0, 0, 0)`

---

### **PASSO 3: Combinar Condições**

**APÓS ambas as validações:**

1. **Adicione `Boolean AND`:**
   - **A:** `ReturnValue` do `Greater (PlayerID > 0)`
   - **B:** `ReturnValue` do `Not Equal (Location != 0,0,0)`
   - **ReturnValue:** Boolean (`true` se AMBAS forem verdadeiras)

2. **Conecte ao `Branch`:**
   - **Condition:** `ReturnValue` do `Boolean AND`

**COMO FAZER:**
- Arraste do `ReturnValue` do `Greater` → Busque: `Boolean AND`
- Conecte ao `A`
- Arraste do `ReturnValue` do `Not Equal` → Conecte ao `B`
- Arraste do `ReturnValue` do `Boolean AND` → Conecte ao `Condition` do `Branch`

---

### **PASSO 4: Reorganizar Conexões**

**ATUAL (INCORRETO):**
```
Set Variable: MyGameInstance
  ↓ (then)
Branch (Condition: True - sempre executa)
  ↓ (True)
Get Active Player ID
  ↓
Get Actor Location
  ↓
Save Player Position
```

**CORRETO:**
```
Set Variable: MyGameInstance
  ↓ (then)
Get Active Player ID
  ↓
Greater: PlayerID > 0?
  ↓
Get Actor Location (em paralelo ou sequencial)
  ↓
Not Equal: Location != (0,0,0)?
  ↓
Boolean AND: (PlayerID > 0) AND (Location != 0,0,0)
  ↓
Branch (Condition: Boolean AND)
  ├─ True:
  │    ↓
  │   Save Player Position
  │
  └─ False:
       ↓
      Print String: "⚠️ Não foi possível salvar"
```

---

## 🎯 **ESTRUTURA FINAL COMPLETA:**

```
SavePositionTimer (Event)
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ↓ (Cast Success)
Set Variable: MyGameInstance
  ↓ (then)
Get Active Player ID
  - Target: MyGameInstance
  → PlayerID
  ↓
Greater (Integer)
  - A: PlayerID
  - B: 0
  → PlayerID > 0?
  ↓
Get Actor Location
  - Target: Self
  → Location
  ↓
Not Equal (Vector)
  - A: Location
  - B: (0, 0, 0)
  → Location != 0,0,0?
  ↓
Boolean AND
  - A: PlayerID > 0?
  - B: Location != 0,0,0?
  → Ambas válidas?
  ↓
Branch
  - Condition: Ambas válidas?
  ├─ True: ✅
  │    ↓
  │   Save Player Position
  │     - Target: MyGameInstance
  │     - PlayerID: PlayerID
  │     - Position: Location
  │     - CurrentZone: Current Zone
  │
  └─ False: ❌
       ↓
      Print String: "⚠️ Não foi possível salvar - PlayerID ou Location inválidos"
```

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **`Get Active Player ID` e `Get Actor Location` podem ser executados em paralelo** (não dependem um do outro)
2. **`Boolean AND` DEVE receber AMBAS as condições** antes de conectar ao `Branch`
3. **O `Branch` só executa `Save Player Position` se AMBAS as condições forem verdadeiras**

---

## 🧪 **TESTE:**

Após implementar:

1. **PlayerID válido + Location válida:**
   - `PlayerID > 0` = `true`
   - `Location != (0,0,0)` = `true`
   - `Boolean AND` = `true`
   - `Branch` → `True` → `Save Player Position` ✅

2. **PlayerID inválido:**
   - `PlayerID > 0` = `false`
   - `Boolean AND` = `false`
   - `Branch` → `False` → Não salva ❌

3. **Location inválida:**
   - `Location != (0,0,0)` = `false`
   - `Boolean AND` = `false`
   - `Branch` → `False` → Não salva ❌

---

**Status:** ✅ **IMPLEMENTAR VALIDAÇÕES ANTES DO BRANCH**

