# 🎯 **ONDE CONECTAR: Lógica de Aplicar Posição no OnWSConnected**

## 📍 **LOCALIZAÇÃO EXATA:**

**Blueprint:** `BP_NetMovementClient`  
**Evento:** `OnWSConnected` (Custom Event)

---

## 🔍 **ANÁLISE DO CÓDIGO XML FORNECIDO:**

A lógica que você já criou está assim:

```
Delay (0.5 segundos)
  ↓
GetFirstPlayerPawnHelper
  ↓
IsValid
  ↓
Branch → True (Pawn válido)
  ↓
HasActiveCharacter
  ↓
Branch → True (tem personagem ativo)
  ↓
GetActiveCharacter
  ↓
Break UmbraPlayerData → Position
  ↓
NotEqual (Vector): Position != (0, 0, 0)
  ↓
Branch
  ├─ True: Set Actor Location (Position)
  └─ False: Print String ("Posição inválida")
```

---

## ✅ **ONDE CONECTAR NO OnWSConnected:**

### **FLUXO COMPLETO DO OnWSConnected:**

```
OnWSConnected (Custom Event)
  ↓
Set ExpectedPlayerID
  ↓
Set IsConnected = true
  ↓
Print String: "WebSocket Connected!"
  ↓
[✅ CONECTAR AQUI - LÓGICA DE POSIÇÃO]
  ↓
Set Timer by Function Name (SendMoveUpdate)
  ↓
Set SendTimerHandle
  ↓
Print String: "Timer interval set to: ..."
```

---

## 🔧 **CONEXÃO ESPECÍFICA:**

### **PASSO 1: Localizar o Ponto de Conexão**

**NO `OnWSConnected`:**

1. **Localize:** `Print String: "WebSocket Connected!"`
2. **Do pino `then` (execução) deste Print String:**
   - **CONECTE** ao pino `execute` do `Delay` (0.5 segundos) da sua lógica

**ESTRUTURA:**

```
Print String: "WebSocket Connected!"
  ↓ (then)
Delay (0.5 segundos) ← SUA LÓGICA COMEÇA AQUI
  ↓
[Resto da sua lógica...]
  ↓
Set Actor Location (ou Print String "Posição inválida")
  ↓ (then)
[✅ CONECTAR AQUI] → Set Timer by Function Name (SendMoveUpdate)
```

---

## ⚠️ **PONTO CRÍTICO:**

**O pino `then` do último nó da sua lógica (seja `Set Actor Location` ou `Print String`) DEVE estar conectado ao `Set Timer by Function Name`!**

**SE NÃO ESTIVER CONECTADO:**
- Timer não inicia
- `SendMoveUpdate` nunca é chamado
- Cliente não se move

---

## 📋 **VERIFICAÇÃO:**

### **CHECKLIST:**

1. **`Delay` está conectado ao `Print String: "WebSocket Connected!"`?**
   - ✅ `Print String.then` → `Delay.execute`

2. **`Set Actor Location` (ou `Print String "Posição inválida"`) está conectado ao `Set Timer`?**
   - ✅ `Set Actor Location.then` → `Set Timer.execute` (se True)
   - ✅ `Print String.then` → `Set Timer.execute` (se False)

3. **Ambos os caminhos (True e False) convergem para o `Set Timer`?**
   - ✅ Sim: Ambos conectam ao `Set Timer`
   - ❌ Não: Adicione um nó de convergência ou conecte ambos

---

## 🎯 **ESTRUTURA FINAL CORRETA:**

```
OnWSConnected
  ↓
Set ExpectedPlayerID
  ↓
Set IsConnected = true
  ↓
Print String: "WebSocket Connected!"
  ↓ (then)
Delay (0.5)
  ↓ (Completed)
GetFirstPlayerPawnHelper
  ↓
IsValid
  ↓
Branch → True
  ↓
HasActiveCharacter
  ↓
Branch → True
  ↓
GetActiveCharacter
  ↓
Break UmbraPlayerData → Position
  ↓
NotEqual: Position != (0,0,0)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Set Actor Location (Position, bTeleport=True)
  │    ↓ (then)
  │   Print String: "Posição aplicada"
  │    ↓ (then)
  │   [CONECTAR AQUI] → Set Timer
  │
  └─ False:
       ↓
      Print String: "Posição inválida"
       ↓ (then)
      [CONECTAR AQUI] → Set Timer
  ↓
Set Timer by Function Name (SendMoveUpdate)
  ↓
Set SendTimerHandle
  ↓
Print String: "Timer interval set to: ..."
```

---

## 🔧 **COMO CONECTAR OS DOIS CAMINHOS:**

**OPÇÃO 1: Conectar Ambos Diretamente**

- `Set Actor Location.then` → `Set Timer.execute`
- `Print String "Posição inválida".then` → `Set Timer.execute`

**OPÇÃO 2: Usar um Nó de Convergência**

Se os nós estão muito distantes, você pode usar um `Sequence` ou simplesmente conectar ambos ao mesmo `Set Timer` (o Unreal permite múltiplas conexões de execução).

---

## ⚠️ **IMPORTANTE:**

**O `Set Timer` DEVE receber execução de AMBOS os caminhos (True e False) do Branch!**

Se apenas um caminho estiver conectado, o timer pode não iniciar em alguns casos.

---

**Status:** 🎯 **CONECTAR APÓS A LÓGICA DE POSIÇÃO, ANTES DO SET TIMER**

