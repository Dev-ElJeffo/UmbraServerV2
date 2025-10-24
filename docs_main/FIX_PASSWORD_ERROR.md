# 🔐 Fix: Access Denied - Senha do MySQL

**Erro visto**: `Access denied for user 'root'@'localhost' (using password: YES)`

---

## ⚡ SOLUÇÃO RÁPIDA (30 segundos)

### Opção A: Tentar Sem Senha (90% dos casos)

**No Workbench (janela "Setup New Connection" ainda aberta)**:

1. Clique no botão **"Clear"** (ao lado de "Store in Vault")
2. Deixe o campo **Password VAZIO**
3. Clique em **"Test Connection"**

**Esperado**: ✅ "Successfully made the MySQL connection"

4. Se funcionar, clique **"OK"** duas vezes

✅ **PRONTO!** Pule para "Próximos Passos" abaixo.

---

## 🔧 Opção B: Resetar Senha (se Opção A não funcionar)

### Método 1: PowerShell (Rápido)

**Execute como Administrador**:

```powershell
# 1. Parar MySQL
Stop-Service MySQL80

# 2. Iniciar em modo seguro (em background)
Start-Process "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" -ArgumentList "--skip-grant-tables" -WindowStyle Hidden

# 3. Aguardar 5 segundos
Start-Sleep -Seconds 5

# 4. Resetar senha para vazio
& "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -e "FLUSH PRIVILEGES; ALTER USER 'root'@'localhost' IDENTIFIED BY '';"

# 5. Parar processo seguro
Get-Process mysqld -ErrorAction SilentlyContinue | Stop-Process -Force

# 6. Reiniciar MySQL normal
Start-Sleep -Seconds 3
Start-Service MySQL80

Write-Host "✓ Senha resetada! Use senha VAZIA no Workbench"
```

### Método 2: Script BAT (Automático)

1. **Execute como Admin**: `reset_mysql_password.bat`
2. Aguarde a mensagem: "SENHA RESETADA! (senha vazia)"
3. Volte ao Workbench

---

## 🎯 Depois de Resetar

**No Workbench**:

1. Se a janela "Setup New Connection" estiver fechada:
   - Clique no [+] novamente
   - Preencha os campos novamente

2. **Password**: 
   - Clique "Clear"
   - Deixe VAZIO (sem senha)

3. **Test Connection**

4. Deve funcionar agora! ✅

---

## 🔒 Opção C: Criar Senha Nova (Recomendado para produção)

Se você QUER ter uma senha:

### Passo 1: Conectar sem senha (Opção A)

### Passo 2: Executar no Workbench

```sql
-- Definir nova senha
ALTER USER 'root'@'localhost' IDENTIFIED BY 'SuaSenhaAqui123!';
FLUSH PRIVILEGES;
```

### Passo 3: Atualizar conexão

1. Edit Connection (botão direito na conexão)
2. "Store in Vault" → Digite: `SuaSenhaAqui123!`
3. Test Connection
4. Save

### Passo 4: Atualizar config/db.json

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "umbra_eternum",
    "user": "root",
    "password": "SuaSenhaAqui123!"
  }
}
```

---

## 🛠️ Troubleshooting Adicional

### Erro persiste mesmo sem senha?

**Tente**: Criar novo usuário dedicado

```sql
-- No Workbench (como root, sem senha)
CREATE USER 'umbra_admin'@'localhost' IDENTIFIED BY 'umbra123';
GRANT ALL PRIVILEGES ON *.* TO 'umbra_admin'@'localhost' WITH GRANT OPTION;
FLUSH PRIVILEGES;
```

**Depois**: Crie nova conexão com:
- Username: `umbra_admin`
- Password: `umbra123`

---

### MySQL não inicia após reset?

```powershell
# Verificar erros
Get-Content "C:\ProgramData\MySQL\MySQL Server 8.0\Data\*.err" -Tail 50

# Forçar reinício
Get-Process mysqld -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3
Start-Service MySQL80
```

---

### Verificar qual senha está configurada

```powershell
# Ver se root precisa de senha
"C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root

# Se conectar SEM pedir senha = não tem senha
# Se pedir senha = tem senha configurada
```

---

## 📋 Checklist de Diagnóstico

Marque o que você tentou:

- [ ] Opção A: Password vazio (Clear)
- [ ] Test Connection passou?
  - [ ] ✅ Sim → Vá para "Próximos Passos"
  - [ ] ❌ Não → Tente Opção B

- [ ] Opção B: Reset password
  - [ ] Script executado como Admin
  - [ ] MySQL80 reiniciado
  - [ ] Voltou ao Workbench com senha vazia
  - [ ] Test Connection passou?
    - [ ] ✅ Sim → Vá para "Próximos Passos"
    - [ ] ❌ Não → Veja Troubleshooting

---

## ✅ Próximos Passos (Depois que Test Connection passar)

### 1. Finalizar Conexão
```
- Clique "OK" no Test Connection
- Clique "OK" no Setup New Connection
```

### 2. Conectar
```
- Clique na nova conexão "UmbraEternum MySQL80"
- Digite senha (ou Enter se vazia)
```

### 3. Executar Script
```
- File → Open SQL Script
- Arquivo: D:\UmbraServerV2\UmbraServer\setup_database.sql
- Execute (⚡)
```

### 4. Verificar
```sql
SHOW DATABASES LIKE 'umbra_eternum';
USE umbra_eternum;
SHOW TABLES;
```

### 5. Testar Servidor
```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

---

## 💡 Por Que Isso Acontece?

### Instalações MySQL podem ter:

1. **Sem senha** (padrão em dev):
   - Instalação rápida
   - WAMP/XAMPP
   - Instalação custom

2. **Com senha**:
   - Instalação típica/completa
   - Produção
   - Senha definida durante instalação

### Recomendação:

- **Dev/Local**: Sem senha (mais fácil)
- **Produção**: Com senha forte (seguro)

Para este projeto:
- ✅ Sem senha está OK (desenvolvimento local)
- ⚠️ Se for produção, configure senha depois

---

## 🎯 Resumo

**Erro**: Access denied (senha incorreta)

**Solução mais comum**: 
1. Clear password
2. Test Connection
3. Funciona! ✅

**Se não funcionar**:
1. Reset password → sem senha
2. Test Connection
3. Funciona! ✅

**Depois**:
- Criar banco de dados
- Executar servidor
- Tudo funcionando! 🎉

---

**Tempo**: 30 segundos - 2 minutos  
**Dificuldade**: ⭐ Fácil  
**Solução**: 90% resolve com password vazio

