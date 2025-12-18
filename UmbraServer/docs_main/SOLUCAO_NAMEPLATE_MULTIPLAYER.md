# ✅ **SOLUÇÃO: Nameplate Multiplayer e Rotação Fixa**

## 🎯 **PROBLEMAS**

1. Cada cliente só vê o próprio nome (não vê nomes dos outros jogadores)
2. O widget rotaciona junto com o player (deveria ficar sempre virado para a câmera)

---

## ✅ **SOLUÇÃO 1: Widget Sempre Virado para a Câmera**

### **No `BP_ThirdPersonCharacter`:**

**No `Event Tick` (ou em um Timer):**

```
[Event Tick]
    ↓
[Get Player Controller]
    ↓
[Get Control Rotation]
    ↓
[Get NameplateWidgetComponent]
    ↓
[Set World Rotation]
    Rotation: Control Rotation (do Player Controller)
```

**OU configure o Widget Component:**

1. **Selecione o `NameplateWidgetComponent` no `BP_ThirdPersonCharacter`**
2. **No Details Panel:**
   - **Space:** `World`
   - **Widget Space:** `Screen` (isso faz o widget sempre olhar para a câmera automaticamente)
   - **OR** use **`Set Widget Space`** no Blueprint para `Screen`

**Melhor solução: usar `Set Widget Space`:**

```
[Event BeginPlay]
    ↓
[Get NameplateWidgetComponent]
    ↓
[Set Widget Space]
    Space: Screen
```

---

## ✅ **SOLUÇÃO 2: Sincronizar Nameplates entre Clientes**

### **Opção A: Usar Replication (Recomendado)**

**No `BP_ThirdPersonCharacter`:**

1. **Crie uma variável replicada:**
   - Nome: `ReplicatedCharacterName`
   - Tipo: `String`
   - **Replication:** `Replicated`
   - **Replication Condition:** `Always`

2. **No `Event BeginPlay`:**
   ```
   [Event BeginPlay]
       ↓
   [Get Game Instance]
       ↓
   [Cast to Umbra Game Instance]
       ↓
   [Get Current Character Info]
       ↓
   [Break Umbra Character Info]
       ↓
   [Set ReplicatedCharacterName]
           Value: Character Name
   ```

3. **Crie um Event `OnRep_ReplicatedCharacterName`:**
   ```
   [OnRep_ReplicatedCharacterName]
       ↓
   [UpdateNameplateFromReplicatedName]
       CharacterName: ReplicatedCharacterName
   ```

4. **Crie uma função `UpdateNameplateFromReplicatedName`:**
   ```
   [UpdateNameplateFromReplicatedName]
       Input: CharacterName (String)
       ↓
   [Get NameplateWidgetComponent]
       ↓
   [Get User Widget Object]
       ↓
   [Cast to WBP Player Nameplate]
       ↓
   [Update Nameplate]
           CharacterName: CharacterName (do input)
           TitleName: "" (vazio, ou também replicar se necessário)
   ```

### **Opção B: Usar Game State (Alternativa)**

Se você já tem um sistema de sincronização de jogadores, atualize os nameplates quando outros jogadores entrarem:

```
[Quando outro jogador entra/spawna]
    ↓
[Get All Actors of Class] (BP_ThirdPersonCharacter)
    ↓
[For Each Loop]
    ↓
[Get ReplicatedCharacterName] (do outro player)
    ↓
[UpdateNameplateFromReplicatedName] (no outro player)
```

---

## 📋 **IMPLEMENTAÇÃO COMPLETA**

### **1. No `BP_ThirdPersonCharacter`:**

**Variáveis:**
- `ReplicatedCharacterName` (String, Replicated)

**Event BeginPlay:**
```
[Event BeginPlay]
    ↓
[Get NameplateWidgetComponent]
    ↓
[Set Widget Space]
    Space: Screen
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info]
    ↓
[Break Umbra Character Info]
    ↓
[Set ReplicatedCharacterName]
    Value: Character Name
```

**Event OnRep_ReplicatedCharacterName:**
```
[OnRep_ReplicatedCharacterName]
    ↓
[UpdateNameplateFromReplicatedName]
    CharacterName: ReplicatedCharacterName
```

**Função UpdateNameplateFromReplicatedName:**
```
[UpdateNameplateFromReplicatedName]
    Input: CharacterName (String)
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
                        CharacterName: CharacterName
                        TitleName: ""
```

### **2. Para atualizar nameplates de outros jogadores:**

**Crie uma função pública `UpdateOtherPlayerNameplate`:**

```
[UpdateOtherPlayerNameplate]
    Input: CharacterName (String)
    ↓
[Set ReplicatedCharacterName]
    Value: CharacterName
```

**Chame esta função quando o `CurrentCharacterInfo` for atualizado:**

```
[OnCharacterInfoLoaded] (delegate)
    ↓
[Break Umbra Character Info]
    ↓
[UpdateOtherPlayerNameplate]
    CharacterName: Character Name
```

---

## ⚠️ **IMPORTANTE**

1. **`Set Widget Space` para `Screen`** faz o widget sempre olhar para a câmera automaticamente
2. **Replication** só funciona se o `BP_ThirdPersonCharacter` for um `Actor` replicado
3. **Para ver nameplates de outros jogadores**, você precisa acessar o `ReplicatedCharacterName` deles e atualizar o widget deles

---

**FIM DA SOLUÇÃO**

