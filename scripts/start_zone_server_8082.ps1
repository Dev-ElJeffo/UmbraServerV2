# Script PowerShell para iniciar Zone Server na porta 8082 (Zone ID 0)
# Este script deve ser executado ANTES de iniciar o cliente Unreal

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Iniciando Zone Server (Porta 8082)" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar se o executável existe
$ZoneServerPath = "build\bin\Release\zone_server.exe"
if (-not (Test-Path $ZoneServerPath)) {
    Write-Host "ERRO: zone_server.exe nao encontrado!" -ForegroundColor Red
    Write-Host "Por favor, compile o projeto primeiro." -ForegroundColor Yellow
    pause
    exit 1
}

# Criar diretório de logs se não existir
$LogsDir = "build\bin\Release\logs"
if (-not (Test-Path $LogsDir)) {
    New-Item -ItemType Directory -Path $LogsDir -Force | Out-Null
}

Write-Host "Iniciando Zone Server com Zone ID 0 (porta 8082)..." -ForegroundColor Green
Write-Host ""
Write-Host "IMPORTANTE:" -ForegroundColor Yellow
Write-Host "- Porta configurada: 8082" -ForegroundColor White
Write-Host "- WebSocket URL: ws://127.0.0.1:8082" -ForegroundColor White
Write-Host "- Mantenha esta janela aberta enquanto testar o cliente" -ForegroundColor Yellow
Write-Host ""
Write-Host "Pressione Ctrl+C para parar o servidor" -ForegroundColor Cyan
Write-Host ""

# Executar o Zone Server com Zone ID 0 (porta 8082)
Set-Location "build\bin\Release"
& .\zone_server.exe 0

