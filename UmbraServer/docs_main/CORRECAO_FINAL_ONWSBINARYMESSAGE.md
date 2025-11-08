# 🔧 **CORREÇÃO FINAL: OnWSBinaryMessage - Problemas Identificados**

## ❌ **PROBLEMAS NO CÓDIGO ATUAL:**

### **PROBLEMA 1: Verificação de tipo está usando `OutFrame.Data` em vez de `Data`**

**Situação atual:**
- `K2Node_IfThenElse_12` verifica se `OutFrame.Data[0] == 3`
- Mas `OutFrame` só existe **DEPOIS** de `ProcessBinaryBuffer`
- E `ProcessBinaryBuffer` está no caminho `else` de `Is5Bytes?`
- Isso significa que quando `DataLength == 5`, a verificação tenta usar `OutFrame` que ainda não foi criado

**Correção:**
- Remover `K2Node_BreakStruct_2`, `K2Node_GetArrayItem_0` que usam `OutFrame`
- Adicionar `Get Array Item (Data, 0)` **DIRETAMENTE** do `Data` recebido
- Usar esse valor para verificar se `MessageType == 3`

---

### **PROBLEMA 2: Ordem incorreta - Verificação de tipo está DEPOIS de ProcessBinaryBuffer**

**Situação atual:**
- `K2Node_IfThenElse_14` verifica se `DataLength == 5` ✅ (correto)
- No caminho `then` (quando é 5 bytes), verifica `OutFrame.Data[0] == 3` ❌ (errado - `OutFrame` não existe ainda)
- No caminho `else` (quando não é 5 bytes), chama `ProcessBinaryBuffer` ✅ (correto)

**Correção:**
- No caminho `then` de `K2Node_IfThenElse_14`:
  - Adicionar `Get Array Item (Data, 0)` → `MessageType`
  - Adicionar `Equal (Byte): MessageType == 3?` → `IsType3`
  - Adicionar `Branch: IsType3?`
  - No `then` de `IsType3?`: chamar `ParsePlayerDisconnected` e `RemoveRemoteActor`
  - No `else` de `IsType3?`: logar "Tipo desconhecido" (não processar)

---

### **PROBLEMA 3: Lógica de verificação está no lugar errado**

**Situação atual:**
- `K2Node_IfThenElse_12` está verificando `OutFrame.Data[0] == 3`
- Mas `OutFrame` vem de `ProcessBinaryBuffer`, que só é chamado quando `DataLength != 5`
- Isso significa que a verificação nunca será executada para mensagens de 5 bytes

**Correção:**
- Remover toda a lógica que usa `OutFrame.Data` para verificar o tipo
- Adicionar a verificação de tipo **ANTES** de qualquer processamento, usando `Data` diretamente

---

## ✅ **ESTRUTURA CORRETA (ORDEM EXATA):**

```
OnWSBinaryMessage (Data)
  ↓
Set IsFirstCall = true
  ↓
Branch: IsFirstCall?
  ├─ then: Print String: "Received binary message, size: " + ToString(DataLength)
  └─ else: (pular)
  ↓
Get Array Length (Data) → DataLength
  ↓
Equal (Integer): DataLength == 5? → Is5Bytes
  ↓
Branch: Is5Bytes? ← K2Node_IfThenElse_14
  ├─ then: Get Array Item (Data, 0) → MessageType ← ADICIONAR!
          ↓
          Equal (Byte): MessageType == 3? → IsType3 ← ADICIONAR!
          ↓
          Branch: IsType3? ← ADICIONAR!
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId) ← K2Node_CallFunction_60
                      ↓
                      Branch: ReturnValue? ← K2Node_IfThenElse_13
                          ├─ then: RemoveRemoteActor (OutPlayerId) ← K2Node_CallFunction_61
                                  ↓
                                  Print String: "PlayerDisconnected processado" ← K2Node_CallFunction_75
                          └─ else: Print String: "Erro ao parsear" ← K2Node_CallFunction_73
              └─ else: Print String: "Tipo desconhecido" ← K2Node_CallFunction_72
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID) ← K2Node_CallFunction_43
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame ← K2Node_CallFunction_71
              └─ else: (não há frame completo)
```

---

## 🔧 **CORREÇÕES ESPECÍFICAS NECESSÁRIAS:**

### **CORREÇÃO 1: Remover verificação que usa `OutFrame.Data`**

**Nós a remover ou modificar:**
- `K2Node_IfThenElse_12` (verifica `OutFrame.Data[0] == 3`) - **REMOVER**
- `K2Node_BreakStruct_2` (Break OutFrame) - **REMOVER** (ou mover para outro lugar se necessário)
- `K2Node_GetArrayItem_0` (que usa `OutFrame.Data`) - **REMOVER**
- `K2Node_PromotableOperator_2` (Equal Byte que compara `OutFrame.Data[0] == 3`) - **REMOVER**

**Ou manter apenas para outra lógica (se necessário), mas NÃO usar para verificar mensagens de 5 bytes.**

---

### **CORREÇÃO 2: Adicionar verificação de tipo usando `Data` diretamente**

**No caminho `then` de `K2Node_IfThenElse_14` (quando `DataLength == 5`):**

1. **Adicionar `Get Array Item`:**
   - Array: `Data` (do `OnWSBinaryMessage`, via `K2Node_Knot_15`)
   - Index: `0`
   - Output: `MessageType` (Byte)

2. **Adicionar `Equal (Byte)`:**
   - A: `MessageType`
   - B: `3`
   - ReturnValue: `IsType3` (Boolean)

3. **Adicionar `Branch: IsType3?`:**
   - Condition: `IsType3`
   - `then`: chamar `ParsePlayerDisconnected` (já existe: `K2Node_CallFunction_60`)
   - `else`: logar "Tipo desconhecido" (já existe: `K2Node_CallFunction_72`)

---

### **CORREÇÃO 3: Conectar `ParsePlayerDisconnected` corretamente**

**Situação atual:**
- `K2Node_CallFunction_60` (ParsePlayerDisconnected) está conectado ao `then` de `K2Node_IfThenElse_12`
- Mas `K2Node_IfThenElse_12` deve ser removido

**Correção:**
- Conectar `K2Node_CallFunction_60` ao `then` do **NOVO** `Branch: IsType3?`
- O `Data` para `ParsePlayerDisconnected` deve vir de `K2Node_Knot_15` (não de `OutFrame`)

---

### **CORREÇÃO 4: Garantir que `ProcessBinaryBuffer` está no caminho correto**

**Situação atual:**
- `K2Node_CallFunction_43` (ProcessBinaryBuffer) está conectado via `K2Node_Knot_22` ao `else` de `K2Node_IfThenElse_14`
- Isso está **CORRETO** ✅

**Manter como está!**

---

## 📋 **RESUMO DAS AÇÕES:**

1. ✅ **Manter `K2Node_IfThenElse_14`** (verifica se `DataLength == 5`)
2. ❌ **Remover `K2Node_IfThenElse_12`** (verifica `OutFrame.Data[0] == 3` - está errado)
3. ❌ **Remover `K2Node_BreakStruct_2`** (Break OutFrame - não necessário para verificação de 5 bytes)
4. ❌ **Remover `K2Node_GetArrayItem_0`** (que usa `OutFrame.Data` - não necessário)
5. ❌ **Remover `K2Node_PromotableOperator_2`** (Equal Byte que compara `OutFrame.Data[0] == 3` - não necessário)
6. ✅ **Adicionar `Get Array Item (Data, 0)`** no caminho `then` de `K2Node_IfThenElse_14`
7. ✅ **Adicionar `Equal (Byte): MessageType == 3?`** após `Get Array Item`
8. ✅ **Adicionar `Branch: IsType3?`** após `Equal`
9. ✅ **Conectar `K2Node_CallFunction_60` (ParsePlayerDisconnected)** ao `then` do novo `Branch: IsType3?`
10. ✅ **Conectar `K2Node_CallFunction_72` (Print String: "Tipo desconhecido")** ao `else` do novo `Branch: IsType3?`
11. ✅ **Manter `K2Node_CallFunction_43` (ProcessBinaryBuffer)** no caminho `else` de `K2Node_IfThenElse_14`

---

## 🎯 **ESTRUTURA FINAL CORRIGIDA:**

```
OnWSBinaryMessage (Data)
  ↓
Set IsFirstCall = true
  ↓
Branch: IsFirstCall?
  ├─ then: Print String: "Received binary message, size: " + ToString(DataLength)
  └─ else: (pular)
  ↓
Get Array Length (Data) → DataLength
  ↓
Equal (Integer): DataLength == 5? → Is5Bytes
  ↓
Branch: Is5Bytes? ← K2Node_IfThenElse_14
  ├─ then: Get Array Item (Data, 0) → MessageType ← ADICIONAR!
          ↓
          Equal (Byte): MessageType == 3? → IsType3 ← ADICIONAR!
          ↓
          Branch: IsType3? ← ADICIONAR!
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId) ← K2Node_CallFunction_60
                      ↓
                      Branch: ReturnValue? ← K2Node_IfThenElse_13
                          ├─ then: RemoveRemoteActor (OutPlayerId) ← K2Node_CallFunction_61
                                  ↓
                                  Print String: "PlayerDisconnected processado" ← K2Node_CallFunction_75
                          └─ else: Print String: "Erro ao parsear" ← K2Node_CallFunction_73
              └─ else: Print String: "Tipo desconhecido" ← K2Node_CallFunction_72
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID) ← K2Node_CallFunction_43
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame ← K2Node_CallFunction_71
              └─ else: (não há frame completo)
```

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **A verificação de tipo DEVE usar `Data` diretamente**, não `OutFrame.Data`
2. **A verificação de tipo DEVE estar no caminho `then` de `Is5Bytes?`**, antes de qualquer processamento
3. **Se for 5 bytes e tipo 3, NÃO chamar `ProcessBinaryBuffer`**
4. **Se NÃO for 5 bytes, chamar `ProcessBinaryBuffer` normalmente**

