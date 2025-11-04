# ⚡ **GUIA RÁPIDO: Debug Visibilidade Assimétrica**

## 🔴 **PROBLEMA:**
- Standalone 0 vê Standalone 1 ✅
- Standalone 1 **NÃO** vê Standalone 0 ❌

**Você já corrigiu o filtro**, mas o problema persiste.

---

## ✅ **ADICIONAR ESTES LOGS NO `ProcessNextFrame`:**

### **LOG 1: Verificar se Frame está Chegando**

**APÓS `ParseStateUpdateFrame` (quando `ReturnValue == true`):**

```
Format Text: "📥 Frame - OutPlayerId: {0}"
  - {0}: OutPlayerId
  ↓
Print String
```

---

### **LOG 2: Verificar Filtro**

**APÓS `Get Active Player ID` E ANTES DO `Branch`:**

```
Format Text: "🔍 Filtro - Active: {0}, Out: {1}, Processar: {2}"
  - {0}: [Get Active Player ID] → Return Value
  - {1}: OutPlayerId
  - {2}: [Not Equal] → Result
  ↓
Print String
```

**VERIFICAR:**
- Standalone 1 deve mostrar: `Active: 1, Out: 0, Processar: True`
- Se mostrar `Processar: False` → **Filtro está invertido!** ❌

---

### **LOG 3: Verificar Spawn**

**APÓS `SpawnActorFromClass`:**

```
Format Text: "🎬 Spawn - OutPlayerId: {0}, Actor válido: {1}"
  - {0}: OutPlayerId
  - {1}: Is Valid (SpawnedActor)
  ↓
Print String
```

**VERIFICAR:**
- Se `Actor válido: True` → Spawn funcionou ✅
- Se `Actor válido: False` → Spawn falhou ❌

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

**SE O LOG 3 MOSTRAR `Actor válido: False`:**

**VERIFICAR:**
- `OutLocation` não é (0,0,0)?
- Há collision no spawn?
- `BP_RemotePlayer` está configurado corretamente?

---

### **3. Actor não está sendo Adicionado aos Arrays**

**ADICIONAR LOG APÓS `Array_Add`:**

```
Format Text: "📋 Arrays - Size: {0}"
  - {0}: Length (RemoteActorIds)
  ↓
Print String
```

**VERIFICAR:**
- Se `Size` aumenta após spawn → Funcionando ✅
- Se não aumenta → Arrays não estão sendo atualizados ❌

---

## 📊 **COMPARTILHE OS LOGS:**

**Após adicionar os logs, compartilhe:**
1. Log do Standalone 1 quando recebe frame do PlayerID=0
2. Valor do filtro (`Processar: True/False`)
3. Se o actor foi spawnado (`Actor válido: True/False`)

**Isso vai identificar exatamente onde está o problema!**

