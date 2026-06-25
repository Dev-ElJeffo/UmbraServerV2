# Guia UmbraManager (UmbraEternum)

Painel desktop **UmbraManager.exe** (WPF / .NET 8) para operação do stack de servidores UmbraEternum.

> A versão Qt6/C++ foi arquivada em `tools/UmbraManager.qt-archive/`. O build CMake dos servidores **não** inclui mais o manager desktop.

## Requisitos

- Windows 10/11
- [.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0) (Windows Desktop)
- Build Release dos servidores C++ em `build/bin/Release/`
- MySQL + Apache/PHP (`umbra_api`) para abas Accounts/Items

## Build (desenvolvimento)

```bat
cd D:\UmbraServerV2\tools\UmbraManagerWpf
dotnet build UmbraManager/UmbraManager.csproj -c Release
```

Executável: `tools/UmbraManagerWpf/UmbraManager/bin/Release/net8.0-windows/UmbraManager.exe`

> **Atenção:** `dotnet build` **não** atualiza `dist/UmbraManager/UmbraManager.exe`. Se você abrir o exe em `dist/` e a data for antiga, verá uma versão desatualizada (ex.: sem aba **NPCs**). Para distribuição ou teste do single-file, use [Empacotamento](#empacotamento) abaixo.

## Configuração

| Arquivo | Função |
|---------|--------|
| `config/manager.json` | Raiz do projeto, build dir, API PHP, zonas, polling |
| `config/server.json` | Bloco `admin.*` com `shared_secret` e portas 9100–9104 |
| `config/admin.key.example` | Exemplo de segredo (copiar e não versionar) |

Portas admin padrão:

| Serviço | Jogo | Admin |
|---------|------|-------|
| Auth | 8080 | 9100 |
| World | 8081 | 9101 |
| Zone N | 8082+N | 9102+N |
| Chat | 8084 | 9110 |
| Gateway | 9000 | 9104 |

> **Nota:** Chat usa porta **8084** (não 8083) para não conflitar com **Zone 1** (`8082+1`). Para múltiplas zones, ajuste `zone_instances` em `config/manager.json`.

## Abas

1. **Dashboard** — cards por serviço, gráfico CPU (LiveCharts2), totais players/zones
2. **Servers** — Start/Stop/Restart, Start All/Stop All, auto-restart
3. **Zones** — instâncias zone e players por zone
4. **Players** — lista tempo real; menu Kick/Teleport/Broadcast
5. **Logs** — tail por serviço (TabControl), filtro por nível, busca, export
6. **Accounts** — list/ban/unban via PHP admin API
7. **Items** — create/list/delete de itens master
8. **NPCs** — sub-abas **Templates** (CRUD + spawn) e **Instâncias no mundo** (grid X/Y/Z estilo Players; PHP `api/admin/*`; ver [`GUIA_COMBAT_V2_DANO_BASIC_ATTACK_NPC.md`](GUIA_COMBAT_V2_DANO_BASIC_ATTACK_NPC.md) seção 13)
9. **Config** — editor `server.json` + reload runtime
10. **GM Console** — REPL de comandos admin (Tab autocompletar, ↑↓ histórico)
11. **Scheduler** — tarefas periódicas (restart stack)

## Protocolo Admin Channel

- TCP `127.0.0.1`, framing `4 bytes LE size + JSON UTF-8`
- Handshake: `{"type":"handshake","nonce":"...","hmac":"sha256_hex_lowercase"}`
- Comando: `{"type":"command","cmd":"stats","args":{}}`

Comandos comuns: `ping`, `stats`, `set_log_level`, `reload_config`, `shutdown`.

Zone: `players`, `kick_player`, `teleport`, `broadcast`, `force_save_positions`, `zone_info`.

## GM Console — exemplos

```
stats
players
kick_player player_id=42
teleport player_id=42 x=1000 y=2000 z=100
broadcast message=Servidor reinicia em 5 min
reload_config
shutdown grace_sec=5
```

## Empacotamento

```bat
scripts\package_umbra_manager.bat
```

Gera `dist/UmbraManager/UmbraManager.exe` (single-file, self-contained ~70–100 MB) + exemplos em `config/`.

Após publish, abra:

`D:\UmbraServerV2\dist\UmbraManager\UmbraManager.exe`

Confira **Data de modificação** no Explorer — deve ser a data/hora do publish, não um build antigo.

Comando manual:

```bat
cd tools\UmbraManagerWpf
dotnet publish UmbraManager/UmbraManager.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -o ../../dist/UmbraManager
```

## Persistência local

SQLite em `%LOCALAPPDATA%\UmbraManager\manager.db`:

- `metrics` — histórico CPU/RAM por serviço (60 min)
- `audit_log` — ações do operador

## Segurança

- Admin TCP escuta em localhost; altere `shared_secret` em produção
- Login exige conta com `isadmin=1` (via `server_status.php` → `verify_admin.php`)
- Ações destrutivas pedem confirmação; audit log persistente

## Fluxo recomendado

1. Subir MySQL + Apache
2. Sincronizar API PHP no WAMP (ver abaixo se **NPCs** retornar erro HTML)
3. Abrir UmbraManager → login admin
4. Aba **Servers** → Start All
5. Acompanhar **Logs** e **Dashboard**
6. Gerenciar players em **Players** / **Zones**; spawn NPC em **NPCs → Templates**; confirmar em **NPCs → Instâncias no mundo**

## Sincronizar `umbra_api` no WAMP

A aba **Items** pode funcionar enquanto **NPCs** falha se endpoints novos não foram copiados para o Apache.

Erro típico: *"API PHP retornou HTML em vez de JSON (…/list_npc_templates.php)"* → **404** (arquivo ausente no WAMP).

**Recomendado** — script do repo (copia todos os endpoints admin NPC):

```bat
D:\UmbraServerV2\scripts\sync_umbra_api_admin_npc.bat
```

Ou copiar manualmente do repo para `C:\wamp64\www\umbra_api\` (ajuste o caminho do WAMP se necessário):

```bat
set SRC=D:\UmbraServerV2\www\umbra_api
set DST=C:\wamp64\www\umbra_api
xcopy /Y /I "%SRC%\api\admin\require_admin_auth.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\list_npc_templates.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\create_npc_template.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\update_npc_template.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\spawn_npc.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\list_npc_instances.php" "%DST%\api\admin\"
xcopy /Y /I "%SRC%\api\admin\delete_npc_instance.php" "%DST%\api\admin\"
```

Teste listagem de templates (403 JSON = endpoint OK, login admin inválido; HTML = ainda 404):

```bat
curl -X POST http://localhost/umbra_api/api/admin/list_npc_templates.php -H "Content-Type: application/json" -d "{\"admin_username\":\"SEU_ADMIN\"}"
```

Teste spawn (esperado: `"success":true,"npc_instance_id":N` — **não** HTML):

**Windows — use PowerShell (recomendado).** O `curl` do CMD/PowerShell quebra o JSON fácil; por isso o teste “falha” com *Token não fornecido*.

```powershell
cd D:\UmbraServerV2
.\scripts\test_spawn_npc.ps1 -AdminUsername "SEU_USER_ADMIN"
```

O script pede o **mesmo username** do login do UmbraManager (conta com `isadmin=1`), lista templates, faz spawn e lista instâncias.

Teste manual em PowerShell (substitua `SEU_USER_ADMIN`):

```powershell
$body = @{
  admin_username   = "SEU_USER_ADMIN"
  npc_template_id  = 1
  zone_id          = 1
  pos_x            = -1347
  pos_y            = -1310
  pos_z            = 67
  yaw              = 0
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/admin/spawn_npc.php" `
  -Method POST -ContentType "application/json; charset=utf-8" -Body $body
```

**CMD (Prompt de Comando)** — só se preferir `curl.exe` (não use `curl` sem `.exe` no PowerShell):

```bat
curl.exe -X POST http://localhost/umbra_api/api/admin/spawn_npc.php ^
  -H "Content-Type: application/json" ^
  -d "{\"admin_username\":\"SEU_USER_ADMIN\",\"npc_template_id\":1,\"zone_id\":1,\"pos_x\":-1347,\"pos_y\":-1310,\"pos_z\":67,\"yaw\":0}"
```

Respostas comuns:

| Resposta | Significado |
|----------|-------------|
| `"Token não fornecido"` | JSON não chegou (escape errado no curl) ou faltou `admin_username` |
| `"Usuário não encontrado"` | Username errado |
| `"Acesso negado. Apenas administradores"` | Conta existe mas `isadmin=0` |
| `"success":true,"npc_instance_id":N` | Spawn OK — confira em **NPCs → Instâncias no mundo** |

### Checklist E2E (spawn + instâncias)

1. Rodar `scripts\sync_umbra_api_admin_npc.bat`
2. Rebuild `zone_server` + `scripts\package_umbra_manager.bat` — abrir `dist\UmbraManager\UmbraManager.exe`
3. Matar `zone_server.exe` órfãos (Task Manager) → UmbraManager **Stop All** → **Start** só **Zone 0**
4. Verificar aba **Servers**: `zone_0` = Running, Admin TCP = **OK** (não `handshake timeout`)
5. PIE conectado na porta **8082** (zone 0)
6. **NPCs → Templates** → **Zone ID = 0** → **Spawn na zone** (ou **Copiar posição do player**)
7. **NPCs → Instâncias no mundo** → **Atualizar instâncias** → linha com X/Y/Z
8. GM Console: `list_npcs` em `zone_0` → `count` > 0; log `logs/zone_server.log`: `hot spawn NPC instance`
9. Dummy aparece no PIE **sem restart** (opcode 100); matar → opcode 101; respawn ~10s → opcode 100

> **zone_id=0** no MySQL deve coincidir com `zone_server.exe 0` (ou sem argumento). Chat usa porta **8084** para não conflitar com zone_1 (8083).

Tabelas NPC exigem `combat_v2.sql` no MySQL — ver [`GUIA_COMBAT_V2_DANO_BASIC_ATTACK_NPC.md`](GUIA_COMBAT_V2_DANO_BASIC_ATTACK_NPC.md).

## Impacto nos servidores C++

- A lib `umbra_admin` em `src/admin/` continua linkada nos exes (`auth_server`, `zone_server`, etc.)
- O manager WPF é apenas **cliente** do Admin Channel; não altera binários dos servidores
- `WITH_MANAGER` foi removido do `CMakeLists.txt` (build Qt descontinuado)
