# 🔧 **GUIA PRÁTICO DETALHADO: Correção de Condição de Corrida - Múltiplos Spawns**

## 📋 **PROBLEMA IDENTIFICADO:**

Mesmo com a estrutura correta, múltiplos spawns ocorrem devido a uma **condição de corrida**:
- Múltiplos frames do mesmo PlayerID chegam antes que o `Array_Add` atualize o array
- `Array_Find` retorna `-1` (não encontrado) para todos os frames simultâneos
- Cada frame spawna um novo actor, causando duplicatas

---

## 🎯 **LOCALIZAÇÃO NO BLUEPRINT:**

### **PASSO 0: Abrir o Blueprint**

1. **Abra o Unreal Editor**
2. **Navegue até:** `Content Browser` → `BP_NetMovementClient`
3. **Duplo clique** em `BP_NetMovementClient` para abrir o Blueprint Editor
4. **No painel esquerdo**, encontre a função `ProcessNextFrame`
5. **Clique** em `ProcessNextFrame` para abrir o grafo da função

---

## 🔍 **ETAPA 1: Localizar o Nó `K2Node_IfThenElse_6`**

### **1.1. Identificar o Nó:**

1. **No grafo de `ProcessNextFrame`, procure por um nó `Branch` que:**
   - Está conectado a um `Greater or Equal` que compara `FoundIndex >= 0`
   - Tem dois pins de saída: `then` (True) e `else` (False)
   - O pin `else` está conectado a `SpawnActorFromClass`

2. **Este é o nó `K2Node_IfThenElse_6`**

### **1.2. Verificar Conexões Atuais:**

**Verifique se o pin `else` está conectado diretamente a `SpawnActorFromClass`:**
- Se sim, você precisa **DESCONECTAR** essa conexão temporariamente
- Clique com o botão direito na linha de conexão entre o pin `else` e `SpawnActorFromClass`
- Selecione **"Break Link"** ou **"Break Connection"**

**IMPORTANTE:** Você vai reconectar depois, mas primeiro precisa adicionar a verificação dupla.

---

## 🔧 **ETAPA 2: Adicionar Primeira Verificação Dupla (`Array_Find`)**

### **2.1. Adicionar o Nó `Array_Find`:**

1. **Clique com o botão direito** na área vazia após o pin `else` do `K2Node_IfThenElse_6`
2. **No menu de busca**, digite: `Array Find`
3. **Selecione:** `Array Find` (função genérica de array)
4. **O nó será criado automaticamente**

### **2.2. Configurar o Nó `Array_Find`:**

#### **2.2.1. Conectar o Array (`Target`):**

1. **Localize o pin `Target` do `Array_Find`**
   - É o pin de entrada mais à esquerda do nó
   - Geralmente tem um ícone de array `[]`

2. **Você precisa conectar `RemoteActorIds`:**

   **OPÇÃO A: Se já existe um `Get Variable` para `RemoteActorIds`:**
   - Localize o nó `Get Variable (RemoteActorIds)` no grafo
   - Conecte a saída desse nó ao pin `Target` do `Array_Find`
   - **Pin exato:** `Get Variable (RemoteActorIds)` → `Return Value` → `Array_Find` → `Target`

   **OPÇÃO B: Se não existe, criar um `Get Variable`:**
   - Clique com o botão direito na área vazia
   - Digite: `Get RemoteActorIds`
   - Selecione: `Get RemoteActorIds` (variável do Blueprint)
   - **Conecte:** `Get RemoteActorIds` → `Return Value` → `Array_Find` → `Target`

#### **2.2.2. Conectar o Item a Buscar (`Item to Find`):**

1. **Localize o pin `Item to Find` do `Array_Find`**
   - É o segundo pin de entrada do nó
   - Geralmente aparece como `Item to Find` ou `Item`

2. **Você precisa conectar `OutPlayerId`:**

   **Localize o nó `ParseStateUpdateFrame`:**
   - Procure por um nó chamado `ParseStateUpdateFrame` ou `Parse State Update Frame`
   - Este nó tem uma saída chamada `Out Player Id` (ou `OutPlayerId`)

3. **Conecte:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Array_Find` → `Item to Find`

#### **2.2.3. Configurar o Tipo do Array:**

1. **Clique no nó `Array_Find`**
2. **No painel de detalhes**, verifique se `Array Type` está configurado como `Integer`
3. Se não estiver, clique em `Array Type` e selecione `Integer`

### **2.3. Renomear o Nó (Opcional, mas Recomendado):**

1. **Clique no nó `Array_Find`**
2. **Pressione `F2`** ou clique com o botão direito → `Rename`
3. **Digite:** `Array_Find_Check` (para diferenciar do primeiro `Array_Find`)
4. **Pressione `Enter`**

### **2.4. Verificar a Saída:**

O nó `Array_Find` tem uma saída chamada `Found Index` (ou `Return Value`):
- **Esta saída retorna:** `-1` se não encontrado, ou um número `>= 0` se encontrado
- **Você vai usar esta saída no próximo passo**

---

## 🔧 **ETAPA 3: Adicionar `Greater or Equal` para Verificação**

### **3.1. Adicionar o Nó `Greater or Equal`:**

1. **Clique com o botão direito** após o nó `Array_Find_Check`
2. **No menu de busca**, digite: `Greater or Equal`
3. **Selecione:** `Greater or Equal (Integer)` (não Float!)
4. **O nó será criado automaticamente**

### **3.2. Configurar o Nó `Greater or Equal`:**

#### **3.2.1. Conectar `FoundIndexCheck` ao Pin `A`:**

1. **Localize o pin `A` do `Greater or Equal`**
   - É o primeiro pin de entrada (mais à esquerda)

2. **Conecte:**
   - **Pin exato:** `Array_Find_Check` → `Found Index` → `Greater or Equal` → `A`

#### **3.2.2. Conectar `0` ao Pin `B`:**

1. **Localize o pin `B` do `Greater or Equal`**
   - É o segundo pin de entrada

2. **Você precisa conectar o valor `0`:**

   **OPÇÃO A: Criar uma constante `0`:**
   - Clique com o botão direito na área vazia
   - Digite: `Make Integer 0` ou simplesmente `0`
   - Selecione: `Make Integer` ou `Integer`
   - Digite `0` no valor
   - **Conecte:** `Make Integer (0)` → `Return Value` → `Greater or Equal` → `B`

   **OPÇÃO B: Conectar diretamente:**
   - Alguns editores permitem conectar diretamente ao pin `B` e digitar `0`
   - Tente clicar no pin `B` e digitar `0` no campo de valor

### **3.3. Verificar a Saída:**

O nó `Greater or Equal` tem uma saída chamada `Return Value` (Boolean):
- **Esta saída retorna:** `true` se `A >= B`, `false` caso contrário
- **Você vai usar esta saída no próximo passo**

---

## 🔧 **ETAPA 4: Adicionar `Branch` para Decisão**

### **4.1. Adicionar o Nó `Branch`:**

1. **Clique com o botão direito** após o nó `Greater or Equal`
2. **No menu de busca**, digite: `Branch`
3. **Selecione:** `Branch` (não `Branch on Bool`!)
4. **O nó será criado automaticamente**

### **4.2. Configurar o Nó `Branch`:**

#### **4.2.1. Conectar `Return Value` do `Greater or Equal` ao Pin `Condition`:**

1. **Localize o pin `Condition` do `Branch`**
   - É o pin de entrada no topo do nó
   - Geralmente tem um ícone de `?` ou `Condition`

2. **Conecte:**
   - **Pin exato:** `Greater or Equal` → `Return Value` → `Branch` → `Condition`

### **4.3. Entender os Pins de Saída:**

O nó `Branch` tem dois pins de saída:
- **`True` (verde):** Executado quando `Condition == true` (ou seja, quando `FoundIndexCheck >= 0`)
- **`False` (vermelho):** Executado quando `Condition == false` (ou seja, quando `FoundIndexCheck < 0`)

**O que cada caminho significa:**
- **`True`:** O actor **JÁ EXISTE** (encontrado no array) → **PULAR SPAWN**, usar caminho `then`
- **`False`:** O actor **NÃO EXISTE** (não encontrado no array) → **CONTINUAR COM SPAWN**

---

## 🔧 **ETAPA 5: Conectar o Caminho `True` (Actor Já Existe)**

### **5.1. Adicionar `Get Array Item`:**

1. **Clique com o botão direito** após o pin `True` do `Branch`
2. **No menu de busca**, digite: `Get Array Item`
3. **Selecione:** `Get Array Item` (função genérica de array)
4. **O nó será criado automaticamente**

### **5.2. Configurar o Nó `Get Array Item`:**

#### **5.2.1. Conectar o Array (`Target`):**

1. **Localize o pin `Target` do `Get Array Item`**

2. **Você precisa conectar `RemoteActors`:**

   **OPÇÃO A: Se já existe um `Get Variable` para `RemoteActors`:**
   - Localize o nó `Get Variable (RemoteActors)` no grafo
   - **Conecte:** `Get Variable (RemoteActors)` → `Return Value` → `Get Array Item` → `Target`

   **OPÇÃO B: Se não existe, criar um `Get Variable`:**
   - Clique com o botão direito na área vazia
   - Digite: `Get RemoteActors`
   - Selecione: `Get RemoteActors` (variável do Blueprint)
   - **Conecte:** `Get RemoteActors` → `Return Value` → `Get Array Item` → `Target`

#### **5.2.2. Conectar o Índice (`Index`):**

1. **Localize o pin `Index` do `Get Array Item`**
   - É o segundo pin de entrada
   - Geralmente aparece como `Index` ou `Dimension 1`

2. **Conecte:**
   - **Pin exato:** `Array_Find_Check` → `Found Index` → `Get Array Item` → `Index`

### **5.3. Verificar a Saída:**

O nó `Get Array Item` tem uma saída chamada `Array Element` (ou `Return Value`):
- **Esta saída retorna:** O actor existente do array `RemoteActors` na posição `FoundIndexCheck`
- **Você vai usar esta saída no próximo passo**

---

## 🔧 **ETAPA 6: Adicionar `Set Variable: RemoteActorRef`**

### **6.1. Adicionar o Nó `Set Variable`:**

1. **Clique com o botão direito** após o nó `Get Array Item`
2. **No menu de busca**, digite: `Set RemoteActorRef`
3. **Selecione:** `Set RemoteActorRef` (variável do Blueprint)
4. **O nó será criado automaticamente**

### **6.2. Configurar o Nó `Set Variable`:**

#### **6.2.1. Conectar `Array Element` ao Pin `Value`:**

1. **Localize o pin `Value` do `Set Variable`**
   - É o pin de entrada (geralmente no topo ou à direita)

2. **Conecte:**
   - **Pin exato:** `Get Array Item` → `Array Element` → `Set Variable (RemoteActorRef)` → `Value`

### **6.3. Conectar `Set Variable` ao Fluxo de Atualização:**

Agora você precisa conectar `Set Variable` aos nós `Set Actor Location` e `Set Actor Rotation` que já existem no grafo:

1. **Localize os nós `Set Actor Location` e `Set Actor Rotation` no grafo**
   - Eles devem estar conectados ao caminho `else` original (antes da modificação)

2. **Conecte:**
   - **Pin exato:** `Set Variable (RemoteActorRef)` → `Output` → `Set Actor Location` → `Target`
   - **Pin exato:** `Set Variable (RemoteActorRef)` → `Output` → `Set Actor Rotation` → `Target`

**IMPORTANTE:** Se `Set Actor Location` e `Set Actor Rotation` já estavam conectados ao caminho `else` original, você pode precisar desconectá-los temporariamente e reconectá-los ao novo caminho `True`.

---

## 🔧 **ETAPA 7: Conectar o Caminho `False` (Actor Não Existe - Spawnar)**

### **7.1. Reconectar `SpawnActorFromClass`:**

1. **Localize o nó `SpawnActorFromClass` no grafo**
   - Ele deve estar desconectado do pin `else` do `K2Node_IfThenElse_6` (você desconectou no Passo 0)

2. **Conecte:**
   - **Pin exato:** `Branch` → `False` → `SpawnActorFromClass` → `Input` (ou `Execute`)

**IMPORTANTE:** Verifique se `SpawnActorFromClass` está configurado corretamente:
- **Class:** `BP_RemotePlayer_C` (ou a classe correta do actor remoto)
- **CollisionHandlingOverride:** `Always Spawn`
- **Transform:** Deve estar conectado a `OutLocation` e `OutRotation` do `ParseStateUpdateFrame`

### **7.2. Verificar `Set Variable: RemoteActorRef` Após Spawn:**

1. **Localize o nó `Set Variable (RemoteActorRef)` que vem após `SpawnActorFromClass`**

2. **Verifique se está conectado:**
   - **Pin exato:** `SpawnActorFromClass` → `Return Value` → `Set Variable (RemoteActorRef)` → `Value`

---

## 🔧 **ETAPA 8: Adicionar Segunda Verificação Antes de `Array_Add`**

### **8.1. Adicionar Segundo `Array_Find`:**

1. **Clique com o botão direito** após o nó `Set Variable (RemoteActorRef)` (que vem após `SpawnActorFromClass`)
2. **No menu de busca**, digite: `Array Find`
3. **Selecione:** `Array Find` (função genérica de array)
4. **O nó será criado automaticamente**

### **8.2. Configurar o Segundo `Array_Find`:**

#### **8.2.1. Conectar o Array (`Target`):**

1. **Conecte `RemoteActorIds` ao pin `Target`:**
   - **Pin exato:** `Get RemoteActorIds` → `Return Value` → `Array_Find` → `Target`

#### **8.2.2. Conectar o Item a Buscar (`Item to Find`):**

1. **Conecte `OutPlayerId` ao pin `Item to Find`:**
   - **Pin exato:** `ParseStateUpdateFrame` → `Out Player Id` → `Array_Find` → `Item to Find`

### **8.3. Renomear o Nó:**

1. **Clique no nó `Array_Find`**
2. **Pressione `F2`** ou clique com o botão direito → `Rename`
3. **Digite:** `Array_Find_BeforeAdd` (para diferenciar)
4. **Pressione `Enter`**

### **8.4. Adicionar `Less` para Verificação:**

1. **Clique com o botão direito** após o nó `Array_Find_BeforeAdd`
2. **No menu de busca**, digite: `Less`
3. **Selecione:** `Less (Integer)` (não Float!)
4. **O nó será criado automaticamente**

### **8.5. Configurar o Nó `Less`:**

#### **8.5.1. Conectar `CheckIndex` ao Pin `A`:**

1. **Conecte:**
   - **Pin exato:** `Array_Find_BeforeAdd` → `Found Index` → `Less` → `A`

#### **8.5.2. Conectar `0` ao Pin `B`:**

1. **Conecte:**
   - **Pin exato:** `Make Integer (0)` → `Return Value` → `Less` → `B`

**Isso verifica se `FoundIndex < 0` (ou seja, se o actor ainda não existe no array).**

### **8.6. Adicionar Segundo `Branch`:**

1. **Clique com o botão direito** após o nó `Less`
2. **No menu de busca**, digite: `Branch`
3. **Selecione:** `Branch`
4. **O nó será criado automaticamente**

### **8.7. Configurar o Segundo `Branch`:**

1. **Conecte:**
   - **Pin exato:** `Less` → `Return Value` → `Branch` → `Condition`

**O que cada caminho significa:**
- **`True`:** `FoundIndex < 0` (actor não existe) → **EXECUTAR `Array_Add`**
- **`False`:** `FoundIndex >= 0` (actor já existe) → **PULAR `Array_Add`**, log de erro

---

## 🔧 **ETAPA 9: Conectar `Array_Add` no Caminho `True`**

### **9.1. Localizar os Nós `Array_Add`:**

1. **Procure por dois nós `Array_Add` no grafo:**
   - Um para `RemoteActorIds`
   - Um para `RemoteActors`

2. **Se não existirem, crie-os:**

   **Para `RemoteActorIds`:**
   - Clique com o botão direito na área vazia
   - Digite: `Array Add`
   - Selecione: `Array Add` (função genérica de array)
   - Configure `Array Type` como `Integer`
   - Conecte `Get RemoteActorIds` ao pin `Target`
   - Conecte `OutPlayerId` ao pin `Item`

   **Para `RemoteActors`:**
   - Clique com o botão direito na área vazia
   - Digite: `Array Add`
   - Selecione: `Array Add` (função genérica de array)
   - Configure `Array Type` como `Object` ou `Actor Reference`
   - Conecte `Get RemoteActors` ao pin `Target`
   - Conecte `Get RemoteActorRef` (ou `Set Variable` output) ao pin `Item`

### **9.2. Conectar `Array_Add` ao Pin `True` do Segundo `Branch`:**

1. **Conecte:**
   - **Pin exato:** `Branch` → `True` → `Array_Add (RemoteActorIds)` → `Input` (ou `Execute`)

2. **Conecte `Array_Add (RemoteActorIds)` a `Array_Add (RemoteActors)`:**
   - **Pin exato:** `Array_Add (RemoteActorIds)` → `Output` → `Array_Add (RemoteActors)` → `Input` (ou `Execute`)

### **9.3. Conectar `Array_Add` ao Fluxo de Atualização:**

1. **Conecte `Array_Add (RemoteActors)` a `Set Actor Location`:**
   - **Pin exato:** `Array_Add (RemoteActors)` → `Output` → `Set Actor Location` → `Input` (ou `Execute`)

**IMPORTANTE:** Verifique se `Set Actor Location` e `Set Actor Rotation` estão conectados corretamente:
- **`Set Actor Location`:**
  - **Target:** `Get RemoteActorRef` (ou `Set Variable` output)
  - **New Location:** `ParseStateUpdateFrame` → `Out Location`

- **`Set Actor Rotation`:**
  - **Target:** `Get RemoteActorRef` (ou `Set Variable` output)
  - **New Rotation:** `ParseStateUpdateFrame` → `Out Yaw Degrees` (convertido para rotator)

---

## 🔧 **ETAPA 10: Adicionar Log de Erro no Caminho `False`**

### **10.1. Adicionar `Print String`:**

1. **Clique com o botão direito** após o pin `False` do segundo `Branch`
2. **No menu de busca**, digite: `Print String`
3. **Selecione:** `Print String`
4. **O nó será criado automaticamente**

### **10.2. Configurar o `Print String`:**

1. **No campo `In String`, digite:**
   ```
   Warning: Actor já existe mas FoundIndex era -1!
   ```

2. **Conecte:**
   - **Pin exato:** `Branch` → `False` → `Print String` → `Input` (ou `Execute`)

**Este log ajudará a diagnosticar se há uma condição de corrida ocorrendo.**

---

## 🔧 **ETAPA 11: Verificar Convergência dos Caminhos**

### **11.1. Verificar se Ambos os Caminhos Convergem:**

Ambos os caminhos (`True` e `False` do primeiro `Branch`) devem convergir em:
- `Set Actor Location`
- `Set Actor Rotation`

**Verifique:**
- Caminho `True` (actor já existe): `Get Array Item` → `Set Variable` → `Set Actor Location` → `Set Actor Rotation`
- Caminho `False` (actor não existe): `SpawnActorFromClass` → `Set Variable` → `Array_Add` → `Set Actor Location` → `Set Actor Rotation`

### **11.2. Verificar Conexões Finais:**

1. **Verifique se `Set Actor Location` está conectado a `Set Actor Rotation`:**
   - **Pin exato:** `Set Actor Location` → `Output` → `Set Actor Rotation` → `Input` (ou `Execute`)

2. **Verifique se `Set Actor Rotation` está conectado ao segundo `ProcessBinaryBuffer`:**
   - **Pin exato:** `Set Actor Rotation` → `Output` → `Make Array` → `Input` (ou `Execute`)

---

## 🔧 **ETAPA 12: Verificar Limpeza do Buffer (Opcional)**

### **12.1. Adicionar Log para Verificar Buffer:**

1. **Após o segundo `ProcessBinaryBuffer` (no final de `ProcessNextFrame`), adicione:**

   **Adicionar `Get Array Length`:**
   - Clique com o botão direito após `ProcessBinaryBuffer`
   - Digite: `Get Array Length`
   - Selecione: `Get Array Length`
   - Conecte `Get BinaryMessageBuffer` ao pin `Target`

   **Adicionar `Format Text`:**
   - Clique com o botão direito após `Get Array Length`
   - Digite: `Format Text`
   - Selecione: `Format Text`
   - No campo `Format`, digite: `Buffer após processar: {0} bytes`
   - Conecte `Get Array Length` → `Length` → `Format Text` → `{0}`

   **Adicionar `Print String`:**
   - Clique com o botão direito após `Format Text`
   - Digite: `Print String`
   - Selecione: `Print String`
   - Conecte `Format Text` → `Result` → `Print String` → `In String`

---

## ✅ **CHECKLIST FINAL DE VERIFICAÇÃO:**

### **Verificações de Conexões:**

- [ ] Pin `else` de `K2Node_IfThenElse_6` conectado ao primeiro `Array_Find_Check`?
- [ ] `Array_Find_Check` → `Target` conectado a `Get RemoteActorIds`?
- [ ] `Array_Find_Check` → `Item to Find` conectado a `ParseStateUpdateFrame` → `Out Player Id`?
- [ ] `Array_Find_Check` → `Found Index` conectado a `Greater or Equal` → `A`?
- [ ] `Greater or Equal` → `A` conectado a `FoundIndexCheck`?
- [ ] `Greater or Equal` → `B` conectado a `Make Integer (0)`?
- [ ] `Greater or Equal` → `Return Value` conectado a `Branch` → `Condition`?
- [ ] `Branch` → `True` conectado a `Get Array Item` → `Input`?
- [ ] `Get Array Item` → `Target` conectado a `Get RemoteActors`?
- [ ] `Get Array Item` → `Index` conectado a `Array_Find_Check` → `Found Index`?
- [ ] `Get Array Item` → `Array Element` conectado a `Set Variable (RemoteActorRef)` → `Value`?
- [ ] `Set Variable (RemoteActorRef)` → `Output` conectado a `Set Actor Location` → `Target`?
- [ ] `Branch` → `False` conectado a `SpawnActorFromClass` → `Input`?
- [ ] `SpawnActorFromClass` → `Return Value` conectado a `Set Variable (RemoteActorRef)` → `Value`?
- [ ] `Set Variable (RemoteActorRef)` → `Output` conectado a `Array_Find_BeforeAdd` → `Input`?
- [ ] `Array_Find_BeforeAdd` → `Target` conectado a `Get RemoteActorIds`?
- [ ] `Array_Find_BeforeAdd` → `Item to Find` conectado a `ParseStateUpdateFrame` → `Out Player Id`?
- [ ] `Array_Find_BeforeAdd` → `Found Index` conectado a `Less` → `A`?
- [ ] `Less` → `A` conectado a `FoundIndexCheck`?
- [ ] `Less` → `B` conectado a `Make Integer (0)`?
- [ ] `Less` → `Return Value` conectado a segundo `Branch` → `Condition`?
- [ ] Segundo `Branch` → `True` conectado a `Array_Add (RemoteActorIds)` → `Input`?
- [ ] `Array_Add (RemoteActorIds)` → `Output` conectado a `Array_Add (RemoteActors)` → `Input`?
- [ ] `Array_Add (RemoteActors)` → `Output` conectado a `Set Actor Location` → `Input`?
- [ ] Segundo `Branch` → `False` conectado a `Print String` → `Input`?
- [ ] `Set Actor Location` → `Target` conectado a `Get RemoteActorRef`?
- [ ] `Set Actor Location` → `New Location` conectado a `ParseStateUpdateFrame` → `Out Location`?
- [ ] `Set Actor Rotation` → `Target` conectado a `Get RemoteActorRef`?
- [ ] `Set Actor Rotation` → `New Rotation` conectado a `Out Yaw Degrees` (convertido para rotator)?

---

## 🎯 **DIAGRAMA FINAL DE CONEXÕES:**

```
K2Node_IfThenElse_6 (else)
  ↓ Execute
Array_Find_Check
  ├─ Target ← Get RemoteActorIds → Return Value
  └─ Item to Find ← ParseStateUpdateFrame → Out Player Id
  ↓ Found Index
Greater or Equal
  ├─ A ← Array_Find_Check → Found Index
  └─ B ← Make Integer (0) → Return Value
  ↓ Return Value
Branch
  ├─ True → Get Array Item
  │           ├─ Target ← Get RemoteActors → Return Value
  │           └─ Index ← Array_Find_Check → Found Index
  │           ↓ Array Element
  │           Set Variable (RemoteActorRef)
  │           ├─ Value ← Get Array Item → Array Element
  │           ↓ Output
  │           Set Actor Location
  │           └─ Target ← Set Variable (RemoteActorRef) → Output
  │
  └─ False → SpawnActorFromClass
              ↓ Return Value
              Set Variable (RemoteActorRef)
              ├─ Value ← SpawnActorFromClass → Return Value
              ↓ Output
              Array_Find_BeforeAdd
              ├─ Target ← Get RemoteActorIds → Return Value
              └─ Item to Find ← ParseStateUpdateFrame → Out Player Id
              ↓ Found Index
              Less
              ├─ A ← Array_Find_BeforeAdd → Found Index
              └─ B ← Make Integer (0) → Return Value
              ↓ Return Value
              Branch
              ├─ True → Array_Add (RemoteActorIds)
              │           ├─ Target ← Get RemoteActorIds → Return Value
              │           └─ Item ← ParseStateUpdateFrame → Out Player Id
              │           ↓ Output
              │           Array_Add (RemoteActors)
              │           ├─ Target ← Get RemoteActors → Return Value
              │           └─ Item ← Get RemoteActorRef → Return Value
              │           ↓ Output
              │           Set Actor Location
              │
              └─ False → Print String
                          └─ In String ← "Warning: Actor já existe mas FoundIndex era -1!"
```

---

## 📝 **NOTAS IMPORTANTES:**

1. **Ordem de Execução:**
   - O fluxo sempre verifica primeiro se o actor existe antes de spawnar
   - Se o actor já existe, usa o caminho `True` (atualização)
   - Se o actor não existe, usa o caminho `False` (spawn)

2. **Condição de Corrida:**
   - A verificação dupla (`Array_Find_Check`) garante que mesmo se múltiplos frames chegarem simultaneamente, apenas um spawn será executado
   - A verificação antes de `Array_Add` (`Array_Find_BeforeAdd`) evita adicionar duplicatas mesmo se houver uma condição de corrida

3. **Performance:**
   - As verificações adicionais têm um custo mínimo de performance
   - O benefício de evitar múltiplos spawns supera o custo adicional

---

**Fim do Guia Detalhado**

