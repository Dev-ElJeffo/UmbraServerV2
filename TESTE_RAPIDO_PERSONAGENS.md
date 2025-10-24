# 🧪 TESTE RÁPIDO - APIs de Personagens

**Data**: 16/10/2025  
**Status**: ✅ APIs Atualizadas com Sistema Robusto  

---

## 🎯 TESTE EM 5 PASSOS

### **PASSO 1: Diagnóstico** ⏱️ 10 segundos

Abra no navegador:
```
http://localhost/umbra_api/api/character/debug_test.php
```

**✅ Sucesso**: JSON com `"success": true`  
**❌ Erro**: Veja seção "Troubleshooting" abaixo

---

### **PASSO 2: Hard Refresh** ⏱️ 5 segundos

Abra:
```
http://localhost/umbra_api/test_character.html
```

**Pressione**: `Ctrl + Shift + R` (limpa cache)

---

### **PASSO 3: Listar Personagens** ⏱️ 10 segundos

1. Account ID: `1`
2. Clique: **"📋 Listar Personagens"**

**✅ Esperado**:
```json
{
  "success": true,
  "count": 0,
  "max_characters": 5,
  "players": []
}
```

---

### **PASSO 4: Criar Personagem** ⏱️ 15 segundos

1. Account ID: `1`
2. Nome: `ElJeffo`
3. Clique: **"✨ Criar Personagem"**

**✅ Esperado**:
```json
{
  "success": true,
  "message": "Personagem criado com sucesso!",
  "player": {
    "player_id": 1,
    "character_name": "ElJeffo",
    "level": 1,
    "experience": 0,
    "current_zone": "Tutorial"
  }
}
```

---

### **PASSO 5: Verificar Lista** ⏱️ 5 segundos

1. Account ID: `1`
2. Clique: **"📋 Listar Personagens"** novamente

**✅ Esperado**: Agora deve mostrar **1 personagem** na lista!

---

## ⏱️ TEMPO TOTAL: ~45 segundos

---

## 🛠️ TROUBLESHOOTING

### ❌ Erro: "Tabela players não existe"

**Solução**: Criar tabela no MySQL

#### Via MySQL Workbench:

1. Abra MySQL Workbench
2. Conecte ao **MySQL80**
3. Abra nova Query Tab
4. Cole e execute:

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

#### Via PowerShell:

```powershell
# 1. Salvar SQL em arquivo temporário
$sql = @"
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"@

$sql | Out-File -FilePath "$env:TEMP\create_players.sql" -Encoding UTF8

# 2. Executar no MySQL
& "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p"!Mister4126" -P 3306 < "$env:TEMP\create_players.sql"

# 3. Verificar
& "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p"!Mister4126" -P 3306 -e "USE umbra_eternum; SHOW TABLES LIKE 'players';"
```

---

### ❌ Erro: "Conta não encontrada"

**Causa**: Account ID não existe

**Solução**: Use um account_id válido

#### Verificar accounts existentes:

```powershell
& "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p"!Mister4126" -P 3306 -e "USE umbra_eternum; SELECT account_id, username FROM accounts;"
```

Ou via phpMyAdmin:
```
http://localhost/phpmyadmin
→ umbra_eternum
→ accounts
```

---

### ❌ Erro: "Unexpected end of JSON input"

**Causa**: Cache do navegador

**Solução**: Hard refresh

```
Ctrl + Shift + R
ou
Ctrl + F5
```

---

### ❌ MySQL não está rodando

```powershell
# Verificar status
Get-Service -Name MySQL80

# Se parado, iniciar
Start-Service -Name MySQL80
```

---

## 📊 RESULTADO ESPERADO COMPLETO

### **1. debug_test.php**:
```json
{
  "success": true,
  "message": "✅ Todos os testes passaram!",
  "database_file": "C:/wamp64/www/umbra_api/config/database.php",
  "connection": "OK",
  "table_players_exists": true,
  "table_accounts_exists": true,
  "total_characters": 1,
  "total_accounts": 1,
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
  ],
  "php_version": "7.4.x",
  "pdo_drivers": ["mysql"]
}
```

### **2. list_characters (vazio)**:
```json
{
  "success": true,
  "count": 0,
  "max_characters": 5,
  "players": []
}
```

### **3. create_character**:
```json
{
  "success": true,
  "message": "Personagem criado com sucesso!",
  "player": {
    "player_id": 1,
    "account_id": 1,
    "character_name": "ElJeffo",
    "level": 1,
    "experience": 0,
    "current_zone": "Tutorial",
    "position": {
      "x": 0,
      "y": 0,
      "z": 0
    },
    "created_at": "2025-10-16 ...",
    "last_login": null
  }
}
```

### **4. list_characters (com personagem)**:
```json
{
  "success": true,
  "count": 1,
  "max_characters": 5,
  "players": [
    {
      "player_id": 1,
      "account_id": 1,
      "character_name": "ElJeffo",
      "level": 1,
      "experience": 0,
      "current_zone": "Tutorial",
      "position": { "x": 0, "y": 0, "z": 0 },
      "created_at": "2025-10-16 ...",
      "last_login": null
    }
  ]
}
```

---

## 🎉 SE TUDO FUNCIONOU

**Próximo passo**: Criar widgets UE5!

Abra o guia:
```
D:\UmbraServerV2\UmbraServer\GUIA_WIDGETS_PERSONAGENS_UE5.md
```

**Ou no VSCode**:
```powershell
code "D:\UmbraServerV2\UmbraServer\GUIA_WIDGETS_PERSONAGENS_UE5.md"
```

---

## 🚀 COMANDOS RÁPIDOS

```powershell
# Abrir diagnóstico
start "http://localhost/umbra_api/api/character/debug_test.php"

# Abrir teste
start "http://localhost/umbra_api/test_character.html"

# Verificar MySQL
Get-Service -Name MySQL80

# Iniciar MySQL
Start-Service -Name MySQL80

# MySQL Workbench
start "C:\Program Files\MySQL\MySQL Workbench 8.0 CE\MySQLWorkbench.exe"

# Abrir guia de widgets
code "D:\UmbraServerV2\UmbraServer\GUIA_WIDGETS_PERSONAGENS_UE5.md"
```

---

## 📸 COMO REPORTAR ERRO

Se algo der errado:

1. **Tire print** da tela com erro
2. **Copie** o JSON de erro
3. **Execute** debug_test.php
4. **Envie** todos os 3 para análise

---

**✅ TESTE AGORA E ME AVISE O RESULTADO!** 🚀

