# 🔍 GUIA COMPLETO: Verificação e Correção dos Widgets de Seleção

## 🎯 OBJETIVO

Fazer os widgets funcionarem corretamente:
1. ✅ **WBP_SelectedPlayerInfo** aparece ao clicar com **botão esquerdo** no remote actor
2. ✅ **WBP_PlayerContextMenu** aparece ao clicar com **botão direito** no remote actor
3. ✅ Ambos fecham ao clicar em **espaço vazio** na tela
4. ✅ Não abrem sozinhos

---

## 📋 CHECKLIST DE VERIFICAÇÃO

### **ETAPA 1: Verificar Inputs (Input Actions/Input Mappings)**

#### **1.1. Verificar se os Input Actions existem**

**Localização:** `Content/Input/` ou `Config/Input/`

**Verificar:**
- [ ] Existe `IA_LeftClick` (ou similar) para clique esquerdo
- [ ] Existe `IA_RightClick` (ou similar) para clique direito

**Se não existirem, criar:**
1. **Right Click** no Content Browser → **Input** → **Input Action**
2. Nome: `IA_LeftClick`
3. **Value Type:** `Digital (bool)`
4. Repetir para `IA_RightClick`

---

#### **1.2. Verificar Input Mapping Context**

**Localização:** `Content/Input/IMC_Default` (ou similar)

**Verificar:**
- [ ] `IA_LeftClick` está mapeado para `Left Mouse Button`
- [ ] `IA_RightClick` está mapeado para `Right Mouse Button`

**Se não estiver mapeado:**
1. Abrir `IMC_Default`
2. **Add Mapping**
3. **Action:** `IA_LeftClick` → **Key:** `Left Mouse Button`
4. **Action:** `IA_RightClick` → **Key:** `Right Mouse Button`

---

#### **1.3. Verificar se Input Mapping Context está sendo adicionado**

**Localização:** `BP_ThirdPersonCharacter` ou `BP_PlayerController`

**No `BeginPlay` ou `SetupInputComponent`:**
- [ ] `IMC_Default` está sendo adicionado ao `Enhanced Input Subsystem`

**Se não estiver:**
```
[Event BeginPlay]
    ↓
[Get Enhanced Input Subsystem]
    ↓
[Add Mapping Context]
    Mapping Context: IMC_Default
    Priority: 0
```

---

### **ETAPA 2: Verificar Conexão dos Inputs com HandlePlayerSelectionClick**

#### **2.1. Verificar onde HandlePlayerSelectionClick é chamado**

**Localização:** `BP_ThirdPersonCharacter` ou `BP_PlayerController` ou `WBP_PlayerHUD`

**Verificar:**
- [ ] `IA_LeftClick` está conectado a `HandlePlayerSelectionClick` com `bRightClick = false`
- [ ] `IA_RightClick` está conectado a `HandlePlayerSelectionClick` com `bRightClick = true`

**Se não estiver conectado:**

**No Blueprint (Event Graph):**
```
[Event: OnActionStarted (IA_LeftClick)]
    ↓
[Get Player Controller]
    ↓
[Cast to UmbraEternumUEPlayerController]
    ↓
[Handle Player Selection Click]
    bRightClick: false
```

```
[Event: OnActionStarted (IA_RightClick)]
    ↓
[Get Player Controller]
    ↓
[Cast to UmbraEternumUEPlayerController]
    ↓
[Handle Player Selection Click]
    bRightClick: true
```

---

### **ETAPA 3: Verificar Delegates no WBP_PlayerHUD**

#### **3.1. Verificar Event Construct**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → **Event Construct**

**Verificar se está conectado:**
- [ ] `Get Player Controller`
- [ ] `Cast to UmbraEternumUEPlayerController`
- [ ] `Get Player Selection Component`
- [ ] **Bind Event to OnPlayerSelected** → `OnPlayerSelected_Event` (Custom Event)
- [ ] **Bind Event to OnPlayerDeselected** → `OnPlayerDeselected_Event` (Custom Event)
- [ ] **Bind Event to OnPlayerContextMenuRequested** → `OnPlayerContextMenuRequested_Event` (Custom Event)

**Estrutura correta:**
```
[Event Construct]
    ↓
[Get Player Controller]
    Player Index: 0
    ↓
[Cast to UmbraEternumUEPlayerController]
    ↓ (Cast Success)
[Get Player Selection Component]
    ↓
[Bind Event to OnPlayerSelected]
    Target: (PlayerSelectionComponent)
    Event: OnPlayerSelected_Event (Custom Event)
    ↓
[Bind Event to OnPlayerDeselected]
    Target: (PlayerSelectionComponent)
    Event: OnPlayerDeselected_Event (Custom Event)
    ↓
[Bind Event to OnPlayerContextMenuRequested]
    Target: (PlayerSelectionComponent)
    Event: OnPlayerContextMenuRequested_Event (Custom Event)
```

---

#### **3.2. Verificar OnPlayerSelected_Event (Clique Esquerdo)**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → **Custom Event: OnPlayerSelected_Event**

**Verificar:**
- [ ] Tem **INPUT** `PlayerInfo` (tipo: `FUmbraRemotePlayerInfo`)
- [ ] Cria `WBP_SelectedPlayerInfo`
- [ ] Adiciona à viewport
- [ ] Posiciona corretamente
- [ ] Chama `UpdateDisplay`
- [ ] **NÃO cria** `WBP_PlayerContextMenu` aqui

**Estrutura correta:**
```
[OnPlayerSelected_Event]
    PlayerInfo (FUmbraRemotePlayerInfo) - INPUT
    ↓
[Print String] "🔵 Clique Esquerdo - PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE: (já existe, apenas atualizar)
    │   ↓
    │   [UpdateDisplay]
    │       Target: SelectedPlayerInfoWidgetREF
    │       UpdateDisplay: PlayerInfo
    │
    └─ FALSE: (criar novo)
        ↓
        [Get Player Controller]
        ↓
        [Create Widget]
            Class: WBP_SelectedPlayerInfo
            OwningPlayer: (PlayerController)
        ↓
        [Set SelectedPlayerInfoWidgetREF] = (widget criado)
        ↓
        [Add to Viewport]
            Target: (widget)
            Z-Order: 100
        ↓
        [Set Position in Viewport]
            Target: (widget)
            Position: X=50.0, Y=50.0
        ↓
        [Set Visibility]
            Target: (widget)
            Visibility: Visible
        ↓
        [UpdateDisplay]
            Target: (widget)
            UpdateDisplay: PlayerInfo
```

---

#### **3.3. Verificar OnPlayerContextMenuRequested_Event (Clique Direito)**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → **Custom Event: OnPlayerContextMenuRequested_Event**

**Verificar:**
- [ ] Tem **INPUT** `PlayerInfo` (tipo: `FUmbraRemotePlayerInfo`)
- [ ] Tem **INPUT** `ScreenPosition` (tipo: `Vector2D`)
- [ ] Cria `WBP_PlayerContextMenu`
- [ ] Adiciona à viewport
- [ ] Posiciona na posição do cursor (`ScreenPosition`)
- [ ] **NÃO cria** `WBP_SelectedPlayerInfo` aqui

**Estrutura correta:**
```
[OnPlayerContextMenuRequested_Event]
    PlayerInfo (FUmbraRemotePlayerInfo) - INPUT
    ScreenPosition (Vector2D) - INPUT
    ↓
[Print String] "🔵 Clique Direito - PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE: (já existe, apenas atualizar posição)
    │   ↓
    │   [Set Position in Viewport]
    │       Target: PlayerContextMenuWidgetREF
    │       Position: ScreenPosition
    │
    └─ FALSE: (criar novo)
        ↓
        [Get Player Controller]
        ↓
        [Create Widget]
            Class: WBP_PlayerContextMenu
            OwningPlayer: (PlayerController)
        ↓
        [Set PlayerContextMenuWidgetREF] = (widget criado)
        ↓
        [Add to Viewport]
            Target: (widget)
            Z-Order: 200 (acima do SelectedPlayerInfo)
        ↓
        [Set Position in Viewport]
            Target: (widget)
            Position: ScreenPosition (do input)
        ↓
        [Set Visibility]
            Target: (widget)
            Visibility: Visible
        ↓
        [UpdateDisplay] (se o widget tiver essa função)
            Target: (widget)
            UpdateDisplay: PlayerInfo
```

---

#### **3.4. Verificar OnPlayerDeselected_Event (Clique em Espaço Vazio)**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → **Custom Event: OnPlayerDeselected_Event**

**Verificar:**
- [ ] **NÃO tem inputs** (é um evento sem parâmetros)
- [ ] Remove/esconde `WBP_SelectedPlayerInfo`
- [ ] Remove/esconde `WBP_PlayerContextMenu`

**Estrutura correta:**
```
[OnPlayerDeselected_Event]
    (sem inputs)
    ↓
[Print String] "🔵 Deselecionado - Fechando widgets" (DEBUG)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Remove from Parent]
    │       Target: SelectedPlayerInfoWidgetREF
    │   ↓
    │   [Set SelectedPlayerInfoWidgetREF] = null
    │
    └─ FALSE: (não fazer nada)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Remove from Parent]
    │       Target: PlayerContextMenuWidgetREF
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = null
    │
    └─ FALSE: (não fazer nada)
```

---

### **ETAPA 4: Verificar Variáveis no WBP_PlayerHUD**

#### **4.1. Verificar Variáveis de Referência**

**Localização:** `WBP_PlayerHUD` → **My Blueprint** → **Variables**

**Verificar se existem:**
- [ ] `SelectedPlayerInfoWidgetREF` (tipo: `WBP_SelectedPlayerInfo` ou `User Widget`)
- [ ] `PlayerContextMenuWidgetREF` (tipo: `WBP_PlayerContextMenu` ou `User Widget`)

**Se não existirem, criar:**
1. **+ Variable**
2. **Variable Name:** `SelectedPlayerInfoWidgetREF`
3. **Variable Type:** `WBP_SelectedPlayerInfo` (ou `User Widget` se não encontrar)
4. **Instance Editable:** ❌ (não marcar)
5. Repetir para `PlayerContextMenuWidgetREF`

---

### **ETAPA 5: Verificar se Widgets Não Estão Sendo Criados Automaticamente**

#### **5.1. Verificar Event Construct dos Widgets**

**Localização:** `WBP_SelectedPlayerInfo` → **Event Graph** → **Event Construct**

**Verificar:**
- [ ] **NÃO há** `Add to Viewport` no `Event Construct`
- [ ] **NÃO há** lógica que cria o widget automaticamente

**Se houver, REMOVER!**

---

**Localização:** `WBP_PlayerContextMenu` → **Event Graph** → **Event Construct**

**Verificar:**
- [ ] **NÃO há** `Add to Viewport` no `Event Construct`
- [ ] **NÃO há** lógica que cria o widget automaticamente

**Se houver, REMOVER!**

---

#### **5.2. Verificar se Há Bindings Automáticos**

**Localização:** `WBP_PlayerHUD` → **Event Graph**

**Verificar:**
- [ ] **NÃO há** `Bind Event to OnPlayerSelected` que cria widgets automaticamente no `BeginPlay` sem verificar clique
- [ ] **NÃO há** timers ou delays que criam widgets automaticamente

---

### **ETAPA 6: Verificar Logs para Debug**

#### **6.1. Adicionar Logs Temporários**

**No `OnPlayerSelected_Event`:**
```
[Print String]
    In String: "🔵 [OnPlayerSelected] PlayerID: " + ToString(PlayerInfo.PlayerID) + ", Nome: " + PlayerInfo.CharacterName
```

**No `OnPlayerContextMenuRequested_Event`:**
```
[Print String]
    In String: "🔵 [OnPlayerContextMenuRequested] PlayerID: " + ToString(PlayerInfo.PlayerID) + ", Pos: " + ToString(ScreenPosition.X) + "," + ToString(ScreenPosition.Y)
```

**No `OnPlayerDeselected_Event`:**
```
[Print String]
    In String: "🔵 [OnPlayerDeselected] Fechando widgets"
```

**No `HandlePlayerSelectionClick` (se possível adicionar no Blueprint):**
```
[Print String]
    In String: "🔵 [HandlePlayerSelectionClick] bRightClick: " + ToString(bRightClick)
```

---

## 🔧 CORREÇÕES COMUNS

### **Problema 1: Widgets Abrem Sozinhos**

**Causa:** Widgets sendo criados no `Event Construct` ou `BeginPlay` sem verificar clique.

**Solução:**
- Remover qualquer `Add to Viewport` dos `Event Construct` dos widgets
- Garantir que widgets só são criados nos eventos `OnPlayerSelected_Event` e `OnPlayerContextMenuRequested_Event`

---

### **Problema 2: Widgets Não Fecham ao Clicar em Espaço Vazio**

**Causa:** `OnPlayerDeselected_Event` não está sendo chamado ou não está removendo os widgets.

**Solução:**
- Verificar se `OnPlayerDeselected` está conectado corretamente
- Verificar se `OnPlayerDeselected_Event` está removendo os widgets com `Remove from Parent`
- Verificar se `ClearSelection()` está sendo chamado quando clica em espaço vazio (já está no C++, mas verificar logs)

---

### **Problema 3: Widget Aparece no Lugar Errado**

**Causa:** Falta `Set Position in Viewport` ou posição incorreta.

**Solução:**
- Adicionar `Set Position in Viewport` após `Add to Viewport`
- Para `WBP_SelectedPlayerInfo`: posição fixa (ex: X=50, Y=50)
- Para `WBP_PlayerContextMenu`: usar `ScreenPosition` do input

---

### **Problema 4: Widgets Não Aparecem**

**Causa:** Falta `Add to Viewport` ou `Z-Order` muito baixo.

**Solução:**
- Adicionar `Add to Viewport` após criar o widget
- Aumentar `Z-Order` (ex: 100 para SelectedPlayerInfo, 200 para ContextMenu)

---

## 📝 CHECKLIST FINAL

Antes de testar, verificar:

- [ ] Input Actions criados e mapeados
- [ ] Input Mapping Context adicionado no BeginPlay
- [ ] `HandlePlayerSelectionClick` conectado aos Input Actions
- [ ] Delegates conectados no `Event Construct` do `WBP_PlayerHUD`
- [ ] `OnPlayerSelected_Event` cria apenas `WBP_SelectedPlayerInfo`
- [ ] `OnPlayerContextMenuRequested_Event` cria apenas `WBP_PlayerContextMenu`
- [ ] `OnPlayerDeselected_Event` remove ambos os widgets
- [ ] Variáveis de referência criadas
- [ ] Logs de debug adicionados
- [ ] Widgets **NÃO** têm `Add to Viewport` no `Event Construct`

---

## 🧪 TESTE PASSO A PASSO

1. **Compilar** o projeto
2. **Abrir** o jogo com 2 clients
3. **Clicar com botão esquerdo** em um remote actor
   - ✅ Deve aparecer `WBP_SelectedPlayerInfo`
   - ✅ Deve aparecer log: `🔵 [OnPlayerSelected] ...`
4. **Clicar com botão direito** no mesmo remote actor
   - ✅ Deve aparecer `WBP_PlayerContextMenu` na posição do cursor
   - ✅ Deve aparecer log: `🔵 [OnPlayerContextMenuRequested] ...`
5. **Clicar em espaço vazio**
   - ✅ Ambos os widgets devem desaparecer
   - ✅ Deve aparecer log: `🔵 [OnPlayerDeselected] ...`
6. **Verificar se widgets NÃO abrem sozinhos**
   - ✅ Não devem aparecer sem clicar

---

**FIM DO GUIA**
