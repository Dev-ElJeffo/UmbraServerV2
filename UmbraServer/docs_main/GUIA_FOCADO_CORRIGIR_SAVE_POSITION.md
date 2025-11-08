# 🎯 **GUIA FOCADO: Corrigir Save Position (PRIORIDADE MÁXIMA)**

## 🔍 **PROBLEMA REAL:**

**As posições NÃO estão sendo salvas no banco de dados**, então:
- Todos os players sempre têm `pos_x=0, pos_y=0, pos_z=0` no banco
- Quando spawnam, todos usam a mesma posição padrão (PlayerStart)
- Resultado: Todos spawnam no mesmo lugar

**SOLUÇÃO:** Corrigir `SavePlayerPosition` primeiro. Depois, cada player terá sua própria posição salva e spawnará em lugares diferentes naturalmente.

---

## ✅ **CORREÇÃO: SavePlayerPosition**

### **LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

### **PROBLEMA IDENTIFICADO:**

**Erro no nó `SavePlayerPosition`:**
```
ErrorType=1
ErrorMsg="Este Blueprint (próprio) não é UmbraGameInstance, por isso \" Target \" deve ter uma conexão."
```

**CAUSA:** O `Target` está conectado via `K2Node_Knot_0` (Reroute), que pode não estar propagando corretamente.

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Localizar o Knot**

1. **Abrir `BP_Player:SavePositionTimer`**
2. **Localizar `K2Node_Knot_0`** (Reroute node)
   - Deve estar entre `K2Node_VariableGet_2` (MyGameInstance) e `K2Node_CallFunction_2` (SavePlayerPosition)

---

### **PASSO 2: Desconectar Conexões do Knot**

1. **Desconectar `MyGameInstance` do `InputPin` do Knot:**
   - Localizar `K2Node_VariableGet_2` (MyGameInstance)
   - Desconectar o pin de saída do `InputPin` do Knot

2. **Desconectar `OutputPin` do Knot do `Target` do SavePlayerPosition:**
   - Localizar `K2Node_CallFunction_2` (SavePlayerPosition)
   - Desconectar o `OutputPin` do Knot do pin `Target` (self) do SavePlayerPosition

---

### **PASSO 3: Deletar o Knot**

1. **Selecionar `K2Node_Knot_0`**
2. **Deletar** (Delete ou Del)

---

### **PASSO 4: Conectar Diretamente**

1. **Conectar `MyGameInstance` diretamente ao `Target` do SavePlayerPosition:**
   - Localizar `K2Node_VariableGet_2` (MyGameInstance)
   - Arrastar do pin de saída (`MyGameInstance`)
   - Conectar diretamente ao pin `Target` (self) do `K2Node_CallFunction_2` (SavePlayerPosition)

**RESULTADO:**
```
K2Node_VariableGet_2 (MyGameInstance)
  ↓ (conexão direta, sem Knot)
K2Node_CallFunction_2 (SavePlayerPosition)
  - Target: MyGameInstance (DIRETO) ✅
```

---

## ✅ **ESTRUTURA FINAL CORRIGIDA:**

```
SavePositionTimer (Event)
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ↓ (Cast Success)
Set Variable: MyGameInstance
  ↓
Get Active Player ID
  ↓
Greater (Integer): PlayerID > 0?
  ↓
Get Actor Location
  ↓
Not Equal (Vector): Location != (0,0,0)?
  ↓
Boolean AND: (PlayerID > 0) AND (Location != 0,0,0)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Save Player Position
  │     - Target: MyGameInstance (DIRETO, sem Knot)  ← CORRIGIDO ✅
  │     - PlayerID: Get Active Player ID
  │     - Position: Get Actor Location
  │     - CurrentZone: Current Zone
  │
  └─ False:
       ↓
      [Não conectado - opcional: adicionar log]
```

---

## 🧪 **TESTE IMEDIATO:**

### **TESTE 1: Verificar Erro Removido**

1. **Compilar o Blueprint**
2. **Verificar:** O erro `ErrorType=1` no `SavePlayerPosition` deve desaparecer
3. **Verificar:** O pin `Target` deve estar conectado (não mais desconectado)

---

### **TESTE 2: Verificar se Está Salvando**

1. **Conectar um client**
2. **Mover o personagem** para uma posição conhecida (ex: X=100, Y=200, Z=50)
3. **Aguardar 6 segundos** (timer de 5s + margem)
4. **VERIFICAR NO BANCO:**
   ```sql
   SELECT pos_x, pos_y, pos_z, last_played_at 
   FROM players 
   WHERE id = [PlayerID];
   ```
5. **VERIFICAR:**
   - `pos_x` deve ser `100` (ou próximo)
   - `pos_y` deve ser `200` (ou próximo)
   - `pos_z` deve ser `50` (ou próximo)
   - `last_played_at` deve estar atualizado

---

### **TESTE 3: Verificar Spawn na Posição Salva**

1. **Conectar um client**
2. **Mover o personagem** para uma posição conhecida (ex: X=500, Y=1000, Z=100)
3. **Aguardar 6 segundos** (salvar)
4. **Fechar o client**
5. **Reconectar o mesmo client**
6. **VERIFICAR:** Deve spawnar na posição salva (X=500, Y=1000, Z=100), não no PlayerStart

---

### **TESTE 4: Verificar Múltiplos Players**

1. **Conectar Client 1**
2. **Mover Client 1** para posição A (ex: X=100, Y=200, Z=50)
3. **Aguardar 6 segundos** (salvar)
4. **Conectar Client 2**
5. **Mover Client 2** para posição B (ex: X=300, Y=400, Z=50)
6. **Aguardar 6 segundos** (salvar)
7. **Fechar ambos**
8. **Reconectar ambos**
9. **VERIFICAR:**
   - Client 1 deve spawnar em posição A (X=100, Y=200, Z=50)
   - Client 2 deve spawnar em posição B (X=300, Y=400, Z=50)
   - **NÃO devem spawnar no mesmo lugar!**

---

## 📝 **LOGS PARA ADICIONAR (OPCIONAL, MAS ÚTIL):**

### **NO SavePositionTimer:**

**APÓS `Set Variable: MyGameInstance`:**
```
Is Valid (MyGameInstance)
  ↓
Print String: "SavePositionTimer - MyGameInstance válido: {IsValid}"
```

**ANTES DE `Save Player Position`:**
```
Format Text: "SavePositionTimer - PlayerID: {0}, Position: X={1}, Y={2}, Z={3}"
  - {0}: PlayerID
  - {1}: Location.X
  - {2}: Location.Y
  - {3}: Location.Z
  ↓
Print String
```

**APÓS `Save Player Position`:**
```
Print String: "SavePositionTimer - SavePlayerPosition chamado com sucesso"
```

---

## 🔍 **VERIFICAÇÃO ADICIONAL:**

### **Verificar se MyGameInstance está inicializado no BeginPlay:**

**NO `BP_Player:EventGraph (BeginPlay)`:**
- Verificar se existe:
  ```
  BeginPlay
    ↓
  Get Game Instance
    ↓
  Cast To UmbraGameInstance
    ↓ (Cast Success)
  Set Variable: MyGameInstance
  ```

**SE NÃO EXISTIR:**
- Adicionar essa lógica antes do `Set Timer (SavePositionTimer)`

---

## ⚠️ **IMPORTANTE:**

### **NÃO ADICIONAR OFFSET AINDA!**

**Depois de corrigir SavePlayerPosition:**
1. Testar se as posições estão sendo salvas
2. Testar se cada player spawna em sua posição salva
3. **SE** ainda houver problema de múltiplos players spawnando no mesmo lugar (mesma posição salva)
4. **AÍ SIM** considerar adicionar offset por PlayerID

**Mas provavelmente não será necessário**, pois cada player terá sua própria posição salva.

---

## 📊 **RESUMO:**

1. ✅ **Remover `K2Node_Knot_0`** (Knot)
2. ✅ **Conectar `MyGameInstance` diretamente ao `Target` do `SavePlayerPosition`**
3. ✅ **Testar se posições estão sendo salvas no banco**
4. ✅ **Testar se cada player spawna em sua posição salva**

**Status:** 🚨 **PRIORIDADE MÁXIMA - IMPLEMENTAR AGORA**

