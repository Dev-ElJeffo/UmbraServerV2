# 🔍 **DIAGNÓSTICO: Múltiplos Spawns de RemoteActors**

## 📋 **ANÁLISE DOS LOGS:**

### **✅ O QUE ESTÁ FUNCIONANDO:**

1. **Filtro está correto:**
   - `Processar: falso` para próprio player (ignorado)
   - `Processar: verdadeiro` para outros players (processado)

2. **ProcessBinaryBuffer está funcionando:**
   - Frames de 25 bytes sendo aceitos corretamente
   - PlayerIDs corretos (14, 18, 4)

3. **EndPlay está funcionando:**
   - "EndPlay - Closing WebSocket..." aparece
   - "Actor Destroyed" aparece
   - "Cleanup complete!" aparece

### **❌ PROBLEMA IDENTIFICADO:**

**Não há logs sobre:**
- `Array_Find` (verificação se actor já existe)
- Spawn de actors
- `Array_Add` (adicionar IDs aos arrays)

**Isso indica que:**
1. `Array_Find` pode não estar sendo usado
2. Ou está retornando `-1` sempre (actor não encontrado)
3. Ou `Array_Add` não está sendo chamado após spawnar

---

## 🎯 **SOLUÇÃO: Adicionar Logs Detalhados em ProcessNextFrame**

### **LOGS NECESSÁRIOS:**

1. **Após `Array_Find`:**
   ```
   Print String: "Array_Find - OutPlayerId: {OutPlayerId}, FoundIndex: {FoundIndex}"
   ```

2. **Após `Branch` (FoundIndex >= 0):**
   - **Se True (actor existe):**
     ```
     Print String: "Actor já existe - atualizando (Index: {FoundIndex})"
     ```
   - **Se False (actor não existe):**
     ```
     Print String: "Actor não existe - spawnando novo (OutPlayerId: {OutPlayerId})"
     ```

3. **Após `SpawnActorFromClass`:**
   ```
   Print String: "Actor spawnado - OutPlayerId: {OutPlayerId}, Actor válido: {Is Valid}"
   ```

4. **Após `Array_Add` para `RemoteActorIds`:**
   ```
   Print String: "Array_Add - RemoteActorIds adicionado: {OutPlayerId}"
   ```

5. **Após `Array_Add` para `RemoteActors`:**
   ```
   Print String: "Array_Add - RemoteActors adicionado"
   ```

---

## 📝 **VERIFICAÇÕES NECESSÁRIAS:**

### **VERIFICAÇÃO 1: Array_Find Está Presente?**

**No Blueprint Editor:**
1. Abra `ProcessNextFrame`
2. Procure por `Array_Find` após o filtro (`OutPlayerId != Active Player ID`)
3. Verifique se está:
   - Buscando em `RemoteActorIds`
   - Com `Item To Find` = `OutPlayerId`
   - Retornando `FoundIndex` (Integer)

**Se não existir:**
- Adicione `Array_Find` após o filtro

---

### **VERIFICAÇÃO 2: Branch Após Array_Find**

**Verifique se há:**
1. `Greater or Equal` conectando `FoundIndex` e `0`
2. `Branch` verificando `FoundIndex >= 0`
3. **Pin `then` (True) conectado** para atualizar actor existente
4. **Pin `else` (False) conectado** para spawnar novo actor

---

### **VERIFICAÇÃO 3: Array_Add Após Spawn**

**Verifique se há:**
1. `Array_Add` para `RemoteActorIds` após `SpawnActorFromClass`
   - Deve adicionar `OutPlayerId`
2. `Array_Add` para `RemoteActors` após `SpawnActorFromClass`
   - Deve adicionar o actor spawnado

**Se não existir:**
- Adicione `Array_Add` para ambos os arrays após spawnar

---

## 🔧 **ESTRUTURA CORRETA ESPERADA:**

```
ProcessNextFrame
  ↓
[... filtros ...]
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Print String: "Array_Find - OutPlayerId: {OutPlayerId}, FoundIndex: {FoundIndex}" ← ADICIONAR!
  ↓
Greater or Equal: FoundIndex >= 0?
  ↓
Branch: FoundIndex >= 0?
  ├─ True (actor existe):
  │   ├─ Print String: "Actor já existe - atualizando (Index: {FoundIndex})" ← ADICIONAR!
  │   ├─ Get Array Item (RemoteActors, FoundIndex) → ExistingActor
  │   ├─ Set Variable: RemoteActorRef = ExistingActor
  │   └─ [CONTINUA PARA ATUALIZAÇÃO]
  │
  └─ False (actor não existe):
      ├─ Print String: "Actor não existe - spawnando novo (OutPlayerId: {OutPlayerId})" ← ADICIONAR!
      ├─ SpawnActorFromClass
      ├─ Print String: "Actor spawnado - OutPlayerId: {OutPlayerId}, Válido: {Is Valid}" ← ADICIONAR!
      ├─ Set Variable: RemoteActorRef = SpawnedActor
      ├─ Array_Add (RemoteActorIds, OutPlayerId) ← VERIFICAR!
      ├─ Print String: "Array_Add - RemoteActorIds adicionado: {OutPlayerId}" ← ADICIONAR!
      ├─ Array_Add (RemoteActors, SpawnedActor) ← VERIFICAR!
      ├─ Print String: "Array_Add - RemoteActors adicionado" ← ADICIONAR!
      └─ [CONTINUA PARA ATUALIZAÇÃO]
  ↓
[PONTO DE CONVERGÊNCIA]
  ↓
Set Actor Location (RemoteActorRef, OutLocation)
  ↓
Set Actor Rotation (RemoteActorRef, OutRotation)
```

---

## ✅ **CHECKLIST:**

### **ProcessNextFrame:**
- [ ] `Array_Find` presente após filtro
- [ ] `Greater or Equal` presente (`FoundIndex >= 0`)
- [ ] `Branch` presente após `Greater or Equal`
- [ ] Pin `then` (True) conectado (para atualizar actor existente)
- [ ] Pin `else` (False) conectado (para spawnar novo actor)
- [ ] `Get Array Item` presente no caminho `then` (True)
- [ ] `Array_Add` para `RemoteActorIds` presente no caminho `else` (False)
- [ ] `Array_Add` para `RemoteActors` presente no caminho `else` (False)
- [ ] Logs adicionados para debug

---

## 🧪 **TESTE COM LOGS:**

Após adicionar os logs, execute novamente e verifique:

1. **Se `FoundIndex` está sempre `-1`:**
   - Indica que `Array_Add` não está sendo chamado após spawnar
   - Ou que `RemoteActorIds` está sendo limpo incorretamente

2. **Se `FoundIndex` está sempre `>= 0` mas ainda spawna múltiplos:**
   - Indica que o pin `then` (True) não está conectado corretamente
   - Ou que `Get Array Item` não está sendo usado

3. **Se logs de `Array_Add` não aparecem:**
   - Indica que `Array_Add` não está sendo chamado após spawnar

---

## 📊 **PRÓXIMOS PASSOS:**

1. **Adicionar logs detalhados** em `ProcessNextFrame` conforme descrito acima
2. **Executar o teste** e coletar novos logs
3. **Analisar os logs** para identificar onde está o problema
4. **Enviar XML atualizado** de `ProcessNextFrame` para análise completa

---

**O problema de múltiplos spawns está relacionado à lógica de `ProcessNextFrame`. Os logs adicionais ajudarão a identificar exatamente onde está o problema.**

