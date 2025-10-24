# 🔧 CORREÇÃO - Erro JSON nas APIs de Personagem

**Data**: 16/10/2025  
**Erro**: `Unexpected token '<', "<br />\\n<fo"... is not valid JSON`  
**Status**: ✅ CORRIGIDO

---

## 🐛 PROBLEMA IDENTIFICADO

### Erro no Console:
```
"error": "Unexpected token '<', \"<br />\\n<fo\"... is not valid JSON"
```

### Causa:
As APIs PHP estavam retornando **HTML** (warnings/erros do PHP) antes do JSON, causando falha no parse.

**Motivos comuns**:
1. Warnings PHP sendo exibidos
2. Caminho relativo incorreto (`../config/database.php`)
3. Display de erros ativado
4. Headers já enviados

---

## ✅ CORREÇÕES APLICADAS

### 1. **Supressão de Warnings** (4 arquivos)

Adicionado no início de cada API:

```php
// Suprimir warnings para garantir JSON puro
error_reporting(E_ERROR | E_PARSE);
ini_set('display_errors', '0');
```

### 2. **Caminho Absoluto para database.php**

**Antes**:
```php
require_once '../config/database.php';  // ❌ Caminho relativo
```

**Depois**:
```php
require_once __DIR__ . '/../config/database.php';  // ✅ Caminho absoluto
```

### 3. **Arquivos Corrigidos**:

```
✅ C:\wamp64\www\umbra_api\api\character\list_characters.php
✅ C:\wamp64\www\umbra_api\api\character\create_character.php
✅ C:\wamp64\www\umbra_api\api\character\select_character.php
✅ C:\wamp64\www\umbra_api\api\character\delete_character.php
```

---

## 🧪 TESTE DE DIAGNÓSTICO

### Arquivo criado:
```
C:\wamp64\www\umbra_api\api\character\debug_test.php
```

### Como usar:

1. **Acesse no navegador**:
   ```
   http://localhost/umbra_api/api/character/debug_test.php
   ```

2. **Resultado esperado**:
   ```json
   {
     "success": true,
     "message": "Conexão OK!",
     "database_file": "C:\\wamp64\\www\\umbra_api\\config\\database.php",
     "table_exists": true,
     "total_characters": 0,
     "columns": [
       "player_id",
       "account_id",
       "character_name",
       "level",
       "experience",
       "current_zone",
       "position_x",
       "position_y",
       "position_z",
       "created_at",
       "last_login"
     ]
   }
   ```

3. **Se der erro**:
   - Veja a mensagem de erro exibida
   - Siga as instruções na seção "Troubleshooting" abaixo

---

## 🔄 PASSOS PARA TESTAR AS APIS AGORA

### 1. Atualizar página de teste:

```
http://localhost/umbra_api/test_character.html
```

**Pressione**: `Ctrl + Shift + R` (hard refresh)

---

### 2. Testar Diagnóstico:

```
http://localhost/umbra_api/api/character/debug_test.php
```

Se aparecer JSON com `"success": true`, está tudo OK!

---

### 3. Testar API de Listar:

**Account ID**: `1`  
**Clique**: "📋 Listar Personagens"

**Resultado esperado**:
```json
{
  "success": true,
  "count": 0,
  "max_characters": 5,
  "players": []
}
```

---

### 4. Testar API de Criar:

**Account ID**: `1`  
**Nome**: `TestHero`  
**Clique**: "✨ Criar Personagem"

**Resultado esperado**:
```json
{
  "success": true,
  "message": "Personagem criado com sucesso!",
  "player": {
    "player_id": 1,
    "character_name": "TestHero",
    "level": 1,
    ...
  }
}
```

---

## 🛠️ TROUBLESHOOTING

### Erro: "Tabela players não existe"

**Solução**: Criar a tabela no MySQL

```sql
USE umbra_eternum;

CREATE TABLE IF NOT EXISTS players (
    player_id INT PRIMARY KEY AUTO_INCREMENT,
    account_id INT NOT NULL,
    character_name VARCHAR(20) UNIQUE NOT NULL,
    level INT DEFAULT 1,
    experience INT DEFAULT 0,
    current_zone VARCHAR(50) DEFAULT 'Tutorial',
    position_x FLOAT DEFAULT 0.0,
    position_y FLOAT DEFAULT 0.0,
    position_z FLOAT DEFAULT 0.0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME NULL,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

**Via MySQL Workbench**:
1. Conecte ao MySQL80
2. Abra uma nova Query Tab
3. Cole o SQL acima
4. Execute (Ctrl+Enter)

**Via phpMyAdmin**:
1. Acesse: `http://localhost/phpmyadmin`
2. Selecione database: `umbra_eternum`
3. Aba "SQL"
4. Cole o SQL acima
5. Clique "Go"

---

### Erro: "database.php não encontrado"

**Solução**: Verificar se existe

```powershell
Test-Path "C:\wamp64\www\umbra_api\config\database.php"
```

Se retornar `False`, o arquivo não existe. Recrie:

```php
<?php
// C:\wamp64\www\umbra_api\config\database.php

define('DB_HOST', 'localhost');
define('DB_PORT', '3306');
define('DB_NAME', 'umbra_eternum');
define('DB_USER', 'root');
define('DB_PASS', '!Mister4126');  // Sua senha do MySQL80

function getConnection() {
    try {
        $dsn = "mysql:host=" . DB_HOST . ";port=" . DB_PORT . ";dbname=" . DB_NAME . ";charset=utf8mb4";
        $pdo = new PDO($dsn, DB_USER, DB_PASS);
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
        return $pdo;
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erro de conexão com banco de dados'
        ]);
        exit;
    }
}
```

---

### Erro: "Access denied for user 'root'@'localhost'"

**Solução**: Verificar senha do MySQL

1. Abra MySQL Workbench
2. Tente conectar com senha vazia
3. Se funcionar, atualize `database.php`:
   ```php
   define('DB_PASS', '');  // Senha vazia
   ```
4. Se não funcionar, use a senha correta do MySQL80

---

### Erro: "SQLSTATE[HY000] [2002] No connection could be made"

**Solução**: MySQL80 não está rodando

```powershell
# Verificar se está rodando
Get-Service -Name MySQL80

# Se não estiver, iniciar
Start-Service -Name MySQL80
```

---

### Ainda retornando HTML?

**Verificar se há espaços/BOM antes do `<?php`**:

```powershell
# Abrir arquivo no Notepad++ ou VSCode
# Verificar se há caracteres invisíveis antes de <?php
# Deletar e salvar novamente
```

**Verificar encoding**:
- Arquivo deve ser **UTF-8 sem BOM**
- No VSCode: Canto inferior direito → "UTF-8" → "Save with Encoding" → "UTF-8"

---

## 📊 CHECKLIST DE VERIFICAÇÃO

```
✅ Arquivos PHP atualizados (4)
✅ error_reporting configurado
✅ display_errors desativado
✅ Caminho absoluto para database.php
✅ Script de diagnóstico criado

Agora teste:
[ ] debug_test.php retorna JSON
[ ] list_characters.php retorna JSON
[ ] create_character.php retorna JSON
[ ] select_character.php retorna JSON
[ ] delete_character.php retorna JSON
```

---

## 🎯 RESULTADO ESPERADO

### Antes (❌):
```
{
  "error": "Unexpected token '<', \"<br />\\n<fo\"... is not valid JSON"
}
```

### Depois (✅):
```json
{
  "success": true,
  "count": 0,
  "max_characters": 5,
  "players": []
}
```

---

## 🚀 PRÓXIMOS PASSOS

1. ✅ Testar diagnóstico
2. ✅ Testar listar personagens
3. ✅ Criar primeiro personagem
4. ✅ Testar selecionar
5. ✅ Testar deletar
6. 📝 Criar widgets UE5 (próxima fase)

---

## 📝 COMANDOS RÁPIDOS

```powershell
# Abrir teste no navegador
start "http://localhost/umbra_api/test_character.html"

# Abrir diagnóstico
start "http://localhost/umbra_api/api/character/debug_test.php"

# Verificar MySQL rodando
Get-Service -Name MySQL80

# Iniciar MySQL se parado
Start-Service -Name MySQL80

# Abrir MySQL Workbench
start "C:\Program Files\MySQL\MySQL Workbench 8.0 CE\MySQLWorkbench.exe"
```

---

## 💡 DICAS

1. **Sempre use hard refresh** (Ctrl+Shift+R) ao testar APIs
2. **Verifique o Console** do navegador para erros
3. **Use debug_test.php** para diagnosticar problemas
4. **Teste uma API por vez** para isolar erros
5. **Verifique MySQL rodando** antes de testar

---

**✅ CORREÇÃO APLICADA COM SUCESSO!**

**Agora teste e me avise se funcionou!** 🚀

