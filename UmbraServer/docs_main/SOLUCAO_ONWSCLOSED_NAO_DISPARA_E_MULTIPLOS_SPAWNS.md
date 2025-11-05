# 🔍 **ANÁLISE: OnWSClosed Não Disparando e Múltiplos Spawns**

## 📋 **PROBLEMAS IDENTIFICADOS:**

### **PROBLEMA #1: OnWSClosed Não Dispara ao Fechar PIE com ESC**

**Causa:**
- O delegate `OnClosed` do WebSocket só dispara quando o WebSocket fecha **naturalmente** (lado do servidor ou cliente)
- Quando você fecha o PIE com ESC, o Unreal Engine pode destruir o objeto **antes** de fechar o WebSocket corretamente
- O WebSocket não está sendo fechado explicitamente antes de fechar o PIE

**Solução:**
- Adicionar um evento `EndPlay` ou `BeginDestroy` para fechar o WebSocket explicitamente antes de destruir o objeto
- Chamar `OnWSClosed` manualmente quando `EndPlay` for disparado

---

### **PROBLEMA #2: Format Text com Erro de Compilação**

**Evidência:**
- `K2Node_FormatText_6` tem erro: "Referência de Objeto não é compatível com Matriz de Actor Referência de Objetos"
- O pin `{0}` está conectado ao `ReturnValue` de `Get Array Length` (correto), mas o erro persiste

**Causa:**
- O `Format Text` ainda está tentando usar o array diretamente em vez do length
- Mas pelo XML, parece estar conectado corretamente ao `Get Array Length`

**Solução:**
- Verificar se o `Format Text` está realmente usando `Get Array Length` e não o array diretamente
- Se o erro persistir, remover o `Format Text` completamente (não é crítico)

---

### **PROBLEMA #3: Múltiplos Spawns Novamente**

**Possíveis Causas:**
1. `ProcessNextFrame` não está verificando se o actor já existe antes de spawnar
2. `Array_Find` não está funcionando corretamente
3. `RemoteActorIds` não está sendo atualizado corretamente quando spawna
4. Cleanup não está funcionando corretamente, deixando IDs órfãos

**Verificação Necessária:**
- Verificar se `ProcessNextFrame` está usando `Array_Find` corretamente
- Verificar se `RemoteActorIds` está sendo atualizado quando spawna um novo actor
- Verificar se o `then` pin de `K2Node_IfThenElse_6` está conectado corretamente (para atualizar actors existentes)

---

## 🔧 **SOLUÇÕES:**

### **SOLUÇÃO 1: Fechar WebSocket ao Fechar PIE**

**AÇÃO:**
1. Adicionar evento `EndPlay` no `BP_NetMovementClient`
2. No `EndPlay`, verificar se `WebSocketRef` é válido
3. Se válido, fechar o WebSocket explicitamente
4. Chamar `OnWSClosed` manualmente (ou deixar o WebSocket chamar naturalmente)

**ESTRUTURA:**
```
Event EndPlay (Reason: Level Transition)
  ↓
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Call Function: Close WebSocket (WebSocketRef) ← Se existir função de fechar
  │   ├─ OU: Call Function: OnWSClosed (Manual) ← Chamar diretamente
  │   └─ Print String: "Closing WebSocket on EndPlay..."
  └─ False: (Não fazer nada)
```

**OU usar `BeginDestroy`:**
```
Event BeginDestroy
  ↓
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Call Function: CleanupRemoteActors ← Limpar antes de fechar
  │   └─ Print String: "Destroying WebSocket connection..."
  └─ False: (Não fazer nada)
```

---

### **SOLUÇÃO 2: Verificar ProcessNextFrame**

**Verificações Necessárias:**
1. `Array_Find` está sendo usado para verificar se `OutPlayerId` já existe em `RemoteActorIds`?
2. Se `Array_Find` retorna `>= 0` (encontrado), deve usar `Get Array Item` e atualizar actor existente
3. Se `Array_Find` retorna `-1` (não encontrado), deve spawnar novo actor e adicionar ao array
4. `RemoteActorIds` está sendo atualizado quando spawna um novo actor?

**ESTRUTURA CORRETA:**
```
ProcessNextFrame
  ↓
ParseStateUpdateFrame
  ↓ (ReturnValue == true)
[... filtros ...]
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → Index
  ↓
Branch: Index >= 0?
  ├─ True (Actor existe):
  │   ├─ Get Array Item (RemoteActors, Index) → ExistingActor
  │   ├─ Set Variable: RemoteActorRef = ExistingActor
  │   ├─ Set Actor Location (RemoteActorRef, OutLocation)
  │   └─ Set Actor Rotation (RemoteActorRef, OutRotation)
  │
  └─ False (Actor não existe):
      ├─ SpawnActorFromClass (RemotePlayerClass, OutLocation, OutRotation)
      ├─ Set Variable: RemoteActorRef = SpawnedActor
      ├─ Array_Add (RemoteActors, RemoteActorRef)
      └─ Array_Add (RemoteActorIds, OutPlayerId) ← CRÍTICO!
```

---

### **SOLUÇÃO 3: Corrigir Format Text (Se Necessário)**

**AÇÃO:**
1. Verificar se `Get Array Length` está conectado ao pin `{0}` do `Format Text`
2. Se sim, mas o erro persiste, **remover completamente** o `Format Text` e usar apenas um log simples
3. O log não é crítico para o funcionamento

---

## 📝 **PROCEDIMENTO COMPLETO:**

### **PASSO 1: Adicionar EndPlay para Fechar WebSocket**

1. Abra o `Event Graph` do `BP_NetMovementClient`
2. Adicione um novo evento: **`Event EndPlay`**
   - Parâmetro `Reason`: `Level Transition` (ou qualquer um)
3. Conecte a lógica:
   ```
   Event EndPlay
     ↓
   Branch: Is Valid (WebSocketRef)?
     ├─ True:
     │   ├─ Print String: "EndPlay - Cleaning up WebSocket..."
     │   ├─ CleanupRemoteActors ← Limpar actors primeiro
     │   └─ [OPCIONAL] Call Function: Close WebSocket (se existir)
     └─ False: (Não fazer nada)
   ```

**OU:**

Se `UmbraWSClient` tem uma função `Close` ou `Disconnect`:
```
Event EndPlay
  ↓
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Call Function: Close (WebSocketRef) ← Fechar WebSocket explicitamente
  │   └─ CleanupRemoteActors ← Limpar actors
  └─ False: (Não fazer nada)
```

---

### **PASSO 2: Verificar ProcessNextFrame**

**Verificações:**
1. Abra a função `ProcessNextFrame`
2. Verifique se `Array_Find` está sendo usado:
   - Entrada: `RemoteActorIds` (Array of Integers)
   - Entrada: `OutPlayerId` (Integer)
   - Saída: `Index` (Integer)
3. Verifique se há um `Branch` após `Array_Find`:
   - Condição: `Index >= 0` (ou `Greater or Equal`)
   - `True`: Actor existe → atualizar
   - `False`: Actor não existe → spawnar
4. Verifique se `Array_Add` está sendo usado para adicionar `OutPlayerId` ao `RemoteActorIds` quando spawna um novo actor

**Se não estiver correto:**
- Siga o guia `GUIA_PRATICO_CORRECAO_MULTIPLOS_SPAWNS_E_MOVIMENTO.md`

---

### **PASSO 3: Remover Format Text com Erro (Opcional)**

**AÇÃO:**
1. Remova o `Format Text_6` e `Conv_TextToString` do `OnWSClosed`
2. Use apenas um `Print String` simples: "Starting cleanup - cleaning up RemoteActors..."

---

## ✅ **CHECKLIST:**

### **EndPlay:**
- [ ] Evento `EndPlay` adicionado
- [ ] `Is Valid (WebSocketRef)` verificado
- [ ] `CleanupRemoteActors` chamado antes de fechar
- [ ] WebSocket fechado explicitamente (se possível)

### **ProcessNextFrame:**
- [ ] `Array_Find` está presente e conectado corretamente
- [ ] `Branch` após `Array_Find` verifica `Index >= 0`
- [ ] `Get Array Item` usado para actors existentes
- [ ] `Array_Add` usado para `RemoteActorIds` quando spawna novo actor

### **OnWSClosed:**
- [ ] Format Text removido (se erro persistir)
- [ ] Log simples adicionado

---

## 🎯 **TESTE:**

1. **Conectar um client**
2. **Mover o personagem** (verificar que outros clients veem)
3. **Fechar PIE com ESC**
4. **Verificar logs:**
   ```
   [Cliente] EndPlay - Cleaning up WebSocket...
   [Cliente] Cleaning up RemoteActors...
   [Cliente] Actor Destroyed (para cada actor)
   [Cliente] Cleanup complete!
   [Servidor] WS client X disconnected
   [Servidor] Removing player Y (client X) from players map
   ```
5. **Reconectar com o mesmo ID**
6. **Verificar que não há múltiplos spawns**

---

**Próximos Passos:**
1. Adicionar `EndPlay` para fechar WebSocket
2. Verificar `ProcessNextFrame` para garantir que não há múltiplos spawns
3. Remover `Format Text` com erro (se necessário)

