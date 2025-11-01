# ✅ Correção: Timestamp e Validação de Delay

## 🐛 Problema Identificado

**Sintoma**: Todos os `MoveUpdate` do cliente são rejeitados com erro:
```
[warning] MoveUpdate from client 1 rejected: delay too high (497780938ms)
```

**Causa Raiz**:
- **Cliente** usa `Get Game Time in Seconds` que retorna tempo **relativo ao início do jogo** (ex.: 497780 segundos = ~138 horas desde o início da sessão)
- **Servidor** usa `steady_clock::now().time_since_epoch()` que retorna tempo **absoluto desde época Unix** (1 de janeiro de 1970)

**Resultado**: 
- Cliente envia timestamp ~497780938ms (tempo relativo)
- Servidor compara com timestamp ~1727820023000ms (tempo absoluto)
- Diferença de ~497780938ms → rejeitado como "delay too high"

## ✅ Correção Aplicada

### 1. Validação Mais Tolerante para Timestamps Relativos

**`src/zone/MovementServer.hpp`** - método `handleMoveUpdate`:

**ANTES**:
```cpp
if (nowMs > f.tsMs && (nowMs - f.tsMs) > maxDelayMs_) {
  // Rejeita se delay > 300ms
  return;
}
```

**DEPOIS**:
```cpp
// Para novos players, sempre aceitar o primeiro movimento
if (!isNewPlayer) {
  // Validação apenas para players existentes
  // Não podemos validar delay absoluto porque cliente usa "Game Time" (relativo)
  // Apenas validamos se timestamp está muito no futuro
  if (f.tsMs > nowMs && (f.tsMs - nowMs) > 5000) {
    // Rejeita apenas se timestamp está mais de 5 segundos no futuro
    return;
  }
}
```

**Mudanças**:
- ✅ Remove validação de delay absoluto (não funciona com timestamps relativos)
- ✅ Apenas valida se timestamp está muito no futuro (>5 segundos)
- ✅ Aceita primeiro movimento de novos players sempre

### 2. Timestamp do Servidor para Novos Players

```cpp
// Para novos players, usar timestamp do servidor (sincronizado)
// Para players existentes, manter timestamp relativo do cliente (para cálculo de velocidade)
uint32_t finalTimestamp = isNewPlayer ? nowMs : f.tsMs;
```

**Funcionalidade**:
- **Novos players**: Usa timestamp do servidor (absoluto) para sincronização inicial
- **Players existentes**: Mantém timestamp relativo do cliente para cálculo correto de velocidade

### 3. Cálculo de Velocidade Usando Timestamp Relativo

```cpp
// velocidade aprox em uu/s usando timestamp relativo do próprio cliente
// Se o timestamp do frame é maior que o anterior, usar a diferença
// Senão, assumir intervalo mínimo (1 frame = ~16ms)
uint32_t prevTs = it->second.tsMs;
float dt = (f.tsMs > prevTs) ? (f.tsMs - prevTs) / 1000.0f : 0.016f;
float speed = (dt > 0.0f) ? std::sqrt(dist2) / dt : 0.0f;
```

**Funcionalidade**:
- Usa diferença entre timestamps **relativos** do próprio cliente
- Se timestamp anterior > atual (problema de ordem), assume 16ms (1 frame a 60 FPS)
- Isso funciona mesmo com timestamps relativos ao jogo

## 📋 Como Funciona Agora

### Fluxo para Novo Player:

1. **Cliente conecta** → não há players ainda
2. **Cliente envia primeiro `MoveUpdate`**:
   - Timestamp relativo: `497780938ms` (tempo desde início do jogo)
   - Servidor detecta: `isNewPlayer = true`
   - Servidor aceita movimento (sem validação de delay)
   - Servidor usa timestamp do servidor: `1727820023000ms` (absoluto)
   - Servidor atualiza estado e faz broadcast

### Fluxo para Player Existente:

1. **Cliente envia `MoveUpdate` subsequente**:
   - Timestamp relativo: `497781005ms` (incremento de 67ms)
   - Servidor detecta: `isNewPlayer = false`
   - Servidor valida apenas se timestamp > servidor + 5000ms (futuro distante)
   - Servidor usa timestamp relativo do cliente: `497781005ms`
   - Servidor calcula velocidade usando diferença entre timestamps relativos do cliente
   - Servidor atualiza estado e faz broadcast

## ✅ Benefícios

1. **Aceita movimentos de novos players** → não rejeita primeiro movimento
2. **Funciona com timestamps relativos** → compatível com "Game Time" do Unreal
3. **Cálculo de velocidade correto** → usa diferença entre timestamps relativos do mesmo cliente
4. **Validação anti-cheat mantida** → ainda valida velocidade, teleporte, timestamps no futuro

## 🧪 Teste

### Logs Esperados:

**Novo Player**:
```
[DEBUG] Received MoveUpdate from client 1: player_id=4, pos=(...), yaw=...
[INFO] First MoveUpdate from player 4 (client 1): accepting with server timestamp
[INFO] New player 4 (from client 1) - broadcasting initial state to all clients
[DEBUG] Broadcasted StateUpdate for player 4 (from client 1)
```

**Player Existente**:
```
[DEBUG] Received MoveUpdate from client 1: player_id=4, pos=(...), yaw=...
[DEBUG] Broadcasted StateUpdate for player 4 (from client 1)
```

### Não Deve Mais Aparecer:
```
[WARNING] MoveUpdate from client X rejected: delay too high (...)
```

---

**Data**: 2025-11-01
**Status**: ✅ **CORRIGIDO E PRONTO PARA TESTE**

