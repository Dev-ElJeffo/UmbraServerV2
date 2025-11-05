# 🔍 **GUIA PRÁTICO: Verificação XML ProcessNextFrame**

## 📋 **COMO USAR ESTE GUIA:**

1. Abra o arquivo XML do Blueprint `BP_NetMovementClient`
2. Procure por cada padrão XML listado abaixo
3. Verifique se a estrutura corresponde ao esperado
4. Corrija os problemas identificados

---

## 🔎 **BUSCA 1: Verificar Estrutura do Filtro**

### **BUSCA 1.1: ParseStateUpdateFrame**

**Procurar por:**
```xml
<FunctionName>ParseStateUpdateFrame</FunctionName>
```

**Verificar:**
- ✅ Está presente após `Get Array Item: Data[0]`?
- ✅ Está dentro de um `Branch` que verifica `Data[0] == 2`?

**Problema esperado:**
- Se não estiver presente, o frame não está sendo parseado.

---

### **BUSCA 1.2: Log do Filtro (PROBLEMA CRÍTICO)**

**Procurar por:**
```xml
<StringFormat>Filtro</StringFormat>
```
ou
```xml
<StringFormat>🔍</StringFormat>
```

**Verificar a ordem dos nós:**
1. `Not Equal: OutPlayerId != Active Player ID`
2. **[LOG DO FILTRO DEVE ESTAR AQUI]** ← ANTES DO BRANCH!
3. `Branch: K2Node_IfThenElse_1`

**Se o log estiver DEPOIS do Branch:**
- ❌ **PROBLEMA:** Frames do próprio player não aparecerão nos logs
- ✅ **SOLUÇÃO:** Mover o log para ANTES do `Branch`

**Procurar por:**
```xml
<NodeName>K2Node_IfThenElse_1</NodeName>
```

**Verificar conexões:**
- Pin `then` (True): Conectado → CONTINUA (outro player)
- Pin `else` (False): Desconectado ou com `Return` → PARA (próprio player)

**Se o log estiver conectado ao pin `then`:**
- ❌ **PROBLEMA:** Quando o filtro bloqueia (`False`), o log não executa
- ✅ **SOLUÇÃO:** Mover o log para ANTES do `Branch`

---

## 🔎 **BUSCA 2: Verificar Validação de OutLocation (PROBLEMA CRÍTICO)**

### **BUSCA 2.1: Validação de OutLocation**

**Procurar por:**
```xml
<OperatorName>NotEqual</OperatorName>
<InputType>Struct</InputType>
<StructType>Vector</StructType>
```

**OU procurar por:**
```xml
<FunctionName>Break Vector</FunctionName>
```

**Verificar:**
- ✅ Está presente ANTES de `SpawnActorFromClass`?
- ✅ Está dentro do pin `else` (False) do `K2Node_IfThenElse_6`?

**Se NÃO estiver presente:**
- ❌ **PROBLEMA:** Spawn falhará com `Location (0,0,0)`
- ✅ **SOLUÇÃO:** Adicionar validação `Not Equal (Vector): OutLocation != (0,0,0)` ANTES de `SpawnActorFromClass`

---

### **BUSCA 2.2: SpawnActorFromClass**

**Procurar por:**
```xml
<NodeName>K2Node_SpawnActorFromClass</NodeName>
```

**Verificar:**
1. **CollisionHandlingOverride:**
   ```xml
   <PinName>CollisionHandlingOverride</PinName>
   <DefaultValue>Always Spawn</DefaultValue>
   ```
   - ✅ Deve ser `Always Spawn` ou `AdjustIfNeeded`
   - ❌ Se for `Undefined`, spawn falhará com colisão

2. **SpawnTransform:**
   ```xml
   <PinName>SpawnTransform</PinName>
   ```
   - ✅ Deve estar conectado ao `Return Value` do `Make Transform`
   - ❌ Se não estiver conectado, spawn usará `(0,0,0)`

---

## 🔎 **BUSCA 3: Verificar Array_Find e Gestão de Actors**

### **BUSCA 3.1: Array_Find**

**Procurar por:**
```xml
<FunctionName>Array_Find</FunctionName>
```

**Verificar:**
- ✅ Está presente após o filtro do próprio player?
- ✅ Target Array: `RemoteActorIds`?
- ✅ Item To Find: `OutPlayerId`?
- ✅ Return Value conectado a `Greater or Equal`?

**Se NÃO estiver presente:**
- ❌ **PROBLEMA:** Múltiplos spawns sequenciais do mesmo `OutPlayerId`
- ✅ **SOLUÇÃO:** Adicionar `Array_Find` após o filtro

---

### **BUSCA 3.2: Greater or Equal**

**Procurar por:**
```xml
<OperatorName>GreaterEqual</OperatorName>
```

**Verificar:**
- ✅ Input A: `FoundIndex` (do `Array_Find`)?
- ✅ Input B: `0` (constante)?
- ✅ Return Value conectado ao `Condition` do `K2Node_IfThenElse_6`?

---

### **BUSCA 3.3: K2Node_IfThenElse_6 (PROBLEMA CRÍTICO)**

**Procurar por:**
```xml
<NodeName>K2Node_IfThenElse_6</NodeName>
```

**Verificar conexões do pin `then` (True):**
```xml
<ThenPin>
  <!-- Verificar se há conexões aqui -->
</ThenPin>
```

**Se o pin `then` estiver DESCONECTADO:**
- ❌ **PROBLEMA CRÍTICO:** Actors existentes nunca são atualizados!
- ✅ **SOLUÇÃO:** Conectar o pin `then` a:
  1. `Get Array Item(RemoteActors, FoundIndex)`
  2. `Set Variable: RemoteActorRef = ExistingActorRef`
  3. [CONVERGÊNCIA]

**Verificar conexões do pin `else` (False):**
```xml
<ElsePin>
  <!-- Deve conectar a SpawnActorFromClass -->
</ElsePin>
```

- ✅ Deve estar conectado a `SpawnActorFromClass`
- ❌ Se desconectado, novos actors nunca são spawnados

---

### **BUSCA 3.4: Get Array Item**

**Procurar por:**
```xml
<FunctionName>Get Array Item</FunctionName>
```

**Verificar:**
- ✅ Está presente no pin `then` do `K2Node_IfThenElse_6`?
- ✅ Target Array: `RemoteActors`?
- ✅ Dimension 1 (Index): `FoundIndex` (do `Array_Find`)?
- ✅ Return Value conectado a `Set Variable: RemoteActorRef`?

**Se NÃO estiver presente:**
- ❌ **PROBLEMA:** Actors existentes não podem ser recuperados
- ✅ **SOLUÇÃO:** Adicionar `Get Array Item` no pin `then` do `K2Node_IfThenElse_6`

---

## 🔎 **BUSCA 4: Verificar Array_Add (PROBLEMA CRÍTICO)**

### **BUSCA 4.1: Array_Add para RemoteActorIds**

**Procurar por:**
```xml
<FunctionName>Array_Add</FunctionName>
<TargetArray>RemoteActorIds</TargetArray>
```

**Verificar:**
- ✅ Está presente APÓS `SpawnActorFromClass`?
- ✅ Item: `OutPlayerId` (do `ParseStateUpdateFrame`)?
- ✅ Execute conectado após `Set Variable: RemoteActorRef`?

**Se NÃO estiver presente:**
- ❌ **PROBLEMA CRÍTICO:** `Array_Find` sempre retornará `-1`!
- ✅ **SOLUÇÃO:** Adicionar `Array_Add` para `RemoteActorIds` com `OutPlayerId`

---

### **BUSCA 4.2: Array_Add para RemoteActors**

**Procurar por:**
```xml
<FunctionName>Array_Add</FunctionName>
<TargetArray>RemoteActors</TargetArray>
```

**Verificar:**
- ✅ Está presente APÓS `SpawnActorFromClass`?
- ✅ Item: `Return Value` (do `SpawnActorFromClass`)?
- ✅ Execute conectado após `Array_Add` para `RemoteActorIds`?

**Se NÃO estiver presente:**
- ❌ **PROBLEMA CRÍTICO:** `Get Array Item` não funcionará para actors existentes!
- ✅ **SOLUÇÃO:** Adicionar `Array_Add` para `RemoteActors` com `SpawnedActor`

---

## 🔎 **BUSCA 5: Verificar Convergência dos Caminhos**

**Procurar por:**
```xml
<FunctionName>Is Valid</FunctionName>
```

**Verificar:**
- ✅ Está presente após ambos os caminhos convergirem?
- ✅ Input: `RemoteActorRef` (variável)?
- ✅ Ambos os caminhos (actor existe / actor não existe) convergem ANTES de `Is Valid`?

**Estrutura esperada:**
```
[Caminho 1: Actor existe]
  Get Array Item → Set Variable: RemoteActorRef
  ↓
[CONVERGÊNCIA] ← Ambos os caminhos chegam aqui
  ↓
Is Valid (RemoteActorRef)
  ↓
Set Actor Location / Set Actor Rotation

[Caminho 2: Actor não existe]
  SpawnActorFromClass → Array_Add → Set Variable: RemoteActorRef
  ↓
[CONVERGÊNCIA] ← Ambos os caminhos chegam aqui
  ↓
Is Valid (RemoteActorRef)
  ↓
Set Actor Location / Set Actor Rotation
```

---

## 📊 **CHECKLIST RÁPIDO:**

Copie e cole este checklist no seu editor de texto e marque conforme verifica:

```
[ ] ParseStateUpdateFrame presente e conectado corretamente
[ ] Get Active Player ID usado (NÃO MyPlayerId)
[ ] Not Equal compara OutPlayerId != Active Player ID
[ ] Log do filtro ANTES do Branch (K2Node_IfThenElse_1)
[ ] Array_Find presente e busca em RemoteActorIds
[ ] Greater or Equal verifica FoundIndex >= 0
[ ] K2Node_IfThenElse_6 pin then CONECTADO
[ ] Get Array Item presente no pin then
[ ] Validação OutLocation != (0,0,0) ANTES de SpawnActorFromClass
[ ] Make Transform recebe OutLocation corretamente
[ ] CollisionHandlingOverride = Always Spawn
[ ] Array_Add para RemoteActorIds presente
[ ] Array_Add para RemoteActors presente
[ ] Convergência dos caminhos antes de Is Valid
[ ] Set Actor Location presente e conectado
[ ] Set Actor Rotation presente e conectado
```

---

## 🎯 **PROBLEMAS ESPECÍFICOS IDENTIFICADOS NOS LOGS:**

### **PROBLEMA 1: Log do Filtro Não Aparece para Próprio Player**

**Como identificar no XML:**
1. Procure por `K2Node_FormatText` com texto "Filtro"
2. Procure por `K2Node_IfThenElse_1` (Branch do filtro)
3. Verifique a ordem: `Not Equal` → **[LOG]** → `Branch`?
   - ✅ Se SIM: Correto
   - ❌ Se NÃO: Mover log para ANTES do Branch

---

### **PROBLEMA 2: Spawn Falhando com Location (0,0,0)**

**Como identificar no XML:**
1. Procure por `K2Node_SpawnActorFromClass`
2. Verifique se há `Not Equal (Vector)` ANTES dele:
   - ✅ Se SIM: Correto
   - ❌ Se NÃO: Adicionar validação
3. Verifique `CollisionHandlingOverride`:
   - ✅ Se `Always Spawn`: Correto
   - ❌ Se `Undefined`: Mudar para `Always Spawn`

---

### **PROBLEMA 3: Múltiplos Spawns Sequenciais**

**Como identificar no XML:**
1. Procure por `Array_Find`:
   - ✅ Se presente: Verificar conexões
   - ❌ Se ausente: Adicionar
2. Procure por `K2Node_IfThenElse_6`:
   - ✅ Se pin `then` conectado: Correto
   - ❌ Se pin `then` desconectado: Conectar
3. Procure por `Array_Add`:
   - ✅ Se DOIS presentes: Correto
   - ❌ Se faltar algum: Adicionar

---

## ✅ **CORREÇÕES PRIORITÁRIAS:**

### **PRIORIDADE 1 (CRÍTICO):**
1. ✅ Conectar pin `then` do `K2Node_IfThenElse_6` a `Get Array Item` → `Set Variable`
2. ✅ Adicionar validação `OutLocation != (0,0,0)` ANTES de `SpawnActorFromClass`
3. ✅ Configurar `CollisionHandlingOverride` como `Always Spawn`
4. ✅ Adicionar `Array_Add` para ambos os arrays após `SpawnActorFromClass`

### **PRIORIDADE 2 (IMPORTANTE):**
5. ✅ Mover log do filtro para ANTES do Branch
6. ✅ Verificar se `Array_Find` está presente e conectado corretamente
7. ✅ Verificar convergência dos caminhos antes de `Is Valid`

### **PRIORIDADE 3 (DEBUG):**
8. ✅ Adicionar logs após `Array_Find`
9. ✅ Adicionar log antes de `SpawnActorFromClass`
10. ✅ Adicionar log após `SpawnActorFromClass`

---

## 📝 **NOTAS FINAIS:**

- Use este guia como referência ao verificar o XML
- Cada busca deve ser feita sequencialmente
- Se um item não estiver presente, isso explica o problema correspondente nos logs
- Após corrigir, teste novamente e verifique se os logs mudam

---

## 🔗 **RELACIONADO:**

- `ANALISE_XML_COMPLETO_PROCESSNEXTFRAME_VERIFICACAO.md` - Checklist completo
- `CORRECAO_DEFINITIVA_PROCESSNEXTFRAME.md` - Guia de correção passo a passo
- `ANALISE_LOGS_SPAWN_FALHANDO.md` - Análise dos logs específicos
