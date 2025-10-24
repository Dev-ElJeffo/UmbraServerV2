# Changelog

Todas as mudanças notáveis do projeto UmbraEternum Server serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
e este projeto adere ao [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.0] - 2025-10-12

### Added
- **Core Layer**
  - ConfigManager com suporte a JSON e hot-reload
  - Logger assíncrono com spdlog e rotação de arquivos
  - Timer para agendamento de tarefas periódicas
  - Utils com helpers gerais (UUID, base64, hash)

- **Network Layer**
  - SocketServer TCP/UDP com ASIO
  - WebSocketServer (RFC 6455) para integração UE5
  - MessageHandler para serialização/deserialização
  - Suporte a rate-limiting e reconexão

- **Database Layer**
  - MySQLConnector com prepared statements
  - AccountDAO e PlayerDAO seguindo padrão DAO
  - Models completos (Account, Player, Item, Guild, etc.)
  - Sistema de migrations SQL

- **Auth Server**
  - Autenticação JWT (HS256)
  - SessionManager com suporte a Redis
  - Endpoints REST para login/registro
  - Sistema de banimento de contas
  - Rate-limiting de login attempts

- **World Server**
  - Coordenação de eventos globais
  - EventManager com sistema pub/sub
  - TimeManager para sincronização de tempo
  - Suporte a múltiplos Zone Servers

- **Zone Servers**
  - Gerenciamento de regiões específicas
  - PlayerManager para jogadores na zona
  - EntitySystem para NPCs e mobs
  - Suporte a escalabilidade horizontal

- **Chat Server**
  - Canais globais, trade, guild
  - ChannelManager com permissões
  - Whisper (mensagens privadas)
  - Redis pub/sub para broadcast

- **Gateway Server**
  - Proxy de conexões
  - LoadBalancer round-robin
  - Validação JWT centralizada
  - Proteção DDoS com rate-limiting

- **Services**
  - Matchmaking básico
  - InventoryService (50 slots)
  - CombatService com cálculos de dano

- **Build & Testing**
  - CMake 3.20+ multi-platform
  - Google Test framework integrado
  - Testes para Auth, Database, Zone, Network
  - Scripts de build e deploy

- **Documentation**
  - README completo com guias
  - Documentação de arquitetura
  - Guia de integração UE5
  - QuickStart guide
  - Contributing guidelines

### Security
- JWT tokens com expiração configurável
- Bcrypt-style password hashing com salt
- SQL injection prevention (prepared statements)
- Rate-limiting em todos os endpoints
- Server-authoritative validação
- Audit logging para ações sensíveis

### Infrastructure
- Git submodules para dependências
- Docker support (preparado)
- Logs estruturados com rotação
- Backup automatizado de banco
- Scripts de inicialização (Windows/Linux)

## [1.2.0] - 2025-10-05 (Planejado)

### Planned
- Suporte a Protobuf para serialização
- gRPC para comunicação entre servidores
- Redis cache layer completo
- Metrics e monitoring (Prometheus)
- Docker Compose para desenvolvimento
- Kubernetes manifests para produção

## [1.1.0] - 2025-09-28 (Planejado)

### Planned
- Sistema de guilds completo
- Trade system
- Auction house
- Friends list
- Party system
- Achievements

## [1.0.0] - 2025-09-15 (Milestone)

### Target Features
- MVP funcional com todos os servidores
- Integração UE5 completa e testada
- Suporte a 1000+ jogadores simultâneos
- Documentação completa
- CI/CD pipeline
- Testes de carga aprovados

## Roadmap

### Q4 2025
- [ ] Otimizações de performance
- [ ] Load testing com 10k+ players
- [ ] Security audit
- [ ] Beta testing

### Q1 2026
- [ ] Public beta
- [ ] Stress testing
- [ ] Bug fixes e polish
- [ ] Production deployment

### Q2 2026
- [ ] Official launch
- [ ] Post-launch support
- [ ] Content updates

## Notes

### Versioning Strategy
- **Major (X.0.0)**: Breaking changes, major features
- **Minor (1.X.0)**: New features, backward compatible
- **Patch (1.0.X)**: Bug fixes, minor improvements

### Release Cycle
- Sprint releases: Semanalmente (develop)
- Minor releases: Mensalmente
- Major releases: Trimestralmente

---

**Mantido por**: UmbraEternum Development Team  
**Última atualização**: 2025-10-12

