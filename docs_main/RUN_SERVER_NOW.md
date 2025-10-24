# 🚀 Executar Servidor AGORA!

**Status**: ✅ Configurações prontas  
**Banco de dados**: ✅ Funcionando  
**Próximo**: Iniciar o servidor

---

## ⚡ EXECUTE AGORA (2 métodos)

### 🎯 Método 1: Script Automático (RECOMENDADO)

**No terminal do VS Code (PowerShell)**:

```powershell
cd D:\UmbraServerV2\UmbraServer
.\run_server.ps1
```

O script vai:
- ✅ Verificar configurações
- ✅ Criar diretórios necessários
- ✅ Copiar configs se necessário
- ✅ Verificar MySQL
- ✅ Iniciar servidor

---

### 🔧 Método 2: Manual (Controle Total)

**Terminal 1 - Servidor**:
```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

**Terminal 2 - Logs (novo terminal)**:
```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
Get-Content logs\umbra_server.log -Wait -Tail 20
```

---

## ✅ Saída Esperada

### Se Tudo Estiver OK:

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

🎉 **SUCESSO!** Todos os servidores rodando!

---

## 🔴 Possíveis Erros e Soluções

### Erro: "Failed to load configuration"

**Causa**: Arquivos config não encontrados

**Solução**:
```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\run_server.ps1  # Usa o script que copia automaticamente
```

---

### Erro: "Failed to connect to database"

**Causa**: MySQL não está rodando

**Solução**:
```powershell
# Como Administrador
Start-Service MySQL80

# Verificar
Get-Service MySQL80
```

---

### Erro: "Port already in use"

**Causa**: Outra instância já rodando

**Solução**:
```powershell
# Ver processos usando portas
netstat -ano | findstr ":8080 :8081 :9000"

# Matar processo específico (substitua PID)
taskkill /F /PID <PID>
```

---

### Erro: "Access denied for database"

**Causa**: Senha incorreta em config/db.json

**Solução**:
```powershell
# Editar senha
notepad D:\UmbraServerV2\UmbraServer\build\bin\Release\config\db.json

# Verificar:
{
  "database": {
    "password": ""  ← Deve estar vazio (ou senha correta)
  }
}
```

---

## 🧪 Testar o Servidor

### 1. Verificar Portas Abertas

```powershell
netstat -ano | findstr "LISTENING" | findstr ":8080 :8081 :9000"
```

**Esperado**:
```
TCP    0.0.0.0:8080    LISTENING    <PID>
TCP    0.0.0.0:8081    LISTENING    <PID>
TCP    0.0.0.0:9000    LISTENING    <PID>
```

---

### 2. Ver Logs em Tempo Real

```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
Get-Content logs\umbra_server.log -Wait
```

Pressione `Ctrl+C` para sair

---

### 3. Testar Auth Server (Quando implementado)

```powershell
# Health check
curl http://localhost:8080/health

# Ou no navegador
start http://localhost:8080
```

---

## 📊 Estrutura de Diretórios (Pronta)

```
D:\UmbraServerV2\UmbraServer\build\bin\Release\
├── umbra_server.exe          ✅ Executável principal
├── auth_server.exe           ✅ Servidor de auth
├── world_server.exe          ✅ Servidor de mundo
├── zone_server.exe           ✅ Servidor de zona
├── chat_server.exe           ✅ Servidor de chat
├── gateway_server.exe        ✅ Gateway
├── config\
│   ├── server.json           ✅ Configurações
│   ├── db.json              ✅ Database config
│   └── jwt_secret.key       ✅ JWT secret
└── logs\
    └── umbra_server.log     ✅ Logs do servidor
```

---

## 🎯 Checklist Final

Antes de executar, verifique:

- [ ] MySQL80 rodando (`Get-Service MySQL80`)
- [ ] Banco `umbra_eternum` criado (via Workbench)
- [ ] Arquivos em `build\bin\Release\config\` existem
- [ ] Diretório `build\bin\Release\logs\` existe
- [ ] Nenhuma porta em uso (8080, 8081, 9000)

**Tudo OK?** Execute o servidor! ⬇️

---

## 🚀 COMANDOS FINAIS

### Opção A: Script PowerShell
```powershell
cd D:\UmbraServerV2\UmbraServer
.\run_server.ps1
```

### Opção B: Manual
```powershell
cd D:\UmbraServerV2\UmbraServer\build\bin\Release
.\umbra_server.exe
```

### Opção C: Batch File
```cmd
cd /d D:\UmbraServerV2\UmbraServer
run_server.bat
```

---

## 📝 Próximos Passos (Depois do Servidor Rodando)

1. ✅ Testar conexão do Auth Server
2. ✅ Criar conta de teste no banco
3. ✅ Testar autenticação
4. ✅ Iniciar integração com UE5
5. ✅ Implementar features (#1, #2, #3, #4)

---

## 💡 Dicas

### Manter Servidor Rodando

Para deixar rodando em background:
```powershell
# Iniciar em nova janela
Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd D:\UmbraServerV2\UmbraServer\build\bin\Release; .\umbra_server.exe"
```

### Ver Processos Rodando

```powershell
Get-Process umbra_server,auth_server,world_server,zone_server,chat_server,gateway_server -ErrorAction SilentlyContinue
```

### Parar Todos os Servidores

```powershell
Get-Process umbra_server,auth_server,world_server,zone_server,chat_server,gateway_server -ErrorAction SilentlyContinue | Stop-Process -Force
```

---

## 🎉 Quando Ver Isso:

```
[OK] Auth Server started on port 8080
[OK] World Server started on port 8081
[OK] Gateway Server started on port 9000
```

**VOCÊ CONSEGUIU!** 🎊

O servidor está FUNCIONANDO! Todos os componentes estão conectados:
- ✅ MySQL database
- ✅ Configurações carregadas
- ✅ Servidores iniciados
- ✅ Portas abertas
- ✅ Sistema operacional!

---

**Tempo**: 30 segundos  
**Dificuldade**: ⭐ Fácil  
**Status**: 🟢 Pronto para executar

👉 **EXECUTE AGORA**: `.\run_server.ps1`

