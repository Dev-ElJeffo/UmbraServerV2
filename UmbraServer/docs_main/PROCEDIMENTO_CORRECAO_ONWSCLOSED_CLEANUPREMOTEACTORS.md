# 🔧 **PROCEDIMENTO: Correção de OnWSClosed e CleanupRemoteActors**

## 📋 **PROBLEMAS IDENTIFICADOS:**

1. ✅ **Lógica duplicada**: `OnWSClosed` tem `ForEachLoop` e `Clear Array` que já existem em `CleanupRemoteActors`
2. ✅ **Format Text executando após limpar**: Log mostra sempre 0 porque executa depois de limpar
3. ✅ **RemoteActorRef não atualizado**: Usa variável antiga em vez de `Array Element` do loop
4. ✅ **RemoteActorIds não limpo**: `CleanupRemoteActors` não limpa `RemoteActorIds`

---

## 🎯 **PASSO A PASSO PARA CORREÇÃO:**

### **PASSO 1: Simplificar OnWSClosed**

**AÇÃO:**
1. Abra o `Event Graph` do `BP_NetMovementClient`
2. Localize o evento `OnWSClosed`
3. **REMOVER** os seguintes nós (manter apenas os logs e `CleanupRemoteActors`):
   - ❌ `Format Text_6` com `Get Array Length`
   - ❌ `ForEachLoop` (K2Node_MacroInstance_1)
   - ❌ `Branch` (K2Node_IfThenElse_0)
   - ❌ `Is Valid` (K2Node_CallFunction_22)
   - ❌ `Destroy Actor` (se houver)
   - ❌ `Clear Array` para `RemoteActors` (K2Node_CallArrayFunction_4)
   - ❌ `Clear Array` para `RemoteActorIds` (K2Node_CallArrayFunction_0)
   - ❌ Logs relacionados ao loop

**MANTER:**
- ✅ `Print String: "WebSocket closed - cleaning up..."`
- ✅ `CleanupRemoteActors` (K2Node_CallFunction_4)
- ✅ `Print String: "Cleanup complete - WebSocket closed"` (opcional, após `CleanupRemoteActors`)

**ESTRUTURA FINAL DO OnWSClosed:**
```
OnWSClosed (Custom Event)
  ↓
Print String: "WebSocket closed - cleaning up..."
  ↓
CleanupRemoteActors
  ↓
Print String: "Cleanup complete - WebSocket closed" (opcional)
```

---

### **PASSO 2: Corrigir CleanupRemoteActors**

**AÇÃO:**

#### **2.1: Adicionar Set Variable no LoopBody**

1. Abra a função `CleanupRemoteActors`
2. Localize o `ForEachLoop` (K2Node_MacroInstance_0)
3. No `LoopBody`, **ADICIONAR** `Set Variable` antes do `Is Valid`:
   ```
   ForEachLoop (LoopBody)
     ↓
   Set Variable: RemoteActorRef = Array Element ← ADICIONAR ESTE NÓ!
     ↓
   Is Valid (RemoteActorRef)
   ```

**OU** conectar `Array Element` diretamente:

1. **DESCONECTAR** `RemoteActorRef` do `Is Valid` (K2Node_CallFunction_1)
2. **CONECTAR** `Array Element` (do `ForEachLoop`) ao `Is Valid`

#### **2.2: Corrigir Destroy Actor**

1. **DESCONECTAR** `RemoteActorRef` do `Destroy Actor` (K2Node_CallFunction_2)
2. **CONECTAR** `Array Element` (do `ForEachLoop`) ao `Destroy Actor`
   **OU** usar `RemoteActorRef` se você adicionou `Set Variable` no passo 2.1

#### **2.3: Adicionar Clear Array para RemoteActorIds**

1. Após o `Clear Array` para `RemoteActors` (K2Node_CallArrayFunction_0)
2. **ADICIONAR** `Clear Array` para `RemoteActorIds`:
   ```
   Clear Array (RemoteActors)
     ↓
   Clear Array (RemoteActorIds) ← ADICIONAR ESTE NÓ!
     - TargetArray: RemoteActorIds (Array of Integers)
   ```

---

### **PASSO 3: Verificar Bind Event (Opcional)**

**VERIFICAÇÃO:**
1. No `Event Graph`, localize onde o `Bind Event to OnClosed` é chamado
2. Verifique se está conectado corretamente:
   ```
   Get WebSocketRef
     ↓
   Bind Event to OnClosed
     - self: WebSocketRef
     - Event pin → OnWSClosed
   ```

**STATUS:** ✅ Já está correto no XML fornecido!

---

## 📊 **ESTRUTURA CORRETA FINAL:**

### **OnWSClosed (Simplificado):**
```
OnWSClosed (Custom Event)
  ↓
Print String: "WebSocket closed - cleaning up..."
  ↓
CleanupRemoteActors
  ↓
Print String: "Cleanup complete - WebSocket closed" (opcional)
```

---

### **CleanupRemoteActors (Corrigido):**
```
CleanupRemoteActors (Custom Event)
  ↓
Print String: "Cleaning up RemoteActors..."
  ↓
ForEachLoop (RemoteActors)
  ├─ LoopBody:
  │   ├─ Set Variable: RemoteActorRef = Array Element ← ADICIONAR!
  │   │   ↓
  │   ├─ Is Valid (RemoteActorRef)
  │   │   ↓
  │   ├─ Branch (Is Valid)
  │   │   ├─ then (True): Destroy Actor (RemoteActorRef)
  │   │   │              ↓
  │   │   │              Print String: "Actor Destroyed"
  │   │   └─ else (False): Print String: "Skipping invalid actor" (opcional)
  │   └─ [Continue Loop]
  ↓
Clear Array (RemoteActors)
  ↓
Clear Array (RemoteActorIds) ← ADICIONAR!
  ↓
Print String: "Cleanup complete - All RemoteActors destroyed and arrays cleared"
```

---

## ✅ **CHECKLIST DE VERIFICAÇÃO:**

### **Antes de Testar:**
- [ ] `OnWSClosed` não tem `ForEachLoop` duplicado
- [ ] `OnWSClosed` não tem `Clear Array` duplicado
- [ ] `OnWSClosed` apenas chama `CleanupRemoteActors`
- [ ] `CleanupRemoteActors` usa `Array Element` (ou `Set Variable: RemoteActorRef = Array Element`)
- [ ] `CleanupRemoteActors` limpa `RemoteActorIds`

### **Após Testar:**
- [ ] Quando fecha o client, `OnWSClosed` é disparado
- [ ] `CleanupRemoteActors` destrói todos os RemoteActors corretamente
- [ ] Arrays são limpos corretamente
- [ ] Não há erros ou warnings
- [ ] Ao reconectar, não há múltiplos spawns

---

## 🎯 **TESTE FINAL:**

1. **Conectar um client**
2. **Mover o personagem** (verificar que outros clients veem)
3. **Fechar o client**
4. **Verificar logs:**
   ```
   [Cliente] WebSocket closed - cleaning up...
   [Cliente] Cleaning up RemoteActors...
   [Cliente] Actor Destroyed (para cada actor)
   [Cliente] Cleanup complete - All RemoteActors destroyed and arrays cleared
   [Servidor] WS client X disconnected
   [Servidor] Removing player Y (client X) from players map
   ```
5. **Reconectar com o mesmo ID**
6. **Verificar que não há múltiplos spawns**

---

**Fim do Procedimento**

