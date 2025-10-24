# 🗄️ Database Setup - Escolha o Melhor Método

**Problema com WAMP?** Sem problemas! Temos 3 métodos alternativos.

---

## 🎯 Qual Método Usar?

| Método | Dificuldade | Tempo | Quando Usar |
|--------|-------------|-------|-------------|
| **🔧 MySQL Workbench** | ⭐ Fácil | 3 min | ✅ Recomendado se já usa Workbench |
| **📝 Script Automático** | ⭐⭐ Médio | 2 min | Se prefere linha de comando |
| **🔵 WAMP/phpMyAdmin** | ⭐ Fácil | 5 min | Se WAMP estiver funcionando |

---

## 🔧 Método 1: MySQL Workbench (RECOMENDADO)

### ✅ Vantagens
- Interface gráfica amigável
- Você já conhece
- Sem problemas do WAMP
- Controle total

### 📋 Passo a Passo

1. **Iniciar MySQL Server**
   ```
   Win+R → services.msc → MySQL80 → Start
   ```

2. **Abrir MySQL Workbench**
   - Conectar à "Local instance MySQL80"
   - Digite senha do root

3. **Executar Script**
   - File → Open SQL Script
   - Selecione: `D:\UmbraServerV2\UmbraServer\setup_database.sql`
   - Clique Execute (⚡)

4. **Verificar**
   ```sql
   SHOW DATABASES LIKE 'umbra_eternum';
   USE umbra_eternum;
   SHOW TABLES;
   ```

**Guia completo**: [`docs/MYSQL_WORKBENCH_SETUP.md`](docs/MYSQL_WORKBENCH_SETUP.md)

---

## 📝 Método 2: Script Automático

### ✅ Vantagens
- Automático
- Inicia MySQL automaticamente
- Para WAMP se necessário

### 📋 Passo a Passo

1. **Executar Script BAT** (Como Administrador)
   ```
   Right-click: start_mysql_and_setup.bat
   → Executar como Administrador
   ```

2. **Digite senha do MySQL** quando solicitado
   (Se não tiver senha, apenas Enter)

3. **Pronto!** O script faz tudo automaticamente:
   - Inicia MySQL80
   - Para WAMP
   - Cria banco de dados
   - Verifica instalação

**Guia completo**: [`setup_mysql.ps1`](setup_mysql.ps1) (PowerShell avançado)

---

## 🔵 Método 3: WAMP/phpMyAdmin (SE FUNCIONAR)

### ⚠️ Problema Atual
```
Twig\Error\RuntimeError: Failed to write cache file
```

### 🔧 Solução do Problema

1. **Dar permissões de escrita**
   ```
   Right-click: C:\wamp64\apps\phpmyadmin5.2.1\tmp
   → Properties → Security → Edit
   → Add → Everyone → Full Control
   ```

2. **Ou limpar cache**
   ```
   Deletar: C:\wamp64\apps\phpmyadmin5.2.1\tmp\twig\*
   ```

3. **Reiniciar WAMP**

**Guia completo**: [`docs/DATABASE_SETUP.md`](docs/DATABASE_SETUP.md)

---

## 🚀 Após Configurar o Banco

### Testar Conexão

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Esperado**:
```
[INFO] Database connected successfully ✓
[OK] Auth Server started on port 8080
[OK] World Server started on port 8081
[OK] Gateway Server started on port 9000
```

### Executar Testes

```powershell
cd D:\UmbraServerV2\UmbraServer\build
ctest -C Release -R DatabaseTests -V
```

**Esperado**: Testes passam sem timeout ✅

---

## 🛠️ Troubleshooting Geral

### MySQL não inicia
```powershell
# Como Administrador
net start MySQL80
```

### Erro "Access denied"
- Verifique senha em `config/db.json`
- Resete senha do MySQL se necessário

### Banco não foi criado
- Re-execute o script `setup_database.sql`
- Verifique erros no output

---

## 📊 Comparação Detalhada

| Aspecto | Workbench | Script | WAMP |
|---------|-----------|--------|------|
| **Requer Admin** | Sim (iniciar serviço) | Sim | Não |
| **Interface** | GUI | CLI | Web |
| **Depuração** | Excelente | Boa | Boa |
| **Backup/Restore** | Integrado | Manual | Manual |
| **Queries ad-hoc** | Sim | Não | Sim |
| **Funciona agora** | ✅ Sim | ✅ Sim | ❌ Não |

---

## 💡 Recomendação Final

### Para Você (Já usava Workbench)

**👉 Use Método 1: MySQL Workbench**

Motivos:
- ✅ Você já conhece
- ✅ Mais poderoso
- ✅ Melhor para desenvolvimento
- ✅ Não depende do WAMP
- ✅ Melhor debug

---

## 📁 Arquivos Criados

```
UmbraServer/
├── setup_database.sql              # Script SQL principal
├── start_mysql_and_setup.bat       # Script BAT automático
├── setup_mysql.ps1                 # Script PowerShell avançado
├── docs/
│   ├── MYSQL_WORKBENCH_SETUP.md   # Guia Workbench detalhado
│   └── DATABASE_SETUP.md           # Guia WAMP/phpMyAdmin
└── DATABASE_SETUP_CHOICE.md        # Este arquivo
```

---

## ⏭️ Próximos Passos

1. **Escolha um método** (Recomendado: Workbench)
2. **Execute o setup** (~3 minutos)
3. **Verifique o banco** (SHOW TABLES)
4. **Inicie o servidor** (umbra_server.exe)
5. **Veja os logs** (logs/umbra_server.log)

---

## 🎯 Objetivo

Ter o banco `umbra_eternum` funcionando com:
- ✅ Tabela `accounts`
- ✅ Tabela `players`
- ✅ Tabela `schema_version`
- ✅ Servidor C++ conectando sem erros

---

## 📞 Suporte

**Se tiver problemas**:
1. Veja o guia específico do método escolhido
2. Verifique a seção Troubleshooting
3. Confira os logs: `logs/umbra_server.log`

---

**Escolha o método que preferir e comece! 🚀**

---

**Atualizado**: 2025-10-14  
**Versão**: 1.0  
**Status**: ✅ Pronto para uso

