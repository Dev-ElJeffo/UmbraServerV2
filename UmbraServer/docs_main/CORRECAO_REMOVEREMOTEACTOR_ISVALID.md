# 🔧 **CORREÇÃO: RemoveRemoteActor - Is Valid**

## ❌ **PROBLEMA IDENTIFICADO:**

1. **`Array_Find` retorna Integer (índice)**, não Boolean
   - Retorna `-1` se não encontrou
   - Retorna `>= 0` se encontrou
   - **NÃO pode ser conectado diretamente ao `Branch`**

2. **Você está usando `RemoteActorRef` (variável única)** em vez de pegar do array `RemoteActors`

---

## ✅ **CORREÇÃO:**

### **PASSO 1: Verificar se Array_Find encontrou o item**

**NÃO use `Is Valid` aqui!** Use comparação de Integer:

```
Array_Find (RemoteActorIds, PlayerID)
  → FoundIndex (Integer)
  ↓
Greater or Equal (Integer)
  - A: FoundIndex
  - B: 0
  → bFound (Boolean)
  ↓
Branch: bFound?
  ├─ then: (player encontrado)
  └─ else: Print String: "Player não encontrado"
```

**OU:**

```
Array_Find (RemoteActorIds, PlayerID)
  → FoundIndex (Integer)
  ↓
Not Equal (Integer)
  - A: FoundIndex
  - B: -1
  → bFound (Boolean)
  ↓
Branch: bFound?
```

---

### **PASSO 2: Obter o Actor do array `RemoteActors`**

**NÃO use `RemoteActorRef`!** Use `Get Array Item`:

```
Branch: bFound? (then)
  ↓
Get Array Item (RemoteActors, FoundIndex)
  → Actor (Actor Reference)
  ↓
Is Valid (Actor) ← AQUI SIM, use Is Valid!
  ↓
Branch: Is Valid?
  ├─ then: Destroy Actor (Actor)
  └─ else: Print String: "Actor inválido"
```

---

### **PASSO 3: Remover dos arrays**

**IMPORTANTE:** Remover pelo **índice**, não pelo valor:

```
Destroy Actor (Actor)
  ↓
Remove Array Item (RemoteActorIds, FoundIndex) ← Remove pelo índice!
  ↓
Remove Array Item (RemoteActors, FoundIndex) ← Remove pelo índice!
```

**NÃO use `Array_RemoveItem`** (remove pelo valor), use `Remove Array Item` (remove pelo índice)!

---

## 📋 **ESTRUTURA COMPLETA CORRIGIDA:**

```
RemoveRemoteActor (Custom Event)
  Input: PlayerID
  ↓
Print String: "[RemoveRemoteActor] Removendo player: " + ToString(PlayerID)
  ↓
Array_Find (RemoteActorIds, PlayerID)
  → FoundIndex, bFound (se disponível)
  ↓
Greater or Equal (Integer)
  - A: FoundIndex
  - B: 0
  → bFound
  ↓
Branch: bFound?
  ├─ then: Get Array Item (RemoteActors, FoundIndex)
          ↓
          Is Valid (Actor) ← AQUI SIM!
          ↓
          Branch: Is Valid?
              ├─ then: Destroy Actor (Actor)
                      ↓
                      Print String: "[RemoveRemoteActor] Actor destruído!"
              └─ else: Print String: "[RemoveRemoteActor] Actor inválido!"
          ↓
          Remove Array Item (RemoteActorIds, FoundIndex) ← Pelo índice!
          ↓
          Remove Array Item (RemoteActors, FoundIndex) ← Pelo índice!
          ↓
          Print String: "[RemoveRemoteActor] Player removido dos arrays!"
  └─ else: Print String: "[RemoveRemoteActor] Player não encontrado nos arrays!"
```

---

## ⚠️ **RESUMO:**

- **Para verificar se `Array_Find` encontrou:** Use `Greater or Equal (Integer >= 0)` ou `Not Equal (Integer != -1)`
- **Para verificar se Actor é válido:** Use `Is Valid` (funciona com objetos/referências)
- **Para obter o Actor:** Use `Get Array Item (RemoteActors, FoundIndex)`, não `RemoteActorRef`
- **Para remover dos arrays:** Use `Remove Array Item` (pelo índice), não `Array_RemoveItem` (pelo valor)

