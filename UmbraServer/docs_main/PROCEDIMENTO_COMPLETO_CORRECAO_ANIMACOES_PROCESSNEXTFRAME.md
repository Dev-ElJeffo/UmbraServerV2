# 🔧 **PROCEDIMENTO COMPLETO: Correção de Animações em ProcessNextFrame**

## 📋 **RESUMO EXECUTIVO:**

Com base na análise completa do XML do `ProcessNextFrame`, foram identificados **2 problemas críticos** que impedem as animações de funcionarem corretamente:

### **❌ PROBLEMA CRÍTICO #1: `Set Velocity` ausente no caminho de atualização de actor existente**
- **Status Atual:** `Set Velocity` está presente apenas após `SpawnActorFromClass` (novo actor)
- **Problema:** Quando um actor já existe (`FoundIndex >= 0`), o caminho de atualização (`Get Array Item` → `Set RemoteActorRef` → `Set Actor Location` → `Set Actor Rotation`) **NÃO aplica `Set Velocity`**
- **Impacto:** Actors existentes não recebem atualização de velocidade/animação, apenas posição e rotação
- **Solução:** Adicionar `Cast to Character` → `Get Character Movement` → `Set Velocity` no caminho de atualização de actors existentes

### **❌ PROBLEMA CRÍTICO #2: `OutIsInAir` não utilizado**
- **Status Atual:** `OutIsInAir` está sendo parseado, mas não está conectado a nenhuma lógica
- **Problema:** O `Movement Mode` do remote actor não é atualizado baseado em `OutIsInAir`
- **Impacto:** Animações de queda/pulo podem não funcionar corretamente
- **Solução:** Adicionar `Branch` com `OutIsInAir` → `Set Movement Mode` (OPCIONAL, mas recomendado)

### **⚠️ PROBLEMA MENOR: `NewTimestampMs` não conectado**
- **Status Atual:** `UpdatePlayerStateBuffer` recebe `NewTimestampMs` com valor padrão `0`
- **Problema:** Timestamps sempre 0, afetando interpolação/timing
- **Impacto:** Baixo (interpolação pode funcionar sem timestamp correto)
- **Solução:** Conectar `OutTimestampMs` ao pin `NewTimestampMs` de `UpdatePlayerStateBuffer`

---

## 🔧 **PROCEDIMENTO PASSO A PASSO PARA CORREÇÕES:**

### **CORREÇÃO #1: Adicionar `Set Velocity` no caminho de atualização de actor existente**

#### **📍 LOCALIZAÇÃO NO BLUEPRINT:**

Encontre o nó `K2Node_VariableSet_1` (`Set RemoteActorRef`) que está no caminho `then` do `K2Node_IfThenElse_6` (quando `FoundIndex >= 0`).

**Fluxo atual (INCORRETO):**
```
K2Node_IfThenElse_6 (FoundIndex >= 0?)
  ├─ then: Get Array Item → Set RemoteActorRef → [FIM - vai para Set Actor Location]
  └─ else: SpawnActorFromClass → Set RemoteActorRef → Cast to Character → Set Velocity ✅
```

**Fluxo esperado (CORRETO):**
```
K2Node_IfThenElse_6 (FoundIndex >= 0?)
  ├─ then: Get Array Item → Set RemoteActorRef → Cast to Character → Set Velocity → Set Actor Location ✅
  └─ else: SpawnActorFromClass → Set RemoteActorRef → Cast to Character → Set Velocity ✅
```

#### **📝 PASSOS DETALHADOS:**

1. **Localizar o nó `K2Node_VariableSet_1` (`Set RemoteActorRef`)**
   - Este nó está no caminho `then` do `K2Node_IfThenElse_6`
   - Ele recebe o output de `K2Node_GetArrayItem_0` (actor existente)
   - Atualmente, seu pin `then` está conectado a `K2Node_Knot_133` → `K2Node_IfThenElse_4` → `Set Actor Location`

2. **Desconectar temporariamente o pin `then` de `K2Node_VariableSet_1`**
   - Desconecte o pin `then` de `K2Node_VariableSet_1` do `K2Node_Knot_133`
   - Isso permitirá inserir a lógica de animação antes de `Set Actor Location`

3. **Adicionar `Cast to Character`**
   - **Nó:** `Cast to Character` (Dynamic Cast)
   - **Target Type:** `Character`
   - **Pin `Object`:** Conecte ao `Output_Get` de `K2Node_VariableSet_1` (ou use `K2Node_VariableGet` de `RemoteActorRef`)
   - **Pin `execute`:** Conecte ao pin `then` de `K2Node_VariableSet_1`
   - **Pin `D` (sucesso):** Conecte ao próximo passo (`Get Character Movement`)
   - **Pin `Cast Failed`:** Conecte diretamente a `K2Node_Knot_133` (pular animação se não for Character)

4. **Adicionar `Get Character Movement`**
   - **Nó:** `Get Character Movement` (Variable Get)
   - **Target:** Conecte ao pin `As Character` do `Cast to Character`
   - **Output:** Conecte ao `Target` do próximo passo (`Set Velocity`)

5. **Reutilizar o cálculo de `New Velocity` existente**
   - **Nó:** `K2Node_Knot_132` (já existe e está conectado ao `New Velocity` calculado)
   - **Conecte:** O pin `OutputPin` de `K2Node_Knot_132` ao pin `New Velocity` do `Set Velocity`

6. **Adicionar `Set Velocity`**
   - **Nó:** `Set Velocity` (Variable Set)
   - **Target:** Conecte ao `Return Value` do `Get Character Movement`
   - **New Velocity:** Conecte ao pin `OutputPin` de `K2Node_Knot_132` (mesmo `New Velocity` usado após spawn)
   - **Pin `execute`:** Conecte ao pin `D` do `Cast to Character`
   - **Pin `then`:** Conecte ao `K2Node_Knot_133` (continuar para `Set Actor Location`)

7. **Verificar conexões finais:**
   ```
   Set RemoteActorRef (then)
     ↓
   Cast to Character (execute)
     ├─ D (sucesso) → Get Character Movement → Set Velocity → Knot_133 → Set Actor Location
     └─ Cast Failed → Knot_133 → Set Actor Location (pular animação)
   ```

#### **✅ VERIFICAÇÃO:**

- [ ] `Cast to Character` está presente após `Set RemoteActorRef` no caminho `then`?
- [ ] `Get Character Movement` está presente após `Cast to Character`?
- [ ] `Set Velocity` está presente após `Get Character Movement`?
- [ ] `New Velocity` está conectado ao `Set Velocity` (via `K2Node_Knot_132`)?
- [ ] Pin `then` de `Set Velocity` está conectado a `K2Node_Knot_133`?
- [ ] Pin `Cast Failed` está conectado a `K2Node_Knot_133` (para pular animação)?

---

### **CORREÇÃO #2: Adicionar uso de `OutIsInAir` para definir `Movement Mode` (OPCIONAL, mas recomendado)**

#### **📍 LOCALIZAÇÃO NO BLUEPRINT:**

**⚠️ IMPORTANTE:** A lógica do `OutIsInAir` deve ser adicionada em **AMBOS os caminhos**:

1. **Caminho 1:** Após `Set Velocity` no caminho de **actor existente** (`FoundIndex >= 0`)
2. **Caminho 2:** Após `Set Velocity` no caminho de **novo actor** (`FoundIndex < 0`)

#### **📝 PASSOS DETALHADOS:**

**Para AMBOS os caminhos, siga os mesmos passos:**

1. **Localizar o pin `OutIsInAir` de `ParseStateUpdateFrameWithAnimation`**
   - **Nó:** `K2Node_CallFunction_38` (`ParseStateUpdateFrameWithAnimation`)
   - **Pin:** `OutIsInAir` (Boolean)
   - **Status Atual:** Não conectado

2. **Adicionar `Branch` com `OutIsInAir`**
   - **Nó:** `Branch` (If Then Else)
   - **Pin `Condition`:** Conecte ao pin `OutIsInAir` de `ParseStateUpdateFrameWithAnimation`
   - **Pin `execute`:** Conecte ao pin `then` de `Set Velocity` (após cada `Set Velocity`)
   - **Pin `then` (True):** Player no ar → `Set Movement Mode: Falling`
   - **Pin `else` (False):** Player no chão → `Set Movement Mode: Walking` (ou pular se preferir)

3. **Adicionar `Set Movement Mode` para Falling**
   - **Nó:** `Set Movement Mode` (Call Function)
   - **Target:** Conecte ao `Return Value` do `Get Character Movement` (mesmo usado em `Set Velocity`)
   - **Movement Mode:** Selecione `Falling` (ou `MOVE_Falling`)
   - **Pin `execute`:** Conecte ao pin `then` do `Branch` (`OutIsInAir == true`)

4. **Adicionar `Set Movement Mode` para Walking**
   - **Nó:** `Set Movement Mode` (Call Function)
   - **Target:** Conecte ao `Return Value` do `Get Character Movement` (mesmo usado em `Set Velocity`)
   - **Movement Mode:** Selecione `Walking` (ou `MOVE_Walking`)
   - **Pin `execute`:** Conecte ao pin `else` do `Branch` (`OutIsInAir == false`)

5. **Conectar ambos os caminhos ao próximo passo:**
   - **Pin `then` de `Set Movement Mode (Falling)`:** Conecte a `K2Node_Knot_133` (ou diretamente a `Set Actor Location`)
   - **Pin `then` de `Set Movement Mode (Walking)`:** Conecte a `K2Node_Knot_133` (ou diretamente a `Set Actor Location`)

#### **🔄 APLICAR EM AMBOS OS CAMINHOS:**

**Caminho 1 (Actor Existente):**
```
Set Velocity (actor existente)
  ↓ (then)
Branch (OutIsInAir)
  ├─ True → Set Movement Mode (Falling) → Knot_133 → Set Actor Location
  └─ False → Set Movement Mode (Walking) → Knot_133 → Set Actor Location
```

**Caminho 2 (Novo Actor):**
```
Set Velocity (novo actor)
  ↓ (then)
Branch (OutIsInAir)
  ├─ True → Set Movement Mode (Falling) → Knot_133 → Set Actor Location
  └─ False → Set Movement Mode (Walking) → Knot_133 → Set Actor Location
```

**💡 DICA:** Você pode criar a lógica uma vez e depois **copiar/colar** para o outro caminho, ajustando apenas as conexões de entrada (`execute`) e saída (`then`).

#### **✅ VERIFICAÇÃO:**

- [ ] `Branch` com `OutIsInAir` está presente após `Set Velocity`?
- [ ] `Set Movement Mode` está presente no caminho `then` (`OutIsInAir == true`)?
- [ ] `Set Movement Mode` está presente no caminho `else` (`OutIsInAir == false`)? (OPCIONAL)
- [ ] Ambos os caminhos convergem para `Set Actor Location`?

**⚠️ NOTA:** `Set Movement Mode` é **OPCIONAL**. O Character Movement Component pode inferir o modo automaticamente baseado na `Velocity`. Se preferir, pode pular esta correção e deixar o sistema inferir automaticamente.

---

### **CORREÇÃO #3: Conectar `NewTimestampMs` em `UpdatePlayerStateBuffer` (OPCIONAL)**

#### **📍 LOCALIZAÇÃO NO BLUEPRINT:**

Encontre o nó `K2Node_CallFunction_6` (`UpdatePlayerStateBuffer`).

#### **📝 PASSOS DETALHADOS:**

1. **Localizar o pin `NewTimestampMs` de `UpdatePlayerStateBuffer`**
   - **Nó:** `K2Node_CallFunction_6` (`UpdatePlayerStateBuffer`)
   - **Pin:** `NewTimestampMs` (Integer)
   - **Status Atual:** Valor padrão `0`

2. **Conectar `OutTimestampMs` ao pin `NewTimestampMs`**
   - **Nó:** `K2Node_CallFunction_38` (`ParseStateUpdateFrameWithAnimation`)
   - **Pin:** `OutTimestampMs` (Integer)
   - **Conecte:** Ao pin `NewTimestampMs` de `UpdatePlayerStateBuffer`

#### **✅ VERIFICAÇÃO:**

- [ ] `OutTimestampMs` está conectado ao pin `NewTimestampMs` de `UpdatePlayerStateBuffer`?

---

## 📊 **FLUXO COMPLETO CORRIGIDO:**

```
ProcessNextFrame
  ↓
ParseStateUpdateFrameWithAnimation
  ├─ OutPlayerId
  ├─ OutLocation
  ├─ OutYawDegrees
  ├─ OutSpeed
  ├─ OutVelocityZ
  ├─ OutIsInAir
  ├─ OutTimestampMs
  └─ ReturnValue
  ↓
Branch (ReturnValue == true?)
  ├─ True: [Frame novo - 34 bytes]
  │   ↓
  │   [Filtro: OutPlayerId != Active Player ID]
  │   ↓
  │   Array_Find (RemoteActorIds, OutPlayerId)
  │   ↓
  │   Branch (FoundIndex >= 0?)
  │   ├─ True: [Actor existe]
  │   │   ↓
  │   │   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │   │   ↓
  │   │   Set RemoteActorRef
  │   │   ↓
  │   │   [APLICAR ANIMAÇÃO - NOVO] ✅
  │   │   ↓
  │   │   Cast to Character (RemoteActorRef)
  │   │   ├─ D (sucesso): [É Character]
  │   │   │   ↓
  │   │   │   Get Character Movement
  │   │   │   ↓
  │   │   │   [CALCULAR VELOCITY]
  │   │   │   │
  │   │   │   OutYawDegrees → Make Rotator (0, OutYawDegrees, 0)
  │   │   │     ↓
  │   │   │   Get Forward Vector
  │   │   │     ↓
  │   │   │   Multiply (Vector * OutSpeed)
  │   │   │     ↓
  │   │   │   Break Vector → X, Y
  │   │   │   │
  │   │   │   OutVelocityZ
  │   │   │   ↓
  │   │   │   Make Vector (X, Y, OutVelocityZ) → New Velocity
  │   │   │   ↓
  │   │   │   Set Velocity ✅
  │   │   │     ├─ Target: Get Character Movement
  │   │   │     ├─ New Velocity: [Do Make Vector]
  │   │   │     └─ then
  │   │   │   ↓
  │   │   │   [OPCIONAL: Branch (OutIsInAir) → Set Movement Mode] ✅
  │   │   │   ├─ True → Set Movement Mode (Falling)
  │   │   │   └─ False → Set Movement Mode (Walking)
  │   │   │
  │   │   └─ Cast Failed: [Não é Character - pular animação]
  │   │   ↓
  │   │   Set Actor Location (OutLocation)
  │   │   Set Actor Rotation (OutYawDegrees)
  │   │
  │   └─ False: [Actor não existe]
  │       ↓
  │       SpawnActorFromClass → RemoteActorRef
  │       ↓
  │       Set RemoteActorRef
  │       ↓
  │       [APLICAR ANIMAÇÃO - JÁ EXISTE] ✅
  │       ↓
  │       Cast to Character (RemoteActorRef)
  │       ├─ D (sucesso): Get Character Movement → Set Velocity ✅
  │       │   ↓
  │       │   [OPCIONAL: Branch (OutIsInAir) → Set Movement Mode] ✅
  │       │   ├─ True → Set Movement Mode (Falling)
  │       │   └─ False → Set Movement Mode (Walking)
  │       └─ Cast Failed: [Pular animação]
  │       ↓
  │       Array_Add (RemoteActorIds, OutPlayerId)
  │       Array_Add (RemoteActors, RemoteActorRef)
  │
  └─ False: [Tentar frame antigo - 25 bytes]
      ↓
      ParseStateUpdateFrame (Data)
      ↓
      [Lógica existente sem animação]
```

---

## ✅ **CHECKLIST FINAL DE VERIFICAÇÃO:**

### **CORREÇÃO #1: Set Velocity no caminho de actor existente**

- [ x] `Cast to Character` está presente após `Set RemoteActorRef` no caminho `then` (`FoundIndex >= 0`)?
- [x ] Pin `Object` do `Cast to Character` está conectado ao `RemoteActorRef`?
- [x ] Pin `D` (sucesso) do `Cast to Character` está conectado ao `Get Character Movement`?
- [x ] Pin `Cast Failed` do `Cast to Character` está conectado a `K2Node_Knot_133` (pular animação)?
- [ x] `Get Character Movement` está presente após `Cast to Character`?
- [x ] Pin `Target` do `Get Character Movement` está conectado ao `As Character` do `Cast to Character`?
- [x ] `Set Velocity` está presente após `Get Character Movement`?
- [ x] Pin `Target` do `Set Velocity` está conectado ao `Return Value` do `Get Character Movement`?
- [ ] Pin `New Velocity` do `Set Velocity` está conectado ao `K2Node_Knot_132` (mesmo `New Velocity` usado após spawn)?
- [ ] Pin `execute` do `Set Velocity` está conectado ao pin `D` do `Cast to Character`?
- [ ] Pin `then` do `Set Velocity` está conectado a `K2Node_Knot_133` (continuar para `Set Actor Location`)?

### **CORREÇÃO #2: OutIsInAir para Movement Mode (OPCIONAL)**

**⚠️ IMPORTANTE:** Verifique em **AMBOS os caminhos** (actor existente e novo actor):

**Caminho 1 (Actor Existente):**
- [ ] `Branch` com `OutIsInAir` está presente após `Set Velocity` (actor existente)?
- [ ] Pin `Condition` do `Branch` está conectado ao pin `OutIsInAir` de `ParseStateUpdateFrameWithAnimation`?
- [ ] Pin `execute` do `Branch` está conectado ao pin `then` de `Set Velocity` (actor existente)?
- [ ] `Set Movement Mode` está presente no caminho `then` (`OutIsInAir == true`)? (OPCIONAL)
- [ ] `Set Movement Mode` está presente no caminho `else` (`OutIsInAir == false`)? (OPCIONAL)
- [ ] Ambos os caminhos convergem para `Set Actor Location`?

**Caminho 2 (Novo Actor):**
- [ ] `Branch` com `OutIsInAir` está presente após `Set Velocity` (novo actor)?
- [ ] Pin `Condition` do `Branch` está conectado ao pin `OutIsInAir` de `ParseStateUpdateFrameWithAnimation`?
- [ ] Pin `execute` do `Branch` está conectado ao pin `then` de `Set Velocity` (novo actor)?
- [ ] `Set Movement Mode` está presente no caminho `then` (`OutIsInAir == true`)? (OPCIONAL)
- [ ] `Set Movement Mode` está presente no caminho `else` (`OutIsInAir == false`)? (OPCIONAL)
- [ ] Ambos os caminhos convergem para `Set Actor Location`?

### **CORREÇÃO #3: NewTimestampMs (OPCIONAL)**

- [ ] `OutTimestampMs` está conectado ao pin `NewTimestampMs` de `UpdatePlayerStateBuffer`?

---

## 🧪 **TESTES RECOMENDADOS:**

### **Teste #1: Verificar animações em actors existentes**

1. **Cenário:** Conectar 2 clients ao servidor
2. **Ação:** Client 1 move-se, Client 2 observa
3. **Verificação:**
   - [ ] Client 2 vê o Client 1 se movendo com animações corretas?
   - [ ] Logs mostram `Set Velocity` sendo chamado para actors existentes?
   - [ ] `Speed` e `VelocityZ` estão sendo aplicados corretamente?

### **Teste #2: Verificar animações em novos actors**

1. **Cenário:** Conectar 1 client, depois conectar outro
2. **Ação:** Segundo client move-se imediatamente após spawn
3. **Verificação:**
   - [ ] Segundo client aparece com animações corretas desde o spawn?
   - [ ] Logs mostram `Set Velocity` sendo chamado após spawn?

### **Teste #3: Verificar animações de queda/pulo (se CORREÇÃO #2 implementada)**

1. **Cenário:** Client 1 pula ou cai de uma altura
2. **Ação:** Client 2 observa
3. **Verificação:**
   - [ ] Client 2 vê o Client 1 com animação de queda/pulo?
   - [ ] `Movement Mode` está sendo atualizado corretamente?

### **Teste #4: Verificar múltiplos clients**

1. **Cenário:** Conectar 3-4 clients simultaneamente
2. **Ação:** Todos movem-se simultaneamente
3. **Verificação:**
   - [ ] Todos os clients veem animações corretas uns dos outros?
   - [ ] Não há múltiplos spawns?
   - [ ] Performance está aceitável?

---

## 📝 **LOGS DE DEBUG RECOMENDADOS:**

Adicione os seguintes logs para verificar se as correções estão funcionando:

### **Log após Set Velocity (actor existente):**
```
[ProcessNextFrame] Set Velocity aplicado (actor existente) - PlayerID: {0}, Speed: {1}, VelocityZ: {2}, IsInAir: {3}
```

### **Log após Set Velocity (novo actor):**
```
[ProcessNextFrame] Set Velocity aplicado (novo actor) - PlayerID: {0}, Speed: {1}, VelocityZ: {2}, IsInAir: {3}
```

### **Log após Set Movement Mode (se implementado):**
```
[ProcessNextFrame] Movement Mode atualizado - PlayerID: {0}, IsInAir: {1}, Mode: {2}
```

---

## 🎯 **RESUMO DAS CORREÇÕES:**

1. **✅ CORREÇÃO #1 (CRÍTICA):** Adicionar `Cast to Character` → `Get Character Movement` → `Set Velocity` no caminho de atualização de actors existentes
2. **⚠️ CORREÇÃO #2 (OPCIONAL):** Adicionar uso de `OutIsInAir` para definir `Movement Mode`
3. **⚠️ CORREÇÃO #3 (OPCIONAL):** Conectar `OutTimestampMs` ao pin `NewTimestampMs` de `UpdatePlayerStateBuffer`

---

## 📚 **REFERÊNCIAS:**

- **Documento de Análise:** `ANALISE_DETALHADA_XML_PROCESSNEXTFRAME_ANIMACOES.md`
- **Guia de Implementação:** `GUIA_PASSO_A_PASSO_MODIFICAR_BLUEPRINT_ANIMACOES.md`
- **Resumo Executivo:** `RESUMO_EXECUTIVO_MODIFICACOES_BLUEPRINT_ANIMACOES.md`

---

**Este documento deve ser usado como guia passo a passo para implementar as correções necessárias no Blueprint `BP_NetMovementClient`.**
