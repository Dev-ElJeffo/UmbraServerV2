# UmbraEternum - Iniciar MySQL 8.0
# Execute como Administrador: Right-click → Run as Administrator

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  UmbraEternum - Iniciar MySQL" -ForegroundColor Cyan
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

# Verificar status atual
if ($mysqlService.Status -eq "Running") {
    Write-Host "[INFO] MySQL já está rodando" -ForegroundColor Yellow
    Write-Host ""
} else {
    Write-Host "[1/2] Iniciando MySQL Server..." -ForegroundColor Yellow
    try {
        Start-Service -Name "MySQL80"
        Start-Sleep -Seconds 3
        
        # Verificar se iniciou
        $mysqlService.Refresh()
        if ($mysqlService.Status -eq "Running") {
            Write-Host "✓ MySQL iniciado com sucesso!" -ForegroundColor Green
            Write-Host ""
        } else {
            Write-Host "❌ ERRO: MySQL não iniciou corretamente" -ForegroundColor Red
            Write-Host ""
            Read-Host "Pressione Enter para sair"
            exit 1
        }
    } catch {
        Write-Host "❌ ERRO: Falha ao iniciar MySQL" -ForegroundColor Red
        Write-Host "  $($_.Exception.Message)" -ForegroundColor Red
        Write-Host ""
        Write-Host "Tente iniciar manualmente:" -ForegroundColor Yellow
        Write-Host "  Win+R -> services.msc -> MySQL80 -> Start" -ForegroundColor Yellow
        Write-Host ""
        Read-Host "Pressione Enter para sair"
        exit 1
    }
}

# Testar conexão
Write-Host "[2/2] Testando conexão..." -ForegroundColor Yellow
Write-Host ""

$mysqlPath = "C:\Program Files\MySQL\MySQL Server 8.0\bin"
if (Test-Path "$mysqlPath\mysql.exe") {
    try {
        $testResult = & "$mysqlPath\mysql.exe" -u root -p -e "SELECT VERSION();" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ MySQL conectado com sucesso!" -ForegroundColor Green
            Write-Host ""
        } else {
            Write-Host "⚠ AVISO: Não foi possível testar conexão automaticamente" -ForegroundColor Yellow
            Write-Host "  (Pode ser necessário digitar a senha)" -ForegroundColor Yellow
            Write-Host ""
        }
    } catch {
        Write-Host "⚠ AVISO: Não foi possível testar conexão" -ForegroundColor Yellow
        Write-Host ""
    }
} else {
    Write-Host "⚠ AVISO: mysql.exe não encontrado em $mysqlPath" -ForegroundColor Yellow
    Write-Host ""
}

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✓ MySQL está pronto para uso!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Read-Host "Pressione Enter para sair"

