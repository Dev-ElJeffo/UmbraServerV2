## Regras para Agentes (UmbraEternum)

### 0) Regra global
- Seguir `.cursor/rules/modify-files.mdc`: pedir permissão antes de editar e dizer exatamente o que será alterado.
- **Idioma:** todas as respostas ao usuário em **português do Brasil (pt-BR)** — normas e exemplos na **§0.1**.

### 0.1) Idioma — português do Brasil (pt-BR)
- Respostas ao **usuário** em **português do Brasil (pt-BR)**: **você**, vocabulário e tom brasileiros.
- **Evitar** português de Portugal (ex.: *ficheiro* → **arquivo**; *grelha* (UI) → **grade**; *ecrã* → **tela**; *noutro* → **em outro**; *tem de* → **tem que** / **precisa**; *está a* / *a ser* → **está** / **sendo**; preferir expressões usadas no Brasil ao explicar UI/docs).
- Documentação ou comentários em português **só quando o usuário pedir**; quando forem em português, seguir **pt-BR**.

### 1) Confirmação e escopo
- Antes de editar: listar arquivos/trechos e o objetivo (ex.: `src/auth/AuthServer.hpp` para ajuste de validação).
- Se afetar protocolo, explicar impacto cliente/servidor e quais arquivos serão atualizados em ambos os lados.

### 2) Contexto mínimo obrigatório
- Servidor C++: consultar arquivos relevantes em `src/` antes de propor mudança (`src/auth/AuthServer.hpp`, `src/network/SocketServer.hpp`, `src/zone/MovementProtocol.hpp`, `src/core/Logger.hpp`).
- Cliente UE: consultar `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`; para TCP, também `.../Network/UmbraTCPClient.cpp`.
- Movimento: frames binários little-endian 25B/34B conforme `src/zone/MovementProtocol.hpp`.
- **Combate V2 (dano real):** guia autoritativo em `docs_main/GUIA_COMBATE_V2_DANO_REAL.md` — `CombatCoreEngine`, `CharacterStateLoader`, `CombatCalculator`, opcodes 87/92/93/96–103, mana sync, regen, miss, DOT/HOT. Complementos: `UmbraServer/docs_main/GUIA_COMBAT_V2_DANO_BASIC_ATTACK_NPC.md` (BP/NPC/LMB), `UmbraServer/docs_main/GUIA_SISTEMA_COMBATE.md` (V1, 86–95).

### 2.1) Unreal Engine — versão do projeto (**5.6.1**)
- O cliente **UmbraEternumUE** é desenvolvido e testado em **Unreal Engine 5.6.1**. Ao sugerir APIs UMG/Slate, `UWidgetTree::ConstructWidget`, módulos ou depreciações, usar como referência a documentação da **5.6.x**.
- Guia nó a nó (loja + nameplate + comprador + bloqueio de movimento): `docs_main/GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md`.

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
- Indicar arquivos-alvo e efeito esperado (ex.: "Atualizar `MovementProtocol.hpp` e `UmbraTCPClient.cpp` para novo campo mantendo 25/34 bytes").
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
- Build servidor C++: cd d:\UmbraServerV2\build; cmake --build . --config Release --target (nome server) em `build/`.
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
- Cliente UE5 (**5.6.1**): `UmbraEternumUE/` (código em `Source/`, assets em `Content/`, docs em `docs/`, scripts de build/clean em `scripts/`).
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

### Exemplo de workflow completo:

```bash
# 1. Push do submódulo UmbraEternumUE (branch develop)
cd D:\UmbraServerV2\UmbraEternumUE
git add -A
git status
git commit -m "feat: skill drag & drop e remoção do wrapper"
git push -u origin develop

# 2. Push do repositório principal UmbraServerV2 (branch Development)
cd D:\UmbraServerV2
git add -A
git status
git commit -m "feat: atualizar submódulo UmbraEternumUE"
git push -u origin Development
```

## Cursor Cloud specific instructions

### Serviços e como iniciar

O projeto tem 3 componentes principais que rodam neste ambiente Linux:

| Componente | Tecnologia | Como iniciar |
|---|---|---|
| **Servidor C++ (monolítico)** | C++17 / CMake | `cd /workspace/build/bin && ./umbra_server` (Auth:8080, World:8081, Gateway:9000) |
| **API PHP REST** | PHP 8.3 / Apache | `sudo service apache2 start` (porta 80, symlink em `/var/www/html/umbra_api`) |
| **MySQL** | MySQL 8.0 | `sudo service mysql start` (porta 3306, banco `umbra_eternum`, root password em `config/server.json`) |

O cliente UE **5.6.1** (`UmbraEternumUE/`) é um submódulo orientado a Windows e **não** é buildável neste ambiente Linux do Cursor Cloud.

### Gotchas importantes

- **Socket MySQL**: Após iniciar o MySQL, executar `sudo chmod 755 /var/run/mysqld && sudo chmod 777 /var/run/mysqld/mysqld.sock` para que o servidor C++ e testes consigam conectar via socket.
- **CMake com g++**: O ambiente tem Clang como default, mas a build precisa de `g++`. Sempre configurar com `-DCMAKE_CXX_COMPILER=g++`.
- **Build**: `cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ && cmake --build build --parallel`.
- **Testes**: `cd build && ctest -C Release --output-on-failure --timeout 20`. O teste `DatabaseTests` dá timeout pois tenta conectar ao MySQL com credenciais padrão (senha vazia, banco `umbra_eternum_test`); os outros 6 testes passam normalmente.
- **Lint**: CI usa `git grep -I --perl-regexp '\s+$' -- '*.cpp' '*.hpp'` para whitespace e `file | grep CRLF` para line endings.
- **Código Windows-only**: Alguns arquivos usam APIs Windows (`sprintf_s`, includes implícitos). As correções de compatibilidade Linux foram aplicadas em `Matchmaking.hpp`, `EntitySystem.hpp`, `MovementServer.hpp`, `GatewayServer.cpp` e `tests/CMakeLists.txt`.
- **Schema DB**: Rodar `scripts_main/setup_database.sql` + scripts em `www/umbra_api/scripts/` (na ordem: `create_*`, `add_*`) para criar o schema completo. Alguns scripts falham parcialmente se colunas já existem (idempotente).
- **PHP API**: Composer deps em `www/umbra_api/`: `cd www/umbra_api && composer install`.

### Fluxo de teste "hello world"

1. Iniciar MySQL + Apache + servidor C++
2. `curl POST /umbra_api/api/register.php` (criar conta)
3. `curl POST /umbra_api/api/login.php` (obter JWT)
4. `curl POST /umbra_api/api/character/create_character.php` (criar personagem com token)
5. Conectar TCP na porta 9000 para verificar Gateway
