# ✅ Como Iniciar o Zone Server Corretamente

## ⚠️ Problema Comum

Se você executar `zone_server.exe` **sem argumentos**, ele usará:
- Zone ID padrão = **1**
- Porta = **8082 + 1 = 8083**

Mas o cliente Unreal está configurado para conectar na porta **8082**.

## ✅ Solução: Sempre Executar com Zone ID 0

### Método 1: Linha de Comando (Direto)

```powershell
# No diretório do projeto
cd build\bin\Release
.\zone_server.exe 0
```

**OU** a partir da raiz do projeto:

```powershell
# Do diretório raiz (D:\UmbraServerV2)
build\bin\Release\zone_server.exe 0
```

### Método 2: Script Batch (Recomendado)

```batch
# Execute o script:
scripts\start_zone_server_8082.bat
```

### Método 3: Script PowerShell (Recomendado)

```powershell
# Execute o script:
.\scripts\start_zone_server_8082.ps1
```

## 📋 Verificação

Após iniciar, verifique:

### 1. Porta 8082 em LISTENING:

```powershell
netstat -ano | findstr :8082
```

**Resultado esperado**:
```
TCP    0.0.0.0:8082           0.0.0.0:0              LISTENING       [PID]
```

### 2. Logs do Servidor:

```powershell
Get-Content build\bin\Release\logs\zone_server.log -Tail 5
```

**Resultado esperado**:
```
[INFO] ZoneServer 'Zone_0' (ID: 0) started on port 8082
[INFO] WebSocketServer started on port 8082
```

**NÃO deve aparecer**:
```
[INFO] ZoneServer 'Zone_1' (ID: 1) started on port 8083  ← ERRADO!
```

## 🔧 Lógica da Porta

O Zone Server calcula a porta assim:
```cpp
config.port = 8082 + config.zoneId;
```

- **Zone ID 0** → Porta **8082** ✅ (cliente conecta aqui)
- **Zone ID 1** → Porta **8083** ❌ (cliente não conecta)
- **Zone ID 2** → Porta **8084**
- etc.

## ⚠️ Se o Servidor Iniciar na Porta Errada

### Parar o Servidor:

```powershell
Get-Process -Name zone_server | Stop-Process -Force
```

### Iniciar Corretamente:

```powershell
build\bin\Release\zone_server.exe 0
```

## ✅ Status Atual

- ✅ Zone Server configurado para porta **8082** (Zone ID 0)
- ✅ Cliente Unreal configurado para **ws://127.0.0.1:8082**
- ✅ Scripts de inicialização disponíveis em `scripts/`

**Para iniciar rapidamente:**
```powershell
.\scripts\start_zone_server_8082.ps1
```

---

**Data**: 2025-11-01
**Status**: ✅ **SERVIDOR RODANDO NA PORTA 8082**

