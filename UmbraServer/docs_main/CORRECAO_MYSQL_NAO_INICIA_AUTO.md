# 🔧 **CORREÇÃO: MySQL Não Inicia Automaticamente Após Reiniciar**

## 🎯 **PROBLEMA:**

Após reiniciar o computador, o MySQL não está conectando. Erro:
- `ERROR 2003 (HY000): Can't connect to MySQL server on 'localhost:3306' (10061)`
- `Failed to Connect to MySQL at 127.0.0.1:3306 with user root`

**Causa:** O serviço MySQL80 não está configurado para iniciar automaticamente no Windows.

---

## ✅ **SOLUÇÃO RÁPIDA (2 minutos):**

### **OPÇÃO 1: Script Automático (Recomendado)**

1. **Navegue até:** `D:\UmbraServerV2\UmbraServer\scripts_main\`
2. **Clique com botão direito** em `configure_mysql_autostart.ps1`
3. **Selecione:** "Executar como Administrador"
4. **Aguarde:** O script configura o MySQL para iniciar automaticamente

**Pronto!** O MySQL agora iniciará automaticamente ao reiniciar o computador.

---

### **OPÇÃO 2: Manual (Interface Gráfica)**

1. **Pressione `Win+R`**
2. **Digite:** `services.msc`
3. **Pressione Enter**
4. **Encontre:** `MySQL80` na lista
5. **Clique com botão direito** → **Properties**
6. **Na aba "General":**
   - **Startup type:** Selecione `Automatic`
7. **Clique em "OK"**
8. **Se o serviço não estiver rodando:**
   - Clique com botão direito → **Start**

---

### **OPÇÃO 3: PowerShell (Linha de Comando)**

**Abra PowerShell como Administrador:**

```powershell
# Configurar para iniciar automaticamente
Set-Service -Name "MySQL80" -StartupType Automatic

# Iniciar serviço agora
Start-Service -Name "MySQL80"

# Verificar status
Get-Service -Name "MySQL80"
```

---

## 🚀 **INICIAR MYSQL AGORA (Se Não Estiver Rodando):**

### **Script Rápido:**

1. **Navegue até:** `D:\UmbraServerV2\UmbraServer\scripts_main\`
2. **Clique com botão direito** em `start_mysql.bat` ou `start_mysql.ps1`
3. **Selecione:** "Executar como Administrador"

### **Ou Manualmente:**

**Via Interface:**
- `Win+R` → `services.msc` → `MySQL80` → Right-click → **Start**

**Via PowerShell (Admin):**
```powershell
Start-Service -Name "MySQL80"
```

**Via CMD (Admin):**
```cmd
net start MySQL80
```

---

## 🔍 **VERIFICAÇÃO:**

### **1. Verificar se MySQL está rodando:**

**PowerShell:**
```powershell
Get-Service -Name "MySQL80"
```

**Deve mostrar:**
```
Status   Name               DisplayName
------   ----               -----------
Running  MySQL80            MySQL80
```

### **2. Verificar tipo de inicialização:**

**PowerShell:**
```powershell
Get-Service -Name "MySQL80" | Select-Object Name, Status, StartType
```

**Deve mostrar:**
```
Name    Status  StartType
----    ------  ---------
MySQL80 Running Automatic
```

### **3. Testar conexão:**

**MySQL Workbench:**
- Abra MySQL Workbench
- Clique em "Local instance MySQL80"
- Digite a senha (ou Enter se sem senha)
- Deve conectar com sucesso

**Ou via linha de comando:**
```cmd
"C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p
```

---

## 📋 **SCRIPTS DISPONÍVEIS:**

### **1. `start_mysql.bat` / `start_mysql.ps1`**
- **Função:** Inicia o MySQL manualmente
- **Uso:** Quando MySQL não está rodando e você precisa iniciar agora
- **Execute como:** Administrador

### **2. `configure_mysql_autostart.ps1`**
- **Função:** Configura MySQL para iniciar automaticamente
- **Uso:** Após instalar MySQL ou quando ele não inicia automaticamente
- **Execute como:** Administrador

### **3. `start_mysql_and_setup.bat`**
- **Função:** Inicia MySQL e configura banco de dados
- **Uso:** Primeira configuração do projeto
- **Execute como:** Administrador

---

## ⚠️ **PROBLEMAS COMUNS:**

### **Problema 1: "Serviço MySQL80 não encontrado"**

**Solução:**
1. Verifique se MySQL está instalado em: `C:\Program Files\MySQL\MySQL Server 8.0`
2. Se não estiver instalado, instale MySQL 8.0
3. Se estiver instalado mas o serviço não existe, reinstale MySQL

### **Problema 2: "Acesso negado" ao configurar auto-start**

**Solução:**
- Execute o script como **Administrador**
- Right-click → "Executar como Administrador"

### **Problema 3: "MySQL inicia mas não conecta"**

**Solução:**
1. Verifique se a porta 3306 está livre:
   ```powershell
   netstat -ano | findstr :3306
   ```
2. Verifique se há outro MySQL rodando (WAMP, XAMPP, etc.)
3. Pare outros serviços MySQL:
   ```powershell
   Stop-Service -Name "wampmysqld64" -ErrorAction SilentlyContinue
   ```

### **Problema 4: "MySQL inicia mas trava"**

**Solução:**
1. Verifique os logs do MySQL:
   - `C:\ProgramData\MySQL\MySQL Server 8.0\Data\*.err`
2. Verifique se há espaço em disco suficiente
3. Reinicie o serviço:
   ```powershell
   Restart-Service -Name "MySQL80"
   ```

---

## 📝 **CHECKLIST:**

Após configurar, verifique:

- [ ] **MySQL está configurado para iniciar automaticamente:**
  - `Get-Service MySQL80 | Select-Object StartType` → Deve mostrar `Automatic`

- [ ] **MySQL está rodando agora:**
  - `Get-Service MySQL80` → Deve mostrar `Running`

- [ ] **Conexão funciona:**
  - MySQL Workbench conecta com sucesso
  - Ou linha de comando: `mysql -u root -p` funciona

- [ ] **Após reiniciar o computador:**
  - MySQL inicia automaticamente
  - Conexão funciona sem intervenção manual

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Configure auto-start** usando `configure_mysql_autostart.ps1`
2. **Teste a conexão** no MySQL Workbench
3. **Reinicie o computador** para verificar se MySQL inicia automaticamente
4. **Se ainda não funcionar**, siga as verificações de problemas comuns acima

---

## 📚 **REFERÊNCIAS:**

- **Scripts:** `UmbraServer/scripts_main/`
- **Setup MySQL:** `docs_main/QUICK_DATABASE_FIX.md`
- **MySQL Workbench Setup:** `docs_main/MYSQL_WORKBENCH_SETUP.md`

---

**✅ Guia completo para corrigir MySQL não iniciar automaticamente!**

