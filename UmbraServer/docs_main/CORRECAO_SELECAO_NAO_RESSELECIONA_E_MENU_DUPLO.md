# 🔴 CORREÇÃO: Seleção Não Resseleciona + Menu Duplo no Primeiro Clique

## 🎯 PROBLEMAS IDENTIFICADOS

### **Problema 1: Primeiro Clique Esquerdo Abre Ambos Widgets**

**Causa:** Quando você clica pela primeira vez com botão esquerdo, o Blueprint pode estar criando ambos os widgets (`WBP_SelectedPlayerInfo` e `WBP_PlayerContextMenu`) ao receber `OnPlayerContextMenuRequested`.

**Possível causa no Blueprint:**
- O `OnPlayerContextMenuRequested_Event` pode estar criando ambos os widgets
- OU o `OnPlayerSelected_Event` está sendo disparado mesmo quando não deveria

---

### **Problema 2: Após Desselecionar, Não Consegue Resselecionar**

**Causa:** O estado pode não estar sendo limpo corretamente, ou há algum problema com a lógica de seleção quando já há uma seleção ativa.

**Status:** ✅ **CORRIGIDO NO C++** - Ver abaixo

---

### **Problema 3: Clique Direito Não Abre Menu de Contexto**

**Causa:** Pode ser que o `bRightClick` esteja sendo passado incorretamente no Blueprint, ou que o `OpenContextMenu()` não esteja sendo chamado.

---

## ✅ CORREÇÕES IMPLEMENTADAS NO C++

### **1. Verificação de Mesmo Player Selecionado**

**Modificado:** `TrySelectPlayerUnderCursor`

**Mudança:**
- Agora verifica se o player clicado já é o mesmo que está selecionado
- Se for o mesmo player:
  - **Clique direito**: Ignora (não reabre `WBP_SelectedPlayerInfo`)
  - **Clique esquerdo**: Apenas abre o menu de contexto (não seleciona novamente)

**Código:**
```cpp
// Verificar se já está selecionado o mesmo player
bool bIsSamePlayer = bHasSelection && SelectedPlayer.RemoteActor == HitActor;

if (bRightClick)
{
    // Clique direito - selecionar jogador
    if (bIsSamePlayer)
    {
        // Mesmo player já selecionado, ignorar
        return true;
    }
    return SelectPlayer(HitActor, true);
}
else
{
    // Clique esquerdo - abrir menu de contexto
    if (bIsSamePlayer)
    {
        // Mesmo player já selecionado, apenas abrir menu
        OpenContextMenu();
        return true;
    }
    // Selecionar e abrir menu
    if (SelectPlayer(HitActor, false))
    {
        OpenContextMenu();
        return true;
    }
}
```

---

### **2. Melhor Validação em OpenContextMenu**

**Modificado:** `OpenContextMenu`

**Mudança:**
- Agora verifica se o `RemoteActor` é válido antes de abrir o menu
- Adiciona mais logs para debug

---

### **3. Limpeza de Estado em ClearSelection**

**Modificado:** `ClearSelection`

**Mudança:**
- Limpa o estado (`bHasSelection = false`) **ANTES** de disparar o evento
- Isso garante que o estado está limpo quando o Blueprint recebe `OnPlayerDeselected`

---

## 🔧 VERIFICAÇÕES NECESSÁRIAS NO BLUEPRINT

### **VERIFICAÇÃO 1: OnPlayerSelected_Event NÃO Deve Criar WBP_PlayerContextMenu**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerSelected_Event`

**Verificar:**
- [ ] **NÃO há** `Create Widget` para `WBP_PlayerContextMenu` neste evento
- [ ] **APENAS** cria `WBP_SelectedPlayerInfo`

**Se houver criação de `WBP_PlayerContextMenu` no `OnPlayerSelected_Event`, REMOVER!**

---

### **VERIFICAÇÃO 2: OnPlayerContextMenuRequested_Event NÃO Deve Criar WBP_SelectedPlayerInfo**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerContextMenuRequested_Event`

**Verificar:**
- [ ] **NÃO há** `Create Widget` para `WBP_SelectedPlayerInfo` neste evento
- [ ] **APENAS** cria `WBP_PlayerContextMenu`

**Se houver criação de `WBP_SelectedPlayerInfo` no `OnPlayerContextMenuRequested_Event`, REMOVER!**

---

### **VERIFICAÇÃO 3: Verificar Valores de bRightClick no Blueprint**

**Localização:** `BP_ThirdPersonCharacter` → **Event Graph**

**Verificar:**

1. **`IA_SelectPlayer` (Left Mouse Button)**
   - Deve chamar `HandlePlayerSelectionClick` com `bRightClick = true`
   - **Por quê?** Porque o usuário quer que clique esquerdo selecione o player

2. **`IA_ContextMenu` (Right Mouse Button)**
   - Deve chamar `HandlePlayerSelectionClick` com `bRightClick = false`
   - **Por quê?** Porque o usuário quer que clique direito abra o menu de contexto

**⚠️ ATENÇÃO:** Isso está **INVERTIDO** em relação ao comportamento padrão!

**Lógica esperada pelo C++:**
- `bRightClick = true` → Seleciona player (dispara `OnPlayerSelected`)
- `bRightClick = false` → Abre menu de contexto (dispara `OnPlayerContextMenuRequested`)

**Então no Blueprint:**
- Clique **ESQUERDO** (`IA_SelectPlayer`) → `bRightClick = true` ✅
- Clique **DIREITO** (`IA_ContextMenu`) → `bRightClick = false` ✅

---

### **VERIFICAÇÃO 4: OnPlayerDeselected_Event Limpa Ambos Widgets**

**Localização:** `WBP_PlayerHUD` → **Event Graph** → `OnPlayerDeselected_Event`

**Estrutura correta:**
```
[OnPlayerDeselected_Event]
    ↓
[Print String] "🔵 [OnPlayerDeselected] DISPARADO!" (DEBUG)
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

## 📋 CHECKLIST DE CORREÇÃO

- [ ] Compilar o C++ (corrige problema de resseleção)
- [ ] Verificar se `OnPlayerSelected_Event` **NÃO** cria `WBP_PlayerContextMenu`
- [ ] Verificar se `OnPlayerContextMenuRequested_Event` **NÃO** cria `WBP_SelectedPlayerInfo`
- [ ] Verificar se `IA_SelectPlayer` chama com `bRightClick = true`
- [ ] Verificar se `IA_ContextMenu` chama com `bRightClick = false`
- [ ] Verificar se `OnPlayerDeselected_Event` limpa ambos os widgets
- [ ] Testar primeiro clique esquerdo → deve abrir **APENAS** `WBP_PlayerContextMenu`
- [ ] Testar primeiro clique direito → deve abrir **APENAS** `WBP_SelectedPlayerInfo`
- [ ] Testar clique em espaço vazio → ambos devem fechar
- [ ] Testar clique novamente após desselecionar → deve funcionar

---

## 🧪 TESTE PASSO A PASSO

1. **Compilar** o C++
2. **Abrir** o jogo com 2 clients
3. **Clicar com botão ESQUERDO** em um remote actor (primeira vez)
   - ✅ Deve aparecer **APENAS** `WBP_PlayerContextMenu`
   - ❌ **NÃO** deve aparecer `WBP_SelectedPlayerInfo`
4. **Clicar em espaço vazio**
   - ✅ Ambos os widgets devem desaparecer
5. **Clicar com botão DIREITO** no mesmo remote actor
   - ✅ Deve aparecer **APENAS** `WBP_SelectedPlayerInfo`
   - ❌ **NÃO** deve aparecer `WBP_PlayerContextMenu`
6. **Clicar com botão ESQUERDO** no mesmo remote actor novamente
   - ✅ Deve aparecer **APENAS** `WBP_PlayerContextMenu` (sem criar novo `WBP_SelectedPlayerInfo`)
7. **Clicar em espaço vazio**
   - ✅ Ambos os widgets devem desaparecer
8. **Clicar com botão DIREITO** no remote actor novamente
   - ✅ Deve aparecer **APENAS** `WBP_SelectedPlayerInfo` (deve funcionar após desselecionar)

---

## 🔍 DEBUG: Adicionar Logs Temporários

**No `OnPlayerSelected_Event`, adicione no início:**
```
[Print String]
    In String: "🔵 [OnPlayerSelected_Event] DISPARADO! PlayerID: " + ToString(PlayerInfo.PlayerID)
```

**No `OnPlayerContextMenuRequested_Event`, adicione no início:**
```
[Print String]
    In String: "🔵 [OnPlayerContextMenuRequested] DISPARADO! PlayerID: " + ToString(PlayerInfo.PlayerID)
```

**No `OnPlayerDeselected_Event`, adicione no início:**
```
[Print String]
    In String: "🔵 [OnPlayerDeselected_Event] DISPARADO!"
```

**Verificar no Output Log:**
- Quando clica com botão esquerdo, deve aparecer **APENAS** `[OnPlayerContextMenuRequested]`
- Quando clica com botão direito, deve aparecer **APENAS** `[OnPlayerSelected]`
- Quando clica em espaço vazio, deve aparecer `[OnPlayerDeselected]`

---

**FIM DO GUIA**
