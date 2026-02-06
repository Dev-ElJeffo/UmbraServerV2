# 🔴 CORREÇÃO: Widget com Dados Vazios e Posicionamento Incorreto

## 🎯 PROBLEMAS IDENTIFICADOS

### **Problema 1: Dados Vazios no Widget**

**Causa:** Quando você clica no remote actor, o `SelectPlayer` é chamado. Se o jogador **não está no cache**, ele cria um `FUmbraRemotePlayerInfo` vazio com apenas:
- `RemoteActor` = actor clicado
- `CharacterName` = nome do actor (ex: "BP_RemotePlayer_C_315")
- `PlayerID` = 0
- `Level` = 1 (padrão)
- `CurrentHealth` = 100 (padrão)
- `MaxHealth` = 100 (padrão)
- etc.

**Resultado:** O widget mostra "315" ou "0" ao invés do nome real do jogador.

---

### **Problema 2: Posicionamento no Canto da Tela**

**Causa:** O widget está sendo adicionado à viewport sem posicionamento específico, então ele aparece no canto superior esquerdo (posição padrão).

---

## ✅ SOLUÇÃO COMPLETA

### **SOLUÇÃO 1: Chamar `InspectPlayer` quando dados estão vazios**

**No `OnPlayerSelected_Event` do `WBP_PlayerHUD`:**

**Após receber o `PlayerInfo`, verifique se os dados estão vazios:**

```
[OnPlayerSelected_Event]
    PlayerInfo (FUmbraRemotePlayerInfo) - INPUT
    ↓
[Print String] "✅ OnPlayerSelected_Event - PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Branch]
    Condition: PlayerInfo.PlayerID > 0
    ↓
    ├─ TRUE (tem dados válidos):
    │   ↓
    │   [Create Widget] → WBP_SelectedPlayerInfo
    │   ↓
    │   [Add to Viewport]
    │   ↓
    │   [Set Position in Viewport]  ← ADICIONAR PARA POSICIONAMENTO!
    │       Target: (widget)
    │       Position: (calcular posição - ver abaixo)
    │   ↓
    │   [Set Visibility] → Visible
    │   ↓
    │   [UpdateDisplay] → PlayerInfo
    │
    └─ FALSE (dados vazios - PlayerID = 0):
        ↓
        [Get Player Controller]
        ↓
        [Cast to UmbraEternumUEPlayerController]
        ↓
        [Get Player Selection Component]
        ↓
        [Get Selected Player Actor]
        ↓
        [Is Valid?] (actor)
            ├─ TRUE:
            │   ↓
            │   [Get Game Instance]
            │   ↓
            │   [Cast to UmbraGameInstance]
            │   ↓
            │   [Inspect Player]
            │       Target Player ID: (obter PlayerID do actor - ver abaixo)
            │   ↓
            │   [Print String] "⏳ Carregando dados do jogador..." (DEBUG)
            │
            └─ FALSE:
                [Print String] "❌ Actor inválido!" (DEBUG)
```

---

### **SOLUÇÃO 2: Obter PlayerID do Actor**

**O problema:** Quando o `PlayerID` é 0, você precisa obter o PlayerID do actor de outra forma.

**Opção A: Usar Tag no Actor (Recomendado)**

Se o `BP_RemotePlayer` tem uma tag ou variável com o PlayerID:

```
[Get Selected Player Actor]
    ↓
[Get Actor Tags]
    ↓
[Find Tag]
    Tag: "PlayerID"
    ↓
[Get Tag Value]
    Tag: "PlayerID"
    ↓
[To Integer] (se necessário)
    ↓
[Inspect Player]
    Target Player ID: (PlayerID obtido)
```

**Opção B: Usar Map do GameInstance**

Se você armazenou o PlayerID no Map quando spawnou o actor:

```
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Get Remote Player Actors Map] (se exposto)
    ↓
[Find in Map]
    Key: (precisa do PlayerID de outra forma)
```

**Opção C: Modificar o C++ para obter PlayerID do Actor**

Se o `BP_RemotePlayer` tem uma variável `PlayerID`, você pode expor uma função C++ para obtê-la.

---

### **SOLUÇÃO 3: Posicionar o Widget Corretamente**

**No `OnPlayerSelected_Event`, após `Add to Viewport`:**

```
[Add to Viewport]
    Target: (widget)
    Z-Order: 100 (acima de outros widgets)
    ↓
[Get Viewport Size]
    ↓
[Set Position in Viewport]
    Target: (widget)
    Position:
        X: (ViewportSize.X * 0.5) - (WidgetSize.X * 0.5)  ← Centralizado horizontalmente
        Y: 50.0  ← 50 pixels do topo
    ↓
[Set Anchos in Viewport]
    Target: (widget)
    Anchors: Top Center (ou Top Left se preferir)
```

**OU, para posicionar próximo ao cursor:**

```
[Get Mouse Position]
    ↓
[Set Position in Viewport]
    Target: (widget)
    Position:
        X: (MouseX) + 20.0  ← 20 pixels à direita do cursor
        Y: (MouseY) - 20.0  ← 20 pixels acima do cursor
```

---

### **SOLUÇÃO 4: Atualizar Widget quando dados chegarem**

**Conecte ao `OnSelectedPlayerInfoUpdated`:**

No `Event Construct` do `WBP_PlayerHUD`, você já conectou `OnSelectedPlayerInfoUpdated`. 

**No `OnSelectedPlayerInfoUpdated_Event`:**

```
[OnSelectedPlayerInfoUpdated_Event]
    PlayerInfo (FUmbraRemotePlayerInfo) - INPUT
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [UpdateDisplay]
    │       Target: SelectedPlayerInfoWidgetREF
    │       UpdateDisplay: PlayerInfo
    │
    └─ FALSE:
        (widget não existe ainda, não fazer nada)
```

---

## 📋 CÓDIGO QUE PRECISO VER

Para fornecer a solução exata, preciso ver:

### **1. Blueprint do `WBP_SelectedPlayerInfo`**
- **Função `UpdateDisplay`:** Como ela está implementada?
- **Como os dados são exibidos:** Quais Text Blocks estão sendo atualizados?
- **Bindings:** Há algum binding que pode estar causando problemas?

### **2. Blueprint do `BP_RemotePlayer`**
- **Variáveis:** Há alguma variável que armazena o `PlayerID`?
- **Tags:** Quais tags o actor tem? Há uma tag com o PlayerID?

### **3. Blueprint do `WBP_PlayerHUD` - `OnPlayerSelected_Event`**
- **Código completo:** Envie o XML completo ou screenshot mostrando TODOS os nós
- **Como o `PlayerInfo` está sendo usado:** Está sendo passado corretamente para `UpdateDisplay`?

### **4. Logs do Output Log**
- Quando você clica no remote actor, aparecem logs?
- Procure por:
  - `[UmbraPlayerSelection] ⚠️ Jogador ... não está no cache`
  - `[UmbraPlayerSelection] ✅ Jogador selecionado: ...`
  - `[UmbraGameInstance] ✅ HandlePlayerInspectedInternal: ...`

---

## 🔧 CORREÇÃO TEMPORÁRIA (Enquanto não vejo o código)

### **No `OnPlayerSelected_Event`, adicione verificação:**

1. **Adicione um `Print String` para debug:**
   ```
   [Print String]
       In String: "PlayerID: " + ToString(PlayerInfo.PlayerID) + ", Nome: " + PlayerInfo.CharacterName
   ```

2. **Se `PlayerID` for 0, chame `InspectPlayer`:**
   ```
   [Branch]
       Condition: PlayerInfo.PlayerID > 0
       ├─ TRUE: (criar widget normalmente)
       └─ FALSE: (chamar InspectPlayer primeiro)
   ```

3. **Aguarde `OnSelectedPlayerInfoUpdated` para criar o widget:**
   - Não crie o widget imediatamente se `PlayerID = 0`
   - Crie apenas quando `OnSelectedPlayerInfoUpdated` disparar com dados válidos

---

## 📝 RESUMO DOS PROBLEMAS

1. **Dados vazios:** `SelectPlayer` cria dados básicos quando jogador não está no cache
2. **Solução:** Chamar `InspectPlayer` quando `PlayerID = 0`
3. **Posicionamento:** Widget aparece no canto porque não há `Set Position in Viewport`
4. **Solução:** Adicionar `Set Position in Viewport` após `Add to Viewport`

---

**Envie os códigos/blueprints solicitados para que eu possa fornecer a solução exata!**

---

**FIM DO GUIA**
