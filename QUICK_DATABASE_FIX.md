# ⚡ QUICK FIX - Banco de Dados (3 minutos)

**Problema**: WAMP fecha sozinho + phpMyAdmin com erro Twig  
**Solução**: Usar MySQL 8.0 standalone que você já tem instalado

---

## 🎯 MÉTODO RÁPIDO (RECOMENDADO)

### Você JÁ usava MySQL Workbench? Use este método! 👇

---

## 📋 Passo a Passo (3 minutos)

### 1️⃣ Iniciar MySQL (30 segundos)

**Opção A: Via Interface**
```
1. Pressione Win+R
2. Digite: services.msc
3. Encontre: MySQL80
4. Right-click → Start
```

**Opção B: Como Administrador no PowerShell**
```powershell
Start-Service MySQL80
```

---

### 2️⃣ Abrir MySQL Workbench (30 segundos)

```
1. Abra MySQL Workbench
2. Clique em "Local instance MySQL80"
3. Digite senha (ou Enter se sem senha)
```

---

### 3️⃣ Executar Script SQL (1 minuto)

```
1. Menu: File → Open SQL Script
2. Navegue: D:\UmbraServerV2\UmbraServer\setup_database.sql
3. Clique: Execute (ícone ⚡) ou Ctrl+Shift+Enter
4. Aguarde: "Database setup complete!"
```

---

### 4️⃣ Verificar (30 segundos)

Cole e execute no Workbench:

```sql
SHOW DATABASES LIKE 'umbra_eternum';
USE umbra_eternum;
SHOW TABLES;
SELECT * FROM schema_version;
```

**Deve mostrar**:
```
✓ umbra_eternum database
✓ accounts table
✓ players table  
✓ schema_version: 1.3.0
```

---

### 5️⃣ Testar Servidor (30 segundos)

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Esperado**:
```
✓ Database connected successfully
✓ Auth Server started on port 8080
✓ World Server started on port 8081
✓ Gateway Server started on port 9000

All servers running!
```

---

## 🎉 PRONTO!

Seu banco está funcionando! 🚀

---

## 🔄 Alternativas

### Prefere Script Automático?

**Execute como Admin**:
```
Right-click: start_mysql_and_setup.bat
→ Executar como Administrador
```

### Quer Detalhes?

Veja guias completos:
- **Workbench**: `docs/MYSQL_WORKBENCH_SETUP.md`
- **Comparação**: `DATABASE_SETUP_CHOICE.md`

---

## 🛠️ Troubleshooting Rápido

### MySQL não inicia?
```powershell
# Como Admin
net start MySQL80
```

### Senha incorreta?
```
Atualize config/db.json:
"password": "sua_senha_aqui"
```

### Ainda com problemas?
```
Veja: docs/MYSQL_WORKBENCH_SETUP.md
Seção: Troubleshooting (página 15)
```

---

## ✅ Checklist

- [ ] MySQL80 rodando
- [ ] Workbench conectado
- [ ] Script executado
- [ ] Banco verificado
- [ ] Servidor testado
- [ ] Sem erros!

---

**Tempo total**: ~3 minutos  
**Dificuldade**: ⭐ Fácil  
**Status**: ✅ Testado e funcionando

---

**PRÓXIMO**: Execute o servidor e comece a desenvolver! 🎮

