# Script de Setup do GitHub para UmbraServer
# Versão: 1.0
# Execute este script no PowerShell para configurar o repositório Git

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Setup GitHub - UmbraEternum Server" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Configurações
$GITHUB_USERNAME = Read-Host "Digite seu username do GitHub"
$REPO_NAME = "UmbraServer"

Write-Host ""
Write-Host "[1/6] Configurando Git..." -ForegroundColor Yellow

# Configurar Git (se ainda não estiver configurado)
$gitUser = git config --global user.name
if (-not $gitUser) {
    $userName = Read-Host "Digite seu nome completo para o Git"
    git config --global user.name "$userName"
}

$gitEmail = git config --global user.email
if (-not $gitEmail) {
    $userEmail = Read-Host "Digite seu email do Git"
    git config --global user.email "$userEmail"
}

Write-Host "[OK] Git configurado" -ForegroundColor Green

Write-Host ""
Write-Host "[2/6] Adicionando arquivos ao staging..." -ForegroundColor Yellow

# Adicionar todos os arquivos
git add .

Write-Host "[OK] Arquivos adicionados" -ForegroundColor Green

Write-Host ""
Write-Host "[3/6] Criando commit inicial..." -ForegroundColor Yellow

# Commit inicial
git commit -m "feat: implementação inicial da arquitetura de servidores

- Estrutura completa de micro-serviços C++17
- Auth Server com JWT e sessions
- World Server com event management
- Zone Servers com player/entity management
- Chat Server com channel management
- Gateway Server com load balancing
- Database layer com MySQL DAOs
- Network layer com TCP/UDP/WebSocket
- Core components (Config, Logger, Timer, Utils)
- Sistema de testes com Google Test
- Documentação completa
- Scripts de automação
- CMake build system

Versão: 1.3.0"

Write-Host "[OK] Commit criado" -ForegroundColor Green

Write-Host ""
Write-Host "[4/6] Configurando branch principal..." -ForegroundColor Yellow

# Renomear branch para main
git branch -M main

Write-Host "[OK] Branch 'main' configurada" -ForegroundColor Green

Write-Host ""
Write-Host "[5/6] Adicionando remote origin..." -ForegroundColor Yellow

# Adicionar remote
$REMOTE_URL = "https://github.com/$GITHUB_USERNAME/$REPO_NAME.git"
git remote add origin $REMOTE_URL

Write-Host "[OK] Remote adicionado: $REMOTE_URL" -ForegroundColor Green

Write-Host ""
Write-Host "[6/6] Preparando para push..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Setup Completo!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor White
Write-Host ""
Write-Host "1. Crie o repositório no GitHub:" -ForegroundColor Yellow
Write-Host "   https://github.com/new" -ForegroundColor White
Write-Host "   - Nome: $REPO_NAME" -ForegroundColor White
Write-Host "   - NÃO inicialize com README, .gitignore ou license" -ForegroundColor Red
Write-Host ""
Write-Host "2. Execute o push:" -ForegroundColor Yellow
Write-Host "   git push -u origin main" -ForegroundColor White
Write-Host ""
Write-Host "3. Para adicionar submodules:" -ForegroundColor Yellow
Write-Host "   .\setup_submodules.ps1" -ForegroundColor White
Write-Host ""
Write-Host "Repositório local configurado em:" -ForegroundColor White
Write-Host "   $PWD" -ForegroundColor Cyan
Write-Host ""

