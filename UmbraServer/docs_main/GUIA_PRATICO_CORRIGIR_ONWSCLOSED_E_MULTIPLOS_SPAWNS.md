# 🔧 **GUIA PRÁTICO: Corrigir OnWSClosed Não Disparando e Múltiplos Spawns**

## 📋 **PROBLEMAS:**

1. ❌ `OnWSClosed` não dispara ao fechar PIE com ESC
2. ❌ Múltiplos RemoteActors sendo spawnados novamente
3. ❌ Format Text com erro de compilação (não crítico)

---

## 🎯 **SOLUÇÃO 1: Fechar WebSocket ao Fechar PIE**

### **PASSO 1: Adicionar Event EndPlay**

1. Abra o `Event Graph` do `BP_NetMovementClient`
2. Clique com botão direito → **`Event EndPlay`**
3. Deixe o parâmetro `Reason` como padrão (ou selecione `Level Transition`)

### **PASSO 2: Conectar Lógica de Fechamento**

**ESTRUTURA:**
```
Event EndPlay
  ↓
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Print String: "EndPlay - Closing WebSocket..."
  │   ├─ Call Function: Close (WebSocketRef) ← Chamar função Close do UmbraWSClient
  │   └─ CleanupRemoteActors ← Limpar actors
  └─ False: (Não fazer nada)
```

**AÇÃO:**
1. Após `Event EndPlay`, adicione:
   - **`Is Valid`** → Conecte ao `WebSocketRef`
   - **`Branch`** → Conecte o resultado do `Is Valid` à condição
2. No caminho `True` do `Branch`:
   - **`Print String`**: "EndPlay - Closing WebSocket..."
   - **`Call Function`**: Procure por `Close` na classe `UmbraWSClient`
     - Se não encontrar, procure por "Close" ou "Disconnect" no `WebSocketRef`
   - **`CleanupRemoteActors`** → Chame a função de cleanup

**NOTA:** Se `Close` não estiver disponível no Blueprint, você pode chamar `OnWSClosed` manualmente:
```
Event EndPlay
  ↓
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Print String: "EndPlay - Cleaning up..."
  │   ├─ CleanupRemoteActors ← Limpar actors primeiro
  │   └─ Call Function: OnWSClosed ← Chamar manualmente
  └─ False: (Não fazer nada)
```

---

## 🎯 **SOLUÇÃO 2: Verificar Múltiplos Spawns**

### **PASSO 1: Verificar ProcessNextFrame**

**Verificações Necessárias:**

1. **`Array_Find` está presente?**
   - Deve buscar `OutPlayerId` em `RemoteActorIds`
   - Deve retornar `FoundIndex` (Integer)

2. **`Branch` após `Array_Find` verifica `FoundIndex >= 0`?**
   - Deve ter um `Greater or Equal` conectando `FoundIndex` e `0`
   - O `Branch` deve verificar se `FoundIndex >= 0`

3. **O pin `then` (True) do `Branch` está conectado?**
   - Se não estiver conectado, actors existentes nunca são atualizados
   - Deve conectar a `Get Array Item` → `Set Variable` → `Set Actor Location`

4. **`Array_Add` está sendo usado quando spawna novo actor?**
   - Deve adicionar `OutPlayerId` ao `RemoteActorIds`
   - Deve adicionar o actor spawnado ao `RemoteActors`

### **PASSO 2: Estrutura Correta Esperada**

```
ProcessNextFrame
  ↓
[... filtros ...]
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal: FoundIndex >= 0?
  ↓
Branch: FoundIndex >= 0?
  ├─ True (actor existe):
  │   ├─ Get Array Item (RemoteActors, FoundIndex) → ExistingActor
  │   ├─ Set Variable: RemoteActorRef = ExistingActor
  │   └─ [CONTINUA PARA ATUALIZAÇÃO]
  │
  └─ False (actor não existe):
      ├─ SpawnActorFromClass
      ├─ Set Variable: RemoteActorRef = SpawnedActor
      ├─ Array_Add (RemoteActorIds, OutPlayerId) ← CRÍTICO!
      ├─ Array_Add (RemoteActors, SpawnedActor) ← CRÍTICO!
      └─ [CONTINUA PARA ATUALIZAÇÃO]
  ↓
[PONTO DE CONVERGÊNCIA]
  ↓
Set Actor Location (RemoteActorRef, OutLocation)
  ↓
Set Actor Rotation (RemoteActorRef, OutRotation)
```

### **PASSO 3: Se Múltiplos Spawns Persistirem**

**Causas Possíveis:**
1. `Array_Add` não está sendo executado antes do próximo frame chegar
2. `Array_Find` está retornando `-1` mesmo quando o actor existe (race condition)
3. `RemoteActorIds` não está sendo atualizado corretamente

**Solução:**
- Adicionar logs detalhados:
  ```
  Array_Find → FoundIndex
    ↓
  Print String: "FoundIndex: {FoundIndex}, OutPlayerId: {OutPlayerId}"
    ↓
  Branch: FoundIndex >= 0?
  ```

---

## 🎯 **SOLUÇÃO 3: Corrigir Format Text (Opcional)**

### **PASSO 1: Remover Format Text com Erro**

1. No `OnWSClosed`, localize o `Format Text_6` com erro
2. **Remova** completamente:
   - `Format Text_6`
   - `Conv_TextToString` (K2Node_CallFunction_7)
   - Conexões relacionadas
3. **Substitua** por um log simples:
   - `Print String`: "Starting cleanup..."

### **OU: Corrigir Format Text**

1. Verifique se `Get Array Length` está conectado ao pin `{0}` do `Format Text`
2. Se sim, mas o erro persiste, **remova completamente** o `Format Text`
3. O log não é crítico para o funcionamento

---

## ✅ **CHECKLIST COMPLETO:**

### **EndPlay:**
- [ ] Evento `EndPlay` adicionado
- [ ] `Is Valid (WebSocketRef)` verificado
- [ ] `Close (WebSocketRef)` chamado (ou `OnWSClosed` chamado manualmente)
- [ ] `CleanupRemoteActors` chamado

### **ProcessNextFrame:**
- [ ] `Array_Find` presente e conectado corretamente
- [ ] `Greater or Equal` presente e conectado (`FoundIndex >= 0`)
- [ ] `Branch` após `Array_Find` verifica `FoundIndex >= 0`
- [ ] Pin `then` (True) do `Branch` está conectado
- [ ] `Get Array Item` presente para actors existentes
- [ ] `Array_Add` presente para `RemoteActorIds` quando spawna novo actor
- [ ] `Array_Add` presente para `RemoteActors` quando spawna novo actor

### **OnWSClosed:**
- [ ] Format Text removido (se erro persistir)
- [ ] Log simples adicionado

---

## 🧪 **TESTE FINAL:**

1. **Conectar um client**
2. **Mover o personagem** (verificar que outros clients veem)
3. **Fechar PIE com ESC**
4. **Verificar logs:**
   ```
   [Cliente] EndPlay - Closing WebSocket...
   [Cliente] Cleaning up RemoteActors...
   [Cliente] Actor Destroyed (para cada actor)
   [Cliente] Cleanup complete!
   [Servidor] WS client X disconnected
   [Servidor] Removing player Y (client X) from players map
   ```
5. **Reconectar com o mesmo ID**
6. **Verificar que não há múltiplos spawns**
7. **Verificar que apenas 1 RemoteActor é spawnado por PlayerID**

---

## 📝 **NOTAS IMPORTANTES:**

- O `OnClosed` delegate do WebSocket só dispara quando o WebSocket fecha **naturalmente**
- Quando você fecha o PIE com ESC, o Unreal Engine pode destruir o objeto antes de fechar o WebSocket
- Por isso, é necessário fechar o WebSocket explicitamente em `EndPlay`
- Se `ProcessNextFrame` ainda causar múltiplos spawns após as correções, adicione logs para identificar onde está o problema

---

**Próximos Passos:**
1. Adicionar `EndPlay` para fechar WebSocket
2. Verificar `ProcessNextFrame` para garantir que não há múltiplos spawns
3. Remover `Format Text` com erro (se necessário)

