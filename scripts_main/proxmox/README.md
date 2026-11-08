# Deploy Proxmox — UmbraEternum

Scripts para exportar o MySQL local e configurar os containers `pve-database` (192.168.3.11) e `pve-compiler` (192.168.3.10).

## Arquitetura

| Host | IP | Função |
|------|-----|--------|
| pve-database | 192.168.3.11 | MariaDB + Apache/PHP (`www/umbra_api`) |
| pve-compiler | 192.168.3.10 | Servidor C++ (`umbra_server`) |
| Windows (UE) | LAN | Cliente — HTTP → .11, TCP → .10 |

## Deploy automático (Windows)

```powershell
$env:PROXMOX_SSH_PASSWORD = 'SUA_SENHA_SSH'
.\scripts_main\proxmox\deploy_proxmox.ps1
```

Opções: `-SkipExport` (reutiliza dump existente), `-RepoPathRemote /root/UmbraServerV2`

## Passos manuais

### 1. Exportar MySQL local

```powershell
.\scripts_main\proxmox\export_local_db.ps1
```

Gera `umbra_eternum_proxmox_dump.sql` na raiz do repo.

### 2. pve-database (se deploy automático falhar)

```bash
bash /tmp/setup_database_container.sh /root/UmbraServerV2 '!Mister4126' '!Mister4126'
```

### 3. pve-compiler — server.json

Se SSH no compiler usar outra senha, no console do container:

```bash
curl -o /root/UmbraServerV2/config/server.json http://192.168.3.11/server.proxmox.json
cd /root/UmbraServerV2/build/bin && ./umbra_server
```

Ou: `bash /tmp/setup_compiler_container.sh /root/UmbraServerV2 '!Mister4126'`

### 4. Cliente UE

Em `UmbraGameInstance` (já configurado no código):

- `ServerURL` = `http://192.168.3.11/umbra_api`
- `GameServerIP` = `192.168.3.10`

## Validar

```powershell
.\scripts_main\proxmox\validate_proxmox.ps1
curl http://192.168.3.11/umbra_api/api/test.php
```

## Reexportar após mudanças no schema

```powershell
.\scripts_main\proxmox\export_local_db.ps1
$env:PROXMOX_SSH_PASSWORD = '...'
.\scripts_main\proxmox\deploy_proxmox.ps1
```
