---
name: validar
description: Valida as alterações do working tree do UmbraServerV2 antes de entregar ou commitar. Detecta as camadas tocadas (servidor C++, PHP, Manager, UE) e roda só o que se aplica, ou seja, build, ctest, php -l, lint do CI e checagem das regras da zone, e lista os PHP para sincronizar com o WAMP. Usar ao terminar uma implementação, quando o usuário pedir /validar, ou antes de /commit-umbra.
---

# Validar alterações

Objetivo: um relatório curto e honesto (passou / falhou, com a saída) do que foi validado. **Não editar nada** aqui; se algo falhar, reportar e propor o conserto.

## 1) Descobrir o que mudou

```bash
cd /d/UmbraServerV2
git status --short
git diff --name-only HEAD; git ls-files --others --exclude-standard
git -C UmbraEternumUE status --short
```

Classificar os arquivos (ignorar `dist/`, `archive/`, `tmp_*`, `secrets.php`):

| Camada | Padrão | Validação |
|---|---|---|
| Servidor C++ | `src/**`, `tests/**`, `CMakeLists.txt` | §2 + §3 |
| PHP | `www/umbra_api/**.php` | §4 |
| SQL | `www/umbra_api/scripts/*.sql` | §5 |
| Manager | `tools/UmbraManagerWpf/**` | §6 |
| UE | `UmbraEternumUE/Source/**` | §7 (só estático, **não compilar**) |

## 2) Servidor C++: build e testes

Targets conforme o que mudou: `src/zone/**` → `zone_server`; `src/auth|database|gateway|world/**` → `auth_server umbra_server`; `src/services|core/**` → todos.

```powershell
cd D:\UmbraServerV2\build
cmake --build . --config Release --target <targets> -- /m /v:minimal 2>&1 | Select-String -Pattern " error |warning C4" | Select-Object -First 30
cmake --build . --config Release -- /m /v:minimal 2>&1 | Select-String " error " | Select-Object -First 10
ctest -C Release --output-on-failure --timeout 30
```

- `ThreadCpuTimeTest.BusyLoopConsumesCpuSleepDoesNot` (NetworkTests) é **instável sob carga**. Se só ele falhar, rodar `ctest -C Release -R NetworkTests` isolado e reportar como instável, não como regressão.
- `DatabaseTests` pode dar timeout sem MySQL de teste. Reportar como ambiente.

## 3) Regras da zone e lint do CI (no diff)

```bash
cd /d/UmbraServerV2
git diff --check                                    # whitespace no fim de linha
git grep -I -n --perl-regexp '\s+$' -- '*.cpp' '*.hpp' | head    # mesmo check do CI
for f in $(git diff --name-only HEAD -- '*.cpp' '*.hpp'); do grep -l $'\r' "$f"; done   # CRLF (deve sair vazio)
git diff -U0 HEAD -- src | grep -nE '^\+.*(printf\(|std::cout|std::cerr)'              # logs fora do Logger
```

Revisão manual das linhas adicionadas em `src/zone/MovementServer.hpp` (`handleInboundBinary`, que roda no tick) e nos callbacks de recv: **não pode** haver `db_->`, `executePrepared*`, `executeQuery` nem chamada que faça DB de forma síncrona. O certo é resolver o sender sob lock curto e enfileirar no worker (`enqueueSkillCast`, `enqueueForeignVitals`...). Também conferir:
- lock `mu_` liberado antes de chamar o `CombatCoreEngine` (deadlock recursivo);
- SQL novo com `executePrepared*` ou `enqueueDbWrite` (sem concatenar input do cliente);
- coluna nova no SELECT posicional do `NpcManager.cpp`, com os índices conferidos.

Avisos "LF will be replaced by CRLF" do git são do `autocrlf` e não são erro.

## 4) PHP

```bash
PHP=$(ls /c/wamp64/bin/php/*/php.exe | tail -1)
for f in <php alterados>; do "$PHP" -n -l "$f"; done
```

Revisão rápida de cada PHP alterado: prepared statements, sem `SELECT *`, `player_id` vindo do body validado com `assertPlayerBelongsToAccount` (`helpers/personal_shop_helper.php`), admin com `requireAdminAuth`, erro 500 genérico ("Erro interno") com `error_log('[x] ...')`.

**Sempre** terminar listando os PHP alterados com o destino no WAMP:

| Repo | WAMP |
|---|---|
| `www/umbra_api/<caminho>` | `C:\wamp64\www\umbra_api\<caminho>` |

e lembrar de usar `/sync-wamp` (ou `scripts_main\sync_umbra_api_to_wamp.bat`).

## 5) SQL

Cada `scripts/add_*.sql` precisa ser idempotente (`INFORMATION_SCHEMA` + `PREPARE/EXECUTE`) e ter header em pt-BR. Não executar no banco sem pedido.

## 6) Manager

```powershell
dotnet build D:\UmbraServerV2\tools\UmbraManagerWpf\UmbraManager.sln -v minimal 2>&1 | Select-String -Pattern "error|Warn|Build succeeded" | Select-Object -Last 15
```

(Não atualiza o `dist/`.)

## 7) UE (estático)

Não compilar o editor. Conferir no diff: `UPROPERTY(meta=(BindWidgetOptional))` nos binds, nada de lógica nova em `BlueprintImplementableEvent`, `UE_LOG` com a tag `[Classe]`, opcode novo incluído na lista de forward do `NetMovementClient.cpp`. Listar os arquivos UE alterados para o usuário compilar.

## 8) Relatório

```
| Camada | Validação | Resultado |
```
Depois: falhas com a saída relevante, a lista de PHP → WAMP e a lista de arquivos UE para compilar.
