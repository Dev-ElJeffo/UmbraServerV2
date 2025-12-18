# ✅ **GUIA: Configurar Replicação pela Primeira Vez**

## 🎯 **SITUAÇÃO**

Nada no projeto usa replicação ainda. Precisamos configurar o `BP_ThirdPersonCharacter` para suportar replicação.

---

## ✅ **PASSO 1: Verificar Classe Pai do BP_ThirdPersonCharacter**

### **No `BP_ThirdPersonCharacter`:**

1. **Abra o Blueprint `BP_ThirdPersonCharacter`**
2. **No topo do Blueprint, verifique a classe pai:**
   - Deve herdar de `Character` ou `Pawn`
   - **SE NÃO HERDAR:** A replicação pode não funcionar corretamente

**✅ Se herdar de `Character` ou `Pawn`:** Continue para o Passo 2.

**❌ Se NÃO herdar:** Considere mudar a classe pai para `Character`.

---

## ✅ **PASSO 2: Marcar BP_ThirdPersonCharacter como Replicado**

### **No `BP_ThirdPersonCharacter`:**

1. **Abra o Blueprint `BP_ThirdPersonCharacter`**
2. **No Viewport ou na hierarquia, selecione o próprio Blueprint (não um componente)**
3. **No Details Panel, procure por "Replication":**
   - **Replicates:** **MARQUE ESTA OPÇÃO** ✅
   - **Replicate Movement:** Pode deixar desmarcado (não afeta variáveis)
   - **Net Update Frequency:** Deixe padrão (100.0)

**⚠️ IMPORTANTE:** Se você não encontrar a opção "Replicates":
- Certifique-se de que está selecionando o **Blueprint em si**, não um componente
- A opção pode estar em "Advanced" → "Replication"

---

## ✅ **PASSO 3: Criar Variáveis Replicadas**

### **No `BP_ThirdPersonCharacter`:**

1. **Abra a aba "Variables" ou "My Blueprint"**
2. **Crie as variáveis:**

   **Variável 1:**
   - **Nome:** `ReplicatedCharacterName`
   - **Tipo:** `String`
   - **Replication:** `Replicated`
   - **Replication Condition:** `Skip Owner`

   **Variável 2:**
   - **Nome:** `ReplicatedCharacterTitle`
   - **Tipo:** `String`
   - **Replication:** `Replicated`
   - **Replication Condition:** `Skip Owner`

---

## ✅ **PASSO 4: Criar Event OnRep**

### **No `BP_ThirdPersonCharacter`:**

1. **Abra o Event Graph**
2. **Clique com botão direito** → `Add Custom Event`
3. **No Details Panel:**
   - **Event Name:** `OnRep_ReplicatedCharacterName` (EXATAMENTE este nome)
4. **Dentro do evento, adicione:**

```
[OnRep_ReplicatedCharacterName]
    ↓
[Print String]
    InString: "🔥 [OnRep] ReplicatedCharacterName = {ReplicatedCharacterName}"
    ↓
[Get NameplateWidgetComponent]
    ↓
[Is Valid]
    ↓
[Branch]
    True → [Get User Widget Object]
         → [Cast to WBP Player Nameplate]
         → [Branch] (bSuccess)
              True → [Update Nameplate]
                        CharacterName: ReplicatedCharacterName
                        TitleName: ReplicatedCharacterTitle
```

**Repita para `ReplicatedCharacterTitle` (se necessário):**

1. **Crie outro Custom Event:**
   - **Event Name:** `OnRep_ReplicatedCharacterTitle`
2. **Lógica similar ao acima**

---

## ✅ **PASSO 5: Atualizar BeginPlay para Setar Variáveis Replicadas**

### **No `Event BeginPlay` do `BP_ThirdPersonCharacter`:**

**Adicione APÓS obter `CurrentCharacterInfo`:**

```
[Event BeginPlay]
    ↓
... (código existente para obter CharacterInfo)
    ↓
[Break Umbra Character Info]
    ↓
[Print String]
    InString: "🔵 [BeginPlay] Setando ReplicatedCharacterName = {Character Name}"
    ↓
[Set ReplicatedCharacterName]
    Value: Character Name
    ↓
[Set ReplicatedCharacterTitle]
    Value: Title Name
    ↓
[Print String]
    InString: "✅ [BeginPlay] Variáveis replicadas setadas!"
    ↓
... (código existente para atualizar nameplate do próprio player)
```

---

## ✅ **PASSO 6: Compilar e Testar**

### **Compilar:**

1. **Compile o Blueprint** (`Compile` button)
2. **Salve o Blueprint** (`Ctrl+S`)

### **Testar com 2 Clientes:**

1. **No Editor, abra o nível**
2. **Play Settings:**
   - `Play` → `Number of Players: 2`
   - Ou `Window` → `Play Settings` → `Number of Players: 2`
3. **Execute o PIE (Play In Editor)**
4. **Verifique os logs:**
   - **Cliente 1:** Deve ver logs do `BeginPlay` setando variáveis
   - **Cliente 2:** Deve ver logs do `OnRep_ReplicatedCharacterName` sendo chamado

---

## 🔍 **VERIFICAÇÕES FINAIS**

### **Checklist:**

- [ ] `BP_ThirdPersonCharacter` herda de `Character` ou `Pawn`
- [ ] `Replicates` está **MARCADO** no `BP_ThirdPersonCharacter`
- [ ] Variáveis `ReplicatedCharacterName` e `ReplicatedCharacterTitle` criadas
- [ ] Variáveis têm `Replication: Replicated`
- [ ] Variáveis têm `Replication Condition: Skip Owner`
- [ ] Event `OnRep_ReplicatedCharacterName` criado (nome exato)
- [ ] `BeginPlay` está setando `ReplicatedCharacterName` e `ReplicatedCharacterTitle`
- [ ] Logs foram adicionados para diagnóstico
- [ ] Blueprint foi compilado
- [ ] Teste com 2 clientes foi realizado

---

## ⚠️ **PROBLEMAS COMUNS**

### **1. "Replicates" não aparece no Details Panel:**

**SOLUÇÃO:**
- Certifique-se de selecionar o **Blueprint em si** (não um componente)
- Verifique se a classe pai é `Character` ou `Pawn`
- A opção pode estar em "Advanced" → "Replication"

### **2. OnRep não é chamado:**

**VERIFICAÇÕES:**
- Nome do evento está exato? (`OnRep_ReplicatedCharacterName`)
- Variável está marcada como `Replicated`?
- `Replicates` está marcado no Blueprint?
- Teste com 2 clientes (OnRep só é chamado em outros clientes, não no owner)

### **3. Variável não replica:**

**VERIFICAÇÕES:**
- `Replicates` está marcado no Blueprint?
- Variável tem `Replication: Replicated`?
- Variável está sendo setada no `BeginPlay`?
- Blueprint foi compilado após mudanças?

---

## 📋 **ORDEM DE IMPLEMENTAÇÃO:**

1. **Passo 1:** Verificar classe pai
2. **Passo 2:** Marcar `Replicates` ← **MAIS IMPORTANTE**
3. **Passo 3:** Criar variáveis replicadas
4. **Passo 4:** Criar eventos OnRep
5. **Passo 5:** Atualizar BeginPlay
6. **Passo 6:** Compilar e testar

---

**FIM DO GUIA**

