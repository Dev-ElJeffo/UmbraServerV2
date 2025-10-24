# 🔧 MySQL Workbench - Criar Nova Conexão (MySQL80)

**Problema Identificado**: Workbench está conectando no WAMP que fecha sozinho!  
**Solução**: Criar conexão direta para MySQL80

---

## 🎯 O Problema

Você está vendo:
```
MySQL Workbench → "Local instance wampmysqld64"
                   ↓
                  WAMP fecha sozinho
                   ↓
                  Conexão perdida ❌
```

**Precisamos**:
```
MySQL Workbench → "UmbraEternum MySQL80"
                   ↓
                  MySQL80 (estável)
                   ↓
                  Funcionando! ✅
```

---

## 📋 Passo a Passo Detalhado

### Etapa 1: Voltar para Tela Inicial

1. Se tiver uma conexão aberta, **feche** a aba
2. Menu: **Database → Disconnect**
3. Menu: **File → Close Tab** (ou Ctrl+W)
4. Você deve ver a tela "Welcome to MySQL Workbench"

---

### Etapa 2: Criar Nova Conexão

#### Passo 2.1: Clicar no Botão +

Na seção **"MySQL Connections"**:
- Procure o ícone **⊕** (mais/plus) ao lado direito
- Ou clique em **"+"** próximo às conexões existentes
- Clique nele

#### Passo 2.2: Preencher Formulário "Setup New Connection"

```
╔═══════════════════════════════════════════════════╗
║  Setup New Connection                             ║
╠═══════════════════════════════════════════════════╣
║                                                   ║
║  Connection Name:                                 ║
║  ┌─────────────────────────────────────────────┐ ║
║  │ UmbraEternum MySQL80                        │ ║
║  └─────────────────────────────────────────────┘ ║
║                                                   ║
║  Connection Method:                               ║
║  ┌─────────────────────────────────────────────┐ ║
║  │ Standard (TCP/IP)                ▼          │ ║
║  └─────────────────────────────────────────────┘ ║
║                                                   ║
║  Parameters:                                      ║
║  ┌─────────────────────────────────────────────┐ ║
║  │ Hostname:     localhost                     │ ║
║  │ Port:         3306                          │ ║
║  │ Username:     root                          │ ║
║  │ Password:     [Store in Vault...]          │ ║
║  │ Default Schema: (leave empty)               │ ║
║  └─────────────────────────────────────────────┘ ║
║                                                   ║
║         [Test Connection]    [Cancel]    [OK]    ║
╚═══════════════════════════════════════════════════╝
```

**Valores exatos**:
- **Connection Name**: `UmbraEternum MySQL80`
- **Connection Method**: `Standard (TCP/IP)` (já deve estar selecionado)
- **Hostname**: `localhost`
- **Port**: `3306`
- **Username**: `root`
- **Password**: Clique em "Store in Vault..." e:
  - Digite sua senha do MySQL root (se tiver)
  - Ou deixe vazio e clique OK (se não tiver senha)
- **Default Schema**: Deixe vazio

#### Passo 2.3: Testar Conexão

1. Clique no botão **"Test Connection"**

2. **Se pedir senha novamente**: Digite e marque "Save password in vault"

3. **Resultado esperado**:
   ```
   ✓ Successfully made the MySQL connection
   
   Information related to this connection:
   Host: localhost
   Port: 3306
   User: root
   SSL: not enabled
   
   A successful MySQL connection was made with
   the parameters defined for this connection.
   
   Server version: 8.0.x
   ```

4. Clique **"OK"** na mensagem de sucesso

5. Clique **"OK"** no formulário de conexão

---

### Etapa 3: Conectar

Na tela inicial, você verá a nova conexão:

```
╔════════════════════════════════════════╗
║ MySQL Connections                 ⊕   ║
╠════════════════════════════════════════╣
║                                        ║
║  ┌──────────────────────────────────┐ ║
║  │  UmbraEternum MySQL80            │ ║
║  │  ├─ root                         │ ║
║  │  └─ localhost:3306               │ ║
║  └──────────────────────────────────┘ ║
║                                        ║
║  ┌──────────────────────────────────┐ ║
║  │  Local instance wampmysqld64     │ ║ ← antiga/ignore
║  │  ├─ root                         │ ║
║  │  └─ localhost:3306               │ ║
║  └──────────────────────────────────┘ ║
╚════════════════════════════════════════╝
```

**Clique** na nova conexão: **"UmbraEternum MySQL80"**

---

### Etapa 4: Verificar Conexão

Após conectar, você deve ver:

**Barra de título**:
```
MySQL Workbench - Local instance MySQL80
```

**Painel esquerdo (Navigator)**:
```
SCHEMAS
├─ sys
├─ mysql
├─ information_schema
└─ performance_schema
```

✅ Se você vê isso, está conectado no MySQL80 correto!

---

### Etapa 5: Executar Script SQL

#### 5.1 Abrir Script

1. **Menu**: File → Open SQL Script... (ou `Ctrl+Shift+O`)

2. **Navegue para**:
   ```
   D:\UmbraServerV2\UmbraServer\setup_database.sql
   ```

3. **Clique**: Open

#### 5.2 Executar

O script aparecerá na aba "Query 1":

```sql
-- Quick setup para desenvolvimento
-- Execute no phpMyAdmin ou MySQL Workbench

CREATE DATABASE IF NOT EXISTS umbra_eternum
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE umbra_eternum;

-- Tabela de contas
CREATE TABLE IF NOT EXISTS accounts (
  ...
```

**Executar**:
- Clique no ícone **⚡** (raio amarelo) na toolbar
- Ou pressione `Ctrl+Shift+Enter`
- Ou Menu: Query → Execute (All or Selection)

#### 5.3 Verificar Output

Na parte inferior (Action Output):

```
Action Output:
17:15:03  CREATE DATABASE IF NOT EXISTS umbra_eternum...  0 row(s) affected
17:15:03  CREATE TABLE IF NOT EXISTS accounts...  0 row(s) affected
17:15:03  CREATE TABLE IF NOT EXISTS players...  0 row(s) affected
17:15:03  CREATE TABLE IF NOT EXISTS schema_version...  0 row(s) affected
17:15:03  INSERT INTO schema_version (version)...  1 row(s) affected
17:15:03  SELECT 'Database setup complete!' as status
          status: Database setup complete!  ✓
```

✅ Se você vê "Database setup complete!", está tudo certo!

---

### Etapa 6: Verificar Banco Criado

#### 6.1 Atualizar Schemas

No painel esquerdo (Navigator):
- Clique no ícone **🔄** (refresh) ao lado de "SCHEMAS"

#### 6.2 Ver Novo Banco

Você deve ver:

```
SCHEMAS
├─ umbra_eternum  ← NOVO!
│  ├─ Tables
│  │  ├─ accounts
│  │  ├─ players
│  │  └─ schema_version
│  ├─ Views
│  ├─ Stored Procedures
│  └─ Functions
├─ sys
├─ mysql
...
```

#### 6.3 Testar Queries

Crie uma nova aba de query (Ctrl+T) e execute:

```sql
-- Verificar banco
SHOW DATABASES LIKE 'umbra_eternum';

-- Usar o banco
USE umbra_eternum;

-- Ver tabelas
SHOW TABLES;

-- Ver estrutura da tabela accounts
DESCRIBE accounts;

-- Ver estrutura da tabela players
DESCRIBE players;

-- Verificar versão
SELECT * FROM schema_version;
```

**Resultado esperado**:
```sql
-- SHOW DATABASES
Database: umbra_eternum

-- SHOW TABLES
Tables_in_umbra_eternum
accounts
players
schema_version

-- SELECT schema_version
version       applied_at
1.3.0        2025-10-14 17:15:03
```

✅ Perfeito! Banco criado com sucesso!

---

## ✅ Checklist Final

- [ ] Nova conexão "UmbraEternum MySQL80" criada
- [ ] Test Connection passou
- [ ] Conectado com sucesso
- [ ] Script setup_database.sql executado
- [ ] "Database setup complete!" exibido
- [ ] Banco "umbra_eternum" aparece nos Schemas
- [ ] Tabelas accounts, players, schema_version existem
- [ ] SELECT * FROM schema_version retorna 1.3.0

---

## 🚀 Próximo Passo: Testar Servidor C++

Agora que o banco está funcionando:

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Esperado**:
```
===========================================
    UmbraEternum Server Stack v1.3.0      
===========================================

[INFO] Starting UmbraEternum Server Stack...
[INFO] Configuration loaded successfully
[INFO] Database connected successfully ✓✓✓
[OK] Auth Server started on port 8080
[OK] World Server started on port 8081
[OK] Gateway Server started on port 9000

===========================================
  All servers running. Press Ctrl+C to stop.
===========================================
```

🎉 Se você vê isso, TUDO ESTÁ FUNCIONANDO!

---

## 🛠️ Troubleshooting

### Erro: "Access denied for user 'root'@'localhost'"

**Causa**: Senha incorreta

**Solução**:
1. Edit Connection (clique com botão direito na conexão)
2. Clique em "Store in Vault" ao lado de Password
3. Digite a senha correta
4. Test Connection novamente

---

### Erro: "Can't connect to MySQL server on 'localhost'"

**Causa**: MySQL80 não está rodando

**Solução**:
```powershell
# Como Administrador
net start MySQL80
```

Ou via Services:
1. Win+R → `services.msc`
2. Encontre "MySQL80"
3. Right-click → Start

---

### Workbench ainda conecta no WAMP

**Causa**: Você clicou na conexão antiga

**Solução**: 
- Use APENAS a conexão "UmbraEternum MySQL80"
- Você pode deletar a conexão antiga:
  - Right-click → Delete Connection

---

### Script dá erro "Table already exists"

**Causa**: Banco já foi criado antes

**Solução**: Isso é OK! O script usa `IF NOT EXISTS`

Ou se quiser recriar:
```sql
DROP DATABASE IF EXISTS umbra_eternum;
-- Depois execute o script novamente
```

---

## 💡 Dicas Pro

### Tornar UmbraEternum MySQL80 a Conexão Padrão

1. Right-click na conexão "UmbraEternum MySQL80"
2. "Set as Default Connection"
3. Ela será aberta automaticamente no próximo startup

### Deletar Conexão Antiga do WAMP

1. Right-click em "Local instance wampmysqld64"
2. "Delete Connection"
3. Confirme
4. Agora você tem apenas uma conexão (a correta!)

### Atalhos Úteis

- `Ctrl+Enter` - Executar query sob o cursor
- `Ctrl+Shift+Enter` - Executar todas as queries
- `Ctrl+T` - Nova aba de query
- `Ctrl+W` - Fechar aba
- `F5` - Refresh schemas

---

## 🎯 Resumo

**O que fizemos**:
1. ✅ Criamos nova conexão MySQL80 (não WAMP)
2. ✅ Executamos setup_database.sql
3. ✅ Banco umbra_eternum criado
4. ✅ Tabelas accounts, players criadas
5. ✅ Pronto para usar!

**O que NÃO usar mais**:
- ❌ Conexão "Local instance wampmysqld64"
- ❌ WAMP (instável e fecha sozinho)
- ❌ phpMyAdmin (tem erro Twig)

**O que usar agora**:
- ✅ Conexão "UmbraEternum MySQL80"
- ✅ MySQL Workbench diretamente
- ✅ Estável e confiável!

---

**Criado**: 2025-10-14  
**Para**: UmbraEternum Server v1.3.0  
**Problema**: WAMP fecha sozinho  
**Solução**: MySQL80 standalone via Workbench

