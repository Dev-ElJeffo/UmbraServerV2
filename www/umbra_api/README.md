# UmbraEternum PHP API

API PHP para testes de comunicação com servidor C++ e banco de dados MySQL80.

## 📁 Estrutura

```
umbra_api/
├── config/
│   └── database.php      # Configuração MySQL80
├── api/
│   ├── test.php          # Testar conexão
│   ├── register.php      # Criar conta
│   └── login.php         # Autenticar
├── includes/
│   └── (futuro)
├── index.php             # UI de teste
└── README.md             # Este arquivo
```

## 🚀 Endpoints

### GET /api/test.php
Testa conexão com banco e retorna estatísticas

**Response**:
```json
{
    "success": true,
    "mysql_version": "8.0.x",
    "database": "umbra_eternum",
    "stats": {
        "accounts": 0,
        "players": 0
    }
}
```

### POST /api/register.php
Registra nova conta de usuário

**Body**:
```json
{
    "username": "player1",
    "email": "player1@test.com",
    "password": "senha123"
}
```

**Response**:
```json
{
    "success": true,
    "account_id": 1,
    "username": "player1"
}
```

### POST /api/login.php
Autentica usuário

**Body**:
```json
{
    "username": "player1",
    "password": "senha123"
}
```

**Response**:
```json
{
    "success": true,
    "token": "...",
    "account": {...},
    "players": [...]
}
```

## 🧪 Testar

### Via Browser
```
http://localhost/umbra_api/
```

### Via PowerShell
```powershell
# Test
Invoke-RestMethod -Uri "http://localhost/umbra_api/api/test.php"

# Register
$body = @{username="player1";email="p1@test.com";password="123"} | ConvertTo-Json
Invoke-RestMethod -Uri "http://localhost/umbra_api/api/register.php" -Method POST -Body $body -ContentType "application/json"

# Login
$body = @{username="player1";password="123"} | ConvertTo-Json
Invoke-RestMethod -Uri "http://localhost/umbra_api/api/login.php" -Method POST -Body $body -ContentType "application/json"
```

## ⚙️ Configuração

### Banco de Dados

Edite `config/database.php`:

```php
define('DB_HOST', 'localhost');
define('DB_PORT', 3306);
define('DB_NAME', 'umbra_eternum');
define('DB_USER', 'root');
define('DB_PASS', '!Mister4126'); // Sua senha
```

### PHP Extensions

Verifique em `php.ini`:
```ini
extension=mysqli
extension=pdo_mysql
```

## 📊 Stack

- PHP 7.4+
- MySQL 8.0
- PDO (database)
- JSON (responses)
- CORS enabled

## 🔗 Integração

Esta API conecta:
- Frontend (Browser/UE5) → PHP API → MySQL80
- PHP API e Servidor C++ compartilham mesmo MySQL

## 📝 Notas

- Password hashing: bcrypt
- Database: PDO prepared statements
- CORS: Habilitado para UE5
- JWT: TODO (implementar)

---

**Criado**: 2025-10-14
**Versão**: 1.0
**Para**: UmbraEternum Server v1.3.0

