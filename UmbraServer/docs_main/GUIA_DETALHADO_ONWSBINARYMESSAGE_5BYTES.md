# 📋 **GUIA DETALHADO: OnWSBinaryMessage - Processar 5 bytes ANTES de ProcessBinaryBuffer**

## 🎯 **OBJETIVO:**

Processar mensagens `PlayerDisconnected` (5 bytes) **ANTES** de chamar `ProcessBinaryBuffer`, pois `ProcessBinaryBuffer` espera frames de 25 ou 34 bytes.

---

## 📊 **ESTRUTURA CORRETA:**

### **FLUXO PRINCIPAL:**

```
OnWSBinaryMessage (Data)
  ↓
[PASSO 1] Verificar se Data tem 5 bytes
  ↓
[PASSO 2] Se for 5 bytes, verificar se tipo é 3
  ↓
[PASSO 3] Se for tipo 3, processar PlayerDisconnected (NÃO chamar ProcessBinaryBuffer)
  ↓
[PASSO 4] Se NÃO for 5 bytes, chamar ProcessBinaryBuffer normalmente
```

---

## 🔧 **IMPLEMENTAÇÃO DETALHADA:**

### **PASSO 1: Verificar Tamanho da Mensagem**

**IMEDIATAMENTE após `OnWSBinaryMessage` receber `Data`:**

```
OnWSBinaryMessage (Data)
  ↓
Get Array Length (Data)
  → DataLength (Integer)
  ↓
Equal (Integer)
  - A: DataLength
  - B: 5
  → Is5Bytes (Boolean)
  ↓
Branch: Is5Bytes?
  ├─ then: [PASSO 2] Verificar tipo
  └─ else: [PASSO 4] ProcessBinaryBuffer (lógica normal)
```

**⚠️ IMPORTANTE:** Esta verificação deve ser a **PRIMEIRA** coisa após receber `Data`, **ANTES** de qualquer outra lógica (incluindo logs de debug).

---

### **PASSO 2: Verificar Tipo da Mensagem (se for 5 bytes)**

**No caminho `then` do Branch `Is5Bytes?`:**

```
Branch: Is5Bytes? (then)
  ↓
Get Array Item (Data, 0)
  → MessageType (Byte)
  ↓
Equal (Byte)
  - A: MessageType
  - B: 3 (PlayerDisconnected)
  → IsType3 (Boolean)
  ↓
Branch: IsType3?
  ├─ then: [PASSO 3] Processar PlayerDisconnected
  └─ else: Print String: "Mensagem de 5 bytes com tipo desconhecido: " + ToString(MessageType)
          ↓
          (NÃO chamar ProcessBinaryBuffer - mensagem desconhecida)
```

---

### **PASSO 3: Processar PlayerDisconnected**

**No caminho `then` do Branch `IsType3?`:**

```
Branch: IsType3? (then)
  ↓
ParsePlayerDisconnected (Data, OutPlayerId)
  → ReturnValue (Boolean), OutPlayerId (Integer)
  ↓
Branch: ReturnValue?
  ├─ then: RemoveRemoteActor (OutPlayerId)
          ↓
          Print String: "✅ [OnWSBinaryMessage] PlayerDisconnected processado: " + ToString(OutPlayerId)
  └─ else: Print String: "❌ [OnWSBinaryMessage] Erro ao parsear PlayerDisconnected"
          ↓
          (NÃO chamar ProcessBinaryBuffer - parse falhou)
```

**⚠️ IMPORTANTE:** Após processar `PlayerDisconnected`, **NÃO** chamar `ProcessBinaryBuffer`. A mensagem já foi processada completamente.

---

### **PASSO 4: Processar Frames Normais (se NÃO for 5 bytes)**

**No caminho `else` do Branch `Is5Bytes?`:**

```
Branch: Is5Bytes? (else)
  ↓
ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID)
  → ReturnValue (Boolean)
  ↓
Branch: ReturnValue?
  ├─ then: ProcessNextFrame (lógica normal)
  └─ else: (não há frame completo - aguardar mais dados)
```

---

## 📋 **ESTRUTURA COMPLETA (DIAGRAMA):**

```
OnWSBinaryMessage (Data)
  ↓
Set IsFirstCall = true (se necessário para logs)
  ↓
Get Array Length (Data) → DataLength
  ↓
Equal (Integer): DataLength == 5?
  → Is5Bytes
  ↓
Branch: Is5Bytes?
  ├─ then: Get Array Item (Data, 0) → MessageType
          ↓
          Equal (Byte): MessageType == 3?
          → IsType3
          ↓
          Branch: IsType3?
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId)
                      ↓
                      Branch: ReturnValue?
                          ├─ then: RemoveRemoteActor (OutPlayerId)
                                  ↓
                                  Print String: "✅ PlayerDisconnected processado"
                          └─ else: Print String: "❌ Erro ao parsear PlayerDisconnected"
              └─ else: Print String: "⚠️ Mensagem de 5 bytes com tipo desconhecido"
                      ↓
                      (FIM - não processar mais)
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID)
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame (lógica normal)
              └─ else: (não há frame completo)
```

---

## ❌ **PROBLEMAS NO CÓDIGO ATUAL:**

### **PROBLEMA 1: Verificação de 5 bytes está DEPOIS de ProcessBinaryBuffer**

**Situação atual:**
- `ProcessBinaryBuffer` é chamado primeiro
- Depois, verifica se `OutFrame.Data[0] == 3`
- Isso está **ERRADO** porque `ProcessBinaryBuffer` espera 25 ou 34 bytes

**Correção:**
- Mover a verificação de `Data.Num() == 5` para **ANTES** de `ProcessBinaryBuffer`
- Se for 5 bytes, processar `PlayerDisconnected` e **NÃO** chamar `ProcessBinaryBuffer`

---

### **PROBLEMA 2: Verificação de tipo está usando OutFrame.Data**

**Situação atual:**
- Verifica `OutFrame.Data[0] == 3` (depois de `ProcessBinaryBuffer`)
- Mas `OutFrame` só é válido se `ProcessBinaryBuffer` retornar `true`
- E `ProcessBinaryBuffer` não processa mensagens de 5 bytes corretamente

**Correção:**
- Verificar `Data[0] == 3` **DIRETAMENTE** do `Data` recebido
- **ANTES** de chamar `ProcessBinaryBuffer`

---

### **PROBLEMA 3: Lógica de verificação de tipo está no lugar errado**

**Situação atual:**
- A verificação `Data[0] == 3` está no caminho `then` de `ProcessBinaryBuffer` retornar `true`
- Isso significa que só verifica se `ProcessBinaryBuffer` processou algo
- Mas mensagens de 5 bytes não devem passar por `ProcessBinaryBuffer`

**Correção:**
- Mover toda a lógica de verificação de 5 bytes e tipo 3 para **ANTES** de `ProcessBinaryBuffer`
- Criar um Branch separado que decide: "5 bytes e tipo 3?" → `PlayerDisconnected` OU "outro tamanho" → `ProcessBinaryBuffer`

---

## ✅ **ESTRUTURA CORRIGIDA (ORDEM CORRETA):**

```
OnWSBinaryMessage (Data)
  ↓
[OPCIONAL] Set IsFirstCall = true (para logs)
  ↓
[OPCIONAL] Print String: "Received binary message, size: " + ToString(DataLength) (se IsFirstCall)
  ↓
Get Array Length (Data) → DataLength
  ↓
Equal (Integer): DataLength == 5?
  → Is5Bytes
  ↓
Branch: Is5Bytes?
  ├─ then: Get Array Item (Data, 0) → MessageType
          ↓
          Equal (Byte): MessageType == 3?
          → IsType3
          ↓
          Branch: IsType3?
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId)
                      ↓
                      Branch: ReturnValue?
                          ├─ then: RemoveRemoteActor (OutPlayerId)
                          └─ else: Print String: "Erro ao parsear PlayerDisconnected"
              └─ else: Print String: "Mensagem de 5 bytes com tipo desconhecido"
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID)
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame
              └─ else: (não há frame completo)
```

---

## 🔍 **ANÁLISE DO SEU CÓDIGO ATUAL:**

### **O QUE ESTÁ ERRADO:**

1. ❌ **`ProcessBinaryBuffer` é chamado ANTES da verificação de 5 bytes**
   - `K2Node_CallFunction_43` (ProcessBinaryBuffer) está sendo chamado no caminho `else` de `IsFirstCall`
   - Mas a verificação de 5 bytes (`K2Node_IfThenElse_14`) está DEPOIS do `Print String`
   - Isso significa que `ProcessBinaryBuffer` pode ser chamado mesmo para mensagens de 5 bytes

2. ❌ **Verificação de tipo está usando `OutFrame.Data`**
   - `K2Node_IfThenElse_12` verifica `OutFrame.Data[0] == 3`
   - Mas isso só funciona se `ProcessBinaryBuffer` retornar `true`
   - Mensagens de 5 bytes não devem passar por `ProcessBinaryBuffer`

3. ❌ **Ordem incorreta das verificações**
   - A verificação de 5 bytes (`K2Node_IfThenElse_14`) está DEPOIS do `Print String`
   - Deveria estar ANTES de qualquer processamento

---

## ✅ **CORREÇÃO NECESSÁRIA:**

### **REORGANIZAR A ORDEM:**

1. **Mover `Get Array Length (Data)` para logo após `OnWSBinaryMessage`**
2. **Mover `Equal (Integer): DataLength == 5?` para logo após `Get Array Length`**
3. **Mover `Branch: Is5Bytes?` para logo após `Equal`**
4. **No caminho `then` de `Is5Bytes?`:**
   - Adicionar `Get Array Item (Data, 0)`
   - Adicionar `Equal (Byte): MessageType == 3?`
   - Adicionar `Branch: IsType3?`
   - No `then` de `IsType3?`: chamar `ParsePlayerDisconnected` e `RemoveRemoteActor`
5. **No caminho `else` de `Is5Bytes?`:**
   - Chamar `ProcessBinaryBuffer` (lógica normal)

---

## 📋 **ESTRUTURA FINAL CORRIGIDA (ORDEM EXATA):**

```
OnWSBinaryMessage (Data)
  ↓
[OPCIONAL] Set IsFirstCall = true
  ↓
[OPCIONAL] Branch: IsFirstCall?
  ├─ then: Print String: "Received binary message, size: " + ToString(DataLength)
  └─ else: (pular log)
  ↓
Get Array Length (Data) → DataLength
  ↓
Equal (Integer): DataLength == 5?
  → Is5Bytes
  ↓
Branch: Is5Bytes?
  ├─ then: Get Array Item (Data, 0) → MessageType
          ↓
          Equal (Byte): MessageType == 3?
          → IsType3
          ↓
          Branch: IsType3?
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId)
                      ↓
                      Branch: ReturnValue?
                          ├─ then: RemoveRemoteActor (OutPlayerId)
                          └─ else: Print String: "Erro ao parsear"
              └─ else: Print String: "Tipo desconhecido"
  └─ else: ProcessBinaryBuffer (Buffer, Data, OutFrame, ExpectedPlayerID)
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame
              └─ else: (não há frame completo)
```

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **A verificação de 5 bytes DEVE ser a PRIMEIRA coisa** (após logs opcionais)
2. **Se for 5 bytes e tipo 3, NÃO chamar `ProcessBinaryBuffer`**
3. **Se NÃO for 5 bytes, chamar `ProcessBinaryBuffer` normalmente**
4. **Usar `Data` diretamente, não `OutFrame.Data`** para verificar o tipo

