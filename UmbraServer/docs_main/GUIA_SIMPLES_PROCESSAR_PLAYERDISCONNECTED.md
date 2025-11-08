# 📋 **GUIA SIMPLES: Processar PlayerDisconnected no Blueprint**

## ✅ **CÓDIGO C++ IMPLEMENTADO:**

- ✅ Servidor envia mensagem `PlayerDisconnected` (5 bytes: `[type:3][playerId:4]`) quando um player desconecta
- ✅ Função `ParsePlayerDisconnected(Data, OutPlayerId)` disponível no Blueprint

---

## 📋 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **PASSO 1: Verificar Tipo de Mensagem ANTES de ProcessBinaryBuffer**

**No `BP_NetMovementClient:OnWSBinaryMessage`:**

**IMPORTANTE:** A mensagem `PlayerDisconnected` tem apenas 5 bytes, então precisa ser processada ANTES de `ProcessBinaryBuffer` (que espera 25 ou 34 bytes).

```
OnWSBinaryMessage (Data)
  ↓
Get Array Length (Data)
  ↓
Equal (Integer)
  - A: Return Value (Length)
  - B: 5
  → IsDisconnectMessage
  ↓
Branch: IsDisconnectMessage?
  ├─ then: Get Array Item (Data, 0) → MessageType
          ↓
          Equal (Byte)
            - A: MessageType
            - B: 3 (PlayerDisconnected)
            → IsType3
          ↓
          Branch: IsType3?
              ├─ then: ParsePlayerDisconnected (Data, OutPlayerId)
                      ↓
                      Branch: ReturnValue?
                          ├─ then: RemoveRemoteActor (OutPlayerId)
                          └─ else: Print String: "Erro ao parsear PlayerDisconnected"
              └─ else: Print String: "Mensagem de 5 bytes com tipo desconhecido"
  └─ else: ProcessBinaryBuffer (lógica normal para frames de 25/34 bytes)
          ↓
          Branch: ReturnValue?
              ├─ then: ProcessNextFrame
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

- **A mensagem `PlayerDisconnected` tem 5 bytes**, então precisa ser processada ANTES de `ProcessBinaryBuffer`
- **Verificar se `Data.Num() == 5` e `Data[0] == 3`** antes de processar
- **Se for `PlayerDisconnected`, NÃO chamar `ProcessBinaryBuffer`** (é uma mensagem diferente)

---

## 🧪 **TESTE:**

1. **Recompile o servidor C++**
2. **Recompile o projeto Unreal**
3. **Conecte 2 clientes**
4. **Pare o servidor** (Ctrl+C)
5. **VERIFICAR LOGS:**
   - Deve aparecer: "[RemoveRemoteActor] Removendo player: X"
   - Deve aparecer: "[RemoveRemoteActor] Actor destruído!"
   - O remote actor deve desaparecer imediatamente no outro cliente

