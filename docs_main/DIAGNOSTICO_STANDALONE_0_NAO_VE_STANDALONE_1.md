# 🔍 **DIAGNÓSTICO: Standalone 0 Não Vê Standalone 1**

## 📋 **PROBLEMA ATUAL:**

**EVIDÊNCIA VISUAL:**
- ✅ **Standalone 1** vê **Standalone 0** (2 characters visíveis)
- ❌ **Standalone 0** NÃO vê **Standalone 1** (apenas 1 character visível)

**LOGS:**
- Ambos os clientes estão recebendo frames de múltiplos PlayerIDs (18, 14, 4, 1, 2)
- `ProcessNextFrame called!` está sendo executado em ambos
- `ParseStateUpdateFrame` está funcionando em ambos

**CONCLUSÃO:** O problema está na lógica de spawn/atualização do actor remoto em Standalone 0.

---

## 🔍 **DIAGNÓSTICO PASSO A PASSO:**

### **1. Verificar se o Cliente está RECEBENDO frames do Player remoto**

**ADICIONAR LOG NO `ProcessNextFrame` (APÓS `ParseStateUpdateFrame`):**

```
ParseStateUpdateFrame
  ↓ (ReturnValue == true)
  ├─ Return Value: Boolean
  ├─ OutPlayerId: Integer ← USAR ESTE VALOR!
  ├─ OutLocation: Vector
  ├─ OutYawDegrees: Float
  └─ OutTimestampMs: Integer
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID  ← OBTER ID DO CLIENTE ATUAL
  ↓
Format Text: "📥 [MyID:{0}] Frame recebido - OutPlayerId: {1}"
  - {0}: Return Value (do Get Active Player ID) ← Active Player ID
  - {1}: OutPlayerId (do ParseStateUpdateFrame) ← NÃO USE RemoteActorIds!
  ↓
Print String
```

**⚠️ IMPORTANTE:**
- **Use `OutPlayerId`** (output do `ParseStateUpdateFrame`) - é um **Integer simples**
- **NÃO use `RemoteActorIds`** - é um **Array de Integers** e causará erro de compilação

**VERIFICAR:**
- Cliente com `MyID: 0` deve mostrar logs como `OutPlayerId: 1` quando o outro cliente se move
- Cliente com `MyID: 1` deve mostrar logs como `OutPlayerId: 0` quando o outro cliente se move
- Se **NÃO** aparecer → Servidor não está enviando frames ou está sendo filtrado antes
- Se aparecer → Frame está chegando, mas pode estar sendo filtrado depois

---

### **2. Verificar se o Filtro está Funcionando Corretamente**

**ADICIONAR LOG APÓS O FILTRO:**

```
Get Active Player ID
  ↓
Not Equal: OutPlayerId != [Return Value do Get Active Player ID]
  ↓
  ├─ Input A: OutPlayerId (do ParseStateUpdateFrame)
  ├─ Input B: Return Value (do Get Active Player ID)
  └─ Output: Boolean (True/False) ← CONECTAR ESTE AO FORMAT TEXT!
  ↓
Format Text: "🔍 [MyID:{0}] Filtro - Active: {0}, Out: {1}, Processar: {2}"
  - {0}: Return Value (do Get Active Player ID) ← Active Player ID
  - {1}: OutPlayerId (do ParseStateUpdateFrame) ← OutPlayerId
  - {2}: Output (do Not Equal) ← CONECTE O OUTPUT DO NOT EQUAL AQUI!
  ↓
Print String
  ↓
Branch: Condition = Output (do Not Equal)
  ├─ True: CONTINUA (outro player - processa)
  └─ False: PARA (próprio player - ignora)
```

**⚠️ IMPORTANTE:**
- Conecte o **output do `Not Equal`** ao pin `{2}` do `Format Text`
- O `Not Equal` retorna um **Boolean** (True/False)
- Se o `Format Text` não aceitar Boolean diretamente:
  - Use `To String (Boolean)` para converter antes de conectar ao `{2}`

**ALTERNATIVA (se Format Text não aceitar Boolean):**
```
Not Equal: OutPlayerId != Active Player ID
  ↓
To String (Boolean) ← CONVERTER PARA STRING
  ↓
Format Text: "🔍 [MyID:{0}] Filtro - Active: {0}, Out: {1}, Processar: {2}"
  - {0}: Return Value (do Get Active Player ID)
  - {1}: OutPlayerId (do ParseStateUpdateFrame)
  - {2}: Return Value (do To String) ← String "True" ou "False"
  ↓
Print String
```

**VERIFICAR:**
- Cliente com `MyID: 0` recebendo `OutPlayerId: 1`:
  - Se `Active: 0, Out: 1, Processar: True` → Filtro está correto ✅
  - Se `Active: 0, Out: 1, Processar: False` → **PROBLEMA!** Filtro está invertido ❌
- Cliente com `MyID: 1` recebendo `OutPlayerId: 0`:
  - Se `Active: 1, Out: 0, Processar: True` → Filtro está correto ✅
  - Se `Active: 1, Out: 0, Processar: False` → **PROBLEMA!** Filtro está invertido ❌

---

### **3. Verificar se Array_Find está Funcionando**

**ADICIONAR LOG APÓS `Array_Find`:**

```
Array_Find (RemoteActorIds, OutPlayerId)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "🎭 [MyID:{0}] Actor - OutPlayerId: {1}, FoundIndex: {2}, Array Size: {3}"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: FoundIndex
  - {3}: Length (RemoteActorIds)
  ↓
Print String
```

**VERIFICAR:**
- Primeira vez que recebe um `OutPlayerId` diferente do próprio:
  - Se `FoundIndex: -1` → Actor não existe, deve spawnar ✅
  - Se `FoundIndex >= 0` → Actor já existe (improvável na primeira vez) ❌
- Se `Array Size: 0` → Arrays estão vazios, primeiro spawn deve funcionar ✅

---

### **4. Verificar se SpawnActor está sendo Chamado**

**ADICIONAR LOG ANTES DE `SpawnActorFromClass`:**

```
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "🎬 [MyID:{0}] Tentando spawnar - OutPlayerId: {1}, Location: ({2}, {3}, {4})"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: OutLocation.X
  - {3}: OutLocation.Y
  - {4}: OutLocation.Z
  ↓
Print String
  ↓
SpawnActorFromClass
```

**ADICIONAR LOG APÓS `SpawnActorFromClass`:**

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
- Se `Actor válido: True` → Spawn funcionou ✅
- Se `Actor válido: False` → Spawn falhou ❌
- Se `Location: (0, 0, 0)` → Posição zerada, spawn pode falhar ❌

---

### **5. Verificar se Array_Add está sendo Executado**

**ADICIONAR LOG ANTES DE CADA `Array_Add`:**

```
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📋 [MyID:{0}] Adicionando ao array - OutPlayerId: {1}"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  ↓
Print String
  ↓
Array_Add (RemoteActorIds, OutPlayerId)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📋 [MyID:{0}] Array_Add RemoteActorIds - Size: {1}"
  - {0}: Return Value (Active Player ID)
  - {1}: Length (RemoteActorIds)
  ↓
Print String
  ↓
Array_Add (RemoteActors, SpawnedActor)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📋 [MyID:{0}] Array_Add RemoteActors - Size: {1}"
  - {0}: Return Value (Active Player ID)
  - {1}: Length (RemoteActors)
  ↓
Print String
```

**VERIFICAR:**
- Se `Size` aumenta após `Array_Add` → Arrays estão sendo atualizados ✅
- Se `Size` não aumenta → Arrays não estão sendo atualizados ❌
- Se `RemoteActorIds Size != RemoteActors Size` → Arrays desincronizados ❌

---

### **6. Verificar se Set Actor Location está sendo Executado**

**ADICIONAR LOG APÓS `Set Actor Location`:**

```
Set Actor Location (RemoteActorRef, OutLocation)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Format Text: "📍 [MyID:{0}] Location atualizada - OutPlayerId: {1}, Location: ({2}, {3}, {4})"
  - {0}: Return Value (Active Player ID)
  - {1}: OutPlayerId
  - {2}: OutLocation.X
  - {3}: OutLocation.Y
  - {4}: OutLocation.Z
  ↓
Print String
```

**VERIFICAR:**
- Se a posição está sendo atualizada corretamente
- Se a posição não é (0,0,0)

---

## 🎯 **CHECKLIST ESPECÍFICO:**

**Para identificar qual cliente está tendo problema, procure pelos logs com `MyID: 0`:**

**1. Frame está chegando?**
- [ ] Log mostra `[MyID:0] OutPlayerId: 1` quando o outro cliente se move?
- [ ] Se não → Problema no servidor ou conexão

**2. Filtro está correto?**
- [ ] Log mostra `[MyID:0] Active: 0, Out: 1, Processar: True`?
- [ ] Se `Processar: False` → Filtro está invertido ❌

**3. Actor está sendo spawnado?**
- [ ] Log mostra `[MyID:0] FoundIndex: -1` (primeira vez)?
- [ ] Log mostra `[MyID:0] Tentando spawnar`?
- [ ] Log mostra `[MyID:0] Actor válido: True`?
- [ ] Se não → Spawn está falhando ❌

**4. Arrays estão sendo atualizados?**
- [ ] Log mostra `[MyID:0] Adicionando ao array`?
- [ ] Log mostra `[MyID:0] Array_Add RemoteActorIds - Size: 1`?
- [ ] Log mostra `[MyID:0] Array_Add RemoteActors - Size: 1`?
- [ ] Se não → Arrays não estão sendo atualizados ❌

**5. Posição está sendo atualizada?**
- [ ] Log mostra `[MyID:0] Location atualizada`?
- [ ] Se não → Posição não está sendo atualizada ❌

**COMPARAÇÃO:**
- Compare os logs `[MyID:0]` com `[MyID:1]`
- Se `[MyID:1]` mostra todos os logs mas `[MyID:0]` para em algum ponto → Problema identificado!

---

## 🔧 **CORREÇÕES ESPECÍFICAS:**

### **PROBLEMA 1: Filtro Invertido em Standalone 0**

**SE O LOG MOSTRAR `Processar: False` QUANDO `OutPlayerId: 1`:**

**CORREÇÃO:**
- Verifique se o `Branch` está conectado corretamente:
  - `True` (Not Equal = True) → Deve processar (outro player)
  - `False` (Not Equal = False) → Deve ignorar (próprio player)
- Se estiver invertido, troque as conexões do `Branch`

---

### **PROBLEMA 2: Spawn Falhando Silenciosamente**

**SE O LOG MOSTRAR `Actor válido: False`:**

**CORREÇÕES:**
1. **Validar Location antes de spawnar:**
   ```
   Not Equal (Vector): OutLocation != (0, 0, 0)
     ↓
   Branch: Location válida?
     ├─ True: SpawnActorFromClass
     └─ False: IGNORAR (não spawnar)
   ```

2. **Verificar collision:**
   - Tente spawnar em posição ligeiramente diferente (Y + 50 ou Z + 10)

3. **Verificar World Context:**
   - Certifique-se de que `SpawnActorFromClass` está usando o `World` correto

---

### **PROBLEMA 3: Arrays Não Sincronizados**

**SE O LOG MOSTRAR `RemoteActorIds Size != RemoteActors Size`:**

**CORREÇÃO:**
- Verifique se ambos os `Array_Add` estão sendo executados na ordem correta
- Verifique se não há nenhum `Array_Remove` ou `Array_Clear` sendo chamado

---

### **PROBLEMA 4: Actor Spawnado mas Não Visível**

**SE O LOG MOSTRAR `Actor válido: True` MAS NÃO APARECE NA TELA:**

**VERIFICAR:**
1. **Actor está sendo spawnado no nível correto?**
   - Verifique se está no nível do jogo (não no nível do editor)

2. **Actor está visível?**
   - Verifique se o mesh está configurado como visível
   - Verifique se não está sendo spawnado dentro de outro objeto

3. **Câmera está olhando para o lugar certo?**
   - Verifique se a câmera está na posição correta
   - Verifique se o actor está na área visível da câmera

---

## 📊 **RESUMO:**

**Como identificar qual cliente está tendo problema:**

1. **Todos os clientes compartilham o mesmo código**, então use `Get Active Player ID` para identificar nos logs
2. **Compare os logs `[MyID:0]` com `[MyID:1]`**
3. **O cliente que não está vendo o outro vai parar de logar em algum ponto**

**Compartilhe os logs de ambos os clientes quando um se move:**

**Cliente com `MyID: 0` deve mostrar:**
1. ✅ Frame está chegando? (`[MyID:0] OutPlayerId: 1`)
2. ✅ Filtro está correto? (`[MyID:0] Processar: True`)
3. ✅ Actor está sendo spawnado? (`[MyID:0] Actor válido: True`)
4. ✅ Arrays estão sendo atualizados? (`[MyID:0] Size: 1`)
5. ✅ Posição está sendo atualizada? (`[MyID:0] Location atualizada`)

**Cliente com `MyID: 1` deve mostrar:**
1. ✅ Frame está chegando? (`[MyID:1] OutPlayerId: 0`)
2. ✅ Filtro está correto? (`[MyID:1] Processar: True`)
3. ✅ Actor está sendo spawnado? (`[MyID:1] Actor válido: True`)
4. ✅ Arrays estão sendo atualizados? (`[MyID:1] Size: 1`)
5. ✅ Posição está sendo atualizada? (`[MyID:1] Location atualizada`)

**O primeiro log que mostrar `False` ou não aparecer para um dos clientes indica onde está o problema.**

