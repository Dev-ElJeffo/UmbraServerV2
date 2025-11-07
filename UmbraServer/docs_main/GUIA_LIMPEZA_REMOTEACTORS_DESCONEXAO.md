# 🔧 **IMPLEMENTAÇÃO: Limpeza de RemoteActors na Desconexão**

## 📋 **PROBLEMA:**
Quando um client fecha o WebSocket, o servidor não detecta a desconexão corretamente, causando:
- Player permanece "conectado" no servidor mesmo após fechar o client
- Ao reconectar com o mesmo ID, há múltiplos spawns (actor antigo + novo)
- O servidor continua broadcastando frames do player offline

---

## ✅ **SOLUÇÃO IMPLEMENTADA:**

### **PARTE 1: Servidor (C++) - JÁ IMPLEMENTADO**

**Arquivo:** `src/zone/MovementServer.hpp`

**Alterações:**
1. ✅ Adicionado mapeamento `clientIdToPlayerId_` para rastrear ClientID → PlayerID
2. ✅ No callback de desconexão, chama `handleClientDisconnect(cid)` para remover o player
3. ✅ Em `handleMoveUpdate`, atualiza o mapeamento ClientID → PlayerID
4. ✅ Função `handleClientDisconnect` remove o PlayerID do `players_` map quando desconecta

**Resultado:**
- Quando um client desconecta, o servidor remove automaticamente o PlayerID do `players_` map
- O servidor para de broadcastar frames desse player
- Ao reconectar, o player é tratado como novo player

---

### **PARTE 2: Cliente (Blueprint) - A IMPLEMENTAR**

**Arquivo:** `BP_NetMovementClient`

**Objetivo:** Criar uma função para destruir todos os RemoteActors e limpar arrays quando o WebSocket desconecta.

---

## 🎯 **INSTRUÇÕES PARA O BLUEPRINT:**

### **PASSO 1: Criar Custom Event `CleanupRemoteActors`**

1. No `BP_NetMovementClient`, vá em `Meu Blueprint` → `GRÁFICOS`
2. Clique no `+` ao lado de "GRÁFICOS" → selecione "Add Custom Event"
3. Nome: `CleanupRemoteActors`
4. **NÃO adicione inputs** (este evento não tem parâmetros)

---

### **PASSO 2: Implementar a Lógica de Limpeza**

No grafo do `CleanupRemoteActors`, adicione a seguinte lógica:

```
CleanupRemoteActors (Custom Event)
  ↓
Print String: "Cleaning up RemoteActors..."
  ↓
[LOOP] ForEachLoop (RemoteActors)
  ├─ Loop Body:
  │   ├─ Get Array Item (RemoteActors, Array Index) → ActorRef
  │   ├─ Is Valid (ActorRef)
  │   ├─ Branch: Is Valid?
  │   │   ├─ then (True): Destroy Actor (ActorRef)
  │   │   │              ↓
  │   │   │              Print String: "Destroyed RemoteActor"
  │   │   └─ else (False): [Ignorar]
  │   └─ [Continue Loop]
  ↓
Clear Array (RemoteActors)
  ↓
Clear Array (RemoteActorIds)
  ↓
Print String: "'Cleanup complete - All RemoteActors destroyed and arrays cleared'"
```

---

### **PASSO 3: Conectar ao Evento OnWSClosed**

**OPÇÃO A: Se já existe evento `OnWSClosed`:**

1. No `Event Graph` do `BP_NetMovementClient`, localize o evento `OnWSClosed`
2. Conecte a execução do `OnWSClosed` ao `CleanupRemoteActors`:

```
OnWSClosed (Custom Event)
  ↓
Print String: "WebSocket closed - cleaning up..."
  ↓
CleanupRemoteActors
```

**OPÇÃO B: Se não existe evento `OnWSClosed`:**

1. Verifique se o `Umbra WS Client` tem um evento `OnClosed`
2. Se sim, conecte o `Bind Event to OnClosed` ao `CleanupRemoteActors`:

```
Bind Event to OnClosed (WebSocketRef)
  ↓ (Event pin)
CleanupRemoteActors
```

**OPÇÃO C: Se usar VaRest WebSocket:**

1. Procure por eventos de desconexão do VaRest
2. Conecte ao `CleanupRemoteActors` quando o WebSocket desconectar

---

### **PASSO 4: Adicionar Logs Detalhados (Opcional)**

Para debug, adicione logs detalhados:

```
CleanupRemoteActors
  ↓
Get Array Length (RemoteActors) → NumActors
  ↓
Format Text: "Starting cleanup - {0} RemoteActors to destroy"
  - {0}: NumActors (convertido para String)
  ↓
Print String
  ↓
ForEachLoop (RemoteActors)
  ├─ Loop Body:
  │   ├─ Get Array Item → ActorRef
  │   ├─ Is Valid (ActorRef)
  │   ├─ Branch: Is Valid?
  │   │   ├─ then:
  │   │   │   ├─ Get Display Name (ActorRef) → ActorName
  │   │   │   ├─ Format Text: "Destroying RemoteActor: {0}"
  │   │   │   ├─ Print String
  │   │   │   ├─ Destroy Actor (ActorRef)
  │   │   │   └─ Print String: "Destroyed"
  │   │   └─ else:
  │   │       └─ Print String: "Skipping invalid actor"
  └─ Continue Loop
  ↓
Clear Array (RemoteActors)
  ↓
Clear Array (RemoteActorIds)
  ↓
Print String: "Cleanup complete!"
```

---

## 🔍 **VERIFICAÇÃO:**

### **Checklist:**

- [ ] `CleanupRemoteActors` Custom Event foi criado
- [ ] `ForEachLoop` está iterando sobre `RemoteActors`
- [ ] `Is Valid` está verificando cada actor antes de destruir
- [ ] `Destroy Actor` está sendo chamado para cada actor válido
- [ ] `Clear Array` está sendo chamado para `RemoteActors`
- [ ] `Clear Array` está sendo chamado para `RemoteActorIds`
- [ ] `CleanupRemoteActors` está conectado ao evento de desconexão do WebSocket

---

## 📊 **FLUXO COMPLETO:**

### **Quando um Client Desconecta:**

1. **Servidor (C++):**
   - WebSocket detecta desconexão
   - `handleClientDisconnect(cid)` é chamado
   - PlayerID é removido do `players_` map
   - Servidor para de broadcastar frames desse player

2. **Cliente (Blueprint):**
   - Evento `OnWSClosed` é disparado
   - `CleanupRemoteActors` é executado
   - Todos os RemoteActors são destruídos
   - Arrays `RemoteActors` e `RemoteActorIds` são limpos

### **Quando o mesmo Client Reconecta:**

1. **Servidor:**
   - Novo ClientID é atribuído
   - PlayerID é tratado como novo player
   - PlayerID é adicionado ao `players_` map novamente

2. **Cliente:**
   - Novos RemoteActors são spawnados normalmente
   - Arrays começam vazios (sem duplicatas)

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Implementar `CleanupRemoteActors` no Blueprint**
2. **Conectar ao evento de desconexão do WebSocket**
3. **Testar desconexão/reconexão:**
   - Conectar um client
   - Mover o personagem
   - Fechar o client
   - Verificar logs no servidor (deve mostrar "Removing player X")
   - Reconectar com o mesmo ID
   - Verificar que não há múltiplos spawns

---

## 📝 **NOTAS:**

- O servidor já está implementado e funcional
- Apenas o Blueprint precisa ser atualizado
- Se o WebSocket plugin não tiver evento `OnClosed`, pode ser necessário usar um timer ou verificação periódica da conexão
- Considere também limpar o buffer binário (`BinaryMessageBuffer`) na desconexão

---

**Fim do Documento**

