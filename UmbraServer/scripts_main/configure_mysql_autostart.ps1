# UmbraEternum - Configurar MySQL para Iniciar Automaticamente
# Execute como Administrador: Right-click → Run as Administrator

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Configurar MySQL Auto-Start" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar se está rodando como admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "❌ ERRO: Execute este script como Administrador!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Como executar:" -ForegroundColor Yellow
    Write-Host "1. Clique com botão direito no arquivo" -ForegroundColor Yellow
    Write-Host "2. Selecione 'Executar como Administrador'" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "✓ Executando como Administrador" -ForegroundColor Green
Write-Host ""

# Verificar se serviço existe
try {
    $mysqlService = Get-Service -Name "MySQL80" -ErrorAction Stop
    Write-Host "✓ Serviço MySQL80 encontrado" -ForegroundColor Green
    Write-Host ""
} catch {
    Write-Host "❌ ERRO: Serviço MySQL80 não encontrado!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Verifique se MySQL 8.0 está instalado em:" -ForegroundColor Yellow
    Write-Host "  C:\Program Files\MySQL\MySQL Server 8.0" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Pressione Enter para sair"
    exit 1
}

# Verificar tipo de inicialização atual
$currentStartType = (Get-WmiObject Win32_Service -Filter "Name='MySQL80'").StartMode
Write-Host "[INFO] Tipo de inicialização atual: $currentStartType" -ForegroundColor Yellow
Write-Host ""

# Configurar para iniciar automaticamente
Write-Host "[1/2] Configurando MySQL para iniciar automaticamente..." -ForegroundColor Yellow
try {
    Set-Service -Name "MySQL80" -StartupType Automatic
    Write-Host "✓ MySQL configurado para iniciar automaticamente!" -ForegroundColor Green
    Write-Host ""
} catch {
    Write-Host "❌ ERRO: Falha ao configurar auto-start" -ForegroundColor Red
    Write-Host "  $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    Read-Host "Pressione Enter para sair"
    exit 1
}

# Iniciar serviço se não estiver rodando
Write-Host "[2/2] Iniciando MySQL (se não estiver rodando)..." -ForegroundColor Yellow
if ($mysqlService.Status -ne "Running") {
    try {
        Start-Service -Name "MySQL80"
        Start-Sleep -Seconds 3
        Write-Host "✓ MySQL iniciado!" -ForegroundColor Green
        Write-Host ""
    } catch {
        Write-Host "⚠ AVISO: MySQL não pôde ser iniciado automaticamente" -ForegroundColor Yellow
        Write-Host "  Mas está configurado para iniciar no próximo boot" -ForegroundColor Yellow
        Write-Host ""
    }
} else {
    Write-Host "✓ MySQL já está rodando" -ForegroundColor Green
    Write-Host ""
}

# Verificar configuração final
$finalStartType = (Get-WmiObject Win32_Service -Filter "Name='MySQL80'").StartMode
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✓ Configuração Completa!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Tipo de inicialização: $finalStartType" -ForegroundColor White
Write-Host "Status atual: $($mysqlService.Status)" -ForegroundColor White
Write-Host ""
Write-Host "MySQL agora iniciará automaticamente ao reiniciar o computador!" -ForegroundColor Green
Write-Host ""

Read-Host "Pressione Enter para sair"

