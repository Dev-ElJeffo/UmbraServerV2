# 🔬 **HIPÓTESES ESPECÍFICAS: Múltiplos Spawns e Movimento Não Replicado**

## 📋 **ANÁLISE BASEADA NOS SINTOMAS:**

### **Sintoma 1: 2 Instâncias de RemotePlayers ao Spawnar**

**Hipóteses:**

#### **Hipótese A: `Array_Add` está sendo executado duas vezes**
- Uma vez no caminho `else` (spawn)
- Outra vez em algum lugar incorreto (talvez no caminho `then` ou após `Get Array Item`)

**Como verificar:**
- Adicionar log antes de cada `Array_Add`
- Verificar se o log aparece duas vezes para o mesmo `OutPlayerId`

#### **Hipótese B: `Array_Find` não está encontrando o actor recém-spawnado**
- O actor é spawnado e adicionado aos arrays
- Mas o próximo frame não encontra o actor porque `Array_Find` retorna `-1`
- Isso causa um segundo spawn

**Como verificar:**
- Adicionar log após `Array_Find` mostrando `FoundIndex`
- Verificar se `FoundIndex` é `-1` mesmo quando o actor deveria existir

#### **Hipótese C: Ambos os caminhos (`then` e `else`) estão executando spawn**
- O caminho `then` pode estar incorretamente conectado ao `SpawnActorFromClass`
- Isso causaria spawn tanto quando o actor existe quanto quando não existe

**Como verificar:**
- Verificar se o caminho `then` está conectado apenas a `Get Array Item`, não a `SpawnActorFromClass`

---

### **Sintoma 2: Movimento Não Replicado Assimetricamente**

**Padrão observado:**
- Client 1: Ninguém vê movimento
- Client 2: Ninguém vê movimento
- Client 3: Apenas Client 4 vê movimento
- Client 4: Todos veem movimento

**Análise do padrão:**
- Client 4 funciona completamente (todos veem)
- Client 3 funciona parcialmente (apenas Client 4 vê)
- Clients 1 e 2 não funcionam (ninguém vê)

**Hipóteses:**

#### **Hipótese A: Problema com PlayerIDs ou Filtro**
- Clients 1, 2, 3 podem estar enviando `PlayerID` incorreto
- Ou o filtro pode estar bloqueando frames desses clients incorretamente
- Client 4 pode ter um `PlayerID` diferente que não está sendo filtrado

**Como verificar:**
- Adicionar log no filtro mostrando `Active Player ID` e `OutPlayerId` para cada client
- Verificar se o filtro está bloqueando frames incorretamente

#### **Hipótese B: Problema com Atualização de Actors Existentes**
- O caminho `then` (actor existe) pode não estar funcionando corretamente para clients 1, 2, 3
- Client 4 pode estar funcionando porque é o último a spawnar ou tem um `PlayerID` diferente
- Ou client 4 pode estar sendo atualizado corretamente enquanto 1, 2, 3 não

**Como verificar:**
- Adicionar log após `Get Array Item` mostrando qual actor está sendo recuperado
- Adicionar log antes de `Set Actor Location` mostrando qual actor está sendo atualizado

#### **Hipótese C: Problema com Ordem de Spawn/Processamento**
- Clients 1, 2, 3 podem estar spawnando antes que os arrays estejam prontos
- Client 4 pode estar spawnando depois que tudo está funcionando
- Isso pode causar problemas de sincronização

**Como verificar:**
- Adicionar log mostrando a ordem de spawn de cada client
- Verificar se há uma relação entre ordem de spawn e funcionamento

#### **Hipótese D: `Array_Find` retornando índice incorreto**
- `FoundIndex` pode estar retornando um índice incorreto para clients 1, 2, 3
- Isso faria com que `Get Array Item` recupere o actor errado
- Client 4 pode estar funcionando porque seu índice está correto

**Como verificar:**
- Adicionar log após `Array_Find` mostrando `FoundIndex` e `OutPlayerId`
- Verificar se `FoundIndex` corresponde ao índice correto no array

---

## 🎯 **TESTES SUGERIDOS:**

### **Teste 1: Verificar Múltiplos Spawns**

**Adicionar logs:**
```cpp
// Antes de Array_Add:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] ANTES Array_Add - OutPlayerId: %d, RemoteActorIds.Num(): %d"), 
       OutPlayerId, RemoteActorIds.Num());

// Após Array_Add:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] APÓS Array_Add - OutPlayerId: %d, RemoteActorIds.Num(): %d"), 
       OutPlayerId, RemoteActorIds.Num());

// Após Array_Find:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Array_Find - OutPlayerId: %d, FoundIndex: %d, RemoteActorIds.Num(): %d"), 
       OutPlayerId, FoundIndex, RemoteActorIds.Num());
```

**O que procurar:**
- Se `FoundIndex` é `-1` mesmo quando o actor deveria existir
- Se `Array_Add` está sendo executado múltiplas vezes para o mesmo `OutPlayerId`
- Se `RemoteActorIds.Num()` aumenta mais do que deveria

---

### **Teste 2: Verificar Movimento Não Replicado**

**Adicionar logs:**
```cpp
// Após filtro:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Filtro - [MyID:%d] OutPlayerId: %d, Processar: %s"), 
       ActivePlayerID, OutPlayerId, bShouldProcess ? TEXT("true") : TEXT("false"));

// Após Get Array Item:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Get Array Item - FoundIndex: %d, Actor: %p, OutPlayerId: %d"), 
       FoundIndex, ExistingActorRef, OutPlayerId);

// Antes de Set Actor Location:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Set Actor Location - RemoteActorRef: %p, OutLocation: (%f, %f, %f), OutPlayerId: %d"), 
       RemoteActorRef, OutLocation.X, OutLocation.Y, OutLocation.Z, OutPlayerId);
```

**O que procurar:**
- Se o filtro está bloqueando frames incorretamente para clients 1, 2, 3
- Se `Get Array Item` está recuperando o actor correto
- Se `Set Actor Location` está sendo executado para clients 1, 2, 3

---

### **Teste 3: Verificar Sincronização de Arrays**

**Adicionar logs:**
```cpp
// No início de ProcessNextFrame:
UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] INÍCIO - RemoteActorIds.Num(): %d, RemoteActors.Num(): %d"), 
       RemoteActorIds.Num(), RemoteActors.Num());

// Verificar se arrays estão sincronizados:
for (int32 i = 0; i < RemoteActorIds.Num(); i++)
{
    UE_LOG(LogTemp, Warning, TEXT("[ProcessNextFrame] Array[%d] - PlayerID: %d, Actor: %p"), 
           i, RemoteActorIds[i], RemoteActors.IsValidIndex(i) ? RemoteActors[i] : nullptr);
}
```

**O que procurar:**
- Se `RemoteActorIds.Num()` é igual a `RemoteActors.Num()`
- Se há duplicatas em `RemoteActorIds`
- Se há `nullptr` em `RemoteActors`

---

## 🔧 **CORREÇÕES PROPOSTAS:**

### **Correção 1: Prevenir Múltiplos Spawns**

**Adicionar validação antes de `Array_Add`:**
```
Antes de Array_Add (RemoteActorIds, OutPlayerId):
  ├─ Array_Find (RemoteActorIds, OutPlayerId) → CheckIndex
  ├─ Branch: CheckIndex < 0? (não existe)
  │   ├─ then (True): Executar Array_Add
  │   └─ else (False): Log de erro e pular Array_Add
```

---

### **Correção 2: Garantir que `Get Array Item` recupera o actor correto**

**Adicionar validação após `Get Array Item`:**
```
Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  ↓
Is Valid (ExistingActorRef)
  ↓
Branch: Is Valid?
  ├─ then (True): Continuar com atualização
  └─ else (False): Log de erro e spawne novo actor
```

---

### **Correção 3: Adicionar logs detalhados em todos os pontos críticos**

**Pontos críticos:**
1. Após `Array_Find`
2. Antes e após `Array_Add`
3. Após `Get Array Item`
4. Antes de `Set Actor Location`
5. No filtro (antes e depois do branch)

---

## 📊 **CHECKLIST DE VERIFICAÇÃO:**

### **Para Múltiplos Spawns:**
- [ ] `Array_Find` está retornando `-1` quando deveria retornar índice válido?
- [ ] `Array_Add` está sendo executado múltiplas vezes?
- [ ] Ambos os caminhos (`then` e `else`) estão executando spawn?
- [ ] Os arrays `RemoteActorIds` e `RemoteActors` estão sincronizados?

### **Para Movimento Não Replicado:**
- [ ] Filtro está bloqueando frames incorretamente para clients 1, 2, 3?
- [ ] `Active Player ID` está correto para cada client?
- [ ] `Get Array Item` está recuperando o actor correto?
- [ ] `Set Actor Location` está sendo executado para clients 1, 2, 3?
- [ ] `RemoteActorRef` está correto quando `Set Actor Location` é executado?

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Aguardar estrutura atual da lógica** para análise detalhada
2. **Adicionar logs detalhados** conforme sugerido acima
3. **Executar testes** com 4 clients e coletar logs
4. **Analisar logs** para identificar padrões
5. **Aplicar correções** baseadas nos resultados

---

**Fim do Documento**

