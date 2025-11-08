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
        .nav-links {
            display: flex;
            gap: 15px;
            margin-bottom: 20px;
            padding: 15px;
            background: #f8f9fa;
            border-radius: 8px;
            justify-content: center;
        }
        .nav-links a {
            color: #667eea;
            text-decoration: none;
            font-weight: 600;
            padding: 8px 16px;
            border-radius: 5px;
            transition: all 0.3s;
        }
        .nav-links a:hover {
            background: #667eea;
            color: white;
        }
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
        
        <div class="nav-links">
            <a href="register.html">➕ Criar Conta</a>
            <a href="login.html">🔐 Login</a>
            <a href="dashboard.html">📊 Dashboard</a>
        </div>
        
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

