# 🔍 **ANÁLISE COMPLETA: Event Tick e ProcessNextFrame XMLs**

## 📋 **PROBLEMAS REPORTADOS:**

### **Problema 1: Múltiplos Spawns Iniciais**
- Ao spawnar um client, **2 instâncias de RemotePlayers** aparecem no mapa
- Não deveria acontecer

### **Problema 2: Movimento Não Replicado (Assimétrico)**
- **Client 1** se move → Ninguém vê
- **Client 2** se move → Ninguém vê
- **Client 3** se move → Apenas Client 4 vê
- **Client 4** se move → Todos (1, 2, 3) veem

### **Problema 3: Após Últimas Alterações**
- **Nenhum client** está passando movimento para outro

---

## 🔍 **ANÁLISE DO EVENT TICK:**

### **Estrutura Esperada:**
```
Event Tick
  ↓
ProcessBinaryBuffer (Get BinaryMessageBuffer, NewData)
  ↓ (ReturnValue == true)
Get OutFrame
  ↓
ParseStateUpdateFrame (OutFrame)
  ↓ (ReturnValue == true)
ProcessNextFrame
```

### **Pontos Críticos a Verificar:**

#### **1. Múltiplas Chamadas para ProcessNextFrame**
- ⚠️ **PROBLEMA POTENCIAL**: Se `Event Tick` está chamando `ProcessNextFrame` múltiplas vezes por frame
- ⚠️ **PROBLEMA POTENCIAL**: Se `ProcessBinaryBuffer` está retornando `true` múltiplas vezes para o mesmo buffer

#### **2. Recursão Infinita**
- ⚠️ **PROBLEMA POTENCIAL**: Se `ProcessNextFrame` está chamando a si mesmo recursivamente sem controle adequado
- ⚠️ **PROBLEMA POTENCIAL**: Se o buffer não está sendo limpo corretamente após processar cada frame

#### **3. Array Vazio no Segundo ProcessBinaryBuffer**
- ⚠️ **PROBLEMA POTENCIAL**: Se `ProcessNextFrame` não está passando um array vazio (`Make Array` com 0 elementos) para o segundo `ProcessBinaryBuffer`
- Isso causaria processamento duplicado do mesmo frame

---

## 🔍 **ANÁLISE DO PROCESSNEXTFRAME:**

### **Estrutura Esperada:**
```
ProcessNextFrame
  ↓
ParseStateUpdateFrame (Get OutFrame)
  ↓ (ReturnValue == true)
Branch: Parse OK?
  ↓ (then = true)
[FILTRO] Get Active Player ID
  ↓
Not Equal: OutPlayerId != Active Player ID?
  ↓
Branch: OutPlayerId != Active Player ID?
  ├─ True: CONTINUA (é outro player)
  └─ False: IGNORA E PARA (é o próprio player)
  ↓ (True)
[VALIDAÇÃO OutLocation] Not Equal: OutLocation != (0,0,0)?
  ↓
Branch: OutLocation != (0,0,0)?
  ├─ True: CONTINUA
  └─ False: IGNORA E PARA
  ↓ (True)
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal: FoundIndex >= 0?
  ↓
Branch: FoundIndex >= 0?
  ├─ then (True): [ATOR EXISTE] Get Array Item → Set Variable → Update Location/Rotation
  └─ else (False): [NOVO ATOR] SpawnActorFromClass → Array_Add → Update Location/Rotation
  ↓
[CONTINUAÇÃO] ProcessBinaryBuffer (array vazio) → Recursão se houver mais frames
```

---

## 🚨 **PROBLEMAS IDENTIFICADOS NO XML:**

### **PROBLEMA CRÍTICO 1: Pin `then` de `K2Node_IfThenElse_6` Conectado Incorretamente**

**O que deveria acontecer:**
- **Caminho `then` (FoundIndex >= 0)**: Actor existe → `Get Array Item` → `Set Variable: RemoteActorRef` → `Set Actor Location/Rotation`
- **Caminho `else` (FoundIndex < 0)**: Actor não existe → `SpawnActorFromClass` → `Array_Add` (ambos arrays) → `Set Actor Location/Rotation`

**Problema identificado:**
- O pin `then` está conectado a `Set Variable: RemoteActorRef` e depois a `Array_Add`
- Isso significa que **mesmo quando o actor existe**, o sistema está tentando adicionar ao array novamente
- Isso causa **duplicatas** nos arrays e pode causar múltiplos spawns

**Correção necessária:**
```
K2Node_IfThenElse_6
  ├─ then (FoundIndex >= 0): Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │                           ↓
  │                           Set Variable: RemoteActorRef = ExistingActorRef
  │                           ↓
  │                           Set Actor Location/Rotation
  │                           ↓
  │                           [NÃO EXECUTAR Array_Add!]
  │
  └─ else (FoundIndex < 0): SpawnActorFromClass
                            ↓
                            Array_Add (RemoteActorIds, OutPlayerId)
                            ↓
                            Array_Add (RemoteActors, RemoteActorRef)
                            ↓
                            Set Actor Location/Rotation
```

---

### **PROBLEMA CRÍTICO 2: Array_Add no Caminho `then`**

**Evidência:**
- O XML mostra que `Array_Add` está sendo executado no caminho `then` (quando o actor já existe)
- Isso causa:
  - **Duplicatas** em `RemoteActorIds` e `RemoteActors`
  - **Múltiplos spawns** porque `Array_Find` pode não encontrar o actor corretamente se houver duplicatas
  - **Sincronização incorreta** entre os arrays

**Correção necessária:**
- **Remover `Array_Add` do caminho `then`**
- `Array_Add` deve ser executado **apenas** no caminho `else` (spawn)

---

### **PROBLEMA CRÍTICO 3: Get Array Item Pode Não Estar Presente**

**Evidência:**
- O XML pode não mostrar `Get Array Item` no caminho `then`
- Sem `Get Array Item`, o sistema não recupera o actor existente
- Isso causaria tentativas de atualizar um actor que não foi recuperado corretamente

**Correção necessária:**
- **Adicionar `Get Array Item` no caminho `then`**
- `Get Array Item` deve usar `FoundIndex` para recuperar o actor de `RemoteActors`
- O actor recuperado deve ser atribuído a `RemoteActorRef` usando `Set Variable`

---

### **PROBLEMA CRÍTICO 4: Validação de Actor Após Get Array Item**

**Problema potencial:**
- `Get Array Item` pode retornar um actor inválido (`nullptr`)
- Se o actor for inválido, `Set Actor Location` falhará silenciosamente
- Isso explicaria por que alguns clients não veem movimento

**Correção necessária:**
- **Adicionar `Is Valid` após `Get Array Item`**
- Se inválido, tratar como actor não encontrado e spawne um novo

---

### **PROBLEMA CRÍTICO 5: Filtro Pode Estar Bloqueando Incorretamente**

**Problema potencial:**
- O filtro pode estar usando `MyPlayerId` em vez de `Get Active Player ID`
- `MyPlayerId` pode estar desatualizado, causando bloqueio incorreto
- Isso explicaria por que alguns clients não veem movimento

**Verificação necessária:**
- Confirmar que o filtro usa `Get Active Player ID` diretamente
- Confirmar que o filtro está logando corretamente (`[MyID:X] Filtro - Active: X, Out: Y, Processar: true/false`)

---

### **PROBLEMA CRÍTICO 6: ProcessBinaryBuffer no Final de ProcessNextFrame**

**Problema potencial:**
- Se `ProcessNextFrame` não está passando um array vazio para o segundo `ProcessBinaryBuffer`
- O mesmo frame pode ser processado múltiplas vezes
- Isso causaria múltiplos spawns do mesmo actor

**Verificação necessária:**
- Confirmar que `Make Array` com **0 elementos** está sendo usado
- Confirmar que `NewData` do segundo `ProcessBinaryBuffer` está conectado ao `Make Array vazio`

---

## 🔧 **CORREÇÕES NECESSÁRIAS:**

### **CORREÇÃO 1: Remover Array_Add do Caminho `then`**

**Ação:**
1. No Blueprint Editor, localize `K2Node_IfThenElse_6`
2. Siga o pin `then` (FoundIndex >= 0)
3. **DESCONECTE `Array_Add` do caminho `then`**
4. `Array_Add` deve estar **apenas** no caminho `else` (spawn)

---

### **CORREÇÃO 2: Adicionar Get Array Item no Caminho `then`**

**Ação:**
1. No caminho `then` de `K2Node_IfThenElse_6`, adicione `Get Array Item`
2. Configure:
   - **Array**: `RemoteActors`
   - **Index**: `FoundIndex` (do `Array_Find`)
3. Conecte a saída `ExistingActorRef` a `Set Variable: RemoteActorRef`
4. Conecte `Set Variable` a `Set Actor Location/Rotation`
5. **NÃO conecte `Array_Add` após `Set Variable`**

---

### **CORREÇÃO 3: Adicionar Validação Is Valid**

**Ação:**
1. Após `Get Array Item`, adicione `Is Valid`
2. Conecte `ExistingActorRef` a `Is Valid`
3. Adicione um `Branch` após `Is Valid`
4. **Caminho `True`**: Continue com `Set Variable` e atualização
5. **Caminho `False`**: Trate como actor não encontrado e spawne um novo (caminho `else`)

---

### **CORREÇÃO 4: Verificar Filtro**

**Ação:**
1. Confirme que o filtro usa `Get Active Player ID` diretamente
2. Confirme que o log do filtro está posicionado **antes** do `Branch`
3. Adicione logs detalhados:
   ```
   [MyID:{0}] Filtro - Active: {1}, Out: {2}, Processar: {3}
   ```

---

### **CORREÇÃO 5: Verificar Array Vazio no Segundo ProcessBinaryBuffer**

**Ação:**
1. No final de `ProcessNextFrame`, localize o segundo `ProcessBinaryBuffer`
2. Confirme que `NewData` está conectado a um `Make Array` com **0 elementos**
3. Se não estiver, corrija para usar `Make Array` vazio

---

## 📊 **CHECKLIST DE VERIFICAÇÃO:**

### **Para Múltiplos Spawns:**
- [ ] `Array_Add` está sendo executado apenas no caminho `else` (spawn)?
- [ ] `Array_Add` **NÃO** está sendo executado no caminho `then` (atualização)?
- [ ] `Get Array Item` está presente no caminho `then`?
- [ ] `Is Valid` está verificando o actor após `Get Array Item`?
- [ ] `Array_Find` está funcionando corretamente (retornando índice válido quando o actor existe)?

### **Para Movimento Não Replicado:**
- [ ] Filtro está usando `Get Active Player ID` diretamente?
- [ ] Filtro está logando corretamente (`[MyID:X] Filtro - Active: X, Out: Y, Processar: true/false`)?
- [ ] `Set Actor Location` está sendo executado no caminho `then` (ator existe)?
- [ ] `RemoteActorRef` está correto quando `Set Actor Location` é executado?
- [ ] `OutLocation` está sendo passado corretamente para `Set Actor Location`?

### **Para Processamento de Frames:**
- [ ] `ProcessBinaryBuffer` no final usa `Make Array` vazio (0 elementos)?
- [ ] Recursão de `ProcessNextFrame` está controlada corretamente?
- [ ] Buffer está sendo limpo após processar cada frame?

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Aplicar correções acima** no Blueprint Editor
2. **Adicionar logs detalhados** em todos os pontos críticos:
   - Após `Array_Find`: `FoundIndex`, `OutPlayerId`
   - Antes de `Array_Add`: `OutPlayerId`, `RemoteActorIds.Num()`
   - Após `Get Array Item`: `FoundIndex`, `ExistingActorRef`
   - Antes de `Set Actor Location`: `RemoteActorRef`, `OutLocation`
3. **Testar com 4 clients** e coletar logs
4. **Analisar logs** para identificar padrões
5. **Aplicar correções adicionais** baseadas nos resultados

---

## 📝 **NOTAS ADICIONAIS:**

### **Sobre o Padrão Assimétrico:**
- O fato de que apenas Client 4 funciona pode indicar que:
  - Client 4 é o último a spawnar e não tem problemas de sincronização
  - Client 4 tem um `PlayerID` diferente que não está sendo filtrado incorretamente
  - Os arrays estão sincronizados corretamente apenas para Client 4

### **Sobre os Múltiplos Spawns:**
- 2 instâncias podem indicar que:
  - `Array_Add` está sendo executado duas vezes (uma vez no caminho incorreto)
  - `Array_Find` não está encontrando o actor recém-spawnado antes do próximo frame chegar
  - Há uma race condition entre múltiplos frames chegando simultaneamente

---

**Fim do Documento**

