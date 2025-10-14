# ⚡ QUICK FIX - Erro Painel Admin

**Problema**: Erro JSON no painel admin  
**Causa**: Coluna `isadmin` provavelmente não existe no banco

---

## 🚀 SOLUÇÃO RÁPIDA (1 CLIQUE)

### Execute o Setup Automático:

```
http://localhost/umbra_api/setup_admin.php
```

**O que faz**:
1. ✅ Verifica se coluna `isadmin` existe
2. ✅ Cria a coluna se não existir
3. ✅ Cria índice
4. ✅ Torna primeira conta admin
5. ✅ Mostra estrutura da tabela
6. ✅ Lista admins existentes
7. ✅ Mostra estatísticas

**Resultado**: Tudo configurado automaticamente!

---

## 📋 PASSO A PASSO

### 1. Acesse o Setup

```
http://localhost/umbra_api/setup_admin.php
```

### 2. Veja o Resultado

**Se tudo OK**:
- ✅ Conexão OK
- ✅ Coluna criada (ou já existe)
- ✅ Admin configurado
- ✅ Estatísticas mostradas

**Se houver erro**:
- ❌ Mostra o erro específico
- ℹ️ Dica de como resolver

### 3. Ir para Painel Admin

Clique no botão "👑 Ir para Painel Admin" ou:

```
http://localhost/umbra_api/admin.html
```

---

## 🔧 SOLUÇÃO MANUAL (SE PREFERIR)

### Via MySQL Workbench

```sql
USE umbra_eternum;

-- Adicionar coluna
ALTER TABLE accounts 
ADD COLUMN isadmin TINYINT(1) NOT NULL DEFAULT 0 
AFTER banned;

-- Criar índice
CREATE INDEX idx_isadmin ON accounts(isadmin);

-- Tornar primeira conta admin
UPDATE accounts SET isadmin = 1 WHERE id = 1;

-- Verificar
SELECT id, username, email, isadmin FROM accounts;
```

---

## ✅ VERIFICAR SE FUNCIONOU

### Teste 1: Setup

```
http://localhost/umbra_api/setup_admin.php
```

Deve mostrar:
- ✅ Coluna 'isadmin' já existe
- ✅ Lista de admins
- ✅ Estatísticas

### Teste 2: Dashboard

```
1. Acesse: http://localhost/umbra_api/login.html
2. Faça login com sua conta
3. No dashboard, deve aparecer:
   - Badge "👑 ADMIN"
   - Botão "👑 Painel Admin"
```

### Teste 3: Painel Admin

```
1. Clique em "👑 Painel Admin"
2. Deve carregar sem erro
3. Tab "Servidor" deve funcionar
```

---

## 🐛 SE AINDA HOUVER ERRO

### Erro: "Table 'umbra_eternum.accounts' doesn't exist"

**Causa**: Banco não está configurado

**Solução**: Execute o script de setup do banco primeiro:
```
D:\UmbraServerV2\UmbraServer\setup_database.sql
```

### Erro: "Access denied"

**Causa**: Senha do MySQL incorreta

**Solução**: Edite `config/database.php`:
```php
define('DB_PASS', 'SUA_SENHA_AQUI');
```

### Erro: "Column 'isadmin' cannot be null"

**Causa**: Coluna existe mas sem valor padrão

**Solução**: Execute no MySQL:
```sql
ALTER TABLE accounts 
MODIFY isadmin TINYINT(1) NOT NULL DEFAULT 0;

UPDATE accounts SET isadmin = 0 WHERE isadmin IS NULL;
```

---

## 📝 CHECKLIST

- [ ] Acessou `setup_admin.php`
- [ ] Viu mensagem "✅ Setup Completo!"
- [ ] Fez login no dashboard
- [ ] Vê badge "👑 ADMIN"
- [ ] Painel admin carrega sem erro
- [ ] Tab "Servidor" funciona

---

## 🎯 APÓS CORRIGIR

### 1. Limpar Cache

**No navegador**:
- `Ctrl + Shift + R` (hard refresh)
- Ou `Ctrl + F5`

### 2. Testar Novamente

```
http://localhost/umbra_api/admin.html
```

### 3. Deletar Setup (Opcional)

Após usar, pode deletar por segurança:
```
C:\wamp64\www\umbra_api\setup_admin.php
```

---

## 🚀 RESUMO

```
1. Acesse: http://localhost/umbra_api/setup_admin.php
2. Aguarde setup automático
3. Clique em "👑 Ir para Painel Admin"
4. Recarregue se necessário (Ctrl+Shift+R)
5. Pronto! ✅
```

---

**Criado**: 2025-10-14  
**Arquivo**: `setup_admin.php`  
**Tempo**: < 1 minuto  
**Status**: Automático ✅

