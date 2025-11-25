# 📦 Guia: Instalar Composer e Biblioteca JWT Manualmente

**IMPORTANTE**: Este guia é apenas para instalar o Composer e a biblioteca JWT. **NÃO ALTERA NADA** do código existente que já funciona (login, dashboard, etc.).

---

## 🎯 O Que Vamos Fazer

1. Baixar e instalar o Composer (gerenciador de dependências PHP)
2. Instalar a biblioteca `firebase/php-jwt` via Composer
3. Testar se a API de inventário funciona

**Tempo estimado**: 5 minutos

---

## 📥 Passo 1: Baixar o Composer

### Opção A: Instalador Windows (Recomendado)

1. **Baixe** o instalador:
   ```
   https://getcomposer.org/Composer-Setup.exe
   ```

2. **Execute** o instalador (Composer-Setup.exe)

3. **Siga as instruções**:
   - Deixe todas as opções padrão
   - O instalador vai detectar automaticamente seu PHP
   - Clique em "Next" até finalizar

4. **Reinicie** o PowerShell ou CMD após a instalação

5. **Teste** se instalou corretamente:
   ```powershell
   composer --version
   ```
   
   Deve mostrar algo como:
   ```
   Composer version 2.x.x
   ```

---

### Opção B: Download Manual (Se o instalador não funcionar)

1. **Baixe** o arquivo `composer.phar`:
   ```
   https://getcomposer.org/download/
   ```
   
   Clique em "Download Latest (2.x.x)" → Escolha "composer.phar"

2. **Mova** o arquivo para `C:\wamp64\bin\composer\`
   - Se a pasta não existir, crie ela

3. **Crie** um arquivo batch `C:\wamp64\bin\composer\composer.bat`:
   ```batch
   @echo off
   php "%~dp0composer.phar" %*
   ```

4. **Adicione** ao PATH:
   - Pressione `Win + X` → "Sistema"
   - "Configurações avançadas do sistema"
   - "Variáveis de ambiente"
   - Em "Path", adicione: `C:\wamp64\bin\composer`
   - OK → OK → Reinicie o terminal

5. **Teste**:
   ```powershell
   composer --version
   ```

---

## 📦 Passo 2: Instalar a Biblioteca JWT

1. **Abra o PowerShell como Administrador**

2. **Navegue** para o diretório da API:
   ```powershell
   cd C:\wamp64\www\umbra_api
   ```

3. **Execute** o comando do Composer:
   ```powershell
   composer require firebase/php-jwt
   ```

4. **Aguarde** a instalação (pode demorar 30 segundos a 2 minutos)

5. **Você verá** algo assim:
   ```
   Using version ^6.10 for firebase/php-jwt
   ./composer.json has been created
   Running composer update firebase/php-jwt
   Loading composer repositories with package information
   Updating dependencies
   Lock file operations: 1 install, 0 updates, 0 removals
     - Locking firebase/php-jwt (v6.10.0)
   Writing lock file
   Installing dependencies from lock file (including require-dev)
   Package operations: 1 install, 0 updates, 0 removals
     - Installing firebase/php-jwt (v6.10.0): Extracting archive
   Generating autoload files
   ```

---

## ✅ Passo 3: Verificar Instalação

### Verificar arquivos criados:

Após a instalação, você deve ter:

```
C:\wamp64\www\umbra_api\
├── composer.json          ✅ (criado)
├── composer.lock          ✅ (criado)
└── vendor\                ✅ (criado)
    ├── autoload.php       ✅ (IMPORTANTE!)
    ├── composer\
    └── firebase\
        └── php-jwt\
```

### Teste rápido via navegador:

1. **Acesse**:
   ```
   http://localhost/umbra_api/diagnostico_completo.php
   ```

2. **Verifique**:
   - ✅ `vendor/autoload.php` - Deve estar VERDE agora
   - ✅ `Biblioteca Firebase JWT instalada` - Deve estar VERDE
   - ✅ `jwt_helper.php carregado sem erros` - Deve estar VERDE

---

## 🧪 Passo 4: Testar a API de Inventário

1. **Acesse**:
   ```
   http://localhost/umbra_api/test_inventory.php
   ```

2. **Cole seu token JWT** no campo

3. **Clique em** "📋 Listar Templates"
   - Deve mostrar 17 itens únicos

4. **Clique em** "📦 Carregar Inventário"
   - Deve carregar o inventário sem erro "Unexpected token"

---

## 🐛 Troubleshooting

### Problema 1: "composer não é reconhecido"

**Solução**:
1. Reinicie o PowerShell/CMD
2. Se ainda não funcionar, adicione ao PATH manualmente (veja Opção B)
3. Ou use o caminho completo:
   ```powershell
   C:\ProgramData\ComposerSetup\bin\composer.bat require firebase/php-jwt
   ```

---

### Problema 2: "Your requirements could not be resolved"

**Solução**:
1. Verifique se você está no diretório correto:
   ```powershell
   cd C:\wamp64\www\umbra_api
   pwd  # Deve mostrar: C:\wamp64\www\umbra_api
   ```

2. Tente instalar especificando a versão:
   ```powershell
   composer require firebase/php-jwt:^6.0
   ```

---

### Problema 3: "Failed to open stream: Permission denied"

**Solução**:
1. Feche todos os programas que possam estar usando os arquivos
2. Execute o PowerShell como Administrador
3. Tente novamente

---

### Problema 4: Composer muito lento

**Solução**:
1. Desative o antivírus temporariamente
2. Ou use mirror brasileiro:
   ```powershell
   composer config -g repos.packagist composer https://packagist.com.br
   ```

---

## 🎯 Checklist Final

Após completar todos os passos, verifique:

- [ ] Composer instalado e funcionando (`composer --version`)
- [ ] Biblioteca JWT instalada (`vendor/firebase/php-jwt/` existe)
- [ ] Arquivo `vendor/autoload.php` existe
- [ ] `diagnostico_completo.php` mostra tudo em VERDE
- [ ] `test_inventory.php` funciona sem erro "Unexpected token"

---

## 📝 Comandos em Resumo

```powershell
# 1. Baixar Composer
# Visite: https://getcomposer.org/download/

# 2. Instalar JWT
cd C:\wamp64\www\umbra_api
composer require firebase/php-jwt

# 3. Verificar
composer show firebase/php-jwt

# 4. Testar
# http://localhost/umbra_api/diagnostico_completo.php
```

---

## ✅ O Que Este Guia NÃO Faz

- ❌ NÃO altera o código de login existente
- ❌ NÃO modifica o dashboard
- ❌ NÃO altera nenhuma API que já funciona
- ❌ NÃO mexe no banco de dados

**Apenas instala** a biblioteca JWT necessária para as novas APIs de inventário.

---

## 🚀 Após Concluir

Quando o Composer e a biblioteca JWT estiverem instalados:

1. **Teste** a API: `http://localhost/umbra_api/test_inventory.php`
2. **Envie** o resultado se ainda houver erro
3. **Continue** com a integração Blueprint no Unreal Engine

---

**Qualquer problema, envie screenshot da tela do erro! 📸**

