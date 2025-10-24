# Fix WAMP MariaDB Conflict
# Execute como Administrador: Right-click → Run as Administrator

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Fix WAMP Port 3306 Conflict" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "❌ ERRO: Execute este script como Administrador!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Como executar:" -ForegroundColor Yellow
    Write-Host "1. Right-click no arquivo" -ForegroundColor Yellow
    Write-Host "2. 'Executar com PowerShell' ou 'Run as Administrator'" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "✓ Executando como Administrador" -ForegroundColor Green
Write-Host ""

Write-Host "[1/5] Identificando conflitos..." -ForegroundColor Yellow
Write-Host ""

# Ver processos usando porta 3306
$processes = netstat -ano | findstr ":3306" | findstr "LISTENING"
if ($processes) {
    Write-Host "Processos usando porta 3306:" -ForegroundColor Cyan
    $processes | ForEach-Object { Write-Host "  $_" -ForegroundColor White }
} else {
    Write-Host "  Nenhum processo usando porta 3306" -ForegroundColor Green
}

Write-Host ""
Write-Host "[2/5] Parando WAMP MariaDB..." -ForegroundColor Yellow

try {
    $service = Get-Service -Name "wampmariadb64" -ErrorAction SilentlyContinue
    if ($service) {
        if ($service.Status -eq "Running") {
            Stop-Service -Name "wampmariadb64" -Force
            Write-Host "  ✓ WAMP MariaDB parado" -ForegroundColor Green
        } else {
            Write-Host "  ✓ WAMP MariaDB já está parado" -ForegroundColor Green
        }
        
        # Desabilitar startup automático
        Set-Service -Name "wampmariadb64" -StartupType Disabled
        Write-Host "  ✓ WAMP MariaDB desabilitado" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Serviço wampmariadb64 não encontrado" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  ❌ Erro ao parar wampmariadb64: $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "[3/5] Garantindo MySQL80 rodando..." -ForegroundColor Yellow

try {
    $mysql80 = Get-Service -Name "MySQL80" -ErrorAction Stop
    if ($mysql80.Status -ne "Running") {
        Start-Service -Name "MySQL80"
        Start-Sleep -Seconds 2
        Write-Host "  ✓ MySQL80 iniciado" -ForegroundColor Green
    } else {
        Write-Host "  ✓ MySQL80 já está rodando" -ForegroundColor Green
    }
} catch {
    Write-Host "  ⚠ MySQL80 não encontrado ou erro: $_" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[4/5] Verificando Apache WAMP..." -ForegroundColor Yellow

try {
    $apache = Get-Service -Name "wampapache64" -ErrorAction SilentlyContinue
    if ($apache) {
        if ($apache.Status -eq "Running") {
            Write-Host "  ✓ Apache WAMP rodando (porta 80)" -ForegroundColor Green
        } else {
            Write-Host "  ⚠ Apache WAMP parado" -ForegroundColor Yellow
            Write-Host "    Inicie via ícone WAMP na bandeja" -ForegroundColor Yellow
        }
    }
} catch {
    Write-Host "  ⚠ Apache WAMP não encontrado" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[5/5] Verificando portas..." -ForegroundColor Yellow
Write-Host ""

# Verificar porta 80
$port80 = netstat -ano | findstr ":80 " | findstr "LISTENING"
if ($port80) {
    Write-Host "  ✓ Porta 80  (Apache): LISTENING" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Porta 80  (Apache): NOT LISTENING" -ForegroundColor Yellow
}

# Verificar porta 3306
$port3306 = netstat -ano | findstr ":3306" | findstr "LISTENING"
if ($port3306) {
    $lines = ($port3306 | Measure-Object -Line).Lines
    if ($lines -eq 1 -or $lines -eq 2) {
        Write-Host "  ✓ Porta 3306 (MySQL): LISTENING (sem conflito)" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Porta 3306: $lines processos (CONFLITO!)" -ForegroundColor Red
    }
} else {
    Write-Host "  ⚠ Porta 3306 (MySQL): NOT LISTENING" -ForegroundColor Yellow
}

# Verificar porta 8080
$port8080 = netstat -ano | findstr ":8080" | findstr "LISTENING"
if ($port8080) {
    Write-Host "  ✓ Porta 8080 (C++ Server): LISTENING" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Porta 8080: NOT LISTENING (servidor parado?)" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✅ DIAGNÓSTICO COMPLETO!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Status dos Serviços:" -ForegroundColor Cyan
Get-Service -Name "MySQL80","wampapache64","wampmariadb64" -ErrorAction SilentlyContinue | 
    Format-Table Name, Status, StartType -AutoSize

Write-Host ""
Write-Host "Configuração Recomendada:" -ForegroundColor Cyan
Write-Host "  ✓ MySQL80:        Running  (porta 3306)" -ForegroundColor Green
Write-Host "  ✓ Apache WAMP:    Running  (porta 80)" -ForegroundColor Green
Write-Host "  ✓ MariaDB WAMP:   Disabled (evitar conflito)" -ForegroundColor Green
Write-Host ""

Write-Host "Próximos passos:" -ForegroundColor Cyan
Write-Host "  1. Reinicie o WAMP (ícone na bandeja)" -ForegroundColor White
Write-Host "  2. WAMP deve ficar verde" -ForegroundColor White
Write-Host "  3. Acesse: http://localhost/" -ForegroundColor White
Write-Host "  4. Copie os arquivos PHP para C:\wamp64\www\umbra_api\" -ForegroundColor White
Write-Host ""

Read-Host "Pressione Enter para sair"

