# 📋 INSTRUÇÕES EXATAS: Atualizar Blueprint para Nameplate com Actor

## 🎯 PROBLEMA ATUAL

O delegate `OnRemotePlayerNameplateUpdated` agora tem **4 parâmetros** (PlayerID, RemoteActor, CharacterName, CharacterTitle), mas o Blueprint ainda está usando a versão antiga com 3 parâmetros e tentando buscar o actor usando `GetRemoteActorByPlayerID`.

## ✅ SOLUÇÃO PASSO A PASSO

### **PASSO 1: Desconectar o Delegate Antigo**

1. Abra o Blueprint `BP_NetMovementClient2`
2. No `Event BeginPlay`, encontre o nó `Assign OnRemotePlayerNameplateUpdated` (ou `Bind Event`)
3. **DELETE completamente** esse nó e todas as conexões relacionadas
4. **DELETE** o Custom Event `UpdateNameplateFromDelegate` antigo (se existir)

---

### **PASSO 2: Criar o Novo Custom Event**

1. **Crie um NOVO Custom Event** chamado `UpdateNameplateFromDelegate`
2. **Adicione 4 inputs** (não 3!):
   - `PlayerID` (Integer)
   - `RemoteActor` (Actor) ⭐ **NOVO - ESSENCIAL**
   - `CharacterName` (String)
   - `CharacterTitle` (String)

**Como adicionar inputs no Custom Event:**
- Clique com botão direito no Custom Event
- Selecione "Add Input"
- Defina o tipo e nome de cada input

---

### **PASSO 3: Conectar o Delegate no BeginPlay**

No `Event BeginPlay`, adicione:

```
[Event BeginPlay]
    ↓
[Get Game Instance]
    World Context Object: self
    ↓
[Cast to Umbra Game Instance]
    Object: Return Value
    ↓
[Assign OnRemotePlayerNameplateUpdated] (ou Bind Event)
    Target: As Umbra Game Instance (do Cast)
    Delegate: (conecte ao Output Delegate do Custom Event UpdateNameplateFromDelegate)
```

**IMPORTANTE:** O delegate agora tem 4 parâmetros, então o Custom Event DEVE ter 4 inputs correspondentes!

---

### **PASSO 4: Implementar a Lógica do Custom Event**

**⚠️ CRÍTICO: NÃO USE `Get Remote Actor By Player ID` - O actor já vem no delegate!**

```
[Custom Event: UpdateNameplateFromDelegate]
    Inputs: PlayerID, RemoteActor, CharacterName, CharacterTitle
    ↓
[Print String] "UpdateNameplateFromDelegate: PlayerID=X, Actor=Y"
    InString: (formate com PlayerID e RemoteActor)
    ↓
[Is Valid?] RemoteActor
    Object: RemoteActor (do input)
    ↓
    ├──► TRUE: [Actor disponível]
    │       ↓
    │       [Get Component By Class]
    │           Actor: RemoteActor (do input)
    │           Component Class: Widget Component
    │           Return Value: WidgetComp
    │           ↓
    │           [Is Valid?] WidgetComp
    │               ↓
    │               then → [Get User Widget Object]
    │                       Target: WidgetComp
    │                       Return Value: NameplateWidget
    │                       ↓
    │                       [Is Valid?] NameplateWidget
    │                           ↓
    │                           then → [Cast to WBP Player Nameplate]
    │                                   Object: NameplateWidget
    │                                   ↓
    │                                   [Branch] bSuccess?
    │                                       ↓
    │                                       then → [Update Nameplate]
    │                                               Target: As WBP Player Nameplate
    │                                               CharacterName: CharacterName (do input)
    │                                               TitleName: CharacterTitle (do input)
    │
    └──► FALSE: [Actor não disponível]
            ↓
            [Print String] "Actor não disponível (nullptr), será atualizado quando spawnar"
```

---

## ⚠️ ERROS COMUNS

1. **Usar `Get Remote Actor By Player ID`** - ❌ ERRADO! O actor já vem no delegate
2. **Custom Event com apenas 3 inputs** - ❌ ERRADO! Deve ter 4 inputs
3. **Não desconectar o delegate antigo** - ❌ ERRADO! O delegate mudou, precisa reconectar

---

## ✅ VERIFICAÇÃO

Após atualizar, você deve ver nos logs:
- `UpdateNameplateFromDelegate chamado: PlayerID=X, Actor=Y` (com o nome do actor, não "nullptr")
- Se o actor for válido, o nameplate deve ser atualizado visualmente

---

**O actor agora vem diretamente no delegate - não precisa buscar!**
