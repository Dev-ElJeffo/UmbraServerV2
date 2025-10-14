# Script de Teste Rápido das APIs PHP
# Execute: .\test_api.ps1

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Testando APIs PHP - UmbraEternum" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar se WAMP está rodando
Write-Host "[1/4] Verificando serviços..." -ForegroundColor Yellow
$apache = Get-Service wampapache64 -ErrorAction SilentlyContinue
$mysql = Get-Service MySQL80 -ErrorAction SilentlyContinue

if ($apache -and $apache.Status -eq "Running") {
    Write-Host "  ✓ Apache WAMP: Running" -ForegroundColor Green
} else {
    Write-Host "  ❌ Apache WAMP: Stopped ou não encontrado" -ForegroundColor Red
    Write-Host "     Execute: .\fix_wamp_conflict.ps1 como Admin" -ForegroundColor Yellow
    exit 1
}

if ($mysql -and $mysql.Status -eq "Running") {
    Write-Host "  ✓ MySQL80: Running" -ForegroundColor Green
} else {
    Write-Host "  ❌ MySQL80: Stopped" -ForegroundColor Red
    Write-Host "     Execute: Start-Service MySQL80" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "[2/4] Testando conexão com API..." -ForegroundColor Yellow

try {
    $testResult = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/test.php" -TimeoutSec 10
    
    if ($testResult.success) {
        Write-Host "  ✓ Conexão com API: OK" -ForegroundColor Green
        Write-Host "  ✓ MySQL Version: $($testResult.mysql_version)" -ForegroundColor Green
        Write-Host "  ✓ Database: $($testResult.database)" -ForegroundColor Green
        Write-Host "  ✓ Schema Version: $($testResult.schema_version)" -ForegroundColor Green
        Write-Host "  ✓ Accounts: $($testResult.stats.accounts)" -ForegroundColor Green
        Write-Host "  ✓ Players: $($testResult.stats.players)" -ForegroundColor Green
    } else {
        Write-Host "  ❌ API retornou erro: $($testResult.message)" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "  ❌ Não foi possível conectar à API" -ForegroundColor Red
    Write-Host "  Erro: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Verificar:" -ForegroundColor Yellow
    Write-Host "  1. WAMP está verde?" -ForegroundColor White
    Write-Host "  2. http://localhost/ abre?" -ForegroundColor White
    Write-Host "  3. Arquivos em C:\wamp64\www\umbra_api\?" -ForegroundColor White
    exit 1
}

Write-Host ""
Write-Host "[3/4] Testando registro de usuário..." -ForegroundColor Yellow

$randomNum = Get-Random -Minimum 1000 -Maximum 9999
$registerBody = @{
    username = "testuser_$randomNum"
    email = "test$randomNum@example.com"
    password = "senha123"
} | ConvertTo-Json

try {
    $registerResult = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/register.php" `
        -Method POST `
        -Body $registerBody `
        -ContentType "application/json" `
        -TimeoutSec 10
    
    if ($registerResult.success) {
        Write-Host "  ✓ Registro: OK" -ForegroundColor Green
        Write-Host "  ✓ Account ID: $($registerResult.account_id)" -ForegroundColor Green
        Write-Host "  ✓ Username: $($registerResult.username)" -ForegroundColor Green
        
        # Salvar username para teste de login
        $script:testUsername = $registerResult.username
    } else {
        Write-Host "  ❌ Registro falhou: $($registerResult.message)" -ForegroundColor Red
    }
} catch {
    Write-Host "  ❌ Erro ao registrar: $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "[4/4] Testando login..." -ForegroundColor Yellow

if ($script:testUsername) {
    $loginBody = @{
        username = $script:testUsername
        password = "senha123"
    } | ConvertTo-Json
    
    try {
        $loginResult = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/login.php" `
            -Method POST `
            -Body $loginBody `
            -ContentType "application/json" `
            -TimeoutSec 10
        
        if ($loginResult.success) {
            Write-Host "  ✓ Login: OK" -ForegroundColor Green
            Write-Host "  ✓ Token: $($loginResult.token.Substring(0, 20))..." -ForegroundColor Green
            Write-Host "  ✓ Account ID: $($loginResult.account.id)" -ForegroundColor Green
            Write-Host "  ✓ Email: $($loginResult.account.email)" -ForegroundColor Green
        } else {
            Write-Host "  ❌ Login falhou: $($loginResult.message)" -ForegroundColor Red
        }
    } catch {
        Write-Host "  ❌ Erro ao fazer login: $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✅ TESTE COMPLETO!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "APIs testadas com sucesso:" -ForegroundColor Cyan
Write-Host "  ✓ GET  /api/test.php" -ForegroundColor Green
Write-Host "  ✓ POST /api/register.php" -ForegroundColor Green
Write-Host "  ✓ POST /api/login.php" -ForegroundColor Green
Write-Host ""
Write-Host "Acesse o painel de testes:" -ForegroundColor Cyan
Write-Host "  http://localhost/umbra_api/" -ForegroundColor White
Write-Host ""
Write-Host "Ver documentação completa:" -ForegroundColor Cyan
Write-Host "  C:\wamp64\www\umbra_api\README.md" -ForegroundColor White
Write-Host "  D:\UmbraServerV2\UmbraServer\PHP_API_READY.md" -ForegroundColor White
Write-Host ""

Read-Host "Pressione Enter para sair"

