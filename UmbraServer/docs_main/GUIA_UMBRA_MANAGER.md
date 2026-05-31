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
| Chat | 8083 | 9103 |
| Gateway | 9000 | 9104 |

## Abas

1. **Dashboard** — cards por serviço, gráfico CPU (LiveCharts2), totais players/zones
2. **Servers** — Start/Stop/Restart, Start All/Stop All, auto-restart
3. **Zones** — instâncias zone e players por zone
4. **Players** — lista tempo real; menu Kick/Teleport/Broadcast
5. **Logs** — tail por serviço (TabControl), filtro por nível, busca, export
6. **Accounts** — list/ban/unban via PHP admin API
7. **Items** — create/list/delete de itens master
8. **Config** — editor `server.json` + reload runtime
9. **GM Console** — REPL de comandos admin (Tab autocompletar, ↑↓ histórico)
10. **Scheduler** — tarefas periódicas (restart stack)

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
2. Abrir UmbraManager → login admin
3. Aba **Servers** → Start All
4. Acompanhar **Logs** e **Dashboard**
5. Gerenciar players em **Players** / **Zones**

## Impacto nos servidores C++

- A lib `umbra_admin` em `src/admin/` continua linkada nos exes (`auth_server`, `zone_server`, etc.)
- O manager WPF é apenas **cliente** do Admin Channel; não altera binários dos servidores
- `WITH_MANAGER` foi removido do `CMakeLists.txt` (build Qt descontinuado)
