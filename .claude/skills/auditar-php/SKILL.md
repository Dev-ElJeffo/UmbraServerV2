---
name: auditar-php
description: Audita a segurança dos endpoints PHP em www/umbra_api/api, procurando IDOR (player_id do body sem checar posse), endpoint admin sem requireAdminAuth, SQL concatenado, SELECT *, vazamento de erro interno e falta de validação de entrada. Só lê e relata, sem editar. Usar quando o usuário pedir /auditar-php ou uma auditoria das APIs PHP. Não rodar por iniciativa própria.
---

# Auditoria de segurança das APIs PHP

**Só leitura.** O resultado é uma tabela de achados e uma proposta de correção; editar só após o OK (aí vale o fluxo normal + `/validar` + lista de PHP para o WAMP).

Escopo padrão: `www/umbra_api/api/**`. Se o usuário indicar um domínio (ex.: `auction`), restringir a ele. Antes de começar, ler [ja_auditados.md](ja_auditados.md) para não repetir o que já foi tratado.

## 1) Inventário

```bash
cd /d/UmbraServerV2/www/umbra_api
find api -name '*.php' | sort | awk -F/ '{print $2}' | uniq -c
```

Contexto (ler uma vez):
- `helpers/jwt_helper.php`: `validateJWTRequest($data, $_SERVER)` devolve `['valid', 'payload' => ['player_id', 'account_id', ...]]`;
- `helpers/personal_shop_helper.php`: `assertPlayerBelongsToAccount(PDO, player_id, account_id)`;
- `api/admin/require_admin_auth.php`: `requireAdminAuth($data)` (JWT com escopo `umbra_manager`).

## 2) Varreduras (Grep direto; se for grande, subagente Explore com `model: "haiku"`)

| Checagem | Como encontrar | É achado quando |
|---|---|---|
| **IDOR** | `\$data\['(player_id\|source_player_id\|target_player_id\|character_id\|seller_id\|owner_id)'\]` | O ID vem do body e o arquivo não chama `assertPlayerBelongsToAccount` nem compara com `account_id` do JWT antes de **escrever** ou de **ler dado privado** (inventário, storage, mail, ouro) |
| ID de recurso alheio | `item_id`, `inventory_id`, `listing_id`, `mail_id`, `guild_id` vindos do body | O UPDATE/DELETE não tem `AND player_id = :pid` (ou equivalente) amarrando ao dono |
| Sem auth | arquivos em `api/` sem `validateJWTRequest`/`requireAdminAuth` | Endpoint que escreve ou expõe dado de jogador (exceções legítimas: login, register, `verify_admin*`, status público) |
| Admin sem guarda | `api/admin/*.php` sem `requireAdminAuth` | Qualquer escrita; e escrita sem `auditAdminWrite()` (achado leve) |
| SQL concatenado | `->query\(\|->exec\(\|"SELECT .*\$\|\. \$[a-z_]+ \.` | Variável de input dentro da string SQL |
| `SELECT *` | `SELECT \*` | Regra do repo (achado leve) |
| Vazamento de erro | `getMessage\(\)` dentro de `json_encode`/`echo` | Mensagem de exceção indo para o cliente (o certo é `error_log` + "Erro interno") |
| Valor do cliente como autoridade | `delta_`, `damage`, `price`, `amount`, `gold`, `exp` vindos do body | O servidor aplica o valor sem limite ou recálculo (marcar como **design**; ver o conhecido em `ja_auditados.md`) |
| Transação | compra/venda/troca/craft | Débito e crédito sem `beginTransaction` + `FOR UPDATE` (race/dupe) |

Para cada candidato, **abrir o arquivo** e confirmar no fluxo real (o check pode estar num helper incluído). Grep sozinho não é achado.

## 3) Classificação

| Severidade | Critério |
|---|---|
| **Crítica** | Escrita em dado de outro jogador ou admin sem auth (dupe, roubo, ban) |
| **Alta** | Leitura de dado privado alheio; race em economia |
| **Média** | Valor do cliente como autoridade com limite parcial; erro interno vazando |
| **Baixa** | `SELECT *`, falta de `auditAdminWrite`, validação de tipo fraca |

Também conferir se o UE chama o endpoint (`grep -rn "<arquivo>.php" UmbraEternumUE/Source`). Endpoint não usado pode ser candidato a remoção.

## 4) Relatório

```
| # | Sev. | Endpoint | Problema | Evidência (arquivo:linha) | Usado pelo UE? | Correção proposta |
```

Agrupar por domínio. Depois, propor o lote de correção (arquivos e padrão de fix, normalmente `assertPlayerBelongsToAccount` logo após o `getConnection()`, com 401 sem `account_id` e 403 sem posse) e aguardar o OK.

## 5) Após corrigir

Atualizar [ja_auditados.md](ja_auditados.md) com a data, os endpoints auditados e os achados (corrigidos, aceitos ou em aberto).
