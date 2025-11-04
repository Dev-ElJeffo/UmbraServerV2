# 🔍 **GUIA DE VERIFICAÇÃO E CORREÇÃO: `ProcessNextFrame`**

## 📋 **OBJETIVO:**
Fornecer um guia passo a passo para verificar e corrigir a estrutura do Blueprint `ProcessNextFrame` no Unreal Editor.

---

## 🔍 **ETAPA 1: VERIFICAÇÃO INICIAL**

### **1.1 Abrir o Blueprint:**
1. No Unreal Editor, abra `BP_NetMovementClient`
2. Abra a função `ProcessNextFrame` (Custom Event)

### **1.2 Localizar os Nós Críticos:**
Procure e identifique os seguintes nós:

1. **`Array_Find`** (K2Node_CallArrayFunction_4)
   - Deve buscar em `RemoteActorIds`
   - Deve buscar `OutPlayerId`
   - Retorna `FoundIndex` (Integer)

2. **`Greater or Equal`** (K2Node_PromotableOperator_10)
   - Input A: `FoundIndex`
   - Input B: `0` (constante)
   - Output: Boolean

3. **`Branch`** (K2Node_IfThenElse_6)
   - Condition: conectado ao output do `Greater or Equal`
   - Pin `then` (True): **VERIFICAR SE ESTÁ CONECTADO**
   - Pin `else` (False): deve estar conectado ao `SpawnActorFromClass`

4. **`SpawnActorFromClass`** (K2Node_SpawnActorFromClass_0)
   - Deve estar conectado ao pin `else` do `Branch`

---

## 🎯 **ETAPA 2: VERIFICAR O PIN `then` DO `K2Node_IfThenElse_6`**

### **2.1 Verificação Visual:**
1. Localize o nó `K2Node_IfThenElse_6` (Branch)
2. Observe o pin `then` (True) - geralmente à direita, marcado com "true"
3. **Verifique se há uma linha/conexão saindo deste pin**

### **2.2 Se o Pin `then` ESTÁ DESCONECTADO:**
- Você verá o pin sem nenhuma linha saindo dele
- Isso significa que actors existentes nunca são atualizados
- **AÇÃO:** Implemente a Correção A abaixo

### **2.3 Se o Pin `then` ESTÁ CONECTADO:**
- Você verá uma linha saindo do pin
- **AÇÃO:** Siga a linha e verifique se ela leva a:
  1. `Get Array Item` (obtendo `RemoteActors[FoundIndex]`)
  2. `Set Variable` (definindo `RemoteActorRef`)
  3. Conexão ao mesmo ponto que recebe `RemoteActorRef` após o spawn

---

## 🔧 **ETAPA 3: CORREÇÕES NECESSÁRIAS**

### **CORREÇÃO A: Conectar o Pin `then` do `K2Node_IfThenElse_6`**

**SE O PIN `then` ESTÁ DESCONECTADO, SIGA ESTES PASSOS:**

#### **PASSO 1: Adicionar `Get Array Item`**
1. Clique com o botão direito no canvas do Blueprint
2. Procure por: **"Get Array Item"**
3. Selecione: **"Get Array Item"** (não confunda com "Set Array Item")
4. Configure:
   - **Array:** Clique no pin de entrada e selecione `RemoteActors` (variável do Blueprint)
   - **Index:** Conecte `FoundIndex` (do `Array_Find`)
   - **Output:** Será o `ExistingActorRef` (Actor Reference)

#### **PASSO 2: Conectar ao Pin `then`**
1. Localize o pin `then` (True) do `K2Node_IfThenElse_6`
2. Arraste uma linha do pin `then` até o pin `execute` (entrada) do `Get Array Item`

#### **PASSO 3: Adicionar `Set Variable`**
1. Clique com o botão direito no canvas do Blueprint
2. Procure por: **"Set RemoteActorRef"** (ou "Set Variable" e selecione `RemoteActorRef`)
3. Configure:
   - **Variable:** `RemoteActorRef` (deve aparecer automaticamente)
   - **Value:** Conecte o `Output` do `Get Array Item` (ExistingActorRef)
   - **Pin `execute`:** Conecte ao pin `then` (saída) do `Get Array Item`

#### **PASSO 4: Conectar ao Ponto de Convergência**
1. Localize o nó que atualmente recebe `RemoteActorRef` após o spawn
   - Isso geralmente é um `Is Valid` (K2Node_IfThenElse_9) ou diretamente `Set Actor Location`
2. Arraste uma linha do pin `then` (saída) do `Set Variable` até o pin `execute` do nó identificado
3. **IMPORTANTE:** Este deve ser o mesmo ponto onde o caminho do spawn converge

---

### **CORREÇÃO B: Adicionar `Array_Add` Após Spawn**

**VERIFICAÇÃO:**
1. Localize o nó `Set Variable` (K2Node_VariableSet_3) que define `RemoteActorRef` após o spawn
2. Verifique se após este nó há dois `Array_Add`:
   - `Array_Add` para `RemoteActorIds`
   - `Array_Add` para `RemoteActors`

**SE NÃO EXISTIREM, SIGA ESTES PASSOS:**

#### **PASSO 1: Adicionar Primeiro `Array_Add`**
1. Clique com o botão direito no canvas do Blueprint
2. Procure por: **"Array Add"**
3. Selecione: **"Array Add"**
4. Configure:
   - **Array:** Clique no pin de entrada e selecione `RemoteActorIds` (variável do Blueprint)
   - **Item:** Conecte `OutPlayerId` (do `ParseStateUpdateFrame`)
   - **Pin `execute`:** Conecte ao pin `then` (saída) do `Set Variable` (K2Node_VariableSet_3)

#### **PASSO 2: Adicionar Segundo `Array_Add`**
1. Clique com o botão direito no canvas do Blueprint
2. Procure por: **"Array Add"**
3. Selecione: **"Array Add"**
4. Configure:
   - **Array:** Clique no pin de entrada e selecione `RemoteActors` (variável do Blueprint)
   - **Item:** Conecte `RemoteActorRef` (do `Set Variable`)
   - **Pin `execute`:** Conecte ao pin `then` (saída) do primeiro `Array_Add`

#### **PASSO 3: Conectar ao Ponto de Convergência**
1. Arraste uma linha do pin `then` (saída) do segundo `Array_Add` até o mesmo ponto de convergência usado na Correção A
2. **IMPORTANTE:** Ambos os caminhos (actor existe / actor não existe) devem convergir no mesmo ponto

---

## 📊 **ESTRUTURA CORRETA ESPERADA:**

### **VISUALIZAÇÃO DO FLUXO:**

```
[ParseStateUpdateFrame]
  ↓
[Not Equal: OutPlayerId != LocalPlayerId]
  ↓
[Branch: OutPlayerId != LocalPlayerId?]
  ├─ True: CONTINUA
  └─ False: FIM (ignora)
  ↓
[Array_Find(RemoteActorIds, OutPlayerId)]
  ↓
[Greater or Equal: FoundIndex >= 0?]
  ↓
[Branch: FoundIndex >= 0?] ← K2Node_IfThenElse_6
  │
  ├─ then (True - actor existe):
  │   ├─ [Get Array Item(RemoteActors, FoundIndex)]
  │   ├─ [Set Variable: RemoteActorRef = ExistingActorRef]
  │   └─ [CONVERGE AQUI] ──────┐
  │                             │
  └─ else (False - actor não existe):  │
      ├─ [Make Transform]               │
      ├─ [SpawnActorFromClass]          │
      ├─ [Set Variable: RemoteActorRef = SpawnedActor]
      ├─ [Array_Add(RemoteActorIds, OutPlayerId)]
      ├─ [Array_Add(RemoteActors, RemoteActorRef)]
      └─ [CONVERGE AQUI] ──────────────┘
                                    ↓
                        [Is Valid (RemoteActorRef)?]
                                    ↓
                        [Branch: Is Valid?]
                                    ├─ True:
                                    │   ├─ [Set Actor Location]
                                    │   └─ [Set Actor Rotation]
                                    └─ False: FIM (ignora)
```

---

## ✅ **CHECKLIST FINAL:**

### **Verifique se TODOS os itens estão corretos:**

1. ✅ `Array_Find` existe e busca em `RemoteActorIds` com `OutPlayerId`
2. ✅ `Greater or Equal` existe e compara `FoundIndex >= 0`
3. ✅ `K2Node_IfThenElse_6` existe e está conectado ao `Greater or Equal`
4. ✅ Pin `else` do `K2Node_IfThenElse_6` conectado ao `SpawnActorFromClass`
5. ✅ Pin `then` do `K2Node_IfThenElse_6` conectado ao `Get Array Item`
6. ✅ `Get Array Item` obtém `RemoteActors[FoundIndex]`
7. ✅ `Set Variable` define `RemoteActorRef = ExistingActorRef` (no caminho do pin `then`)
8. ✅ `Set Variable` define `RemoteActorRef = SpawnedActor` (no caminho do spawn)
9. ✅ `Array_Add` adiciona `OutPlayerId` a `RemoteActorIds` (após spawn)
10. ✅ `Array_Add` adiciona `RemoteActorRef` a `RemoteActors` (após spawn)
11. ✅ Ambos os caminhos convergem no mesmo ponto antes de `Is Valid`
12. ✅ `Is Valid` verifica `RemoteActorRef` antes de atualizar
13. ✅ `Set Actor Location` e `Set Actor Rotation` são executados após `Is Valid`

---

## 🎯 **TESTE APÓS CORREÇÕES:**

1. Compile o Blueprint
2. Execute o jogo
3. Conecte dois clientes
4. Verifique se:
   - ✅ Cada cliente spawna apenas um actor para cada player remoto
   - ✅ Os actors se movem quando os players remotos se movem
   - ✅ Não há múltiplos spawns do mesmo `PlayerID`

---

## ⚠️ **PROBLEMAS COMUNS:**

### **Problema 1: "Array_Add não está adicionando"**
- **Causa:** `Array_Add` pode estar usando um array local ao invés da variável do Blueprint
- **Solução:** Certifique-se de que está usando `RemoteActorIds` e `RemoteActors` como variáveis do Blueprint (não arrays locais)

### **Problema 2: "Actor existe mas não é atualizado"**
- **Causa:** Pin `then` do `K2Node_IfThenElse_6` não está conectado ou não leva ao ponto de atualização
- **Solução:** Verifique se o caminho do pin `then` converge no mesmo ponto que o caminho do spawn

### **Problema 3: "Múltiplos spawns do mesmo PlayerID"**
- **Causa:** `Array_Add` não está sendo executado ou está usando o array errado
- **Solução:** Verifique se `Array_Add` está conectado imediatamente após `Set Variable` no caminho do spawn

---

## 📝 **NOTAS IMPORTANTES:**

1. **Ordem de Execução:** Os `Array_Add` devem ser executados **IMEDIATAMENTE** após `Set Variable` no caminho do spawn
2. **Convergência:** Ambos os caminhos (actor existe / actor não existe) devem convergir no mesmo ponto antes de `Is Valid`
3. **Variáveis:** Certifique-se de que `RemoteActorIds` e `RemoteActors` são variáveis do Blueprint (não arrays locais de função)
4. **Validação:** Sempre valide `RemoteActorRef` com `Is Valid` antes de atualizar posição/rotação

