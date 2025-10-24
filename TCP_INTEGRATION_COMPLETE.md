# Implementação de Conexão TCP com Servidor de Autenticação

## Visão Geral

Este documento descreve a implementação completa da conexão TCP entre o Gateway Server e o Auth Server para autenticação real em tempo real.

## Arquitetura Implementada

### 1. AuthClient (Cliente TCP Individual)
- **Arquivo**: `src/gateway/AuthClient.hpp` e `AuthClient.cpp`
- **Função**: Cliente TCP individual para comunicação com Auth Server
- **Recursos**:
  - Conexão TCP persistente
  - Validação de tokens JWT
  - Verificação de sessões
  - Revogação de tokens
  - Reconexão automática
  - Timeout configurável

### 2. AuthConnectionPool (Pool de Conexões)
- **Arquivo**: `src/gateway/AuthConnectionPool.hpp` e `AuthConnectionPool.cpp`
- **Função**: Pool de conexões TCP para alta disponibilidade
- **Recursos**:
  - Múltiplas conexões por servidor
  - Balanceamento de carga
  - Health check automático
  - Limpeza de conexões não saudáveis
  - Estatísticas de performance

### 3. GatewayServer Atualizado
- **Arquivo**: `src/gateway/GatewayServer.hpp` e `GatewayServer.cpp`
- **Função**: Servidor gateway com integração TCP
- **Recursos**:
  - Suporte a cliente individual ou pool
  - Cache de sessões
  - Fallback para validação local JWT
  - Limpeza automática de sessões expiradas

## Protocolo de Comunicação TCP

### Formato de Mensagem
```json
{
  "action": "validate_token|check_session|revoke_token",
  "data": {
    "token": "jwt_token_string",
    "account_id": 12345
  },
  "timestamp": 1640995200000
}
```

### Resposta do Auth Server
```json
{
  "success": true,
  "valid": true,
  "account_id": 12345,
  "player_id": 67890,
  "username": "player_name",
  "message": "Token válido"
}
```

## Configuração

### Arquivo de Configuração (`config/server.json`)
```json
{
  "gateway": {
    "port": 9000,
    "auth_host": "localhost",
    "auth_port": 8080,
    "auth_timeout_ms": 5000,
    "use_connection_pool": true,
    "max_connections_per_host": 3,
    "health_check_interval_ms": 30000
  }
}
```

### Configurações Disponíveis
- `use_connection_pool`: Usar pool de conexões (true) ou cliente único (false)
- `max_connections_per_host`: Número máximo de conexões por servidor Auth
- `health_check_interval_ms`: Intervalo de verificação de saúde das conexões
- `auth_timeout_ms`: Timeout para operações de autenticação

## Fluxo de Autenticação

### 1. Cliente conecta ao Gateway
```
Cliente → Gateway Server (porta 9000)
```

### 2. Cliente envia token para autenticação
```json
{
  "action": "authenticate",
  "token": "jwt_token_here"
}
```

### 3. Gateway valida token via TCP
```
Gateway → Auth Server (porta 8080)
```

### 4. Auth Server responde com validação
```json
{
  "success": true,
  "valid": true,
  "account_id": 12345,
  "player_id": 67890,
  "username": "player_name"
}
```

### 5. Gateway responde ao cliente
```json
{
  "success": true,
  "message": "Authentication successful",
  "account_id": 12345,
  "player_id": 67890,
  "username": "player_name"
}
```

## Recursos de Alta Disponibilidade

### Pool de Conexões
- **Múltiplas conexões**: Até 3 conexões por servidor Auth
- **Balanceamento**: Distribuição automática de requisições
- **Health check**: Verificação periódica da saúde das conexões
- **Reconexão**: Reconexão automática em caso de falha

### Fallback de Segurança
- **Validação local**: Se Auth Server não estiver disponível, usa validação JWT local
- **Cache de sessões**: Cache de sessões válidas para reduzir latência
- **Limpeza automática**: Limpeza periódica de sessões expiradas

## Monitoramento e Estatísticas

### Estatísticas do Pool de Conexões
```json
{
  "total_connections": 3,
  "available_connections": 2,
  "active_connections": 3,
  "total_requests": 1250,
  "successful_requests": 1200,
  "failed_requests": 50,
  "pool_hits": 1100,
  "pool_misses": 150,
  "hit_rate": 0.88
}
```

### Logs de Monitoramento
- Conexões estabelecidas/perdidas
- Requisições de autenticação
- Falhas de conexão
- Health check results
- Performance metrics

## Testes

### Script de Teste Automatizado
- **Arquivo**: `test_tcp_integration.py`
- **Função**: Testa integração TCP completa
- **Testes incluídos**:
  - Conexão com Gateway Server
  - Ping/pong
  - Informações do servidor
  - Autenticação com token válido
  - Rejeição de token inválido
  - Simulador de Auth Server

### Execução dos Testes
```bash
# Executar testes
python test_tcp_integration.py

# Build e teste completo
build_and_test.bat
```

## Compilação

### Pré-requisitos
- CMake 3.20+
- Compilador C++17 (MSVC, GCC, Clang)
- Bibliotecas de terceiros (nlohmann/json, spdlog, jwt-cpp)

### Comandos de Build
```bash
# Configurar projeto
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compilar
cmake --build build --config Release

# Executar
./build/bin/Release/umbra_server.exe
```

## Benefícios da Implementação

### 1. Autenticação em Tempo Real
- Validação de tokens em tempo real
- Revogação imediata de tokens
- Verificação de sessões ativas

### 2. Alta Disponibilidade
- Pool de conexões para redundância
- Reconexão automática
- Fallback para validação local

### 3. Performance
- Cache de sessões válidas
- Múltiplas conexões simultâneas
- Balanceamento de carga

### 4. Monitoramento
- Estatísticas detalhadas
- Logs de performance
- Health check automático

### 5. Segurança
- Comunicação TCP segura
- Validação centralizada
- Controle de sessões

## Próximos Passos

### Melhorias Futuras
1. **Criptografia**: Adicionar criptografia TLS/SSL
2. **Load Balancing**: Balanceamento entre múltiplos Auth Servers
3. **Metrics**: Integração com sistemas de monitoramento
4. **Caching**: Cache distribuído com Redis
5. **Rate Limiting**: Limitação de taxa por IP/usuário

### Integração com UE5
1. **VaRest**: Integração com VaRest para comunicação HTTP
2. **Blueprints**: Criação de Blueprints para autenticação
3. **Widgets**: Interface de usuário para login/logout
4. **Saves**: Sistema de salvamento de progresso

## Conclusão

A implementação da conexão TCP entre Gateway e Auth Server fornece uma base sólida para autenticação real em tempo real, com recursos de alta disponibilidade, performance e segurança. O sistema é extensível e pode ser facilmente integrado com o Unreal Engine 5 para criar uma experiência de jogo completa.
