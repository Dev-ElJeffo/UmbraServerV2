# Setup do Repositório GitHub - UmbraServer

Guia passo a passo para criar e configurar o repositório no GitHub.

## 📋 Pré-requisitos

1. Conta no GitHub
2. Git instalado no Windows
3. Acesso ao PowerShell

## 🚀 Passos para Setup

### Opção A: Usando Script Automatizado (Recomendado)

#### 1. Execute o Script de Setup

Abra o PowerShell no diretório do projeto:

```powershell
cd D:\UmbraServerV2\UmbraServer
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\setup_github.ps1
```

O script irá:
- ✅ Configurar Git (se necessário)
- ✅ Adicionar todos os arquivos
- ✅ Criar commit inicial
- ✅ Configurar branch main
- ✅ Adicionar remote origin

#### 2. Crie o Repositório no GitHub

1. Acesse: https://github.com/new
2. Preencha:
   - **Repository name**: `UmbraServer`
   - **Description**: `Servidor C++ para UmbraEternum MMORPG com integração Unreal Engine 5`
   - **Visibility**: Private (ou Public)
3. **IMPORTANTE**: NÃO marque nenhuma opção de inicialização
4. Clique em **Create repository**

#### 3. Faça o Push

```powershell
git push -u origin main
```

#### 4. Configure Submodules (Opcional)

```powershell
.\setup_submodules.ps1
git add .
git commit -m "chore: adiciona bibliotecas third-party como submodules"
git push
```

---

### Opção B: Comandos Manuais

Se preferir executar comandos manualmente:

#### 1. Inicializar Git (já feito)

```powershell
cd D:\UmbraServerV2\UmbraServer
git init
```

#### 2. Configurar Git

```powershell
git config --global user.name "Seu Nome"
git config --global user.email "seu.email@example.com"
```

#### 3. Adicionar Arquivos

```powershell
git add .
```

#### 4. Commit Inicial

```powershell
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
```

#### 5. Configurar Branch

```powershell
git branch -M main
```

#### 6. Adicionar Remote

```powershell
# Substitua SEU_USERNAME pelo seu username do GitHub
git remote add origin https://github.com/SEU_USERNAME/UmbraServer.git
```

#### 7. Push Inicial

```powershell
git push -u origin main
```

---

## 🌿 Estrutura de Branches

Após o setup inicial, crie os branches de desenvolvimento:

```powershell
# Branch de desenvolvimento
git checkout -b develop
git push -u origin develop

# Configurar develop como branch padrão no GitHub:
# Settings → Branches → Default branch → develop
```

### Branches Recomendados

- `main` - Produção (protegido)
- `develop` - Desenvolvimento (protegido)
- `feature/*` - Novas features
- `bugfix/*` - Correções de bugs
- `hotfix/*` - Correções urgentes
- `release/*` - Preparação de releases

---

## 📦 Adicionar Submodules

### Automaticamente

```powershell
.\setup_submodules.ps1
```

### Manualmente

```powershell
# nlohmann/json
git submodule add https://github.com/nlohmann/json.git third_party/json

# spdlog
git submodule add https://github.com/gabime/spdlog.git third_party/spdlog

# googletest
git submodule add https://github.com/google/googletest.git third_party/googletest

# jwt-cpp
git submodule add https://github.com/Thalhammer/jwt-cpp.git third_party/jwt-cpp

# Commit
git add .
git commit -m "chore: adiciona bibliotecas third-party como submodules"
git push
```

---

## ⚙️ Configurações do Repositório no GitHub

### 1. Proteger Branches

**Settings → Branches → Add rule**

Para `main` e `develop`:
- ✅ Require pull request reviews before merging
- ✅ Require status checks to pass
- ✅ Require branches to be up to date
- ✅ Include administrators

### 2. Configurar Issues

**Settings → Features**
- ✅ Enable Issues
- ✅ Enable Projects

**Issues → Labels** - Adicionar labels:
- `enhancement` - Nova feature
- `bug` - Bug report
- `ue5-integration` - Integração UE5
- `documentation` - Documentação
- `priority:high` - Alta prioridade
- `priority:medium` - Média prioridade
- `priority:low` - Baixa prioridade

### 3. Configurar GitHub Actions (CI/CD)

Crie `.github/workflows/build.yml`:

```yaml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest]
        
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Install dependencies (Ubuntu)
      if: matrix.os == 'ubuntu-latest'
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential libmysqlclient-dev
        
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      
    - name: Build
      run: cmake --build build --config Release
      
    - name: Test
      run: cd build && ctest -C Release --output-on-failure
```

### 4. Configurar Secrets

**Settings → Secrets and variables → Actions**

Adicionar:
- `DB_PASSWORD` - Senha do banco de dados
- `JWT_SECRET` - Chave secreta do JWT

### 5. README Badges

Adicione ao topo do README.md:

```markdown
![Build Status](https://github.com/SEU_USERNAME/UmbraServer/workflows/Build%20and%20Test/badge.svg)
![License](https://img.shields.io/badge/license-Proprietary-red)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
```

---

## 🔄 Workflow de Desenvolvimento

### 1. Criar Feature

```powershell
git checkout develop
git pull origin develop
git checkout -b feature/minha-feature
```

### 2. Desenvolver

```powershell
# Fazer mudanças
git add .
git commit -m "feat(module): descrição da mudança"
```

### 3. Push e PR

```powershell
git push -u origin feature/minha-feature
# Abrir Pull Request no GitHub
```

### 4. Após Merge

```powershell
git checkout develop
git pull origin develop
git branch -d feature/minha-feature
```

---

## 🏷️ Tags e Releases

### Criar Tag

```powershell
git tag -a v1.3.0 -m "Release 1.3.0 - Implementação inicial"
git push origin v1.3.0
```

### Criar Release no GitHub

1. Acesse: `https://github.com/SEU_USERNAME/UmbraServer/releases/new`
2. Escolha a tag: `v1.3.0`
3. Título: `v1.3.0 - Implementação Inicial`
4. Descrição: Copie do CHANGELOG.md
5. Adicione binários compilados (opcional)
6. Publique

---

## 📝 Notas Importantes

### .gitignore

Já configurado para ignorar:
- `build/` - Artifacts de build
- `logs/` - Logs dos servidores
- `config/jwt_secret.key` - Chave secreta
- `config/db.json` - Credenciais do banco
- Arquivos temporários e caches

### Arquivos Sensíveis

**NUNCA commite:**
- Senhas ou credenciais
- Chaves privadas
- Tokens de API
- Arquivos de configuração com dados sensíveis

Use `.example` para templates:
- `db.json.example` ✅
- `jwt_secret.key.example` ✅

---

## 🆘 Troubleshooting

### Erro: "Permission denied (publickey)"

Configure SSH ou use HTTPS:
```powershell
git remote set-url origin https://github.com/SEU_USERNAME/UmbraServer.git
```

### Erro: "Updates were rejected"

```powershell
git pull origin main --rebase
git push origin main
```

### Remover arquivo commitado por engano

```powershell
# Remove do Git mas mantém no disco
git rm --cached arquivo.txt
git commit -m "chore: remove arquivo sensível"
git push
```

### Resetar para commit anterior

```powershell
# Soft reset (mantém mudanças)
git reset --soft HEAD~1

# Hard reset (descarta mudanças)
git reset --hard HEAD~1
```

---

## 📚 Recursos

- [Git Documentation](https://git-scm.com/doc)
- [GitHub Docs](https://docs.github.com)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Git Flow](https://nvie.com/posts/a-successful-git-branching-model/)

---

**Última atualização**: 2025-10-14  
**Versão**: 1.0

