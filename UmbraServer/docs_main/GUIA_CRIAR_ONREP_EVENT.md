# ✅ **GUIA: Como Criar e Conectar Event OnRep**

## 🎯 **O QUE É `OnRep_ReplicatedCharacterName`?**

O `OnRep_ReplicatedCharacterName` é um **Custom Event** que o Unreal Engine chama **automaticamente** quando a variável replicada `ReplicatedCharacterName` muda em outros clientes (não no servidor/dono).

---

## ✅ **PASSO 1: Criar o Custom Event**

### **No `BP_ThirdPersonCharacter`:**

1. **Abra o Event Graph**
2. **Clique com botão direito** no gráfico
3. **Digite:** `Add Custom Event` ou `Custom Event`
4. **Selecione:** `Add Custom Event`
5. **No Details Panel:**
   - **Event Name:** `OnRep_ReplicatedCharacterName` (EXATAMENTE este nome, com `OnRep_` no início)
   - **Call In Editor:** Desmarcado
   - **Replicated:** Desmarcado (não precisa ser replicado, ele é chamado localmente)

**⚠️ IMPORTANTE:** O nome DEVE ser `OnRep_` + nome da variável (`ReplicatedCharacterName`). O Unreal Engine reconhece esse padrão automaticamente.

---

## ✅ **PASSO 2: Conectar a Lógica**

**DENTRO do evento `OnRep_ReplicatedCharacterName`:**

```
[OnRep_ReplicatedCharacterName] (exec pin)
    ↓
[Get NameplateWidgetComponent]
    ↓
[Is Valid] (verificar se o componente existe)
    ↓
[Branch]
    True → [Get User Widget Object]
         → [Cast to WBP Player Nameplate]
         → [Branch] (bSuccess do Cast)
              True → [Update Nameplate]
                        CharacterName: ReplicatedCharacterName
                        TitleName: "" (ou TitleName se você também replicar)
```

---

## ✅ **PASSO 3: Verificação Completa**

### **Estrutura Final:**

**No Event Graph do `BP_ThirdPersonCharacter`:**

1. **`Event BeginPlay`** (atualiza o nameplate do próprio player):
   ```
   [Event BeginPlay]
       ↓
   [Get Current Character Info]
       ↓
   [Break Umbra Character Info]
       ↓
   [Set ReplicatedCharacterName] ← Isso dispara OnRep em outros clientes
       Value: Character Name
       ↓
   [Update Nameplate] ← Atualiza o próprio nameplate diretamente
       CharacterName: Character Name
       TitleName: Title Name
   ```

2. **`OnRep_ReplicatedCharacterName`** (atualiza nameplate de outros players):
   ```
   [OnRep_ReplicatedCharacterName] ← Chamado automaticamente em outros clientes
       ↓
   [Get NameplateWidgetComponent]
       ↓
   [Is Valid]
       ↓
   [Branch]
       True → [Get User Widget Object]
            → [Cast to WBP Player Nameplate]
            → [Update Nameplate]
                 CharacterName: ReplicatedCharacterName
                 TitleName: ""
   ```

---

## 🔍 **COMO FUNCIONA:**

1. **Player 1 (Owner):**
   - No `BeginPlay`, seta `ReplicatedCharacterName = "ElJeffo"`
   - Isso **NÃO** chama `OnRep` no Player 1 (porque ele é o owner)
   - O Player 1 atualiza seu próprio nameplate diretamente no `BeginPlay`

2. **Player 2 (Outro Cliente):**
   - Recebe a replicação de `ReplicatedCharacterName = "ElJeffo"` do Player 1
   - O Unreal Engine **chama automaticamente** `OnRep_ReplicatedCharacterName` no Player 2
   - O Player 2 atualiza o nameplate do Player 1 com "ElJeffo"

3. **Player 3 (Outro Cliente):**
   - Recebe a replicação de `ReplicatedCharacterName = "ElJeffo"` do Player 1
   - O Unreal Engine **chama automaticamente** `OnRep_ReplicatedCharacterName` no Player 3
   - O Player 3 atualiza o nameplate do Player 1 com "ElJeffo"

---

## ⚠️ **IMPORTANTE:**

1. **O nome do evento DEVE ser exato:** `OnRep_ReplicatedCharacterName`
   - `OnRep_` (prefixo obrigatório)
   - `ReplicatedCharacterName` (nome exato da variável replicada)

2. **O evento NÃO precisa ser conectado manualmente** - o Unreal Engine o chama automaticamente quando a variável replica

3. **O evento é chamado APENAS em outros clientes** (não no servidor/dono), a menos que você use `Replication Condition: None`

4. **Se você usar `Skip Owner`**, o `OnRep` NÃO é chamado no owner (que já atualiza no `BeginPlay`)

---

## 📋 **RESUMO:**

- **Criar:** Custom Event com nome `OnRep_ReplicatedCharacterName`
- **Conectar:** Não precisa conectar a nada - o Unreal Engine chama automaticamente
- **Lógica:** Dentro do evento, atualize o nameplate usando `ReplicatedCharacterName`

---

**FIM DO GUIA**

