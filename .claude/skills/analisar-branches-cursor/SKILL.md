---
name: analisar-branches-cursor
description: Analisa as branches origin/cursor/* abertas pela automação do Cursor Agent, confere cada achado contra o Development atual, porta à mão o que ainda é bug numa branch fix/cursor-findings-<data>, abre o PR e, com o OK do usuário, apaga só as branches analisadas. Usar quando o usuário pedir "análise das branches do Cursor" ou /analisar-branches-cursor.
---

# Análise das branches do Cursor

A automação agendada do Cursor abre branches `origin/cursor/*` (ex.: `critical-bug-investigation-xxxx`, `critical-correctness-bugs-xxxx`), mais ou menos uma por semana. Ela fica **ligada de propósito**: pode achar bugs novos. As branches nascem antigas e repetem achados, então **nunca mergear nem fazer cherry-pick direto**.

Responder em pt-BR e seguir o `CLAUDE.md` (plano antes de editar, guia por feature, regras de zone).

## 1) Levantamento (só leitura)

```bash
git fetch origin --prune
git ls-remote --heads origin 'cursor/*'
for b in $(git branch -r --format='%(refname:short)' | grep cursor/); do
  mb=$(git merge-base Development $b)
  echo "=== $b ahead=$(git rev-list --count Development..$b) behind=$(git rev-list --count $b..Development)"
  git log --format='  %h %cs %s' Development..$b
  git diff --stat $mb $b | tail -n 25
done
```

- **Anotar o SHA da ponta de cada branch.** Ele é usado no passo 5.
- Agrupar as branches que repetem o mesmo tema. Ler o diff completo (`git diff <merge-base> <branch>`) só da mais completa de cada grupo.
- Descartar logo de cara os achados já listados em [ja_tratados.md](ja_tratados.md).

## 2) Triagem contra o `Development` atual

Para cada achado, abrir o código **atual** (Grep/Read) e classificar:

| Veredito | Quando |
|---|---|
| **Corrigir** | O bug ainda existe e é alcançável |
| **Código morto** | O bug existe, mas ninguém chama o trecho (`grep` dos chamadores). Remover ou corrigir, com prioridade baixa |
| **Obsoleto** | Já foi resolvido por outro caminho, ou o código foi reescrito |
| **Rejeitar** | O fix do Cursor viola regra do repo (ex.: DB/SELECT síncrono no recv ou no tick da zone, `printf`, SQL concatenado com input) |

Apresentar uma tabela `# | Problema | Situação hoje (arquivo:linha) | Veredito` e a proposta: lista de arquivos e objetivo. **Aguardar o OK** antes de editar.

## 3) Correção (após o OK)

1. Branch `fix/cursor-findings-<AAAA-MM-DD>` a partir do `Development` atualizado.
2. Reescrever à mão, seguindo o padrão atual do código. Na zone: o recv só enfileira, o trabalho com DB vai para o worker do `CombatCoreEngine` e as escritas usam `enqueueDbWrite`.
3. Guia `docs_main/GUIA_<TEMA>_UE561.md` a partir do template.
4. Validar:
   - `cmake --build . --config Release --target zone_server auth_server umbra_server` (em `build/`);
   - `ctest -C Release --output-on-failure` (o `ThreadCpuTimeTest` é flaky sob carga);
   - `php -l` em cada PHP alterado (`C:\wamp64\bin\php\php*\php.exe`);
   - `git diff --check` (sem whitespace no fim de linha, LF).
5. **Se algum PHP mudar:** listar todos com o destino `C:\wamp64\www\umbra_api\...` e lembrar de rodar `scripts_main\sync_umbra_api_to_wamp.bat`.
6. O usuário testa no PIE com 2 clients.

## 4) Commit, PR e merge (quando o usuário pedir)

- `git add` **só** dos arquivos da correção, nunca `-A`. Ficam fora: `CLAUDE.md`, o ponteiro do submódulo se não foi alterado, `secrets.php`, `tmp_*`, `dist/` e `archive/`.
- Commit `fix: ...` com a linha de atribuição, depois `git push -u origin <branch>`.
- `gh pr create --base Development ...`, depois `gh pr merge <n> --merge`, depois `git switch Development && git pull --ff-only`.

## 5) Apagar as branches analisadas (só com OK explícito)

**Atenção extrema:** apagar **somente** as branches que entraram na análise desta rodada, com o SHA anotado no passo 1. Branches `cursor/*` que surgiram depois ficam para a próxima rodada. Nunca usar padrão/glob no delete.

```bash
# 1. Lista explícita, um nome por linha (sem 'origin/')
cat > "$TEMP/analyzed.txt" <<'EOF'
cursor/critical-bug-investigation-xxxx
EOF
# 2. Conferir: cada nome existe no remoto e o SHA bate com o anotado
git ls-remote --heads origin 'cursor/*'
# 3. Apagar com lease no SHA (recusa se a ponta mudou desde a análise)
git push origin --force-with-lease=refs/heads/<branch>:<sha> :refs/heads/<branch> [...]
# 4. Verificar o que sobrou
git fetch origin --prune && git branch -r
```

Reportar a contagem exata de branches apagadas e as que sobraram.

## 6) Fechamento

- Acrescentar em [ja_tratados.md](ja_tratados.md) os achados desta rodada (corrigidos, obsoletos e rejeitados), com a data e o PR.
- Resumo ao usuário: PR, arquivos PHP para sincronizar, branches apagadas e pontos em aberto.
