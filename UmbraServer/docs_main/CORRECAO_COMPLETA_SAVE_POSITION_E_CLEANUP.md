# 🚨 **CORREÇÃO COMPLETA: Save Position + Cleanup Actors**

## 📋 **PROBLEMAS IDENTIFICADOS:**

1. ❌ **`SavePlayerPosition` tem `ErrorType=1`** - Target não conectado corretamente
2. ❌ **Posições não são salvas no banco de dados** - apenas no servidor (memória)
3. ❌ **Actors não são removidos na desconexão**
4. ❌ **Ao logar, spawnam no PlayerStart** - mesmo que posição esteja salva no servidor

---

## ✅ **CORREÇÃO 1: SavePlayerPosition Target (CRÍTICO)**

### **LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

### **PROBLEMA:**

No XML fornecido, o `SavePlayerPosition` (`K2Node_CallFunction_2`) está usando:
```
Target: K2Node_VariableSet_0 (Output_Get)
```

**PROBLEMA:** O `Output_Get` do `Set Variable` pode não funcionar corretamente em todos os contextos, causando `ErrorType=1`.

### **SOLUÇÃO:**

**USAR `Variable Get` diretamente ao invés de `Output_Get` do `Set Variable`.**

### **COMO CORRIGIR:**

#### **PASSO 1: Localizar o SavePlayerPosition**

1. **Abrir `BP_Player:SavePositionTimer`**
2. **Localizar `K2Node_CallFunction_2` (SavePlayerPosition)**
3. **Verificar conexão do pin `Target` (self):**
   - Atualmente conectado a `K2Node_VariableSet_0` (Output_Get)
   - **DEVE SER:** `K2Node_VariableGet_2` (MyGameInstance) diretamente

#### **PASSO 2: Desconectar Output_Get**

1. **Desconectar** o pin `Target` do `SavePlayerPosition` do `Output_Get` do `Set Variable`

#### **PASSO 3: Conectar Variable Get**

1. **Localizar `K2Node_VariableGet_2` (MyGameInstance)**
   - Deve estar na posição X=4272, Y=144
2. **Conectar** o pin de saída (`MyGameInstance`) diretamente ao pin `Target` (self) do `SavePlayerPosition`

**RESULTADO:**
```
K2Node_VariableGet_2 (MyGameInstance)
  ↓ (conexão direta)
K2Node_CallFunction_2 (SavePlayerPosition)
  - Target: MyGameInstance (DIRETO) ✅
```

---

## ✅ **CORREÇÃO 2: Garantir que SavePlayerPosition é Chamado**

### **VERIFICAÇÃO:**

No XML fornecido, o fluxo está:
```
K2Node_IfThenElse_1 (Is Valid)
  ↓ (then)
K2Node_IfThenElse_0 (Boolean AND: PlayerID > 0 AND Location != 0,0,0)
  ↓ (then)
K2Node_CallFunction_11 (Print String)
  ↓
K2Node_CallFunction_2 (SavePlayerPosition)
```

**ESTÁ CORRETO!** Mas vamos garantir que:
1. `Is Valid (MyGameInstance)` está conectado corretamente
2. `Boolean AND` está recebendo ambos os valores corretamente

### **VERIFICAR:**

1. **`K2Node_IfThenElse_1` (Is Valid):**
   - `Condition` deve estar conectado a `K2Node_CallFunction_6` (Is Valid) → `ReturnValue`
   - **VERIFICAR:** Está conectado? ✅ (XML mostra: `LinkedTo=(K2Node_CallFunction_6 22FE589D4CC86D093D2123ACE9BBE7AB,)`)

2. **`K2Node_IfThenElse_0` (Boolean AND):**
   - `Condition` deve estar conectado a `K2Node_CommutativeAssociativeBinaryOperator_0` (Boolean AND) → `ReturnValue`
   - **VERIFICAR:** Está conectado? ✅ (XML mostra: `LinkedTo=(K2Node_CommutativeAssociativeBinaryOperator_0 716B0B914DD52B987FFB7EB6077623BB,)`)

**CONCLUSÃO:** A lógica está correta. O problema é apenas o `Target` do `SavePlayerPosition`.

---

## ✅ **CORREÇÃO 3: Cleanup Remote Actors na Desconexão**

### **LOCALIZAÇÃO:** `BP_NetMovementClient:Event EndPlay`

### **PROBLEMA:**

O usuário relatou que **actors não estão sendo removidos na desconexão**.

### **SOLUÇÃO:**

**GARANTIR que `CleanupRemoteActors` seja chamado ANTES de qualquer verificação de WebSocket válido.**

### **COMO CORRIGIR:**

#### **ESTRUTURA ATUAL (PROBLEMÁTICA):**
```
Event EndPlay
  ↓
Is Valid (WebSocket)?
  ├─ True:
  │    ↓
  │   Close (WebSocket)
  │    ↓
  │   CleanupRemoteActors  ← SÓ EXECUTA SE WEBSOCKET FOR VÁLIDO ❌
  │
  └─ False:
       (nada)
```

#### **ESTRUTURA CORRIGIDA:**
```
Event EndPlay
  ↓
Save Player Position (se necessário)
  ↓
CleanupRemoteActors  ← SEMPRE EXECUTA ✅
  ↓
Is Valid (WebSocket)?
  ├─ True:
  │    ↓
  │   Close (WebSocket)
  │
  └─ False:
       (nada)
```

### **IMPLEMENTAÇÃO:**

1. **Localizar `Event EndPlay` em `BP_NetMovementClient`**
2. **Mover `CleanupRemoteActors` para ANTES do `Is Valid (WebSocket)`**
3. **Garantir que `CleanupRemoteActors` sempre execute**, independente do estado do WebSocket

---

## ✅ **CORREÇÃO 4: Salvar Posição na Desconexão**

### **LOCALIZAÇÃO:** `BP_NetMovementClient:Event EndPlay`

### **PROBLEMA:**

O usuário relatou que **as posições estão sendo salvas no servidor (memória) mas não no banco de dados**.

### **SOLUÇÃO:**

**ADICIONAR chamada a `SavePlayerPosition` no `Event EndPlay` ANTES de `CleanupRemoteActors`.**

### **COMO IMPLEMENTAR:**

#### **ESTRUTURA:**

```
Event EndPlay
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ↓ (Cast Success)
Get Active Player ID
  ↓
Greater (Integer): PlayerID > 0?
  ↓
Get Actor Location (do Pawn local)
  ↓
Not Equal (Vector): Location != (0,0,0)?
  ↓
Boolean AND: (PlayerID > 0) AND (Location != 0,0,0)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Save Player Position
  │     - Target: MyGameInstance
  │     - PlayerID: Get Active Player ID
  │     - Position: Get Actor Location
  │     - CurrentZone: Current Zone
  │    ↓
  │   CleanupRemoteActors
  │
  └─ False:
       ↓
      CleanupRemoteActors
```

### **IMPLEMENTAÇÃO PASSO A PASSO:**

#### **PASSO 1: Obter Referência do Character Local**

1. **Adicionar `Get First Player Controller`**
2. **Adicionar `Get Pawn`** (conectado ao Controller)
3. **Adicionar `Cast To Character`** (conectado ao Pawn)
4. **Adicionar `Get Actor Location`** (conectado ao Character)

#### **PASSO 2: Obter MyGameInstance**

1. **Adicionar `Get Game Instance`**
2. **Adicionar `Cast To UmbraGameInstance`**
3. **Adicionar `Set Variable: MyGameInstance`** (ou usar diretamente)

#### **PASSO 3: Validar e Salvar**

1. **Adicionar `Get Active Player ID`** (conectado ao MyGameInstance)
2. **Adicionar `Greater (Integer)`:** PlayerID > 0
3. **Adicionar `Not Equal (Vector)`:** Location != (0,0,0)
4. **Adicionar `Boolean AND`:** (PlayerID > 0) AND (Location != 0,0,0)
5. **Adicionar `Branch`**
6. **Conectar `then` (True) ao `Save Player Position`**
7. **Conectar `Save Player Position` ao `CleanupRemoteActors`**
8. **Conectar `else` (False) ao `CleanupRemoteActors`**

---

## ✅ **CORREÇÃO 5: Aplicar Posição Salva no Spawn**

### **LOCALIZAÇÃO:** `BP_NetMovementClient:OnWSConnected`

### **PROBLEMA:**

O usuário relatou que **ao logar, spawnam no PlayerStart**, mesmo que a posição esteja salva no servidor.

### **SOLUÇÃO:**

**APLICAR a posição salva do banco de dados APÓS o WebSocket conectar, mas ANTES de começar a enviar updates.**

### **VERIFICAÇÃO:**

**VERIFICAR se já existe lógica para aplicar posição salva no `OnWSConnected`.**

**SE NÃO EXISTIR:**
1. **Adicionar `Get Game Instance`**
2. **Adicionar `Cast To UmbraGameInstance`**
3. **Adicionar `Get Active Character`** (retorna `FUmbraPlayerData`)
4. **Adicionar `Break Struct`** (para obter `Position`)
5. **Adicionar `Not Equal (Vector)`:** Position != (0,0,0)
6. **Adicionar `Branch`**
7. **Conectar `then` (True) ao `Get First Player Pawn Helper`**
8. **Adicionar `Set Actor Location`** (conectado ao Pawn e Position)

**SE JÁ EXISTIR:**
- **VERIFICAR** se está sendo executado corretamente
- **VERIFICAR** se a posição está sendo obtida do `CurrentPlayers` array atualizado

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **PRIORIDADE 1: SavePlayerPosition Target**

- [ ] **SavePositionTimer:** Desconectar `Target` do `SavePlayerPosition` do `Output_Get` do `Set Variable`
- [ ] **SavePositionTimer:** Conectar `K2Node_VariableGet_2` (MyGameInstance) diretamente ao `Target` do `SavePlayerPosition`
- [ ] **Testar:** Compilar e verificar se `ErrorType=1` desapareceu

### **PRIORIDADE 2: Salvar Posição na Desconexão**

- [ ] **Event EndPlay:** Adicionar lógica para obter referência do Character local
- [ ] **Event EndPlay:** Adicionar validações (PlayerID > 0, Location != 0,0,0)
- [ ] **Event EndPlay:** Adicionar chamada a `SavePlayerPosition` antes de `CleanupRemoteActors`
- [ ] **Testar:** Fechar client, verificar se posição foi salva no banco

### **PRIORIDADE 3: Cleanup Remote Actors**

- [ ] **Event EndPlay:** Mover `CleanupRemoteActors` para ANTES do `Is Valid (WebSocket)`
- [ ] **Testar:** Conectar 2 clients, fechar um, verificar se remote actor foi destruído

### **PRIORIDADE 4: Aplicar Posição no Spawn**

- [ ] **OnWSConnected:** Verificar se lógica de aplicar posição existe
- [ ] **OnWSConnected:** Se não existir, adicionar lógica para aplicar posição salva
- [ ] **Testar:** Fechar e reconectar client, verificar se spawna na posição salva

---

## 🧪 **TESTES FINAIS:**

### **TESTE 1: Save Position Funcionando**

1. Conectar um client
2. Mover o personagem para uma posição conhecida (ex: X=100, Y=200, Z=50)
3. Aguardar 6 segundos (timer de 5s + margem)
4. **VERIFICAR NO BANCO:**
   ```sql
   SELECT pos_x, pos_y, pos_z FROM players WHERE id = [PlayerID];
   ```
5. **VERIFICAR:** Valores devem estar atualizados (não mais 0,0,0)

### **TESTE 2: Save Position na Desconexão**

1. Conectar um client
2. Mover o personagem para uma posição conhecida (ex: X=500, Y=1000, Z=100)
3. Fechar o client imediatamente (sem aguardar timer)
4. **VERIFICAR NO BANCO:**
   ```sql
   SELECT pos_x, pos_y, pos_z FROM players WHERE id = [PlayerID];
   ```
5. **VERIFICAR:** Valores devem estar atualizados (não mais 0,0,0)

### **TESTE 3: Spawn na Posição Salva**

1. Conectar um client
2. Mover o personagem para uma posição conhecida (ex: X=500, Y=1000, Z=100)
3. Aguardar 6 segundos (salvar)
4. Fechar o client
5. Reconectar o mesmo client
6. **VERIFICAR:** Deve spawnar na posição salva (X=500, Y=1000, Z=100), não no PlayerStart

### **TESTE 4: Cleanup Remote Actors**

1. Conectar Client 1
2. Conectar Client 2
3. **VERIFICAR:** Client 1 vê Client 2 (remote actor spawnado)
4. Fechar Client 2
5. **VERIFICAR:** Client 1 não vê mais Client 2 (remote actor destruído)

---

## 📝 **LOGS PARA ADICIONAR:**

### **NO SavePositionTimer:**

**APÓS `SavePlayerPosition`:**
```
Print String: "✅ SavePositionTimer - Posição salva no banco (PlayerID: {PlayerID}, Position: X={X}, Y={Y}, Z={Z})"
```

### **NO Event EndPlay:**

**ANTES DE `SavePlayerPosition`:**
```
Print String: "🔵 Event EndPlay - Salvando posição final (PlayerID: {PlayerID}, Position: X={X}, Y={Y}, Z={Z})"
```

**APÓS `SavePlayerPosition`:**
```
Print String: "✅ Event EndPlay - Posição final salva no banco"
```

**ANTES DE `CleanupRemoteActors`:**
```
Print String: "🔵 Event EndPlay - Limpando remote actors..."
```

---

**Status:** 🚨 **PRIORIDADE MÁXIMA - IMPLEMENTAR AGORA**

