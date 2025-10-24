# Instruções de Build Detalhadas

Guia completo para compilar o UmbraEternum Server em diferentes plataformas.

## Índice

1. [Windows (Visual Studio)](#windows-visual-studio)
2. [Linux (Ubuntu/Debian)](#linux-ubuntudebian)
3. [Linux (CentOS/RHEL)](#linux-centosrhel)
4. [macOS](#macos)
5. [Troubleshooting](#troubleshooting)

---

## Windows (Visual Studio)

### Pré-requisitos

1. **Visual Studio 2019 ou superior**
   - Workload: "Desktop development with C++"
   - Componentes: CMake tools, C++17 support

2. **CMake 3.20+**
   - Download: https://cmake.org/download/
   - Adicione ao PATH durante instalação

3. **MySQL 8.0+**
   - Download: https://dev.mysql.com/downloads/installer/
   - Instale MySQL Server e MySQL Workbench

4. **Git for Windows**
   - Download: https://git-scm.com/download/win

### Passos

#### 1. Clone o Repositório

```powershell
git clone https://github.com/yourusername/UmbraServer.git
cd UmbraServer
```

#### 2. Inicialize Submodules

```powershell
git submodule update --init --recursive
```

#### 3. Adicione Bibliotecas

```powershell
git submodule add https://github.com/nlohmann/json.git third_party/json
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
git submodule add https://github.com/google/googletest.git third_party/googletest
```

#### 4. MySQL Connector (Manual)

1. Download: https://dev.mysql.com/downloads/connector/cpp/
2. Escolha: "Windows (x86, 64-bit), ZIP Archive"
3. Extraia para `third_party/mysql-connector-cpp/`

#### 5. Configure com CMake

**Opção A: CMake GUI**
1. Abra CMake GUI
2. Source: `D:/UmbraServerV2/UmbraServer`
3. Build: `D:/UmbraServerV2/UmbraServer/build`
4. Click "Configure" → Selecione "Visual Studio 16 2019" → "x64"
5. Click "Generate"

**Opção B: Command Line**
```powershell
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
```

#### 6. Build

**Visual Studio:**
```powershell
# Abrir solução
start UmbraServer.sln

# Ou build via command line
cmake --build . --config Release --parallel
```

**MSBuild direto:**
```powershell
msbuild UmbraServer.sln /p:Configuration=Release /m
```

#### 7. Executáveis

Estarão em:
```
build/bin/Release/
  - umbra_server.exe
  - auth_server.exe
  - world_server.exe
  - zone_server.exe
  - chat_server.exe
  - gateway_server.exe
```

---

## Linux (Ubuntu/Debian)

### Pré-requisitos

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  git \
  libmysqlclient-dev \
  libssl-dev \
  pkg-config
```

### Passos

#### 1. Clone e Submodules

```bash
git clone https://github.com/yourusername/UmbraServer.git
cd UmbraServer
git submodule update --init --recursive
```

#### 2. Adicione Bibliotecas

```bash
git submodule add https://github.com/nlohmann/json.git third_party/json
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
git submodule add https://github.com/google/googletest.git third_party/googletest
```

#### 3. MySQL Connector

```bash
# Opção A: Via apt (recomendado)
sudo apt install libmysqlcppconn-dev

# Opção B: Manual
cd third_party
wget https://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-8.0.33-linux-glibc2.28-x86-64bit.tar.gz
tar -xzf mysql-connector-c++-8.0.33-linux-glibc2.28-x86-64bit.tar.gz
mv mysql-connector-c++-8.0.33-linux-glibc2.28-x86-64bit mysql-connector-cpp
cd ..
```

#### 4. Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### 5. Install (Opcional)

```bash
sudo make install
```

#### 6. Executáveis

```bash
ls -lh bin/
  - umbra_server
  - auth_server
  - world_server
  - zone_server
  - chat_server
  - gateway_server
```

---

## Linux (CentOS/RHEL)

### Pré-requisitos

```bash
# CentOS 8+
sudo dnf install -y \
  gcc-c++ \
  cmake \
  git \
  mysql-devel \
  openssl-devel

# CentOS 7
sudo yum install -y centos-release-scl
sudo yum install -y \
  devtoolset-9-gcc-c++ \
  cmake3 \
  git \
  mysql-devel \
  openssl-devel

# Enable devtoolset
scl enable devtoolset-9 bash
```

### Build

```bash
# CentOS 8+
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# CentOS 7 (use cmake3)
mkdir build && cd build
cmake3 -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

---

## macOS

### Pré-requisitos

```bash
# Instalar Homebrew (se ainda não tiver)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Instalar dependências
brew install cmake git mysql-client openssl
```

### Passos

#### 1. Clone e Submodules

```bash
git clone https://github.com/yourusername/UmbraServer.git
cd UmbraServer
git submodule update --init --recursive
```

#### 2. Adicione Bibliotecas

```bash
git submodule add https://github.com/nlohmann/json.git third_party/json
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
git submodule add https://github.com/google/googletest.git third_party/googletest
```

#### 3. MySQL Connector

```bash
brew install mysql-connector-c++
```

#### 4. Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
  -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) \
  -DMYSQL_ROOT_DIR=$(brew --prefix mysql-client) ..
make -j$(sysctl -n hw.ncpu)
```

#### 5. Executáveis

```bash
ls -lh bin/
```

---

## Opções de Build Avançadas

### Build Types

```bash
# Debug (com símbolos de debug)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release (otimizado)
cmake -DCMAKE_BUILD_TYPE=Release ..

# RelWithDebInfo (otimizado + debug info)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# MinSizeRel (otimizado para tamanho)
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
```

### Opções Customizadas

```bash
# Disable testes
cmake -DBUILD_TESTING=OFF ..

# Especificar compilador
cmake -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11 ..

# Especificar flags customizadas
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Build estático
cmake -DBUILD_SHARED_LIBS=OFF ..
```

### Build Paralelo

```bash
# Linux/Mac: usar todos os cores
make -j$(nproc)          # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# Windows: 
cmake --build . --parallel

# Especificar número de jobs
make -j4
```

---

## Troubleshooting

### Erro: "CMake 3.20 or higher is required"

**Solução:**
```bash
# Ubuntu/Debian
sudo apt remove cmake
sudo snap install cmake --classic

# macOS
brew upgrade cmake

# Windows
# Baixar e instalar versão mais recente de cmake.org
```

### Erro: "Could NOT find MySQL"

**Solução Linux:**
```bash
sudo apt install libmysqlclient-dev

# Se ainda falhar, especifique manualmente:
cmake -DMYSQL_ROOT_DIR=/usr/local/mysql ..
```

**Solução Windows:**
```powershell
# Baixe MySQL Connector C++ e extraia para third_party/
# Ou especifique path:
cmake -DMYSQL_ROOT_DIR="C:/Program Files/MySQL/MySQL Connector C++ 8.0" ..
```

### Erro: "nlohmann/json.hpp: No such file"

**Solução:**
```bash
# Atualizar submodules
git submodule update --init --recursive

# Ou adicionar manualmente
git submodule add https://github.com/nlohmann/json.git third_party/json
```

### Erro: Linking errors com MySQL

**Solução Linux:**
```bash
# Instalar bibliotecas de desenvolvimento
sudo apt install libmysqlcppconn-dev

# Adicionar ao LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/mysql/lib:$LD_LIBRARY_PATH
```

**Solução Windows:**
```powershell
# Copiar DLLs para diretório de build
copy "third_party\mysql-connector-cpp\lib64\*.dll" "build\bin\Release\"
```

### Erro: "error C2039: 'optional': is not a member of 'std'"

**Solução:**
```bash
# Verificar suporte a C++17
# CMakeLists.txt deve ter:
set(CMAKE_CXX_STANDARD 17)

# Visual Studio: usar VS 2017 15.7+
# GCC: usar versão 7+
# Clang: usar versão 5+
```

### Performance de Build Lenta

**Solução:**
```bash
# Usar compilação paralela
make -j$(nproc)

# Usar ccache (Linux)
sudo apt install ccache
export CC="ccache gcc"
export CXX="ccache g++"

# Usar precompiled headers (adicionar ao CMakeLists.txt)
target_precompile_headers(umbra_core PRIVATE <iostream> <vector>)
```

### Warnings excessivos

**Solução:**
```bash
# Desabilitar warnings de third-party
cmake -DCMAKE_CXX_FLAGS="-w" ..

# Ou em CMakeLists.txt:
target_compile_options(third_party_lib PRIVATE -w)
```

---

## Verificação de Build

### 1. Verificar Executáveis

```bash
# Linux/Mac
ls -lh build/bin/

# Windows
dir build\bin\Release\
```

### 2. Executar Testes

```bash
cd build
ctest --output-on-failure

# Ou individualmente
./bin/test_auth
./bin/test_db
./bin/test_zone
./bin/test_network
```

### 3. Verificar Dependências

**Linux:**
```bash
ldd build/bin/umbra_server
```

**macOS:**
```bash
otool -L build/bin/umbra_server
```

**Windows:**
```powershell
dumpbin /dependents build\bin\Release\umbra_server.exe
```

---

## Build para Produção

### Otimizações

```bash
mkdir build-prod && cd build-prod

cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" \
  -DBUILD_TESTING=OFF \
  ..

make -j$(nproc)

# Strip símbolos de debug
strip bin/*
```

### Validação

```bash
# Verificar tamanho
du -sh bin/*

# Verificar símbolos removidos
nm bin/umbra_server | grep -i debug

# Benchmark
time ./bin/umbra_server --benchmark
```

---

## Recursos Adicionais

- [CMake Documentation](https://cmake.org/documentation/)
- [MySQL Connector/C++ Documentation](https://dev.mysql.com/doc/connector-cpp/8.0/en/)
- [C++17 Features](https://en.cppreference.com/w/cpp/17)

---

**Última atualização**: 2025-10-12  
**Versão**: 1.3.0

