# Script para adicionar Git Submodules
# Execute APÓS criar o repositório no GitHub e fazer o primeiro push

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Configurando Git Submodules" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

$submodules = @(
    @{
        Name = "nlohmann/json"
        URL = "https://github.com/nlohmann/json.git"
        Path = "third_party/json"
    },
    @{
        Name = "spdlog"
        URL = "https://github.com/gabime/spdlog.git"
        Path = "third_party/spdlog"
    },
    @{
        Name = "googletest"
        URL = "https://github.com/google/googletest.git"
        Path = "third_party/googletest"
    },
    @{
        Name = "jwt-cpp"
        URL = "https://github.com/Thalhammer/jwt-cpp.git"
        Path = "third_party/jwt-cpp"
    }
)

$count = 1
foreach ($module in $submodules) {
    Write-Host "[$count/4] Adicionando $($module.Name)..." -ForegroundColor Yellow
    
    # Remover .gitkeep se existir
    $gitkeepPath = Join-Path $module.Path ".gitkeep"
    if (Test-Path $gitkeepPath) {
        Remove-Item $gitkeepPath -Force
    }
    
    # Adicionar submodule
    git submodule add $module.URL $module.Path
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] $($module.Name) adicionado" -ForegroundColor Green
    } else {
        Write-Host "[ERRO] Falha ao adicionar $($module.Name)" -ForegroundColor Red
    }
    
    $count++
    Write-Host ""
}

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Submodules Configurados!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor White
Write-Host ""
Write-Host "1. Commit as mudanças:" -ForegroundColor Yellow
Write-Host "   git add ." -ForegroundColor White
Write-Host "   git commit -m 'chore: adiciona bibliotecas third-party como submodules'" -ForegroundColor White
Write-Host ""
Write-Host "2. Push para o GitHub:" -ForegroundColor Yellow
Write-Host "   git push" -ForegroundColor White
Write-Host ""
Write-Host "3. Para clonar em outra máquina:" -ForegroundColor Yellow
Write-Host "   git clone --recurse-submodules <url-do-repo>" -ForegroundColor White
Write-Host ""

