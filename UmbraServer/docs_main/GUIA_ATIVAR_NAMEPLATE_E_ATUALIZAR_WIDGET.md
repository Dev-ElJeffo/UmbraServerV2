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

---

## ✅ PARTE 2: Processar PlayerInfoUpdate no Blueprint

### **2.1. No Blueprint `BP_NetMovementClient` → `OnWSBinaryMessage`**

**Adicione a verificação para PlayerInfoUpdate (msgType = 4):**

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
    True → [Print String]
              InString: "✅ [OnWSBinaryMessage] PlayerInfoUpdate recebido: PlayerID={OutPlayerID}, Name={OutCharacterName}, Title={OutCharacterTitle}"
           ↓
           [Get Game Instance]
           ↓
           [Cast to Umbra Game Instance]
           ↓
           [Get Player ID From Actor] (do UmbraGameInstance)
              RemoteActor: (buscar o actor do PlayerID no RemoteActors array)
           ↓
           [Branch]
              Condition: ReturnValue (PlayerID encontrado)
              True → [Update Nameplate] (função customizada no Blueprint)
                        PlayerID: OutPlayerID
                        CharacterName: OutCharacterName
                        CharacterTitle: OutCharacterTitle
    False → (continuar processamento normal de movimento)
```

### **2.2. Criar Função `UpdateNameplate` no Blueprint**

**No Blueprint `BP_NetMovementClient`:**

1. **Crie uma nova função:**
   - **Nome:** `UpdateNameplate`
   - **Parâmetros:**
     - `PlayerID` (Integer)
     - `CharacterName` (String)
     - `CharacterTitle` (String)

2. **Implemente a lógica:**

```
[UpdateNameplate]
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
    B: PlayerID
    ↓
[Branch]
    Condition: ReturnValue (do Equal)
    True → [Get Array Element] (do RemoteActors)
              Index: Array Index
              ↓
           [Is Valid]
              ↓
           [Branch]
              Condition: ReturnValue (do Is Valid)
              True → [Get Component by Class]
                        Class: WBP_PlayerNameplate (ou o componente que gerencia o nameplate)
                        ↓
                     [Is Valid]
                        ↓
                     [Branch]
                        Condition: ReturnValue
                        True → [Update Nameplate Widget]
                                  PlayerID: PlayerID
                                  CharacterName: CharacterName
                                  CharacterTitle: CharacterTitle
```

**OU mais simples (se o nameplate é um componente do actor):**

```
[UpdateNameplate]
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
    B: PlayerID
    ↓
[Branch]
    Condition: ReturnValue (do Equal)
    True → [Get Array Element] (do RemoteActors)
              Index: Array Index
              ↓
           [Is Valid]
              ↓
           [Branch]
              Condition: ReturnValue (do Is Valid)
              True → [Call Function by Name] (ou usar interface/cast)
                        Function Name: "UpdateNameplate"
                        Target: ReturnValue (do Get Array Element)
                        CharacterName: CharacterName
                        CharacterTitle: CharacterTitle
```

---

## ✅ PARTE 3: Atualizar Widget ao Equipar/Desequipar

### **3.1. Problema Identificado**

O widget `WBP_SelectedPlayerInfo` não está atualizando quando:
- O próprio player equipa/desequipa itens (deve atualizar via `OnLoadCharacterInfoComplete`)
- Outros players equipam/desequipam itens (precisa re-inspecionar)

### **3.2. Solução para Próprio Player**

**JÁ IMPLEMENTADO:** O `OnLoadCharacterInfoComplete` já atualiza o `SelectedPlayerInfo` quando o player selecionado é o próprio player (`ActivePlayerID`).

**Verifique se está funcionando:**
- Quando você equipa/desequipa um item, o `LoadCharacterInfo()` é chamado
- O `OnLoadCharacterInfoComplete` verifica se `SelectedPlayer.PlayerID == ActivePlayerID`
- Se sim, atualiza o `SelectedPlayerInfo` com os novos valores de HP/MP

### **3.3. Solução para Outros Players**

**PROBLEMA:** Quando outro player equipa/desequipa itens, o widget não é atualizado automaticamente.

**SOLUÇÃO:** Não há uma forma automática de detectar quando outros players equipam/desequipam itens. O widget só será atualizado quando:
- O player for re-selecionado (chama `InspectPlayer` novamente)
- Ou quando o servidor enviar uma atualização via WebSocket (não implementado ainda)

**PARA AGORA:** O widget será atualizado quando o player for re-selecionado. Isso é o comportamento esperado.

---

## ✅ PARTE 4: Verificar Implementação no Blueprint

### **4.1. Verificar `OnWSBinaryMessage` no `BP_NetMovementClient`**

1. **Abra o Blueprint `BP_NetMovementClient`**
2. **Localize o evento `OnWSBinaryMessage`**
3. **Verifique se há processamento para `PlayerInfoUpdate` (msgType = 4)**

**Se NÃO existir, adicione:**

```
[OnWSBinaryMessage]
    Data: (TArray<uint8>)
    ↓
[WSBinaryBPFL::ParsePlayerInfoUpdate]
    Data: Data
    ↓
[Branch]
    Condition: ReturnValue
    True → [UpdateNameplate]
              PlayerID: OutPlayerID
              CharacterName: OutCharacterName
              CharacterTitle: OutCharacterTitle
    False → (continuar processamento normal)
```

### **4.2. Verificar `WBP_PlayerNameplate`**

**Verifique se o widget `WBP_PlayerNameplate` tem uma função `UpdateNameplate` ou similar que aceita:**
- `PlayerID` (Integer)
- `CharacterName` (String)
- `CharacterTitle` (String)

**Se não existir, crie:**

```
[UpdateNameplate] (função no WBP_PlayerNameplate)
    Inputs: PlayerID, CharacterName, CharacterTitle
    ↓
[Text_PlayerName] → Set Text = CharacterName
[Text_PlayerTitle] → Set Text = CharacterTitle
```

---

## 📋 CHECKLIST DE IMPLEMENTAÇÃO

### **C++ (JÁ IMPLEMENTADO):**
- [x] Função `SendPlayerInfoUpdate` implementada
- [x] Chamada automática em `OnLoadCharacterInfoComplete`
- [x] Busca automática do `WebSocketClient` se não fornecido

### **Blueprint (PRECISA SER FEITO):**
- [ ] Processar `PlayerInfoUpdate` no `OnWSBinaryMessage` do `BP_NetMovementClient`
- [ ] Criar função `UpdateNameplate` no `BP_NetMovementClient`
- [ ] Verificar/criar função `UpdateNameplate` no `WBP_PlayerNameplate`
- [ ] Conectar tudo corretamente

---

## 🔍 TESTES

1. **Teste Nameplate:**
   - Conecte 2 clients
   - Verifique se o nome e título aparecem acima dos remote actors
   - Equipe/desequipe um item e verifique se o nameplate é atualizado

2. **Teste Widget:**
   - Selecione seu próprio player
   - Equipe/desequipe um item
   - Verifique se o `WBP_SelectedPlayerInfo` atualiza com os novos valores de HP/MP

3. **Teste com Outros Players:**
   - Selecione outro player
   - Peça para ele equipar/desequipar um item
   - Re-selecione o player
   - Verifique se o widget mostra os valores atualizados

---

## 🎯 PRÓXIMOS PASSOS

1. **Compile o projeto C++** com as correções
2. **Implemente o processamento de `PlayerInfoUpdate` no Blueprint** conforme o guia
3. **Teste o nameplate** - deve aparecer nome e título acima dos remote actors
4. **Teste a atualização do widget** - deve atualizar quando você equipa/desequipa itens

---

**Após essas implementações, o sistema de nameplate e atualização do widget devem funcionar corretamente!**
