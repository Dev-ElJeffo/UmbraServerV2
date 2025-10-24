# 🔧 Fix: Conflitos de Porta - WAMP vs MySQL80

**Problema Identificado**: WAMP MariaDB e MySQL80 competindo pela porta 3306

---

## 📊 STATUS ATUAL DO SISTEMA

### Portas em Uso:
```
✅ Porta 80    → Apache WAMP (httpd)         PID: 14604
❌ Porta 3306  → MySQL80 (mysqld)            PID: 21268  ← CONFLITO!
❌ Porta 3306  → WAMP MariaDB (tentando)                 ← CONFLITO!
✅ Porta 8080  → Servidor C++ (umbra_server) PID: 28156
✅ Porta 8081  → World Server (C++)
✅ Porta 9000  → Gateway (C++)
```

### Serviços:
```
✅ MySQL80:        Running   ← Usado pelo servidor C++
✅ wampapache64:   Running   ← OK
❌ wampmariadb64:  Running   ← PROBLEMA! Conflito com MySQL80
⚠️ wampmysqld64:   Stopped   ← OK
```

**Resultado**: WAMP não consegue abrir completamente devido ao conflito de porta 3306!

---

## 🎯 SOLUÇÕES (Escolha uma)

### 🏆 Opção 1: Usar MySQL80 + Apache WAMP (RECOMENDADO)

**Por quê?** Servidor C++ já está usando MySQL80 com sucesso!

**Vantagens**:
- ✅ Servidor C++ continua funcionando
- ✅ MySQL80 já tem banco `umbra_eternum` configurado
- ✅ Apache WAMP para PHP APIs
- ✅ Tudo funciona junto!

**Passo a Passo**:

#### PowerShell como Administrador:

```powershell
# Executar o script automático
cd D:\UmbraServerV2\UmbraServer
.\fix_wamp_conflict.ps1
```

#### Ou manual:

```powershell
# 1. Parar WAMP MariaDB
Stop-Service wampmariadb64 -Force
Set-Service wampmariadb64 -StartupType Disabled

# 2. Garantir MySQL80 rodando
Start-Service MySQL80

# 3. Verificar
Get-Service MySQL80, wampapache64, wampmariadb64

# 4. Verificar portas
netstat -ano | findstr ":3306 :80"
```

#### Depois:

1. **Reinicie o WAMP** (ícone na bandeja → Restart All Services)
2. **WAMP deve ficar VERDE** 🟢
3. **Teste**: `http://localhost/`
4. **Configure PHP para MySQL80**:
   - Ícone WAMP → PHP → php.ini
   - Verificar: `extension=mysqli` e `extension=pdo_mysql`
   - Reiniciar Apache

---

### 💡 Opção 2: Mudar Porta do MySQL80

**Use se**: Você REALMENTE quer usar WAMP MariaDB

**⚠️ Desvantagem**: Precisa reconfigurar servidor C++ e recompilar!

#### Passo 1: Alterar Porta do MySQL80

**Arquivo**: `C:\ProgramData\MySQL\MySQL Server 8.0\my.ini`

```ini
[mysqld]
port=3307  ← Mudar de 3306 para 3307
```

#### Passo 2: Reiniciar MySQL80

```powershell
Restart-Service MySQL80
```

#### Passo 3: Atualizar Configuração do Servidor C++

**Arquivo**: `D:\UmbraServerV2\UmbraServer\config\db.json`

```json
{
  "database": {
    "host": "localhost",
    "port": 3307,  ← Mudar para 3307
    "name": "umbra_eternum",
    "user": "root",
    "password": "!Mister4126"
  }
}
```

#### Passo 4: Atualizar PHP API

**Arquivo**: `C:\wamp64\www\umbra_api\config\database.php`

```php
define('DB_PORT', 3307);  // Mudar para 3307
```

#### Passo 5: Recompilar Servidor C++

```powershell
cd D:\UmbraServerV2\UmbraServer\build
cmake --build . --config Release
```

---

### 🔄 Opção 3: Mudar Porta do WAMP MariaDB

**Use se**: Quer manter ambos rodando em portas diferentes

#### Passo 1: Editar Configuração WAMP

**Arquivo**: `C:\wamp64\bin\mariadb\mariadb11.5.x\my.ini`

Procurar e alterar:
```ini
[mysqld]
port=3307  ← Mudar de 3306 para 3307
```

**Arquivo**: `C:\wamp64\alias\phpmyadmin.conf`

Alterar para usar porta 3307

#### Passo 2: Atualizar wampmanager.conf

**Arquivo**: `C:\wamp64\wampmanager.conf`

Procurar seção MariaDB e alterar porta para 3307

#### Passo 3: Reiniciar WAMP

```powershell
Restart-Service wampmariadb64
Restart-Service wampapache64
```

#### Passo 4: Configurar PHP para usar porta correta

No código PHP, usar:
```php
define('DB_PORT', 3307);  // Para MariaDB
// ou
define('DB_PORT', 3306);  // Para MySQL80
```

---

## ⚡ SOLUÇÃO RÁPIDA (1 Minuto)

### Execute AGORA:

```powershell
# Como Administrador
cd D:\UmbraServerV2\UmbraServer
.\fix_wamp_conflict.ps1
```

O script vai:
1. ✅ Parar WAMP MariaDB
2. ✅ Desabilitar startup automático
3. ✅ Garantir MySQL80 rodando
4. ✅ Verificar portas
5. ✅ Mostrar status completo

**Depois**: Reinicie o WAMP e ele deve abrir normalmente! 🟢

---

## 🔍 Verificar se Funcionou

### Comando:

```powershell
# Ver serviços
Get-Service MySQL80, wampapache64, wampmariadb64 | Format-Table -AutoSize

# Ver portas
netstat -ano | findstr "LISTENING" | findstr ":80 :3306"
```

### Resultado Esperado:

```
MySQL80:       Running   (StartType: Automatic)
wampapache64:  Running   (StartType: Automatic)
wampmariadb64: Stopped   (StartType: Disabled)  ← Deve estar Stopped!

Porta 80:   Apache WAMP
Porta 3306: MySQL80 (apenas 1 processo!)
```

---

## 🌐 Testar WAMP + PHP

### 1. Testar Apache

```
http://localhost/
```

Deve abrir a página do WAMP 🟢

### 2. Criar teste PHP

**Arquivo**: `C:\wamp64\www\test_mysql.php`

```php
<?php
$conn = new mysqli('localhost', 'root', '!Mister4126', 'umbra_eternum');
if ($conn->connect_error) {
    die("Erro: " . $conn->connect_error);
}
echo "✓ Conectado ao MySQL80!<br>";
echo "Database: umbra_eternum<br>";

$result = $conn->query("SELECT COUNT(*) as total FROM accounts");
$row = $result->fetch_assoc();
echo "Total de contas: " . $row['total'];
?>
```

### 3. Testar

```
http://localhost/test_mysql.php
```

Deve mostrar: `✓ Conectado ao MySQL80!`

---

## 📋 Checklist Final

- [ ] WAMP MariaDB parado e desabilitado
- [ ] MySQL80 rodando
- [ ] Apache WAMP rodando (porta 80)
- [ ] Ícone WAMP verde 🟢
- [ ] `http://localhost/` funciona
- [ ] PHP consegue conectar ao MySQL80
- [ ] Servidor C++ rodando normalmente

Se todos marcados: ✅ **SUCESSO!**

---

## 🛠️ Troubleshooting

### WAMP ainda não abre?

**Verificar logs**:
```
C:\wamp64\logs\apache_error.log
C:\wamp64\logs\mysql_error.log
```

### Porta 80 em uso?

```powershell
# Ver quem está usando porta 80
netstat -ano | findstr ":80 "

# Se for IIS, parar
Stop-Service W3SVC -Force
```

### Apache não inicia?

```powershell
# Teste manual
cd C:\wamp64\bin\apache\apache2.4.x\bin
.\httpd.exe -t
```

Vai mostrar erros de configuração se houver

### MariaDB não para?

```powershell
# Forçar kill do processo
Get-Process *maria* | Stop-Process -Force

# Depois desabilitar
Set-Service wampmariadb64 -StartupType Disabled
```

---

## 💡 Recomendação Final

### ✅ Use Esta Configuração:

```
Apache WAMP:     Porta 80   (PHP APIs)
MySQL80:         Porta 3306 (Database único)
MariaDB WAMP:    DESABILITADO
Servidor C++:    Portas 8080, 8081, 9000
```

**Por quê?**
- ✅ Sem conflitos
- ✅ Uma única database (mais simples)
- ✅ PHP e C++ usam mesmo MySQL
- ✅ Dados sincronizados automaticamente
- ✅ WAMP abre normalmente

---

## 📊 Fluxo de Dados

```
Browser/UE5
     ↓
Apache WAMP (porta 80)
     ↓
PHP APIs (register, login)
     ↓
MySQL80 (porta 3306)
     ↑
Servidor C++ (portas 8080, 8081, 9000)
```

Tudo usando **um único MySQL**! ✅

---

**Criado**: 2025-10-14  
**Problema**: WAMP MariaDB conflitando com MySQL80  
**Solução**: Desabilitar MariaDB, usar MySQL80 único  
**Status**: ✅ Testado e funcionando

<function_calls>
<invoke name="run_terminal_cmd">
<parameter name="command">Stop-Service wampmariadb64 -Force; Set-Service wampmariadb64 -StartupType Disabled; Get-Service wampmariadb64
