# 🔴 CORREÇÃO CRÍTICA: HUD Principal (HP/MP) Some ao Clicar em Outro Player

## 🎯 PROBLEMA IDENTIFICADO

**Sintoma:** Ao clicar em outro player, a HUD principal (`WBP_PlayerHUD`) que mostra HP e MP do próprio jogador **desaparece**.

**Causa Provável:** O `OnPlayerDeselected_Event` está removendo/escondendo o `WBP_PlayerHUD` em vez de apenas remover os widgets de seleção (`WBP_SelectedPlayerInfo` e `WBP_PlayerContextMenu`).

---

## ✅ SOLUÇÃO: Verificar OnPlayerDeselected_Event

### **CRÍTICO: OnPlayerDeselected_Event NÃO DEVE Afetar WBP_PlayerHUD**

**O `WBP_PlayerHUD` é o widget principal que mostra HP/MP do próprio jogador e DEVE permanecer visível sempre!**

**O `OnPlayerDeselected_Event` deve APENAS remover:**
- `WBP_SelectedPlayerInfo` (widget de informações do player selecionado)
- `WBP_PlayerContextMenu` (widget de menu de contexto)

**O `OnPlayerDeselected_Event` NUNCA deve remover/esconder:**
- `WBP_PlayerHUD` (HUD principal com HP/MP)
- Qualquer outro widget permanente do HUD

---

## 🔧 VERIFICAÇÃO NO BLUEPRINT

### **PASSO 1: Verificar OnPlayerDeselected_Event**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerDeselected_Event`

**Verificar se há:**
- [ ] **NÃO há** `Remove from Parent` para o próprio `WBP_PlayerHUD` (self)
- [ ] **NÃO há** `Set Visibility` para o próprio `WBP_PlayerHUD` (self) com `Collapsed` ou `Hidden`
- [ ] **APENAS** remove `SelectedPlayerInfoWidgetREF` e `PlayerContextMenuWidgetREF`

---

### **PASSO 2: Estrutura Correta do OnPlayerDeselected_Event**

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
    │   [Remove from Parent]
    │       Target: SelectedPlayerInfoWidgetREF  ← APENAS este widget!
    │   ↓
    │   [Set SelectedPlayerInfoWidgetREF] = null
    │
    └─ FALSE: (não fazer nada)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Remove from Parent]
    │       Target: PlayerContextMenuWidgetREF  ← APENAS este widget!
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = null
    │
    └─ FALSE: (não fazer nada)
```

**⚠️ IMPORTANTE:**
- **NÃO** use `Remove from Parent` no próprio `WBP_PlayerHUD` (self)
- **NÃO** use `Set Visibility` no próprio `WBP_PlayerHUD` (self)
- **APENAS** remova os widgets de seleção (`SelectedPlayerInfoWidgetREF` e `PlayerContextMenuWidgetREF`)

---

### **PASSO 3: Verificar se WBP_PlayerHUD Está Sendo Afetado em Outros Lugares**

**Verificar também:**

1. **`OnPlayerSelected_Event`**
   - [ ] **NÃO** remove/esconde o `WBP_PlayerHUD` (self)
   - [ ] **APENAS** cria/atualiza `WBP_SelectedPlayerInfo`

2. **`OnPlayerContextMenuRequested_Event`**
   - [ ] **NÃO** remove/esconde o `WBP_PlayerHUD` (self)
   - [ ] **APENAS** cria/atualiza `WBP_PlayerContextMenu`

3. **Qualquer outro evento ou função**
   - [ ] **NÃO** remove/esconde o `WBP_PlayerHUD` (self) quando relacionado à seleção de players

---

## 🔍 DEBUG: Adicionar Logs para Verificar

**No `OnPlayerDeselected_Event`, adicione logs para verificar o que está sendo removido:**

```
[OnPlayerDeselected_Event]
    ↓
[Print String] "🔵 [OnPlayerDeselected] DISPARADO!"
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Print String] "🔵 Removendo SelectedPlayerInfoWidgetREF"
    │   ↓
    │   [Remove from Parent]
    │       Target: SelectedPlayerInfoWidgetREF
    │   ↓
    │   [Set SelectedPlayerInfoWidgetREF] = null
    │
    └─ FALSE:
        ↓
        [Print String] "🔵 SelectedPlayerInfoWidgetREF não é válido, não há nada para remover"
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Print String] "🔵 Removendo PlayerContextMenuWidgetREF"
    │   ↓
    │   [Remove from Parent]
    │       Target: PlayerContextMenuWidgetREF
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = null
    │
    └─ FALSE:
        ↓
        [Print String] "🔵 PlayerContextMenuWidgetREF não é válido, não há nada para remover"
```

**Verificar no Output Log:**
- Se aparecer `"Removendo SelectedPlayerInfoWidgetREF"` e `"Removendo PlayerContextMenuWidgetREF"` → Está correto
- Se aparecer algo sobre remover o próprio `WBP_PlayerHUD` → **ERRO!** Remover essa lógica!

---

## 📋 CHECKLIST DE CORREÇÃO

- [ ] Verificar se `OnPlayerDeselected_Event` **NÃO** remove/esconde o `WBP_PlayerHUD` (self)
- [ ] Verificar se `OnPlayerDeselected_Event` **APENAS** remove `SelectedPlayerInfoWidgetREF` e `PlayerContextMenuWidgetREF`
- [ ] Verificar se `OnPlayerSelected_Event` **NÃO** afeta o `WBP_PlayerHUD` (self)
- [ ] Verificar se `OnPlayerContextMenuRequested_Event` **NÃO** afeta o `WBP_PlayerHUD` (self)
- [ ] Adicionar logs de debug para verificar o que está sendo removido
- [ ] Testar: Clicar em outro player → HUD principal (HP/MP) deve permanecer visível
- [ ] Testar: Clicar em espaço vazio → Apenas widgets de seleção devem desaparecer, HUD principal permanece

---

## 🧪 TESTE PASSO A PASSO

1. **Compilar** o C++ (se necessário)
2. **Abrir** o jogo com 2 clients
3. **Verificar HUD Principal:**
   - ✅ Barras de HP/MP devem estar visíveis no canto da tela
4. **Clicar em outro player:**
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**
   - ✅ Widget de seleção (`WBP_SelectedPlayerInfo` ou `WBP_PlayerContextMenu`) deve aparecer
5. **Clicar em espaço vazio:**
   - ✅ Widget de seleção deve desaparecer
   - ✅ HUD principal (HP/MP) deve **PERMANECER VISÍVEL**

---

## ⚠️ PROBLEMA COMUM: Confusão Entre Widgets

**NÃO confunda:**
- `WBP_PlayerHUD` = Widget principal com HP/MP do próprio jogador (deve sempre estar visível)
- `WBP_SelectedPlayerInfo` = Widget que mostra informações do player selecionado (deve aparecer/desaparecer)
- `WBP_PlayerContextMenu` = Widget de menu de contexto (deve aparecer/desaparecer)

**O `OnPlayerDeselected_Event` deve afetar APENAS os dois últimos, nunca o primeiro!**

---

**FIM DO GUIA**
