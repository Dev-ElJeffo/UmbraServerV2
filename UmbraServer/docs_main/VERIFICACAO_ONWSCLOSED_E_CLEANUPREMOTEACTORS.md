# 🔍 **VERIFICAÇÃO: OnWSClosed e CleanupRemoteActors**

## ✅ **CONFIRMADO:**

A estrutura está correta:
```
OnWSClosed
  ↓
Print String: "WebSocket closed - cleaning up..."
  ↓
Print String: "Starting cleanup - {0} RemoteActors to destroy"
  ↓
CleanupRemoteActors
```

---

## 🧪 **TESTE 1: Verificar se OnWSClosed Está Disparando**

**Adicione um log no início de `OnWSClosed`:**

```
OnWSClosed (Custom Event)
  ↓
Print String: "🔴 [OnWSClosed] EVENTO DISPARADO!"
  ↓
Print String: "WebSocket closed - cleaning up..."
```

**Teste:**
1. Conecte 2 clientes
2. Feche o WebSocket de um cliente (pare o servidor ou feche manualmente)
3. **VERIFICAR:** Aparece "OnWSClosed EVENTO DISPARADO!"?

---

## 🧪 **TESTE 2: Verificar se CleanupRemoteActors Está Sendo Chamado**

**Adicione um log ANTES de chamar `CleanupRemoteActors`:**

```
Print String: "Starting cleanup - {0} RemoteActors to destroy"
  ↓
Print String: "🔴 [OnWSClosed] Chamando CleanupRemoteActors..."
  ↓
CleanupRemoteActors
```

**E um log DENTRO de `CleanupRemoteActors` (primeira linha):**

```
CleanupRemoteActors (Custom Event)
  ↓
Print String: "🔴 [CleanupRemoteActors] EXECUTADO!"
  ↓
Print String: "🔴 [CleanupRemoteActors] RemoteActors.Num(): " + ToString(Get Array Length(RemoteActors))
```

---

## 🧪 **TESTE 3: Verificar se os Actors Estão Sendo Destruídos**

**DENTRO de `CleanupRemoteActors`, no `ForEachLoop`:**

Adicione logs:

```
ForEachLoop (RemoteActors)
  LoopBody:
    ↓
    Print String: "🔴 [CleanupRemoteActors] Processando actor..."
    ↓
    Is Valid (Array Element)
      ↓
    Branch: Is Valid?
      ├─ then: Print String: "🔴 [CleanupRemoteActors] Actor válido, destruindo..."
              ↓
              Destroy Actor
              ↓
              Print String: "🔴 [CleanupRemoteActors] Actor destruído!"
      └─ else: Print String: "🔴 [CleanupRemoteActors] Actor inválido, ignorando..."
```

---

## 🧪 **TESTE 4: Verificar se os Arrays Estão Sendo Limpos**

**No FINAL de `CleanupRemoteActors`:**

```
Clear Array (RemoteActors)
  ↓
Print String: "🔴 [CleanupRemoteActors] RemoteActors limpo. Num(): " + ToString(Get Array Length(RemoteActors))
  ↓
Clear Array (RemoteActorIds)
  ↓
Print String: "🔴 [CleanupRemoteActors] RemoteActorIds limpo. Num(): " + ToString(Get Array Length(RemoteActorIds))
  ↓
Print String: "🔴 [CleanupRemoteActors] Cleanup completo!"
```

---

## 🎯 **COMO TESTAR:**

1. **Inicie o servidor C++ (`zone_server`)**
2. **Inicie 2 clientes PIE**
3. **Pare o servidor** (Ctrl+C no terminal do servidor)
4. **VERIFICAR LOGS:**
   - Deve aparecer: "[OnWSClosed] EVENTO DISPARADO!"
   - Deve aparecer: "[OnWSClosed] Chamando CleanupRemoteActors..."
   - Deve aparecer: "[CleanupRemoteActors] EXECUTADO!"
   - Deve aparecer: "[CleanupRemoteActors] RemoteActors.Num(): X"
   - Deve aparecer: "[CleanupRemoteActors] Processando actor..."
   - Deve aparecer: "[CleanupRemoteActors] Actor válido, destruindo..."
   - Deve aparecer: "[CleanupRemoteActors] Actor destruído!"
   - Deve aparecer: "[CleanupRemoteActors] Cleanup completo!"

---

## 🔍 **DIAGNÓSTICO:**

**Se não aparecer "[OnWSClosed] EVENTO DISPARADO!":**
- O `OnWSClosed` não está disparando
- Verifique se o delegate está conectado corretamente no `BeginPlay`

**Se aparecer "[OnWSClosed] EVENTO DISPARADO!" mas não aparecer "[CleanupRemoteActors] EXECUTADO!":**
- A conexão entre `OnWSClosed` e `CleanupRemoteActors` está quebrada
- Verifique se o `then` do último `Print String` está conectado ao `execute` de `CleanupRemoteActors`

**Se aparecer "[CleanupRemoteActors] EXECUTADO!" mas `RemoteActors.Num()` é 0:**
- Não há actors para limpar (normal se já foram destruídos)
- Ou os actors não estão sendo adicionados ao array quando spawnados

**Se aparecer "Actor inválido, ignorando...":**
- Os actors já foram destruídos por outro processo
- Ou nunca foram válidos

**Se aparecer "Actor válido, destruindo..." mas não aparecer "Actor destruído!":**
- O `Destroy Actor` não está funcionando
- Verifique se há outras referências aos actors

