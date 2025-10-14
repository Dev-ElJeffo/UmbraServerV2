# 🌐 WAMP + PHP API Setup para UmbraEternum

**Objetivo**: Configurar WAMP para criar APIs PHP que testam comunicação com servidor C++ e MySQL

---

## 🎯 Arquitetura de Teste

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   Cliente   │ ───> │  PHP APIs   │ ───> │   MySQL80   │
│ (Browser/UE)│ HTTP │  (WAMP)     │ SQL  │  Database   │
└─────────────┘      └─────────────┘      └─────────────┘
                            │
                            ↓ REST
                     ┌─────────────┐
                     │  C++ Server │
                     │  (Auth/etc) │
                     └─────────────┘
```

---

## 🔧 Configuração do WAMP

### Etapa 1: Parar MySQL do WAMP

**PowerShell como Administrador**:

```powershell
# Parar e desabilitar MySQL do WAMP
Stop-Service wampmysqld64 -ErrorAction SilentlyContinue
Set-Service wampmysqld64 -StartupType Disabled -ErrorAction SilentlyContinue

# Garantir que MySQL80 está rodando
Start-Service MySQL80
Get-Service MySQL80
```

---

### Etapa 2: Configurar PHP para MySQL80

**Arquivo**: `C:\wamp64\bin\apache\apache2.4.x\bin\php.ini`

Ou use o ícone do WAMP → PHP → php.ini

**Verificar extensões**:
```ini
extension=mysqli
extension=pdo_mysql
```

---

### Etapa 3: Criar Estrutura de API

**Diretório**: `C:\wamp64\www\umbra_api\`

```
C:\wamp64\www\umbra_api\
├── config\
│   └── database.php      # Conexão MySQL
├── api\
│   ├── register.php      # Criar conta
│   ├── login.php         # Autenticar
│   ├── logout.php        # Deslogar
│   └── test.php          # Testar conexão
├── includes\
│   ├── functions.php     # Funções auxiliares
│   └── auth.php          # Autenticação
└── index.php             # Landing page
```

---

## 📁 Criar Arquivos PHP

### 1. config/database.php

```php
<?php
// Configuração de conexão com MySQL80
define('DB_HOST', 'localhost');
define('DB_PORT', 3306);
define('DB_NAME', 'umbra_eternum');
define('DB_USER', 'root');
define('DB_PASS', ''); // Senha vazia (ou sua senha)

class Database {
    private $conn;
    
    public function connect() {
        $this->conn = null;
        
        try {
            $this->conn = new PDO(
                "mysql:host=" . DB_HOST . 
                ";port=" . DB_PORT . 
                ";dbname=" . DB_NAME,
                DB_USER,
                DB_PASS
            );
            $this->conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $this->conn->exec("set names utf8mb4");
        } catch(PDOException $e) {
            echo "Connection Error: " . $e->getMessage();
        }
        
        return $this->conn;
    }
}
?>
```

---

### 2. api/test.php - Testar Conexão

```php
<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');

include_once '../config/database.php';

$response = array();

try {
    $database = new Database();
    $db = $database->connect();
    
    if ($db) {
        // Testar query
        $query = "SELECT VERSION() as version, DATABASE() as database";
        $stmt = $db->prepare($query);
        $stmt->execute();
        $result = $stmt->fetch(PDO::FETCH_ASSOC);
        
        // Contar registros
        $query = "SELECT 
            (SELECT COUNT(*) FROM accounts) as total_accounts,
            (SELECT COUNT(*) FROM players) as total_players,
            (SELECT version FROM schema_version LIMIT 1) as schema_version";
        $stmt = $db->prepare($query);
        $stmt->execute();
        $counts = $stmt->fetch(PDO::FETCH_ASSOC);
        
        $response['success'] = true;
        $response['message'] = 'Conexão bem-sucedida!';
        $response['mysql_version'] = $result['version'];
        $response['database'] = $result['database'];
        $response['schema_version'] = $counts['schema_version'];
        $response['stats'] = array(
            'accounts' => (int)$counts['total_accounts'],
            'players' => (int)$counts['total_players']
        );
    }
} catch (Exception $e) {
    $response['success'] = false;
    $response['message'] = 'Erro: ' . $e->getMessage();
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>
```

---

### 3. api/register.php - Criar Conta

```php
<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

include_once '../config/database.php';

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Receber dados JSON
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->username) && !empty($data->email) && !empty($data->password)) {
        try {
            $database = new Database();
            $db = $database->connect();
            
            // Verificar se username já existe
            $query = "SELECT id FROM accounts WHERE username = :username";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':username', $data->username);
            $stmt->execute();
            
            if ($stmt->rowCount() > 0) {
                $response['success'] = false;
                $response['message'] = 'Username já existe';
            } else {
                // Hash da senha
                $password_hash = password_hash($data->password, PASSWORD_BCRYPT);
                $salt = bin2hex(random_bytes(16));
                
                // Inserir conta
                $query = "INSERT INTO accounts (username, email, password_hash, salt) 
                         VALUES (:username, :email, :password_hash, :salt)";
                $stmt = $db->prepare($query);
                $stmt->bindParam(':username', $data->username);
                $stmt->bindParam(':email', $data->email);
                $stmt->bindParam(':password_hash', $password_hash);
                $stmt->bindParam(':salt', $salt);
                
                if ($stmt->execute()) {
                    $account_id = $db->lastInsertId();
                    
                    $response['success'] = true;
                    $response['message'] = 'Conta criada com sucesso!';
                    $response['account_id'] = $account_id;
                    $response['username'] = $data->username;
                } else {
                    $response['success'] = false;
                    $response['message'] = 'Erro ao criar conta';
                }
            }
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'Dados incompletos (username, email, password)';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>
```

---

### 4. api/login.php - Autenticar

```php
<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');

include_once '../config/database.php';

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->username) && !empty($data->password)) {
        try {
            $database = new Database();
            $db = $database->connect();
            
            // Buscar conta
            $query = "SELECT id, username, email, password_hash, banned, ban_reason 
                     FROM accounts WHERE username = :username";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':username', $data->username);
            $stmt->execute();
            
            if ($stmt->rowCount() > 0) {
                $account = $stmt->fetch(PDO::FETCH_ASSOC);
                
                // Verificar ban
                if ($account['banned']) {
                    $response['success'] = false;
                    $response['message'] = 'Conta banida: ' . $account['ban_reason'];
                }
                // Verificar senha
                else if (password_verify($data->password, $account['password_hash'])) {
                    // Atualizar last_login
                    $update = "UPDATE accounts SET last_login_at = NOW() WHERE id = :id";
                    $stmt = $db->prepare($update);
                    $stmt->bindParam(':id', $account['id']);
                    $stmt->execute();
                    
                    // Buscar personagens
                    $query = "SELECT id, character_name, level, current_zone 
                             FROM players WHERE account_id = :account_id";
                    $stmt = $db->prepare($query);
                    $stmt->bindParam(':account_id', $account['id']);
                    $stmt->execute();
                    $players = $stmt->fetchAll(PDO::FETCH_ASSOC);
                    
                    $response['success'] = true;
                    $response['message'] = 'Login bem-sucedido!';
                    $response['account'] = array(
                        'id' => $account['id'],
                        'username' => $account['username'],
                        'email' => $account['email']
                    );
                    $response['players'] = $players;
                    
                    // TODO: Gerar JWT token aqui
                    $response['token'] = base64_encode($account['username'] . ':' . time());
                } else {
                    $response['success'] = false;
                    $response['message'] = 'Senha incorreta';
                }
            } else {
                $response['success'] = false;
                $response['message'] = 'Username não encontrado';
            }
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'Username e senha são obrigatórios';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>
```

---

### 5. index.php - Landing Page

```php
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>UmbraEternum API Test</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            padding: 30px;
            box-shadow: 0 10px 50px rgba(0,0,0,0.2);
        }
        h1 { color: #667eea; margin-bottom: 20px; }
        .status { padding: 15px; border-radius: 5px; margin: 20px 0; }
        .success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .endpoint {
            background: #f8f9fa;
            padding: 15px;
            border-left: 4px solid #667eea;
            margin: 15px 0;
        }
        .endpoint h3 { color: #667eea; margin-bottom: 10px; }
        .code {
            background: #2d2d2d;
            color: #f8f8f2;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            margin: 10px 0;
        }
        button {
            background: #667eea;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            margin: 5px;
        }
        button:hover { background: #5568d3; }
        #result { margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎮 UmbraEternum API Test Panel</h1>
        
        <div id="status" class="status">
            Carregando status da conexão...
        </div>
        
        <h2>🧪 Endpoints Disponíveis</h2>
        
        <div class="endpoint">
            <h3>GET /api/test.php</h3>
            <p>Testa conexão com banco de dados e retorna estatísticas</p>
            <button onclick="testConnection()">Testar Conexão</button>
        </div>
        
        <div class="endpoint">
            <h3>POST /api/register.php</h3>
            <p>Registra nova conta de usuário</p>
            <div class="code">
{
    "username": "player1",
    "email": "player1@test.com",
    "password": "senha123"
}
            </div>
            <button onclick="testRegister()">Testar Register</button>
        </div>
        
        <div class="endpoint">
            <h3>POST /api/login.php</h3>
            <p>Autentica usuário e retorna token + personagens</p>
            <div class="code">
{
    "username": "player1",
    "password": "senha123"
}
            </div>
            <button onclick="testLogin()">Testar Login</button>
        </div>
        
        <div id="result"></div>
    </div>
    
    <script>
        // Testar conexão ao carregar
        window.onload = function() {
            testConnection();
        };
        
        function testConnection() {
            fetch('api/test.php')
                .then(response => response.json())
                .then(data => {
                    const status = document.getElementById('status');
                    if (data.success) {
                        status.className = 'status success';
                        status.innerHTML = `
                            <strong>✅ Conexão OK!</strong><br>
                            MySQL: ${data.mysql_version}<br>
                            Database: ${data.database}<br>
                            Schema: ${data.schema_version}<br>
                            Contas: ${data.stats.accounts} | Jogadores: ${data.stats.players}
                        `;
                    } else {
                        status.className = 'status error';
                        status.innerHTML = `<strong>❌ Erro:</strong> ${data.message}`;
                    }
                    showResult(data);
                })
                .catch(error => {
                    const status = document.getElementById('status');
                    status.className = 'status error';
                    status.innerHTML = `<strong>❌ Erro de conexão:</strong> ${error}`;
                });
        }
        
        function testRegister() {
            const data = {
                username: 'testuser_' + Math.floor(Math.random() * 1000),
                email: 'test' + Math.floor(Math.random() * 1000) + '@example.com',
                password: 'senha123'
            };
            
            fetch('api/register.php', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(response => response.json())
            .then(result => {
                showResult(result);
                if (result.success) testConnection();
            });
        }
        
        function testLogin() {
            const data = {
                username: 'player1',
                password: 'senha123'
            };
            
            fetch('api/login.php', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(response => response.json())
            .then(result => showResult(result));
        }
        
        function showResult(data) {
            const result = document.getElementById('result');
            result.innerHTML = `
                <h3>📊 Resultado:</h3>
                <div class="code">${JSON.stringify(data, null, 2)}</div>
            `;
        }
    </script>
</body>
</html>
```

---

## 🚀 EXECUTAR SETUP

Execute o script abaixo para criar toda a estrutura:

```powershell
# Criar estrutura de diretórios
$apiDir = "C:\wamp64\www\umbra_api"
New-Item -ItemType Directory -Path "$apiDir\config" -Force | Out-Null
New-Item -ItemType Directory -Path "$apiDir\api" -Force | Out-Null
New-Item -ItemType Directory -Path "$apiDir\includes" -Force | Out-Null

Write-Host "✓ Estrutura criada em: $apiDir"
Write-Host ""
Write-Host "Agora copie os arquivos PHP dos exemplos acima"
Write-Host ""
Write-Host "Acesse: http://localhost/umbra_api/"
```

---

## ✅ Testar API

### 1. Via Browser

```
http://localhost/umbra_api/
```

### 2. Via PowerShell (curl)

```powershell
# Testar conexão
Invoke-RestMethod -Uri "http://localhost/umbra_api/api/test.php"

# Registrar usuário
$body = @{
    username = "player1"
    email = "player1@test.com"
    password = "senha123"
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/register.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"

# Login
$body = @{
    username = "player1"
    password = "senha123"
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost/umbra_api/api/login.php" `
    -Method POST `
    -Body $body `
    -ContentType "application/json"
```

### 3. Via Postman

- Method: POST
- URL: `http://localhost/umbra_api/api/register.php`
- Headers: `Content-Type: application/json`
- Body (raw JSON):
```json
{
    "username": "player1",
    "email": "player1@test.com",
    "password": "senha123"
}
```

---

## 🔧 Troubleshooting

### WAMP não inicia Apache

```powershell
# Verificar porta 80
netstat -ano | findstr ":80"

# Se algo estiver usando, pare o serviço
# Exemplo: IIS
Stop-Service W3SVC -ErrorAction SilentlyContinue
```

### Erro "Call to undefined function mysqli_connect"

Editar `php.ini`:
```ini
extension=mysqli
extension=pdo_mysql
```

Reiniciar WAMP

### phpMyAdmin ainda com erro?

Use a API PHP diretamente! Mais leve e direto.

---

## 📊 Fluxo Completo de Teste

```
1. Cliente → PHP API (register.php)
   ├─> Validar dados
   ├─> Hash senha
   └─> INSERT em MySQL80

2. Cliente → PHP API (login.php)
   ├─> Buscar conta
   ├─> Verificar senha
   ├─> Gerar token
   └─> Retornar dados

3. Cliente → C++ Server (com token)
   ├─> Validar token
   ├─> Conectar ao jogo
   └─> Sincronizar com MySQL80
```

---

## 🎯 Próximos Passos

1. ✅ Criar arquivos PHP
2. ✅ Testar cada endpoint
3. ✅ Integrar com servidor C++
4. ✅ Adicionar JWT real
5. ✅ Criar API para personagens
6. ✅ Integrar com UE5

---

**Criado**: 2025-10-14  
**Para**: UmbraEternum Server v1.3.0  
**Stack**: PHP 7.4+ + MySQL 8.0 + WAMP

