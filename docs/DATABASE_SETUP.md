# 🗄️ Database Setup - UmbraEternum Server

Guia completo para configurar o banco de dados MySQL/MariaDB.

---

## 📋 Pré-requisitos

Você já tem WAMP instalado com MariaDB 11.5.2 rodando ✅

- **Serviço**: `wampmysqld64` (Running)
- **Porta**: 3306 (padrão)
- **User**: root
- **Password**: (vazio - padrão WAMP)

---

## 🚀 Configuração Rápida (5 minutos)

### Passo 1: Abrir phpMyAdmin

1. Acesse: http://localhost/phpmyadmin
2. Login: `root` / (sem senha)

### Passo 2: Criar Banco de Dados

**Opção A: Via Interface (Fácil)**

1. Clique em "New" na barra lateral
2. Nome do banco: `umbra_eternum`
3. Collation: `utf8mb4_unicode_ci`
4. Clique em "Create"

**Opção B: Via SQL (Recomendado)**

1. Clique na aba "SQL" no topo
2. Cole o conteúdo do arquivo `setup_database.sql`
3. Clique em "Go"

### Passo 3: Verificar

Execute no SQL:
```sql
SHOW DATABASES LIKE 'umbra_eternum';
SELECT * FROM schema_version;
```

Deve retornar: `version: 1.3.0`

---

## 📁 Schema do Banco

### Tabelas Criadas

#### 1. `accounts` - Contas de Usuário
```sql
id (BIGINT) - Primary Key
username (VARCHAR 20) - Único
email (VARCHAR 255) - Único
password_hash (VARCHAR 255)
salt (VARCHAR 32)
banned (BOOLEAN)
ban_reason (TEXT)
created_at (TIMESTAMP)
last_login_at (TIMESTAMP)
```

**Índices**: `username`, `email`

#### 2. `players` - Personagens
```sql
id (BIGINT) - Primary Key
account_id (BIGINT) - Foreign Key → accounts.id
character_name (VARCHAR 30) - Único
level (INT) - Padrão: 1
experience (BIGINT) - Padrão: 0
pos_x, pos_y, pos_z (FLOAT) - Posição no mundo
current_zone (VARCHAR 50) - Padrão: 'Zone_1'
health, max_health (INT) - Padrão: 100
mana, max_mana (INT) - Padrão: 100
stamina, max_stamina (INT) - Padrão: 100
strength, dexterity, intelligence, vitality (INT) - Padrão: 10
created_at (TIMESTAMP)
last_played_at (TIMESTAMP)
```

**Índices**: `account_id`, `character_name`  
**Foreign Key**: Cascade delete (deleta personagens se conta for deletada)

#### 3. `schema_version` - Controle de Versão
```sql
version (VARCHAR 20) - Primary Key
applied_at (TIMESTAMP)
```

---

## 🔧 Configuração do Servidor

### Arquivo: `config/db.json`

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "umbra_eternum",
    "user": "root",
    "password": ""
  },
  "redis": {
    "host": "localhost",
    "port": 6379,
    "password": "",
    "db": 0
  }
}
```

✅ **Já configurado para WAMP!**

---

## ✅ Testar Conexão

### Via Servidor C++

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Saída esperada**:
```
[INFO] Database connected successfully
[OK] Auth Server started on port 8080
[OK] World Server started on port 8081
[OK] Gateway Server started on port 9000
```

### Via Testes

```powershell
cd D:\UmbraServerV2\UmbraServer\build
ctest -C Release -R DatabaseTests -V
```

**Esperado**: Testes passam sem timeout ✅

---

## 📊 Dados de Teste (Opcional)

### Criar Conta de Teste

```sql
USE umbra_eternum;

INSERT INTO accounts (username, email, password_hash, salt)
VALUES (
  'testuser',
  'test@example.com',
  'hashed_password_here',
  'random_salt_32_chars_here'
);

INSERT INTO players (account_id, character_name, level)
VALUES (
  LAST_INSERT_ID(),
  'TestHero',
  10
);
```

### Verificar Dados

```sql
SELECT 
  a.username, 
  p.character_name, 
  p.level
FROM accounts a
LEFT JOIN players p ON a.id = p.account_id;
```

---

## 🛠️ Troubleshooting

### Erro: "Can't connect to MySQL server"

**Causa**: Serviço MySQL/MariaDB não está rodando

**Solução**:
```powershell
# Verificar serviço
Get-Service wampmysqld64

# Iniciar se necessário
Start-Service wampmysqld64
```

Ou via WAMP Manager: Left-click → MySQL → Service → Start/Resume Service

---

### Erro: "Access denied for user 'root'"

**Causa**: Senha incorreta

**Solução**:
1. Abra phpMyAdmin
2. User Accounts → root → Edit privileges
3. Verifique/remova senha
4. Atualize `config/db.json`

---

### Erro: "Table 'umbra_eternum.accounts' doesn't exist"

**Causa**: Schema não foi criado

**Solução**:
```sql
-- Re-executar o setup
SOURCE D:/UmbraServerV2/UmbraServer/setup_database.sql;
```

---

### Erro: "Unknown character set utf8mb4"

**Causa**: MariaDB antiga (< 5.5)

**Solução**: Atualize MariaDB ou mude para `utf8` em `setup_database.sql`

---

## 🔒 Segurança

### Para Produção

1. **Criar usuário dedicado**:
```sql
CREATE USER 'umbra_server'@'localhost' IDENTIFIED BY 'SENHA_FORTE_AQUI';
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* TO 'umbra_server'@'localhost';
FLUSH PRIVILEGES;
```

2. **Atualizar `config/db.json`**:
```json
{
  "database": {
    "user": "umbra_server",
    "password": "SENHA_FORTE_AQUI"
  }
}
```

3. **Remover acesso root sem senha**:
```sql
ALTER USER 'root'@'localhost' IDENTIFIED BY 'senha_root_forte';
```

---

## 📦 Backup

### Manual (Rápido)

```bash
# Via WAMP MariaDB
cd D:\UmbraServerV2\UmbraServer
& "C:\wamp64\bin\mariadb\mariadb11.5.2\bin\mysqldump.exe" -u root umbra_eternum > backup_$(Get-Date -Format 'yyyyMMdd_HHmmss').sql
```

### Automático (Script já incluído)

```bash
# Linux/Mac
./scripts/backup_db.sh

# Windows - criar backup_db.bat similar
```

### Restaurar Backup

```bash
& "C:\wamp64\bin\mariadb\mariadb11.5.2\bin\mysql.exe" -u root umbra_eternum < backup_file.sql
```

---

## 📈 Próximos Passos

### Tabelas Futuras (Planejadas)

- `items` - Inventário
- `guilds` - Guildas
- `quests` - Missões
- `achievements` - Conquistas
- `chat_logs` - Logs de chat
- `transactions` - Logs de transações
- `game_sessions` - Sessões de jogo

### Otimizações

- Índices compostos para queries frequentes
- Particionamento de tabelas grandes
- Read replicas para escalabilidade
- Cache Redis para dados quentes

---

## 🔗 Links Úteis

- **phpMyAdmin**: http://localhost/phpmyadmin
- **WAMP Manager**: Click no ícone na bandeja
- **MariaDB Docs**: https://mariadb.com/kb/en/
- **MySQL Workbench**: https://www.mysql.com/products/workbench/

---

## ✅ Checklist Final

- [ ] WAMP rodando
- [ ] Banco `umbra_eternum` criado
- [ ] Tabelas `accounts`, `players`, `schema_version` existem
- [ ] `config/db.json` configurado
- [ ] Testes de DB passando
- [ ] Servidor conecta ao banco sem erros

---

**Configurado com sucesso?** 🎉  
Agora execute `.\umbra_server.exe` e comece a desenvolver!

---

**Data**: 2025-10-14  
**Versão do Schema**: 1.3.0  
**Banco**: MariaDB 11.5.2 via WAMP

