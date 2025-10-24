# Arquitetura do Sistema UmbraEternum

## Visão Geral

O UmbraEternum utiliza arquitetura de micro-serviços em C++17 para alta performance e escalabilidade horizontal.

## Topologia de Servidores

```
┌──────────────────────┐
│  Cliente UE5         │
└──────┬───────────────┘
       │ REST/WebSocket
       ▼
┌──────────────────────┐
│  Game Gateway        │  ← Proxy, JWT validation, Load balancing
└──────┬───────────────┘
       │
   ┌───┴────┬──────────┬──────────┐
   ▼        ▼          ▼          ▼
┌────┐  ┌────┐     ┌────┐    ┌────┐
│Auth│  │Zone│ ... │Zone│    │Chat│
│Svr │  │ #1 │     │ #N │    │Svr │
└─┬──┘  └──┬─┘     └──┬─┘    └──┬─┘
  │        └────┬─────┘         │
  │             ▼               │
  │       ┌──────────┐          │
  │       │  World   │          │
  │       │  Server  │          │
  │       └────┬─────┘          │
  │            │                │
  └────────┬───┴────────────────┘
           ▼
    ┌──────────────┐
    │  Database    │  MySQL + Redis
    │  (MySQL)     │
    └──────────────┘
```

## Componentes Principais

### 1. Core Layer (`src/core/`)
Componentes base compartilhados:
- **ConfigManager**: Gerenciamento de configurações JSON
- **Logger**: Sistema de logging assíncrono (spdlog)
- **Timer**: Agendamento de tarefas
- **Utils**: Utilitários gerais (hash, UUID, base64)

### 2. Network Layer (`src/network/`)
Comunicação de rede:
- **SocketServer**: TCP/UDP com ASIO
- **WebSocketServer**: WebSocket RFC 6455 para UE5
- **MessageHandler**: Serialização/deserialização de mensagens

### 3. Database Layer (`src/database/`)
Persistência de dados:
- **MySQLConnector**: Wrapper para MySQL com prepared statements
- **DAOs**: AccountDAO, PlayerDAO (padrão DAO)
- **Models**: Estruturas de dados (Account, Player, Item, etc.)

### 4. Auth Server (`src/auth/`)
Autenticação e sessões:
- **AuthServer**: REST API para login/registro
- **JWTManager**: Tokens JWT (HS256)
- **SessionManager**: Gerenciamento de sessões em memória/Redis

### 5. World Server (`src/world/`)
Lógica global:
- **WorldServer**: Coordenação de eventos globais
- **EventManager**: Sistema pub/sub para eventos
- **TimeManager**: Sincronização de tempo do jogo

### 6. Zone Servers (`src/zone/`)
Gerenciamento de regiões (instâncias múltiplas):
- **ZoneServer**: Lógica de região específica
- **PlayerManager**: Gerenciamento de jogadores na zona
- **EntitySystem**: NPCs, mobs, objetos

### 7. Chat Server (`src/chat/`)
Mensageria:
- **ChatServer**: Processamento de mensagens
- **ChannelManager**: Canais (global, trade, guild, whisper)

### 8. Gateway Server (`src/gateway/`)
Ponto de entrada:
- **GatewayServer**: Validação JWT, rate-limiting
- **LoadBalancer**: Distribuição de carga entre Zone Servers

### 9. Services (`src/services/`)
Serviços auxiliares:
- **Matchmaking**: Matchmaking de jogadores
- **InventoryService**: Lógica de inventário
- **CombatService**: Cálculos de combate

## Fluxos de Comunicação

### Fluxo de Login
```
1. Cliente → Gateway: HTTP POST /auth/login
2. Gateway → Auth Server: Valida credenciais
3. Auth Server → Database: Busca conta
4. Auth Server → Cliente: Retorna JWT + servidor info
5. Cliente → Zone Server: Conecta via WebSocket com JWT
```

### Fluxo de Movimentação
```
1. Cliente → Zone Server: UDP packet (posição)
2. Zone Server: Valida movimento (anti-cheat)
3. Zone Server → Clientes próximos: Broadcast delta
4. Zone Server → Database: Sincroniza posição (periódico)
```

### Fluxo de Chat
```
1. Cliente → Chat Server: Mensagem
2. Chat Server → ChannelManager: Valida permissões
3. Chat Server → Clientes no canal: Broadcast
4. Chat Server → Database: Log de mensagem (opcional)
```

## Escalabilidade

### Horizontal
- **Zone Servers**: Múltiplas instâncias por região
- **Load Balancer**: Round-robin com awareness de carga
- **Database**: Master-slave replication

### Vertical
- **Thread Pools**: Workers para processamento paralelo
- **Async I/O**: ASIO para operações não-bloqueantes
- **Cache**: Redis para dados quentes (sessões, leaderboards)

## Segurança

- **JWT**: Autenticação stateless
- **HTTPS/TLS**: Comunicação encriptada
- **Rate Limiting**: DDoS protection
- **Server-Authoritative**: Validação server-side
- **Prepared Statements**: SQL injection protection
- **Input Validation**: Sanitização de dados

## Performance Targets

- **Latência**: < 50ms (target)
- **Throughput**: 10k+ jogadores simultâneos por cluster
- **Tick Rate**: 60 Hz para Zone Servers
- **Database**: < 10ms query time
- **Uptime**: 99.9% (target)

## Monitoramento

- **Logs**: spdlog com rotação diária/tamanho
- **Métricas**: Jogadores ativos, latência, carga
- **Alertas**: Crash reports, high load
- **Audit**: Todas as ações sensíveis

## Deployment

- **Build**: CMake cross-platform
- **Containers**: Docker (opcional)
- **Orchestration**: Kubernetes (fase futura)
- **CI/CD**: GitHub Actions

## Referências

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [WebSocket RFC 6455](https://tools.ietf.org/html/rfc6455)
- [JWT RFC 7519](https://tools.ietf.org/html/rfc7519)
- [Unreal Engine Networking](https://docs.unrealengine.com/en-US/InteractiveExperiences/Networking/)

