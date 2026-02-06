# ✅ GUIA COMPLETO: Ativar Nameplate e Atualizar Widget ao Equipar/Desequipar

## 🎯 OBJETIVOS

1. **Ativar sistema de nameplate** - Transmitir nome e título para outros clients via WebSocket
2. **Atualizar widget ao equipar/desequipar** - `WBP_SelectedPlayerInfo` deve atualizar HP/MP quando itens são equipados/desequipados

---

## ✅ PARTE 1: Sistema de Nameplate (JÁ IMPLEMENTADO NO C++)

### **1.1. Função `SendPlayerInfoUpdate` Implementada**

A função `SendPlayerInfoUpdate` já está implementada no C++ e é chamada automaticamente quando:
- O `CharacterInfo` é carregado (após login ou quando itens são equipados/desequipados)

**O que ela faz:**
- Busca automaticamente o `WebSocketClient` do `NetMovementClient` se não for fornecido
- Codifica a mensagem usando `WSBinaryBPFL::EncodePlayerInfoUpdate`
- Envia via WebSocket para sincronizar nome e título com outros clients

### **1.2. Função `UpdateRemotePlayerNameplate` Implementada**

A função `UpdateRemotePlayerNameplate` foi criada para processar `PlayerInfoUpdate` recebido:
- Atualiza o cache do `PlayerSelectionComponent`
- Atualiza o widget `WBP_SelectedPlayerInfo` se o player estiver selecionado
- Dispara o delegate `OnRemotePlayerNameplateUpdated` para Blueprint atualizar o nameplate visual

---

## ✅ PARTE 2: Processar PlayerInfoUpdate no Blueprint

### **2.1. No Blueprint `BP_NetMovementClient` → `OnWSBinaryMessage`**

**Adicione a verificação para PlayerInfoUpdate (msgType = 4) NO INÍCIO do evento, ANTES de processar StateUpdate:**

```
[OnWSBinaryMessage]
    Data: (TArray<uint8>)
    ↓
[WSBinaryBPFL::ParsePlayerInfoUpdate]
    Data: Data
    OutPlayerID: (variável local)
    OutCharacterName: (variável local)
    OutCharacterTitle: (variável local)
    ↓
[Branch]
    Condition: ReturnValue (do ParsePlayerInfoUpdate)
    True → [Get Game Instance]
           ↓
           [Cast to Umbra Game Instance]
           ↓
           [Update Remote Player Nameplate] (função do UmbraGameInstance)
                 PlayerID: OutPlayerID
                 CharacterName: OutCharacterName
                 CharacterTitle: OutCharacterTitle
           ↓
           [Return Node] (IMPORTANTE: retornar para não processar como StateUpdate)
    False → (continuar processamento normal de movimento - StateUpdate)
```

### **2.2. Conectar Delegate `OnRemotePlayerNameplateUpdated` no Blueprint**

**No Blueprint `BP_NetMovementClient` → `Event BeginPlay`:**

```
[Event BeginPlay]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Assign OnRemotePlayerNameplateUpdated] (delegate do UmbraGameInstance)
    ↓
[Custom Event: UpdateNameplateFromDelegate]
    Inputs: PlayerID, CharacterName, CharacterTitle
    ↓
[ForEach Loop] (iterar sobre RemoteActors e RemoteActorIds)
    Array: RemoteActors
    Array Index: (variável local)
    ↓
[Get Array Element] (do RemoteActorIds)
    Index: Array Index
    ↓
[Equal (Integer)]
    A: ReturnValue (do Get Array Element)
    B: PlayerID (do input)
    ↓
[Branch]
    Condition: ReturnValue (do Equal)
    True → [Get Array Element] (do RemoteActors)
              Index: Array Index
              ↓
           [Is Valid]
              ↓
           [Branch]
              Condition: ReturnValue
              True → [Get Component by Class]
                        Class: Widget Component (ou o componente que gerencia o nameplate)
                        ↓
                     [Is Valid]
                        ↓
                     [Branch]
                        Condition: ReturnValue
                        True → [Get User Widget Object]
                                  ↓
                              [Cast to WBP Player Nameplate]
                                  ↓
                              [Branch] (bSuccess)
                                  True → [Update Nameplate] (função do widget)
                                            CharacterName: CharacterName (do input)
                                            TitleName: CharacterTitle (do input)
```

---

## ✅ PARTE 3: Atualizar Widget ao Equipar/Desequipar

### **3.1. Sistema Já Implementado no C++**

O sistema já está implementado no C++:
- Quando um item é equipado/desequipado, `OnEquipItemRequestComplete` é chamado
- Ele chama `LoadCharacterInfo()`, que dispara `OnCharacterInfoLoaded`
- `OnCharacterInfoLoaded` chama `SendPlayerInfoUpdate` automaticamente
- Se o próprio player estiver selecionado, `InspectPlayer` é chamado para atualizar o widget

### **3.2. Verificar se o Widget Está Atualizando**

**No Blueprint `WBP_PlayerHUD` → `OnSelectedPlayerInfoUpdated_Event`:**

Certifique-se de que o `UpdateDisplay` está recebendo o `PlayerInfo` do INPUT do evento:

```
[OnSelectedPlayerInfoUpdated_Event]
    Input: PlayerInfo (FUmbraRemotePlayerInfo)
    ↓
[Is Valid] (SelectedPlayerInfoWidgetREF)
    ↓
[Branch]
    True → [Remove from Parent] (SelectedPlayerInfoWidgetREF)
           ↓
           [Create Widget] (WBP_SelectedPlayerInfo)
           ↓
           [Set SelectedPlayerInfoWidgetREF] = ReturnValue
           ↓
           [Add to Viewport]
           ↓
           [Update Display] (do widget criado)
                 PlayerInfo: PlayerInfo (do INPUT do evento)
           ↓
           [Set Position in Viewport]
                 X: 500, Y: 200 (ou a posição desejada)
```

**IMPORTANTE:** O `UpdateDisplay` deve receber o `PlayerInfo` do **INPUT** do evento, não de `CachedPlayerInfo` ou qualquer outra fonte.

---

## ✅ PARTE 4: Verificar Nameplate do Próprio Player

### **4.1. No Blueprint `BP_ThirdPersonCharacter` → `Event BeginPlay`**

**Conecte o delegate `OnCharacterInfoLoaded` para atualizar o nameplate:**

```
[Event BeginPlay]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Assign OnCharacterInfoLoaded] (delegate do UmbraGameInstance)
    ↓
[Custom Event: UpdateNameplateFromCharacterInfo]
    Input: CharacterInfo (FUmbraCharacterInfo)
    ↓
[Get NameplateWidgetComponent] (Variable Get)
    ↓
[Is Valid]
    ↓
[Branch]
    True → [Get User Widget Object]
              ↓
           [Cast to WBP Player Nameplate]
              ↓
           [Branch] (bSuccess)
              True → [Update Nameplate] (função do widget)
                        CharacterName: Character Name (do Break UmbraCharacterInfo)
                        TitleName: Title Name (do Break UmbraCharacterInfo)
```

---

## 📋 CHECKLIST DE IMPLEMENTAÇÃO

### **No `BP_NetMovementClient`:**

- [ ] Adicionar verificação de `PlayerInfoUpdate` no `OnWSBinaryMessage` (ANTES de processar StateUpdate)
- [ ] Conectar delegate `OnRemotePlayerNameplateUpdated` no `Event BeginPlay`
- [ ] Criar função `UpdateNameplateFromDelegate` para atualizar nameplate de remote actors

### **No `BP_ThirdPersonCharacter`:**

- [ ] Conectar delegate `OnCharacterInfoLoaded` no `Event BeginPlay`
- [ ] Criar função `UpdateNameplateFromCharacterInfo` para atualizar nameplate do próprio player

### **No `WBP_PlayerHUD`:**

- [ ] Verificar se `OnSelectedPlayerInfoUpdated_Event` está passando `PlayerInfo` do INPUT para `UpdateDisplay`
- [ ] Verificar se o widget está sendo removido e recriado corretamente

### **No `WBP_PlayerNameplate`:**

- [ ] Verificar se a função `UpdateNameplate` existe e está configurando `CharacterName` e `TitleName` corretamente

---

## 🔍 LOGS PARA VERIFICAR

### **Quando enviar PlayerInfoUpdate:**
```
[UmbraGameInstance] 📤 PlayerInfoUpdate enviado via WebSocket: PlayerID X, Nome: NomeDoPlayer, Título: TítuloDoPlayer
```

### **Quando receber PlayerInfoUpdate:**
```
[UmbraGameInstance] 📝 Atualizando nameplate: PlayerID X, Nome: NomeDoPlayer, Título: TítuloDoPlayer, Actor: BP_RemotePlayer_C_X
[UmbraGameInstance] ✅ Nameplate atualizado e widget do player selecionado atualizado!
```

### **Quando equipar/desequipar:**
```
[UmbraGameInstance] 🔄 Atualizando widget do próprio player após equipar/desequipar...
[UmbraGameInstance] 🔍 Inspecionando jogador X...
```

---

## 🎯 PRÓXIMOS PASSOS

1. **Implementar processamento de `PlayerInfoUpdate` no Blueprint** (`BP_NetMovementClient::OnWSBinaryMessage`)
2. **Conectar delegate `OnRemotePlayerNameplateUpdated`** para atualizar nameplates de remote actors
3. **Testar com 2+ clients** - verificar se os nameplates aparecem corretamente
4. **Testar equipar/desequipar** - verificar se o widget atualiza HP/MP corretamente

---

**Após essas implementações, o sistema de nameplate e atualização de widget deve funcionar completamente!**

