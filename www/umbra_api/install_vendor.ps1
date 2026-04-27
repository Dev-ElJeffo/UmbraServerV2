# Instala dependencias em www/umbra_api (firebase/php-jwt) via Composer.
# Executar na pasta da API: powershell -ExecutionPolicy Bypass -File install_vendor.ps1
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

Write-Host "=== Umbra API - Composer install ===" -ForegroundColor Cyan
Write-Host "Pasta: $(Get-Location)"

function Find-PhpExe {
    $cmd = Get-Command php -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $roots = @(
        'C:\wamp64\bin\php',
        'C:\wamp\bin\php',
        'D:\wamp64\bin\php'
    )
    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        $dirs = Get-ChildItem -Path $root -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -like 'php*' }
        foreach ($d in ($dirs | Sort-Object Name -Descending)) {
            $exe = Join-Path $d.FullName 'php.exe'
            if (Test-Path $exe) { return $exe }
        }
    }
    return $null
}

$phpExe = Find-PhpExe
if (-not $phpExe) {
    Write-Error "php.exe nao encontrado. Adiciona PHP do WAMP ao PATH ou instala PHP CLI."
}

Write-Host "Usando: $phpExe"
& $phpExe -v

$composerPhar = Join-Path $PSScriptRoot 'composer.phar'
if (-not (Test-Path $composerPhar)) {
    Write-Host "A descarregar composer.phar..."
    Invoke-WebRequest -Uri 'https://getcomposer.org/download/latest-stable/composer.phar' -OutFile $composerPhar -UseBasicParsing
}

if (-not (Test-Path (Join-Path $PSScriptRoot 'composer.json'))) {
    Write-Error "composer.json nao encontrado em $PSScriptRoot"
}

Write-Host "A executar composer install..."
& $phpExe $composerPhar install --no-interaction

$autoload = Join-Path $PSScriptRoot 'vendor\autoload.php'
if (-not (Test-Path $autoload)) {
    Write-Error "vendor\autoload.php nao foi criado. Verifica a saida do Composer acima."
}

Write-Host "OK: dependencias instaladas. Reinicia o Apache." -ForegroundColor Green
