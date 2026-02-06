# 🔧 CORREÇÃO: Enviar PlayerInfoUpdate quando WebSocket conectar

## ❌ PROBLEMA IDENTIFICADO

**As mensagens tipo 4 (PlayerInfoUpdate) não estão sendo enviadas porque:**
- `SendPlayerInfoUpdate` é chamado em `OnLoadCharacterInfoComplete`
- Mas o WebSocket pode ainda não estar conectado nesse momento
- Quando o WebSocket conecta, a mensagem não é enviada automaticamente

**Resultado:** Nenhuma mensagem tipo 4 é enviada/recebida, apenas mensagens tipo 2 (movimento).

---

## ✅ SOLUÇÃO IMPLEMENTADA

### **1. Nova Função: `TrySendPlayerInfoUpdateOnConnect`**

Foi adicionada uma função no `UmbraGameInstance` que:
- Verifica se o `CharacterInfo` já está carregado
- Se sim, envia `PlayerInfoUpdate` automaticamente quando o WebSocket conectar
- Deve ser chamada no evento `OnWSConnected` do Blueprint

### **2. Logs Detalhados Adicionados**

Foram adicionados logs em `SendPlayerInfoUpdate` para debug:
- Quando a função é chamada
- Se o WebSocketClient foi encontrado
- Se a mensagem foi enviada com sucesso
- Tamanho da mensagem codificada

---

## 🔧 CORREÇÃO NO BLUEPRINT

### **PASSO 1: Conectar TrySendPlayerInfoUpdateOnConnect no OnWSConnected**

**No Blueprint `BP_NetMovementClient2`:**

1. **Localize o evento `OnWSConnected`** (ou crie se não existir)

2. **Adicione os seguintes nós:**

```
[OnWSConnected Event]
    ↓
[Get Game Instance]
    World Context Object: self
    ↓
[Cast to Umbra Game Instance]
    Object: Return Value do Get Game Instance
    ↓
[Try Send Player Info Update On Connect]
    Target: As Umbra Game Instance (do Cast)
```

---

### **ESTRUTURA COMPLETA:**

```
[Event: OnWSConnected]
    ↓
[Get Game Instance]
    World Context Object: self (ou Get Self)
    Return Value: Game Instance
    ↓
[Cast to Umbra Game Instance]
    Object: Return Value (do Get Game Instance)
    As Umbra Game Instance: (output)
    ↓
[Try Send Player Info Update On Connect]
    Target: As Umbra Game Instance (do Cast)
```

---

## ✅ RESULTADO ESPERADO

Após essa correção:

1. **Quando o WebSocket conectar:**
   - Se `CharacterInfo` já estiver carregado → `PlayerInfoUpdate` será enviado automaticamente
   - Logs mostrarão: `[UmbraGameInstance] 🔄 TrySendPlayerInfoUpdateOnConnect: WebSocket conectou, enviando PlayerInfoUpdate...`
   - Logs mostrarão: `[UmbraGameInstance] ✅ PlayerInfoUpdate ENVIADO via WebSocket`

2. **Quando o CharacterInfo for carregado:**
   - Se o WebSocket já estiver conectado → `PlayerInfoUpdate` será enviado imediatamente
   - Se o WebSocket ainda não estiver conectado → Será enviado quando conectar (via `TrySendPlayerInfoUpdateOnConnect`)

3. **No servidor:**
   - Logs mostrarão: `Received PlayerInfoUpdate from client X: playerId=Y, name=Z, title=W`
   - Logs mostrarão: `Broadcasted PlayerInfoUpdate for player Y`

4. **Nos outros clients:**
   - Receberão mensagem tipo 4 via WebSocket
   - Logs mostrarão: `[NetMovementClient] ✅ Mensagem tipo 4 (PlayerInfoUpdate) recebida!`
   - `UpdateRemotePlayerNameplate` será chamado automaticamente

---

## 🐛 VERIFICAÇÃO

Após implementar, verifique os logs:

### **Logs do Cliente (Unreal):**
```
[UmbraGameInstance] 🔄 TrySendPlayerInfoUpdateOnConnect: WebSocket conectou, enviando PlayerInfoUpdate...
[UmbraGameInstance] ✅ PlayerInfoUpdate ENVIADO via WebSocket: PlayerID X, Nome: Y, Título: Z
```

### **Logs do Servidor (C++):**
```
Received PlayerInfoUpdate from client X: playerId=Y, name=Z, title=W
Broadcasted PlayerInfoUpdate for player Y (name=Z, title=W)
```

### **Logs do Cliente Recebendo:**
```
[NetMovementClient] ✅ Mensagem tipo 4 (PlayerInfoUpdate) recebida! Processando no C++...
[ParsePlayerInfoUpdate] Parseado: PlayerID=Y, Name='Z', Title='W'
[UmbraGameInstance] 📝 Atualizando nameplate: PlayerID Y, Nome: Z, Título: W
```

---

## ⚠️ IMPORTANTE

- **Certifique-se de que o evento `OnWSConnected` está conectado corretamente no Blueprint**
- **Se o CharacterInfo ainda não estiver carregado quando o WebSocket conectar, a mensagem será enviada quando o CharacterInfo for carregado**
- **Os logs detalhados ajudarão a identificar exatamente onde está falhando**

---

**Após essa correção, as mensagens tipo 4 devem ser enviadas e recebidas corretamente!**
