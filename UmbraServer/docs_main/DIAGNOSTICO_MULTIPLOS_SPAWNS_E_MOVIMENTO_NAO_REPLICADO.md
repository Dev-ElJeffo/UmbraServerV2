# 🔍 **DIAGNÓSTICO: Múltiplos Spawns e Movimento Não Replicado**

## 📋 **PROBLEMAS REPORTADOS:**

### **Problema 1: Múltiplos Spawns**
- Quando um client spawna, já existem **2 instâncias de RemotePlayers** no mapa
- Isso não deveria acontecer

### **Problema 2: Movimento Não Replicado Assimetricamente**
- **Client 1 se move** → Ninguém vê
- **Client 2 se move** → Ninguém vê
- **Client 3 se move** → Apenas Client 4 vê
- **Client 4 se move** → Todos os outros (1, 2, 3) veem

**Padrão identificado:**
- Client 4 funciona corretamente (todos veem)
- Client 3 funciona parcialmente (apenas Client 4 vê)
- Clients 1 e 2 não funcionam (ninguém vê)

---

## 🔍 **ANÁLISE DOS PROBLEMAS:**

### **1. Múltiplos Spawns (2 instâncias)**

**Possíveis causas:**

#### **Causa A: `Array_Find` não está encontrando actors existentes**
- `Array_Find` pode estar retornando `-1` mesmo quando o actor já existe
- Isso faria com que o sistema pense que o actor não existe e spawne novamente

#### **Causa B: `Array_Add` está sendo executado múltiplas vezes**
- O actor pode estar sendo adicionado aos arrays várias vezes
- Isso pode causar problemas na busca subsequente

#### **Causa C: Race condition entre frames**
- Múltiplos frames podem chegar antes que `Array_Add` atualize o array
- Cada frame vê o array vazio e spawne um novo actor

#### **Causa D: `Get Array Item` não está sendo usado corretamente**
- O caminho `then` pode não estar recuperando o actor existente corretamente
- Isso pode fazer com que o sistema pense que precisa spawne um novo actor

---

### **2. Movimento Não Replicado**

**Possíveis causas:**

#### **Causa A: Filtro bloqueando frames incorretamente**
- O filtro pode estar bloqueando frames dos clients 1, 2, 3
- Client 4 funciona porque seu `Active Player ID` pode estar sendo usado corretamente

#### **Causa B: PlayerIDs incorretos ou inconsistentes**
- Clients 1, 2, 3 podem estar enviando `PlayerID` incorreto
- Ou o servidor pode estar enviando `PlayerID` incorreto para esses clients

#### **Causa C: Atualização de actors existentes não está funcionando**
- O caminho `then` (actor existe) pode não estar atualizando corretamente
- Isso explicaria por que apenas Client 4 funciona (talvez seja o único que está sendo spawnado corretamente)

#### **Causa D: `Array_Find` retornando índice incorreto**
- `FoundIndex` pode estar retornando um índice incorreto
- Isso faria com que `Get Array Item` recupere o actor errado

---

## 🎯 **VERIFICAÇÕES NECESSÁRIAS:**

### **Verificação 1: `Array_Find` está funcionando corretamente?**

**O que verificar:**
1. `Array_Find` está sendo executado antes de `SpawnActorFromClass`?
2. `FoundIndex` está sendo logado corretamente?
3. `FoundIndex >= 0` está sendo verificado corretamente?

**Logs necessários:**
```cpp
// Adicionar após Array_Find:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Array_Find - OutPlayerId: %d, FoundIndex: %d"), OutPlayerId, FoundIndex);
```

---

### **Verificação 2: `Array_Add` está sendo executado apenas uma vez?**

**O que verificar:**
1. `Array_Add` está sendo executado apenas no caminho `else` (spawn)?
2. `Array_Add` não está sendo executado no caminho `then` (atualização)?

**Logs necessários:**
```cpp
// Adicionar antes de cada Array_Add:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Array_Add - RemoteActorIds, OutPlayerId: %d"), OutPlayerId);
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Array_Add - RemoteActors, RemoteActorRef: %p"), RemoteActorRef);
```

---

### **Verificação 3: `Get Array Item` está recuperando o actor correto?**

**O que verificar:**
1. `Get Array Item` está usando o `FoundIndex` correto?
2. O actor retornado é válido?
3. O actor retornado corresponde ao `OutPlayerId` esperado?

**Logs necessários:**
```cpp
// Adicionar após Get Array Item:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Get Array Item - FoundIndex: %d, Actor: %p"), FoundIndex, ExistingActorRef);
```

---

### **Verificação 4: Filtro está funcionando corretamente para todos os clients?**

**O que verificar:**
1. `Active Player ID` está sendo lido corretamente para cada client?
2. O filtro está bloqueando frames do próprio client corretamente?
3. O filtro está permitindo frames de outros clients corretamente?

**Logs necessários:**
```cpp
// Adicionar após filtro:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Filtro - [MyID:%d] OutPlayerId: %d, Processar: %s"), 
       ActivePlayerID, OutPlayerId, bShouldProcess ? TEXT("true") : TEXT("false"));
```

---

### **Verificação 5: `Set Actor Location` está sendo executado?**

**O que verificar:**
1. `Set Actor Location` está sendo executado para actors existentes?
2. `Set Actor Location` está recebendo o `RemoteActorRef` correto?
3. `OutLocation` está sendo passado corretamente?

**Logs necessários:**
```cpp
// Adicionar antes de Set Actor Location:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Set Actor Location - RemoteActorRef: %p, OutLocation: (%f, %f, %f)"), 
       RemoteActorRef, OutLocation.X, OutLocation.Y, OutLocation.Z);
```

---

## 🔧 **CORREÇÕES PROPOSTAS:**

### **Correção 1: Adicionar Validação antes de `Array_Add`**

**Problema:** `Array_Add` pode estar sendo executado múltiplas vezes

**Solução:**
1. Verificar se `OutPlayerId` já existe em `RemoteActorIds` antes de adicionar
2. Ou usar `Array_Find` antes de `Array_Add` para garantir que não está duplicando

**Implementação:**
```
Antes de Array_Add (RemoteActorIds, OutPlayerId):
  ├─ Array_Find (RemoteActorIds, OutPlayerId) → CheckIndex
  ├─ Branch: CheckIndex < 0? (não existe)
  │   ├─ then (True): Executar Array_Add
  │   └─ else (False): Pular Array_Add (já existe)
```

---

### **Correção 2: Adicionar Validação `Is Valid` antes de Atualizar**

**Problema:** `Get Array Item` pode retornar um actor inválido

**Solução:**
1. Adicionar `Is Valid` após `Get Array Item`
2. Se inválido, spawne um novo actor

**Implementação:**
```
Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  ↓
Is Valid (ExistingActorRef)
  ↓
Branch: Is Valid?
  ├─ then (True): Set Variable: RemoteActorRef = ExistingActorRef
  │                 ↓
  │                 Set Actor Location/Rotation
  └─ else (False): [Desconectar Array_Add e spawne novo actor]
```

---

### **Correção 3: Adicionar Logs Detalhados**

**Problema:** Difícil diagnosticar sem logs

**Solução:**
1. Adicionar logs em todos os pontos críticos
2. Incluir `PlayerID`, `FoundIndex`, `RemoteActorRef`, etc.

---

### **Correção 4: Verificar Sincronização de Arrays**

**Problema:** Race condition entre frames

**Solução:**
1. Garantir que `Array_Add` atualiza os arrays antes que o próximo frame seja processado
2. Ou usar uma estrutura de dados thread-safe

---

## 📊 **CHECKLIST DE DIAGNÓSTICO:**

### **Para o Problema de Múltiplos Spawns:**
- [ ] `Array_Find` está retornando `-1` quando deveria retornar índice válido?
- [ ] `Array_Add` está sendo executado múltiplas vezes?
- [ ] `Get Array Item` está recuperando o actor correto?
- [ ] Os arrays `RemoteActorIds` e `RemoteActors` estão sincronizados?

### **Para o Problema de Movimento Não Replicado:**
- [ ] Filtro está bloqueando frames incorretamente?
- [ ] `Active Player ID` está correto para cada client?
- [ ] `Set Actor Location` está sendo executado para actors existentes?
- [ ] `RemoteActorRef` está correto quando `Set Actor Location` é executado?
- [ ] `OutLocation` está sendo passado corretamente?

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Aguardar estrutura atual da lógica** para análise detalhada
2. **Adicionar logs detalhados** em todos os pontos críticos
3. **Verificar se `Array_Find` está funcionando corretamente**
4. **Verificar se `Array_Add` está sendo executado apenas uma vez**
5. **Verificar se `Get Array Item` está recuperando o actor correto**
6. **Verificar se o filtro está funcionando corretamente para todos os clients**

---

## 📝 **NOTAS ADICIONAIS:**

### **Sobre o Padrão Assimétrico:**
- O fato de que apenas Client 4 funciona e Client 3 funciona parcialmente sugere que pode haver um problema com PlayerIDs ou com a ordem de spawn
- Pode ser que Client 4 seja o último a spawnar e, portanto, não tenha problemas de sincronização
- Pode ser que Client 4 tenha um PlayerID diferente que não está sendo filtrado incorretamente

### **Sobre os Múltiplos Spawns:**
- 2 instâncias podem indicar que o spawn está sendo executado duas vezes:
  - Uma vez no caminho `else` (quando `FoundIndex < 0`)
  - Outra vez em algum outro lugar (talvez no caminho `then` incorretamente conectado)

---

**Fim do Documento**

