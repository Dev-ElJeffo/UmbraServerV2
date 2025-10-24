# Estrutura do Projeto UmbraEternum Server

## Visão Geral Completa

```
UmbraServer/
├── 📄 README.md                      # Documentação principal
├── 📄 LICENSE                        # Licença proprietária
├── 📄 CHANGELOG.md                   # Histórico de mudanças
├── 📄 CONTRIBUTING.md                # Guia de contribuição
├── 📄 .gitignore                     # Arquivos ignorados pelo Git
├── 📄 .gitmodules                    # Configuração de submodules
├── 📄 CMakeLists.txt                 # Build principal (3.20+)
│
├── 📁 src/                           # ⭐ CÓDIGO-FONTE PRINCIPAL
│   ├── 📄 main.cpp                   # Entry point (servidor completo)
│   │
│   ├── 📁 core/                      # ⚙️ Componentes Base
│   │   ├── CMakeLists.txt
│   │   ├── ConfigManager.{hpp,cpp}   # Gerenciamento de configs (JSON)
│   │   ├── Logger.{hpp,cpp}          # Logging assíncrono (spdlog)
│   │   ├── Timer.{hpp,cpp}           # Agendamento de tarefas
│   │   └── Utils.{hpp,cpp}           # Utilitários (hash, UUID, base64)
│   │
│   ├── 📁 network/                   # 🌐 Camada de Rede
│   │   ├── CMakeLists.txt
│   │   ├── SocketServer.{hpp,cpp}    # TCP/UDP com ASIO
│   │   ├── WebSocketServer.{hpp,cpp} # WebSocket RFC 6455
│   │   └── MessageHandler.{hpp,cpp}  # Serialização de mensagens
│   │
│   ├── 📁 database/                  # 💾 Persistência
│   │   ├── CMakeLists.txt
│   │   ├── Models.hpp                # Structs (Account, Player, Item)
│   │   ├── MySQLConnector.{hpp,cpp}  # Wrapper MySQL
│   │   ├── AccountDAO.{hpp,cpp}      # CRUD de contas
│   │   └── PlayerDAO.{hpp,cpp}       # CRUD de jogadores
│   │
│   ├── 📁 auth/                      # 🔐 Autenticação
│   │   ├── CMakeLists.txt
│   │   ├── main_auth.cpp             # Entry point standalone
│   │   ├── AuthServer.{hpp,cpp}      # Servidor de autenticação
│   │   ├── JWTManager.{hpp,cpp}      # Tokens JWT (HS256)
│   │   └── SessionManager.{hpp,cpp}  # Gerenciamento de sessões
│   │
│   ├── 📁 world/                     # 🌍 Mundo Global
│   │   ├── CMakeLists.txt
│   │   ├── main_world.cpp            # Entry point standalone
│   │   ├── WorldServer.{hpp,cpp}     # Servidor de mundo
│   │   ├── EventManager.{hpp,cpp}    # Sistema pub/sub
│   │   └── TimeManager.{hpp,cpp}     # Sincronização de tempo
│   │
│   ├── 📁 zone/                      # 🗺️ Zonas/Regiões
│   │   ├── CMakeLists.txt
│   │   ├── main_zone.cpp             # Entry point standalone
│   │   ├── ZoneServer.{hpp,cpp}      # Servidor de zona
│   │   ├── PlayerManager.{hpp,cpp}   # Jogadores na zona
│   │   └── EntitySystem.{hpp,cpp}    # NPCs, mobs, objetos
│   │
│   ├── 📁 chat/                      # 💬 Chat/Mensageria
│   │   ├── CMakeLists.txt
│   │   ├── main_chat.cpp             # Entry point standalone
│   │   ├── ChatServer.{hpp,cpp}      # Servidor de chat
│   │   └── ChannelManager.{hpp,cpp}  # Canais (global, guild, etc.)
│   │
│   ├── 📁 gateway/                   # 🚪 Gateway/Proxy
│   │   ├── CMakeLists.txt
│   │   ├── main_gateway.cpp          # Entry point standalone
│   │   ├── GatewayServer.{hpp,cpp}   # Gateway principal
│   │   └── LoadBalancer.{hpp,cpp}    # Balanceamento de carga
│   │
│   └── 📁 services/                  # 🛠️ Serviços Auxiliares
│       ├── CMakeLists.txt
│       ├── Matchmaking.{hpp,cpp}     # Sistema de matchmaking
│       ├── InventoryService.{hpp,cpp}# Lógica de inventário
│       └── CombatService.{hpp,cpp}   # Cálculos de combate
│
├── 📁 config/                        # ⚙️ CONFIGURAÇÕES
│   ├── server.json                   # Config principal (portas, etc.)
│   ├── db.json.example               # Template de credenciais DB
│   └── jwt_secret.key.example        # Template de chave JWT
│
├── 📁 scripts/                       # 🔧 AUTOMAÇÃO
│   ├── migrate_db.sql                # Schema e migrations MySQL
│   ├── start.sh                      # Inicializa todos (Linux/Mac)
│   ├── start.bat                     # Inicializa todos (Windows)
│   └── backup_db.sh                  # Backup automático MySQL
│
├── 📁 tests/                         # ✅ TESTES
│   ├── CMakeLists.txt
│   ├── test_auth.cpp                 # Testes de autenticação
│   ├── test_db.cpp                   # Testes de database
│   ├── test_zone.cpp                 # Testes de zona
│   └── test_network.cpp              # Testes de rede
│
├── 📁 third_party/                   # 📦 BIBLIOTECAS EXTERNAS
│   ├── README.md                     # Instruções de setup
│   ├── json/                         # nlohmann/json (submodule)
│   ├── spdlog/                       # spdlog (submodule)
│   ├── googletest/                   # Google Test (submodule)
│   ├── jwt-cpp/                      # jwt-cpp (submodule)
│   └── mysql-connector-cpp/          # MySQL Connector (manual)
│
├── 📁 docs/                          # 📚 DOCUMENTAÇÃO
│   ├── ARCHITECTURE.md               # Arquitetura do sistema
│   ├── INTEGRATION_UE5.md            # Guia de integração UE5
│   └── QUICKSTART.md                 # Guia de início rápido
│
├── 📁 build/                         # 🏗️ BUILD (gerado, gitignore)
│   ├── bin/                          # Executáveis
│   │   ├── umbra_server              # Servidor completo
│   │   ├── auth_server               # Auth standalone
│   │   ├── world_server              # World standalone
│   │   ├── zone_server               # Zone standalone
│   │   ├── chat_server               # Chat standalone
│   │   ├── gateway_server            # Gateway standalone
│   │   ├── test_auth                 # Testes auth
│   │   ├── test_db                   # Testes database
│   │   ├── test_zone                 # Testes zona
│   │   └── test_network              # Testes rede
│   └── lib/                          # Bibliotecas estáticas
│
└── 📁 logs/                          # 📋 LOGS (gerado, gitignore)
    ├── umbra_server.log
    ├── auth_server.log
    ├── world_server.log
    ├── zone_server.log
    ├── chat_server.log
    └── gateway_server.log
```

## Estatísticas do Projeto

### Código
- **Total de arquivos C++**: ~80+
- **Headers (.hpp)**: ~40+
- **Implementations (.cpp)**: ~40+
- **Linhas de código**: ~8000+ (estimado)
- **Padrão**: C++17

### Componentes
- **Servidores**: 6 (Auth, World, Zone, Chat, Gateway, Main)
- **Módulos Core**: 4 (ConfigManager, Logger, Timer, Utils)
- **DAOs**: 2 (Account, Player)
- **Services**: 3 (Matchmaking, Inventory, Combat)
- **Testes**: 4 suites (Auth, DB, Zone, Network)

### Dependências
- **nlohmann/json**: Parser JSON
- **spdlog**: Logging assíncrono
- **googletest**: Framework de testes
- **jwt-cpp**: Autenticação JWT
- **mysql-connector-cpp**: Conector MySQL
- **ASIO**: I/O assíncrono (opcional)

## Executáveis Principais

| Executável | Porta Padrão | Descrição |
|------------|--------------|-----------|
| `umbra_server` | Multi | Servidor completo (todos integrados) |
| `auth_server` | 8080 | Autenticação standalone |
| `world_server` | 8081 | Mundo global standalone |
| `zone_server` | 8082+ | Zona standalone (aceita ID) |
| `chat_server` | 8083 | Chat standalone |
| `gateway_server` | 9000 | Gateway standalone |

## Portas de Rede

```
8080  → Auth Server (REST API)
8081  → World Server (gRPC)
8082  → Zone Server #1 (WebSocket/UDP)
8083  → Zone Server #2 (WebSocket/UDP)
...
8089  → Chat Server (WebSocket)
9000  → Gateway Server (WebSocket/TCP)
3306  → MySQL Database
6379  → Redis Cache (opcional)
```

## Fluxo de Build

```bash
# 1. Preparação
git clone <repo>
git submodule update --init --recursive

# 2. Dependências
# Instalar MySQL, compilador C++17

# 3. Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# 4. Configuração
cp ../config/server.json.example ../config/server.json
cp ../config/db.json.example ../config/db.json

# 5. Database
mysql -u root -p < ../scripts/migrate_db.sql

# 6. Execução
./bin/umbra_server
# ou
../scripts/start.sh
```

## Tecnologias Utilizadas

### Backend
- **Linguagem**: C++17
- **Build**: CMake 3.20+
- **Logging**: spdlog
- **JSON**: nlohmann/json
- **JWT**: jwt-cpp
- **Testing**: Google Test
- **DB**: MySQL 8.0+ com InnoDB

### Network
- **Protocols**: TCP, UDP, WebSocket, HTTP/REST
- **Library**: ASIO (Boost.Asio standalone)
- **Serialization**: JSON, Protobuf (planejado)

### Database
- **Primary**: MySQL 8.0+
- **Cache**: Redis 6.0+ (opcional)
- **ORM**: Custom DAOs (raw SQL)

## Integração

### Cliente UE5
```cpp
// Autenticação via VaRestX
POST http://localhost:8080/auth/login

// Conexão WebSocket ao Zone Server
ws://localhost:8082

// Envio de posição (UDP planejado)
Binary packet com posição serializada
```

### Comunicação Entre Servidores
```
Auth ←→ Database (MySQL)
Zone ←→ World (gRPC planejado)
Chat ←→ Redis (pub/sub)
Gateway ←→ All (Load balancing)
```

## Próximos Passos

1. **Implementar**: Protobuf para serialização binária
2. **Adicionar**: gRPC para comunicação inter-servidor
3. **Configurar**: Redis para cache de sessões
4. **Otimizar**: Pool de threads para requisições
5. **Testar**: Load testing com 10k+ jogadores
6. **Deploy**: Docker + Kubernetes

---

**Versão**: 1.3.0  
**Data**: 2025-10-12  
**Mantido por**: UmbraEternum Development Team

