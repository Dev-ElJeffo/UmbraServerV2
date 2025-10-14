# Guia Rápido - UmbraEternum Server

Guia para colocar o servidor rodando rapidamente.

## Pré-requisitos

### Windows

- Visual Studio 2019+ com C++17
- CMake 3.20+
- MySQL 8.0+
- Git

### Linux

```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libmysqlclient-dev
```

### macOS

```bash
brew install cmake mysql git
```

## Instalação Rápida

### 1. Clone o Repositório

```bash
git clone https://github.com/yourusername/UmbraServer.git
cd UmbraServer
```

### 2. Baixe Dependências

```bash
# Inicializar submodules
git submodule init
git submodule update

# Adicionar bibliotecas necessárias
git submodule add https://github.com/nlohmann/json.git third_party/json
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
git submodule add https://github.com/google/googletest.git third_party/googletest
```

### 3. Configure o Banco de Dados

```bash
# Criar banco de dados
mysql -u root -p < scripts/migrate_db.sql

# Ou manualmente:
mysql -u root -p
CREATE DATABASE umbra_eternum;
USE umbra_eternum;
SOURCE scripts/migrate_db.sql;
EXIT;
```

### 4. Configurar Servidor

```bash
# Copiar arquivo de exemplo
cp config/db.json.example config/db.json
cp config/jwt_secret.key.example config/jwt_secret.key

# Editar config/db.json com suas credenciais MySQL
# Editar config/jwt_secret.key com uma chave secreta
```

**Gerar chave JWT segura:**
```bash
openssl rand -base64 64 > config/jwt_secret.key
```

### 5. Build

**Windows:**
```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

**Linux/macOS:**
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 6. Executar

**Todos os servidores (recomendado para desenvolvimento):**

Windows:
```bash
cd scripts
start.bat
```

Linux/macOS:
```bash
cd scripts
chmod +x start.sh
./start.sh
```

**Ou individual:**
```bash
# Auth Server
./build/bin/auth_server

# World Server
./build/bin/world_server

# Zone Server
./build/bin/zone_server 1

# Chat Server
./build/bin/chat_server

# Gateway Server
./build/bin/gateway_server

# Servidor completo (tudo junto)
./build/bin/umbra_server
```

## Verificação

### 1. Testar Autenticação

```bash
curl -X POST http://localhost:8080/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "test@example.com",
    "password": "password123"
  }'
```

Resposta esperada:
```json
{
  "success": true,
  "message": "Account created successfully",
  "account_id": 1
}
```

### 2. Testar Login

```bash
curl -X POST http://localhost:8080/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "password123"
  }'
```

Resposta esperada:
```json
{
  "success": true,
  "message": "Login successful",
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "account_id": 1,
  "player_id": 0
}
```

### 3. Verificar Logs

```bash
tail -f logs/umbra_server.log
```

## Testes

```bash
cd build
ctest --output-on-failure
```

Para testes individuais:
```bash
./build/bin/test_auth
./build/bin/test_db
./build/bin/test_zone
./build/bin/test_network
```

## Troubleshooting

### Erro: "Failed to connect to database"

**Solução:**
1. Verifique se MySQL está rodando
2. Confirme credenciais em `config/db.json`
3. Verifique se o banco `umbra_eternum` existe

```bash
# Linux/macOS
sudo systemctl status mysql

# Windows
net start MySQL80
```

### Erro: "Port already in use"

**Solução:**
Mude as portas em `config/server.json`:
```json
{
  "auth": { "port": 8080 },
  "world": { "port": 8081 },
  "gateway": { "port": 9000 }
}
```

### Erro de Compilação: "nlohmann/json.hpp not found"

**Solução:**
```bash
git submodule update --init --recursive
```

### Logs não aparecem

**Solução:**
```bash
mkdir logs
```

## Próximos Passos

1. **Integração UE5**: Veja [docs/INTEGRATION_UE5.md](INTEGRATION_UE5.md)
2. **Arquitetura**: Veja [docs/ARCHITECTURE.md](ARCHITECTURE.md)
3. **Contribuindo**: Veja [CONTRIBUTING.md](../CONTRIBUTING.md)

## Configuração de Produção

### Segurança

1. **Mude o JWT secret:**
   ```bash
   openssl rand -base64 64 > config/jwt_secret.key
   ```

2. **Configure SSL/TLS** para comunicação HTTPS

3. **Configure firewall:**
   ```bash
   # Exemplo: ufw (Linux)
   sudo ufw allow 8080/tcp  # Auth
   sudo ufw allow 9000/tcp  # Gateway
   ```

### Performance

1. **Otimize MySQL:**
   ```sql
   SET GLOBAL max_connections = 500;
   SET GLOBAL innodb_buffer_pool_size = 2G;
   ```

2. **Configure Redis** para cache de sessões

3. **Use build Release:**
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j$(nproc)
   ```

### Monitoramento

```bash
# Logs
tail -f logs/*.log

# Processos
ps aux | grep umbra

# Portas
netstat -tuln | grep -E '8080|8081|8082|8083|9000'
```

## Suporte

- **Issues**: [GitHub Issues](https://github.com/yourusername/UmbraServer/issues)
- **Discord**: [Link do servidor]
- **Email**: support@umbraeternnum.com

---

**Parabéns!** Você tem o UmbraEternum Server rodando! 🎉

