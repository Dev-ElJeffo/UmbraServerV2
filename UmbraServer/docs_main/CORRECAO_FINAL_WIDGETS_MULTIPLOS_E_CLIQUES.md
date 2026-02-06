# 🔴 CORREÇÃO FINAL: Widgets Múltiplos + HUD Some + Cliques Incorretos

## 🎯 PROBLEMAS IDENTIFICADOS (Baseado na Imagem)

1. **Múltiplos widgets sendo criados a cada clique** - Vários `WBP_PlayerContextMenu` abertos simultaneamente
2. **HUD principal some ao clicar fora** - `WBP_PlayerHUD` (HP/MP) desaparece
3. **Clique esquerdo abre menu de contexto** - Quando deveria selecionar o player

---

## ✅ CORREÇÃO 1: Reutilizar Widgets (Não Criar Novos)

### **PROBLEMA:** Cada clique cria um novo widget em vez de reutilizar o existente

### **SOLUÇÃO: Verificar se Widget Já Existe Antes de Criar**

---

### **PASSO 1: Corrigir OnPlayerSelected_Event**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerSelected_Event`

**Estrutura CORRETA:**

```
[OnPlayerSelected_Event]
    PlayerInfo (input)
    ↓
[Print String] "🔵 [OnPlayerSelected] PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE: (widget já existe, apenas atualizar)
    │   ↓
    │   [Print String] "🔵 Widget já existe, apenas atualizando dados"
    │   ↓
    │   [UpdateDisplay] (ou função similar)
    │       Target: SelectedPlayerInfoWidgetREF
    │       PlayerInfo: PlayerInfo (input)
    │   ↓
    │   [Set Visibility]
    │       Target: SelectedPlayerInfoWidgetREF
    │       Visibility: Visible
    │
    └─ FALSE: (criar novo widget)
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
            Target: (widget criado)
            Z-Order: 100
        ↓
        [Set Position in Viewport]
            Target: (widget criado)
            Position: X=50.0, Y=50.0
        ↓
        [Set Visibility]
            Target: (widget criado)
            Visibility: Visible
        ↓
        [UpdateDisplay] (ou função similar)
            Target: (widget criado)
            PlayerInfo: PlayerInfo (input)
```

**⚠️ CRÍTICO:** Sempre verificar `Is Valid?` antes de criar! Se já existe, apenas atualizar!

---

### **PASSO 2: Corrigir OnPlayerContextMenuRequested_Event**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerContextMenuRequested_Event`

**Estrutura CORRETA:**

```
[OnPlayerContextMenuRequested_Event]
    PlayerInfo (input)
    ScreenPosition (input)
    ↓
[Print String] "🔵 [OnPlayerContextMenu] PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE: (widget já existe, apenas atualizar posição)
    │   ↓
    │   [Print String] "🔵 Widget já existe, apenas atualizando posição"
    │   ↓
    │   [Set Position in Viewport]
    │       Target: PlayerContextMenuWidgetREF
    │       Position: ScreenPosition (input)
    │   ↓
    │   [Set Visibility]
    │       Target: PlayerContextMenuWidgetREF
    │       Visibility: Visible
    │   ↓
    │   [UpdateDisplay] (se o widget tiver essa função)
    │       Target: PlayerContextMenuWidgetREF
    │       PlayerInfo: PlayerInfo (input)
    │
    └─ FALSE: (criar novo widget)
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
            Target: (widget criado)
            Z-Order: 200
        ↓
        [Set Position in Viewport]
            Target: (widget criado)
            Position: ScreenPosition (input)
        ↓
        [Set Visibility]
            Target: (widget criado)
            Visibility: Visible
        ↓
        [UpdateDisplay] (se o widget tiver essa função)
            Target: (widget criado)
            PlayerInfo: PlayerInfo (input)
```

**⚠️ CRÍTICO:** Sempre verificar `Is Valid?` antes de criar! Se já existe, apenas atualizar posição e dados!

---

## ✅ CORREÇÃO 2: HUD Principal Não Deve Sumir

### **PROBLEMA:** `OnPlayerDeselected_Event` está removendo o `WBP_PlayerHUD` principal

### **SOLUÇÃO: Remover APENAS Widgets de Seleção**

---

### **PASSO 3: Corrigir OnPlayerDeselected_Event**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerDeselected_Event`

**Estrutura CORRETA:**

```
[OnPlayerDeselected_Event]
    (sem inputs)
    ↓
[Print String] "🔵 [OnPlayerDeselected] Fechando widgets de seleção" (DEBUG)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Print String] "🔵 Removendo SelectedPlayerInfoWidgetREF"
    │   ↓
    │   [Remove from Parent]
    │       Target: SelectedPlayerInfoWidgetREF  ← APENAS este widget!
    │   ↓
    │   [Set SelectedPlayerInfoWidgetREF] = null
    │
    └─ FALSE:
        ↓
        [Print String] "🔵 SelectedPlayerInfoWidgetREF não é válido"
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Print String] "🔵 Removendo PlayerContextMenuWidgetREF"
    │   ↓
    │   [Remove from Parent]
    │       Target: PlayerContextMenuWidgetREF  ← APENAS este widget!
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = null
    │
    └─ FALSE:
        ↓
        [Print String] "🔵 PlayerContextMenuWidgetREF não é válido"
```

**⚠️ CRÍTICO:** 
- **NÃO** use `Remove from Parent` no próprio `WBP_PlayerHUD` (self)
- **NÃO** use `Set Visibility` no próprio `WBP_PlayerHUD` (self)
- **APENAS** remova os widgets de seleção (`SelectedPlayerInfoWidgetREF` e `PlayerContextMenuWidgetREF`)

---

## ✅ CORREÇÃO 3: Inverter Lógica de Cliques

### **PROBLEMA:** Clique esquerdo abre menu de contexto quando deveria selecionar

### **SOLUÇÃO: Inverter Valores de bRightClick no Blueprint**

---

### **PASSO 4: Corrigir Valores de bRightClick**

**Localização:** `BP_ThirdPersonCharacter` → **Event Graph**

**O usuário quer:**
- **Clique DIREITO** = Selecionar player (abrir `WBP_SelectedPlayerInfo`)
- **Clique ESQUERDO** = Menu de contexto (abrir `WBP_PlayerContextMenu`)

**O C++ espera:**
- `bRightClick = true` → Seleciona player (dispara `OnPlayerSelected`)
- `bRightClick = false` → Abre menu de contexto (dispara `OnPlayerContextMenuRequested`)

**Então no Blueprint:**

1. **`IA_SelectPlayer` (Left Mouse Button)**
   - **ANTES:** `HandlePlayerSelectionClick(bRightClick = false)` ❌
   - **DEPOIS:** `HandlePlayerSelectionClick(bRightClick = false)` ✅ **CORRETO!**
   - **Por quê?** `bRightClick = false` abre menu de contexto, que é o que o clique esquerdo deve fazer

2. **`IA_ContextMenu` (Right Mouse Button)**
   - **ANTES:** `HandlePlayerSelectionClick(bRightClick = true)` ❌
   - **DEPOIS:** `HandlePlayerSelectionClick(bRightClick = true)` ✅ **CORRETO!**
   - **Por quê?** `bRightClick = true` seleciona player, que é o que o clique direito deve fazer

**⚠️ ATENÇÃO:** Parece que os valores já estão corretos! O problema pode estar em outro lugar.

**Verificar:**
- [ ] `IA_SelectPlayer` está mapeado para **Left Mouse Button**?
- [ ] `IA_ContextMenu` está mapeado para **Right Mouse Button**?
- [ ] Os valores de `bRightClick` estão corretos conforme acima?

---

## 📋 CHECKLIST COMPLETO DE CORREÇÃO

### **Blueprint - OnPlayerSelected_Event:**
- [ ] Verifica `Is Valid?` antes de criar widget
- [ ] Se widget existe, apenas atualiza (não cria novo)
- [ ] **NÃO** cria `WBP_PlayerContextMenu`
- [ ] **NÃO** remove/esconde `WBP_PlayerHUD` (self)

### **Blueprint - OnPlayerContextMenuRequested_Event:**
- [ ] Verifica `Is Valid?` antes de criar widget
- [ ] Se widget existe, apenas atualiza posição (não cria novo)
- [ ] **NÃO** cria `WBP_SelectedPlayerInfo`
- [ ] **NÃO** remove/esconde `WBP_PlayerHUD` (self)

### **Blueprint - OnPlayerDeselected_Event:**
- [ ] **APENAS** remove `SelectedPlayerInfoWidgetREF`
- [ ] **APENAS** remove `PlayerContextMenuWidgetREF`
- [ ] **NÃO** remove/esconde `WBP_PlayerHUD` (self)
- [ ] Limpa referências (set para null)

### **Blueprint - Input Actions:**
- [ ] `IA_SelectPlayer` (Left Mouse) → `bRightClick = false`
- [ ] `IA_ContextMenu` (Right Mouse) → `bRightClick = true`

---

## 🧪 TESTE PASSO A PASSO

1. **Compilar** o C++ (se necessário)
2. **Abrir** o jogo com 2 clients
3. **Verificar HUD Principal:**
   - ✅ Barras de HP/MP devem estar visíveis no canto da tela
4. **Clicar com botão ESQUERDO** em um remote actor (primeira vez)
   - ✅ Deve aparecer **APENAS** `WBP_PlayerContextMenu`
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**
5. **Clicar com botão ESQUERDO** no mesmo remote actor novamente
   - ✅ **NÃO** deve criar novo widget
   - ✅ Deve apenas atualizar posição do menu existente
6. **Clicar com botão DIREITO** no mesmo remote actor
   - ✅ Deve aparecer **APENAS** `WBP_SelectedPlayerInfo`
   - ✅ `WBP_PlayerContextMenu` deve ser fechado/escondido
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**
7. **Clicar em espaço vazio**
   - ✅ `WBP_SelectedPlayerInfo` deve desaparecer
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**
8. **Clicar com botão DIREITO** no remote actor novamente
   - ✅ Deve aparecer `WBP_SelectedPlayerInfo` (deve funcionar após desselecionar)
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**

---

## 🔍 DEBUG: Logs para Verificar

**Adicione logs temporários em cada evento:**

**No `OnPlayerSelected_Event`:**
```
[Print String] "🔵 [OnPlayerSelected] PlayerID: " + ToString(PlayerInfo.PlayerID)
[Print String] "🔵 [OnPlayerSelected] Widget existe? " + ToString(Is Valid? result)
```

**No `OnPlayerContextMenuRequested_Event`:**
```
[Print String] "🔵 [OnPlayerContextMenu] PlayerID: " + ToString(PlayerInfo.PlayerID)
[Print String] "🔵 [OnPlayerContextMenu] Widget existe? " + ToString(Is Valid? result)
```

**No `OnPlayerDeselected_Event`:**
```
[Print String] "🔵 [OnPlayerDeselected] DISPARADO!"
[Print String] "🔵 [OnPlayerDeselected] Removendo SelectedPlayerInfoWidgetREF"
[Print String] "🔵 [OnPlayerDeselected] Removendo PlayerContextMenuWidgetREF"
```

**Verificar no Output Log:**
- Se aparecer "Widget existe? True" mas ainda criar novo widget → **ERRO!**
- Se aparecer "Removendo WBP_PlayerHUD" → **ERRO CRÍTICO!**

---

**FIM DO GUIA**
