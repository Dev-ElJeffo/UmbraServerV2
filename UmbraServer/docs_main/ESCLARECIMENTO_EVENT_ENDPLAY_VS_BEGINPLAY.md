# 📋 **ESCLARECIMENTO: Event EndPlay vs BeginPlay**

## ❌ **NÃO CONECTAR EndPlay ao BeginPlay**

### **Por que não?**

`Event EndPlay` e `Event BeginPlay` são eventos **automáticos** do Unreal Engine que são disparados em **momentos diferentes** do ciclo de vida do objeto:

- **`BeginPlay`**: Disparado quando o objeto é **criado/começa a existir**
- **`EndPlay`**: Disparado quando o objeto está sendo **destruído/saindo de existência**

Eles são **completamente independentes** e não precisam ser conectados manualmente.

---

## ✅ **COMO FUNCIONA:**

### **Estrutura Correta:**

```
┌─────────────────────────────────────┐
│  EVENT GRAPH (BP_NetMovementClient) │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  Event BeginPlay                    │  ← Disparado automaticamente quando spawna
│    ↓                                │
│  [Lógica de inicialização]          │
│    - Conectar WebSocket             │
│    - Bind Event to OnClosed         │
│    - Etc.                           │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  Event EndPlay                      │  ← Disparado automaticamente quando destrói
│    ↓                                │
│  [Lógica de cleanup]                │
│    - Close WebSocket                │
│    - CleanupRemoteActors            │
│    - Etc.                           │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  Event OnWSClosed (Custom Event)    │  ← Disparado quando WebSocket fecha
│    ↓                                │
│  CleanupRemoteActors                │
└─────────────────────────────────────┘
```

---

## 📝 **INSTRUÇÕES CORRETAS:**

### **PASSO 1: Adicionar Event EndPlay**

1. Abra o `Event Graph` do `BP_NetMovementClient`
2. **Clique com botão direito** em uma área vazia
3. Procure por: **`Event EndPlay`**
4. **Selecione**: `Event EndPlay` (não precisa conectar a nada!)

### **PASSO 2: Conectar Lógica ao EndPlay**

Após adicionar o `Event EndPlay`, conecte a lógica diretamente ao seu pin de execução:

```
Event EndPlay
  ↓ (pin de execução "then")
Branch: Is Valid (WebSocketRef)?
  ├─ True:
  │   ├─ Print String: "EndPlay - Closing WebSocket..."
  │   ├─ Call Function: Close (WebSocketRef)
  │   └─ CleanupRemoteActors
  └─ False: (Não fazer nada)
```

**NÃO precisa conectar ao `BeginPlay`!**

---

## 🔍 **FLUXO COMPLETO DO CICLO DE VIDA:**

```
1. OBJETO É CRIADO
   ↓
2. Event BeginPlay é disparado AUTOMATICAMENTE
   ↓
   [Lógica de inicialização]
   - Conectar WebSocket
   - Bind Event to OnClosed
   ↓
3. OBJETO ESTÁ ATIVO
   ↓
   [Durante o jogo]
   - ProcessNextFrame
   - SendMoveUpdate
   - OnWSClosed (se WebSocket fecha)
   ↓
4. OBJETO ESTÁ SENDO DESTRUÍDO (fechar PIE, mudar nível, etc.)
   ↓
5. Event EndPlay é disparado AUTOMATICAMENTE
   ↓
   [Lógica de cleanup]
   - Close WebSocket
   - CleanupRemoteActors
   ↓
6. OBJETO É DESTRUÍDO
```

---

## ✅ **CHECKLIST CORRETO:**

- [ ] `Event EndPlay` adicionado ao `Event Graph` (não conectado ao `BeginPlay`)
- [ ] Lógica de cleanup conectada ao pin de execução do `Event EndPlay`
- [ ] `Is Valid (WebSocketRef)` verificado antes de fechar
- [ ] `Close (WebSocketRef)` chamado
- [ ] `CleanupRemoteActors` chamado

---

## 🎯 **RESUMO:**

- **`BeginPlay`**: Para inicialização (conectar WebSocket, bind events)
- **`EndPlay`**: Para cleanup (fechar WebSocket, limpar actors)
- **Não conecte um ao outro!** Eles são disparados automaticamente pelo Unreal Engine em momentos diferentes.

---

**Apenas adicione o `Event EndPlay` ao `Event Graph` e conecte a lógica diretamente ao seu pin de execução!**

