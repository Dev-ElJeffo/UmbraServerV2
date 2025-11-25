# =========================================
# Diagnostico Completo - MySQL Server
# =========================================

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  DIAGNOSTICO MYSQL - UmbraEternum" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Funcao para verificar se esta rodando como Admin
function Test-Admin {
    $currentUser = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Admin)) {
    Write-Host "[ERRO] Este script precisa ser executado como Administrador!" -ForegroundColor Red
    Write-Host "Clique com botao direito e escolha 'Executar como Administrador'" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "[OK] Executando como Administrador" -ForegroundColor Green
Write-Host ""

# =========================================
# 1. VERIFICAR SERVICOS MYSQL
# =========================================
Write-Host "[1/6] Verificando servicos MySQL instalados..." -ForegroundColor Yellow

$mysqlServices = Get-Service | Where-Object { $_.Name -like "*mysql*" -or $_.DisplayName -like "*mysql*" }

if ($mysqlServices.Count -eq 0) {
    Write-Host "[AVISO] Nenhum servico MySQL encontrado!" -ForegroundColor Red
    Write-Host "        MySQL pode nao estar instalado ou nao estar registrado como servico." -ForegroundColor Yellow
} else {
    Write-Host "[OK] Encontrados $($mysqlServices.Count) servico(s) MySQL:" -ForegroundColor Green
    foreach ($service in $mysqlServices) {
        Write-Host "      Nome: $($service.Name)" -ForegroundColor Cyan
        Write-Host "      Display: $($service.DisplayName)" -ForegroundColor Cyan
        Write-Host "      Status: $($service.Status)" -ForegroundColor $(if ($service.Status -eq "Running") { "Green" } else { "Yellow" })
        Write-Host "      StartType: $($service.StartType)" -ForegroundColor Cyan
        Write-Host ""
    }
}

# =========================================
# 2. VERIFICAR PORTA 3306
# =========================================
Write-Host "[2/6] Verificando se a porta 3306 esta em uso..." -ForegroundColor Yellow

$port3306 = Get-NetTCPConnection -LocalPort 3306 -ErrorAction SilentlyContinue

if ($port3306) {
    Write-Host "[AVISO] Porta 3306 JA ESTA EM USO!" -ForegroundColor Red
    $process = Get-Process -Id $port3306.OwningProcess -ErrorAction SilentlyContinue
    if ($process) {
        Write-Host "        Processo: $($process.ProcessName) (PID: $($process.Id))" -ForegroundColor Yellow
        Write-Host "        Caminho: $($process.Path)" -ForegroundColor Yellow
    }
} else {
    Write-Host "[OK] Porta 3306 esta livre" -ForegroundColor Green
}

# =========================================
# 3. VERIFICAR WAMP/XAMPP
# =========================================
Write-Host ""
Write-Host "[3/6] Verificando WAMP/XAMPP..." -ForegroundColor Yellow

$wampProcess = Get-Process -Name "wampmanager*", "mysqld" -ErrorAction SilentlyContinue
if ($wampProcess) {
    Write-Host "[AVISO] WAMP/MySQL detectado rodando!" -ForegroundColor Red
    Write-Host "        Processos encontrados:" -ForegroundColor Yellow
    foreach ($proc in $wampProcess) {
        Write-Host "        - $($proc.ProcessName) (PID: $($proc.Id))" -ForegroundColor Yellow
    }
    Write-Host "        WAMP pode estar usando a porta 3306!" -ForegroundColor Red
} else {
    Write-Host "[OK] WAMP/XAMPP nao detectados" -ForegroundColor Green
}

# =========================================
# 4. VERIFICAR MYSQL.EXE / MYSQLD.EXE
# =========================================
Write-Host ""
Write-Host "[4/6] Verificando processos MySQL em execucao..." -ForegroundColor Yellow

$mysqlProcesses = Get-Process -Name "mysql*", "mysqld*" -ErrorAction SilentlyContinue

if ($mysqlProcesses) {
    Write-Host "[OK] MySQL esta rodando:" -ForegroundColor Green
    foreach ($proc in $mysqlProcesses) {
        Write-Host "      Processo: $($proc.ProcessName) (PID: $($proc.Id))" -ForegroundColor Cyan
        Write-Host "      Caminho: $($proc.Path)" -ForegroundColor Cyan
        Write-Host "      Memoria: $([math]::Round($proc.WorkingSet64 / 1MB, 2)) MB" -ForegroundColor Cyan
        Write-Host ""
    }
} else {
    Write-Host "[AVISO] Nenhum processo MySQL em execucao" -ForegroundColor Yellow
}

# =========================================
# 5. VERIFICAR INSTALACAO MYSQL
# =========================================
Write-Host "[5/6] Verificando instalacao do MySQL..." -ForegroundColor Yellow

$possiblePaths = @(
    "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe",
    "C:\Program Files\MySQL\MySQL Server 8.4\bin\mysqld.exe",
    "C:\Program Files\MySQL\MySQL Server 5.7\bin\mysqld.exe",
    "C:\wamp64\bin\mysql\mysql8.0.39\bin\mysqld.exe",
    "C:\xampp\mysql\bin\mysqld.exe",
    "C:\mysql\bin\mysqld.exe"
)

$foundPaths = @()
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        Write-Host "[OK] Encontrado: $path" -ForegroundColor Green
        $foundPaths += $path
    }
}

if ($foundPaths.Count -eq 0) {
    Write-Host "[ERRO] Nenhuma instalacao MySQL encontrada!" -ForegroundColor Red
    Write-Host "       Voce precisa instalar o MySQL primeiro." -ForegroundColor Yellow
} elseif ($foundPaths.Count -gt 1) {
    Write-Host "[AVISO] Multiplas instalacoes MySQL detectadas!" -ForegroundColor Yellow
    Write-Host "        Isso pode causar conflitos." -ForegroundColor Yellow
}

# =========================================
# 6. VERIFICAR ARQUIVO my.ini
# =========================================
Write-Host ""
Write-Host "[6/6] Verificando configuracao (my.ini)..." -ForegroundColor Yellow

$iniPaths = @(
    "C:\Program Files\MySQL\MySQL Server 8.0\my.ini",
    "C:\Program Files\MySQL\MySQL Server 8.4\my.ini",
    "C:\ProgramData\MySQL\MySQL Server 8.0\my.ini",
    "C:\ProgramData\MySQL\MySQL Server 8.4\my.ini",
    "C:\wamp64\bin\mysql\mysql8.0.39\my.ini"
)

$foundIni = $false
foreach ($iniPath in $iniPaths) {
    if (Test-Path $iniPath) {
        Write-Host "[OK] Encontrado: $iniPath" -ForegroundColor Green
        $foundIni = $true
        
        # Verificar porta no arquivo
        $iniContent = Get-Content $iniPath -ErrorAction SilentlyContinue
        $portLine = $iniContent | Select-String "port\s*=\s*(\d+)" | Select-Object -First 1
        if ($portLine) {
            Write-Host "     Porta configurada: $($portLine.Line.Trim())" -ForegroundColor Cyan
        }
    }
}

if (-not $foundIni) {
    Write-Host "[AVISO] Nenhum arquivo my.ini encontrado" -ForegroundColor Yellow
}

# =========================================
# RESUMO E RECOMENDACOES
# =========================================
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  RESUMO E RECOMENDACOES" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "DIAGNOSTICO COMPLETO:" -ForegroundColor White
Write-Host ""

if ($mysqlServices.Count -eq 0) {
    Write-Host "[PROBLEMA 1] Servico MySQL nao encontrado" -ForegroundColor Red
    Write-Host "   SOLUCAO: Instale o MySQL Server ou registre o servico." -ForegroundColor Yellow
    Write-Host "   Execute: setup_mysql.ps1" -ForegroundColor Green
    Write-Host ""
}

if ($port3306 -and $wampProcess) {
    Write-Host "[PROBLEMA 2] WAMP esta usando a porta 3306" -ForegroundColor Red
    Write-Host "   SOLUCAO 1: Pare o WAMP antes de iniciar o MySQL standalone." -ForegroundColor Yellow
    Write-Host "   SOLUCAO 2: Use o MySQL do WAMP em vez de instalar separado." -ForegroundColor Yellow
    Write-Host "   Execute: .\fix_mysql_conflict.bat" -ForegroundColor Green
    Write-Host ""
}

if ($mysqlServices.Count -gt 0 -and $mysqlServices[0].Status -eq "Stopped") {
    Write-Host "[PROBLEMA 3] Servico MySQL esta parado" -ForegroundColor Red
    Write-Host "   SOLUCAO: Tente iniciar o servico manualmente." -ForegroundColor Yellow
    Write-Host "   Execute o script: start_mysql_service.ps1" -ForegroundColor Green
    Write-Host ""
}

if ($foundPaths.Count -gt 1) {
    Write-Host "[PROBLEMA 4] Multiplas instalacoes MySQL" -ForegroundColor Red
    Write-Host "   SOLUCAO: Desinstale as instalacoes antigas ou defina qual usar." -ForegroundColor Yellow
    Write-Host ""
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Diagnostico concluido!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "[?] O que deseja fazer?" -ForegroundColor Yellow
Write-Host "   1. Tentar iniciar MySQL automaticamente" -ForegroundColor Cyan
Write-Host "   2. Parar WAMP e iniciar MySQL standalone" -ForegroundColor Cyan
Write-Host "   3. Ver logs de erro do MySQL" -ForegroundColor Cyan
Write-Host "   4. Sair" -ForegroundColor Cyan
Write-Host ""

$escolha = Read-Host "Escolha uma opcao (1-4)"

switch ($escolha) {
    "1" {
        Write-Host ""
        Write-Host "[*] Tentando iniciar MySQL..." -ForegroundColor Yellow
        if ($mysqlServices.Count -gt 0) {
            $serviceName = $mysqlServices[0].Name
            try {
                Start-Service $serviceName -ErrorAction Stop
                Write-Host "[OK] MySQL iniciado com sucesso!" -ForegroundColor Green
            } catch {
                Write-Host "[ERRO] Falha ao iniciar: $($_.Exception.Message)" -ForegroundColor Red
                Write-Host "Verifique os logs em: C:\ProgramData\MySQL\MySQL Server 8.x\Data\*.err" -ForegroundColor Yellow
            }
        } else {
            Write-Host "[ERRO] Nenhum servico MySQL para iniciar!" -ForegroundColor Red
        }
    }
    "2" {
        Write-Host ""
        Write-Host "[*] Parando WAMP..." -ForegroundColor Yellow
        if ($wampProcess) {
            Stop-Process -Name "wampmanager*", "mysqld" -Force -ErrorAction SilentlyContinue
            Start-Sleep -Seconds 2
            Write-Host "[OK] WAMP parado" -ForegroundColor Green
            
            if ($mysqlServices.Count -gt 0) {
                Write-Host "[*] Iniciando MySQL standalone..." -ForegroundColor Yellow
                Start-Service $mysqlServices[0].Name -ErrorAction SilentlyContinue
                Write-Host "[OK] MySQL iniciado!" -ForegroundColor Green
            }
        } else {
            Write-Host "[AVISO] WAMP nao esta rodando" -ForegroundColor Yellow
        }
    }
    "3" {
        Write-Host ""
        Write-Host "[*] Procurando logs de erro..." -ForegroundColor Yellow
        $errorLogs = Get-ChildItem "C:\ProgramData\MySQL" -Filter "*.err" -Recurse -ErrorAction SilentlyContinue
        if ($errorLogs) {
            Write-Host "[OK] Logs encontrados:" -ForegroundColor Green
            foreach ($log in $errorLogs) {
                Write-Host "   $($log.FullName)" -ForegroundColor Cyan
            }
            Write-Host ""
            Write-Host "Abra esses arquivos para ver os erros detalhados." -ForegroundColor Yellow
        } else {
            Write-Host "[AVISO] Nenhum log de erro encontrado" -ForegroundColor Yellow
        }
    }
    "4" {
        Write-Host ""
        Write-Host "Saindo..." -ForegroundColor Yellow
    }
}

Write-Host ""
pause
