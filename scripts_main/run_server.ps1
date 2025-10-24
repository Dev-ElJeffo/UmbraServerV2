# Run UmbraEternum Server
# PowerShell launcher script

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  UmbraEternum Server Launcher" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Ir para diretório do executável
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$exeDir = Join-Path $scriptDir "build\bin\Release"

if (-not (Test-Path $exeDir)) {
    Write-Host "❌ ERRO: Diretório build\bin\Release não encontrado!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Execute primeiro:" -ForegroundColor Yellow
    Write-Host "  mkdir build && cd build" -ForegroundColor White
    Write-Host "  cmake .." -ForegroundColor White
    Write-Host "  cmake --build . --config Release" -ForegroundColor White
    Read-Host "Pressione Enter para sair"
    exit 1
}

Set-Location $exeDir

if (-not (Test-Path "umbra_server.exe")) {
    Write-Host "❌ ERRO: umbra_server.exe não encontrado!" -ForegroundColor Red
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "[INFO] Verificando configurações..." -ForegroundColor Yellow

# Criar diretórios necessários
if (-not (Test-Path "config")) {
    New-Item -ItemType Directory -Path "config" | Out-Null
    Write-Host "  ✓ Diretório config criado" -ForegroundColor Green
}

if (-not (Test-Path "logs")) {
    New-Item -ItemType Directory -Path "logs" | Out-Null
    Write-Host "  ✓ Diretório logs criado" -ForegroundColor Green
}

# Copiar arquivos de configuração se não existirem
$configFiles = @(
    @{Source="..\..\..\config\server.json"; Dest="config\server.json"},
    @{Source="..\..\..\config\db.json"; Dest="config\db.json"},
    @{Source="..\..\..\config\jwt_secret.key"; Dest="config\jwt_secret.key"}
)

foreach ($file in $configFiles) {
    if (-not (Test-Path $file.Dest)) {
        Copy-Item $file.Source $file.Dest -Force
        Write-Host "  ✓ Copiado: $(Split-Path $file.Dest -Leaf)" -ForegroundColor Green
    }
}

Write-Host "[OK] Configurações prontas" -ForegroundColor Green
Write-Host ""

# Verificar MySQL
Write-Host "[INFO] Verificando MySQL..." -ForegroundColor Yellow
$mysqlService = Get-Service -Name "MySQL80" -ErrorAction SilentlyContinue
if ($mysqlService) {
    if ($mysqlService.Status -eq "Running") {
        Write-Host "  ✓ MySQL80 rodando" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ MySQL80 parado - tentando iniciar..." -ForegroundColor Yellow
        try {
            Start-Service MySQL80 -ErrorAction Stop
            Write-Host "  ✓ MySQL80 iniciado" -ForegroundColor Green
        } catch {
            Write-Host "  ⚠ Não foi possível iniciar MySQL80 automaticamente" -ForegroundColor Yellow
            Write-Host "    Inicie manualmente: Start-Service MySQL80" -ForegroundColor Yellow
        }
    }
} else {
    Write-Host "  ⚠ Serviço MySQL80 não encontrado" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Iniciando servidor..." -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Logs em tempo real: logs\umbra_server.log" -ForegroundColor Yellow
Write-Host "Pressione Ctrl+C para parar" -ForegroundColor Yellow
Write-Host ""

# Executar servidor
& .\umbra_server.exe

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Servidor encerrado" -ForegroundColor Yellow
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Read-Host "Pressione Enter para sair"

