---
name: commit-umbra
description: Faz commit e push no UmbraServerV2 com segurança, com git add por lista explícita, exclusão de segredos e artefatos, submódulo UmbraEternumUE primeiro e bump do ponteiro depois, e opcionalmente PR e merge via gh. Usar só quando o usuário pedir commit, push, PR ou merge, ou quando o commit fizer parte de um plano aprovado.
---

# Commit / push / PR

Regras de base: `CLAUDE.md` §1 (Git) e `AGENTS.md` §13. Nunca `git add -A`/`.`, force push, `reset --hard` nem `--no-verify`.

## 1) Levantar o estado dos dois repos

```bash
cd /d/UmbraServerV2
git branch --show-current; git status --short
git -C UmbraEternumUE branch --show-current; git -C UmbraEternumUE status --short
```

## 2) Montar a lista de arquivos

Entra **só** o que pertence à tarefa atual (o que eu alterei ou criei nesta tarefa, ou o que o usuário indicou).

**Nunca entra** (mesmo que apareça no status):
- `config/server.json`, `config/server.proxmox.json` (segredos versionados: não espalhar);
- `www/umbra_api/config/secrets.php`, `tmp_*`, `*.log`, `debug-*.log`;
- `dist/`, `archive/`, `scripts_main/_dbg_*`, `tools/_an*.sh`, `crash_dumps/`, `*.dmp`;
- `UmbraEternumUE/Saved/**`, `Intermediate/`, `Binaries/`, `Saved/MCP/capability-token`.

**Só entra se fizer parte da tarefa:** `CLAUDE.md`, `AGENTS.md`, o ponteiro do submódulo `UmbraEternumUE`, scripts novos em `scripts_main/`.

Se houver dúvida sobre algum arquivo, perguntar antes. Se o usuário pediu "commit" sem lista, mostrar a lista proposta e o que ficou de fora.

## 3) Submódulo primeiro (se o UE mudou)

```bash
cd /d/UmbraServerV2/UmbraEternumUE
git switch develop            # ou a branch atual do submódulo, se o usuário indicou outra
git add <arquivos UE>          # lista explícita
git diff --cached --name-only
git commit -F - <<'EOF'
<prefixo>: <resumo em pt-BR>

<corpo opcional>

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
EOF
git push -u origin develop
```

## 4) Repositório principal

Branch: `Development` ou a feature branch atual. Se estiver no `Development` e a tarefa for uma feature/fix, criar `feat/<tema>` ou `fix/<tema>` antes, salvo se o usuário pedir commit direto.

```bash
cd /d/UmbraServerV2
git add <arquivos> [UmbraEternumUE]   # o ponteiro só se o passo 3 rodou
git diff --cached --check
git diff --cached --name-only
git commit -F - <<'EOF'
...
Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
EOF
git push -u origin <branch>
```

Prefixos: `feat:`, `fix:`, `docs:`, `refactor:`, `chore:`. Bump de submódulo: `chore: atualizar submodulo UmbraEternumUE (<tema>)`.

## 5) PR e merge (só se o usuário pedir)

```bash
gh pr create --base Development --head <branch> --title "<título do commit>" --body "<resumo, testes, pendências>

🤖 Generated with [Claude Code](https://claude.com/claude-code)"
gh pr merge <n> --merge          # merge commit, padrão do repo ("Merge pull request #N")
git switch Development && git pull --ff-only origin Development
```

Não apagar a branch nem outras branches sem pedido explícito.

## 6) Relatório

- Commits (hash + mensagem) de cada repo, branch e push/PR.
- Arquivos que **ficaram de fora** de propósito.
- Se houver PHP no commit: lembrar de `/sync-wamp`.
