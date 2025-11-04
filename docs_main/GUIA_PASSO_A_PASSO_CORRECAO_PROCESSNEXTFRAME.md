# 🔧 **GUIA PASSO A PASSO: CORREÇÃO COMPLETA DO `ProcessNextFrame`**

## 📋 **OBJETIVO:**

Corrigir o fluxo de `ProcessNextFrame` para:
1. ✅ Evitar múltiplos spawns do mesmo `OutPlayerId`
2. ✅ Atualizar corretamente atores existentes
3. ✅ Garantir que clientes vejam uns aos outros
4. ✅ Garantir que movimento funcione corretamente

---

## 🎯 **ETAPA 1: CORRIGIR ATUALIZAÇÃO DE ACTOR EXISTENTE**

### **Problema Identificado:**
O pin `then` (True) do `Branch [K2Node_IfThenElse_6]` está **desconectado**. Quando um actor já existe (`FoundIndex >= 0`), nada acontece, causando:
- Actor não é atualizado
- Múltiplos frames podem chegar antes que `Array_Add` atualize os arrays
- Race condition: `Array_Find` retorna `-1` mesmo que o actor exista

### **Solução Passo a Passo:**

#### **PASSO 1.1: Conectar o Pin `then` do Branch**

1. **Localize o nó `K2Node_IfThenElse_6`** (Branch que verifica `FoundIndex >= 0`)
2. **Clique no pin `then` (True)** - ele deve estar desconectado
3. **Arraste para criar uma nova conexão**

---

#### **PASSO 1.2: Adicionar `Get Array Item`**

1. **Clique com botão direito** na área vazia após o pin `then` (True)
2. **Procure por:** `Get Array Item` (ou `Get Array Item`)
3. **Selecione:** `Get Array Item`
4. **Configuração do nó:**
   - **Array**: Conecte ao pin de saída de `Get Variable (RemoteActors)`
     - Se não existir `Get Variable (RemoteActors)`, adicione:
       - Botão direito → `Get Variable` → Selecione `RemoteActors`
   - **Index**: Conecte ao pin de saída `FoundIndex` do `Array_Find`
     - Procure pelo nó `K2Node_CallArrayFunction_4` (Array_Find)
     - Conecte `ReturnValue` (FoundIndex) ao pin `Index` do `Get Array Item`
5. **O `Get Array Item` retorna**: O actor do tipo `BP_RemotePlayer` que já existe no array

---

#### **PASSO 1.3: Adicionar `Set Variable (RemoteActorRef)`**

1. **Clique com botão direito** após o `Get Array Item`
2. **Procure por:** `Set Variable` (ou `Set`)
3. **Selecione:** `Set Variable` → Escolha a variável `RemoteActorRef`
4. **Configuração do nó:**
   - **Conecte o pin de execução:**
     - Do pin `then` (True) do `Branch [K2Node_IfThenElse_6]` → ao pin de execução do `Set Variable`
   - **Conecte o valor:**
     - Do pin `Output` do `Get Array Item` → ao pin de entrada (valor) do `Set Variable (RemoteActorRef)`
5. **O resultado:** A variável `RemoteActorRef` agora contém o actor existente

---

#### **PASSO 1.4: Conectar ao Fluxo de Atualização**

1. **Do pin `then` (execução) do `Set Variable (RemoteActorRef)`**, conecte ao mesmo fluxo que atualiza posição/rotação
2. **Localize o nó `Is Valid?`** que verifica `RemoteActorRef`
   - Se não existir, adicione:
     - Botão direito → `Is Valid` → Selecione `RemoteActorRef`
3. **Conecte o pin de execução do `Set Variable (RemoteActorRef)` ao pin de execução do `Is Valid?`**
4. **Fluxo resultante:**
```
Branch [K2Node_IfThenElse_6]: FoundIndex >= 0?
  ├─ True (actor existe):
  │   ↓ (then)
  │   Get Array Item(RemoteActors, FoundIndex)
  │   ↓ (Output)
  │   Set Variable (RemoteActorRef) ← Output do Get Array Item
  │   ↓ (then)
  │   Is Valid? (RemoteActorRef)
  │   ↓ (then = True)
  │   Set Actor Location + Set Actor Rotation
  │
  └─ False (actor não existe):
      ↓ (else)
      SpawnActorFromClass
```

---

## 🎯 **ETAPA 2: GARANTIR `RemoteActorRef` SETADO APÓS SPAWN**

### **Problema Identificado:**
Quando um novo actor é spawnado, `RemoteActorRef` pode não estar sendo setado corretamente, impedindo a atualização posterior.

### **Solução Passo a Passo:**

#### **PASSO 2.1: Localizar o Fluxo de Spawn**

1. **Localize o nó `K2Node_SpawnActorFromClass_0`** (SpawnActorFromClass)
2. **Verifique o pin `then` (execução)**: Deve estar conectado a `Array_Add(RemoteActorIds, OutPlayerId)`

---

#### **PASSO 2.2: Adicionar `Set Variable (RemoteActorRef)` Após `Array_Add(RemoteActors, ...)`**

1. **Localize o segundo `Array_Add`** que adiciona o actor spawnado ao array `RemoteActors`
   - Este `Array_Add` deve ter como `New Item` o `ReturnValue` do `SpawnActorFromClass`
2. **Após o pin `then` (execução) do segundo `Array_Add`**, adicione um `Set Variable (RemoteActorRef)`
3. **Configuração:**
   - **Pin de execução:** Conecte do `then` do segundo `Array_Add` ao `exec` do `Set Variable`
   - **Valor:** Conecte do `ReturnValue` do `SpawnActorFromClass` ao pin de entrada do `Set Variable`
     - **IMPORTANTE:** Use o mesmo `ReturnValue` que foi usado no `Array_Add(RemoteActors, ...)`
4. **Fluxo resultante:**
```
SpawnActorFromClass
  ↓ (then, ReturnValue)
  Array_Add(RemoteActorIds, OutPlayerId)
  ↓ (then)
  Array_Add(RemoteActors, ReturnValue do SpawnActor)
  ↓ (then)
  Set Variable (RemoteActorRef) ← ReturnValue do SpawnActorFromClass
  ↓ (then)
  Is Valid? (RemoteActorRef)
  ↓ (then = True)
  Set Actor Location + Set Actor Rotation
```

---

## 🎯 **ETAPA 3: CONVERGIR AMBOS OS CAMINHOS PARA O MESMO FLUXO**

### **Problema Identificado:**
Os caminhos de "spawn" e "atualização" devem convergir para o mesmo fluxo de atualização de posição/rotação.

### **Solução Passo a Passo:**

#### **PASSO 3.1: Verificar o Fluxo de Atualização Existente**

1. **Localize o nó `Is Valid?`** que verifica `RemoteActorRef`
2. **Verifique se está conectado a:**
   - `Set Actor Location (RemoteActorRef, OutLocation)`
   - `Make Rotator (Yaw = OutYawDegrees)`
   - `Set Actor Rotation (RemoteActorRef, NewRotation)`

---

#### **PASSO 3.2: Garantir que Ambos os Caminhos Conectam ao `Is Valid?`**

1. **Caminho de Spawn:**
   - `Set Variable (RemoteActorRef)` (após `Array_Add(RemoteActors, ...)`)
   - ↓ (then)
   - `Is Valid? (RemoteActorRef)`

2. **Caminho de Atualização:**
   - `Set Variable (RemoteActorRef)` (após `Get Array Item`)
   - ↓ (then)
   - `Is Valid? (RemoteActorRef)` ← **MESMO NÓ**

3. **Verifique:** Ambos os `Set Variable (RemoteActorRef)` devem estar conectados ao **MESMO** nó `Is Valid?`

---

## 🎯 **ETAPA 4: VERIFICAR RECURSÃO CORRETA**

### **Problema Identificado:**
A recursão pode estar causando múltiplos processamentos do mesmo frame se `ProcessBinaryBuffer` retorna `true` incorretamente.

### **Solução Passo a Passo:**

#### **PASSO 4.1: Localizar o Segundo `ProcessBinaryBuffer`**

1. **Após `Set Actor Rotation`**, deve haver uma segunda chamada a `ProcessBinaryBuffer`
2. **Verifique os parâmetros:**
   - **Buffer**: `Get Variable (Buffer)` (mesmo buffer usado anteriormente)
   - **NewData**: Deve ser um **array vazio**
     - Se não for, adicione:
       - Botão direito → `Make Array` → **Não adicione nenhum elemento**
       - Conecte o `Make Array` (vazio) ao pin `NewData`
   - **OutFrame**: `Get Variable (OutFrame)` ou crie um novo `FBinaryFrame`

---

#### **PASSO 4.2: Verificar o Branch Após o Segundo `ProcessBinaryBuffer`**

1. **Após o segundo `ProcessBinaryBuffer`**, deve haver um `Branch` que verifica `ReturnValue`
2. **Configuração:**
   - **Condition**: `ReturnValue` do segundo `ProcessBinaryBuffer`
   - **True (then)**: Conectado a `ProcessNextFrame` (recursão)
   - **False (else)**: **Desconectado** ou conectado a um `Print String` de debug

3. **Verifique:** O pin `then` (True) só deve chamar `ProcessNextFrame` se `ReturnValue == true`

---

## 🔬 **ETAPA 5: ADICIONAR LOGS DE DEBUG**

### **Objetivo:**
Rastrear o fluxo de execução para identificar problemas.

### **Logs Recomendados:**

#### **LOG 1: Após ParseStateUpdateFrame**
```
Após ParseStateUpdateFrame:
  Print String: "Parsed: PlayerID=" + ToString(OutPlayerId) + ", MyPlayerId=" + ToString(MyPlayerId)
  Conecte ao pin `then` do ParseStateUpdateFrame
```

#### **LOG 2: Após Array_Find**
```
Após Array_Find:
  Print String: "FoundIndex=" + ToString(FoundIndex) + " for PlayerID=" + ToString(OutPlayerId)
  Conecte ao pin de execução após Array_Find
```

#### **LOG 3: Após Branch (FoundIndex >= 0)**
```
Após Branch [K2Node_IfThenElse_6]:
  Print String: "Branch: Actor exists=" + ToString(FoundIndex >= 0)
  Conecte aos pins `then` e `else` do Branch
```

#### **LOG 4: Após SpawnActorFromClass**
```
Após SpawnActorFromClass:
  Print String: "Spawned actor for PlayerID=" + ToString(OutPlayerId) + ", Valid=" + ToString(IsValid(ReturnValue))
  Conecte ao pin `then` do SpawnActorFromClass
```

#### **LOG 5: Após Get Array Item (actor existente)**
```
Após Get Array Item:
  Print String: "Got existing actor from index=" + ToString(FoundIndex) + ", Valid=" + ToString(IsValid(Output))
  Conecte ao pin de execução após Get Array Item
```

#### **LOG 6: Após Set Variable (RemoteActorRef)**
```
Após Set Variable (RemoteActorRef):
  Print String: "Set RemoteActorRef, Valid=" + ToString(IsValid(RemoteActorRef))
  Conecte ao pin `then` do Set Variable
```

#### **LOG 7: Após Set Actor Location**
```
Após Set Actor Location:
  Print String: "Updated location for PlayerID=" + ToString(OutPlayerId) + ", Location=" + ToString(OutLocation)
  Conecte ao pin `then` do Set Actor Location
```

---

## ✅ **CHECKLIST FINAL:**

- [ ] **Etapa 1.1**: Pin `then` (True) do `K2Node_IfThenElse_6` está conectado
- [ ] **Etapa 1.2**: `Get Array Item(RemoteActors, FoundIndex)` foi adicionado
- [ ] **Etapa 1.3**: `Set Variable (RemoteActorRef)` após `Get Array Item` foi adicionado
- [ ] **Etapa 1.4**: Caminho de atualização conecta ao `Is Valid?`
- [ ] **Etapa 2.1**: Fluxo de spawn identificado corretamente
- [ ] **Etapa 2.2**: `Set Variable (RemoteActorRef)` após `Array_Add(RemoteActors, ...)` foi adicionado
- [ ] **Etapa 3.1**: Fluxo de atualização existente verificado
- [ ] **Etapa 3.2**: Ambos os caminhos convergem para o mesmo `Is Valid?`
- [ ] **Etapa 4.1**: Segundo `ProcessBinaryBuffer` usa array vazio em `NewData`
- [ ] **Etapa 4.2**: `Branch` após segundo `ProcessBinaryBuffer` só chama recursão se `ReturnValue == true`
- [ ] **Etapa 5**: Logs de debug foram adicionados (opcional, mas recomendado)

---

## 🎯 **RESULTADO ESPERADO:**

Após implementar todas as etapas:

1. ✅ **Um único spawn por `OutPlayerId`** (sem duplicados)
2. ✅ **Actor existente é atualizado corretamente** (movimento funciona)
3. ✅ **Clientes veem uns aos outros** (frames de outros players são processados)
4. ✅ **Personagem local se move normalmente** (frames próprios são ignorados pelo filtro `OutPlayerId != MyPlayerId`)

---

## ⚠️ **NOTAS IMPORTANTES:**

- **Prioridade máxima**: Etapas 1 e 2 (conectar atualização de actor existente e garantir `RemoteActorRef` setado)
- **Prioridade média**: Etapa 3 (convergência dos caminhos)
- **Prioridade baixa**: Etapa 4 (verificação de recursão) e Etapa 5 (logs de debug)

Se após essas correções os problemas persistirem, verifique:
- Servidor está enviando frames duplicados?
- Buffer está processando o mesmo frame múltiplas vezes?
- `MyPlayerId` está sendo setado corretamente no `BeginPlay`?

