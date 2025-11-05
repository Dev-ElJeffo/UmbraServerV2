# 🔧 **GUIA PRÁTICO: Conectar Pin `then` de `K2Node_IfThenElse_6`**

## 📋 **PROBLEMA CRÍTICO:**
O pin `then` (True) do `K2Node_IfThenElse_6` está desconectado, impedindo atualização de actors existentes.

---

## 🎯 **SOLUÇÃO PASSO A PASSO:**

### **PASSO 1: Localizar o Nó `K2Node_IfThenElse_6`**

1. **Abra o Blueprint `BP_NetMovementClient` no Unreal Editor.**
2. **Abra a função `ProcessNextFrame`.**
3. **Procure pelo nó `Branch` que verifica `FoundIndex >= 0`:**
   - Este é o `K2Node_IfThenElse_6`.
   - Sua condição está conectada a um `Greater or Equal` que compara `FoundIndex` com `0`.

**Como identificar:**
- O nó tem dois pins de saída: `then` (True) e `else` (False).
- O pin `else` está conectado ao `SpawnActorFromClass` (via `K2Node_IfThenElse_3`).
- O pin `then` está **desconectado** (sem linha saindo dele).

---

### **PASSO 2: Adicionar `Get Array Item`**

1. **Clique com botão direito** na área vazia após o pin `then` do `K2Node_IfThenElse_6`.
2. **Procure por:** `Get Array Item` (ou digite `Get Array Item`).
3. **Selecione:** `Get Array Item` (função genérica de array).

**Configuração do nó:**
- **Array:** 
  - Conecte ao pin de saída de `Get Variable (RemoteActors)`.
  - Se não existir `Get Variable (RemoteActors)`, adicione:
    - Botão direito → `Get Variable` → Selecione `RemoteActors`.
- **Index:**
  - Conecte ao pin de saída `FoundIndex` do `Array_Find`.
  - Procure pelo nó `Array_Find` (`K2Node_CallArrayFunction_4`).
  - Conecte `ReturnValue` (FoundIndex) ao pin `Index` do `Get Array Item`.

**Resultado esperado:**
- O `Get Array Item` retorna o actor do tipo `BP_RemotePlayer` que já existe no array `RemoteActors` na posição `FoundIndex`.

---

### **PASSO 3: Adicionar `Set Variable: RemoteActorRef`**

1. **Clique com botão direito** após o `Get Array Item`.
2. **Procure por:** `Set Variable` (ou `Set`).
3. **Selecione:** `Set Variable` → Selecione `RemoteActorRef` (variável do Blueprint).

**Configuração do nó:**
- **Variable:** `RemoteActorRef` (deve ser uma variável do tipo `Actor Reference` ou `BP_RemotePlayer`).
- **Value:**
  - Conecte ao pin de saída do `Get Array Item` (o actor existente).

**Resultado esperado:**
- A variável `RemoteActorRef` agora contém o actor existente, pronto para ser atualizado.

---

### **PASSO 4: Conectar à Lógica de Atualização Existente**

**Verificar se já existe lógica de atualização:**
1. Procure por nós `Set Actor Location` e `Set Actor Rotation` após o `SpawnActorFromClass`.
2. Estes nós devem estar no caminho do pin `else` do `K2Node_IfThenElse_6`.

**Conectar o caminho do pin `then`:**
1. **Conecte o pin de saída do `Set Variable: RemoteActorRef`** (do caminho `then`) ao mesmo `Set Actor Location` e `Set Actor Rotation` que são usados no caminho `else`.

**Opções de conexão:**

**Opção A: Se já existe um `Set Actor Location` após `SpawnActorFromClass`:**
- Conecte o `Set Variable: RemoteActorRef` (do caminho `then`) ao mesmo `Set Actor Location`.
- Use um nó `Sequence` ou simplesmente conecte ambos os caminhos ao mesmo nó.

**Opção B: Se não existe lógica de atualização após `SpawnActorFromClass`:**
- Adicione `Set Actor Location` após `Set Variable: RemoteActorRef` (caminho `then`).
- Adicione `Set Actor Rotation` após `Set Actor Location`.
- Configure:
  - **Target:** `RemoteActorRef` (variável do Blueprint).
  - **NewLocation:** `OutLocation` (do `ParseStateUpdateFrame`).
  - **NewRotation:** `OutRotation` (do `ParseStateUpdateFrame`).

---

### **PASSO 5: Verificar Convergência dos Caminhos**

**Fluxo esperado após correção:**

```
K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)
  │
  ├─ then (True - Actor existe):
  │   │
  │   ├─ Get Array Item (RemoteActors, FoundIndex)
  │   │
  │   ├─ Set Variable: RemoteActorRef = [Output do Get Array Item]
  │   │
  │   └─ [Conecta aqui] ↓
  │
  └─ else (False - Actor não existe):
      │
      ├─ K2Node_IfThenElse_3 (Branch: OutLocation != (0,0,0)?)
      │   │
      │   └─ then (True - Location válida):
      │       │
      │       ├─ SpawnActorFromClass
      │       │
      │       ├─ Set Variable: RemoteActorRef = [Output do SpawnActorFromClass]
      │       │
      │       ├─ Array_Add (RemoteActorIds, OutPlayerId)
      │       │
      │       ├─ Array_Add (RemoteActors, RemoteActorRef)
      │       │
      │       └─ [Conecta aqui] ↓
      │
      └─ [Ponto de convergência comum]
          │
          ├─ Set Actor Location (RemoteActorRef, OutLocation)
          │
          └─ Set Actor Rotation (RemoteActorRef, OutRotation)
```

**Verificações:**
- [ ] Ambos os caminhos (`then` e `else`) convergem para os mesmos nós `Set Actor Location` e `Set Actor Rotation`.
- [ ] `Array_Add` está presente apenas no caminho `else` (não no caminho `then`).
- [ ] `RemoteActorRef` está sendo usado corretamente em ambos os caminhos.

---

### **PASSO 6: Adicionar Validação (Opcional mas Recomendado)**

**Adicionar validação `Is Valid` antes de atualizar:**
1. Após `Get Array Item`, adicione um nó `Is Valid`.
2. Conecte o output do `Get Array Item` ao `Is Valid`.
3. Adicione um `Branch` após `Is Valid`.
4. Conecte o pin `then` (True) do `Branch` ao `Set Variable: RemoteActorRef`.
5. Conecte o pin `else` (False) do `Branch` a um `Print String` com mensagem de erro ou simplesmente termine a execução.

**Por que isso é importante:**
- Se o actor foi destruído entre frames, `Get Array Item` pode retornar um actor inválido.
- A validação evita crashes ao tentar atualizar um actor inválido.

---

## 🎨 **VISUALIZAÇÃO DA ESTRUTURA CORRETA:**

```
┌─────────────────────────────────────────────────────────────┐
│ ParseStateUpdateFrame → OutPlayerId, OutLocation, OutRotation│
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ Array_Find (RemoteActorIds, OutPlayerId) → FoundIndex        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ Greater or Equal (FoundIndex >= 0)                          │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ K2Node_IfThenElse_6 (Branch: FoundIndex >= 0?)              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─ then (True - Actor existe):                             │
│  │                                                           │
│  │  ┌──────────────────────────────────────────────┐         │
│  │  │ Get Array Item (RemoteActors, FoundIndex)   │         │
│  │  └──────────────────────────────────────────────┘         │
│  │                     ↓                                      │
│  │  ┌──────────────────────────────────────────────┐         │
│  │  │ Set Variable: RemoteActorRef = [Actor]       │         │
│  │  └──────────────────────────────────────────────┘         │
│  │                     ↓                                      │
│  │                     └──────────────────┐                  │
│  │                                        │                  │
│  └─ else (False - Actor não existe):      │                  │
│     │                                      │                  │
│     │  ┌──────────────────────────────────┐                  │
│     │  │ Branch: OutLocation != (0,0,0)? │                  │
│     │  └──────────────────────────────────┘                  │
│     │         ↓ (then)                                       │
│     │  ┌──────────────────────────────────┐                  │
│     │  │ SpawnActorFromClass               │                  │
│     │  └──────────────────────────────────┘                  │
│     │         ↓                                              │
│     │  ┌──────────────────────────────────┐                  │
│     │  │ Set Variable: RemoteActorRef     │                  │
│     │  └──────────────────────────────────┘                  │
│     │         ↓                                              │
│     │  ┌──────────────────────────────────┐                  │
│     │  │ Array_Add (RemoteActorIds)       │                  │
│     │  └──────────────────────────────────┘                  │
│     │         ↓                                              │
│     │  ┌──────────────────────────────────┐                  │
│     │  │ Array_Add (RemoteActors)         │                  │
│     │  └──────────────────────────────────┘                  │
│     │         ↓                                              │
│     └─────────┘                                              │
│              ↓                                                │
│  ┌──────────────────────────────────────────────┐           │
│  │ Set Actor Location (RemoteActorRef, OutLocation)│         │
│  └──────────────────────────────────────────────┘           │
│              ↓                                                │
│  ┌──────────────────────────────────────────────┐           │
│  │ Set Actor Rotation (RemoteActorRef, OutRotation)│         │
│  └──────────────────────────────────────────────┘           │
└─────────────────────────────────────────────────────────────┘
```

---

## ✅ **CHECKLIST DE VERIFICAÇÃO:**

- [ ] Pin `then` do `K2Node_IfThenElse_6` está conectado.
- [ ] `Get Array Item` está presente após o pin `then`.
- [ ] `Get Array Item` está configurado corretamente:
  - [ ] Array: `RemoteActors`.
  - [ ] Index: `FoundIndex` (do `Array_Find`).
- [ ] `Set Variable: RemoteActorRef` está presente após `Get Array Item`.
- [ ] `Set Variable: RemoteActorRef` recebe o output do `Get Array Item`.
- [ ] Ambos os caminhos (`then` e `else`) convergem para `Set Actor Location` e `Set Actor Rotation`.
- [ ] `Array_Add` está presente apenas no caminho `else` (não no caminho `then`).
- [ ] Compilação do Blueprint está sem erros.
- [ ] Teste em runtime: actors existentes são atualizados corretamente.

---

## 🐛 **TROUBLESHOOTING:**

### **Problema: "Variable 'RemoteActorRef' is not compatible with Array Item output"**
**Solução:**
- Verifique o tipo da variável `RemoteActorRef`.
- Deve ser do tipo `Actor Reference` ou `BP_RemotePlayer` (ou classe base).
- Se necessário, altere o tipo da variável no Blueprint.

### **Problema: "FoundIndex is not connected"**
**Solução:**
- Certifique-se de que `Array_Find` está presente e configurado corretamente.
- Verifique se `ReturnValue` do `Array_Find` está conectado ao `Index` do `Get Array Item`.

### **Problema: "Both paths (then and else) are executing Set Actor Location twice"**
**Solução:**
- Verifique se ambos os caminhos estão convergindo corretamente.
- Use um nó `Sequence` ou garanta que apenas um caminho executa por vez.

### **Problema: "Actor is not updating even after connecting the pin"**
**Solução:**
- Verifique se `OutLocation` e `OutRotation` estão sendo passados corretamente.
- Adicione logs após `Set Actor Location` para verificar se está sendo executado.
- Verifique se o actor não está sendo destruído entre frames.

---

## 📝 **NOTAS FINAIS:**

- Esta correção é **crítica** para o funcionamento correto do sistema de rede.
- Sem ela, actors existentes nunca são atualizados, causando múltiplos spawns e problemas de sincronização.
- Após a correção, teste com múltiplos clientes para verificar que actors são atualizados corretamente.

---

**Fim do Documento**

