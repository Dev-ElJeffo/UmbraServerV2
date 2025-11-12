# 📋 **RESUMO: Opções para Desconectar WebSocket (Teste)**

## 🎯 **OBJETIVO:**

Adicionar uma forma de desconectar o WebSocket manualmente para testar se os actors remotos são destruídos corretamente.

---

## 🚀 **OPÇÕES DISPONÍVEIS:**

### **OPÇÃO 1: Widget HUD com Botão (Mais Visual) ⭐ RECOMENDADO**

**Arquivo:** `GUIA_RAPIDO_BOTAO_DESCONECTAR.md`

**Vantagens:**
- ✅ Visual e fácil de usar
- ✅ Não precisa configurar Input Actions
- ✅ Pode ser escondido/removido facilmente

**Desvantagens:**
- ❌ Requer criar um Widget Blueprint
- ❌ Requer adicionar ao Viewport

**Tempo de Implementação:** ~10 minutos

**Passos:**
1. Criar Widget `WBP_DisconnectButton`
2. Adicionar botão "🔴 DESCONECTAR"
3. Conectar `OnClicked` → `Close()` no `WebSocketRef`
4. Adicionar Widget ao Viewport no `BeginPlay`

---

### **OPÇÃO 2: Tecla de Atalho (Mais Rápido)**

**Arquivo:** `GUIA_SIMPLES_TECLA_DESCONECTAR.md`

**Vantagens:**
- ✅ Mais rápido de implementar
- ✅ Não ocupa espaço na tela
- ✅ Funciona durante o gameplay

**Desvantagens:**
- ❌ Requer configurar Input Action
- ❌ Menos visual (precisa saber qual tecla)

**Tempo de Implementação:** ~5 minutos

**Passos:**
1. Criar função `DisconnectWebSocket` no `BP_NetMovementClient`
2. Configurar Input Action (ex: `F9`)
3. Conectar Input Action → Função

---

### **OPÇÃO 3: Console Command (Mais Técnico)**

**Arquivo:** `GUIA_ADICIONAR_BOTAO_DESCONECTAR_WEBSOCKET.md` (Seção OPÇÃO 3)

**Vantagens:**
- ✅ Não requer modificações em Blueprint
- ✅ Funciona via console

**Desvantagens:**
- ❌ Requer código C++
- ❌ Menos intuitivo (precisa abrir console)

**Tempo de Implementação:** ~15 minutos (incluindo compilação)

**Passos:**
1. Adicionar função `DisconnectWebSocket()` em `UmbraGameInstance.h`
2. Implementar função em `UmbraGameInstance.cpp`
3. Compilar projeto
4. Usar no console: `DisconnectWebSocket`

---

## 🎯 **RECOMENDAÇÃO:**

### **Para Teste Rápido:**
→ Use **OPÇÃO 2 (Tecla de Atalho)**

### **Para Teste Visual:**
→ Use **OPÇÃO 1 (Widget com Botão)**

### **Para Desenvolvimento:**
→ Use **OPÇÃO 3 (Console Command)**

---

## 📋 **CHECKLIST COMUM (Todas as Opções):**

Após implementar qualquer opção, verifique:

- [ ] **WebSocket desconecta corretamente:**
  - Logs mostram: `"🔴 Desconectando WebSocket..."`
  - Logs mostram: `"✅ WebSocket fechado!"`

- [ ] **OnWSClosed dispara:**
  - Logs mostram: `"[OnWSClosed] WebSocket fechado, limpando remote actors..."`

- [ ] **CleanupRemoteActors é chamado:**
  - Logs mostram: `"[OnWSClosed] CleanupRemoteActors executado!"`

- [ ] **Servidor envia PlayerDisconnected:**
  - Logs do servidor mostram: `"Broadcasted PlayerDisconnected message for player X"`

- [ ] **Outros clientes recebem mensagem:**
  - Logs mostram: `"Received binary message, size:5"`
  - Logs mostram: `"PlayerDisconnected processado"`

- [ ] **Actors remotos são destruídos:**
  - Visualmente, o actor desaparece na tela
  - Logs mostram: `"RemoveRemoteActor executado para PlayerID: X"`

---

## 🔍 **SE OS ACTORS NÃO ESTÃO SENDO DESTRUÍDOS:**

### **Verificação 1: OnWSClosed está conectado?**

**No `BP_NetMovementClient`, verifique:**

```
[WebSocketRef] → OnClosed
  ↓
[Custom Event: OnWSClosed]
  ↓
[CleanupRemoteActors]
```

### **Verificação 2: Servidor está enviando PlayerDisconnected?**

**Verifique os logs do servidor C++:**
- Deve aparecer: `"Client X disconnected"`
- Deve aparecer: `"Broadcasted PlayerDisconnected message for player X"`

### **Verificação 3: OnWSBinaryMessage está processando 5 bytes?**

**No `BP_NetMovementClient`, Event Graph, `OnWSBinaryMessage`:**

```
[OnWSBinaryMessage] → Data
  ↓
[Get Array Length] (do Data)
  ↓
[Branch: Length == 5?]
  ├─ True:
  │   ├─ [Get Array Item] (Data[0])
  │   ├─ [Branch: Data[0] == 3?] (PlayerDisconnected)
  │   │   ├─ True:
  │   │   │   ├─ [ParsePlayerDisconnected]
  │   │   │   └─ [RemoveRemoteActor]
  │   │   └─ False:
  │   │       └─ (mensagem desconhecida)
  └─ False:
      └─ [ProcessBinaryBuffer] (mensagem normal)
```

### **Verificação 4: RemoveRemoteActor está funcionando?**

**No `BP_NetMovementClient`, Function `RemoveRemoteActor`:**

```
[Function Entry: RemoveRemoteActor] → PlayerId
  ↓
[Array_Find] (RemoteActorIds, ItemToFind: PlayerId)
  ↓
[Branch: FoundIndex >= 0?]
  ├─ True:
  │   ├─ [Get Array Item] (RemoteActors[FoundIndex])
  │   ├─ [Branch: Is Valid (Actor)?]
  │   │   ├─ True:
  │   │   │   ├─ [Destroy Actor]
  │   │   │   ├─ [Array_Remove] (RemoteActors, Index: FoundIndex)
  │   │   │   └─ [Array_Remove] (RemoteActorIds, Index: FoundIndex)
  │   │   └─ False:
  │   │       └─ (actor já destruído)
  └─ False:
      └─ (player não encontrado)
```

---

## 📚 **ARQUIVOS DE REFERÊNCIA:**

- **Guia Completo:** `GUIA_ADICIONAR_BOTAO_DESCONECTAR_WEBSOCKET.md`
- **Guia Rápido (Widget):** `GUIA_RAPIDO_BOTAO_DESCONECTAR.md`
- **Guia Simples (Tecla):** `GUIA_SIMPLES_TECLA_DESCONECTAR.md`
- **Implementação Servidor:** `IMPLEMENTACAO_SERVIDOR_NOTIFICAR_DESCONEXAO.md`
- **Correção OnWSBinaryMessage:** `CORRECAO_ONWSBINARYMESSAGE_5BYTES.md`
- **Correção RemoveRemoteActor:** `CORRECAO_REMOVEREMOTEACTOR_ISVALID.md`

---

## ✅ **PRÓXIMOS PASSOS:**

1. **Escolha uma opção** (recomendado: OPÇÃO 1 ou 2)
2. **Siga o guia correspondente**
3. **Teste a desconexão**
4. **Verifique se os actors são destruídos**
5. **Se não funcionar, siga as verificações acima**

---

**✅ Resumo completo das opções para desconectar WebSocket!**


