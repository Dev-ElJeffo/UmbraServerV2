# 🎯 **GUIA VISUAL SIMPLIFICADO: Substituir Array_Find por Loop Manual**

## 📋 **OBJETIVO:**

Substituir o nó `Array Find` por um `ForEachLoop` manual que verifica cada elemento do array `RemoteActorIds` sequencialmente.

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Preparação**

1. **Abra `BP_NetMovementClient`**
2. **Abra a função `ProcessNextFrame`**
3. **Localize o nó `Array Find`** (procure por `K2Node_CallArrayFunction_4` ou nó com função `Array Find`)

---

### **PASSO 2: Remover Array_Find**

1. **Selecione o nó `Array Find`**
2. **Pressione `Delete`** para removê-lo
3. **Remova também:**
   - O nó `Greater or Equal` que compara `FoundIndex >= 0`
   - As conexões que vão do `Array Find` para outros nós

---

### **PASSO 3: Criar Variável Local**

1. **No painel `My Blueprint` → `Local Variables`:**
   - Clique em `+ Variable`
   - Nome: `FoundIndex`
   - Tipo: `Integer`
   - Valor padrão: `-1`

---

### **PASSO 4: Adicionar Inicialização**

**Após o filtro `OutPlayerId != Active Player ID`:**

1. **Adicione nó `Set Variable: FoundIndex`**
   - **Value:** `-1` (constante Integer)
   - **Execute:** Conecte ao fluxo após o filtro

---

### **PASSO 5: Adicionar ForEachLoop**

1. **Adicione nó `ForEachLoop`** (ou `ForEachLoopWithBreak`)
   - **Array:** Conecte a `Get RemoteActorIds` → `Return Value`
   - **Execute:** Conecte ao `then` do `Set Variable: FoundIndex`

2. **O `ForEachLoop` criará automaticamente:**
   - Pin `Array Element` (Integer) - cada PlayerID no array
   - Pin `Array Index` (Integer) - índice atual (0, 1, 2...)
   - Pin `Loop Body` (Execute) - onde você coloca a lógica de comparação
   - Pin `Completed` (Execute) - executado quando o loop termina

---

### **PASSO 6: Adicionar Comparação Dentro do Loop**

**Dentro do `Loop Body` do `ForEachLoop`:**

1. **Adicione nó `Equal (Integer)`**
   - **A:** Conecte ao `Array Element` (do `ForEachLoop`)
   - **B:** Conecte ao `OutPlayerId` (do `ParseStateUpdateFrameWithAnimation`)

2. **Adicione nó `Branch`**
   - **Condition:** Conecte ao `Return Value` do `Equal`
   - **True (then):** PlayerID encontrado!
   - **False (else):** Continuar procurando (pode deixar desconectado ou conectar ao `Completed` do loop)

---

### **PASSO 7: Salvar o Índice Quando Encontrar**

**No `Branch` → True (PlayerID encontrado):**

1. **Adicione nó `Set Variable: FoundIndex`**
   - **Value:** Conecte ao `Array Index` (do `ForEachLoop`)
   - **Execute:** Conecte ao `then` do `Branch`

2. **Se usar `ForEachLoopWithBreak`:**
   - **Adicione nó `Break`**
   - **Execute:** Conecte ao `then` do `Set Variable: FoundIndex`
   - **Break:** Conecte ao pin `Break` do `ForEachLoopWithBreak`

3. **Se usar `ForEachLoop` normal:**
   - **Não precisa de `Break`**, o loop continuará mas você verificará `FoundIndex >= 0` depois

---

### **PASSO 8: Verificar Resultado Após o Loop**

**No pin `Completed` do `ForEachLoop`:**

1. **Adicione nó `Greater or Equal (Integer)`**
   - **A:** Conecte a `Get FoundIndex` (variável local)
   - **B:** `0` (constante Integer)

2. **Adicione nó `Branch`** (ou use o `K2Node_IfThenElse_6` existente)
   - **Condition:** Conecte ao `Return Value` do `Greater or Equal`
   - **True (then):** Actor existe → Atualizar
   - **False (else):** Actor não existe → Spawnar

---

### **PASSO 9: Conectar ao Fluxo Existente**

**No `Branch` → True (actor existe):**

1. **Mantenha a validação existente:**
   - `Get Array Item (RemoteActorIds, FoundIndex)` → `FoundPlayerId`
   - `Equal (FoundPlayerId == OutPlayerId?)`
   - `Branch` com validação

2. **Conecte o `then` do `Branch` principal ao início da validação**

**No `Branch` → False (actor não existe):**

1. **Conecte ao fluxo de spawn existente:**
   - `SpawnActorFromClass`
   - `Set Variable: RemoteActorRef`
   - `Array_Add (RemoteActorIds, OutPlayerId)`
   - `Array_Add (RemoteActors, RemoteActorRef)`

---

## 📊 **ESTRUTURA VISUAL:**

```
[Filtro: OutPlayerId != Active Player ID]
  ↓ then
[Set Variable: FoundIndex = -1]
  ↓ then
[ForEachLoop (RemoteActorIds)]
  ├─ Loop Body:
  │   ↓
  │   [Equal: Array Element == OutPlayerId?]
  │   ↓
  │   [Branch]
  │   ├─ True:
  │   │   ↓
  │   │   [Set Variable: FoundIndex = Array Index]
  │   │   ↓
  │   │   [Break] (se usar ForEachLoopWithBreak)
  │   │
  │   └─ False: (continuar loop)
  │
  └─ Completed:
      ↓
      [Greater or Equal: FoundIndex >= 0?]
      ↓
      [Branch]
      ├─ True: [Atualizar actor existente]
      └─ False: [Spawnar novo actor]
```

---

## ✅ **VERIFICAÇÃO FINAL:**

- [ ] `Array Find` foi removido completamente
- [ ] Variável local `FoundIndex` criada (tipo Integer, valor padrão -1)
- [ ] `Set Variable: FoundIndex = -1` adicionado antes do loop
- [ ] `ForEachLoop` adicionado com `RemoteActorIds` como Array
- [ ] `Equal` dentro do loop comparando `Array Element == OutPlayerId`
- [ ] `Branch` dentro do loop
- [ ] `Set Variable: FoundIndex = Array Index` no `Branch` → True
- [ ] `Break` adicionado (se usar `ForEachLoopWithBreak`)
- [ ] `Greater or Equal: FoundIndex >= 0` após o loop
- [ ] `Branch` conectado corretamente ao fluxo existente
- [ ] Validação `FoundPlayerId == OutPlayerId` mantida
- [ ] Fluxo de spawn conectado ao `Branch` → False

---

## 🎯 **RESULTADO:**

Agora o sistema verificará cada elemento do array `RemoteActorIds` sequencialmente até encontrar o PlayerID correto, garantindo que o índice retornado seja sempre o correto e eliminando a sobreposição de atores.

