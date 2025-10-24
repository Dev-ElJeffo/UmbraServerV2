# Third-Party Libraries

Esta pasta contém bibliotecas de terceiros como Git submodules.

## Instalação

Execute os seguintes comandos para baixar todas as dependências:

```bash
git submodule init
git submodule update
```

## Bibliotecas Necessárias

### 1. nlohmann/json
**Descrição**: Parser JSON para C++  
**Repositório**: https://github.com/nlohmann/json  
**Versão**: 3.11.2+

```bash
git submodule add https://github.com/nlohmann/json.git third_party/json
```

### 2. spdlog
**Descrição**: Biblioteca de logging assíncrono  
**Repositório**: https://github.com/gabime/spdlog  
**Versão**: 1.12.0+

```bash
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
```

### 3. googletest
**Descrição**: Framework de testes  
**Repositório**: https://github.com/google/googletest  
**Versão**: 1.14.0+

```bash
git submodule add https://github.com/google/googletest.git third_party/googletest
```

### 4. jwt-cpp
**Descrição**: Biblioteca JWT para C++  
**Repositório**: https://github.com/Thalhammer/jwt-cpp  
**Versão**: 0.7.0+

```bash
git submodule add https://github.com/Thalhammer/jwt-cpp.git third_party/jwt-cpp
```

### 5. MySQL Connector/C++
**Descrição**: Conector MySQL oficial  
**Download**: https://dev.mysql.com/downloads/connector/cpp/  
**Versão**: 8.0+

Baixe manualmente e extraia para `third_party/mysql-connector-cpp/`

### 6. ASIO (Opcional)
**Descrição**: Biblioteca de I/O assíncrono  
**Repositório**: https://github.com/chriskohlhoff/asio  
**Versão**: 1.28.0+

```bash
git submodule add https://github.com/chriskohlhoff/asio.git third_party/asio
```

### 7. Protobuf (Opcional)
**Descrição**: Protocol Buffers  
**Repositório**: https://github.com/protocolbuffers/protobuf  
**Versão**: 25.0+

```bash
git submodule add https://github.com/protocolbuffers/protobuf.git third_party/protobuf
```

## Comandos Rápidos

**Adicionar todas as dependências:**
```bash
cd UmbraServer
git submodule add https://github.com/nlohmann/json.git third_party/json
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog
git submodule add https://github.com/google/googletest.git third_party/googletest
git submodule add https://github.com/Thalhammer/jwt-cpp.git third_party/jwt-cpp
```

**Atualizar todas as dependências:**
```bash
git submodule update --remote --merge
```

## Notas

- Todas as bibliotecas devem ser compatíveis com C++17
- Evite usar `pip` ou gerenciadores de pacotes externos
- Para produção, compile bibliotecas estaticamente quando possível
- Consulte a documentação de cada biblioteca para requisitos específicos

