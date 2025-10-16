# UmbraEternum Server - Arquitetura de Micro-Serviços

![Build Status](https://github.com/Dev-ElJeffo/UmbraServerV2/workflows/Build%20and%20Test/badge.svg)
![Release](https://img.shields.io/github/v/release/Dev-ElJeffo/UmbraServerV2?include_prereleases)
![License](https://img.shields.io/badge/license-Proprietary-red)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
![MySQL](https://img.shields.io/badge/MySQL-8.0%2B-blue)

Arquitetura de servidores C++17 para o MMORPG UmbraEternum com Unreal Engine 5.

## Visão Geral

Sistema de micro-serviços leves com comunicação assíncrona via gRPC, ZeroMQ e Protobuf para alta performance. Suporta escalabilidade horizontal com múltiplas instâncias de Zone Servers.

### Servidores Principais

- **Login/Auth Server**: Autenticação JWT, validação de contas, gestão de sessões
- **World Server**: Lógica global, spawn de NPCs, eventos de mundo
- **Zone Servers**: Gerenciamento de regiões específicas, movimentação, combate
- **Chat Server**: Mensagens globais, privadas e de guilda
- **Game Gateway**: Proxy de conexões, balanceamento de carga, proteção DDoS
- **Database Server**: MySQL com cache Redis

## Requisitos

- **Compilador**: GCC 9+ ou MSVC 2019+ com suporte C++17
- **CMake**: 3.20 ou superior
- **MySQL**: 8.0+ com InnoDB
- **Redis**: 6.0+ (opcional, para cache de sessões)
- **Git**: Para gerenciar submodules

## Estrutura do Projeto

```
UmbraServer/
├── src/              # Código-fonte
│   ├── core/         # Componentes base (Config, Logger, Timer)
│   ├── network/      # Camada de rede (TCP/UDP/WebSocket)
│   ├── auth/         # Servidor de autenticação
│   ├── world/        # Servidor de mundo
│   ├── zone/         # Servidores de zona
│   ├── chat/         # Servidor de chat
│   ├── gateway/      # Gateway/Proxy
│   ├── database/     # Camada de persistência
│   └── services/     # Serviços auxiliares
├── config/           # Configurações JSON
├── scripts/          # Scripts de automação
├── tests/            # Testes unitários e integração
└── third_party/      # Bibliotecas externas (submodules)
```

## Instalação e Build

### 1. Clone o repositório com submodules

```bash
git clone --recurse-submodules https://github.com/yourusername/UmbraServer.git
cd UmbraServer
```

### 2. Configure as dependências

As bibliotecas necessárias estão em `third_party/` como Git submodules:
- jwt-cpp
- nlohmann/json
- spdlog
- asio
- googletest
- mysql-connector-cpp
- protobuf

### 3. Build com CMake

**Linux/macOS:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Windows (Visual Studio):**
```bash
mkdir build && cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

### 4. Configure o banco de dados

Execute as migrations:
```bash
mysql -u root -p < scripts/migrate_db.sql
```

### 5. Configure os servidores

Edite os arquivos em `config/`:
- `server.json`: IPs, portas e configurações de rede
- `db.json`: Credenciais do MySQL/Redis
- `jwt_secret.key`: Gere uma chave secreta segura

Exemplo de `jwt_secret.key`:
```bash
openssl rand -base64 64 > config/jwt_secret.key
```

## Execução

### Iniciar todos os servidores

**Linux/macOS:**
```bash
./scripts/start.sh
```

**Windows:**
```bash
.\scripts\start.bat
```

### Iniciar servidores individualmente

```bash
./build/bin/auth_server
./build/bin/world_server
./build/bin/zone_server --zone-id 1
./build/bin/chat_server
./build/bin/gateway_server
```

## 🎮 Integração com Unreal Engine 5

### 📚 Documentação Completa

**Guia Rápido (5 minutos)**: [`UE5_QUICKSTART.md`](UE5_QUICKSTART.md)  
**Guia Completo**: [`UE5_API_INTEGRATION.md`](UE5_API_INTEGRATION.md)  
**Arquitetura Visual**: [`FULL_ARCHITECTURE.md`](FULL_ARCHITECTURE.md)  
**Código C++**: [`UE5_CPP_EXAMPLES.h`](UE5_CPP_EXAMPLES.h) / [`.cpp`](UE5_CPP_EXAMPLES.cpp)

### Stack de Integração

```
UE5 Client (VaRest/Sockets)
    ↓ HTTP/REST (Login/Register)
PHP APIs (localhost/umbra_api)
    ↓ MySQL
Database (umbra_eternum)
    ↓ Validação
C++ Servers (Auth/World/Gateway)
    ↓ TCP/UDP
Game Synchronization
```

### 1. Login Flow

O cliente UE5 usa VaRest para autenticação:
```
Cliente UE5 → (REST) → PHP API → MySQL
               ← JWT Token + Account Data ←
Cliente UE5 → (TCP) → C++ Gateway → Zone Server
```

### 2. Fluxo Completo

```
1. UE5: Widget Login → VaRest POST /api/login.php
2. PHP: Valida credenciais → Retorna token + personagens
3. UE5: Salva na Game Instance → Conecta Gateway (TCP 9000)
4. C++: Gateway valida token → Redireciona para Zone Server
5. UE5: Spawna personagem → Gameplay inicia
```

### 3. Comunicação em Tempo Real

- **HTTP/REST**: Login, registro, APIs PHP (VaRest plugin)
- **WebSocket**: Para eventos gerais e chat
- **TCP**: Para ações críticas (inventário, trade)
- **UDP**: Para movimentação (com ACK customizado)

### 📦 Sistema de Personagens

**Guia Completo Passo-a-Passo**: [`INTEGRACAO_COMPLETA_PERSONAGENS_UE5.md`](INTEGRACAO_COMPLETA_PERSONAGENS_UE5.md)  
**Referência Rápida**: [`REFERENCIA_RAPIDA_PERSONAGENS.md`](REFERENCIA_RAPIDA_PERSONAGENS.md)  
**Documentação das APIs**: [`SISTEMA_PERSONAGENS.md`](SISTEMA_PERSONAGENS.md)

#### Funcionalidades Implementadas

```
✅ Login de usuário
✅ Listagem de personagens da conta
✅ Criação de personagem (validação 3-20 chars, máx 5 por conta)
✅ Seleção de personagem para jogar
✅ Exclusão de personagem
✅ Stats completos (HP, Mana, Stamina, STR, DEX, INT, VIT)
✅ Sincronização com MySQL via PHP APIs
✅ Widgets UE5 prontos (CharacterSelection, CharacterItem, CreateCharacter)
```

#### APIs de Personagens

| **Endpoint**                          | **Método** | **Descrição**                     |
|---------------------------------------|------------|-----------------------------------|
| `/api/character/list_characters.php`  | POST       | Lista personagens da conta        |
| `/api/character/create_character.php` | POST       | Cria novo personagem              |
| `/api/character/select_character.php` | POST       | Marca personagem como ativo       |
| `/api/character/delete_character.php` | POST       | Remove personagem                 |

**Teste Online**: `http://localhost/umbra_api/test_character.html`

## Testes

### Executar testes unitários

```bash
cd build
ctest --output-on-failure
```

### Cobertura de código

```bash
./scripts/coverage.sh
# Relatório gerado em coverage/index.html
```

### Testes de integração

```bash
# Teste de autenticação
./build/bin/test_auth

# Teste de database
./build/bin/test_db

# Teste de zona
./build/bin/test_zone
```

## Desenvolvimento

### Diretrizes de Código

- **Estilo**: Google C++ Style Guide
- **Indentação**: 2 espaços
- **Limite de coluna**: 80 caracteres
- **Limite de arquivo**: 1000 linhas
- **Nomenclatura**: camelCase para métodos, PascalCase para classes

### Padrões

- Smart pointers (std::unique_ptr, std::shared_ptr)
- RAII para gerenciamento de recursos
- Retornos precoces para evitar aninhamento
- Código DRY (Don't Repeat Yourself)
- Forward declarations em headers

### Contribuindo

1. Crie um branch: `git checkout -b feature/minha-feature`
2. Faça commits seguindo Conventional Commits
3. Execute testes: `ctest`
4. Abra um Pull Request com descrição detalhada
5. Aguarde revisão de pelo menos 2 desenvolvedores

## Segurança

- ✅ JWT Tokens com HS256
- ✅ Rate-limiting no Gateway
- ✅ Server-authoritative em Zone Servers
- ✅ Prepared statements (MySQL)
- ✅ Bcrypt para senhas
- ✅ TLS 1.3 para comunicação
- ✅ Logs de auditoria para ações sensíveis

## Performance

- **Latência**: < 50ms (target)
- **Uptime**: 99.9% (target)
- **Jogadores simultâneos**: 10k+ por cluster
- **Zone Servers**: Escalabilidade horizontal

## Roadmap

### Fase 1 (Sprint 1) - ✅ Base
- [x] Estrutura de diretórios
- [x] Core components (Config, Logger)
- [x] Database layer
- [x] Auth Server

### Fase 2 (Sprint 2) - 🚧 Em Andamento
- [ ] World Server
- [ ] Zone Servers
- [ ] Gateway

### Fase 3 (Sprint 3) - 📋 Planejado
- [ ] Chat Server
- [ ] Matchmaking
- [ ] Testes de carga (10k+ players)

## Suporte

- **Documentação**: Veja `/docs` para documentação detalhada da API
- **Issues**: Reporte bugs e sugestões no GitHub Issues
- **Discord**: Entre no servidor de desenvolvimento

## Licença

Proprietary - © 2025 UmbraEternum Team

---

**Versão**: 1.3 (12/10/2025)
**Última Atualização**: Este documento é atualizado a cada sprint

