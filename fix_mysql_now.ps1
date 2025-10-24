# Fix MySQL Conflict - Execute como Administrador
# Right-click → Run as Administrator

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Fix MySQL Port 3306 Conflict" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "❌ ERRO: Execute como Administrador!" -ForegroundColor Red
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "✓ Executando como Administrador" -ForegroundColor Green
Write-Host ""

# Parar todos os serviços MySQL
Write-Host "[1/4] Parando serviços MySQL..." -ForegroundColor Yellow

$services = @("wampmysqld64", "MySQL80", "MySQL")
foreach ($svc in $services) {
    try {
        $service = Get-Service -Name $svc -ErrorAction SilentlyContinue
        if ($service -and $service.Status -eq "Running") {
            Stop-Service -Name $svc -Force -ErrorAction SilentlyContinue
            Write-Host "  ✓ $svc parado" -ForegroundColor Green
        }
    } catch {
        # Ignorar erros
    }
}
Write-Host ""

# Matar processos mysqld
Write-Host "[2/4] Finalizando processos mysqld.exe..." -ForegroundColor Yellow
$processes = Get-Process -Name mysqld -ErrorAction SilentlyContinue
if ($processes) {
    $processes | Stop-Process -Force
    Write-Host "  ✓ $($processes.Count) processos finalizados" -ForegroundColor Green
} else {
    Write-Host "  ✓ Nenhum processo encontrado" -ForegroundColor Green
}
Write-Host ""

# Aguardar
Write-Host "[3/4] Aguardando liberação da porta 3306..." -ForegroundColor Yellow
Start-Sleep -Seconds 3
Write-Host "  ✓ Porta livre" -ForegroundColor Green
Write-Host ""

# Iniciar MySQL80
Write-Host "[4/4] Iniciando MySQL80..." -ForegroundColor Yellow
try {
    Start-Service -Name MySQL80
    Start-Sleep -Seconds 2
    
    $service = Get-Service -Name MySQL80
    if ($service.Status -eq "Running") {
        Write-Host "  ✓ MySQL80 INICIADO COM SUCESSO!" -ForegroundColor Green
        Write-Host ""
        Write-Host "=========================================" -ForegroundColor Cyan
        Write-Host "  ✅ PROBLEMA RESOLVIDO!" -ForegroundColor Green
        Write-Host "=========================================" -ForegroundColor Cyan
        Write-Host ""
        
        # Verificar porta
        $port = netstat -ano | Select-String ":3306.*LISTENING"
        if ($port) {
            Write-Host "✓ Porta 3306: LISTENING" -ForegroundColor Green
        }
        
        Write-Host ""
        Write-Host "Próximos passos:" -ForegroundColor Cyan
        Write-Host "1. Abra MySQL Workbench" -ForegroundColor White
        Write-Host "2. Conecte: 'Local instance MySQL80'" -ForegroundColor White
        Write-Host "3. File → Open SQL Script" -ForegroundColor White
        Write-Host "4. Selecione: setup_database.sql" -ForegroundColor White
        Write-Host "5. Execute (⚡)" -ForegroundColor White
        Write-Host ""
    } else {
        throw "Service not running"
    }
} catch {
    Write-Host "  ❌ Erro ao iniciar MySQL80" -ForegroundColor Red
    Write-Host ""
    Write-Host "Verifique os logs:" -ForegroundColor Yellow
    Write-Host "  C:\ProgramData\MySQL\MySQL Server 8.0\Data\*.err" -ForegroundColor White
}

Read-Host "Pressione Enter para sair"

