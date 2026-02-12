## Regras para Agentes (UmbraEternum)

### 0) Regra global
- Seguir `.cursor/rules/modify-files.mdc`: pedir permissão antes de editar e dizer exatamente o que será alterado.

### 1) Confirmação e escopo
- Antes de editar: listar arquivos/trechos e o objetivo (ex.: `src/auth/AuthServer.hpp` para ajuste de validação).
- Se afetar protocolo, explicar impacto cliente/servidor e quais arquivos serão atualizados em ambos os lados.

### 2) Contexto mínimo obrigatório
- Servidor C++: consultar arquivos relevantes em `src/` antes de propor mudança (`src/auth/AuthServer.hpp`, `src/network/SocketServer.hpp`, `src/zone/MovementProtocol.hpp`, `src/core/Logger.hpp`).
- Cliente UE: consultar `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`; para TCP, também `.../Network/UmbraTCPClient.cpp`.
- Movimento: frames binários little-endian 25B/34B conforme `src/zone/MovementProtocol.hpp`.

### 3) Fluxos que não podem ser quebrados
- Login cliente: HTTP via VaRest → token/conta → carregar personagens → conectar TCP com delay e binds (`UmbraGameInstance.cpp`, endpoints PHP em `www/umbra_api/api/*`).
- Cliente TCP: resetar antes de reconectar; verificar `bIsConnected` antes de enviar; validar token via `ValidateTokenViaTCP` (`UmbraTCPClient.cpp`).
- Servidor Auth: usar `JWTManager` + `SessionManager`; respeitar limites de tentativa no `AuthServer` antes de autenticar.
- Logging servidor: inicializar `Umbra::Core::Logger` antes de uso; níveis coerentes (DEBUG/INFO/WARN/ERROR/CRITICAL).

### 4) Protocolos e compatibilidade
- Frames de movimento: little-endian; tamanhos 25B/34B. Se adicionar campos, atualizar encode/decode em `src/zone/MovementProtocol.hpp` **e** no lado UE que consome o frame; manter compatibilidade.
- Mensagens TCP cliente: prefixo de tamanho (4 bytes) e criptografia opcional conforme `UmbraTCPClient.cpp`; manter esse contrato.

### 5) Estilo e práticas de código
- C++ servidor: namespaces `Umbra::<Domínio>`, `#pragma once`, ponteiros inteligentes, callbacks via `std::function`.
- C++ UE: macros Unreal (UPROPERTY/UFUNCTION), `CreateDefaultSubobject` no construtor, logs com `UE_LOG`.
- Logs: servidor via spdlog (`Logger`); cliente via `UE_LOG`; evitar prints ad-hoc.
- Segurança: não hardcode segredos; JWT via config/`JWTManager`; respeitar blacklist.

### 6) Como propor mudanças
- Indicar arquivos-alvo e efeito esperado (ex.: “Atualizar `MovementProtocol.hpp` e `UmbraTCPClient.cpp` para novo campo mantendo 25/34 bytes”).
- Sugerir testes aderentes: login HTTP + validação TCP no UE; para movimento, enviar/receber frame e verificar decode dos dois lados.

### 7) Testes recomendados
- Cliente UE: Play-in-Editor com login + seleção + conexão TCP; checar Output Log.
- Servidor/TCP: subir Auth/Gateway/Zone e observar `logs/server.log` (spdlog), usar nível debug em investigação.

### 8) Evitar
- Orientações vagas; preferir referências a arquivos/trechos.
- Mudar ordem de autenticação ou movimento sem plano de compatibilidade.
- Enviar dados TCP sem checar `bIsConnected` ou sem reset prévio antes de reconectar.

### 9) Consolidação (itens confirmados)
- Portas/hosts: consultar `config/server.json` (Gateway 9000, Auth 8080, World 8081, Zone base 8082, Chat 8083) antes de alterar fluxos.
- Build servidor C++: cd d:\UmbraServerV2\build; cmake --build . --config Release em `build/`. --targer (nome do server)
- Config/segredos: usar `config/server.json` e exemplos `config/*.example`; não versionar segredos; gerar chave JWT segura.
- Estilo: C++17 (2 espaços, PascalCase classes, camelCase métodos/variáveis, comentários curtos); Unreal com macros/reflexão; PHP PSR-12.
- Mudanças de protocolo/DB: sincronizar servidor C++ + UE + PHP + docs e retestar.
- Estrutura: manter CMake targets/submódulos; não mover `www/umbra_api` sem ajustar webroot/scripts.

### 10) MySQL (config e uso real)
- Config (PHP API): `www/umbra_api/config/database.php` — PDO DSN `mysql:host=localhost;port=3306;dbname=umbra_eternum;charset=utf8mb4`, `PDO::ATTR_ERRMODE=EXCEPTION`, `PDO::ATTR_EMULATE_PREPARES=false`.
- Config (servidor C++): `config/server.json` define host/porta/db/user/password; não versionar senhas reais.
- Segredos: a senha hardcoded em `database.php` e em `server.json` deve ser movida para ambiente/arquivo seguro em produção.
- Charset/collation: manter utf8mb4 (já no DSN e `set names utf8mb4`).
- SQL seguro: sempre prepared statements; evitar `SELECT *`; retornar só colunas necessárias.
- Schema esperado: banco `umbra_eternum` com tabelas `accounts`, `players`, `schema_version` (ajustar scripts SQL conforme necessário).
- Validação: após mudar schema ou credenciais, testar `getConnection()` (database.php) e rodar `SELECT 1`.

### 11) Estrutura do projeto
- Raiz: `README.md`, `README_NOVO.md`, `config/` (exemplos e server.json), `scripts/`, `scripts_main/`, `docs/`, `docs_main/`, `tests/`.
- Servidor C++: `src/` com módulos `auth/`, `gateway/`, `world/`, `zone/`, `chat/`, `network/`, `core/`, `database/`, `services/`; builds via `CMakeLists.txt`.
- Cliente UE5: `UmbraEternumUE/` (código em `Source/`, assets em `Content/`, docs em `docs/`, scripts de build/clean em `scripts/`).
- APIs PHP: `www/umbra_api/` (config em `config/database.php`, endpoints em `api/`, helpers em `helpers/`, scripts SQL em `scripts/`).
- Terceiros: `third_party/` (spdlog, nlohmann/json, googletest, etc.).
- Documentação adicional: `UmbraServer/docs_main/` e `docs/` para guias, troubleshooting e integrações.

### 12) Estrutura completa (condensada)
- Raiz: `.github/`, `.cursor/`, `config/`, `docs/`, `docs_main/`, `scripts/`, `scripts_main/`, `src/`, `tests/`, `third_party/`, `UmbraEternumUE/`, `UmbraServer/`, `www/`, `CMakeLists.txt`, `README*.md`.
- src/ (servidor C++): `auth/`, `gateway/`, `world/`, `zone/`, `chat/`, `network/`, `core/`, `database/`, `services/`, `main.cpp`.
- UmbraEternumUE/ (cliente UE5): `Source/` (Core, Network, UI), `Content/` (assets .uasset/.umap), `Plugins/VaRest/` (Source/Config/Resources), `scripts/` (build/clean), `tests/` (batch/py/ps1 TCP), `docs/`, `Config/`, `UmbraEternumUE.uproject`.
- www/umbra_api/ (PHP): `api/` (admin, character, common, inventory, storage, login/register, test.php), `config/database.php`, `helpers/jwt_helper.php`, `scripts/` (SQL), páginas de teste HTML/PHP, `composer.json`.
- scripts/: utilitários gerais (bat/sh/ps1); scripts_main/: bat/ps1/cpp/sql para setup e manutenção de dados.
- tests/ (C++): `test_auth.cpp`, `test_db.cpp`, `test_network.cpp`, `test_zone.cpp`.
- UmbraServer/: `docs_main/`, `scripts_main/`, `README_NOVO.md`.
- third_party/: libs externas (spdlog, nlohmann/json, googletest, etc.).

### 13) Git: Commits e Push
- **Antes de executar** qualquer operação git (commit/push), sempre:
  1. Verificar status do git em ambos os repositórios
  2. Listar TODOS os arquivos que serão commitados (modificados, deletados e novos)
  3. Informar os branches corretos (Principal: `Development`, Submódulo: `develop`)
  4. Aguardar confirmação EXPLÍCITA do usuário
- **Ordem de execução**: Sempre commitar/pushar o submódulo (`UmbraEternumUE`) PRIMEIRO, depois o repositório principal.
- **Não modificar arquivos** durante operações git — apenas comandos git.
- **Branches padrão**:
  - Repositório principal (`UmbraServerV2`): branch `Development`
  - Submódulo (`UmbraEternumUE`): branch `develop`
- **Mensagens de commit**: usar prefixos `feat:`, `fix:`, `docs:`, `refactor:`, `chore:` conforme o tipo de alteração.
