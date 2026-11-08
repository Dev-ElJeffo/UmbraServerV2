# Deploy completo Proxmox: exporta DB local, transfere e configura os containers.
# Uso:
#   $env:PROXMOX_SSH_PASSWORD = 'senha_root_ssh'
#   .\scripts_main\proxmox\deploy_proxmox.ps1
#
# Variaveis opcionais:
#   PROXMOX_SSH_PASSWORD  - senha SSH root dos containers
#   MYSQL_ROOT_PASSWORD   - senha root MariaDB (padrao: !Mister4126)
#   UMBRA_SERVER_PASSWORD - senha umbra_server/umbra_api (padrao: !Mister4126)
#   REPO_PATH_REMOTE      - caminho do repo nos containers (padrao: /root/UmbraServerV2)

param(
    [string]$DatabaseHost = "192.168.3.11",
    [string]$CompilerHost = "192.168.3.10",
    [string]$SshPassword = $env:PROXMOX_SSH_PASSWORD,
    [string]$MysqlRootPassword = $(if ($env:MYSQL_ROOT_PASSWORD) { $env:MYSQL_ROOT_PASSWORD } else { "!Mister4126" }),
    [string]$UmbraServerPassword = $(if ($env:UMBRA_SERVER_PASSWORD) { $env:UMBRA_SERVER_PASSWORD } else { "!Mister4126" }),
    [string]$RepoPathRemote = $(if ($env:REPO_PATH_REMOTE) { $env:REPO_PATH_REMOTE } else { "/root/UmbraServerV2" }),
    [switch]$SkipExport
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path "$PSScriptRoot\..\.."
$DumpFile = Join-Path $Root "umbra_eternum_proxmox_dump.sql"
$Plink = "C:\Program Files\PuTTY\plink.exe"
$Pscp = "C:\Program Files\PuTTY\pscp.exe"

function Invoke-Remote {
    param([string]$TargetHost, [string]$Command)
    if ($SshPassword) {
        if (-not (Test-Path $Plink)) { throw "PuTTY plink nao encontrado: $Plink" }
        $hostKeyArg = @()
        if ($TargetHost -eq "192.168.3.10") {
            $hostKeyArg = @("-hostkey", "SHA256:cpRRQ90SThWrG2QLj+kH8AKCVoKGWCb0t/2L5ZQ82NY")
        }
        & $Plink -ssh "root@$TargetHost" -pw $SshPassword -batch @hostKeyArg $Command
    } else {
        ssh -o StrictHostKeyChecking=accept-new "root@$TargetHost" $Command
    }
}

function Copy-Remote {
    param([string]$TargetHost, [string]$Local, [string]$Remote)
    if ($SshPassword) {
        if (-not (Test-Path $Pscp)) { throw "PuTTY pscp nao encontrado: $Pscp" }
        $hostKeyArg = @()
        if ($TargetHost -eq "192.168.3.10") {
            $hostKeyArg = @("-hostkey", "SHA256:cpRRQ90SThWrG2QLj+kH8AKCVoKGWCb0t/2L5ZQ82NY")
        }
        & $Pscp -pw $SshPassword @hostKeyArg $Local "root@${TargetHost}:$Remote"
    } else {
        scp -o StrictHostKeyChecking=accept-new $Local "root@${TargetHost}:$Remote"
    }
}

if (-not $SkipExport) {
    Write-Host "==> Exportando MySQL local..."
    & "$PSScriptRoot\export_local_db.ps1" -Password $MysqlRootPassword -OutputPath $DumpFile
}

if (-not (Test-Path $DumpFile)) {
    throw "Dump nao encontrado: $DumpFile"
}

Write-Host "==> Transferindo dump para $DatabaseHost..."
Copy-Remote -TargetHost $DatabaseHost -Local $DumpFile -Remote "/tmp/umbra_eternum_proxmox_dump.sql"

Write-Host "==> Transferindo scripts de setup..."
Copy-Remote -TargetHost $DatabaseHost -Local "$PSScriptRoot\setup_database_container.sh" -Remote "/tmp/setup_database_container.sh"
Copy-Remote -TargetHost $CompilerHost -Local "$PSScriptRoot\setup_compiler_container.sh" -Remote "/tmp/setup_compiler_container.sh"

Write-Host "==> Configurando pve-database ($DatabaseHost)..."
Invoke-Remote -TargetHost $DatabaseHost "chmod +x /tmp/setup_database_container.sh && bash /tmp/setup_database_container.sh '$RepoPathRemote' '$MysqlRootPassword' '$UmbraServerPassword'"

Write-Host "==> Configurando pve-compiler ($CompilerHost)..."
Invoke-Remote -TargetHost $CompilerHost "chmod +x /tmp/setup_compiler_container.sh && bash /tmp/setup_compiler_container.sh '$RepoPathRemote' '$UmbraServerPassword'"

Write-Host "==> Testes finais..."
try {
    $api = Invoke-RestMethod -Uri "http://${DatabaseHost}/umbra_api/api/test.php" -TimeoutSec 15
    Write-Host "API test.php: success=$($api.success) schema=$($api.schema_version)"
} catch {
    Write-Warning "API test falhou: $_"
}

Write-Host ""
Write-Host "Deploy concluido."
Write-Host "  API:  http://${DatabaseHost}/umbra_api"
Write-Host "  C++:  ${CompilerHost}:9000 (Gateway)"
Write-Host "  UE:   ServerURL=http://${DatabaseHost}/umbra_api  GameServerIP=${CompilerHost}"
