# ⚡ **GUIA RÁPIDO: Debug Visibilidade Assimétrica**

## 🔴 **PROBLEMA:**
- ✅ Cliente com `MyID: 1` vê cliente com `MyID: 0`
- ❌ Cliente com `MyID: 0` **NÃO** vê cliente com `MyID: 1`

**IMPORTANTE:** Todos os clientes compartilham o mesmo código. Use `Get Active Player ID` para identificar qual cliente está logando.

---

## ✅ **ADICIONAR ESTES LOGS NO `ProcessNextFrame`:**

### **LOG 1: Verificar se Frame está Chegando**

**APÓS `ParseStateUpdateFrame` (quando `ReturnValue == true`):**

```
ParseStateUpdateFrame
  ↓ (ReturnValue == true)
  ├─ OutPlayerId: Integer ← USAR ESTE VALOR!
  ├─ OutLocation: Vector
  ├─ OutYawDegrees: Float
  └─ OutTimestampMs: Integer
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📥 [MyID:{0}] Frame - OutPlayerId: {1}"
  - {0}: Return Value (do Get Active Player ID) ← Active Player ID
  - {1}: OutPlayerId (do ParseStateUpdateFrame) ← NÃO USE RemoteActorIds!
  ↓
Print String
```

**⚠️ ERRO COMUM:**
- ❌ **NÃO conecte `RemoteActorIds`** ao `Format Text` - é um Array e causará erro!
- ✅ **Use `OutPlayerId`** do `ParseStateUpdateFrame` - é um Integer simples

**VERIFICAR:** 
- Cliente com `MyID: 0` deve mostrar `[MyID:0] OutPlayerId: 1` quando o outro se move
- Cliente com `MyID: 1` deve mostrar `[MyID:1] OutPlayerId: 0` quando o outro se move

---

### **LOG 2: Verificar Filtro**

**APÓS `Get Active Player ID` E ANTES DO `Branch`:**

```
Get Active Player ID
  ↓
Format Text: "🔍 [MyID:{0}] Filtro - Active: {0}, Out: {1}, Processar: {2}"
  - {0}: Return Value (Active Player ID) ← Mesmo valor usado duas vezes
  - {1}: OutPlayerId
  - {2}: [Not Equal] → Result
  ↓
Print String
```

**VERIFICAR:** 
- Cliente com `MyID: 0` recebendo `OutPlayerId: 1`: deve mostrar `Processar: True`
- Se mostrar `Processar: False` → **Filtro invertido!** ❌

---

### **LOG 3: Verificar Array_Find**

**APÓS `Array_Find`:**

```
Array_Find (RemoteActorIds, OutPlayerId)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "🎭 [MyID:{0}] Actor - OutPlayerId: {1}, FoundIndex: {2}"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: FoundIndex
  ↓
Print String
```

**VERIFICAR:**
- Primeira vez: `FoundIndex: -1` (deve spawnar) ✅
- Se `FoundIndex >= 0` na primeira vez → Problema nos arrays ❌

---

### **LOG 4: Verificar Spawn**

**APÓS `SpawnActorFromClass`:**

```
SpawnActorFromClass
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "🎬 [MyID:{0}] Spawn - OutPlayerId: {1}, Actor válido: {2}"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: Is Valid (SpawnedActor)
  ↓
Print String
```

**VERIFICAR:**
- Deve mostrar: `Actor válido: True`
- Se `Actor válido: False` → Spawn falhou ❌

---

### **LOG 5: Verificar Arrays**

**APÓS `Array_Add`:**

```
Array_Add (RemoteActorIds, OutPlayerId)
  ↓
Array_Add (RemoteActors, SpawnedActor)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📋 [MyID:{0}] Arrays - RemoteActorIds: {1}, RemoteActors: {2}"
  - {0}: Return Value (Active Player ID)
  - {1}: Length (RemoteActorIds)
  - {2}: Length (RemoteActors)
  ↓
Print String
```

**VERIFICAR:**
- Após spawn: `RemoteActorIds: 1, RemoteActors: 1`
- Se não aumentar → Arrays não estão sendo atualizados ❌

---

## 🎯 **POSSÍVEIS CAUSAS:**

### **1. Filtro Invertido** (Mais Provável)

**SE O LOG 2 MOSTRAR `Processar: False` QUANDO DEVERIA SER `True`:**

**CORREÇÃO:**
- Troque as conexões do `Branch`:
  - `True` → Deve processar (outro player)
  - `False` → Deve ignorar (próprio player)

---

### **2. Spawn Falhando**

**SE O LOG 4 MOSTRAR `Actor válido: False`:**

**VERIFICAR:**
- `OutLocation` não é (0,0,0)?
- Há collision no spawn?
- `BP_RemotePlayer` está configurado corretamente?

---

### **3. Arrays Não Atualizados**

**SE O LOG 5 MOSTRAR `Size: 0` APÓS SPAWN:**

**VERIFICAR:**
- `Array_Add` está sendo chamado?
- Pin `execute` está conectado?

---

## 📊 **COMPARTILHE OS LOGS:**

**Após adicionar os logs, compartilhe de AMBOS os clientes:**

1. Logs do cliente com `MyID: 0` quando recebe `OutPlayerId: 1`
2. Logs do cliente com `MyID: 1` quando recebe `OutPlayerId: 0`
3. Compare os dois - o cliente que para de logar em algum ponto tem o problema!

**Isso vai identificar exatamente onde está o problema!**

