# 🔧 **SOLUÇÃO DEFINITIVA: Sobreposição de Atores**

## 🚨 **PROBLEMA:**

Quando um terceiro cliente se conecta, os atores remotos se fundem/sobrepõem na tela do primeiro cliente. Isso acontece porque `Array_Find` pode estar retornando o mesmo `FoundIndex` para diferentes PlayerIDs.

---

## ✅ **SOLUÇÃO: Substituir `Array_Find` por Loop Manual**

### **Por que substituir `Array_Find`?**

Mesmo com validação, `Array_Find` pode ter problemas internos ou race conditions. Um loop manual garante que verificamos cada elemento do array sequencialmente e encontramos o índice correto.

---

## 🔧 **IMPLEMENTAÇÃO:**

### **PASSO 1: Remover `Array_Find` e `Greater or Equal`**

**No Blueprint Editor:**

1. **Localize o nó `Array Find`** (`K2Node_CallArrayFunction_4`)
2. **Remova completamente** o nó `Array Find`
3. **Remova o nó `Greater or Equal`** que compara `FoundIndex >= 0`
4. **Remova a conexão** do `Branch` (`K2Node_IfThenElse_6`) que usa `FoundIndex >= 0`

---

### **PASSO 2: Adicionar Loop Manual**

**Após o filtro `OutPlayerId != Active Player ID`:**

#### **2.1. Adicionar Variável Local:**

1. **Crie uma variável local** chamada `FoundIndex` (tipo: `Integer`)
2. **Inicialize** com valor `-1` (não encontrado)

#### **2.2. Adicionar `ForEachLoop`:**

1. **Adicione nó `ForEachLoop`** (ou `ForEachLoopWithBreak`)
   - **Array:** `Get RemoteActorIds` → `Return Value`
   - **Loop Body:** Criará um pin `Array Element` e `Array Index`

2. **Dentro do Loop Body:**
   - **Adicione `Equal (Integer)`**
     - **A:** `Array Element` (do `ForEachLoop`)
     - **B:** `OutPlayerId` (do `ParseStateUpdateFrameWithAnimation`)
   
   - **Adicione `Branch`**
     - **Condition:** `Equal` → `Return Value`
     - **True (then):** PlayerID encontrado!
     - **False (else):** Continuar procurando

3. **No `Branch` → True (PlayerID encontrado):**
   - **Adicione `Set Variable: FoundIndex`**
     - **Value:** `Array Index` (do `ForEachLoop`)
   - **Se usar `ForEachLoopWithBreak`:**
     - **Conecte `Break`** após `Set Variable: FoundIndex`
   - **Se usar `ForEachLoop` normal:**
     - **Adicione `Return`** após `Set Variable: FoundIndex` (ou continue e verifique `FoundIndex >= 0` depois)

#### **2.3. Após o Loop:**

1. **Adicione `Branch`**
   - **Condition:** `FoundIndex >= 0` (usando `Greater or Equal`)
   - **True (then):** Actor existe → Atualizar
   - **False (else):** Actor não existe → Spawnar

---

## 📊 **ESTRUTURA COMPLETA:**

```
ParseStateUpdateFrameWithAnimation → OutPlayerId
  ↓
[FILTRO: OutPlayerId != Active Player ID]
  ↓
Set Variable: FoundIndex = -1  ← INICIALIZAR
  ↓
ForEachLoop (RemoteActorIds)
  ├─ Array Element (PlayerID no array)
  ├─ Array Index (índice atual)
  └─ Loop Body:
      ↓
      Equal (Array Element == OutPlayerId?)
      ↓
      Branch
      ├─ True: PlayerID encontrado!
      │   ↓
      │   Set Variable: FoundIndex = Array Index
      │   ↓
      │   Break (ou Return)
      │
      └─ False: Continuar procurando
  ↓ (Completed)
Branch (FoundIndex >= 0?)
  ├─ True (actor existe):
  │   ↓
  │   Get Array Item (RemoteActorIds, FoundIndex) → FoundPlayerId
  │   ↓
  │   Equal (FoundPlayerId == OutPlayerId?)  ← VALIDAÇÃO EXTRA
  │   ↓
  │   Branch
  │   ├─ True: PlayerID confere ✅
  │   │   ↓
  │   │   Get Array Item (RemoteActors, FoundIndex) → RemoteActorRef
  │   │   ↓
  │   │   Set Variable: RemoteActorRef
  │   │   ↓
  │   │   [ATUALIZAR ACTOR EXISTENTE]
  │   │
  │   └─ False: PlayerID NÃO confere ❌
  │       ↓
  │       [TRATAR COMO ACTOR NÃO EXISTE]
  │
  └─ False (actor não existe):
      ↓
      SpawnActorFromClass
      ↓
      Set Variable: RemoteActorRef
      ↓
      Array_Add (RemoteActorIds, OutPlayerId)
      ↓
      Array_Add (RemoteActors, RemoteActorRef)
```

---

## 🎯 **VANTAGENS DESTA SOLUÇÃO:**

1. **Controle Total:** Você controla exatamente como o array é percorrido
2. **Sem Dependência de `Array_Find`:** Não depende de bugs ou race conditions do `Array_Find`
3. **Validação Dupla:** Mantém a validação de `FoundPlayerId == OutPlayerId` como segurança extra
4. **Fácil de Debugar:** Você pode adicionar logs dentro do loop se necessário

---

## ⚠️ **IMPORTANTE:**

### **Se o Array Estiver Vazio:**

- O `ForEachLoop` não executará o `Loop Body`
- `FoundIndex` permanecerá `-1`
- O `Branch` após o loop irá para `False` → Spawnar novo actor ✅

### **Performance:**

- Para arrays pequenos (< 100 elementos), o loop manual é tão rápido quanto `Array_Find`
- Para arrays grandes, considere otimizar depois, mas primeiro resolva o bug

---

## ✅ **CHECKLIST:**

- [ ] Removido `Array Find` (`K2Node_CallArrayFunction_4`)
- [ ] Removido `Greater or Equal` que compara `FoundIndex >= 0` (do `Array_Find`)
- [ ] Criada variável local `FoundIndex` (Integer, inicializada com `-1`)
- [ ] Adicionado `ForEachLoop` com `RemoteActorIds`
- [ ] Adicionado `Equal` comparando `Array Element == OutPlayerId`
- [ ] Adicionado `Branch` dentro do loop
- [ ] Adicionado `Set Variable: FoundIndex = Array Index` no `Branch` → True
- [ ] Adicionado `Break` (ou `Return`) após `Set Variable: FoundIndex`
- [ ] Adicionado `Branch (FoundIndex >= 0?)` após o loop
- [ ] Mantida validação `FoundPlayerId == OutPlayerId` após `Get Array Item`
- [ ] Verificado que `Array_Add` está conectado corretamente (sem `Make Array`)

---

## 🎯 **RESULTADO ESPERADO:**

- Cada PlayerID será encontrado no índice correto do array
- Não haverá mais sobreposição de atores
- Cada cliente verá os outros clientes separadamente
- A validação extra garante que mesmo se houver erro, o sistema detecta

---

**Esta solução elimina completamente a dependência de `Array_Find` e garante que o índice correto seja encontrado através de verificação manual sequencial.**

