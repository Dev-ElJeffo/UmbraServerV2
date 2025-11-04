# 🔧 **GUIA PASSO A PASSO: CORRIGIR PIN `then` DESCONECTADO**

## 📋 **PROBLEMA IDENTIFICADO:**

O pin `then` (True) do `K2Node_IfThenElse_6` está **desconectado**. Quando um actor remoto já existe (`FoundIndex >= 0`), nenhuma lógica é executada, causando:
- ❌ Actors existentes nunca são atualizados
- ❌ Múltiplos spawns do mesmo player
- ❌ Clientes não se veem uns aos outros

---

## 🎯 **SOLUÇÃO:**

Conectar o pin `then` e adicionar a lógica para **atualizar actors existentes**.

---

## 📝 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Identificar o Branch Problemático**

1. Abra o Blueprint `BP_NetMovementClient`
2. Navegue até o Custom Event `ProcessNextFrame`
3. Localize o nó `K2Node_IfThenElse_6` (Branch)
4. Verifique:
   - **Input `Condition`**: Conectado a `Greater or Equal (FoundIndex >= 0)`
   - **Pin `then` (True)**: **DESCONECTADO** ← Este é o problema!
   - **Pin `else` (False)**: Conectado ao `SpawnActorFromClass`

---

### **PASSO 2: Adicionar `Get Array Item` para Obter o Actor Existente**

**O que fazer:**
1. **Criar um novo nó**: `Get Array Item`
   - **Como**: Clique com botão direito → `Get Array Item`
   - **Ou**: Pesquise "Get Array Item" na barra de pesquisa

2. **Conectar `Array` pin:**
   - **De**: `Get Variable: RemoteActors` (ou variável `RemoteActors`)
   - **Para**: Pin `Array` do `Get Array Item`

3. **Conectar `Index` pin:**
   - **De**: `Array_Find.ReturnValue` (`FoundIndex`)
   - **Para**: Pin `Index` do `Get Array Item`
   - **Nota**: Se `Array_Find` não estiver conectado diretamente, use um `Knot` para facilitar

4. **Resultado**: `Get Array Item` agora retorna o actor existente (`ExistingActorRef`)

**Visual:**
```
Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex
  ↓
Greater or Equal (FoundIndex >= 0)
  ↓
Branch [K2Node_IfThenElse_6]
  ├─ then (True): [VAMOS CONECTAR AQUI]
  └─ else (False): SpawnActorFromClass

[NOVO] Get Array Item
  - Array: RemoteActors
  - Index: FoundIndex
  - Output: ExistingActorRef
```

---

### **PASSO 3: Definir `RemoteActorRef` com o Actor Existente**

**O que fazer:**
1. **Criar um novo nó**: `Set Variable`
   - **Variável**: `RemoteActorRef`
   - **Como**: Clique com botão direito → `Set RemoteActorRef` (ou arraste a variável e selecione "Set")

2. **Conectar `Value` pin:**
   - **De**: `Get Array Item.Output` (`ExistingActorRef`)
   - **Para**: Pin `Value` do `Set Variable`

3. **Conectar `Execute` pin:**
   - **De**: Pin `then` (True) do `K2Node_IfThenElse_6`
   - **Para**: Pin `Execute` do `Set Variable`

**Visual:**
```
Branch [K2Node_IfThenElse_6]
  ├─ then (True):
  │     ↓
  │     Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │     ↓
  │     Set Variable: RemoteActorRef = ExistingActorRef ✅
  │
  └─ else (False): SpawnActorFromClass
```

---

### **PASSO 4: Atualizar Posição e Rotação do Actor Existente**

**O que fazer:**
1. **Localizar os nós existentes**: `Set Actor Location` e `Set Actor Rotation`
   - Estes nós já existem no Blueprint (conectados ao caminho "actor não existe")
   - **OU** criar novos nós se não existirem

2. **Opção A: Reutilizar nós existentes (RECOMENDADO)**
   - Se os nós `Set Actor Location` e `Set Actor Rotation` já existem no caminho "actor não existe":
     - **Criar `Knot` nodes** para conectar ambos os caminhos
     - **Conectar** o pin `then` do `Set Variable` (do Passo 3) a um `Knot`
     - **Conectar** este `Knot` ao mesmo `Set Actor Location` usado no caminho "actor não existe"
     - **Garantir** que `RemoteActorRef` está conectado corretamente

3. **Opção B: Criar novos nós (SE NÃO EXISTIREM)**
   - Criar `Set Actor Location`:
     - **Input `Target`**: `RemoteActorRef` (variável)
     - **Input `New Location`**: `OutLocation` (do `ParseStateUpdateFrame`)
   - Criar `Set Actor Rotation`:
     - **Input `Target`**: `RemoteActorRef` (variável)
     - **Input `New Rotation`**: Criar um `Make Rotator` com:
       - **Yaw**: `OutYawDegrees` (do `ParseStateUpdateFrame`)
       - **Pitch**: `0.0`
       - **Roll**: `0.0`

4. **Conectar `Execute` pins:**
   - **De**: Pin `then` do `Set Variable` (do Passo 3)
   - **Para**: Pin `Execute` do `Set Actor Location`
   - **De**: Pin `then` do `Set Actor Location`
   - **Para**: Pin `Execute` do `Set Actor Rotation`

**Visual:**
```
Branch [K2Node_IfThenElse_6]
  ├─ then (True):
  │     ↓
  │     Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │     ↓
  │     Set Variable: RemoteActorRef = ExistingActorRef
  │     ↓
  │     Set Actor Location (RemoteActorRef, OutLocation)
  │     ↓
  │     Set Actor Rotation (RemoteActorRef, Make Rotator(Yaw=OutYawDegrees))
  │     ↓
  │     [CONTINUAR PARA PROCESSBINARYBUFFER]
  │
  └─ else (False): SpawnActorFromClass → ... → Set Actor Location → Set Actor Rotation
```

---

### **PASSO 5: Garantir Convergência dos Caminhos**

**O que fazer:**
1. **Localizar** o nó `ProcessBinaryBuffer` (que verifica se há mais frames no buffer)
   - Este nó deve estar após a atualização de posição/rotação no caminho "actor não existe"

2. **Conectar** o pin `then` do `Set Actor Rotation` (do Passo 4, caminho "actor existe") ao **mesmo** `ProcessBinaryBuffer`

3. **Verificar** que ambos os caminhos convergem:
   - **Caminho 1 (actor existe)**: `Set Actor Rotation` → `ProcessBinaryBuffer`
   - **Caminho 2 (actor não existe)**: `Set Actor Rotation` → `ProcessBinaryBuffer`

4. **Se necessário**, usar `Knot` nodes para facilitar a conexão

**Visual:**
```
[Caminho 1: Actor Existe]
  Set Actor Rotation
    ↓
    [KNOT] ← Ponto de convergência
    ↓
    ProcessBinaryBuffer

[Caminho 2: Actor Não Existe]
  Set Actor Rotation
    ↓
    [KNOT] ← Ponto de convergência
    ↓
    ProcessBinaryBuffer
```

---

### **PASSO 6: Verificar Recursão do `ProcessNextFrame`**

**O que fazer:**
1. **Localizar** o `Branch` após `ProcessBinaryBuffer`
   - Este `Branch` verifica se `ProcessBinaryBuffer.ReturnValue == true`

2. **Verificar** que:
   - **Se `true`**: Chama `ProcessNextFrame` recursivamente (para processar o próximo frame no buffer)
   - **Se `false`**: Para (não há mais frames no buffer)

3. **Garantir** que ambos os caminhos (actor existe / actor não existe) chegam a este `Branch`

---

## ✅ **CHECKLIST FINAL:**

Após implementar todos os passos, verifique:

- [ ] Pin `then` (True) do `K2Node_IfThenElse_6` está **conectado**
- [ ] `Get Array Item` obtém o actor de `RemoteActors[FoundIndex]`
- [ ] `Set Variable` define `RemoteActorRef = ExistingActorRef`
- [ ] `Set Actor Location` atualiza a posição do actor existente
- [ ] `Set Actor Rotation` atualiza a rotação do actor existente
- [ ] Ambos os caminhos (actor existe / actor não existe) convergem no mesmo `ProcessBinaryBuffer`
- [ ] `ProcessBinaryBuffer` chama `ProcessNextFrame` recursivamente se houver mais frames
- [ ] Compilação do Blueprint sem erros
- [ ] Teste com múltiplos clientes funciona corretamente

---

## 🎯 **RESULTADO ESPERADO:**

Após a correção:

1. ✅ **Novos actors são spawnados** quando um frame de um novo player é recebido
2. ✅ **Actors existentes são atualizados** a cada frame recebido
3. ✅ **Apenas um spawn por `PlayerID`** (sem duplicatas)
4. ✅ **Movimento sincronizado** entre clientes
5. ✅ **Clientes se veem uns aos outros** corretamente

---

## 📌 **NOTAS IMPORTANTES:**

1. **Ordem de execução**: Garanta que `Set Variable` (RemoteActorRef) é executado **ANTES** de `Set Actor Location` e `Set Actor Rotation`
2. **Arrays sincronizados**: Não modifique `RemoteActorIds` ou `RemoteActors` no caminho "actor existe" (apenas atualize a posição/rotação)
3. **Recursão**: O `ProcessNextFrame` deve ser chamado recursivamente para processar todos os frames no buffer
4. **Filtro do próprio player**: Garanta que o filtro `OutPlayerId != MyPlayerId` está funcionando corretamente

---

**Se ainda houver problemas após a correção, verifique os logs do Unreal Engine para identificar onde o fluxo está falhando.**

