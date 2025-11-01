# ✅ Correção: Spawn de Múltiplos Clientes Simultaneamente

## 🐛 Problema Identificado

**Sintoma**: Ao logar 2 clientes com IDs diferentes, os dois personagens não são spawnados ao mesmo tempo, dificultando o teste de movimentação.

**Causa Raiz**: O servidor (`MovementServer`) não estava enviando um **snapshot inicial** quando um novo cliente conectava. O cliente só recebia updates quando:
1. Um player existente se movia (broadcast imediato)
2. O próximo snapshot periódico era enviado (~0.1s de atraso)

Isso causava um atraso no spawn dos personagens remotos, especialmente quando:
- Cliente 2 conectava após Cliente 1 já estar no jogo
- Cliente 2 só recebia o estado de Cliente 1 no próximo snapshot periódico ou quando Cliente 1 se movia

## ✅ Correção Aplicada

### 1. Adicionada função `sendInitialSnapshot` no `MovementServer`

**`src/zone/MovementServer.hpp`**:

```cpp
private:
  void sendInitialSnapshot(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encode(f);
      ws_.sendBinary(clientId, bytes);
    }
    Umbra::Core::Logger::getInstance().debug("Sent initial snapshot to client {} ({} players)", 
                                             clientId, players_.size());
  }
```

**Funcionalidade**:
- Envia o estado de **todos os players existentes** para o novo cliente imediatamente após a conexão
- Usa `sendBinary(clientId, ...)` em vez de `broadcastBinary(...)` para enviar apenas ao cliente específico
- Thread-safe (usa mutex)

### 2. Chamada no callback de conexão

**`src/zone/MovementServer.hpp`** - método `start()`:

```cpp
ws_.setConnectionCallback([this](uint32_t cid, bool connected){
  if (connected) {
    Umbra::Core::Logger::getInstance().info("WS client {} connected", cid);
    // Enviar snapshot inicial para o novo cliente
    sendInitialSnapshot(cid);
  } else {
    Umbra::Core::Logger::getInstance().info("WS client {} disconnected", cid);
  }
});
```

**Fluxo**:
1. Cliente conecta → WebSocket handshake completa
2. `setConnectionCallback` é disparado com `connected = true`
3. `sendInitialSnapshot(cid)` é chamado imediatamente
4. Novo cliente recebe todos os `StateUpdate` dos players já conectados
5. Cliente processa cada `StateUpdate` no `OnWSBinaryMessage`
6. Para cada `StateUpdate` com `OutPlayerId != ActivePlayerID`, o cliente spawna o personagem remoto

## 📋 Como Funciona Agora

### Cenário: Cliente 1 e Cliente 2 conectando

**ANTES da correção:**
1. Cliente 1 conecta → não há players, snapshot vazio
2. Cliente 1 envia `MoveUpdate` → servidor atualiza estado e faz broadcast
3. Cliente 2 conecta → **não recebe snapshot inicial**, só espera próximo snapshot periódico
4. Cliente 2 pode não ver Cliente 1 até que:
   - Cliente 1 se mova novamente (broadcast imediato), OU
   - Próximo snapshot periódico (~0.1s depois)

**DEPOIS da correção:**
1. Cliente 1 conecta → snapshot vazio (nenhum player ainda)
2. Cliente 1 envia `MoveUpdate` → servidor atualiza estado e faz broadcast
3. Cliente 2 conecta → **recebe snapshot inicial imediatamente** com estado de Cliente 1
4. Cliente 2 processa `StateUpdate` de Cliente 1 → spawna personagem imediatamente
5. Cliente 2 envia `MoveUpdate` → servidor atualiza e faz broadcast
6. Cliente 1 recebe `StateUpdate` de Cliente 2 → spawna personagem imediatamente

## ✅ Verificações no Cliente Blueprint

Certifique-se de que o Blueprint está:

1. **Verificando se `OutPlayerId != ActivePlayerID`** antes de processar `StateUpdate`:
   ```blueprint
   ParseStateUpdateFrame(Data) → OutPlayerId
   GetActivePlayerID() → ActivePlayerID
   Branch: OutPlayerId != ActivePlayerID?
   ```
   - **True**: Processar (é um player remoto)
   - **False**: Ignorar (é o próprio player)

2. **Spawnando personagem remoto** quando recebe `StateUpdate` de um player não existente:
   ```blueprint
   Find Item in Array (RemoteActorIds, OutPlayerId) → FoundIndex
   Branch: FoundIndex >= 0?
   - False: Spawn Actor + Add to Arrays
   - True: Actor já existe, só atualizar posição
   ```

3. **Processando cada `StateUpdate` recebido** no `OnWSBinaryMessage`, mesmo se for do snapshot inicial

## 🧪 Teste

### Passos para testar:

1. **Inicie o Zone Server**: `zone_server.exe 0`
2. **Inicie Cliente 1** (PIE):
   - Login → Select Character
   - Personagem deve aparecer e enviar `MoveUpdate`
3. **Inicie Cliente 2** (PIE em outra janela):
   - Login com **outra conta/personagem**
   - Select Character
   - **Cliente 2 deve ver Cliente 1 imediatamente** (não precisa esperar movimento)
4. **Mova ambos os personagens**:
   - Ambos devem ver o movimento um do outro em tempo real

### Logs esperados no servidor:

```
[INFO] WS client 1 connected
[DEBUG] Sent initial snapshot to client 1 (0 players)  ← Cliente 1 conecta (nenhum player ainda)
[INFO] WS client 2 connected
[DEBUG] Sent initial snapshot to client 2 (1 players)  ← Cliente 2 recebe estado de Cliente 1 imediatamente
```

### Logs esperados no cliente:

- **Cliente 1**: Recebe `StateUpdate` de Cliente 2 quando Cliente 2 envia `MoveUpdate`
- **Cliente 2**: Recebe `StateUpdate` de Cliente 1 **imediatamente ao conectar** (snapshot inicial)

## 📋 Arquivos Modificados

1. `src/zone/MovementServer.hpp`:
   - Adicionada função `sendInitialSnapshot(uint32_t clientId)`
   - Chamada de `sendInitialSnapshot` no callback de conexão

## ⚠️ Notas Importantes

1. **Performance**: O snapshot inicial envia um frame por player. Para muitos players (centenas), considere enviar em batch ou comprimir.

2. **Race Condition**: Se dois clientes conectam quase simultaneamente, ambos podem receber snapshot inicial antes do outro enviar `MoveUpdate`. Isso está correto - cada cliente recebe o estado atual de todos os players.

3. **Handshake**: O snapshot inicial é enviado **após** o handshake WebSocket completar. Se o handshake falhar, o snapshot não é enviado (comportamento correto).

---

**Data**: 2025-11-01
**Status**: ✅ **CORRIGIDO E PRONTO PARA TESTE**

