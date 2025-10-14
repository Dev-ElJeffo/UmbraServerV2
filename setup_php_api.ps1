# Setup PHP API para UmbraEternum
# Cria estrutura de API PHP no WAMP

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  UmbraEternum - PHP API Setup" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar WAMP
$wampPath = "C:\wamp64\www"
if (-not (Test-Path $wampPath)) {
    Write-Host "❌ WAMP não encontrado em: $wampPath" -ForegroundColor Red
    Write-Host ""
    Write-Host "Instale WAMP ou ajuste o caminho no script" -ForegroundColor Yellow
    Read-Host "Pressione Enter para sair"
    exit 1
}

Write-Host "✓ WAMP encontrado" -ForegroundColor Green

# Criar estrutura
$apiDir = Join-Path $wampPath "umbra_api"
Write-Host ""
Write-Host "[1/7] Criando estrutura de diretórios..." -ForegroundColor Yellow

$dirs = @("config", "api", "includes")
foreach ($dir in $dirs) {
    $path = Join-Path $apiDir $dir
    New-Item -ItemType Directory -Path $path -Force | Out-Null
    Write-Host "  ✓ $dir/" -ForegroundColor Green
}

Write-Host ""
Write-Host "[2/7] Parando MySQL do WAMP..." -ForegroundColor Yellow
try {
    Stop-Service wampmysqld64 -ErrorAction SilentlyContinue
    Set-Service wampmysqld64 -StartupType Disabled -ErrorAction SilentlyContinue
    Write-Host "  ✓ MySQL do WAMP desabilitado" -ForegroundColor Green
} catch {
    Write-Host "  ⚠ Não foi possível parar WAMP MySQL" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[3/7] Verificando MySQL80..." -ForegroundColor Yellow
$mysql = Get-Service MySQL80 -ErrorAction SilentlyContinue
if ($mysql -and $mysql.Status -eq "Running") {
    Write-Host "  ✓ MySQL80 rodando" -ForegroundColor Green
} else {
    Write-Host "  ⚠ MySQL80 não está rodando" -ForegroundColor Yellow
    Write-Host "    Execute: Start-Service MySQL80" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[4/7] Copiando arquivos de template..." -ForegroundColor Yellow

# Copiar templates do projeto
$templatesSource = Join-Path (Get-Location) "php_templates"
if (Test-Path $templatesSource) {
    Copy-Item "$templatesSource\*" -Destination $apiDir -Recurse -Force
    Write-Host "  ✓ Templates copiados" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Templates não encontrados" -ForegroundColor Yellow
    Write-Host "    Você precisará criar os arquivos PHP manualmente" -ForegroundColor Yellow
    Write-Host "    Veja: WAMP_PHP_API_SETUP.md" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "[5/7] Criando .htaccess..." -ForegroundColor Yellow

$htaccess = @"
# Enable CORS
Header set Access-Control-Allow-Origin "*"
Header set Access-Control-Allow-Methods "GET, POST, PUT, DELETE, OPTIONS"
Header set Access-Control-Allow-Headers "Content-Type, Authorization"

# Rewrite rules
RewriteEngine On
RewriteCond %{REQUEST_FILENAME} !-f
RewriteCond %{REQUEST_FILENAME} !-d
RewriteRule ^api/(.*)$ api/$1.php [L,QSA]
"@

Set-Content -Path (Join-Path $apiDir ".htaccess") -Value $htaccess
Write-Host "  ✓ .htaccess criado" -ForegroundColor Green

Write-Host ""
Write-Host "[6/7] Criando README..." -ForegroundColor Yellow

$readme = @"
# UmbraEternum PHP API

API PHP para testes de comunicação com servidor C++ e banco de dados.

## Endpoints

- GET  /api/test.php       - Testar conexão
- POST /api/register.php   - Criar conta
- POST /api/login.php      - Autenticar

## Acesso

http://localhost/umbra_api/

## Configuração

Edite config/database.php com credenciais do MySQL80

## Documentação

Ver: WAMP_PHP_API_SETUP.md no projeto principal
"@

Set-Content -Path (Join-Path $apiDir "README.md") -Value $readme
Write-Host "  ✓ README criado" -ForegroundColor Green

Write-Host ""
Write-Host "[7/7] Testando Apache..." -ForegroundColor Yellow

try {
    $response = Invoke-WebRequest -Uri "http://localhost/" -UseBasicParsing -ErrorAction Stop
    Write-Host "  ✓ Apache está rodando" -ForegroundColor Green
} catch {
    Write-Host "  ⚠ Apache não está respondendo" -ForegroundColor Yellow
    Write-Host "    Inicie o WAMP e tente novamente" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  ✅ SETUP COMPLETO!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Estrutura criada em:" -ForegroundColor Cyan
Write-Host "  $apiDir" -ForegroundColor White
Write-Host ""

Write-Host "Próximos passos:" -ForegroundColor Cyan
Write-Host "  1. Copie os arquivos PHP do guia: WAMP_PHP_API_SETUP.md" -ForegroundColor White
Write-Host "  2. Edite config/database.php com suas credenciais" -ForegroundColor White
Write-Host "  3. Acesse: http://localhost/umbra_api/" -ForegroundColor White
Write-Host "  4. Teste os endpoints!" -ForegroundColor White
Write-Host ""

$open = Read-Host "Deseja abrir o diretório agora? (S/N)"
if ($open -eq "S" -or $open -eq "s") {
    explorer $apiDir
}

Write-Host ""
Read-Host "Pressione Enter para sair"

