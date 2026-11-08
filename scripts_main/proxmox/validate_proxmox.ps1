# Valida ambiente Proxmox apos deploy.
# Uso: .\scripts_main\proxmox\validate_proxmox.ps1

param(
    [string]$DatabaseHost = "192.168.3.11",
    [string]$CompilerHost = "192.168.3.10"
)

$ErrorActionPreference = "Continue"
$ok = $true

function Test-TargetPort {
    param([string]$TargetHost, [int]$Port)
    (Test-NetConnection -ComputerName $TargetHost -Port $Port -WarningAction SilentlyContinue).TcpTestSucceeded
}

Write-Host "==> Portas"
foreach ($p in @(22, 80, 3306)) {
    $r = Test-TargetPort -TargetHost $DatabaseHost -Port $p
    Write-Host "  $DatabaseHost`:$p -> $r"
    if ($p -eq 80 -and -not $r) { $ok = $false }
}
foreach ($p in @(22, 9000, 8080, 8082)) {
    $r = Test-TargetPort -TargetHost $CompilerHost -Port $p
    Write-Host "  $CompilerHost`:$p -> $r"
}

Write-Host "==> API test.php"
try {
    $api = Invoke-RestMethod -Uri "http://${DatabaseHost}/umbra_api/api/test.php" -TimeoutSec 15
    if ($api.success) {
        Write-Host "  OK: schema=$($api.schema_version) accounts=$($api.stats.accounts) players=$($api.stats.players)"
    } else {
        Write-Host "  FALHA: $($api.message)"
        $ok = $false
    }
} catch {
    Write-Host "  FALHA: $_"
    $ok = $false
}

Write-Host "==> Dump local"
$dump = Join-Path (Resolve-Path "$PSScriptRoot\..\..") "umbra_eternum_proxmox_dump.sql"
if (Test-Path $dump) {
    $tables = (Select-String -Path $dump -Pattern "^CREATE TABLE").Count
    Write-Host "  OK: $tables tabelas em $dump"
} else {
    Write-Host "  FALHA: dump nao encontrado"
    $ok = $false
}

if ($ok) {
    Write-Host "`nValidacao: PASSOU"
    exit 0
}
Write-Host "`nValidacao: FALHOU (rode deploy_proxmox.ps1 com PROXMOX_SSH_PASSWORD)"
exit 1
