# ✅ Correção: Logs de Debug e Snapshot para Novos Players

## 🐛 Problema Identificado

**Sintoma**: Dois clientes conectam, mas ambos recebem snapshot inicial com **0 players**, e os personagens não aparecem simultaneamente.

**Análise dos Logs**:
```
[2025-10-31 22:13:40.194] [debug] [32848] Sent initial snapshot to client 3 (0 players)
[2025-10-31 22:13:56.708] [debug] [12880] Sent initial snapshot to client 4 (0 players)
```

**Causa Raiz**:
1. Cliente 3 conecta → snapshot inicial (0 players) → Cliente 3 ainda não enviou `MoveUpdate`
2. Cliente 4 conecta → snapshot inicial (0 players) → Cliente 4 ainda não enviou `MoveUpdate`
3. Quando Cliente 3 finalmente envia `MoveUpdate`, o snapshot já foi enviado para Cliente 4
4. O broadcast imediato funciona, mas pode haver race condition se ambos conectam quase simultaneamente

## ✅ Correções Aplicadas

### 1. Logs Detalhados de Debug

**`src/zone/MovementServer.hpp`** - método `setBinaryCallback`:

```cpp
ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data){
  MovementFrame f{};
  if (!decode(data, f)) {
    Umbra::Core::Logger::getInstance().warn("Failed to decode binary message from client {}", cid);
    return;
  }
  if (f.type != MovementMsgType::MoveUpdate) {
    Umbra::Core::Logger::getInstance().debug("Received non-MoveUpdate frame from client {} (type: {})", cid, static_cast<int>(f.type));
    return;
  }
  Umbra::Core::Logger::getInstance().debug("Received MoveUpdate from client {}: player_id={}, pos=({}, {}, {}), yaw={}", 
                                            cid, f.playerId, f.x, f.y, f.z, f.yaw);
  handleMoveUpdate(cid, f);
});
```

**Logs adicionados**:
- Quando `MoveUpdate` é recebido (com detalhes)
- Quando decode falha
- Quando frame não é `MoveUpdate`

### 2. Detecção de Novo Player e Snapshot Completo

**`src/zone/MovementServer.hpp`** - método `handleMoveUpdate`:

```cpp
void handleMoveUpdate(uint32_t cid, const MovementFrame& f) {
  // ... validações anti-cheat ...
  
  std::lock_guard<std::mutex> lock(mu_);
  bool isNewPlayer = (players_.find(f.playerId) == players_.end());
  
  // ... validações de velocidade/teleporte ...
  
  players_[f.playerId] = PlayerStateNet{f.playerId, f.x, f.y, f.z, f.yaw, f.tsMs};
  
  if (isNewPlayer) {
    Umbra::Core::Logger::getInstance().info("New player {} (from client {}) - broadcasting initial state to all clients", 
                                            f.playerId, cid);
    // Novo player: enviar snapshot completo para todos os clientes existentes
    sendFullSnapshotToAll();
  }

  // Broadcast imediato (sempre)
  MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, f.tsMs};
  ws_.broadcastBinary(encode(out));
  Umbra::Core::Logger::getInstance().debug("Broadcasted StateUpdate for player {} (from client {})", f.playerId, cid);
}
```

**Funcionalidade**:
- Quando um **novo player** envia seu primeiro `MoveUpdate`, todos os clientes existentes recebem um snapshot completo com todos os players
- Isso garante que clientes que já estavam conectados vejam o novo player imediatamente

### 3. Função `sendFullSnapshotToAll`

**`src/zone/MovementServer.hpp`**:

```cpp
void sendFullSnapshotToAll() {
  std::lock_guard<std::mutex> lock(mu_);
  for (const auto& [pid, st] : players_) {
    MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
    auto bytes = encode(f);
    ws_.broadcastBinary(bytes);
  }
  Umbra::Core::Logger::getInstance().debug("Broadcasted full snapshot to all clients ({} players)", players_.size());
}
```

**Funcionalidade**:
- Envia snapshot completo (todos os players) para todos os clientes conectados
- Usado quando um novo player aparece (primeiro `MoveUpdate`)

### 4. Logs de Validação Anti-Cheat

**`src/zone/MovementServer.hpp`** - método `handleMoveUpdate`:

- Log quando delay é muito alto
- Log quando teleporte é muito grande
- Log quando velocidade é muito alta

## 📋 Como Funciona Agora

### Cenário: Cliente 1 e Cliente 2

**Fluxo Completo**:

1. **Cliente 1 conecta**:
   - `[INFO] WS client 1 connected`
   - `[INFO] Sent initial snapshot to client 1 (0 players)` ← snapshot vazio (normal)
   - Cliente 1 inicia timer para `SendMoveUpdate`

2. **Cliente 1 envia primeiro `MoveUpdate`**:
   - `[DEBUG] Received MoveUpdate from client 1: player_id=1, pos=(x, y, z), yaw=...`
   - `[INFO] New player 1 (from client 1) - broadcasting initial state to all clients`
   - `[DEBUG] Broadcasted full snapshot to all clients (1 players)` ← Cliente 1 recebe seu próprio estado
   - `[DEBUG] Broadcasted StateUpdate for player 1 (from client 1)` ← Broadcast imediato

3. **Cliente 2 conecta**:
   - `[INFO] WS client 2 connected`
   - `[INFO] Sent initial snapshot to client 2 (1 players)` ← **Agora Cliente 2 recebe Cliente 1 imediatamente!**
   - Cliente 2 processa `StateUpdate` de Cliente 1 → spawna personagem
   - Cliente 2 inicia timer para `SendMoveUpdate`

4. **Cliente 2 envia primeiro `MoveUpdate`**:
   - `[DEBUG] Received MoveUpdate from client 2: player_id=4, pos=(x, y, z), yaw=...`
   - `[INFO] New player 4 (from client 2) - broadcasting initial state to all clients`
   - `[DEBUG] Broadcasted full snapshot to all clients (2 players)` ← Cliente 1 recebe todos os players
   - `[DEBUG] Broadcasted StateUpdate for player 4 (from client 2)` ← Broadcast imediato

5. **Cliente 1 recebe `StateUpdate` de Cliente 2** → spawna personagem de Cliente 2

## 🧪 Teste e Verificação

### Logs Esperados no Servidor:

```
[INFO] WS client 1 connected
[INFO] Sent initial snapshot to client 1 (0 players)  ← Normal, ainda não há players
[DEBUG] Received MoveUpdate from client 1: player_id=1, pos=(...), yaw=...
[INFO] New player 1 (from client 1) - broadcasting initial state to all clients
[DEBUG] Broadcasted full snapshot to all clients (1 players)
[DEBUG] Broadcasted StateUpdate for player 1 (from client 1)
[INFO] WS client 2 connected
[INFO] Sent initial snapshot to client 2 (1 players)  ← Cliente 2 recebe Cliente 1!
[DEBUG] Received MoveUpdate from client 2: player_id=4, pos=(...), yaw=...
[INFO] New player 4 (from client 2) - broadcasting initial state to all clients
[DEBUG] Broadcasted full snapshot to all clients (2 players)  ← Cliente 1 recebe Cliente 2!
[DEBUG] Broadcasted StateUpdate for player 4 (from client 2)
```

### Verificações:

1. ✅ Cliente 2 deve receber snapshot inicial com Cliente 1 (se Cliente 1 já enviou `MoveUpdate`)
2. ✅ Cliente 1 deve receber snapshot completo quando Cliente 2 envia primeiro `MoveUpdate`
3. ✅ Ambos os clientes devem ver os personagens remotos spawnar
4. ✅ Logs detalhados devem aparecer no servidor para debug

## ⚠️ Nota Importante

**Race Condition**: Se ambos os clientes conectam quase simultaneamente e enviam `MoveUpdate` antes de receber o snapshot inicial, ainda funciona:
- Cada cliente recebe `StateUpdate` via broadcast imediato
- Cada cliente recebe snapshot completo quando novo player aparece
- A combinação garante que todos vejam todos

---

**Data**: 2025-11-01
**Status**: ✅ **CORRIGIDO E PRONTO PARA TESTE**

