# 🔧 RESOLVER ERRO DE INICIALIZAÇÃO DO MYSQL

## 🚨 **ERRO ENCONTRADO**

```
[ERRO] Falha ao iniciar MySQL
```

Este erro pode ter várias causas. Vamos diagnosticar e resolver!

---

## 🔍 **PASSO 1: EXECUTAR DIAGNÓSTICO**

Execute o script de diagnóstico **como Administrador**:

### **Opção 1 - Arquivo .bat (MAIS FÁCIL):**

1. Localize o arquivo: `UmbraServer/fix_mysql_now.bat`
2. **Clique com botão direito** → **Executar como Administrador**
3. Aguarde o diagnóstico completo

### **Opção 2 - PowerShell:**

```powershell
cd D:\UmbraServerV2\UmbraServer
powershell -NoProfile -ExecutionPolicy Bypass -File .\diagnostico_mysql.ps1
```

---

## 📋 **CAUSAS COMUNS E SOLUÇÕES**

### **CAUSA 1: CONFLITO COM WAMP/XAMPP** 🔴

**Sintoma:**
```
[AVISO] WAMP/MySQL detectado rodando!
[AVISO] Porta 3306 JÁ ESTÁ EM USO!
```

**O que está acontecendo:**
- Você tem o WAMP ou XAMPP rodando
- Eles já estão usando a porta 3306
- O MySQL standalone não consegue iniciar na mesma porta

**SOLUÇÃO 1 - Usar o MySQL do WAMP (RECOMENDADO):**

Se você já tem o WAMP funcionando, **use o MySQL dele**:

1. **NÃO pare o WAMP**
2. Configure o jogo para conectar no MySQL do WAMP:
   - Host: `localhost` ou `127.0.0.1`
   - Port: `3306`
   - User: `root`
   - Password: (vazia, geralmente)

3. Importe o banco de dados usando o phpMyAdmin do WAMP:
   - Abra: `http://localhost/phpmyadmin`
   - Clique em "Import"
   - Selecione: `setup_inventory_system.sql`
   - Execute

**SOLUÇÃO 2 - Parar WAMP e usar MySQL standalone:**

Se você quer usar o MySQL instalado separadamente:

1. **Pare o WAMP:**
   - Clique no ícone do WAMP na bandeja do sistema
   - Escolha "Exit"

2. **Execute:**
   ```powershell
   .\start_mysql_service.ps1
   ```

3. O script vai automaticamente parar o WAMP e iniciar o MySQL standalone

---

### **CAUSA 2: SERVIÇO MYSQL NÃO ENCONTRADO** 🔴

**Sintoma:**
```
[AVISO] Nenhum serviço MySQL encontrado!
```

**O que está acontecendo:**
- O MySQL não está instalado
- Ou não foi registrado como serviço do Windows

**SOLUÇÃO:**

1. **Instale o MySQL Server:**
   - Baixe: https://dev.mysql.com/downloads/mysql/
   - Escolha: "MySQL Installer for Windows"
   - Durante a instalação, marque:
     - ✅ MySQL Server
     - ✅ Install as Windows Service

2. **OU use o MySQL do WAMP** (veja Solução 1 acima)

---

### **CAUSA 3: MÚLTIPLAS INSTALAÇÕES MYSQL** 🟡

**Sintoma:**
```
[AVISO] Múltiplas instalações MySQL detectadas!
```

**O que está acontecendo:**
- Você tem várias versões do MySQL instaladas
- Pode haver conflito entre elas

**SOLUÇÃO:**

1. **Escolha UMA instalação para usar**
2. **Desinstale as outras:**
   - Painel de Controle → Programas → Desinstalar
   - Procure por "MySQL"
   - Desinstale as versões que você não quer

3. **Mantenha apenas:**
   - MySQL Server 8.0 ou 8.4 (standalone)
   - **OU** MySQL do WAMP

---

### **CAUSA 4: SERVIÇO MYSQL PARADO** 🟡

**Sintoma:**
```
[OK] Encontrados 1 serviço(s) MySQL:
      Status: Stopped
```

**O que está acontecendo:**
- O MySQL está instalado
- Mas o serviço não está rodando

**SOLUÇÃO:**

Execute o script de inicialização:

```powershell
.\start_mysql_service.ps1
```

Ou inicie manualmente:

1. Aperte `Win + R`
2. Digite: `services.msc`
3. Procure por "MySQL" ou "MySQL80"
4. Clique com botão direito → **Iniciar**

---

### **CAUSA 5: ERRO NO ARQUIVO my.ini** 🟡

**Sintoma:**
- O serviço tenta iniciar mas falha imediatamente
- Logs mostram erro de configuração

**SOLUÇÃO:**

1. **Localize o arquivo `my.ini`:**
   - `C:\ProgramData\MySQL\MySQL Server 8.0\my.ini`
   - ou `C:\Program Files\MySQL\MySQL Server 8.0\my.ini`

2. **Abra como Administrador** (Notepad++ ou VS Code)

3. **Verifique estas linhas:**

```ini
[mysqld]
port=3306
datadir=C:/ProgramData/MySQL/MySQL Server 8.0/Data
```

4. **Se o caminho `datadir` não existir:**
   - Crie a pasta manualmente
   - Ou altere o caminho para um que exista

5. **Salve e tente iniciar novamente**

---

## 🛠️ **SCRIPT RÁPIDO - RESOLVER AUTOMATICAMENTE**

Criamos scripts que tentam resolver automaticamente:

### **Script 1: Diagnóstico Completo**
```powershell
cd D:\UmbraServerV2\UmbraServer
.\diagnostico_mysql.ps1
```

**O que faz:**
- ✅ Verifica serviços MySQL instalados
- ✅ Detecta conflitos de porta
- ✅ Identifica WAMP/XAMPP rodando
- ✅ Mostra processos MySQL
- ✅ Verifica arquivos de configuração
- ✅ Dá recomendações específicas

---

### **Script 2: Iniciar MySQL**
```powershell
cd D:\UmbraServerV2\UmbraServer
.\start_mysql_service.ps1
```

**O que faz:**
- ✅ Para WAMP automaticamente (se detectado)
- ✅ Encontra o serviço MySQL
- ✅ Inicia o serviço
- ✅ Verifica conectividade na porta 3306
- ✅ Testa conexão

---

### **Script 3: Corrigir Agora (Arquivo .bat)**
```batch
fix_mysql_now.bat
```

**Clique com botão direito → Executar como Administrador**

**O que faz:**
- ✅ Verifica privilégios de administrador
- ✅ Executa diagnóstico completo
- ✅ Oferece opções de correção interativas

---

## 📊 **FLUXOGRAMA DE DECISÃO**

```
MySQL não inicia?
    │
    ├─> WAMP rodando?
    │   ├─ SIM → Use MySQL do WAMP
    │   └─ NÃO → Continue
    │
    ├─> Serviço instalado?
    │   ├─ SIM → Tente iniciar o serviço
    │   └─ NÃO → Instale MySQL Server
    │
    ├─> Porta 3306 ocupada?
    │   ├─ SIM → Descubra qual processo está usando
    │   └─ NÃO → Continue
    │
    └─> Verifique logs de erro
        └─> C:\ProgramData\MySQL\...\*.err
```

---

## 🔍 **VERIFICAR LOGS DE ERRO**

Se nada funcionar, **verifique os logs**:

### **Localização dos logs:**

```
C:\ProgramData\MySQL\MySQL Server 8.0\Data\[nome-do-pc].err
C:\ProgramData\MySQL\MySQL Server 8.4\Data\[nome-do-pc].err
```

### **Como abrir:**

1. Abra o Explorador de Arquivos
2. Cole o caminho acima na barra de endereço
3. Ordene por "Data de Modificação" (mais recente primeiro)
4. Abra o arquivo `.err` mais recente com o Bloco de Notas

### **O que procurar:**

- `[ERROR]` - erros críticos
- `Can't start server` - falha ao iniciar
- `bind on TCP/IP port: Address already in use` - porta ocupada
- `unknown variable` - erro no my.ini

---

## ✅ **VERIFICAÇÃO FINAL**

Após resolver, verifique se está funcionando:

### **1. Verificar serviço:**
```powershell
Get-Service | Where-Object { $_.Name -like "*mysql*" }
```

**Resultado esperado:**
```
Status   Name               DisplayName
------   ----               -----------
Running  MySQL80            MySQL80
```

---

### **2. Verificar porta:**
```powershell
Get-NetTCPConnection -LocalPort 3306
```

**Resultado esperado:**
```
LocalAddress  LocalPort  RemoteAddress  RemotePort  State
------------  ---------  -------------  ----------  -----
0.0.0.0       3306       0.0.0.0        0           Listen
```

---

### **3. Testar conexão:**

No MySQL Workbench ou via command line:

```bash
mysql -u root -p -h localhost
```

Se conseguir conectar, **MySQL está funcionando!** ✅

---

## 🚀 **PRÓXIMOS PASSOS**

Quando o MySQL estiver funcionando:

1. **Configure o banco de dados:**
   ```sql
   mysql -u root -p < setup_inventory_system.sql
   ```

2. **Inicie a API PHP:**
   ```powershell
   .\setup_php_api.ps1
   ```

3. **Teste no Unreal Engine:**
   - Abra `BP_UmbraGameInstance`
   - Configure `ItemIconsDataTable`
   - Teste carregar o inventário

---

## 📞 **AINDA COM PROBLEMAS?**

Se nenhuma solução funcionou:

1. **Execute o diagnóstico completo:**
   ```
   .\diagnostico_mysql.ps1
   ```

2. **Copie a saída completa do diagnóstico**

3. **Me envie** para análise detalhada

4. **Ou crie uma issue no GitHub** com:
   - Saída do diagnóstico
   - Logs de erro (`*.err`)
   - Versão do Windows
   - Versão do MySQL

---

## 📝 **RESUMO RÁPIDO**

| Problema | Solução Rápida |
|----------|---------------|
| WAMP rodando | Use MySQL do WAMP ou pare o WAMP |
| Serviço não encontrado | Instale MySQL Server |
| Porta 3306 ocupada | Descubra qual processo está usando |
| Serviço parado | Execute `.\start_mysql_service.ps1` |
| Múltiplas instalações | Desinstale as versões antigas |
| Erro no my.ini | Verifique `datadir` e `port` |

---

**Boa sorte! 🍀**

