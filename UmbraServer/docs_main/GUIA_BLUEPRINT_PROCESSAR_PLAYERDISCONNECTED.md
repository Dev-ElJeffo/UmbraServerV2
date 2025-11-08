# 📋 **GUIA: Processar PlayerDisconnected no Blueprint**

## ✅ **CÓDIGO C++ IMPLEMENTADO:**

- ✅ Servidor envia mensagem `PlayerDisconnected` (5 bytes) quando um player desconecta
- ✅ Função `ParsePlayerDisconnected` disponível no Blueprint

---

## 📋 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **PASSO 1: Verificar Tipo de Mensagem em OnWSBinaryMessage**

**No `BP_NetMovementClient:OnWSBinaryMessage`:**

Após `ProcessBinaryBuffer` retornar `true`, **ANTES** de chamar `ProcessNextFrame`:

```
ProcessBinaryBuffer
  → ReturnValue
  ↓
Branch: ReturnValue?
  ├─ then: Get OutFrame
          ↓
          Break BinaryFrame (Data)
          ↓
          Get Array Item (Data, 0) → MessageType (Byte)
          ↓
          Equal (Byte)
            - A: MessageType
            - B: 3 (PlayerDisconnected)
            → IsDisconnectMessage
          ↓
          Branch: IsDisconnectMessage?
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId)
                      ↓
                      Branch: ReturnValue?
                          ├─ then: RemoveRemoteActor (OutPlayerId) ← Custom Event
                          └─ else: Print String: "Erro ao parsear PlayerDisconnected"
              └─ else: ProcessNextFrame (lógica normal)
  └─ else: (não há frame completo)
```

---

### **PASSO 2: Criar Custom Event RemoveRemoteActor**

**No `BP_NetMovementClient`:**

1. **Criar Custom Event:**
   - Nome: `RemoveRemoteActor`
   - Input: `PlayerID` (Integer)

2. **Implementar lógica:**

```
RemoveRemoteActor (Custom Event)
  Input: PlayerID
  ↓
Print String: "🔴 [RemoveRemoteActor] Removendo player: " + ToString(PlayerID)
  ↓
Array_Find (RemoteActorIds, PlayerID)
  → FoundIndex, bFound
  ↓
Branch: bFound?
  ├─ then: Get Array Item (RemoteActors, FoundIndex)
          ↓
          Is Valid (Array Element)
          ↓
          Branch: Is Valid?
              ├─ then: Destroy Actor (Array Element)
                      ↓
                      Print String: "🔴 [RemoveRemoteActor] Actor destruído!"
              └─ else: Print String: "🔴 [RemoveRemoteActor] Actor inválido!"
          ↓
          Remove Array Item (RemoteActorIds, FoundIndex)
          ↓
          Remove Array Item (RemoteActors, FoundIndex)
          ↓
          Print String: "🔴 [RemoveRemoteActor] Player removido dos arrays!"
  └─ else: Print String: "🔴 [RemoveRemoteActor] Player não encontrado nos arrays!"
```

---

## ⚠️ **IMPORTANTE:**

- **A verificação do tipo de mensagem deve ser ANTES de `ProcessNextFrame`**
- **Se for `PlayerDisconnected`, NÃO chamar `ProcessNextFrame`** (é uma mensagem diferente)
- **A mensagem `PlayerDisconnected` tem apenas 5 bytes**, então `ProcessBinaryBuffer` deve detectar corretamente

---

## 🧪 **TESTE:**

1. **Recompile o servidor C++**
2. **Recompile o projeto Unreal**
3. **Conecte 2 clientes**
4. **Pare o servidor** (Ctrl+C)
5. **VERIFICAR LOGS:**
   - Deve aparecer: "[RemoveRemoteActor] Removendo player: X"
   - Deve aparecer: "[RemoveRemoteActor] Actor destruído!"
   - Deve aparecer: "[RemoveRemoteActor] Player removido dos arrays!"
6. **VERIFICAR VISUALMENTE:**
   - O remote actor deve desaparecer imediatamente no outro cliente

