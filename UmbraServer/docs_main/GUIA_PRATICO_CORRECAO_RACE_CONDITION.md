# 🔧 **GUIA PRÁTICO: Correção de Condição de Corrida - Múltiplos Spawns**

## 📋 **PROBLEMA IDENTIFICADO:**

Mesmo com a estrutura correta, múltiplos spawns ocorrem devido a uma **condição de corrida**:
- Múltiplos frames do mesmo PlayerID chegam antes que o `Array_Add` atualize o array
- `Array_Find` retorna `-1` (não encontrado) para todos os frames simultâneos
- Cada frame spawna um novo actor, causando duplicatas

---

## 🎯 **SOLUÇÃO: Verificação Dupla Antes de Spawnar**

### **PASSO 1: Modificar o Caminho `else` de `K2Node_IfThenElse_6`**

**Localização:**
- Abra `BP_NetMovementClient` → `ProcessNextFrame`
- Localize o pin `else` de `K2Node_IfThenElse_6` (quando `FoundIndex < 0`)

**Estrutura Atual:**
```
K2Node_IfThenElse_6 (else)
  ↓
SpawnActorFromClass
  ↓
Set Variable: RemoteActorRef = SpawnedActor
  ↓
Array_Add (RemoteActorIds, OutPlayerId)
  ↓
Array_Add (RemoteActors, RemoteActorRef)
```

---

### **PASSO 2: Adicionar Verificação Dupla ANTES de SpawnActorFromClass**

**Modificação:**

1. **Após o pin `else` de `K2Node_IfThenElse_6`, adicione uma verificação dupla:**

   ```
   K2Node_IfThenElse_6 (else)
     ↓
   [VERIFICAÇÃO DUPLA]
   Array_Find (RemoteActorIds, OutPlayerId) → FoundIndexCheck
     ↓
   Greater or Equal: FoundIndexCheck >= 0?
     ↓
   Branch: FoundIndexCheck >= 0?
     ├─ True (actor JÁ existe): [PULAR SPAWN]
     │                          Get Array Item (RemoteActors, FoundIndexCheck)
     │                          ↓
     │                          Set Variable: RemoteActorRef = ExistingActorRef
     │                          ↓
     │                          Set Actor Location/Rotation
     │
     └─ False (actor NÃO existe): [CONTINUAR COM SPAWN]
                                 SpawnActorFromClass
                                 ↓
                                 Set Variable: RemoteActorRef = SpawnedActor
                                 ↓
                                 [VERIFICAÇÃO ANTES DE ADICIONAR]
                                 Array_Find (RemoteActorIds, OutPlayerId) → CheckIndex
                                 ↓
                                 Branch: CheckIndex < 0? (verificar ANTES de adicionar)
                                 ├─ True: Array_Add (RemoteActorIds, OutPlayerId)
                                 │          ↓
                                 │          Array_Add (RemoteActors, RemoteActorRef)
                                 └─ False: [Já existe - log de erro]
                                           Print String: "Warning: Actor já existe mas FoundIndex era -1!"
   ```

---

### **PASSO 3: Implementação Passo a Passo**

#### **3.1. Adicionar Primeiro Array_Find (Verificação Dupla)**

1. **Após o pin `else` de `K2Node_IfThenElse_6`:**
   - Clique direito → `Array_Find`
   - Configure:
     - **Array**: `RemoteActorIds` (Get Variable)
     - **Item To Find**: `OutPlayerId` (do `ParseStateUpdateFrame`)
   - Saída: `FoundIndexCheck` (Integer)

2. **Adicionar `Greater or Equal`:**
   - Clique direito → `Greater or Equal (Integer)`
   - Configure:
     - **A**: `FoundIndexCheck` (do `Array_Find` acima)
     - **B**: `0` (constante)

3. **Adicionar `Branch`:**
   - Clique direito → `Branch`
   - Configure:
     - **Condition**: `ReturnValue` do `Greater or Equal`

#### **3.2. Conectar o Caminho `True` (Actor Já Existe)**

1. **Do pin `True` do `Branch`:**
   - Conecte a `Get Array Item`:
     - **Array**: `RemoteActors` (Get Variable)
     - **Index**: `FoundIndexCheck` (do primeiro `Array_Find`)
   - Conecte `Array Element` a `Set Variable: RemoteActorRef`
   - Conecte `Set Variable` a `Set Actor Location` e `Set Actor Rotation`

#### **3.3. Conectar o Caminho `False` (Actor Não Existe - Spawnar)**

1. **Do pin `False` do `Branch`:**
   - Conecte a `SpawnActorFromClass` (já existente)
   - Conecte `ReturnValue` do `SpawnActorFromClass` a `Set Variable: RemoteActorRef`

2. **Antes de `Array_Add`, adicione segunda verificação:**
   - Após `Set Variable: RemoteActorRef`, adicione outro `Array_Find`:
     - **Array**: `RemoteActorIds` (Get Variable)
     - **Item To Find**: `OutPlayerId` (do `ParseStateUpdateFrame`)
     - Saída: `CheckIndex` (Integer)

3. **Adicionar `Branch` antes de `Array_Add`:**
   - Clique direito → `Branch`
   - Configure:
     - **Condition**: `CheckIndex < 0` (usar `Less` ou `Not Equal`)

4. **Conectar `Array_Add` apenas no caminho `True`:**
   - Do pin `True` do `Branch`:
     - Conecte a `Array_Add (RemoteActorIds, OutPlayerId)`
     - Conecte a `Array_Add (RemoteActors, RemoteActorRef)`

5. **No caminho `False`, adicionar log de erro:**
   - Do pin `False` do `Branch`:
     - Adicione `Print String`: `"Warning: Actor já existe mas FoundIndex era -1!"`

---

## ✅ **ESTRUTURA CORRETA FINAL:**

### **Fluxo Completo:**

```
ProcessNextFrame
  ↓
ParseStateUpdateFrame
  ↓
[Filtros...]
  ↓
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal: FoundIndex >= 0?
  ↓
K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)
  ├─ then (True): [ATOR EXISTE]
  │                 Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │                 ↓
  │                 Is Valid (ExistingActorRef)
  │                 ↓
  │                 Branch: Is Valid?
  │                 ├─ True: Set Variable: RemoteActorRef = ExistingActorRef
  │                 │          ↓
  │                 │          Set Actor Location/Rotation
  │                 └─ False: [Tratar como não encontrado]
  │
  └─ else (False): [VERIFICAÇÃO DUPLA]
                    Array_Find (RemoteActorIds, OutPlayerId) → FoundIndexCheck
                    ↓
                    Greater or Equal: FoundIndexCheck >= 0?
                    ↓
                    Branch: FoundIndexCheck >= 0?
                    ├─ True (actor JÁ existe): [PULAR SPAWN]
                    │                          Get Array Item (RemoteActors, FoundIndexCheck)
                    │                          ↓
                    │                          Set Variable: RemoteActorRef = ExistingActorRef
                    │                          ↓
                    │                          Set Actor Location/Rotation
                    │
                    └─ False (actor NÃO existe): [CONTINUAR COM SPAWN]
                                                  SpawnActorFromClass
                                                  ↓
                                                  Set Variable: RemoteActorRef = SpawnedActor
                                                  ↓
                                                  [VERIFICAÇÃO ANTES DE ADICIONAR]
                                                  Array_Find (RemoteActorIds, OutPlayerId) → CheckIndex
                                                  ↓
                                                  Branch: CheckIndex < 0?
                                                  ├─ True: Array_Add (RemoteActorIds, OutPlayerId)
                                                  │          ↓
                                                  │          Array_Add (RemoteActors, RemoteActorRef)
                                                  └─ False: [Já existe - log de erro]
                                                            Print String: "Warning: Actor já existe!"
  ↓
[CONTINUAÇÃO] Make Array (0 Elements)
  ↓
ProcessBinaryBuffer (Get BinaryMessageBuffer, Make Array vazio, Get OutFrame)
  ↓ (ReturnValue == true)
ProcessNextFrame (recursão)
```

---

## 🔍 **VERIFICAÇÃO DO BUFFER:**

### **Adicionar Log para Verificar Limpeza do Buffer:**

Após chamar `ProcessBinaryBuffer`, adicionar:

1. **Após `ProcessBinaryBuffer` (o segundo, no final de `ProcessNextFrame`):**
   - Adicione `Get Array Length` do `BinaryMessageBuffer`
   - Conecte a `Format Text`: `"Buffer após processar: {0} bytes"`
   - Conecte a `Print String`

**Se o buffer está sendo limpo corretamente:**
- Após processar um frame de 25 bytes, o buffer deve ter **25 bytes a menos**
- Se o buffer tinha 50 bytes e processou 25, deve ter **25 bytes restantes**

---

## ✅ **CHECKLIST:**

### **Verificações Necessárias:**

- [ ] Verificação dupla (`Array_Find`) adicionada antes de `SpawnActorFromClass`?
- [ ] `Branch` após verificação dupla está configurado corretamente?
- [ ] Caminho `True` (actor já existe) conectado a `Get Array Item`?
- [ ] Caminho `False` (actor não existe) conectado a `SpawnActorFromClass`?
- [ ] Verificação antes de `Array_Add` adicionada?
- [ ] `Array_Add` conectado apenas no caminho `True` da verificação?
- [ ] Log de erro adicionado no caminho `False` da verificação antes de `Array_Add`?
- [ ] Log para verificar limpeza do buffer adicionado?

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Aplicar modificações** no Blueprint Editor conforme descrito acima
2. **Compilar e testar** com múltiplos clients simultâneos
3. **Verificar logs** para confirmar que não há mais múltiplos spawns
4. **Verificar buffer** para confirmar que está sendo limpo corretamente

---

## 📝 **NOTAS:**

- A verificação dupla garante que mesmo se múltiplos frames chegarem simultaneamente, apenas um spawn será executado
- A verificação antes de `Array_Add` evita adicionar duplicatas mesmo se houver uma condição de corrida
- O buffer está sendo limpo corretamente no C++, mas os logs ajudarão a confirmar

---

**Fim do Documento**

