# 🔍 **DEBUG: Visibilidade Assimétrica - Diagnóstico Completo**

## 📋 **PROBLEMA:**

- ✅ Standalone 0 vê Standalone 1
- ❌ Standalone 1 **NÃO** vê Standalone 0

**Você já corrigiu o filtro para usar `Get Active Player ID`**, mas o problema persiste.

---

## 🔍 **DIAGNÓSTICO PASSO A PASSO:**

### **1. Verificar se Standalone 1 está RECEBENDO frames do PlayerID=0**

**ADICIONAR LOG NO `ProcessNextFrame` (ANTES DO FILTRO):**

```
ParseStateUpdateFrame
  ↓
Format Text: "📥 Frame recebido - OutPlayerId: {0}"
  - {0}: OutPlayerId
  ↓
Print String
```

**VERIFICAR:**
- Standalone 1 deve mostrar logs como `OutPlayerId: 0`
- Se **NÃO** aparecer → Servidor não está enviando frames do PlayerID=0 para Standalone 1
- Se aparecer → Frame está chegando, mas está sendo filtrado ou não processado

---

### **2. Verificar se o Filtro está Funcionando Corretamente**

**ADICIONAR LOG APÓS O FILTRO:**

```
Get Active Player ID
  ↓
Format Text: "🔍 Filtro - Active Player ID: {0}, OutPlayerId: {1}, Deve processar: {2}"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: [Not Equal] → Result (True/False)
  ↓
Print String
```

**VERIFICAR:**
- Standalone 1 com `OutPlayerId: 0`:
  - Se `Active Player ID: 1` e `Deve processar: True` → Filtro está correto ✅
  - Se `Active Player ID: 1` e `Deve processar: False` → **PROBLEMA!** Filtro está invertido ❌

---

### **3. Verificar se o Actor está sendo Spawnado**

**ADICIONAR LOG APÓS `Array_Find` (verificação se actor existe):**

```
Array_Find (RemoteActorIds, OutPlayerId)
  ↓
Format Text: "🎭 Actor - OutPlayerId: {0}, FoundIndex: {1}"
  - {0}: OutPlayerId
  - {1}: FoundIndex
  ↓
Print String
```

**VERIFICAR:**
- Se `FoundIndex = -1` → Actor não existe, deve spawnar
- Se `FoundIndex >= 0` → Actor existe, deve atualizar

**ADICIONAR LOG APÓS `SpawnActorFromClass`:**

```
SpawnActorFromClass
  ↓
Format Text: "🎬 Spawn - OutPlayerId: {0}, SpawnedActor: {1}, Is Valid: {2}"
  - {0}: OutPlayerId
  - {1}: SpawnedActor (To String)
  - {2}: Is Valid (SpawnedActor)
  ↓
Print String
```

**VERIFICAR:**
- Se `SpawnedActor` é válido → Actor foi spawnado ✅
- Se `SpawnedActor` é inválido → Spawn falhou ❌

---

### **4. Verificar se o Actor está sendo Adicionado aos Arrays**

**ADICIONAR LOG APÓS `Array_Add`:**

```
Array_Add (RemoteActorIds, OutPlayerId)
  ↓
Array_Add (RemoteActors, SpawnedActor)
  ↓
Format Text: "📋 Arrays - RemoteActorIds Size: {0}, RemoteActors Size: {1}"
  - {0}: Length (RemoteActorIds)
  - {1}: Length (RemoteActors)
  ↓
Print String
```

**VERIFICAR:**
- Se `RemoteActorIds Size` aumenta após spawn → Actor foi adicionado ✅
- Se não aumenta → Actor não foi adicionado ❌

---

### **5. Verificar se a Posição está sendo Atualizada**

**ADICIONAR LOG APÓS `Set Actor Location`:**

```
Set Actor Location (RemoteActorRef, OutLocation)
  ↓
Format Text: "📍 Location - OutPlayerId: {0}, Location: ({1}, {2}, {3})"
  - {0}: OutPlayerId
  - {1}: OutLocation.X
  - {2}: OutLocation.Y
  - {3}: OutLocation.Z
  ↓
Print String
```

**VERIFICAR:**
- Se a posição está sendo atualizada corretamente
- Se a posição não é (0,0,0)

---

## 🎯 **CHECKLIST DE DIAGNÓSTICO:**

### **Standalone 1 (que não vê Standalone 0):**

**1. Frame está chegando?**
- [ ] Log mostra `OutPlayerId: 0`?
- [ ] Se não → Problema no servidor ou conexão

**2. Filtro está correto?**
- [ ] Log mostra `Active Player ID: 1, OutPlayerId: 0, Deve processar: True`?
- [ ] Se `Deve processar: False` → Filtro está invertido ❌

**3. Actor está sendo spawnado?**
- [ ] Log mostra `FoundIndex: -1` (primeira vez)?
- [ ] Log mostra `SpawnedActor: [válido]`?
- [ ] Se não → Spawn está falhando ❌

**4. Actor está sendo adicionado aos arrays?**
- [ ] Log mostra `RemoteActorIds Size` aumentando?
- [ ] Se não → Arrays não estão sendo atualizados ❌

**5. Posição está sendo atualizada?**
- [ ] Log mostra `Location: (X, Y, Z)` (não 0,0,0)?
- [ ] Se não → Posição está zerada ❌

---

## 🔧 **CORREÇÕES POSSÍVEIS:**

### **CORREÇÃO 1: Filtro Invertido**

**SE O LOG MOSTRAR `Deve processar: False` QUANDO DEVERIA SER `True`:**

Verifique se o `Branch` está conectado corretamente:
- `True` (Not Equal = True) → Deve processar (outro player)
- `False` (Not Equal = False) → Deve ignorar (próprio player)

**Se estiver invertido:**
- Troque as conexões do `Branch` (True ↔ False)

---

### **CORREÇÃO 2: Spawn Falhando**

**SE O LOG MOSTRAR `SpawnedActor: [inválido]`:**

Verifique:
1. **Location não é (0,0,0):**
   - Adicione validação antes de spawnar
   - Se `OutLocation == (0,0,0)`, não spawnar

2. **Collision no spawn:**
   - Verifique se há objetos na posição de spawn
   - Tente spawnar em posição ligeiramente diferente (Y + 50)

3. **Class de spawn está correta:**
   - Verifique se `BP_RemotePlayer` está configurado corretamente

---

### **CORREÇÃO 3: Actor não está sendo Adicionado aos Arrays**

**SE O LOG MOSTRAR `RemoteActorIds Size` NÃO AUMENTANDO:**

Verifique:
1. **`Array_Add` está sendo chamado?**
   - Adicione log antes de cada `Array_Add`
   - Verifique se o pin `execute` está conectado

2. **Arrays estão sincronizados?**
   - `RemoteActorIds` e `RemoteActors` devem ter o mesmo tamanho
   - Se não, há problema na lógica de adição

---

### **CORREÇÃO 4: Actor está sendo Spawnado mas Não Aparece**

**SE O LOG MOSTRAR `SpawnedActor: [válido]` MAS NÃO APARECE NA TELA:**

Verifique:
1. **Actor está sendo spawnado no mundo correto?**
   - Verifique se está no nível correto
   - Verifique se está sendo spawnado no nível do jogo (não no nível do editor)

2. **Actor está visível?**
   - Verifique se o mesh está configurado como visível
   - Verifique se não está sendo spawnado dentro de outro objeto

3. **Câmera está olhando para o lugar certo?**
   - Verifique se a câmera está na posição correta
   - Verifique se o actor está na área visível da câmera

---

## 📊 **RESUMO:**

**Adicione os logs acima e compartilhe os resultados para identificar exatamente onde o problema está ocorrendo.**

**Possíveis causas:**
1. ❌ Frame não está chegando (servidor não envia)
2. ❌ Filtro está invertido (rejeita quando deveria aceitar)
3. ❌ Spawn está falhando (actor inválido)
4. ❌ Actor não está sendo adicionado aos arrays
5. ❌ Posição está zerada (spawn em 0,0,0)
6. ❌ Actor está sendo spawnado mas não aparece (problema de visibilidade)

