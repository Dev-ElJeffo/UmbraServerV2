# 🔍 **DIAGNÓSTICO: Clientes Recebendo 25 Bytes Apesar do Servidor Enviar 34 Bytes**

## 📋 **PROBLEMA IDENTIFICADO:**

**Logs do Servidor (✅ CORRETO):**
```
[debug] Broadcasting StateUpdate with animation: PlayerID=19, ..., frame_size=34 bytes
[debug] Broadcasted StateUpdate for player 19 (from client 2, ts=..., hasAnimation=true)
```

**Logs do Cliente (❌ INCORRETO):**
```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Received binary message, size:25expected=25
LogTemp: Warning: [ProcessBinaryBuffer] Frame completo em offset 0 (Buffer.Num()=25, múltiplo de 25)
```

**Problema:** O servidor está enviando frames de **34 bytes**, mas os clientes estão recebendo frames de **25 bytes**.

---

## 🔍 **CAUSA RAIZ:**

### **Hipótese 1: `ProcessBinaryBuffer` Está Truncando os Frames**

O `ProcessBinaryBuffer` no C++ detecta o tamanho do frame (25 ou 34 bytes) e copia os bytes corretos para `OutFrame.Data`, mas **sempre usa `ParseStateUpdateFrame` (função antiga de 25 bytes) para validar**, mesmo quando detecta que o frame é de 34 bytes.

**Código Atual (`WSBinaryBPFL.cpp`):**
```cpp
// Linha 228-272: Detecta tamanho do frame (25 ou 34)
if (Buffer.Num() >= FrameSizeNew && Buffer[0] == StateUpdateType)
{
    if (ParseStateUpdateFrameWithAnimation(...))  // ← Tenta parsear como novo
    {
        FrameSize = FrameSizeNew;  // ← Detecta como 34 bytes
    }
    else
    {
        FrameSize = FrameSizeOld;  // ← Fallback para 25 bytes
    }
}

// Linha 343: SEMPRE usa ParseStateUpdateFrame (antigo) para validar
if (ParseStateUpdateFrame(CandidateFrame, ...))  // ← PROBLEMA: sempre usa função antiga
{
    // Validação...
    OutFrame.Data = CandidateFrame;  // ← Copia apenas 25 bytes!
}
```

**PROBLEMA:** Mesmo quando detecta que o frame é de 34 bytes, o código sempre usa `ParseStateUpdateFrame` que espera apenas 25 bytes. Isso pode fazer com que apenas os primeiros 25 bytes sejam copiados para `OutFrame.Data`.

---

### **Hipótese 2: `ProcessNextFrame` Está Usando Parse Antigo**

O `ProcessNextFrame` no Blueprint pode estar sempre usando `ParseStateUpdateFrame` (antigo) em vez de tentar `ParseStateUpdateFrameWithAnimation` primeiro.

---

## ✅ **SOLUÇÃO:**

### **ETAPA 1: Corrigir `ProcessBinaryBuffer` no C++**

O `ProcessBinaryBuffer` precisa usar o `FrameSize` detectado para copiar os bytes corretos:

```cpp
// Após detectar FrameSize (linha 252 ou 267):
// Em vez de sempre usar ParseStateUpdateFrame, usar o FrameSize detectado:

TArray<uint8> CandidateFrame;
CandidateFrame.Reserve(FrameSize);  // ← Usar FrameSize detectado (não sempre 25)
for (int32 j = 0; j < FrameSize && j < Buffer.Num(); ++j)  // ← Copiar FrameSize bytes
{
    CandidateFrame.Add(Buffer[j]);
}

if (CandidateFrame.Num() != FrameSize)
{
    return false;  // Frame incompleto
}

// Tentar parsear baseado no FrameSize detectado:
if (FrameSize == FrameSizeNew)
{
    // Tentar parsear como frame novo primeiro
    int32 TestPlayerId = 0;
    FVector TestLocation;
    float TestYaw = 0.0f;
    float TestSpeed = 0.0f;
    float TestVelocityZ = 0.0f;
    bool TestIsInAir = false;
    int32 TestTimestamp = 0;
    
    if (ParseStateUpdateFrameWithAnimation(CandidateFrame, TestPlayerId, TestLocation, TestYaw, TestSpeed, TestVelocityZ, TestIsInAir, TestTimestamp))
    {
        // Frame novo válido - usar CandidateFrame completo (34 bytes)
        OutFrame.Data = CandidateFrame;  // ← Copiar FrameSize bytes completos
        Buffer.RemoveAt(0, FrameSize, EAllowShrinking::No);
        return true;
    }
    else
    {
        // Frame novo falhou parse - tentar como frame antigo
        FrameSize = FrameSizeOld;
        CandidateFrame.SetNum(FrameSizeOld);  // ← Reduzir para 25 bytes
    }
}

// Tentar parsear como frame antigo (25 bytes)
int32 CandidatePlayerId = 0;
FVector CandidateLocation;
float CandidateYaw = 0.0f;
int32 CandidateTimestamp = 0;

if (ParseStateUpdateFrame(CandidateFrame, CandidatePlayerId, CandidateLocation, CandidateYaw, CandidateTimestamp))
{
    // ... validação ...
    OutFrame.Data = CandidateFrame;  // ← Copiar FrameSize bytes
    Buffer.RemoveAt(0, FrameSize, EAllowShrinking::No);
    return true;
}
```

---

### **ETAPA 2: Verificar `ProcessNextFrame` no Blueprint**

O `ProcessNextFrame` deve tentar parsear como frame novo primeiro:

```
ProcessBinaryBuffer → OutFrame
  ↓
Break BinaryFrame → Data
  ↓
Get Array Length (Data)  ← Verificar tamanho
  ↓
Branch (Length >= 34?)
  ├─ True: [Tentar frame novo primeiro]
  │   ↓
  │   ParseStateUpdateFrameWithAnimation (34 bytes)
  │   ↓
  │   Branch (ReturnValue)
  │   ├─ True: [Frame novo com animação]
  │   └─ False: [Tentar frame antigo]
  │       ↓
  │       ParseStateUpdateFrame (25 bytes)
  │
  └─ False: [Tentar frame antigo diretamente]
      ↓
      ParseStateUpdateFrame (25 bytes)
```

---

## 🔧 **CORREÇÃO IMEDIATA:**

### **1. Adicionar Log no `ProcessBinaryBuffer` para Diagnosticar:**

```cpp
// Após linha 445 (OutFrame.Data = CandidateFrame):
UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Frame aceito - FrameSize=%d bytes, OutFrame.Data.Num()=%d"), 
       FrameSize, OutFrame.Data.Num());
```

**Isso vai mostrar se o `OutFrame.Data` está sendo preenchido com 34 bytes ou apenas 25 bytes.**

---

### **2. Adicionar Log no Blueprint `ProcessNextFrame`:**

Após `Break BinaryFrame → Data`:
```
Get Array Length (Data)
  ↓
Format Text: "[ProcessNextFrame] Data recebido do ProcessBinaryBuffer - Size: {0} bytes"
  ↓
Print String
```

**Isso vai mostrar se o `Data` recebido do `ProcessBinaryBuffer` tem 34 bytes ou apenas 25 bytes.**

---

## 🎯 **PROBLEMA ADICIONAL: Rastro de Múltiplos Actors**

Os logs mostram que múltiplos actors aparecem temporariamente. Isso pode ser causado por:

1. **Race Condition:** Múltiplos frames sendo processados antes do `Array_Add` atualizar
2. **Frames Duplicados:** O servidor pode estar enviando múltiplos broadcasts do mesmo frame
3. **ProcessNextFrame Recursivo:** Pode estar sendo chamado recursivamente antes de atualizar os arrays

**Solução:** Verificar se há um double-check antes de `SpawnActorFromClass`:

```
Array_Find (RemoteActorIds, OutPlayerId)
  ↓
Branch (FoundIndex >= 0?)
  ├─ True: [Actor existe - atualizar]
  └─ False: [Actor não existe]
      ↓
      Array_Find (RemoteActorIds, OutPlayerId)  ← DOUBLE-CHECK
      ↓
      Branch (FoundIndex >= 0?)
      ├─ True: [Actor foi adicionado entre chamadas - atualizar]
      └─ False: [Realmente não existe - spawnar]
          ↓
          SpawnActorFromClass
          ↓
          Array_Add (RemoteActorIds, OutPlayerId)
          ↓
          Array_Add (RemoteActors, SpawnedActor)
```

---

## 📊 **CHECKLIST DE VERIFICAÇÃO:**

### **C++ (`ProcessBinaryBuffer`):**
- [ ] `OutFrame.Data` está sendo preenchido com `FrameSize` bytes corretos (não sempre 25)
- [ ] Log mostra `FrameSize=34` quando frame tem animação
- [ ] Log mostra `OutFrame.Data.Num()=34` quando frame tem animação

### **Blueprint (`ProcessNextFrame`):**
- [ ] Log mostra `Data recebido do ProcessBinaryBuffer - Size: 34 bytes` quando servidor envia 34 bytes
- [ ] `ParseStateUpdateFrameWithAnimation` está ANTES de `ParseStateUpdateFrame`
- [ ] Há double-check antes de `SpawnActorFromClass` para evitar race condition

---

**Fim do Diagnóstico**

