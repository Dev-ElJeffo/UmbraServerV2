# Setup MySQL para UmbraEternum Server
# Execute como Administrador: Right-click → Run as Administrator

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  UmbraEternum - MySQL Setup" -ForegroundColor Cyan
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

# Parar WAMP MySQL se estiver rodando
Write-Host "[1/5] Parando WAMP MySQL..." -ForegroundColor Yellow
try {
    $wampService = Get-Service -Name "wampmysqld64" -ErrorAction SilentlyContinue
    if ($wampService -and $wampService.Status -eq "Running") {
        Stop-Service -Name "wampmysqld64" -Force
        Write-Host "  ✓ WAMP MySQL parado" -ForegroundColor Green
    } else {
        Write-Host "  ✓ WAMP MySQL já está parado" -ForegroundColor Green
    }
} catch {
    Write-Host "  ⚠ Não foi possível parar WAMP MySQL (pode não estar instalado)" -ForegroundColor Yellow
}
Write-Host ""

# Iniciar MySQL 8.0
Write-Host "[2/5] Iniciando MySQL 8.0..." -ForegroundColor Yellow
try {
    $mysqlService = Get-Service -Name "MySQL80" -ErrorAction Stop
    
    if ($mysqlService.Status -ne "Running") {
        Start-Service -Name "MySQL80"
        Start-Sleep -Seconds 3
        Write-Host "  ✓ MySQL 8.0 iniciado com sucesso" -ForegroundColor Green
    } else {
        Write-Host "  ✓ MySQL 8.0 já está rodando" -ForegroundColor Green
    }
} catch {
    Write-Host "  ❌ ERRO: MySQL 8.0 não encontrado ou não pode ser iniciado" -ForegroundColor Red
    Write-Host "  Verifique se MySQL está instalado: C:\Program Files\MySQL\MySQL Server 8.0" -ForegroundColor Yellow
    Read-Host "Pressione Enter para sair"
    exit 1
}
Write-Host ""

# Configurar variável de ambiente temporária
$mysqlPath = "C:\Program Files\MySQL\MySQL Server 8.0\bin"
$env:PATH += ";$mysqlPath"

Write-Host "[3/5] Testando conexão com MySQL..." -ForegroundColor Yellow
Write-Host "  Digite a senha do MySQL root quando solicitado" -ForegroundColor Cyan
Write-Host "  (Se não tiver senha, apenas pressione Enter)" -ForegroundColor Cyan
Write-Host ""

# Testar conexão
$testConnection = & "$mysqlPath\mysql.exe" -u root -p -e "SELECT VERSION();" 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "  ✓ Conexão bem-sucedida!" -ForegroundColor Green
} else {
    Write-Host "  ❌ Falha na conexão. Verifique a senha." -ForegroundColor Red
    Read-Host "Pressione Enter para sair"
    exit 1
}
Write-Host ""

# Executar script SQL
Write-Host "[4/5] Criando banco de dados..." -ForegroundColor Yellow
$sqlScript = Join-Path $PSScriptRoot "setup_database.sql"

if (-not (Test-Path $sqlScript)) {
    Write-Host "  ❌ ERRO: Arquivo setup_database.sql não encontrado!" -ForegroundColor Red
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "  Executando: $sqlScript" -ForegroundColor Cyan
& "$mysqlPath\mysql.exe" -u root -p < $sqlScript

if ($LASTEXITCODE -eq 0) {
    Write-Host "  ✓ Banco de dados criado com sucesso!" -ForegroundColor Green
} else {
    Write-Host "  ❌ Erro ao criar banco de dados" -ForegroundColor Red
    Read-Host "Pressione Enter para sair"
    exit 1
}
Write-Host ""

# Atualizar configuração
Write-Host "[5/5] Atualizando configuração..." -ForegroundColor Yellow

$configFile = Join-Path $PSScriptRoot "config\db.json"
$configContent = @"
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "umbra_eternum",
    "user": "root",
    "password": ""
  },
  "redis": {
    "host": "localhost",
    "port": 6379,
    "password": "",
    "db": 0
  }
}
"@

Set-Content -Path $configFile -Value $configContent -Encoding UTF8
Write-Host "  ✓ Configuração atualizada: $configFile" -ForegroundColor Green
Write-Host ""

# Verificação final
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✓ SETUP COMPLETO!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Verificando instalação..." -ForegroundColor Yellow
& "$mysqlPath\mysql.exe" -u root -p -e "USE umbra_eternum; SHOW TABLES; SELECT * FROM schema_version;"

Write-Host ""
Write-Host "Próximo passo:" -ForegroundColor Cyan
Write-Host "  cd build\bin\Release" -ForegroundColor White
Write-Host "  .\umbra_server.exe" -ForegroundColor White
Write-Host ""

Read-Host "Pressione Enter para sair"

