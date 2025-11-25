# =========================================
# Iniciar MySQL Server - Múltiplas Tentativas
# =========================================

Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "  INICIAR MYSQL - UmbraEternum" -ForegroundColor Cyan
Write-Host "=========================================`n" -ForegroundColor Cyan

# Verificar Admin
$currentUser = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdmin = $currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "[ERRO] Execute como Administrador!" -ForegroundColor Red
    pause
    exit 1
}

Write-Host "[OK] Executando como Administrador`n" -ForegroundColor Green

# =========================================
# MÉTODO 1: PARAR WAMP PRIMEIRO
# =========================================
Write-Host "[1/4] Verificando conflito com WAMP..." -ForegroundColor Yellow

$wampProcess = Get-Process -Name "wampmanager*", "mysqld" -ErrorAction SilentlyContinue
if ($wampProcess) {
    Write-Host "[AVISO] WAMP/MySQL detectado rodando!" -ForegroundColor Red
    Write-Host "        Parando WAMP para liberar a porta 3306..." -ForegroundColor Yellow
    
    Stop-Process -Name "wampmanager*" -Force -ErrorAction SilentlyContinue
    Stop-Process -Name "mysqld" -Force -ErrorAction SilentlyContinue
    
    Start-Sleep -Seconds 3
    Write-Host "[OK] WAMP parado" -ForegroundColor Green
} else {
    Write-Host "[OK] Nenhum conflito detectado" -ForegroundColor Green
}

# =========================================
# MÉTODO 2: ENCONTRAR SERVIÇO MYSQL
# =========================================
Write-Host "`n[2/4] Procurando serviço MySQL..." -ForegroundColor Yellow

$mysqlServices = Get-Service | Where-Object { 
    $_.Name -like "*mysql*" -or $_.DisplayName -like "*mysql*" 
} | Where-Object {
    $_.Name -notlike "*wamp*"
}

if ($mysqlServices.Count -eq 0) {
    Write-Host "[ERRO] Nenhum serviço MySQL encontrado!" -ForegroundColor Red
    Write-Host "       Possíveis causas:" -ForegroundColor Yellow
    Write-Host "       1. MySQL não está instalado" -ForegroundColor Yellow
    Write-Host "       2. MySQL não foi registrado como serviço" -ForegroundColor Yellow
    Write-Host "`n[SOLUÇÃO] Execute: setup_mysql.ps1" -ForegroundColor Green
    pause
    exit 1
}

Write-Host "[OK] Serviço encontrado: $($mysqlServices[0].DisplayName)" -ForegroundColor Green
Write-Host "     Nome: $($mysqlServices[0].Name)" -ForegroundColor Cyan
Write-Host "     Status atual: $($mysqlServices[0].Status)" -ForegroundColor Cyan

# =========================================
# MÉTODO 3: INICIAR SERVIÇO
# =========================================
Write-Host "`n[3/4] Tentando iniciar MySQL..." -ForegroundColor Yellow

$serviceName = $mysqlServices[0].Name

try {
    # Verificar se já está rodando
    $service = Get-Service $serviceName
    if ($service.Status -eq "Running") {
        Write-Host "[OK] MySQL já está rodando!" -ForegroundColor Green
    } else {
        # Tentar iniciar
        Start-Service $serviceName -ErrorAction Stop
        Start-Sleep -Seconds 2
        
        # Verificar se iniciou
        $service = Get-Service $serviceName
        if ($service.Status -eq "Running") {
            Write-Host "[OK] MySQL iniciado com sucesso!" -ForegroundColor Green
        } else {
            Write-Host "[ERRO] MySQL não iniciou (Status: $($service.Status))" -ForegroundColor Red
        }
    }
} catch {
    Write-Host "[ERRO] Falha ao iniciar MySQL!" -ForegroundColor Red
    Write-Host "       Erro: $($_.Exception.Message)" -ForegroundColor Yellow
    
    # Procurar logs de erro
    Write-Host "`n[*] Procurando logs de erro..." -ForegroundColor Yellow
    $errorLogs = Get-ChildItem "C:\ProgramData\MySQL" -Filter "*.err" -Recurse -ErrorAction SilentlyContinue | 
                 Sort-Object LastWriteTime -Descending | 
                 Select-Object -First 1
    
    if ($errorLogs) {
        Write-Host "[LOG] Último erro em: $($errorLogs.FullName)" -ForegroundColor Cyan
        Write-Host "`nÚltimas 10 linhas do log:" -ForegroundColor Yellow
        Get-Content $errorLogs.FullName -Tail 10 | ForEach-Object { 
            Write-Host "   $_" -ForegroundColor Gray 
        }
    }
    
    Write-Host "`n[SOLUÇÃO] Execute o diagnóstico completo: .\diagnostico_mysql.ps1" -ForegroundColor Green
    pause
    exit 1
}

# =========================================
# MÉTODO 4: VERIFICAR CONECTIVIDADE
# =========================================
Write-Host "`n[4/4] Verificando conectividade..." -ForegroundColor Yellow

Start-Sleep -Seconds 2

$port3306 = Get-NetTCPConnection -LocalPort 3306 -State Listen -ErrorAction SilentlyContinue

if ($port3306) {
    Write-Host "[OK] MySQL está escutando na porta 3306!" -ForegroundColor Green
    
    # Tentar conexão com mysql.exe
    $mysqlExe = "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe"
    if (Test-Path $mysqlExe) {
        Write-Host "`n[*] Testando conexão..." -ForegroundColor Yellow
        $testResult = & $mysqlExe -u root -p"" -e "SELECT 1;" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[OK] Conexão bem-sucedida!" -ForegroundColor Green
        } else {
            Write-Host "[AVISO] MySQL está rodando mas a conexão falhou" -ForegroundColor Yellow
            Write-Host "        Verifique a senha do root ou permissões." -ForegroundColor Yellow
        }
    }
} else {
    Write-Host "[AVISO] MySQL está rodando mas não está escutando na porta 3306" -ForegroundColor Yellow
    Write-Host "        Verifique o arquivo my.ini" -ForegroundColor Yellow
}

# =========================================
# RESUMO
# =========================================
Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "  RESUMO" -ForegroundColor Cyan
Write-Host "=========================================`n" -ForegroundColor Cyan

$finalService = Get-Service $serviceName
Write-Host "Status do MySQL: $($finalService.Status)" -ForegroundColor $(if ($finalService.Status -eq "Running") { "Green" } else { "Red" })

if ($finalService.Status -eq "Running") {
    Write-Host "`n[OK] MySQL está pronto para uso!" -ForegroundColor Green
    Write-Host "`nPróximos passos:" -ForegroundColor Yellow
    Write-Host "   1. Configure o banco: .\setup_database.sql" -ForegroundColor Cyan
    Write-Host "   2. Inicie a API PHP: .\setup_php_api.ps1" -ForegroundColor Cyan
    Write-Host "   3. Teste no Unreal Engine!" -ForegroundColor Cyan
} else {
    Write-Host "`n[ERRO] MySQL não está rodando!" -ForegroundColor Red
    Write-Host "`n[PRÓXIMOS PASSOS]:" -ForegroundColor Yellow
    Write-Host "   1. Execute: .\diagnostico_mysql.ps1" -ForegroundColor Cyan
    Write-Host "   2. Verifique os logs de erro" -ForegroundColor Cyan
    Write-Host "   3. Reinstale o MySQL se necessário" -ForegroundColor Cyan
}

Write-Host "`n=========================================" -ForegroundColor Cyan
pause

