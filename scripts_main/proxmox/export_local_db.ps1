# Exporta o banco umbra_eternum do MySQL local para deploy no Proxmox.
# Uso: .\scripts_main\proxmox\export_local_db.ps1 [-Password 'sua_senha']

param(
    [string]$Password = $env:MYSQL_ROOT_PASSWORD,
    [string]$OutputPath = "$PSScriptRoot\..\..\umbra_eternum_proxmox_dump.sql",
    [string]$MySqlBin = "C:\Program Files\MySQL\MySQL Server 8.0\bin"
)

$ErrorActionPreference = "Stop"

if (-not $Password) {
    $Password = "!Mister4126"
}

$mysqldump = Join-Path $MySqlBin "mysqldump.exe"
if (-not (Test-Path $mysqldump)) {
    throw "mysqldump nao encontrado em: $mysqldump"
}

$env:MYSQL_PWD = $Password
try {
    & $mysqldump -u root `
        --databases umbra_eternum `
        --single-transaction `
        --routines --triggers --events `
        --hex-blob `
        --set-gtid-purged=OFF `
        --default-character-set=utf8mb4 `
        | Out-File -FilePath $OutputPath -Encoding utf8

    $tableCount = (Select-String -Path $OutputPath -Pattern "^CREATE TABLE").Count
    $sizeKb = [math]::Round((Get-Item $OutputPath).Length / 1KB, 1)
    Write-Host "Dump OK: $OutputPath ($sizeKb KB, $tableCount tabelas)"
}
finally {
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
}
