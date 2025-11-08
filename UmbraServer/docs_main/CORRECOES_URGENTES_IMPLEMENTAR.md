# 🚨 **CORREÇÕES URGENTES: Implementar Agora**

## 📋 **PROBLEMAS REPORTADOS:**

1. ❌ **Posições não são salvas** no banco de dados ← **PRIORIDADE MÁXIMA**
2. ❌ **Remote actors não são destruídos** quando um client fecha
3. ❌ **Todos os personagens spawnam no mesmo lugar** (causa colisão e travamento)
   - **CAUSA:** Problema 1 (posições não salvas) - todos têm (0,0,0) no banco
   - **SOLUÇÃO:** Corrigir Problema 1 primeiro. Offset só se necessário depois.

---

## ✅ **CORREÇÃO 1: Save Position (PRIORIDADE MÁXIMA - 10 minutos)**

### **LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

### **PROBLEMA:**
O `Target` do `SavePlayerPosition` está conectado via `K2Node_Knot_0` (Reroute), causando erro:
```
ErrorType=1
ErrorMsg="Este Blueprint (próprio) não é UmbraGameInstance, por isso \" Target \" deve ter uma conexão."
```

**RESULTADO:** Posições não são salvas no banco, então todos os players sempre têm (0,0,0) e spawnam no mesmo lugar.

### **SOLUÇÃO:**

**ESTRUTURA ATUAL (COM ERRO):**
```
Set Variable: MyGameInstance
  ↓
Save Player Position
  - Target: MyGameInstance (via Knot)  ← ERRO ❌
```

**ESTRUTURA CORRIGIDA:**
```
Set Variable: MyGameInstance
  ↓
Save Player Position
  - Target: MyGameInstance (DIRETO, sem Knot)  ← CORRIGIDO ✅
```

### **COMO FAZER:**

#### **PASSO 1: Remover o Knot**
1. Localizar `K2Node_Knot_0` (Reroute node)
2. **Desconectar:**
   - `MyGameInstance` (de `K2Node_VariableGet_2`) do `InputPin` do Knot
   - `OutputPin` do Knot do `Target` do `SavePlayerPosition`
3. **Deletar** o Knot

#### **PASSO 2: Conectar Diretamente**
1. Conectar `MyGameInstance` (de `K2Node_VariableGet_2`) **diretamente** ao `Target` do `SavePlayerPosition`

**VER DOCUMENTO COMPLETO:** `GUIA_FOCADO_CORRIGIR_SAVE_POSITION.md`

---

## ✅ **CORREÇÃO 2: Remote Actors (5 minutos)**

### **LOCALIZAÇÃO:** `BP_NetMovementClient:EventGraph - Event EndPlay`

### **PROBLEMA:**
`CleanupRemoteActors` só executa se `WebSocket` for válido. Se o WebSocket já foi fechado ou é inválido, os remote actors nunca são limpos.

### **SOLUÇÃO:**

**ESTRUTURA ATUAL (INCORRETA):**
```
Event EndPlay
  ↓
Is Valid (Local Pawn)?
  ↓ (True)
Save Player Position
  ↓
Is Valid (WebSocket)?
  ↓ (True)
Close (WebSocket)
  ↓
CleanupRemoteActors  ← SÓ EXECUTA SE WebSocket VÁLIDO ❌
```

**ESTRUTURA CORRIGIDA:**
```
Event EndPlay
  ↓
Is Valid (Local Pawn)?
  ↓ (True)
Save Player Position
  ↓
CleanupRemoteActors  ← SEMPRE EXECUTA ✅
  ↓
Is Valid (WebSocket)?
  ↓ (True)
Close (WebSocket)
```

### **COMO FAZER:**

1. **No `Event EndPlay` do `BP_NetMovementClient`:**
   - Localizar o nó `CleanupRemoteActors`
   - **Desconectar** do `then` de `Close (WebSocket)`
   - **Conectar** ao `then` de `Save Player Position`
   - **Conectar** o `then` de `CleanupRemoteActors` ao Branch `Is Valid (WebSocket)`

---

## ✅ **CORREÇÃO 2: Save Position (10 minutos)**

### **LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

### **PROBLEMA:**
O `Target` do `SavePlayerPosition` está conectado via `K2Node_Knot_0` (Reroute), causando erro:
```
ErrorType=1
ErrorMsg="Este Blueprint (próprio) não é UmbraGameInstance, por isso \" Target \" deve ter uma conexão."
```

### **SOLUÇÃO:**

**ESTRUTURA ATUAL (COM ERRO):**
```
Set Variable: MyGameInstance
  ↓
Get Active Player ID
  ↓
Get Actor Location
  ↓
Branch (Condition: Boolean AND)
  ↓ (True)
Save Player Position
  - Target: MyGameInstance (via Knot)  ← ERRO ❌
```

**ESTRUTURA CORRIGIDA:**
```
Set Variable: MyGameInstance
  ↓
Is Valid (MyGameInstance)?  ← ADICIONAR ✅
  ↓ (True)
Get Active Player ID
  ↓
Get Actor Location
  ↓
Branch (Condition: Boolean AND)
  ↓ (True)
Save Player Position
  - Target: MyGameInstance (DIRETO, sem Knot)  ← CORRIGIDO ✅
```

### **COMO FAZER:**

#### **PASSO 1: Remover o Knot**
1. Localizar `K2Node_Knot_0` (Reroute node)
2. **Desconectar:**
   - `MyGameInstance` (de `K2Node_VariableGet_2`) do `InputPin` do Knot
   - `OutputPin` do Knot do `Target` do `SavePlayerPosition`
3. **Deletar** o Knot

#### **PASSO 2: Conectar Diretamente**
1. Conectar `MyGameInstance` (de `K2Node_VariableGet_2`) **diretamente** ao `Target` do `SavePlayerPosition`

#### **PASSO 3: Adicionar Validação**
1. Após `Set Variable: MyGameInstance`:
   - Adicionar `Is Valid`
   - Conectar `MyGameInstance` ao `Object` do `Is Valid`
   - Adicionar `Branch`
   - Conectar `ReturnValue` (Boolean) do `Is Valid` ao `Condition` do `Branch`
   - Conectar o `then` do `Set Variable` ao `execute` do `Branch`
   - Conectar o `then` (True) do `Branch` ao resto do fluxo (Get Active Player ID)

---

## 🔍 **VERIFICAÇÃO ADICIONAL: CleanupRemoteActors**

### **VERIFICAR SE ESTÁ IMPLEMENTADO CORRETAMENTE:**

**NO `BP_NetMovementClient:CleanupRemoteActors`:**

A função deve ter:
```
CleanupRemoteActors (Custom Event)
  ↓
ForEachLoop (RemoteActors)
  ├─ LoopBody:
  │   ├─ Get Array Element (do ForEachLoop) → ActorRef
  │   ├─ Is Valid (ActorRef)
  │   ├─ Branch: Is Valid?
  │   │   ├─ then (True): Destroy Actor (ActorRef)
  │   │   └─ else (False): [Ignorar]
  │   └─ [Continue Loop]
  ↓
Clear Array (RemoteActors)
  ↓
Clear Array (RemoteActorIds)  ← IMPORTANTE: Deve limpar AMBOS os arrays
```

**SE NÃO ESTIVER CORRETO:**
- Verificar se usa `Array Element` do `ForEachLoop` (não `RemoteActorRef`)
- Verificar se limpa `RemoteActorIds` também

---

## 🧪 **TESTES:**

### **TESTE 1: Remote Actors**
1. Conectar Client 1
2. Conectar Client 2
3. **VERIFICAR:** Ambos veem os remote actors
4. Fechar Client 2 (ESC)
5. **VERIFICAR:** Client 1 não deve mais ver o actor de Client 2
6. Conectar Client 3
7. **VERIFICAR:** Client 1 deve ver apenas Client 3 (não Client 2)

### **TESTE 2: Save Position**
1. Conectar Client
2. Mover personagem para posição conhecida (ex: X=100, Y=200, Z=50)
3. Aguardar 6 segundos (timer de 5s + margem)
4. **VERIFICAR NO BANCO:**
   ```sql
   SELECT pos_x, pos_y, pos_z FROM players WHERE id = [PlayerID];
   ```
5. **VERIFICAR:** Valores devem estar atualizados
6. Fechar o client
7. **VERIFICAR:** Posição final deve ser salva no `Event EndPlay`

---

## 📝 **LOGS PARA ADICIONAR (OPCIONAL, MAS ÚTIL):**

### **NO Event EndPlay:**
```
Print String: "Event EndPlay - Cleaning up remote actors..."
```

### **NO SavePositionTimer:**
```
Print String: "SavePositionTimer - MyGameInstance válido: {IsValid}"
Print String: "SavePositionTimer - Chamando SavePlayerPosition (PlayerID: {PlayerID})"
```

---

## ⚠️ **CORREÇÃO 3: Spawn no Mesmo Lugar (SÓ SE NECESSÁRIO DEPOIS)**

### **IMPORTANTE:**

**Esta correção só deve ser implementada DEPOIS de corrigir o Save Position (Correção 1).**

**Se o Save Position estiver funcionando corretamente:**
- Cada player terá sua própria posição salva no banco
- Cada player spawnará em sua posição salva
- **NÃO será necessário adicionar offset**

**Só adicionar offset se:**
- Save Position estiver funcionando
- Mas múltiplos players ainda spawnarem no mesmo lugar (mesma posição salva)
- Isso seria raro, mas possível

### **LOCALIZAÇÃO:** `BP_NetMovementClient:OnWSConnected`

### **PROBLEMA:**
Todos os personagens spawnam na mesma posição, causando colisão e travamento.

### **SOLUÇÃO:**

**ESTRUTURA ATUAL (PROBLEMÁTICA):**
```
Not Equal (Position != 0,0,0)?
  ↓ (True)
Set Actor Location
  - NewLocation: Position (direto)
  ← PROBLEMA: Todos usam mesma posição ❌
```

**ESTRUTURA CORRIGIDA:**
```
Not Equal (Position != 0,0,0)?
  ↓ (True)
Get Active Player ID
  ↓
Multiply (PlayerID * 200.0) → OffsetX
  ↓
Make Vector (OffsetX, 0.0, 0.0) → Offset
  ↓
Add Vector (Position + Offset) → FinalPosition
  ↓
Set Actor Location
  - NewLocation: FinalPosition (Position + Offset) ✅
```

### **COMO FAZER:**

#### **PASSO 1: Adicionar Get Active Player ID**
1. Após `Not Equal (Position != 0,0,0)?` → True
2. Adicionar `Get Active Player ID`
3. Target: `MyGameInstance`
4. Conectar `then` do `Not Equal` ao `execute` do `Get Active Player ID`

#### **PASSO 2: Calcular Offset**
1. Adicionar `Multiply (Double Double)`
2. A: `PlayerID` (do Get Active Player ID)
3. B: `200.0` (literal)
4. ReturnValue: `OffsetX`

#### **PASSO 3: Criar Vector Offset**
1. Adicionar `Make Vector`
2. X: `OffsetX` (do Multiply)
3. Y: `0.0`
4. Z: `0.0`
5. ReturnValue: `Offset`

#### **PASSO 4: Somar Position + Offset**
1. Adicionar `Add Vector`
2. A: `Position` (do Break UmbraPlayerData)
3. B: `Offset` (do Make Vector)
4. ReturnValue: `FinalPosition`

#### **PASSO 5: Aplicar Posição Final**
1. Modificar `Set Actor Location`
2. Desconectar `Position` do `NewLocation`
3. Conectar `FinalPosition` ao `NewLocation`

**RESULTADO:**
- PlayerID 1: Spawna em `Position + (200, 0, 0)`
- PlayerID 2: Spawna em `Position + (400, 0, 0)`
- PlayerID 3: Spawna em `Position + (600, 0, 0)`
- Cada player spawna em posição única! ✅

**VER DOCUMENTO COMPLETO:** `GUIA_VISUAL_CORRIGIR_SPAWN_MESMO_LUGAR.md`

---

## ✅ **CHECKLIST FINAL (POR PRIORIDADE):**

### **PRIORIDADE 1: Save Position (IMPLEMENTAR PRIMEIRO)**

- [ ] **SavePositionTimer:** Remover `K2Node_Knot_0` (Knot)
- [ ] **SavePositionTimer:** Conectar `MyGameInstance` diretamente ao `Target` do `SavePlayerPosition`
- [ ] **Testar:** Compilar e verificar se erro `ErrorType=1` desapareceu
- [ ] **Testar:** Mover personagem, aguardar 6s, verificar banco de dados (pos_x, pos_y, pos_z atualizados)
- [ ] **Testar:** Fechar e reconectar - deve spawnar na posição salva (não PlayerStart)
- [ ] **Testar:** Múltiplos players - cada um deve spawnar em sua posição salva

### **PRIORIDADE 2: Remote Actors**

- [ ] **Event EndPlay:** Mover `CleanupRemoteActors` para antes do Branch `IsValid(WebSocket)`
- [ ] **Verificar:** `CleanupRemoteActors` limpa ambos os arrays (`RemoteActors` e `RemoteActorIds`)
- [ ] **Testar:** Conectar 2 clients, fechar um, verificar se remote actor foi destruído

### **PRIORIDADE 3: Offset (SÓ SE NECESSÁRIO)**

- [ ] **Aguardar:** Implementar apenas se, após corrigir Save Position, ainda houver problema de spawn no mesmo lugar
- [ ] **OnWSConnected:** Adicionar offset por PlayerID antes de aplicar posição (se necessário)

---

**Status:** 🚨 **URGENTE - IMPLEMENTAR AGORA**

