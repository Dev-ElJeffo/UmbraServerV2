# 🔧 MySQL Workbench Setup - Método Alternativo

**Recomendado**: Se você já usava MySQL Workbench, este é o método mais fácil!

---

## 🎯 Visão Geral

Este guia mostra como configurar o banco de dados usando **MySQL Workbench** em vez do WAMP/phpMyAdmin.

**Tempo**: ~3 minutos  
**Dificuldade**: ⭐ Fácil

---

## ✅ Pré-requisitos

- ✅ MySQL Server 8.0 instalado
- ✅ MySQL Workbench instalado
- ✅ Senha do root MySQL (ou sem senha)

**Localização detectada**: `C:\Program Files\MySQL\MySQL Server 8.0\`

---

## 🚀 Passo a Passo

### 1️⃣ Iniciar MySQL Server

**Opção A: Via Services (Recomendado)**

1. Pressione `Win + R`
2. Digite: `services.msc`
3. Encontre: **MySQL80**
4. Right-click → **Start**

**Opção B: Via PowerShell (Como Admin)**

```powershell
Start-Service MySQL80
```

**Verificar**:
```powershell
Get-Service MySQL80
# Deve mostrar Status: Running
```

---

### 2️⃣ Abrir MySQL Workbench

1. Abra **MySQL Workbench**
2. Clique na conexão: **Local instance MySQL80**
3. Digite a senha (ou apenas Enter se não tiver)
4. Clique **OK**

---

### 3️⃣ Criar o Banco de Dados

**Método 1: Executar Script SQL (Recomendado)**

1. No menu: **File → Open SQL Script...**
2. Navegue até: `D:\UmbraServerV2\UmbraServer\setup_database.sql`
3. Clique **Open**
4. Clique no ícone ⚡ (Execute) ou pressione `Ctrl+Shift+Enter`
5. Aguarde a mensagem: **"Database setup complete!"**

**Método 2: Copiar e Colar SQL**

1. Abra o arquivo `setup_database.sql` no Notepad
2. Copie todo o conteúdo (`Ctrl+A`, `Ctrl+C`)
3. Cole no MySQL Workbench (`Ctrl+V`)
4. Clique no ícone ⚡ (Execute) ou `Ctrl+Shift+Enter`

---

### 4️⃣ Verificar Instalação

Execute os seguintes comandos no MySQL Workbench:

```sql
-- Verificar se banco foi criado
SHOW DATABASES LIKE 'umbra_eternum';

-- Ver tabelas
USE umbra_eternum;
SHOW TABLES;

-- Verificar versão do schema
SELECT * FROM schema_version;
```

**Resultado esperado**:
```
Tables_in_umbra_eternum
- accounts
- players
- schema_version

version: 1.3.0
```

✅ Se ver isso, está tudo certo!

---

### 5️⃣ (Opcional) Criar Dados de Teste

```sql
USE umbra_eternum;

-- Criar conta de teste
INSERT INTO accounts (username, email, password_hash, salt)
VALUES (
  'testuser',
  'test@example.com',
  '$2b$10$abcdefghijklmnopqrstuv',  -- Hash exemplo
  'randomsalt123456789012345678'
);

-- Criar personagem de teste
INSERT INTO players (account_id, character_name, level, pos_x, pos_y, pos_z)
VALUES (
  LAST_INSERT_ID(),
  'TestHero',
  5,
  100.0,
  200.0,
  50.0
);

-- Verificar
SELECT 
  a.id as account_id,
  a.username,
  p.character_name,
  p.level,
  p.pos_x, p.pos_y, p.pos_z
FROM accounts a
LEFT JOIN players p ON a.id = p.account_id;
```

---

## 🔧 Configuração do Servidor C++

### Arquivo: `config/db.json`

O arquivo já está configurado corretamente:

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "umbra_eternum",
    "user": "root",
    "password": ""
  }
}
```

**⚠️ IMPORTANTE**: Se sua instalação MySQL tiver senha para root:

```json
{
  "database": {
    "password": "SUA_SENHA_AQUI"
  }
}
```

---

## ✅ Testar Conexão

### Via Testes C++

```powershell
cd D:\UmbraServerV2\UmbraServer\build

# Recompilar se necessário
cmake --build . --config Release

# Executar testes de banco
ctest -C Release -R DatabaseTests -V
```

**Esperado**: Testes passam sem timeout ✅

---

### Via Servidor Completo

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Saída esperada**:
```
===========================================
    UmbraEternum Server Stack v1.3.0      
===========================================

[INFO] Starting UmbraEternum Server Stack...
[INFO] Configuration loaded successfully
[INFO] Database connected successfully ✓
[OK] Auth Server started on port 8080
[OK] World Server started on port 8081
[OK] Gateway Server started on port 9000

===========================================
  All servers running. Press Ctrl+C to stop.
===========================================
```

---

## 🛠️ Troubleshooting

### Erro: "Can't connect to MySQL server on 'localhost'"

**Causa**: MySQL não está rodando

**Solução**:
```powershell
# Verificar status
Get-Service MySQL80

# Iniciar se necessário (como Admin)
Start-Service MySQL80
```

---

### Erro: "Access denied for user 'root'@'localhost'"

**Causa**: Senha incorreta ou expirada

**Solução**:

1. **Resetar senha** (se necessário):
   ```sql
   -- No MySQL Workbench, como root:
   ALTER USER 'root'@'localhost' IDENTIFIED BY '';
   FLUSH PRIVILEGES;
   ```

2. **Ou atualizar senha** em `config/db.json`:
   ```json
   "password": "sua_senha_atual"
   ```

---

### Erro: "Unknown database 'umbra_eternum'"

**Causa**: Script SQL não foi executado

**Solução**: Volte ao **Passo 3** e execute o script novamente.

---

### Erro: "Table 'accounts' doesn't exist"

**Causa**: Script executou parcialmente

**Solução**:
```sql
-- Limpar e recriar
DROP DATABASE IF EXISTS umbra_eternum;
-- Depois execute setup_database.sql novamente
```

---

## 📊 Queries Úteis

### Ver todas as contas
```sql
SELECT id, username, email, created_at, last_login_at 
FROM accounts 
ORDER BY created_at DESC;
```

### Ver todos os personagens
```sql
SELECT 
  p.id,
  p.character_name,
  p.level,
  p.current_zone,
  a.username as account_owner
FROM players p
JOIN accounts a ON p.account_id = a.id
ORDER BY p.level DESC;
```

### Estatísticas do servidor
```sql
SELECT 
  (SELECT COUNT(*) FROM accounts) as total_accounts,
  (SELECT COUNT(*) FROM players) as total_characters,
  (SELECT AVG(level) FROM players) as avg_level,
  (SELECT MAX(level) FROM players) as max_level;
```

---

## 🔒 Segurança (Produção)

### Criar usuário dedicado

```sql
-- Criar usuário específico para o servidor
CREATE USER 'umbra_server'@'localhost' IDENTIFIED BY 'SENHA_FORTE_123!@#';

-- Dar permissões apenas no banco necessário
GRANT SELECT, INSERT, UPDATE, DELETE ON umbra_eternum.* 
TO 'umbra_server'@'localhost';

-- Aplicar mudanças
FLUSH PRIVILEGES;
```

**Atualizar `config/db.json`**:
```json
{
  "database": {
    "user": "umbra_server",
    "password": "SENHA_FORTE_123!@#"
  }
}
```

---

## 📦 Backup e Restore

### Backup via Workbench

1. **Data Export**:
   - Menu: **Server → Data Export**
   - Selecione: `umbra_eternum`
   - Export to: **Self-Contained File**
   - Escolha local: `D:\Backups\umbra_eternum_backup.sql`
   - Clique: **Start Export**

### Backup via Command Line

```powershell
cd "C:\Program Files\MySQL\MySQL Server 8.0\bin"

# Backup completo
.\mysqldump.exe -u root -p umbra_eternum > D:\Backups\backup_$(Get-Date -Format 'yyyyMMdd_HHmmss').sql
```

### Restore

```powershell
# Via command line
.\mysql.exe -u root -p umbra_eternum < D:\Backups\backup_file.sql

# Ou via Workbench:
# Server → Data Import → Import from Self-Contained File
```

---

## 📈 Monitoramento

### Ver conexões ativas
```sql
SHOW PROCESSLIST;
```

### Ver status do servidor
```sql
SHOW STATUS LIKE 'Threads_connected';
SHOW STATUS LIKE 'Uptime';
SHOW STATUS LIKE 'Questions';
```

### Ver variáveis de configuração
```sql
SHOW VARIABLES LIKE 'max_connections';
SHOW VARIABLES LIKE 'innodb_buffer_pool_size';
```

---

## ✅ Checklist Final

- [ ] MySQL Server 8.0 rodando
- [ ] MySQL Workbench conectado
- [ ] Banco `umbra_eternum` criado
- [ ] Tabelas criadas (accounts, players, schema_version)
- [ ] `config/db.json` configurado
- [ ] Teste de conexão C++ passou
- [ ] Servidor inicia sem erros de DB

---

## 🎉 Pronto!

Seu banco de dados está configurado e pronto para uso!

**Próximos passos**:
1. Execute o servidor: `.\umbra_server.exe`
2. Monitore logs: `logs\umbra_server.log`
3. Comece a desenvolver! 🚀

---

## 🔗 Links Úteis

- **MySQL Workbench Manual**: https://dev.mysql.com/doc/workbench/en/
- **MySQL 8.0 Reference**: https://dev.mysql.com/doc/refman/8.0/en/
- **SQL Tutorial**: https://www.w3schools.com/sql/

---

## 💡 Dica Pro

**Crie uma conexão favorita no Workbench**:
1. Clique em **+** ao lado de "MySQL Connections"
2. Nome: `UmbraEternum Local`
3. Hostname: `localhost`
4. Port: `3306`
5. Username: `root`
6. Test Connection → Save

Agora você pode conectar rapidamente sempre que precisar! 🎯

---

**Método**: MySQL Workbench (GUI)  
**Dificuldade**: ⭐ Fácil  
**Tempo**: ~3 minutos  
**Status**: ✅ Recomendado

---

**Criado**: 2025-10-14  
**Versão**: 1.0  
**Para**: UmbraEternum Server v1.3.0

