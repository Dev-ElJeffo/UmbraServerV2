# 🔍 **CORREÇÃO DA ANÁLISE: Pin `then` Está Conectado, Mas à Lógica Incorreta**

## 📋 **PROBLEMA REAL IDENTIFICADO:**

Você está **correto** - o pin `then` do `K2Node_IfThenElse_6` **ESTÁ CONECTADO** na imagem. No entanto, ele está conectado à **LÓGICA INCORRETA**.

---

## 🚨 **PROBLEMA CRÍTICO:**

### **O que está acontecendo AGORA:**

```
K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)
  │
  ├─ then (True - Actor existe): ✅ CONECTADO
  │   │
  │   └─ Set Variable: RemoteActorRef ← **PROBLEMA: Mesmo nó usado para spawn!**
  │       │
  │       └─ Array_Add (RemoteActorIds, OutPlayerId) ← **PROBLEMA: Adiciona novamente!**
  │       │
  │       └─ Array_Add (RemoteActors, RemoteActorRef) ← **PROBLEMA: Adiciona novamente!**
  │
  └─ else (False - Actor não existe):
      │
      └─ SpawnActorFromClass
          │
          └─ Set Variable: RemoteActorRef ← **MESMO NÓ!**
              │
              └─ Array_Add (RemoteActorIds, OutPlayerId)
              │
              └─ Array_Add (RemoteActors, RemoteActorRef)
```

**RESULTADO:**
- ✅ O pin `then` está conectado fisicamente
- ❌ Mas ele está usando o **mesmo `Set Variable: RemoteActorRef`** que é usado para novos atores
- ❌ Ele está executando **`Array_Add`** mesmo quando o ator já existe
- ❌ **Não há `Get Array Item`** para recuperar o ator existente do array `RemoteActors`
- ❌ O ator existente **não está sendo atualizado**, apenas sendo adicionado novamente aos arrays (causando duplicatas)

---

## ✅ **SOLUÇÃO CORRETA:**

### **O que DEVE acontecer:**

```
K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)
  │
  ├─ then (True - Actor existe):
  │   │
  │   ├─ Get Array Item (RemoteActors, FoundIndex) ← **ADICIONAR ESTE NÓ!**
  │   │   │
  │   │   └─ Output: ExistingActorRef (o ator que já existe)
  │   │
  │   ├─ Set Variable: RemoteActorRef = ExistingActorRef ← **NOVO nó, não reutilizar o do spawn!**
  │   │
  │   └─ [PULAR Array_Add] ← **NÃO executar Array_Add aqui!**
  │       │
  │       └─ Ir direto para Set Actor Location/Rotation
  │
  └─ else (False - Actor não existe):
      │
      └─ SpawnActorFromClass
          │
          ├─ Set Variable: RemoteActorRef = NewActorRef (do spawn)
          │
          ├─ Array_Add (RemoteActorIds, OutPlayerId) ← **Só aqui!**
          │
          ├─ Array_Add (RemoteActors, RemoteActorRef) ← **Só aqui!**
          │
          └─ Set Actor Location/Rotation
```

---

## 🎯 **CORREÇÃO PASSO A PASSO:**

### **PASSO 1: Desconectar o Pin `then` Atual**

1. **Localize o pin `then` do `K2Node_IfThenElse_6`**
2. **Desconecte a linha** que vai para o `Set Variable: RemoteActorRef` atual
   - Clique na linha e pressione `Delete` ou clique com botão direito → `Break Links`

---

### **PASSO 2: Adicionar `Get Array Item`**

1. **Clique com botão direito** após o pin `then` do `K2Node_IfThenElse_6`
2. **Procure por:** `Get Array Item`
3. **Configure:**
   - **Array:** Conecte a `Get Variable: RemoteActors` (ou variável `RemoteActors`)
   - **Index:** Conecte ao `FoundIndex` do `Array_Find` (nó `K2Node_CallArrayFunction_4`)
   - **Output:** `ExistingActorRef` (o ator que já existe no array)

---

### **PASSO 3: Adicionar Novo `Set Variable: RemoteActorRef` (Específico para Actor Existente)**

1. **Clique com botão direito** após o `Get Array Item`
2. **Procure por:** `Set Variable` → Selecione `RemoteActorRef`
3. **Configure:**
   - **Variable:** `RemoteActorRef`
   - **Value:** Conecte ao `Output` do `Get Array Item` (`ExistingActorRef`)
   - **Execute:** Conecte ao pin de execução do `Get Array Item`

**IMPORTANTE:** Este é um **NOVO** nó `Set Variable`, diferente do que é usado após o spawn.

---

### **PASSO 4: Conectar Diretamente à Lógica de Atualização (SEM Array_Add)**

1. **Localize os nós `Set Actor Location` e `Set Actor Rotation`**
   - Eles devem estar após o `SpawnActorFromClass` no caminho `else`
2. **Conecte o pin de execução do `Set Variable: RemoteActorRef`** (do caminho `then`) **diretamente** aos nós `Set Actor Location` e `Set Actor Rotation`
   - **NÃO conecte aos `Array_Add`** - estes devem ser executados apenas no caminho `else` (spawn)

**Verificação:**
- O caminho `then` (ator existe) deve **pular** os `Array_Add`
- O caminho `then` deve ir direto para `Set Actor Location` e `Set Actor Rotation`

---

### **PASSO 5: Verificar Convergência**

**Fluxo final esperado:**

```
K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)
  │
  ├─ then (True - Actor existe):
  │   │
  │   ├─ Get Array Item (RemoteActors, FoundIndex) → ExistingActorRef
  │   │
  │   ├─ Set Variable: RemoteActorRef = ExistingActorRef
  │   │
  │   └─ [PULAR Array_Add] ← **NÃO executar aqui!**
  │       │
  │       └─ Set Actor Location (RemoteActorRef, OutLocation)
  │       │
  │       └─ Set Actor Rotation (RemoteActorRef, OutRotation)
  │
  └─ else (False - Actor não existe):
      │
      └─ SpawnActorFromClass → NewActorRef
          │
          ├─ Set Variable: RemoteActorRef = NewActorRef
          │
          ├─ Array_Add (RemoteActorIds, OutPlayerId) ← **Só aqui!**
          │
          ├─ Array_Add (RemoteActors, RemoteActorRef) ← **Só aqui!**
          │
          └─ Set Actor Location (RemoteActorRef, OutLocation)
          │
          └─ Set Actor Rotation (RemoteActorRef, OutRotation)

[CONVERGÊNCIA] Ambos os caminhos terminam em Set Actor Location/Rotation
```

---

## 🔍 **VERIFICAÇÃO VISUAL:**

### **O que procurar na imagem:**

1. **Pin `then` do `K2Node_IfThenElse_6`:**
   - ✅ Está conectado (como você observou)
   - ❌ Mas está conectado diretamente ao `Set Variable: RemoteActorRef` que também é usado após o spawn
   - ❌ Está executando `Array_Add` mesmo quando o ator existe

2. **Falta:**
   - ❌ `Get Array Item` no caminho `then`
   - ❌ Lógica separada para atualizar o ator existente (sem `Array_Add`)

---

## 📊 **COMPARAÇÃO: ANTES vs DEPOIS**

### **ANTES (Atual - Incorreto):**

```
FoundIndex >= 0? (True)
  ↓
Set Variable: RemoteActorRef ← **Problema: Mesmo nó usado para spawn e atualização**
  ↓
Array_Add (RemoteActorIds) ← **Problema: Adiciona novamente**
  ↓
Array_Add (RemoteActors) ← **Problema: Adiciona novamente**
  ↓
Set Actor Location/Rotation ← **Pode não estar conectado ou estar errado**
```

**Problemas:**
- Actor existente não é recuperado do array
- Actor é adicionado novamente aos arrays (duplicatas)
- Actor pode não estar sendo atualizado corretamente

---

### **DEPOIS (Correto):**

```
FoundIndex >= 0? (True)
  ↓
Get Array Item (RemoteActors, FoundIndex) ← **NOVO: Recupera o ator existente**
  ↓
Set Variable: RemoteActorRef = ExistingActorRef ← **NOVO: Define o ator existente**
  ↓
[PULAR Array_Add] ← **NÃO executar aqui!**
  ↓
Set Actor Location (RemoteActorRef, OutLocation) ← **Atualiza posição**
  ↓
Set Actor Rotation (RemoteActorRef, OutRotation) ← **Atualiza rotação**
```

**Benefícios:**
- Actor existente é recuperado corretamente do array
- Actor não é adicionado novamente aos arrays (sem duplicatas)
- Actor é atualizado corretamente com nova posição/rotação

---

## ✅ **CHECKLIST DE CORREÇÃO:**

- [ ] Pin `then` do `K2Node_IfThenElse_6` está **desconectado** do `Set Variable: RemoteActorRef` atual
- [ ] `Get Array Item` foi adicionado no caminho `then`
- [ ] `Get Array Item` está configurado corretamente:
  - [ ] Array: `RemoteActors`
  - [ ] Index: `FoundIndex` (do `Array_Find`)
- [ ] Novo `Set Variable: RemoteActorRef` foi adicionado no caminho `then`
- [ ] Novo `Set Variable: RemoteActorRef` recebe o output do `Get Array Item`
- [ ] Caminho `then` **NÃO executa** `Array_Add`
- [ ] Caminho `then` conecta diretamente a `Set Actor Location` e `Set Actor Rotation`
- [ ] Caminho `else` mantém `Array_Add` após o spawn
- [ ] Ambos os caminhos convergem para `Set Actor Location` e `Set Actor Rotation`

---

## 🐛 **TROUBLESHOOTING:**

### **Problema: "Não consigo encontrar o `Set Actor Location` após o spawn"**
**Solução:**
- Procure por nós que atualizam a localização do ator após o `SpawnActorFromClass`
- Se não existirem, adicione-os após ambos os caminhos (`then` e `else`)

### **Problema: "Como fazer ambos os caminhos convergirem sem usar Array_Add?"**
**Solução:**
- Use um nó `Sequence` para organizar a execução
- Ou conecte ambos os caminhos diretamente aos mesmos nós `Set Actor Location` e `Set Actor Rotation`
- Os `Array_Add` devem estar **apenas** no caminho `else` (spawn)

### **Problema: "O ator ainda não está sendo atualizado após a correção"**
**Solução:**
- Verifique se `Set Actor Location` e `Set Actor Rotation` estão sendo executados
- Adicione logs após `Get Array Item` para verificar se o ator está sendo recuperado corretamente
- Verifique se `OutLocation` e `OutRotation` estão sendo passados corretamente

---

## 📝 **RESUMO:**

**O problema não é que o pin está desconectado, mas sim que está conectado à lógica incorreta:**

1. ❌ O pin `then` está conectado ao mesmo `Set Variable: RemoteActorRef` usado para spawn
2. ❌ Não há `Get Array Item` para recuperar o ator existente
3. ❌ `Array_Add` está sendo executado mesmo quando o ator existe (causando duplicatas)
4. ❌ O ator existente não está sendo atualizado, apenas sendo adicionado novamente

**A correção envolve:**

1. ✅ Adicionar `Get Array Item` para recuperar o ator existente
2. ✅ Criar um novo `Set Variable: RemoteActorRef` específico para o caminho `then`
3. ✅ Conectar diretamente a `Set Actor Location` e `Set Actor Rotation` (sem `Array_Add`)
4. ✅ Garantir que `Array_Add` seja executado apenas no caminho `else` (spawn)

---

**Fim do Documento**

