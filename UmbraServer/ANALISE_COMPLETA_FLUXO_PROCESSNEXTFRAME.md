# 🔬 **ANÁLISE COMPLETA DO FLUXO `ProcessNextFrame`**

## 📋 **PROBLEMAS REPORTADOS:**

1. ❌ **Múltiplos spawns sequenciais de `BP_RemotePlayer`**
2. ❌ **Movimento não funciona**
3. ❌ **Clientes não se veem uns aos outros**

---

## 🔍 **ANÁLISE DETALHADA DO XML FORNECIDO:**

### **ESTRUTURA ATUAL IDENTIFICADA NO XML:**

#### **1. ENTRADA DA FUNÇÃO:**
```
ProcessNextFrame (Custom Event)
  ↓ (then)
  Get Variable: OutFrame
  ↓
  Break Struct: BinaryFrame → Data (TArray<uint8>)
```

#### **2. VERIFICAÇÃO DO TIPO DE FRAME:**
```
Get Array Item: Data[0]
  ↓
Equal (Byte): Array[0] == 2?
  ↓ (via Knot)
  Branch [K2Node_IfThenElse_4]: type == 2?
    ├─ True: CONTINUA
    └─ False: DESCONECTADO (ignorado)
```

#### **3. PARSE DO FRAME:**
```
ParseStateUpdateFrame(Data)
  ↓ (ReturnValue, OutPlayerId, OutLocation, OutYawDegrees, OutTimestampMs)
  Branch [K2Node_IfThenElse_0]: ParseStateUpdateFrame.ReturnValue?
    ├─ True: CONTINUA
    └─ False: DESCONECTADO (ignorado)
```

#### **4. FILTRO DO PRÓPRIO PLAYER:**
```
Not Equal (Integer): OutPlayerId != MyPlayerId?
  ↓ (via Knot)
  Branch [K2Node_IfThenElse_1]: OutPlayerId != MyPlayerId?
    ├─ True: CONTINUA (é outro player)
    └─ False: DESCONECTADO (é o próprio player - ignorado)
```

#### **5. VERIFICAÇÃO SE ACTOR JÁ EXISTE:**
```
Array_Find(RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal (Integer): FoundIndex >= 0?
  ↓
Branch [K2Node_IfThenElse_6]: FoundIndex >= 0?
    ├─ True (actor existe): [ANALISAR CONEXÃO]
    └─ False (actor não existe): → SpawnActorFromClass
```

#### **6. SPAWN DE NOVO ACTOR (quando não existe):**
```
SpawnActorFromClass
  - Class: BP_RemotePlayer
  - SpawnTransform: Make Transform(OutLocation, Make Rotator(Yaw=OutYawDegrees), Scale=1,1,1)
  - CollisionHandlingOverride: AlwaysSpawn
  ↓ (then, ReturnValue)
  Array_Add(RemoteActorIds, OutPlayerId)
  ↓ (then)
  Array_Add(RemoteActors, ReturnValue do SpawnActor)
  ↓ (then)
  [PRÓXIMO PASSO - ANALISAR]
```

#### **7. ATUALIZAÇÃO DE ACTOR EXISTENTE (quando já existe):**
```
Branch [K2Node_IfThenElse_6] → then (True)
  ↓
[ANALISAR O QUE ESTÁ CONECTADO AQUI]
```

#### **8. VERIFICAÇÃO FINAL E RECURSÃO:**
```
[APÓS SPAWN/UPDATE]
  ↓
Get Variable: RemoteActorRef
  ↓
Is Valid? → Branch [K2Node_IfThenElse_9]
  ├─ True: Set Actor Location + Set Actor Rotation
  └─ False: [DESCONECTADO]
```

---

## 🚨 **PROBLEMAS CRÍTICOS IDENTIFICADOS:**

### **PROBLEMA 1: Lógica de Atualização vs. Spawn está Invertida ou Incompleta**

**Análise do XML:**

O fluxo mostra:
1. `Array_Find` busca `OutPlayerId` em `RemoteActorIds`
2. `Branch` verifica `FoundIndex >= 0`
3. **Se `FoundIndex >= 0` (actor existe)**: O pin `then` está **DESCONECTADO** no XML fornecido
4. **Se `FoundIndex < 0` (actor não existe)**: Conectado ao `SpawnActorFromClass`

**Problema:**
- Quando um actor **já existe**, o fluxo não faz nada (pin `then` desconectado)
- O actor existente não é atualizado
- Múltiplos frames podem chegar antes que `Array_Add` atualize `RemoteActorIds`
- Isso causa race condition: `Array_Find` retorna `-1` mesmo que o actor já exista, causando múltiplos spawns

**Solução:**
- **Conectar o pin `then` (True) do `K2Node_IfThenElse_6`**
- **Adicionar `Get Array Item`** para obter `RemoteActors[FoundIndex]`
- **Set Variable `RemoteActorRef`** com o actor obtido
- **Conectar ao mesmo fluxo de atualização** (Set Actor Location + Set Actor Rotation)

---

### **PROBLEMA 2: Variável `RemoteActorRef` Não Está Sendo Setada Corretamente**

**Análise do XML:**

O fluxo mostra que `RemoteActorRef` é verificado com `Is Valid?`, mas:
- **Quando spawna**: `RemoteActorRef` é setado com `ReturnValue` do `SpawnActorFromClass`?
- **Quando atualiza**: `RemoteActorRef` não está sendo setado do array `RemoteActors`

**Problema:**
- Se `RemoteActorRef` não é setado corretamente após o spawn, o `Is Valid?` falha
- Se `RemoteActorRef` não é obtido do array quando o actor já existe, ele permanece inválido
- Isso impede que `Set Actor Location` e `Set Actor Rotation` sejam executados

**Solução:**
- **Após `SpawnActorFromClass`**: Conectar `ReturnValue` diretamente a `Set Variable (RemoteActorRef)`
- **Após `Get Array Item` (quando actor existe)**: Conectar `Output` a `Set Variable (RemoteActorRef)`
- **Ambos os caminhos** devem convergir para o mesmo fluxo de atualização

---

### **PROBLEMA 3: Ordem de Execução dos `Array_Add`**

**Análise do XML:**

O fluxo mostra:
```
SpawnActorFromClass
  ↓ (then)
  Array_Add(RemoteActorIds, OutPlayerId)
  ↓ (then)
  Array_Add(RemoteActors, ReturnValue)
```

**Potencial Problema:**
- Se `Array_Add` não é executado **imediatamente**, há uma janela onde `Array_Find` ainda retorna `-1`
- Múltiplos frames podem chegar nessa janela, causando múltiplos spawns

**Solução:**
- ✅ **Garantir que ambos `Array_Add` sejam executados sequencialmente, sem interrupções**
- ✅ **Garantir que `Set Variable (RemoteActorRef)` seja executado APÓS os `Array_Add`**
- ✅ **Verificar se não há outros nós entre `SpawnActorFromClass` e os `Array_Add`**

---

### **PROBLEMA 4: Recursão Pode Estar Causando Múltiplos Processamentos**

**Análise do XML:**

O fluxo final deve chamar `ProcessBinaryBuffer` novamente para verificar se há mais frames no buffer:
```
[APÓS TUDO]
  ↓
ProcessBinaryBuffer(Buffer, NewData=[], OutFrame)
  ↓ (ReturnValue)
  Branch: ReturnValue == true?
    ├─ True: ProcessNextFrame (recursão)
    └─ False: FIM
```

**Potencial Problema:**
- Se `ProcessBinaryBuffer` está retornando `true` incorretamente, a recursão continua indefinidamente
- Se múltiplos frames estão no buffer e todos são processados, podem causar múltiplos spawns do mesmo player

**Solução:**
- ✅ **Garantir que `NewData` no segundo `ProcessBinaryBuffer` seja um array vazio (`Make Array` com 0 elementos)**
- ✅ **Garantir que o `Branch` após o segundo `ProcessBinaryBuffer` só chame recursão se `ReturnValue == true`**

---

## ✅ **CORREÇÕES DETALHADAS REQUERIDAS:**

### **CORREÇÃO 1: Conectar Atualização de Actor Existente**

**Localização:** Após `Branch [K2Node_IfThenElse_6]` → pin `then` (True)

**Passos:**

1. **Conectar o pin `then` (True) do `K2Node_IfThenElse_6`**
2. **Adicionar `Get Array Item`:**
   - **Array**: `Get Variable (RemoteActors)`
   - **Index**: `FoundIndex` (do `Array_Find`)
   - **Output**: Conectar a um novo `Set Variable (RemoteActorRef)`
3. **Conectar o `then` do `Set Variable` ao mesmo fluxo que atualiza posição/rotação**

**Fluxo visual:**
```
Branch [K2Node_IfThenElse_6]: FoundIndex >= 0?
  ├─ True (actor existe):
  │   ↓
  │   Get Array Item(RemoteActors, FoundIndex)
  │   ↓
  │   Set Variable (RemoteActorRef)
  │   ↓
  │   [CONTINUAR PARA ATUALIZAÇÃO]
  │
  └─ False (actor não existe):
      ↓
      SpawnActorFromClass
      ↓
      Array_Add(RemoteActorIds, OutPlayerId)
      ↓
      Array_Add(RemoteActors, ReturnValue)
      ↓
      Set Variable (RemoteActorRef) ← ADICIONAR AQUI TAMBÉM!
      ↓
      [CONTINUAR PARA ATUALIZAÇÃO]
```

---

### **CORREÇÃO 2: Garantir `RemoteActorRef` Setado em Ambos os Caminhos**

**Localização:** Após `Array_Add` (novo actor) e após `Get Array Item` (actor existente)

**Passos:**

1. **No caminho de spawn:**
   - Após `Array_Add(RemoteActors, ReturnValue)`, adicionar `Set Variable (RemoteActorRef)` usando o mesmo `ReturnValue` do `SpawnActorFromClass`
2. **No caminho de atualização:**
   - Após `Get Array Item(RemoteActors, FoundIndex)`, conectar `Output` a `Set Variable (RemoteActorRef)`

**Fluxo visual:**
```
Caminho Spawn:
  SpawnActorFromClass → ReturnValue
    ↓
  Array_Add(RemoteActorIds, OutPlayerId)
    ↓
  Array_Add(RemoteActors, ReturnValue)
    ↓
  Set Variable (RemoteActorRef) ← ReturnValue do SpawnActorFromClass
    ↓
  [CONTINUAR]

Caminho Atualização:
  Get Array Item(RemoteActors, FoundIndex) → Output
    ↓
  Set Variable (RemoteActorRef) ← Output do Get Array Item
    ↓
  [CONTINUAR]
```

---

### **CORREÇÃO 3: Convergir Ambos os Caminhos para o Mesmo Fluxo de Atualização**

**Localização:** Após ambos os `Set Variable (RemoteActorRef)`

**Passos:**

1. **Ambos os caminhos** (spawn e atualização) devem convergir para o mesmo `Is Valid?` + `Set Actor Location` + `Set Actor Rotation`

**Fluxo visual:**
```
[APÓS Set Variable (RemoteActorRef) em ambos os caminhos]
  ↓
Get Variable (RemoteActorRef)
  ↓
Is Valid? → Branch
  ├─ True:
  │   ↓
  │   Set Actor Location (RemoteActorRef, OutLocation)
  │   ↓
  │   Make Rotator (Yaw = OutYawDegrees)
  │   ↓
  │   Set Actor Rotation (RemoteActorRef, NewRotation)
  │   ↓
  │   [CONTINUAR PARA RECURSÃO]
  │
  └─ False: [DESCONECTADO ou Print String de debug]
```

---

### **CORREÇÃO 4: Verificar Recursão Correta**

**Localização:** Final do `ProcessNextFrame`

**Passos:**

1. **Verificar que o segundo `ProcessBinaryBuffer` usa `NewData = []` (array vazio)**
2. **Verificar que o `Branch` após o segundo `ProcessBinaryBuffer` só chama recursão se `ReturnValue == true`**

**Fluxo visual:**
```
[APÓS Set Actor Rotation]
  ↓
Make Array (0 elementos) → EmptyArray
  ↓
ProcessBinaryBuffer(Buffer, NewData=EmptyArray, OutFrame)
  ↓ (ReturnValue)
Branch: ReturnValue == true?
  ├─ True: ProcessNextFrame (recursão)
  └─ False: FIM (não há mais frames)
```

---

## 🔬 **VERIFICAÇÕES DE DEBUG RECOMENDADAS:**

### **1. Adicionar Print String Após Cada Passo Crítico:**

```blueprint
1. Após ParseStateUpdateFrame:
   "Parsed: PlayerID=" + ToString(OutPlayerId) + ", MyPlayerId=" + ToString(MyPlayerId)

2. Após Array_Find:
   "FoundIndex=" + ToString(FoundIndex) + " for PlayerID=" + ToString(OutPlayerId)

3. Após Branch (FoundIndex >= 0):
   "Branch: Actor exists=" + ToString(FoundIndex >= 0)

4. Após SpawnActorFromClass:
   "Spawned actor for PlayerID=" + ToString(OutPlayerId) + ", Valid=" + ToString(IsValid(ReturnValue))

5. Após Get Array Item (actor existente):
   "Got existing actor from index=" + ToString(FoundIndex) + ", Valid=" + ToString(IsValid(Output))

6. Após Set Variable (RemoteActorRef):
   "Set RemoteActorRef, Valid=" + ToString(IsValid(RemoteActorRef))

7. Após Set Actor Location:
   "Updated location for PlayerID=" + ToString(OutPlayerId) + ", Location=" + ToString(OutLocation)
```

---

### **2. Verificar Ordem de Execução:**

Adicione um contador global para rastrear quantas vezes `ProcessNextFrame` é chamado para o mesmo `OutPlayerId`:

```blueprint
[NO INÍCIO DE ProcessNextFrame]
Get Variable (DebugCounter)
  ↓
Add (DebugCounter + 1)
  ↓
Set Variable (DebugCounter)
  ↓
Print String: "ProcessNextFrame call #" + ToString(DebugCounter) + " for PlayerID=" + ToString(OutPlayerId)
```

---

## 📝 **CHECKLIST DE IMPLEMENTAÇÃO:**

- [ ] **Correção 1**: Conectar pin `then` (True) do `K2Node_IfThenElse_6`
- [ ] **Correção 1**: Adicionar `Get Array Item(RemoteActors, FoundIndex)`
- [ ] **Correção 1**: Adicionar `Set Variable (RemoteActorRef)` após `Get Array Item`
- [ ] **Correção 2**: Adicionar `Set Variable (RemoteActorRef)` após `Array_Add(RemoteActors, ...)`
- [ ] **Correção 3**: Conectar ambos os caminhos ao mesmo `Is Valid?` + atualização
- [ ] **Correção 4**: Verificar que segundo `ProcessBinaryBuffer` usa array vazio
- [ ] **Debug**: Adicionar `Print String` em cada passo crítico
- [ ] **Debug**: Adicionar contador de chamadas para rastrear múltiplas execuções

---

## 🎯 **RESULTADO ESPERADO APÓS CORREÇÕES:**

1. ✅ **Um único spawn por `OutPlayerId`** (sem duplicados)
2. ✅ **Ator existente é atualizado** (movimento funciona)
3. ✅ **Clientes veem uns aos outros** (frames de outros players são processados corretamente)
4. ✅ **Personagem local se move normalmente** (frames próprios são ignorados)

---

## ⚠️ **NOTAS FINAIS:**

- **Prioridade máxima**: Conectar o pin `then` (True) do `K2Node_IfThenElse_6` e adicionar a lógica de atualização de actor existente
- **Segunda prioridade**: Garantir que `RemoteActorRef` é setado corretamente em ambos os caminhos
- **Terceira prioridade**: Adicionar logs de debug para identificar problemas restantes

Se após essas correções os problemas persistirem, a causa pode estar em:
- Problemas no servidor (envio de frames duplicados)
- Problemas no buffer (frames sendo processados múltiplas vezes)
- Problemas na lógica de recursão (múltiplas chamadas para o mesmo frame)

