# 🎮 UmbraEternum - MMORPG Multiplayer Server

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6-orange.svg)](https://www.unrealengine.com/)
[![MySQL](https://img.shields.io/badge/MySQL-8.0+-blue.svg)](https://www.mysql.com/)
[![PHP](https://img.shields.io/badge/PHP-8.0+-purple.svg)](https://www.php.net/)

**UmbraEternum** é um servidor MMORPG multiplayer desenvolvido com **Unreal Engine 5**, **C++**, **PHP** e **MySQL**. O projeto implementa sistemas completos de autenticação, gerenciamento de personagens, sincronização de movimento em tempo real via WebSocket e persistência de dados.

---

## 📋 **ÍNDICE**

- [Visão Geral](#-visão-geral)
- [Quick Start](#-quick-start)
- [Arquitetura](#-arquitetura)
  - [Stack de Servidores C++](#stack-de-servidores-c)
  - [Diagrama de Arquitetura Completo](#diagrama-de-arquitetura-completo)
- [Configuração e Setup](#-configuração-e-setup)
  - [Configuração do Banco de Dados](#1-configuração-do-banco-de-dados)
  - [Configuração do Servidor C++](#2-configuração-do-servidor-c)
  - [Configuração da API PHP](#3-configuração-da-api-php)
  - [Configuração do Cliente Unreal](#4-configuração-do-cliente-unreal)
- [Comandos Essenciais](#-comandos-essenciais)
  - [Compilação do Servidor C++](#compilação-do-servidor-c)
  - [Compilação Unreal](#compilação-unreal)
- [Roadmap](#-roadmap)
- [Troubleshooting](#-troubleshooting)
  - [Problema: Servidor C++ não inicia](#problema-servidor-c-não-inicia)
- [Comandos de Suporte](#-comandos-de-suporte)
  - [Comandos do Servidor C++](#comandos-do-servidor-c)
- [Contribuindo](#-contribuindo)
- [Licença](#-licença)

---

## 🎯 **VISÃO GERAL**

### **O que é o UmbraEternum?**

UmbraEternum é uma plataforma de servidor MMORPG que permite múltiplos jogadores se conectarem, criarem personagens, explorarem um mundo compartilhado e interagirem em tempo real. O projeto combina:

- **Cliente Unreal Engine 5** (C++) - Engine de jogo com gráficos avançados
- **Servidor C++** - Servidor de alta performance para autenticação e sincronização
- **API PHP REST** - Backend para autenticação e gerenciamento de dados
- **MySQL Database** - Persistência de dados de jogadores e personagens
- **WebSocket Binário** - Sincronização de movimento em tempo real

### **Status do Projeto**

✅ **Sistemas Funcionais:**
- Autenticação (registro, login, validação de token)
- Gerenciamento de personagens (CRUD completo)
- Sincronização de movimento multiplayer (WebSocket binário)
- Persistência de posição e stats
- Sistema de câmera (zoom, rotação automática)

⚠️ **Em Desenvolvimento:**
- Sistema de inventário (estrutura de dados pronta, falta implementação)

❌ **Planejado:**
- Sistema de zonas
- Combate em rede
- Sistema de chat

### **Tecnologias Utilizadas**

| Componente | Tecnologia | Versão |
|------------|-----------|--------|
| Game Engine | Unreal Engine 5 | 5.6+ |
| Linguagem Cliente | C++ | C++17 |
| Servidor C++ | C++ | C++17 |
| Build System | CMake | 3.15+ |
| Backend API | PHP | 8.0+ |
| Banco de Dados | MySQL/MariaDB | 8.0+ / 11.5+ |
| WebSocket | Custom Binary Protocol | - |
| HTTP Client | VaRest Plugin | - |
| JSON Library | nlohmann/json | - |
| Logging | spdlog | - |

---

## 🚀 **QUICK START**

### **Pré-requisitos**

1. **Unreal Engine 5.6+** instalado
2. **Visual Studio 2022** (Community ou superior) com:
   - Desktop development with C++
   - Game development with C++
3. **WAMP Server** (ou XAMPP) com:
   - PHP 8.0+
   - MySQL/MariaDB 8.0+
   - Apache
4. **Git** para clonar o repositório

### **Instalação Rápida (5 minutos)**

#### **1. Clonar Repositório**

```bash
git clone https://github.com/Dev-ElJeffo/UmbraServerV2.git
cd UmbraServerV2
git submodule update --init --recursive
```

#### **2. Configurar Banco de Dados**

```bash
# Windows (PowerShell)
cd UmbraServer
.\scripts_main\start_mysql.bat

# Abrir phpMyAdmin: http://localhost/phpmyadmin
# Executar: scripts_main\setup_database.sql
```

#### **3. Configurar API PHP**

```bash
# Copiar arquivos PHP para WAMP
# Copiar www/umbra_api para C:\wamp64\www\umbra_api

# Configurar banco de dados
# Editar: C:\wamp64\www\umbra_api\config\db.json
{
  "host": "localhost",
  "port": 3306,
  "database": "umbra_eternum",
  "username": "root",
  "password": ""
}
```

#### **4. Compilar Projeto Unreal**

```bash
cd UmbraEternumUE
# Abrir UmbraEternumUE.uproject
# Unreal Editor irá gerar arquivos do Visual Studio automaticamente
# Compilar no Visual Studio (Build > Build Solution)
```

#### **5. Compilar e Executar Servidor C++ (Opcional)**

```bash
# Compilar servidor C++
cd UmbraServer
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# Executar servidor
.\Release\UmbraServer.exe --config ..\config\server.json
```

#### **6. Executar**

1. **Iniciar MySQL:**
   ```bash
   .\scripts_main\start_mysql.bat
   ```

2. **Iniciar Servidor C++ (Opcional):**
   ```bash
   cd UmbraServer\build\Release
   .\UmbraServer.exe --config ..\..\config\server.json
   ```

3. **Abrir Unreal Editor:**
   - Abrir `UmbraEternumUE.uproject`
   - Pressionar `Play` (ou `PIE` para múltiplos clients)

4. **Testar:**
   - Criar conta no menu de login
   - Criar personagem
   - Entrar no jogo
   - Mover o personagem (posição será salva automaticamente)

---

## 🏗️ **ARQUITETURA**

### **Stack de Servidores C++**

O projeto inclui um **servidor C++ de alta performance** que complementa a API PHP, oferecendo:

- **Autenticação TCP** - Validação de tokens e sessões
- **WebSocket Server** - Sincronização de movimento em tempo real
- **Socket Server** - Base TCP/UDP para comunicação customizada
- **Database Layer** - Acesso direto ao MySQL via C++
- **Logging System** - Sistema de logs estruturado (spdlog)
- **Thread Pool** - Gerenciamento eficiente de múltiplas conexões

#### **Componentes do Servidor C++**

| Componente | Responsabilidade | Localização |
|------------|------------------|-------------|
| `SocketServer` | Servidor base TCP/UDP | `src/network/SocketServer.hpp` |
| `WebSocketServer` | Servidor WebSocket | `src/network/WebSocketServer.hpp` |
| `AuthServer` | Servidor de autenticação | `src/auth/AuthServer.hpp` |
| `MySQLConnector` | Conexão com banco de dados | `src/database/MySQLConnector.hpp` |
| `Logger` | Sistema de logging | `src/core/Logger.hpp` |
| `JWTManager` | Gerenciamento de tokens JWT | `src/auth/JWTManager.hpp` |
| `SessionManager` | Gerenciamento de sessões | `src/auth/SessionManager.hpp` |

#### **Arquitetura do Servidor C++**

```
┌─────────────────────────────────────────────────────────────┐
│                    C++ SERVER STACK                         │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │              AuthServer (Port 9000)                  │  │
│  │  • Validação de tokens JWT                          │  │
│  │  • Gerenciamento de sessões                         │  │
│  │  • Autenticação TCP                                 │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         SocketServer (TCP/UDP Base)                  │  │
│  │  • Aceita conexões                                   │  │
│  │  • Thread pool para workers                         │  │
│  │  • Rate limiting                                    │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         WebSocketServer (Port 8082/8083)             │  │
│  │  • Handshake WebSocket                               │  │
│  │  • Broadcast de mensagens                           │  │
│  │  • Gerenciamento de clientes                        │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         Database Layer                               │  │
│  │  • MySQLConnector                                    │  │
│  │  • AccountDAO / PlayerDAO                           │  │
│  │  • Connection pooling                               │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         Core Utilities                              │  │
│  │  • Logger (spdlog)                                  │  │
│  │  • Utils (helpers)                                 │  │
│  │  • Config management                               │  │
│  └─────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

#### **Compilação do Servidor C++**

```bash
# Criar diretório de build
mkdir build
cd build

# Configurar CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compilar
cmake --build . --config Release

# Executar
./UmbraServer.exe  # Windows
./UmbraServer      # Linux
```

#### **Configuração do Servidor C++**

**Arquivo: `config/server.json`**

```json
{
  "auth": {
    "port": 9000,
    "jwt_secret": "sua_chave_secreta_aqui",
    "token_expiry": 3600
  },
  "websocket": {
    "port": 8082,
    "zone": "Tutorial"
  },
  "database": {
    "host": "localhost",
    "port": 3306,
    "database": "umbra_eternum",
    "username": "root",
    "password": ""
  },
  "logging": {
    "level": "info",
    "file": "logs/server.log"
  }
}
```

#### **Executando o Servidor C++**

```bash
# Modo desenvolvimento (com logs detalhados)
./UmbraServer.exe --config config/server.json --log-level debug

# Modo produção
./UmbraServer.exe --config config/server.json --log-level info

# Com validação de banco de dados
./UmbraServer.exe --config config/server.json --validate-db
```

#### **Integração com Cliente Unreal**

O cliente Unreal pode se conectar ao servidor C++ via:

1. **TCP para Autenticação:**
   ```cpp
   // No UmbraGameInstance
   bool ConnectToGatewayTCP();
   bool ValidateCurrentTokenViaTCP();
   ```

2. **WebSocket para Movimento:**
   ```cpp
   // URL do WebSocket obtida do servidor C++
   FString GetZoneServerWebSocketURL() const;
   // Retorna: ws://127.0.0.1:8082 ou ws://127.0.0.1:8083
   ```

---

### **Diagrama de Arquitetura Completo**

```
┌─────────────────────────────────────────────────────────────┐
│                    UNREAL ENGINE 5 CLIENT                  │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │           UmbraGameInstance (C++)                    │  │
│  │  • Autenticação                                      │  │
│  │  • Gerenciamento de Personagens                      │  │
│  │  • Comunicação HTTP (VaRest)                         │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │        ANetMovementClient (C++)                      │  │
│  │  • Gerenciamento de WebSocket                        │  │
│  │  • Sincronização de Remote Actors                    │  │
│  │  • Cleanup automático                                │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         UmbraWSClient (C++)                           │  │
│  │  • Cliente WebSocket                                  │  │
│  │  • Envio/Recebimento de mensagens binárias           │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │         WSBinaryBPFL (C++)                            │  │
│  │  • Serialização binária (25/34 bytes)                │  │
│  │  • Parsing de frames                                 │  │
│  │  • Interpolação de estados                           │  │
│  └──────────────────────────────────────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            │ HTTP (REST API)
                            │ TCP (Auth)
                            │ WebSocket (Binary)
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              SERVIDORES (PHP + C++)                          │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │  PHP API SERVER (HTTP REST)                         │  │
│  │  • POST /api/auth/register                           │  │
│  │  • POST /api/auth/login                              │  │
│  │  • GET  /api/character/list                          │  │
│  │  • POST /api/character/create                        │  │
│  │  • POST /api/character/select                        │  │
│  │  • POST /api/character/update_position                │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │  C++ AUTH SERVER (TCP Port 9000)                     │  │
│  │  • Validação de tokens JWT                           │  │
│  │  • Gerenciamento de sessões                          │  │
│  │  • Autenticação rápida                               │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │  C++ WEBSOCKET SERVER (Port 8082/8083)               │  │
│  │  • Sincronização de movimento                        │  │
│  │  • Broadcast de updates                              │  │
│  │  • Gerenciamento de clientes                         │  │
│  └───────────────────┬─────────────────────────────────┘  │
│                      │                                      │
│  ┌───────────────────▼─────────────────────────────────┐  │
│  │  DATABASE LAYER (C++ + PHP)                          │  │
│  │  • MySQLConnector (C++)                              │  │
│  │  • PDO Connection (PHP)                              │  │
│  │  • Connection pooling                                │  │
│  └───────────────────┬─────────────────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            │ SQL
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    MYSQL DATABASE                            │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │  accounts                                           │  │
│  │  • id, username, email, password_hash, salt         │  │
│  │  • banned, ban_reason, created_at, last_login_at    │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │  players                                             │  │
│  │  • id, account_id, character_name, level, exp       │  │
│  │  • pos_x, pos_y, pos_z, current_zone               │  │
│  │  • health, mana, stamina, stats                     │  │
│  └─────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │  (items) - PLANEJADO                                │  │
│  │  (player_inventory) - PLANEJADO                     │  │
│  └─────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### **Componentes Principais**

#### **Cliente Unreal (C++)**

| Classe | Responsabilidade |
|--------|------------------|
| `UUmbraGameInstance` | Gerenciamento global: autenticação, personagens, estado do jogo |
| `ANetMovementClient` | Gerenciamento de conexão WebSocket e remote actors |
| `UUmbraWSClient` | Cliente WebSocket de baixo nível |
| `UWSBinaryBPFL` | Serialização/deserialização de frames binários |
| `AUmbraEternumUECharacter` | Personagem do jogador com controles de câmera |

#### **Servidor C++**

| Componente | Responsabilidade |
|------------|------------------|
| `AuthServer` | Servidor de autenticação TCP (porta 9000) |
| `WebSocketServer` | Servidor WebSocket para sincronização (porta 8082/8083) |
| `SocketServer` | Servidor base TCP/UDP com thread pool |
| `MySQLConnector` | Conexão e pool de conexões MySQL |
| `JWTManager` | Gerenciamento de tokens JWT |
| `SessionManager` | Gerenciamento de sessões de jogadores |
| `Logger` | Sistema de logging estruturado (spdlog) |

#### **Servidor PHP**

| Componente | Responsabilidade |
|------------|------------------|
| `api/auth/register.php` | Registro de novos usuários |
| `api/auth/login.php` | Autenticação e geração de JWT |
| `api/character/*.php` | CRUD de personagens |
| `api/character/update_position.php` | Persistência de posição |
| `config/db.json` | Configuração do banco de dados |

#### **Banco de Dados**

| Tabela | Descrição |
|--------|-----------|
| `accounts` | Contas de usuário |
| `players` | Personagens dos jogadores |
| `schema_version` | Controle de versão do schema |

---

## ⚙️ **CONFIGURAÇÃO E SETUP**

### **1. Configuração do Banco de Dados**

#### **Criar Banco de Dados**

```sql
CREATE DATABASE umbra_eternum
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;
```

#### **Executar Script de Setup**

```bash
# Via phpMyAdmin ou MySQL Workbench
# Executar: scripts_main/setup_database.sql
```

#### **Verificar Configuração**

```sql
USE umbra_eternum;
SHOW TABLES;
SELECT * FROM schema_version;
```

### **2. Configuração do Servidor C++**

#### **Arquivo: `config/server.json`**

```json
{
  "auth": {
    "port": 9000,
    "jwt_secret": "sua_chave_secreta_jwt_aqui_32_chars",
    "token_expiry": 3600
  },
  "websocket": {
    "port": 8082,
    "zone": "Tutorial",
    "max_clients": 100
  },
  "database": {
    "host": "localhost",
    "port": 3306,
    "database": "umbra_eternum",
    "username": "root",
    "password": "",
    "pool_size": 10
  },
  "logging": {
    "level": "info",
    "file": "logs/server.log",
    "console": true
  }
}
```

#### **Dependências do Servidor C++**

O servidor C++ requer as seguintes bibliotecas (gerenciadas via CMake):

- **spdlog** - Sistema de logging (incluído como submodule)
- **nlohmann/json** - Parsing de JSON (incluído como submodule)
- **MySQL Connector/C++** - Cliente MySQL (ou MariaDB Connector)
- **OpenSSL** - Para JWT e criptografia (opcional)

### **3. Configuração da API PHP**

#### **Arquivo: `www/umbra_api/config/db.json`**

```json
{
  "host": "localhost",
  "port": 3306,
  "database": "umbra_eternum",
  "username": "root",
  "password": ""
}
```

#### **Arquivo: `www/umbra_api/config/jwt_secret.key`**

```bash
# Gerar chave secreta JWT (32 caracteres)
# Exemplo: openssl rand -hex 16
```

### **4. Configuração do Cliente Unreal**

#### **Arquivo: `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h`**

```cpp
// Configurar no Blueprint ou no Editor
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
FString ServerURL = TEXT("http://localhost/umbra_api");

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
FString GameServerIP = TEXT("127.0.0.1");

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
int32 GameServerPort = 9000;
```

### **5. Configuração do MySQL (Auto-start)**

#### **Windows (PowerShell como Admin)**

```powershell
# Verificar status
Get-Service MySQL80

# Configurar auto-start
Set-Service MySQL80 -StartupType Automatic

# Iniciar serviço
Start-Service MySQL80
```

#### **Script Automático**

```bash
.\scripts_main\configure_mysql_autostart.ps1
```

---

## 🛠️ **COMANDOS ESSENCIAIS**

### **Git**

```bash
# Clonar repositório
git clone https://github.com/Dev-ElJeffo/UmbraServerV2.git
cd UmbraServerV2

# Inicializar submódulos
git submodule update --init --recursive

# Atualizar submódulos
cd UmbraEternumUE
git pull origin develop
cd ../UmbraServer
git add UmbraEternumUE
git commit -m "Atualizar submódulo"
git push origin Development
```

### **MySQL**

```bash
# Iniciar MySQL
.\scripts_main\start_mysql.bat

# Verificar status
sc query MySQL80

# Parar MySQL
net stop MySQL80
```

### **Compilação do Servidor C++**

```bash
# Criar diretório de build
cd UmbraServer
mkdir build
cd build

# Configurar CMake (Windows)
cmake .. -G "Visual Studio 17 2022" -A x64

# Ou (Linux/Mac)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compilar
cmake --build . --config Release

# Executável estará em: build/Release/UmbraServer.exe (Windows)
#                      build/UmbraServer (Linux/Mac)
```

### **Compilação Unreal**

```bash
# Gerar arquivos do Visual Studio
# Abrir UmbraEternumUE.uproject no Unreal Editor
# File > Refresh Visual Studio Project

# Compilar no Visual Studio
# Build > Build Solution (F7)

# Ou via linha de comando
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" UmbraEternumUEEditor Win64 Development -Project="D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.uproject"
```

### **Limpeza de Projeto**

```bash
# Limpar arquivos intermediários
cd UmbraEternumUE
.\CleanProject.bat

# Limpeza completa
.\FORCE_CLEAN_AND_REBUILD.bat
```

---

## 🗺️ **ROADMAP**

### **✅ FASE 1: Fundação (COMPLETA)**
- [x] Sistema de autenticação
- [x] Gerenciamento de personagens
- [x] Sincronização de movimento multiplayer
- [x] Persistência de dados
- [x] Sistema de câmera

### **🚧 FASE 2: Inventário (EM DESENVOLVIMENTO)**
- [ ] Tabelas de inventário no banco de dados
- [ ] API PHP para inventário
- [ ] Estruturas C++ de inventário
- [ ] UI de inventário no Unreal
- [ ] Sistema de equipamento

### **📅 FASE 3: Zonas (PLANEJADO)**
- [ ] Sistema de múltiplas zonas
- [ ] Transição entre zonas
- [ ] Sincronização por zona

### **📅 FASE 4: Combate (PLANEJADO)**
- [ ] Sincronização de combate em rede
- [ ] Sistema de dano
- [ ] Sistema de habilidades

### **📅 FASE 5: Features Avançadas (PLANEJADO)**
- [ ] Sistema de chat
- [ ] Sistema de quests
- [ ] Sistema de crafting
- [ ] Sistema de guildas

---

## 🔧 **TROUBLESHOOTING**

### **Problema: MySQL não inicia**

**Sintomas:**
```
Falha ao iniciar MySQL
```

**Soluções:**
1. Verificar se o serviço está rodando:
   ```powershell
   Get-Service MySQL80
   ```

2. Iniciar manualmente:
   ```powershell
   Start-Service MySQL80
   ```

3. Verificar porta 3306:
   ```powershell
   netstat -ano | findstr :3306
   ```

4. Configurar auto-start:
   ```powershell
   Set-Service MySQL80 -StartupType Automatic
   ```

**Documentação:** `docs_main/CORRECAO_MYSQL_NAO_INICIA_AUTO.md`

---

### **Problema: Servidor C++ não inicia**

**Sintomas:**
```
Failed to start server
Port already in use
Database connection failed
```

**Soluções:**
1. Verificar se a porta está em uso:
   ```powershell
   netstat -ano | findstr :9000  # Auth Server
   netstat -ano | findstr :8082  # WebSocket Server
   ```

2. Verificar configuração:
   ```bash
   # Verificar config/server.json
   cat config/server.json
   # Ou no Windows:
   type config\server.json
   ```

3. Verificar logs:
   ```bash
   # Logs do servidor
   tail -f logs/server.log
   # Ou no Windows PowerShell:
   Get-Content logs\server.log -Wait -Tail 50
   ```

4. Verificar banco de dados:
   ```powershell
   # Servidor deve conectar ao MySQL
   # Verificar se MySQL está rodando
   Get-Service MySQL80
   ```

5. Verificar dependências:
   ```bash
   # Verificar se bibliotecas estão instaladas
   # spdlog, nlohmann/json devem estar em third_party/
   ```

**Documentação:** Ver logs em `logs/server.log`

---

### **Problema: Cliente não conecta ao WebSocket**

**Sintomas:**
```
WebSocket connection failed
OnWSConnected não dispara
```

**Soluções:**
1. Verificar se o servidor C++ WebSocket está rodando:
   ```powershell
   # Verificar processo
   Get-Process | Where-Object {$_.ProcessName -like "*UmbraServer*"}
   
   # Verificar porta
   netstat -ano | findstr :8082
   ```

2. Verificar URL do WebSocket:
   ```cpp
   // No UmbraGameInstance
   FString GetZoneServerWebSocketURL() const;
   // Deve retornar: ws://127.0.0.1:8082
   ```

3. Verificar logs do servidor C++:
   ```powershell
   # Ver logs em tempo real
   Get-Content logs\server.log -Wait -Tail 50
   ```

4. Verificar logs no Output Log do Unreal:
   ```
   LogTemp: [NetMovementClient] WebSocket Connected!
   ```

5. Verificar se `ConnectWebSocketManual()` está sendo chamado no Blueprint

**Documentação:** `docs_main/CORRECAO_BLUEPRINT_NAO_CONECTA_WEBSOCKET.md`

---

### **Problema: Remote actors não aparecem**

**Sintomas:**
```
Outros jogadores não aparecem no mundo
```

**Soluções:**
1. Verificar se `OnWSBinaryMessage` está processando frames:
   ```cpp
   // Logs devem aparecer:
   LogTemp: [ProcessBinaryBuffer] Frame aceito
   ```

2. Verificar se `RemoteActors` array está sendo populado

3. Verificar se `BP_RemotePlayer` está sendo spawnado corretamente

4. Verificar posição dos remote actors (pode estar invertida)

**Documentação:** `docs_main/DIAGNOSTICO_DIRECAO_INCORRETA_REMOTE_ACTORS.md`

---

### **Problema: Compilação falha**

**Sintomas:**
```
Error: Failed (OtherCompilationError)
```

**Soluções:**
1. Limpar projeto:
   ```bash
   .\CleanProject.bat
   ```

2. Regenerar arquivos do Visual Studio:
   - Abrir `UmbraEternumUE.uproject`
   - File > Refresh Visual Studio Project

3. Verificar dependências:
   - VaRest Plugin instalado?
   - WebSocket Plugin instalado?

4. Recompilar:
   ```bash
   .\FORCE_CLEAN_AND_REBUILD.bat
   ```

**Documentação:** `docs_main/FIX_BUILD_ERRORS.md`

---

### **Problema: Personagem não salva posição**

**Sintomas:**
```
Posição sempre volta para (0, 0, 0)
```

**Soluções:**
1. Verificar se `SavePlayerPosition` está sendo chamado:
   ```cpp
   // No Blueprint ou C++
   GameInstance->SavePlayerPosition(PlayerID, Location);
   ```

2. Verificar API PHP:
   ```bash
   # Testar endpoint
   curl -X POST http://localhost/umbra_api/api/character/update_position.php
   ```

3. Verificar banco de dados:
   ```sql
   SELECT pos_x, pos_y, pos_z FROM players WHERE id = ?;
   ```

**Documentação:** `docs_main/GUIA_PRATICO_SALVAR_POSICAO.md`

---

## 📞 **COMANDOS DE SUPORTE**

### **Verificar Status do Sistema**

```powershell
# MySQL
Get-Service MySQL80 | Select-Object Status, StartType

# Servidor C++
Get-Process | Where-Object {$_.ProcessName -like "*UmbraServer*"}

# Verificar processos Unreal
Get-Process | Where-Object {$_.ProcessName -like "*Unreal*"}

# Verificar portas
netstat -ano | findstr :9000   # Auth Server
netstat -ano | findstr :8082   # WebSocket Server
netstat -ano | findstr :3306   # MySQL
```

### **Comandos do Servidor C++**

```bash
# Iniciar servidor
.\UmbraServer.exe --config config/server.json

# Modo debug (logs detalhados)
.\UmbraServer.exe --config config/server.json --log-level debug

# Validar configuração
.\UmbraServer.exe --config config/server.json --validate-config

# Validar banco de dados
.\UmbraServer.exe --config config/server.json --validate-db

# Ver ajuda
.\UmbraServer.exe --help

# Verificar versão
.\UmbraServer.exe --version
```

### **Logs e Debug**

#### **Unreal Engine**
- **Output Log:** `Window > Developer Tools > Output Log`
- **Logs C++:** `UE_LOG(LogTemp, Warning, TEXT("Mensagem"));`
- **Logs Blueprint:** `Print String` node

#### **PHP API**
- **Logs:** `C:\wamp64\logs\php_error.log`
- **Apache Logs:** `C:\wamp64\logs\apache_error.log`

#### **Servidor C++**
- **Logs:** `UmbraServer/logs/server.log`
- **Níveis de log:** `debug`, `info`, `warn`, `error`
- **Formato:** Estruturado (spdlog) com timestamp e nível

#### **MySQL**
- **Logs:** `C:\wamp64\bin\mysql\mysql8.x.x\data\*.err`

### **Testar API**

```bash
# Testar login
curl -X POST http://localhost/umbra_api/api/auth/login.php \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"test\",\"password\":\"test123\"}"

# Testar listagem de personagens
curl -X GET http://localhost/umbra_api/api/character/list.php \
  -H "Authorization: Bearer TOKEN_AQUI"
```

### **Testar Servidor C++**

```bash
# Testar conexão TCP (Auth Server)
# Usar telnet ou netcat
telnet localhost 9000

# Testar WebSocket (WebSocket Server)
# Usar ferramenta como wscat ou Postman
wscat -c ws://localhost:8082

# Verificar status do servidor
# Logs devem mostrar:
# [INFO] AuthServer started on port 9000
# [INFO] WebSocketServer started on port 8082
```

### **Resetar Banco de Dados**

```sql
-- CUIDADO: Isso apaga todos os dados!
DROP DATABASE umbra_eternum;
CREATE DATABASE umbra_eternum;
-- Executar setup_database.sql novamente
```

---

## 🤝 **CONTRIBUINDO**

### **Estrutura de Branches**

- `Development` - Branch principal de desenvolvimento
- `develop` (submódulo) - Branch do cliente Unreal

### **Fluxo de Trabalho**

1. Criar branch a partir de `Development`
2. Fazer alterações
3. Commitar com mensagens descritivas
4. Push para branch
5. Criar Pull Request (se aplicável)

### **Padrões de Código**

- **C++:** Seguir padrões do Unreal Engine (UPROPERTY, UFUNCTION, etc.)
- **PHP:** PSR-12
- **SQL:** snake_case para nomes de tabelas/colunas
- **Commits:** Mensagens em português, descritivas

---

## 📚 **DOCUMENTAÇÃO ADICIONAL**

### **Guias Principais**

- `docs_main/ANALISE_COMPLETA_ESTADO_PROJETO.md` - Análise completa do estado atual
- `docs/DATABASE_SETUP.md` - Setup detalhado do banco de dados
- `docs/BUILD_INSTRUCTIONS.md` - Instruções de compilação

### **Guias de Troubleshooting**

- `docs_main/CORRECAO_MYSQL_NAO_INICIA_AUTO.md` - Problemas com MySQL
- `docs_main/CORRECAO_BLUEPRINT_NAO_CONECTA_WEBSOCKET.md` - Problemas de conexão
- `docs_main/DIAGNOSTICO_DIRECAO_INCORRETA_REMOTE_ACTORS.md` - Problemas de sincronização

### **Guias de Implementação**

- `docs_main/GUIA_COMPLETO_TOGGLE_CAMERA_MODE.md` - Sistema de câmera
- `docs_main/GUIA_COMPLETO_INTEGRACAO_NETMOVEMENTCLIENT.md` - Integração de rede

---

## 📄 **LICENÇA**

Este projeto está sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.

---

## 👥 **AUTORES**

- **Dev-ElJeffo** - Desenvolvimento principal

---

## 🔗 **LINKS ÚTEIS**

- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [VaRest Plugin](https://github.com/ufna/VaRest)
- [MySQL Documentation](https://dev.mysql.com/doc/)
- [PHP Documentation](https://www.php.net/docs.php)

---

## 📝 **CHANGELOG**

### **v1.0.0 (2025-01-XX)**
- ✅ Sistema de autenticação completo
- ✅ Gerenciamento de personagens
- ✅ Sincronização de movimento multiplayer
- ✅ Persistência de dados
- ✅ Sistema de câmera (zoom, rotação automática)

---

**Última atualização:** 2025-01-XX  
**Versão do README:** 1.0

