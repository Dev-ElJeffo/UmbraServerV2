---
name: sync-wamp
description: Compara o PHP do repo (www/umbra_api) com o WAMP (C:\wamp64\www\umbra_api, onde as APIs de fato rodam), mostra o que difere e, com OK, sincroniza via scripts_main\sync_umbra_api_to_wamp.bat. Usar depois de alterar PHP, quando o usuário pedir /sync-wamp ou quando um endpoint se comportar diferente do código do repo.
---

# Sincronizar PHP com o WAMP

As APIs rodam em `C:\wamp64\www\umbra_api`, não no repo. O script oficial é `scripts_main\sync_umbra_api_to_wamp.bat`:

| Flag | Efeito |
|---|---|
| (nenhuma) | Repo → WAMP **seguro**: não sobrescreve arquivo mais novo no WAMP e não apaga nada |
| `--dry` | Simulação (combinável com as outras) |
| `--force` | Sobrescreve tudo no WAMP (perde edições feitas direto no WAMP) |
| `--reverse` | WAMP → Repo (também sem regredir) |
| `--mirror` | `/MIR`: **apaga** no WAMP o que não existe no repo. Não usar sem pedido explícito |

Sempre exclui `backups/`, `*.log`, `*.tmp` e `Thumbs.db`.

## 1) Comparar (só leitura)

```bash
bash /d/UmbraServerV2/.claude/skills/sync-wamp/compare_wamp.sh            # tudo
bash /d/UmbraServerV2/.claude/skills/sync-wamp/compare_wamp.sh api/combat # só uma pasta
```

Saída em três grupos:
- **REPO+NOVO**: o sync padrão vai copiar.
- **WAMP+NOVO**: alguém editou direto no WAMP, e o sync padrão **não** copia. Mostrar `diff` desses arquivos ao usuário e perguntar: trazer para o repo (`--reverse`), sobrescrever (`--force`) ou deixar.
- **Só no repo** / **Só no WAMP**: arquivos novos ou órfãos.

Cuidados:
- `config/database.php` e `config/secrets.php` costumam diferir **de propósito** (credenciais do ambiente). Nunca sobrescrever nem copiar o conteúdo para a conversa; só avisar se aparecerem.
- `vendor/` fica fora da comparação; se o `composer.json` mudou, lembrar de rodar `composer install` no WAMP.

## 2) Sincronizar (com OK do usuário)

Mostrar o resultado da simulação antes:

```bash
cmd //c "D:\\UmbraServerV2\\scripts_main\\sync_umbra_api_to_wamp.bat --dry"
```

Depois do OK:

```bash
cmd //c "D:\\UmbraServerV2\\scripts_main\\sync_umbra_api_to_wamp.bat"
```

(O `.bat` pode ter `pause` no fim; se travar, pedir ao usuário que rode pelo Explorer.)

## 3) Conferir

Rodar o `compare_wamp.sh` de novo na pasta alterada: os arquivos da tarefa não devem mais aparecer como diferentes. Opcional: `php -n -l` no arquivo do WAMP.

Relatório: tabela `arquivo | ação (copiado / pulado: WAMP mais novo / ignorado)`.
