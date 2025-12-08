# 🔧 CORREÇÃO: Ordem de SelectCharacter e LoadCharacterInfo

## ❌ PROBLEMA

O `LoadCharacterInfo()` está sendo chamado ANTES de `SelectCharacter()` terminar, então `ActivePlayerID` ainda não está setado e os dados completos não são carregados.

## ✅ SOLUÇÃO

Aguardar o delegate `OnCharacterSelected` ANTES de chamar `LoadCharacterInfo()`.

---

## 📋 O QUE FAZER NO BLUEPRINT

### **No `BP_CharacterSelectionManager` → Função `SelectCharacterPreview`:**

**ESTRUTURA ATUAL (ERRADA):**
```
SelectCharacterPreview
  ↓
SelectCharacterAndMoveCamera
  ↓
LoadCharacterInfo  ← ERRADO! Chamado muito cedo
  ↓
Delay 0.5s
  ↓
AddCharacterItemToWidget
```

**ESTRUTURA CORRETA:**
```
SelectCharacterPreview
  ↓
SelectCharacterAndMoveCamera
  ↓
[AGUARDAR OnCharacterSelected] ← ADICIONAR ISSO!
  ↓
LoadCharacterInfo
  ↓
[AGUARDAR OnCharacterInfoLoaded] ← ADICIONAR ISSO!
  ↓
AddCharacterItemToWidget
```

---

## 🔧 PASSO A PASSO

### **PASSO 1: Conectar ao Delegate OnCharacterSelected**

**No `BP_CharacterSelectionManager` → Event Graph:**

1. **Após `SelectCharacterAndMoveCamera` (se TRUE):**
   - Arraste do pino azul → Digite "Get Game Instance"
   - Conecte ao "Get Game Instance"
   - Arraste do "Return Value" → Digite "Cast to Umbra Game Instance"
   - Conecte ao "Cast to Umbra Game Instance"
   - Arraste do "As Umbra Game Instance" → Digite "Assign On Character Selected"
   - Isso cria automaticamente um Custom Event `On Character Selected`

### **PASSO 2: Criar o Event OnCharacterSelected**

1. **Procure no Event Graph por `On Character Selected`** (foi criado automaticamente)
2. **Se não existir, crie manualmente:**
   - Botão direito → "Custom Event"
   - Nome: `OnCharacterSelected`
   - Input: `SelectedPlayer` (tipo: `Umbra Player Data` - Struct)

### **PASSO 3: Mover LoadCharacterInfo para OnCharacterSelected**

**No evento `OnCharacterSelected`:**

```
OnCharacterSelected
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
      [Print String] "LoadCharacterInfo chamado após SelectCharacter"
```

### **PASSO 4: Conectar ao Delegate OnCharacterInfoLoaded**

**No `BP_CharacterSelectionManager` → Event Graph (Event BeginPlay ou onde você já conecta delegates):**

1. **Arraste do "As Umbra Game Instance" → Digite "Assign On Character Info Loaded"**
2. Isso cria automaticamente um Custom Event `On Character Info Loaded`

### **PASSO 5: Mover AddCharacterItemToWidget para OnCharacterInfoLoaded**

**No evento `OnCharacterInfoLoaded`:**

```
OnCharacterInfoLoaded
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
[Print String] "AddCharacterItemToWidget chamado após LoadCharacterInfo"
```

---

## 📊 ESTRUTURA COMPLETA CORRETA

### **SelectCharacterPreview:**
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
  ├─→ [FALSE] → [Print String] "Erro ao selecionar personagem"
  └─→ [TRUE] →
      ↓
      [Print String] "Aguardando OnCharacterSelected..."
      (NÃO chama LoadCharacterInfo aqui!)
```

### **OnCharacterSelected (criado automaticamente pelo Assign):**
```
OnCharacterSelected
  • SelectedPlayer (input)
  ↓
[Get] MyGameInstance
  ↓
[Is Valid?] MyGameInstance
  ├─→ [FALSE] → STOP
  └─→ [TRUE] →
      ↓
      [Load Character Info]
      • Target: MyGameInstance
      ↓
      [Print String] "LoadCharacterInfo chamado após SelectCharacter"
```

### **OnCharacterInfoLoaded (criado automaticamente pelo Assign):**
```
OnCharacterInfoLoaded
  • CharacterInfo (input)
  ↓
[Get] MyGameInstance
  ↓
[Get] ActivePlayerID (do MyGameInstance)
  ↓
[Call Function] Add Character Item To Widget
  • Target: Self
  • PlayerID: ActivePlayerID
  • Character Item Widget Class: WBP_CharacterItem
  ↓
[Print String] "AddCharacterItemToWidget chamado após LoadCharacterInfo"
```

---

## ✅ CHECKLIST

- [ ] `SelectCharacterPreview` NÃO chama `LoadCharacterInfo` diretamente
- [ ] `SelectCharacterPreview` apenas chama `SelectCharacterAndMoveCamera` e aguarda
- [ ] `OnCharacterSelected` está conectado ao delegate do GameInstance (via "Assign On Character Selected")
- [ ] `OnCharacterSelected` chama `LoadCharacterInfo`
- [ ] `OnCharacterInfoLoaded` está conectado ao delegate do GameInstance (via "Assign On Character Info Loaded")
- [ ] `OnCharacterInfoLoaded` chama `AddCharacterItemToWidget` com `ActivePlayerID`

---

## 🐛 SE NÃO FUNCIONAR

1. **Verifique se os delegates estão conectados:**
   - No Event BeginPlay do `BP_CharacterSelectionManager`, você deve ter:
     - `Assign On Character Selected` → `OnCharacterSelected`
     - `Assign On Character Info Loaded` → `OnCharacterInfoLoaded`

2. **Verifique a ordem:**
   - `SelectCharacterAndMoveCamera` → aguarda → `OnCharacterSelected` → `LoadCharacterInfo` → aguarda → `OnCharacterInfoLoaded` → `AddCharacterItemToWidget`

3. **Compile o Blueprint** após fazer as alterações

