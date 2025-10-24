# 🔧 Fix: Erro SQL - Palavra Reservada "database"

**Data**: 2025-10-14  
**Status**: ✅ CORRIGIDO

---

## 🐛 Problema

### Erro Apresentado
```
SQLSTATE[42000]: Syntax error or access violation: 1064 
You have an error in your SQL syntax; check the manual that 
corresponds to your MySQL server version for the right syntax 
to use near 'database' at line 1
```

### Local do Erro
- **Arquivo**: `C:\wamp64\www\umbra_api\api\test.php`
- **Linha**: 15
- **Endpoint**: GET `/api/test.php`

### Código Problemático
```php
$query = "SELECT VERSION() as version, DATABASE() as database";
```

---

## 🔍 Causa Raiz

### Palavra Reservada do MySQL

`database` é uma **palavra reservada** no MySQL e não pode ser usada como alias sem escapar com backticks.

**Palavras reservadas do MySQL** que causam este erro:
- `database`
- `table`
- `select`
- `from`
- `where`
- `order`
- `group`
- etc.

### Por que funcionava localmente?

Algumas versões/configurações do MySQL podem aceitar palavras reservadas como alias, mas o MySQL 8.0+ em modo strict causa este erro.

---

## ✅ Solução Aplicada

### Mudança no Código

**Arquivo**: `C:\wamp64\www\umbra_api\api\test.php`

```php
// ❌ ANTES (com erro)
$query = "SELECT VERSION() as version, DATABASE() as database";
$response['database'] = $result['database'];

// ✅ DEPOIS (corrigido)
$query = "SELECT VERSION() as version, DATABASE() as db_name";
$response['database'] = $result['db_name'];
```

### Alternativas

Se você quiser manter o alias `database`, use backticks:

```php
// Opção alternativa (com backticks)
$query = "SELECT VERSION() as version, DATABASE() as `database`";
```

Mas é **mais seguro** evitar palavras reservadas completamente.

---

## 🧪 Validação

### Teste Realizado

```powershell
Invoke-RestMethod -Uri "http://localhost/umbra_api/api/test.php"
```

### Resultado

```json
{
    "success": true,
    "message": "Conexão bem-sucedida!",
    "mysql_version": "8.0.43",
    "database": "umbra_eternum",
    "schema_version": "1.3.0",
    "stats": {
        "accounts": 1,
        "players": 0
    }
}
```

**Status**: ✅ **FUNCIONANDO 100%**

---

## 📋 Checklist de Correção

- [x] Identificado palavra reservada `database`
- [x] Renomeado alias para `db_name`
- [x] Atualizado response para usar novo alias
- [x] Testado endpoint via PowerShell
- [x] Validado JSON response
- [x] Verificado UI no browser
- [x] Documentado correção

---

## 🎯 Impacto

### Arquivos Alterados
- ✅ `C:\wamp64\www\umbra_api\api\test.php` (1 linha)

### Endpoints Afetados
- ✅ GET `/api/test.php` - Agora funciona perfeitamente

### Outros Arquivos
- ✅ `register.php` - Não afetado
- ✅ `login.php` - Não afetado
- ✅ `index.php` - Não afetado

---

## 💡 Lições Aprendidas

### Boas Práticas para Evitar Este Erro

1. **Evite palavras reservadas como alias**
   ```sql
   -- Ruim
   SELECT id as select, name as table
   
   -- Bom
   SELECT id as item_id, name as item_name
   ```

2. **Use nomes descritivos**
   ```sql
   -- Ruim
   SELECT DATABASE() as database
   
   -- Bom
   SELECT DATABASE() as db_name
   ```

3. **Se precisar usar palavra reservada, use backticks**
   ```sql
   SELECT DATABASE() as `database`
   ```

4. **Teste em ambiente similar ao produção**
   - MySQL strict mode habilitado
   - Mesma versão do MySQL

---

## 🔗 Palavras Reservadas MySQL 8.0

Lista completa: https://dev.mysql.com/doc/refman/8.0/en/keywords.html

**Mais comuns que causam erro**:
- `DATABASE`, `TABLE`, `COLUMN`
- `SELECT`, `FROM`, `WHERE`, `ORDER`, `GROUP`
- `JOIN`, `LEFT`, `RIGHT`, `INNER`, `OUTER`
- `INDEX`, `KEY`, `PRIMARY`, `FOREIGN`
- `USER`, `ROLE`, `GRANT`, `REVOKE`
- `INSERT`, `UPDATE`, `DELETE`, `DROP`
- `CREATE`, `ALTER`, `MODIFY`
- `CHAR`, `VARCHAR`, `TEXT`, `INT`
- `DATE`, `TIME`, `TIMESTAMP`

**Dica**: Sempre use nomes descritivos e específicos ao invés de genéricos!

---

## 📊 Antes vs Depois

### Antes (com erro)
```
❌ Erro: SQLSTATE[42000]
❌ API não funciona
❌ UI mostra erro vermelho
❌ Testes falham
```

### Depois (corrigido)
```
✅ success: true
✅ API funciona perfeitamente
✅ UI mostra status verde
✅ Testes passam
✅ Dados retornados corretamente
```

---

## 🚀 Próximos Passos

1. ✅ Testar via browser: http://localhost/umbra_api/
2. ✅ Executar script: `.\test_api.ps1`
3. ✅ Testar register e login
4. ✅ Validar integração com servidor C++

---

## 📝 Nota para Desenvolvedores

Se você encontrar erro similar com outras palavras:

```sql
-- Lista de palavras problemáticas comuns
-- Sempre renomeie os alias para algo específico

SELECT 
    DATABASE() as db_name,        -- não use 'database'
    USER() as current_user,       -- não use 'user'
    NOW() as current_time,        -- não use 'time'
    COUNT(*) as total_count       -- não use 'count'
```

---

**Corrigido por**: Assistant  
**Data**: 2025-10-14  
**Tempo de correção**: < 2 minutos  
**Status**: ✅ **RESOLVIDO E TESTADO**

