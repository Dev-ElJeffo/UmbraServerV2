---
name: feature-e2e
description: Planeja e implementa uma feature ponta a ponta no UmbraEternum, cobrindo SQL idempotente, carga no servidor C++, protocolo, PHP admin, UmbraManager (WPF), cliente UE 5.6.1 e o guia docs_main/GUIA_*_UE561.md, seguindo o checklist do CLAUDE.md §3. Entrega primeiro o plano com a lista de arquivos para aprovação e depois executa camada por camada. Usar quando o usuário pedir uma feature ou campo novo configurável pelo Manager que atravesse mais de uma camada.
---

# Feature ponta a ponta

Base: `CLAUDE.md` §3 (checklist), §4 (protocolo), §5 (código), §6 (guia). Commits de referência do padrão: `860569f`, `e29d3e3`, `6a4ab80` (usar `git show --stat <hash>` para ver o formato real).

## Fase A: Plano (sem editar nada)

1. Entender o pedido. Ler só o necessário (Grep/Read direto; subagente de busca só com `model: "haiku"`).
2. Para cada camada, decidir se **se aplica** e quais arquivos mudam:

| # | Camada | Arquivos típicos | Pontos de atenção |
|---|---|---|---|
| 1 | SQL | `www/umbra_api/scripts/add_<tabela>_<campo>.sql` | Idempotente (`INFORMATION_SCHEMA` + `PREPARE/EXECUTE`), header pt-BR, `COMMENT` na coluna. Em `npc_instances`: **nullable** ("NULL = herda template"). Modelo: `add_npc_kite_speed_mult.sql` |
| 2 | C++ carga | `src/zone/NpcManager.cpp`, `src/services/SkillService.cpp`, `LootService`... | `SHOW COLUMNS ... LIKE` cacheado em static; `COALESCE` no SELECT; **conferir o índice posicional** de todas as colunas seguintes; campo no struct + getter `effective*()` com clamp |
| 3 | Protocolo | `/novo-opcode` | Campo **trailing**; decoder tolerante dos dois lados |
| 4 | PHP | `api/admin/create_*.php`, `list_*.php`, `update_*.php` | Coluna + `:param` no create; SELECT no list; whitelist `$allowed` no update; cast com default; `requireAdminAuth` + `auditAdminWrite()` |
| 5 | Manager | `Models/*Row.cs`, `ViewModels/MainViewModel.<Feature>.cs`, `Views/**.xaml` | Model com default e comentário pt-BR; VM `[ObservableProperty]`, `TryGet*Prop`, `Build*Payload`, load-from-row e reset; `TextBox` com `HintAssist`; `reload_*` via `AdminHub.SendCommandAsync` depois do save |
| 6 | Admin TCP | `src/admin/ServiceAdminRegister.cpp` | `registry.registerCommand("reload_<x>", ...)` se ainda não existir |
| 7 | UE | `Core/UmbraGameInstance.*`, `Components/`, `UI/`, `Data/` | Parse no handler → cache no GameInstance → delegate dinâmico → componente/widget reage. Asset por **soft path** (`/Game/...Asset.Asset`) + `LoadObject`. Binds `BindWidgetOptional`. Sem Event Graph |
| 8 | Guia | `docs_main/GUIA_<TEMA>_UE561.md` | Copiar o `_TEMPLATE_GUIA_UE561.md`; parte do usuário (Editor, Manager, SQL, QA PIE ≥ 2) e "O que o C++ já faz" |

3. Apresentar ao usuário:
   - objetivo em 2-3 linhas;
   - tabela `Camada | Arquivos | O que muda` (só as camadas que se aplicam);
   - impacto de protocolo (cliente/servidor), se houver;
   - o que fica para o usuário: Editor, compilar UE, cadastro no Manager, rodar o SQL, sync do WAMP, PIE.
4. **Aguardar o OK.** O plano aprovado autoriza só os arquivos listados; qualquer arquivo novo fora da lista pede confirmação.

## Fase B: Execução (na ordem)

Sempre numa branch `feat/<tema>` (ou na feature branch atual).

1. **SQL.** Escrever a migration. Não executar no banco sem pedido; incluir o comando no guia:
   `mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_<...>.sql`
2. **C++.** Após mexer em SELECT posicional, listar no relatório os índices antes e depois.
3. **Protocolo.** Seguir `/novo-opcode`.
4. **PHP.** Prepared statements, sem `SELECT *`, `JSON_UNESCAPED_UNICODE`, erro 500 genérico.
5. **Manager.** `default protegido (<= 0 ? def : v)`, `MessageBox.Show` + `StatusText` nos erros, classes `sealed`, UI pt-BR.
6. **UE.** Prefixos `UUmbra*`, categoria `Umbra|<Área>`, header `// Arquivo.h — descrição`, `UE_LOG` com `[Classe]`, comentários citando o opcode. APIs da UE 5.6.x.
7. **Guia.** Nomes exatos de binds, paths `/Game/...`, cliques em **negrito**, tabela `Sintoma | Causa | Ação`.

## Fase C: Fechamento

1. `/validar`.
2. Relatório final:
   - arquivos por camada;
   - **PHP → WAMP** (tabela repo → `C:\wamp64\www\umbra_api\...`);
   - **UE para compilar**;
   - **passos do usuário**, na ordem: SQL → sync WAMP → reiniciar a zone → compilar UE → Editor (guia §1) → Manager → PIE.
3. Commit só quando o usuário pedir (`/commit-umbra`; submódulo primeiro).
