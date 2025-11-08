# 🔧 **SOLUÇÃO: Timeout para Remover Remote Actors**

## 🎯 **PROBLEMA:**

Quando um player desconecta:
- O servidor para de enviar frames desse player ✅
- O cliente que desconectou limpa seus remote actors ✅
- **MAS** os outros clientes não sabem que aquele player desconectou ❌
- Os remote actors continuam visíveis nos outros clientes ❌

---

## ✅ **SOLUÇÃO: Timeout de Inatividade**

**Implementar um sistema de timeout:**
- Se não receber frames de um player por X segundos (ex: 5 segundos)
- Assumir que o player desconectou
- Remover o remote actor automaticamente

---

## 📋 **IMPLEMENTAÇÃO NO BLUEPRINT:**

### **PASSO 1: Adicionar Variável de Timestamp por Player**

**No `BP_NetMovementClient`:**

1. **Criar um Array de Struct:**
   - Nome: `PlayerLastSeen`
   - Tipo: Criar um Struct com:
     - `PlayerID` (Integer)
     - `LastSeenTime` (Float)

**OU mais simples:**

2. **Criar um Map:**
   - Nome: `PlayerLastSeenMap`
   - Tipo: `Map<Integer, Float>` (PlayerID → LastSeenTime)

---

### **PASSO 2: Atualizar Timestamp em ProcessNextFrame**

**No `ProcessNextFrame`, após processar um frame:**

```
ProcessNextFrame
  ↓
[Processa frame - SetActorLocation, SetActorRotation, etc.]
  ↓
Get Game Time In Seconds
  ↓
Set Map Value (PlayerLastSeenMap)
  - Key: OutPlayerId
  - Value: Return Value (do Get Game Time In Seconds)
```

---

### **PASSO 3: Criar Timer para Verificar Timeout**

**No `BeginPlay` ou `OnWSConnected`:**

```
Set Timer by Function Name
  - Function Name: "CheckPlayerTimeouts"
  - Time: 1.0 (verificar a cada 1 segundo)
  - Looping: True
```

---

### **PASSO 4: Criar Função CheckPlayerTimeouts**

**Custom Event: `CheckPlayerTimeouts`**

```
CheckPlayerTimeouts (Custom Event)
  ↓
Get Game Time In Seconds → CurrentTime
  ↓
ForEachLoop (RemoteActorIds)
  LoopBody:
    ↓
    Get Array Element (PlayerID do loop)
    ↓
    Get Map Value (PlayerLastSeenMap)
      - Key: Array Element (PlayerID)
      - Value: LastSeenTime
    ↓
    Subtract (Float)
      - A: CurrentTime
      - B: LastSeenTime
      → TimeSinceLastSeen
    ↓
    Greater (Float)
      - A: TimeSinceLastSeen
      - B: 5.0 (timeout de 5 segundos)
      → ShouldRemove
    ↓
    Branch: ShouldRemove?
      ├─ then: Print String: "🔴 Player {PlayerID} timeout, removendo..."
              ↓
              Array_Find (RemoteActorIds, PlayerID)
              ↓
              Branch: bFound?
                  ├─ then: Get Array Item (RemoteActors, FoundIndex)
                          ↓
                          Is Valid (Array Element)
                          ↓
                          Branch: Is Valid?
                              ├─ then: Destroy Actor (Array Element)
                              └─ else: (não conectado)
                          ↓
                          Remove Array Item (RemoteActorIds, FoundIndex)
                          ↓
                          Remove Array Item (RemoteActors, FoundIndex)
                          ↓
                          Remove Map Value (PlayerLastSeenMap, PlayerID)
                  └─ else: (não encontrado)
      └─ else: (não timeout, continuar)
```

---

## ⚠️ **IMPORTANTE:**

- **Timeout de 5 segundos** é um valor sugerido (ajuste conforme necessário)
- **Verificar a cada 1 segundo** para balancear performance e responsividade
- **Remover do Map** quando remover o actor para evitar memory leak

---

## 🧪 **TESTE:**

1. Conecte 2 clientes
2. Mova ambos
3. Pare o servidor (ou feche um cliente)
4. **VERIFICAR:** Após 5 segundos, o remote actor deve desaparecer automaticamente no outro cliente

