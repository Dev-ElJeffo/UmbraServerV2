# 🔧 GUIA SIMPLES: Ordem Correta no Blueprint

## ❌ PROBLEMA

O `LoadCharacterInfo()` está sendo chamado ANTES de `SelectCharacter()` terminar, então `ActivePlayerID` ainda não está setado.

## ✅ SOLUÇÃO SIMPLES

**No Blueprint `BP_CharacterSelectionManager`:**

### **PASSO 1: Remover LoadCharacterInfo de SelectCharacterPreview**

**Na função `SelectCharacterPreview`:**
- **REMOVA** a chamada `LoadCharacterInfo`
- **REMOVA** o `Delay`
- **REMOVA** a chamada `AddCharacterItemToWidget`
- Deixe APENAS `SelectCharacterAndMoveCamera`

**Fica assim:**
```
SelectCharacterPreview
  • PlayerID (input)
  ↓
[Call Function] Select Character And Move Camera
  • Target: Self
  • PlayerID: PlayerID
  • Return Value: Success
  ↓
[Branch] Success
  ├─→ [FALSE] → [Print String] "Erro"
  └─→ [TRUE] → [Print String] "Aguardando SelectCharacter terminar..."
```

---

### **PASSO 2: Conectar OnCharacterSelected no Event BeginPlay**

**No Event Graph do `BP_CharacterSelectionManager`:**

1. **Procure o `Event BeginPlay`** (ou crie se não existir)

2. **Adicione:**
   - `Get Game Instance`
   - `Cast to Umbra Game Instance`
   - `Assign On Character Selected` (arraste do pino azul do Cast)

3. **O `Assign On Character Selected` cria automaticamente um evento `On Character Selected`**

**Fica assim:**
```
Event BeginPlay
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ↓
[Assign On Character Selected] → Cria evento automaticamente
  • Target: As Umbra Game Instance
  • Event: On Character Selected (criado automaticamente)
```

---

### **PASSO 3: No evento OnCharacterSelected (criado automaticamente)**

**Este evento será chamado AUTOMATICAMENTE quando `SelectCharacter()` terminar.**

**No evento `OnCharacterSelected`:**

```
On Character Selected (criado automaticamente)
  • SelectedPlayer (input)
  ↓
[Get] MyGameInstance (variável do BP)
  ↓
[Is Valid?] MyGameInstance
  ├─→ [FALSE] → STOP
  └─→ [TRUE] →
      ↓
      [Load Character Info]
      • Target: MyGameInstance
      ↓
      [Print String] "LoadCharacterInfo chamado"
```

---

### **PASSO 4: Conectar OnCharacterInfoLoaded no Event BeginPlay**

**No mesmo `Event BeginPlay` do PASSO 2:**

**Adicione:**
- `Assign On Character Info Loaded` (arraste do pino azul do Cast)

**Fica assim:**
```
Event BeginPlay
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ↓
[Assign On Character Selected]
  ↓
[Assign On Character Info Loaded] → Cria evento automaticamente
  • Target: As Umbra Game Instance
  • Event: On Character Info Loaded (criado automaticamente)
```

---

### **PASSO 5: No evento OnCharacterInfoLoaded (criado automaticamente)**

**Este evento será chamado AUTOMATICAMENTE quando `LoadCharacterInfo()` terminar.**

**No evento `OnCharacterInfoLoaded`:**

```
On Character Info Loaded (criado automaticamente)
  • CharacterInfo (input)
  ↓
[Get] MyGameInstance
  ↓
[Get] ActivePlayerID (do MyGameInstance)
  ↓
[Call Function] Add Character Item To Widget
  • Target: Self (BP_CharacterSelectionManager)
  • PlayerID: ActivePlayerID
  • Character Item Widget Class: WBP_CharacterItem
  ↓
[Print String] "AddCharacterItemToWidget chamado"
```

---

## 📊 RESUMO DA ORDEM

1. **SelectCharacterPreview** → Chama `SelectCharacterAndMoveCamera` e PARA
2. **OnCharacterSelected** (automático) → Chama `LoadCharacterInfo` e PARA
3. **OnCharacterInfoLoaded** (automático) → Chama `AddCharacterItemToWidget`

**NÃO precisa de Delay, NÃO precisa chamar nada manualmente. Os eventos são chamados AUTOMATICAMENTE quando as coisas terminam.**

---

## ✅ CHECKLIST FINAL

- [ ] `SelectCharacterPreview` NÃO chama `LoadCharacterInfo`
- [ ] `Event BeginPlay` tem `Assign On Character Selected`
- [ ] `Event BeginPlay` tem `Assign On Character Info Loaded`
- [ ] Existe evento `On Character Selected` (criado automaticamente)
- [ ] Existe evento `On Character Info Loaded` (criado automaticamente)
- [ ] `On Character Selected` chama `LoadCharacterInfo`
- [ ] `On Character Info Loaded` chama `AddCharacterItemToWidget`

