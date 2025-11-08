# 🔍 **ANÁLISE: OnWSBinaryMessage XML - Problemas Identificados**

## ❌ **PROBLEMAS NO CÓDIGO ATUAL:**

### **PROBLEMA 1: Ordem Incorreta - ProcessBinaryBuffer está sendo chamado no caminho errado**

**Situação atual:**
- `K2Node_IfThenElse_14` verifica se `DataLength == 5` ✅ (correto)
- Mas `K2Node_CallFunction_43` (ProcessBinaryBuffer) está sendo chamado via `K2Node_Knot_22` que está conectado ao `then` de `K2Node_IfThenElse_13`
- `K2Node_IfThenElse_13` verifica se `ParsePlayerDisconnected` retornou `true`
- Isso significa que `ProcessBinaryBuffer` está sendo chamado **DEPOIS** de processar `PlayerDisconnected`, o que está **ERRADO**

**Correção:**
- `ProcessBinaryBuffer` deve estar no caminho `else` de `K2Node_IfThenElse_14` (quando `DataLength != 5`)
- **NÃO** deve estar no caminho `then` de `K2Node_IfThenElse_13`

---

### **PROBLEMA 2: Verificação de tipo está usando OutFrame.Data**

**Situação atual:**
- `K2Node_IfThenElse_12` verifica se `OutFrame.Data[0] == 3`
- Mas `OutFrame` só é válido se `ProcessBinaryBuffer` retornar `true`
- E `ProcessBinaryBuffer` não processa mensagens de 5 bytes corretamente

**Correção:**
- A verificação `Data[0] == 3` deve usar `Data` diretamente (do `OnWSBinaryMessage`)
- Deve estar no caminho `then` de `K2Node_IfThenElse_14` (quando `DataLength == 5`)

---

### **PROBLEMA 3: Lógica de verificação está invertida ou no lugar errado**

**Situação atual:**
- A verificação de tipo (`Data[0] == 3`) está usando `OutFrame.Data` (que vem de `ProcessBinaryBuffer`)
- Isso significa que só verifica o tipo **DEPOIS** de tentar processar com `ProcessBinaryBuffer`
- Mas mensagens de 5 bytes não devem passar por `ProcessBinaryBuffer`

**Correção:**
- A verificação de tipo deve usar `Data` diretamente
- Deve estar **ANTES** de chamar `ProcessBinaryBuffer`

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
Equal (Integer): DataLength == 5?
  → Is5Bytes
  ↓
Branch: Is5Bytes? ← K2Node_IfThenElse_14
  ├─ then: Get Array Item (Data, 0) → MessageType ← ADICIONAR!
          ↓
          Equal (Byte): MessageType == 3? ← ADICIONAR!
          → IsType3
          ↓
          Branch: IsType3? ← ADICIONAR!
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId) ← K2Node_CallFunction_60
                      ↓
                      Branch: ReturnValue? ← K2Node_IfThenElse_13
                          ├─ then: RemoveRemoteActor (OutPlayerId) ← K2Node_CallFunction_61
                          └─ else: Print String: "Erro ao parsear"
              └─ else: Print String: "Tipo desconhecido"
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID) ← MOVER AQUI!
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame
              └─ else: (não há frame completo)
```

---

## 🔧 **CORREÇÕES NECESSÁRIAS NO SEU CÓDIGO:**

### **CORREÇÃO 1: Mover ProcessBinaryBuffer para o caminho `else` de Is5Bytes**

**Situação atual:**
- `K2Node_CallFunction_43` (ProcessBinaryBuffer) está conectado via `K2Node_Knot_22` ao `then` de `K2Node_IfThenElse_13`

**Correção:**
- Desconectar `K2Node_CallFunction_43` de `K2Node_Knot_22`
- Conectar `K2Node_CallFunction_43` ao caminho `else` de `K2Node_IfThenElse_14` (quando `DataLength != 5`)

---

### **CORREÇÃO 2: Adicionar verificação de tipo ANTES de ParsePlayerDisconnected**

**Situação atual:**
- `K2Node_IfThenElse_12` verifica `OutFrame.Data[0] == 3` (depois de ProcessBinaryBuffer)
- Isso está **ERRADO** para mensagens de 5 bytes

**Correção:**
- Remover `K2Node_IfThenElse_12` e `K2Node_BreakStruct_2` e `K2Node_GetArrayItem_0` que usam `OutFrame`
- Adicionar **NOVOS** nós no caminho `then` de `K2Node_IfThenElse_14`:
  - `Get Array Item (Data, 0)` → `MessageType`
  - `Equal (Byte): MessageType == 3?` → `IsType3`
  - `Branch: IsType3?`
  - No `then` de `IsType3?`: chamar `ParsePlayerDisconnected`

---

### **CORREÇÃO 3: Remover lógica de verificação de tipo que usa OutFrame**

**Situação atual:**
- `K2Node_IfThenElse_12`, `K2Node_BreakStruct_2`, `K2Node_GetArrayItem_0` estão verificando `OutFrame.Data[0] == 3`
- Isso não faz sentido para mensagens de 5 bytes

**Correção:**
- Remover esses nós (ou movê-los para outro lugar se necessário para outra lógica)
- A verificação de tipo deve ser feita **ANTES** de `ProcessBinaryBuffer`, usando `Data` diretamente

---

## 📋 **RESUMO DAS CORREÇÕES:**

1. ✅ **Manter `K2Node_IfThenElse_14`** (verifica se `DataLength == 5`)
2. ❌ **Remover `K2Node_IfThenElse_12`** (verifica `OutFrame.Data[0] == 3` - está errado)
3. ✅ **Adicionar verificação de tipo no caminho `then` de `K2Node_IfThenElse_14`:**
   - `Get Array Item (Data, 0)` → `MessageType`
   - `Equal (Byte): MessageType == 3?` → `IsType3`
   - `Branch: IsType3?`
4. ✅ **Mover `K2Node_CallFunction_43` (ProcessBinaryBuffer) para o caminho `else` de `K2Node_IfThenElse_14`**
5. ✅ **Manter `K2Node_CallFunction_60` (ParsePlayerDisconnected) no caminho `then` de `IsType3?`**
6. ✅ **Manter `K2Node_CallFunction_61` (RemoveRemoteActor) no caminho `then` de `K2Node_IfThenElse_13`**

---

## 🎯 **ESTRUTURA FINAL (ORDEM CORRETA):**

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
                          └─ else: Print String: "Erro"
              └─ else: Print String: "Tipo desconhecido"
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID) ← MOVER AQUI!
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame
              └─ else: (não há frame completo)
```

