# 🔧 **GUIA PRÁTICO: Correção de Múltiplos Spawns e Movimento Não Replicado**

## 📋 **PROBLEMAS A CORRIGIR:**

1. **Múltiplos spawns iniciais** (2 instâncias de RemotePlayers)
2. **Movimento não replicado** (assimétrico entre clients)
3. **Nenhum movimento replicado** após últimas alterações

---

## 🎯 **CORREÇÃO 1: Remover Array_Add do Caminho `then` de K2Node_IfThenElse_6**

### **PROBLEMA:**
O pin `then` de `K2Node_IfThenElse_6` está conectado a `Array_Add`, causando duplicatas quando o actor já existe.

### **AÇÃO NO BLUEPRINT:**

#### **PASSO 1: Localizar K2Node_IfThenElse_6**
1. No grafo de `ProcessNextFrame`, localize o nó `K2Node_IfThenElse_6`
2. Este nó recebe `FoundIndex >= 0` do `Greater or Equal`

#### **PASSO 2: Seguir o Pin `then`**
1. O pin `then` (verde) deve estar conectado quando `FoundIndex >= 0` (actor existe)
2. **ATUALMENTE**: O pin `then` está conectado a `Set Variable: RemoteActorRef` e depois a `Array_Add`
3. **PROBLEMA**: Isso adiciona o actor aos arrays mesmo quando ele já existe

#### **PASSO 3: Remover Array_Add do Caminho `then`**
1. **DESCONECTE** `Array_Add (RemoteActorIds, OutPlayerId)` do caminho `then`
2. **DESCONECTE** `Array_Add (RemoteActors, RemoteActorRef)` do caminho `then`
3. **ARRAY_ADD DEVE ESTAR APENAS NO CAMINHO `else` (spawn)**

#### **PASSO 4: Verificar Conexão Correta**
O caminho `then` deve ser:
```
K2Node_IfThenElse_6 (then)
  ↓
Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  ↓
Set Variable: RemoteActorRef = ExistingActorRef
  ↓
Set Actor Location (RemoteActorRef, OutLocation)
  ↓
Set Actor Rotation (RemoteActorRef, OutYawDegrees)
  ↓
[NÃO CONECTAR Array_Add AQUI!]
```

---

## 🎯 **CORREÇÃO 2: Adicionar Get Array Item no Caminho `then`**

### **PROBLEMA:**
O caminho `then` pode não ter `Get Array Item`, fazendo com que o actor existente não seja recuperado corretamente.

### **AÇÃO NO BLUEPRINT:**

#### **PASSO 1: Verificar se Get Array Item Existe**
1. No caminho `then` de `K2Node_IfThenElse_6`, verifique se há um nó `Get Array Item`
2. Se **NÃO existir**, você precisa adicioná-lo

#### **PASSO 2: Adicionar Get Array Item (se necessário)**
1. Clique direito no espaço vazio após `K2Node_IfThenElse_6` (pin `then`)
2. Busque: `Get Array Item`
3. Selecione **`Get Array Item`**
4. Configure:
   - **Array**: `RemoteActors` (Array of Actor References)
   - **Index**: Conecte `FoundIndex` (do `Array_Find`)

#### **PASSO 3: Conectar Get Array Item**
1. Conecte a saída `ExistingActorRef` (ou `Array Element`) a `Set Variable: RemoteActorRef`
2. **IMPORTANTE**: Use `Set Variable` para atualizar `RemoteActorRef` com o actor existente
3. Conecte `Set Variable` a `Set Actor Location` e `Set Actor Rotation`

---

## 🎯 **CORREÇÃO 3: Adicionar Validação Is Valid**

### **PROBLEMA:**
`Get Array Item` pode retornar um actor inválido (`nullptr`), causando falhas silenciosas na atualização.

### **AÇÃO NO BLUEPRINT:**

#### **PASSO 1: Adicionar Is Valid**
1. Após `Get Array Item`, adicione um nó `Is Valid`
2. Conecte `ExistingActorRef` (do `Get Array Item`) a `Is Valid`
3. Adicione um `Branch` após `Is Valid`

#### **PASSO 2: Configurar Branch**
1. **Caminho `True` (Is Valid = true)**:
   - Continue com `Set Variable: RemoteActorRef`
   - Conecte a `Set Actor Location` e `Set Actor Rotation`
2. **Caminho `False` (Is Valid = false)**:
   - Trate como actor não encontrado
   - **OPÇÃO 1**: Conecte ao caminho `else` (spawn)
   - **OPÇÃO 2**: Adicione um log de erro e ignore

---

## 🎯 **CORREÇÃO 4: Verificar Filtro e Logs**

### **PROBLEMA:**
O filtro pode estar bloqueando frames incorretamente ou usando `MyPlayerId` desatualizado.

### **AÇÃO NO BLUEPRINT:**

#### **PASSO 1: Verificar Filtro**
1. Após `ParseStateUpdateFrame`, localize o filtro
2. Confirme que usa `Get Active Player ID` diretamente (não `MyPlayerId`)
3. Confirme que o log está **antes** do `Branch` do filtro

#### **PASSO 2: Adicionar Logs Detalhados**
1. Após `Array_Find`, adicione:
   ```
   Format Text: "[ProcessNextFrame] Array_Find - OutPlayerId: {0}, FoundIndex: {1}"
   ```
   - `{0}`: `OutPlayerId`
   - `{1}`: `FoundIndex`
2. Antes de `Array_Add` (no caminho `else`), adicione:
   ```
   Format Text: "[ProcessNextFrame] ANTES Array_Add - OutPlayerId: {0}, RemoteActorIds.Num(): {1}"
   ```
   - `{0}`: `OutPlayerId`
   - `{1}`: `RemoteActorIds.Num()` (Length do array)
3. Após `Get Array Item` (no caminho `then`), adicione:
   ```
   Format Text: "[ProcessNextFrame] Get Array Item - FoundIndex: {0}, ExistingActorRef: {1}"
   ```
   - `{0}`: `FoundIndex`
   - `{1}`: `ExistingActorRef` (convertido para String)
4. Antes de `Set Actor Location`, adicione:
   ```
   Format Text: "[ProcessNextFrame] Set Actor Location - RemoteActorRef: {0}, OutLocation: ({1}, {2}, {3})"
   ```
   - `{0}`: `RemoteActorRef` (convertido para String)
   - `{1}`: `OutLocation.X`
   - `{2}`: `OutLocation.Y`
   - `{3}`: `OutLocation.Z`

---

## 🎯 **CORREÇÃO 5: Verificar Array Vazio no Segundo ProcessBinaryBuffer**

### **PROBLEMA:**
Se `ProcessNextFrame` não passar um array vazio para o segundo `ProcessBinaryBuffer`, o mesmo frame pode ser processado múltiplas vezes.

### **AÇÃO NO BLUEPRINT:**

#### **PASSO 1: Localizar Segundo ProcessBinaryBuffer**
1. No final de `ProcessNextFrame`, localize o segundo `ProcessBinaryBuffer`
2. Este deve ser chamado após processar o frame atual

#### **PASSO 2: Verificar Make Array**
1. Confirme que `NewData` está conectado a um `Make Array`
2. **CRÍTICO**: O `Make Array` deve ter **0 elementos** (vazio)
3. Se não estiver vazio, **corrija** para usar `Make Array` com 0 elementos

#### **PASSO 3: Verificar Conexão Correta**
```
ProcessNextFrame
  ↓
[Processa frame atual: Parse → Filter → Spawn/Update]
  ↓
Make Array (0 Elements) ← CRÍTICO: Deve ter 0 elementos!
  ↓
ProcessBinaryBuffer
  - Buffer: Get BinaryMessageBuffer
  - NewData: [Make Array vazio] ← CRÍTICO!
  - OutFrame: Get OutFrame
  → ReturnValue
  ↓
Branch: ReturnValue?
  ├─ True: ProcessNextFrame (recursão) ← Só se houver mais frames
  └─ False: FIM ← Não há mais frames
```

---

## 📊 **ESTRUTURA CORRETA FINAL:**

### **ProcessNextFrame Completo:**
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
  ├─ then (True): [ATOR EXISTE]
  │                 Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │                 ↓
  │                 Is Valid (ExistingActorRef)
  │                 ↓
  │                 Branch: Is Valid?
  │                 ├─ True: Set Variable: RemoteActorRef = ExistingActorRef
  │                 │          ↓
  │                 │          Set Actor Location (RemoteActorRef, OutLocation)
  │                 │          ↓
  │                 │          Set Actor Rotation (RemoteActorRef, OutYawDegrees)
  │                 │          ↓
  │                 │          [NÃO EXECUTAR Array_Add!]
  │                 └─ False: [Tratar como não encontrado - log ou ignorar]
  │
  └─ else (False): [NOVO ATOR]
                    SpawnActorFromClass (BP_RemotePlayer_C, OutLocation, ...)
                    ↓
                    Set Variable: RemoteActorRef = [ReturnValue do Spawn]
                    ↓
                    Array_Add (RemoteActorIds, OutPlayerId)
                    ↓
                    Array_Add (RemoteActors, RemoteActorRef)
                    ↓
                    Set Actor Location (RemoteActorRef, OutLocation)
                    ↓
                    Set Actor Rotation (RemoteActorRef, OutYawDegrees)
  ↓
[CONTINUAÇÃO] Make Array (0 Elements)
  ↓
ProcessBinaryBuffer (Get BinaryMessageBuffer, Make Array vazio, Get OutFrame)
  ↓ (ReturnValue == true)
ProcessNextFrame (recursão)
```

---

## ✅ **CHECKLIST FINAL:**

### **Antes de Testar:**
- [ ] `Array_Add` está **apenas** no caminho `else` (spawn)
- [ ] `Array_Add` **NÃO** está no caminho `then` (atualização)
- [ ] `Get Array Item` está presente no caminho `then`
- [ ] `Is Valid` está verificando o actor após `Get Array Item`
- [ ] Filtro usa `Get Active Player ID` diretamente
- [ ] Logs detalhados estão adicionados
- [ ] `Make Array` vazio (0 elementos) está sendo usado no segundo `ProcessBinaryBuffer`

### **Após Testar:**
- [ ] Não há múltiplos spawns iniciais
- [ ] Movimento é replicado corretamente entre todos os clients
- [ ] Logs mostram `FoundIndex >= 0` quando o actor existe
- [ ] Logs mostram `Array_Add` apenas quando `FoundIndex < 0`
- [ ] Logs mostram `Set Actor Location` sendo executado para actors existentes

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Aplicar todas as correções acima** no Blueprint Editor
2. **Compilar e testar** com 4 clients simultâneos
3. **Coletar logs** de todos os clients
4. **Analisar logs** para confirmar que:
   - `Array_Add` está sendo executado apenas no spawn
   - `Get Array Item` está recuperando actors corretamente
   - `Set Actor Location` está sendo executado para todos os actors
5. **Ajustar** conforme necessário

---

**Fim do Documento**

