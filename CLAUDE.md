# CLAUDE.md — UmbraEternum (Claude Code)

@AGENTS.md

As regras do `AGENTS.md` acima continuam valendo. Este arquivo acrescenta o que é específico do fluxo de trabalho com o Claude Code.

## 1) Fluxo de trabalho combinado

- **Idioma:** responder sempre em pt-BR. Comentários de código e docs também ficam em pt-BR, seguindo o padrão do repo.
- **Antes de editar:** listar arquivos e objetivo e aguardar o OK. **Plano aprovado autoriza editar os arquivos listados nele**; qualquer arquivo fora da lista pede nova confirmação.
- **Divisão de papéis:**
  - **Claude:** código C++ (servidor e cliente UE), PHP, SQL, Manager (C#/XAML) e docs.
  - **Usuário:** Editor UE (WBP/BP/assets/Niagara), compilação do editor, cadastro no UmbraManager e testes no PIE.
- **Custo:** subagentes só de busca/leitura (Explore, localizar arquivo, ler trecho) rodam com `model: "haiku"`; reservar o modelo principal para design e código. Preferir Grep/Read direto a abrir subagente. Ao trocar de tarefa, o usuário roda `/clear` — não acumular contexto entre tarefas.
- **Toda feature nova vem com um guia** em `docs_main/GUIA_<TEMA>_UE561.md`, seguindo `docs_main/_TEMPLATE_GUIA_UE561.md` (ver §6).
- **Git:**
  - commit e push só quando o usuário pedir ou quando estiverem em plano aprovado;
  - submódulo `UmbraEternumUE` (`develop`) primeiro, depois o principal (`Development`, ou a feature branch atual);
  - prefixos `feat:`/`fix:`/`docs:`/`refactor:`/`chore:`;
  - nunca usar `git add -A` às cegas: excluir segredos e artefatos de debug (AGENTS.md §13.5);
  - sem force push, `reset --hard` ou `--no-verify` (já bloqueados em `.claude/settings.json`).
- **Skills do projeto** (`.claude/skills/`): usar por padrão em vez de refazer o roteiro à mão.

  | Skill | Quando usar |
  |-------|-------------|
  | `/feature-e2e` | Feature que atravessa camadas (SQL → C++ → protocolo → PHP → Manager → UE → guia); entrega o plano para o OK |
  | `/novo-opcode` | Opcode novo ou campo trailing no WS da zone (§4) |
  | `/validar` | Ao terminar qualquer implementação e antes de commitar (build, ctest, `php -l`, lint, regras da zone) |
  | `/sync-wamp` | Depois de alterar PHP: compara repo × `C:\wamp64\www\umbra_api` e sincroniza com OK |
  | `/commit-umbra` | Commit/push/PR/merge, só quando pedido ou em plano aprovado |
  | `/diagnosticar-zone` | Crash, lag, tick lento, queda de clients (só leitura) |
  | `/conferir-binds-ue` | Conferir WBP/BP × C++ × guia via MCP EngineWire (só leitura) |
  | `/auditar-php` | Auditoria de segurança das APIs PHP, só quando pedido (só leitura) |
  | `/analisar-branches-cursor` | Rotina de análise das branches `cursor/*` da automação do Cursor |

## 2) Mapa rápido por camada

### Servidor C++ (`src/`, CMake, VS 2022 x64)

| Peça | Arquivo |
|------|---------|
| Monolito Auth+World+Gateway | `src/main.cpp` → `umbra_server` (não sobe a zone) |
| **Zone (jogo real)** | `src/zone/main_zone.cpp` → `zone_server.exe <zoneId>`; WS na porta 8082+id, admin na 9102+id |
| Opcodes + encode/decode | `src/zone/MovementProtocol.hpp` (`enum MovementMsgType`) |
| Dispatch inbound WS | `src/zone/MovementServer.hpp` (`handleInboundBinary`; recv **só enfileira**) |
| Combate autoritativo | `src/zone/CombatCoreEngine.{hpp,cpp}` (worker thread + `enqueueDbWrite`) |
| Stats/CC | `src/zone/CombatEffectRuntime.*`, `src/services/CombatCalculator.hpp` |
| Ranges/tolerâncias | `src/zone/CombatRange.hpp` |
| Skills/buffs | `src/services/SkillService.*`, `src/services/SkillTypes.hpp` |
| NPC | `src/zone/NpcManager.*` (templates/instâncias, SELECT posicional), `src/zone/NpcAiSystem.*` |
| Loot/EXP/Quest | `LootService`, `EnchantRoll.hpp`, `ExperienceService`, `ExpZoneManager`, `QuestProgressService` |
| Admin TCP (Manager) | `src/admin/` (4 bytes LE + JSON + HMAC); comandos em `ServiceAdminRegister.cpp` |
| Infra | `src/core/Logger.hpp` (spdlog, `Logger::getInstance().info(...)`), `ConfigManager` (`get<T>("a.b", def)`), `src/database/MySQLConnector.*` |

### Cliente UE 5.6.1 (`UmbraEternumUE/Source/UmbraEternumUE/`)

| Peça | Arquivo |
|------|---------|
| Hub central | `Core/UmbraGameInstance.{h,cpp}` (+ `UmbraGameInstanceEquipmentVisual.cpp`, `...ClientSettings.cpp`) |
| Dispatch WS | `Network/NetMovementClient.cpp` (`HandleWebSocketBinaryMessage`) → `UUmbraGameInstance::ProcessSocialWebSocketMessage` / `Handle*Message` |
| Frames binários | `Network/WSBinaryBPFL.h` |
| TCP legado (login) | `Network/UmbraTCPClient.cpp` (send-only na prática) |
| Componentes | `Components/` (Combat, CombatEffects=CC/118, BasicAttack, PersistentVfx, PlayerAppearance, PlayerSelection, NpcInteraction, FloatingText...) |
| NPC | `Actors/UmbraNpcCharacter.*`, `Animation/UmbraNpcAnimInstance.*` |
| Widgets | `UI/` (bases C++ de `UUserWidget`) |
| Structs | `Data/UmbraDataStructures.h`, `Data/UmbraSkillDataStructures.h` |
| Subsystem | `Systems/UmbraRefinementSubsystem.*` |

`Variant_*` são sobras do template e não devem ser usados.

### UmbraManager (WPF .NET 8, `tools/UmbraManagerWpf/UmbraManager.sln`)

- **Arquitetura:** MVVM com CommunityToolkit. `ViewModels/MainViewModel*.cs` é partial por feature; `Models/*Row.cs`; `Views/**.xaml`.
- **PHP admin:** `Services/PhpAdminClient.cs` (POST JSON em `admin/*.php`, Bearer JWT com escopo `umbra_manager`).
- **Admin TCP:** `Services/AdminClient.cs` / `AdminChannelHub.cs` (`reload_*`, spawn/teleport...).
- O `UmbraManager.sln` da raiz está vazio; usar o da pasta `tools/`.

### PHP API (`www/umbra_api/`)

- **Endpoints:** `api/<domínio>/*.php`. Admin usa `require_admin_auth.php` + `requireAdminAuth($data)` + whitelist `$allowed` + `auditAdminWrite()`.
- **Banco e resposta:** PDO com prepared statements, resposta `{success, message, ...}` e `JSON_UNESCAPED_UNICODE`.
- **Migrations:** `scripts/add_<tabela>_<campo>.sql`, **idempotentes** (`INFORMATION_SCHEMA` + `PREPARE/EXECUTE`), com header em pt-BR.
- **Deploy no WAMP:** `scripts_main/sync_umbra_api_to_wamp.bat`.

## 3) Checklist de feature ponta a ponta

Padrão extraído de `860569f`, `e29d3e3` e `6a4ab80`. Pular só as etapas que não se aplicam.

1. **SQL:** `www/umbra_api/scripts/add_<tabela>_<campo>.sql`, idempotente. Em `npc_instances`, a coluna fica nullable ("NULL = herda template").
2. **C++ (carga):**
   - detectar a coluna com `SHOW COLUMNS ... LIKE` cacheado em static;
   - incluir no SELECT com `COALESCE`;
   - **conferir o índice posicional** das colunas (`NpcManager.cpp`);
   - criar o campo no struct, com getter `effective*()` com clamp.
3. **Protocolo** (se houver), seguindo a §4.
4. **PHP:** coluna e `:param` no `create_*`; SELECT no `list_*`; whitelist no `update_*`; cast com default.
5. **Manager:**
   - Model: propriedade com default e comentário pt-BR;
   - VM: `[ObservableProperty]`, `TryGet*Prop`, `Build*Payload`, load-from-row e reset;
   - XAML: `TextBox` com `HintAssist`;
   - depois do save, `reload_*` via admin TCP.
6. **UE:** parse no handler, cache no GameInstance, delegate dinâmico, e o componente ou widget reage.
7. **Guia** `docs_main/GUIA_*_UE561.md`, com a parte do usuário: Editor, Manager, SQL e QA.
8. **Submódulo:** commit no UE primeiro e depois o bump no principal.

## 4) Regras de protocolo (zone WS)

- O pacote é binário **little-endian**, com o opcode no 1º byte. Os frames de movimento continuam com 25B/34B.
- **Opcode novo:**
  1. valor no `enum MovementMsgType` e encode/decode em `MovementProtocol.hpp`;
  2. branch em `MovementServer::handleInboundBinary` e lógica no `CombatCoreEngine` ou serviço;
  3. no UE, incluir o opcode na lista de forward do `NetMovementClient.cpp` e um branch/handler no GameInstance.
- **Campo novo em opcode existente:** sempre **trailing**. O decoder dos dois lados tolera frame antigo (string vazia ou 0).
- Strings vão como `u16` de tamanho + UTF-8, com limite explícito.
- Todo opcode novo ou alterado é documentado na seção "Protocolo" do guia.
- O último opcode em uso é **118** (`CombatControlState`). Conferir o enum antes de alocar um novo.

## 5) Regras de código

### Servidor (zone)

- **Nunca** fazer DB ou auth no callback de recv do WebSocket; só enfileirar. Crashes antigos vieram disso.
- Escrita em DB fora do caminho quente: `enqueueDbWrite`. Em código novo, preferir `executePrepared*` a concatenar SQL.
- **Estilo:**
  - `namespace Umbra::<Domínio>`, `#pragma once`, 2 espaços;
  - classes em PascalCase, métodos em camelCase, membros com `member_`;
  - smart pointers e `std::function`.
- **Logs:** pelo `Core::Logger` (sem `printf`/`cout`). Novos blocos `AgentDebugLog` (`// #region agent log`) só para investigação temporária.
- **Lint do CI:** sem whitespace no fim da linha e **LF** (sem CRLF) em `.cpp`/`.hpp`.

### Cliente UE — Blueprint mínimo (obrigatório)

- **Toda lógica em C++:** rede, estado, cálculo, timers e regras. O WBP/BP só define **Parent Class**, layout, estilo e assets.
- **Widgets:**
  - binds via `UPROPERTY(meta=(BindWidgetOptional))`, com o nome exato documentado no guia;
  - fallback `GetWidgetFromName` quando houver nomes alternativos;
  - linhas e itens dinâmicos montados em C++ com `WidgetTree->ConstructWidget` ou `CreateWidget` de uma `TSubclassOf` em `EditDefaultsOnly`.
- `BlueprintImplementableEvent` só para hooks **cosméticos** (som, animação de UI). Nunca para lógica de jogo.
- Não criar lógica de Event Graph. Se a única saída for um nó, o guia traz o pseudo-grafo e o motivo.
- **Componentes:** `CreateDefaultSubobject` no construtor, ou `NewObject` + `RegisterComponent` em runtime (padrão do `UmbraPersistentVfxComponent`).
- **Eventos:** delegates `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` no GameInstance; handlers como `UFUNCTION()` sem exposição a BP.
- **Convenções:**
  - prefixos `UUmbra*`/`AUmbra*`/`FUmbra*`/`EUmbra*`;
  - categorias `Umbra|<Área>`;
  - header `// UmbraXxx.h — descrição`;
  - comentários em pt-BR citando o opcode;
  - `UE_LOG` com tag `[ClassName]`.
- Assets de VFX/anim chegam do servidor como **soft path** (`/Game/...Asset.Asset`), carregados por `StaticLoadObject`/`LoadObject`. Não usar tabela local quando o dado vier do Manager.
- APIs e depreciações seguem a **UE 5.6.x**.
- Não compilo o editor UE sem pedido. Indico os arquivos alterados e o usuário compila.

### Manager / PHP

- **Manager:** namespaces file-scoped, classes `sealed`, UI em pt-BR, erros via `MessageBox.Show(...)` + `StatusText`, default protegido (`<= 0 ? def : v`).
- **PHP:** PSR-12 na medida do arquivo existente. Prepared statements sempre, sem `SELECT *`, `error_log('[admin/x] ...')` e resposta 500 com "Erro interno".

## 6) Guia de implementação (obrigatório por feature)

- **Arquivo:** `docs_main/GUIA_<TEMA>_UE561.md`, copiado de `docs_main/_TEMPLATE_GUIA_UE561.md`.
- **Foco:** o que **o usuário** precisa fazer no Editor, no Manager e no MySQL, e como validar.
- **Formato:**
  - assets com path `/Game/...`;
  - nomes exatos de binds;
  - cliques em **negrito**;
  - tabelas `Sintoma | Causa | Ação`;
  - QA com PIE ≥ 2 players.
- **"O que o C++ já faz":** listar sempre, para deixar claro que não precisa de Graph.

## 7) Build e testes

- **Servidor:** `cd D:\UmbraServerV2\build; cmake --build . --config Release --target zone_server`. Outros targets: `umbra_server`, `auth_server`, `gateway_server` e os testes. Saída em `build/bin/Release`.
- **Testes C++:** `ctest -C Release --output-on-failure` em `build/`.
- **Manager:** `dotnet build tools/UmbraManagerWpf/UmbraManager.sln`. Ele não atualiza o `dist/` (ver `UmbraServer/docs_main/GUIA_UMBRA_MANAGER.md`).
- **UE:** quem compila é o usuário (Live Coding ou rebuild do editor). PIE com 2 clients para qualquer coisa replicada.
- **PHP:** depois de alterar, sincronizar com o WAMP (`scripts_main/sync_umbra_api_to_wamp.bat`).

## 8) Armadilhas conhecidas

- **TCP do login (porta 9000):**
  - O Gateway remonta as mensagens `[len:u32 BE][payload]` com `src/network/LengthPrefixedFramer.hpp`.
  - O modo é escolhido pelo 1º byte: `0x00` = framed; qualquer outro = legado, pedaço cru.
  - Responde no mesmo formato ao cliente framed e aceita `validate_token` e `authenticate`.
  - O canal Gateway↔Auth é JSON por linha (`\n`), e o Auth responde **todo** pedido, porque o `AuthClient` casa as respostas em FIFO.
  - O `SocketServer` continua entregando pedaços crus; o framing fica no consumidor (como no `AdminServer`).
  - O UE ainda **não lê** as respostas TCP: o `ProcessServerResponse` não é chamado. A segurança da sessão é o opcode 109 na zone.
- **Código morto ou stub:** `ZoneOrchestrator`, `network/MessageHandler`, `services/CombatService`, `InventoryService`, `Matchmaking` e a seção `redis` da config.
- **Chat:** o `ChatServer` (`src/chat/`) e a porta `chat.port` **não são usados**. O chat real roda na zone, pelo WebSocket, nos opcodes 66–73. A zone lê de `chat.*` só os limites (`max_message_length`, `rate_limit_per_minute`).
- **SELECT posicional do `NpcManager`:** coluna nova muda os índices. Conferir sempre.
- **Portas:** o admin da zone 2 = 9104 = `admin.gateway_port` (conflito real se subir 3+ zones). A zone 1 usa a porta de jogo 8083.
- **Segredos versionados:** `config/server.json` e `server.proxmox.json` têm segredos versionados; não espalhar e não commitar novos.
- **Chave JWT dev:** a zone usa uma chave dev hardcoded quando `auth.jwt_secret` está vazio (`ZoneServer.cpp` ~108).
- **Sem DB:** se o MySQL falha, a zone sobe **só com movimento**, sem combate, EXP ou loot.
