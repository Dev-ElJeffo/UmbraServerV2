# 🔧 Solução: WebSocket "socket connect failed"

## 🚨 Problema

O cliente Unreal Engine mostra:
- `[BP_NetMovementClient_C_1] socket connect failed`
- Overlay: "socket connect failed"
- WebSocket não conecta

## ✅ Causa

**O Zone Server não está rodando!**

O Zone Server é um servidor C++ separado que **deve ser executado antes** do cliente Unreal.

## 🛠️ Solução Rápida

### Passo 1: Execute o Zone Server

**Opção A - PowerShell (Recomendado):**
```powershell
cd D:\UmbraServerV2\build\bin\Release
.\zone_server.exe 0
```

**Opção B - Script Automático:**
```powershell
.\scripts\start_zone_server_8082.ps1
```

**Opção C - Batch:**
```batch
.\scripts\start_zone_server_8082.bat
```

### Passo 2: Verifique se Está Rodando

**Em um novo terminal:**
```powershell
netstat -ano | findstr :8082
```

**Deve mostrar:**
```
TCP    0.0.0.0:8082           0.0.0.0:0              LISTENING
```

### Passo 3: Logs Esperados

**No terminal do Zone Server, você deve ver:**
```
[INFO] Starting Zone Server...
[INFO] ZoneServer 'Zone_0' (ID: 0) started on port 8082
[INFO] WebSocketServer started on port 8082
```

**Quando o cliente conectar:**
```
[INFO] New WebSocket connection from 127.0.0.1:[PORTA]
[INFO] WS client [ID] connected
```

## 📋 Ordem Correta de Execução

1. ✅ **PRIMEIRO**: Execute `zone_server.exe 0` (porta 8082)
2. ✅ **SEGUNDO**: Inicie o Unreal Engine e faça PIE
3. ✅ **TERCEIRO**: Faça login e selecione personagem
4. ✅ **QUARTO**: O WebSocket deve conectar automaticamente

## ⚠️ Importante

- **Porta 8082**: Execute `zone_server.exe 0`
- **Porta 8083**: Execute `zone_server.exe 1` (e atualize o cliente)
- **Mantenha o servidor rodando** enquanto testar o cliente
- O servidor não fecha sozinho - pressione Ctrl+C para parar

## 📚 Documentação Completa

Para mais detalhes e troubleshooting avançado, veja:
- [`docs_main/CORRECAO_CONEXAO_WEBSOCKET_APOS_SELECTCHARACTER.md`](docs_main/CORRECAO_CONEXAO_WEBSOCKET_APOS_SELECTCHARACTER.md) - Seção TROUBLESHOOTING (linha ~1555)

