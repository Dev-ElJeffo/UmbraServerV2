# ⚡ Quick Start - PHP API (5 minutos)

**Objetivo**: Criar APIs PHP para testar comunicação com servidor C++ e MySQL

---

## 🎯 O Que Vamos Fazer

```
Cliente/Browser → PHP API → MySQL80 → Servidor C++
```

APIs:
- `/api/test.php` - Testar conexão
- `/api/register.php` - Criar conta
- `/api/login.php` - Autenticar usuário

---

## ⚡ SETUP RÁPIDO (Opção 1 - Automático)

### Passo 1: Executar Script

```powershell
cd D:\UmbraServerV2\UmbraServer
.\setup_php_api.ps1
```

### Passo 2: Copiar Arquivos PHP

Copie os códigos do arquivo: `WAMP_PHP_API_SETUP.md`

Para:
- `C:\wamp64\www\umbra_api\config\database.php`
- `C:\wamp64\www\umbra_api\api\test.php`
- `C:\wamp64\www\umbra_api\api\register.php`
- `C:\wamp64\www\umbra_api\api\login.php`
- `C:\wamp64\www\umbra_api\index.php`

### Passo 3: Testar

```
http://localhost/umbra_api/
```

---

## 🔧 SETUP MANUAL (Opção 2)

### 1. Criar Estrutura

```powershell
cd C:\wamp64\www
mkdir umbra_api\config, umbra_api\api, umbra_api\includes
```

### 2. Criar database.php

**Arquivo**: `C:\wamp64\www\umbra_api\config\database.php`

```php
<?php
define('DB_HOST', 'localhost');
define('DB_NAME', 'umbra_eternum');
define('DB_USER', 'root');
define('DB_PASS', ''); // Sem senha

class Database {
    private $conn;
    public function connect() {
        $this->conn = new PDO(
            "mysql:host=" . DB_HOST . ";dbname=" . DB_NAME,
            DB_USER, DB_PASS
        );
        return $this->conn;
    }
}
?>
```

### 3. Criar test.php

**Arquivo**: `C:\wamp64\www\umbra_api\api\test.php`

```php
<?php
header('Content-Type: application/json');
include_once '../config/database.php';

try {
    $db = (new Database())->connect();
    $query = "SELECT COUNT(*) as total FROM accounts";
    $stmt = $db->query($query);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    echo json_encode([
        'success' => true,
        'total_accounts' => $result['total']
    ]);
} catch (Exception $e) {
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}
?>
```

### 4. Testar

```
http://localhost/umbra_api/api/test.php
```

Deve retornar:
```json
{
    "success": true,
    "total_accounts": 0
}
```

---

## 🧪 TESTAR COM CURL

### Testar Conexão

```powershell
curl http://localhost/umbra_api/api/test.php
```

### Registrar Usuário

```powershell
$body = @{username="player1";email="p1@test.com";password="123"} | ConvertTo-Json
Invoke-RestMethod -Uri http://localhost/umbra_api/api/register.php -Method POST -Body $body -ContentType "application/json"
```

### Login

```powershell
$body = @{username="player1";password="123"} | ConvertTo-Json
Invoke-RestMethod -Uri http://localhost/umbra_api/api/login.php -Method POST -Body $body -ContentType "application/json"
```

---

## 🔧 Configurar WAMP para MySQL80

### PowerShell (Como Admin)

```powershell
# Parar MySQL do WAMP
Stop-Service wampmysqld64 -Force
Set-Service wampmysqld64 -StartupType Disabled

# Garantir MySQL80 rodando
Start-Service MySQL80

# Verificar
Get-Service MySQL80
```

---

## 📁 Estrutura Final

```
C:\wamp64\www\umbra_api\
├── config\
│   └── database.php      # Conexão MySQL
├── api\
│   ├── test.php          # Testar conexão
│   ├── register.php      # Criar conta
│   └── login.php         # Autenticar
└── index.php             # UI de teste
```

---

## ✅ Checklist

- [ ] WAMP instalado e Apache rodando
- [ ] MySQL80 rodando (não WAMP MySQL)
- [ ] Estrutura `umbra_api/` criada
- [ ] `database.php` configurado
- [ ] `test.php` responde corretamente
- [ ] APIs de register e login funcionando

---

## 🛠️ Troubleshooting

### Apache não inicia

```powershell
# Verificar porta 80
netstat -ano | findstr ":80"

# Se IIS estiver rodando
Stop-Service W3SVC
```

### Erro "Access denied"

Edite `config/database.php`:
```php
define('DB_PASS', ''); // Deixe vazio
```

### JSON não aparece

Verifique `php.ini`:
```ini
extension=mysqli
extension=pdo_mysql
```

Reinicie Apache pelo WAMP

---

## 🚀 Próximos Passos

1. ✅ Testar cada endpoint
2. ✅ Integrar com servidor C++
3. ✅ Adicionar JWT real
4. ✅ Criar mais APIs (personagens, inventário)
5. ✅ Integrar com UE5

---

## 📖 Documentação Completa

Ver: `WAMP_PHP_API_SETUP.md` (guia detalhado)

---

**Tempo**: 5 minutos  
**Dificuldade**: ⭐⭐ Médio  
**Resultado**: APIs PHP funcionando com MySQL80

👉 **COMECE AGORA**: `.\setup_php_api.ps1`

