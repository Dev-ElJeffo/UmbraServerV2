# 🔧 **CORREÇÃO: Blueprint Novo Não Conecta WebSocket**

## 🎯 **PROBLEMA:**

**Situação:**
- ✅ Novo `BP_NetMovementClient` (baseado em C++) está no level
- ✅ Referência no `BP_ThirdPersonCharacter` aponta para o novo Blueprint
- ❌ **WebSocket NÃO conecta**

**Mas quando:**
- ✅ Novo Blueprint está no level
- ✅ Referência aponta para o **antigo** Blueprint
- ✅ **Actors são spawnados corretamente**

**Isso indica que o `BeginPlay` do C++ está executando antes do personagem estar selecionado!**

---

## 🔍 **CAUSA:**

O `BeginPlay` do C++ verifica `HasActiveCharacter()` e retorna se não houver personagem ativo. Mas quando o novo Blueprint é usado, o `BeginPlay` pode estar executando **antes** do personagem estar totalmente inicializado.

---

## ✅ **SOLUÇÃO: Chamar `ConnectWebSocketManual` do Blueprint**

**Adicionei uma função `ConnectWebSocketManual()` que pode ser chamada do Blueprint quando o personagem estiver pronto!**

---

## 📋 **PASSO 1: Compilar o Projeto**

1. **Compile** o projeto no Visual Studio
2. **Aguarde** a compilação terminar

---

## 📋 **PASSO 2: Implementar `OnWSConnected` no Blueprint**

**No novo `BP_NetMovementClient`, implemente o evento `OnWSConnected`:**

### **2.1: Adicionar Evento `OnWSConnected`**

1. **Event Graph** → **Botão direito** → Procure por **`OnWSConnected`**
2. **Selecione:** `OnWSConnected` (deve aparecer como evento implementável)

### **2.2: Chamar `ConnectWebSocketManual`**

**No evento `OnWSConnected` (ou crie um Custom Event que seja chamado quando o personagem estiver pronto):**

```
[OnWSConnected] (Event) - OU [Custom Event: ConnectWhenReady]
  ↓
[Print String: "[BP_NEW] OnWSConnected - Tentando conectar WebSocket..."]
  ↓
[Call Function: ConnectWebSocketManual] ← FUNÇÃO C++
  ├─ Target: Self (BP_NetMovementClient)
  ↓
[Print String: "[BP_NEW] ConnectWebSocketManual chamado!"]
```

**OU, se o personagem já estiver pronto no BeginPlay do Blueprint:**

```
[Event BeginPlay] (do Blueprint - sobrescreve o C++)
  ↓
[Parent: BeginPlay] ← CHAMA O BEGINPLAY DO C++ PRIMEIRO
  ↓
[Delay: 1.0] ← Aguardar personagem estar pronto
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual] ← FUNÇÃO C++
  │    ↓
  │  [Print String: "[BP_NEW] WebSocket conectado manualmente!"]
  └─ False:
       ↓
     [Print String: "[BP_NEW] Personagem ainda não está pronto, aguardando..."]
     ↓
     [Delay: 1.0] ← Retry
     ↓
     [Call Function: ConnectWebSocketManual] ← Tentar novamente
```

---

## 📋 **PASSO 3: Verificar Logs**

**Execute o jogo e verifique os logs:**

**Você DEVE ver:**
```
[NetMovementClient] BeginPlay INICIADO!
[NetMovementClient] ✅ GameInstance encontrado!
[NetMovementClient] NetMode: X
[NetMovementClient] ✅ Executando no Client!
[NetMovementClient] HasActiveCharacter: 0 ou 1
```

**Se `HasActiveCharacter: 0`, o problema é que o personagem não está pronto ainda!**

**Solução:** Chame `ConnectWebSocketManual` do Blueprint quando o personagem estiver pronto!

---

## 📋 **PASSO 4: Implementar Retry no Blueprint**

**No novo `BP_NetMovementClient`, adicione um Custom Event que tenta conectar:**

```
[Custom Event: TryConnectWebSocket]
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual]
  │    ↓
  │  [Print String: "[BP_NEW] ✅ WebSocket conectado!"]
  └─ False:
       ↓
     [Print String: "[BP_NEW] ⚠️ Personagem não está pronto, tentando novamente em 1s..."]
     ↓
     [Delay: 1.0]
     ↓
     [Call Function: TryConnectWebSocket] ← RECURSIVO (retry)
```

**E chame esse evento no `BeginPlay` do Blueprint:**

```
[Event BeginPlay] (do Blueprint)
  ↓
[Parent: BeginPlay] ← CHAMA O C++ PRIMEIRO
  ↓
[Delay: 0.5] ← Aguardar inicialização
  ↓
[Call Function: TryConnectWebSocket] ← TENTAR CONECTAR
```

---

## 🎯 **SOLUÇÃO ALTERNATIVA: Sobrescrever BeginPlay no Blueprint**

**Se preferir, você pode sobrescrever o `BeginPlay` no Blueprint:**

1. **Event Graph** → **Botão direito** → **Event BeginPlay**
2. **Conecte** ao `Parent: BeginPlay` (chama o C++ primeiro)
3. **Depois**, adicione a lógica de retry:

```
[Event BeginPlay] (do Blueprint)
  ↓
[Parent: BeginPlay] ← CHAMA O C++ PRIMEIRO
  ↓
[Delay: 1.0] ← Aguardar personagem estar pronto
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual]
  └─ False:
       ↓
     [Print String: "[BP_NEW] Personagem não está pronto, aguardando..."]
     ↓
     [Delay: 2.0] ← Aguardar mais
     ↓
     [Call Function: ConnectWebSocketManual] ← Tentar novamente
```

---

## 🧪 **TESTE:**

1. **Compile** o projeto
2. **Abra** o novo `BP_NetMovementClient` no Blueprint Editor
3. **Implemente** o `OnWSConnected` ou adicione retry no `BeginPlay`
4. **Execute** o jogo
5. **Verifique os logs:**

**Deve aparecer:**
```
[NetMovementClient] BeginPlay INICIADO!
[NetMovementClient] ✅ GameInstance encontrado!
[NetMovementClient] HasActiveCharacter: 0 (ou 1)
[BP_NEW] ConnectWebSocketManual chamado!
[NetMovementClient] ConnectWebSocketManual chamado!
[NetMovementClient] ✅ MyPlayerId setado: 1
[NetMovementClient] ✅ Criando e conectando WebSocket manualmente...
[NetMovementClient] WebSocket Connected!
```

---

## ✅ **RESUMO:**

**Problema:** O `BeginPlay` do C++ executa antes do personagem estar selecionado.

**Solução:**
1. ✅ Adicionei função `ConnectWebSocketManual()` que pode ser chamada do Blueprint
2. ✅ Adicionei logs detalhados no `BeginPlay` do C++
3. ✅ Implemente retry no Blueprint quando o personagem estiver pronto

**Com isso, o WebSocket deve conectar corretamente!**

