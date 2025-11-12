# ⚡ **RESUMO RÁPIDO: MySQL Não Inicia Automaticamente**

## 🎯 **PROBLEMA:**
MySQL não conecta após reiniciar o computador (erro 10061).

## ✅ **SOLUÇÃO (2 minutos):**

### **PASSO 1: Configurar Auto-Start**
```
1. Navegue: D:\UmbraServerV2\UmbraServer\scripts_main\
2. Right-click: configure_mysql_autostart.ps1
3. Selecione: "Executar como Administrador"
4. Aguarde: Script configura automaticamente
```

### **PASSO 2: Iniciar MySQL Agora**
```
1. Right-click: start_mysql.bat
2. Selecione: "Executar como Administrador"
3. MySQL inicia imediatamente
```

### **PASSO 3: Verificar**
```
1. Abra MySQL Workbench
2. Conecte em "Local instance MySQL80"
3. Deve conectar com sucesso!
```

---

## 🔄 **ALTERNATIVA MANUAL:**

**Via Services (services.msc):**
1. `Win+R` → `services.msc`
2. Encontre `MySQL80`
3. Right-click → **Properties**
4. **Startup type:** `Automatic`
5. **OK**

---

## 📋 **SCRIPTS DISPONÍVEIS:**

- **`configure_mysql_autostart.ps1`** → Configura auto-start
- **`start_mysql.bat`** → Inicia MySQL agora
- **`start_mysql.ps1`** → Inicia MySQL agora (PowerShell)

---

**✅ Pronto! MySQL iniciará automaticamente após reiniciar!**

