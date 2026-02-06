# 🔴 CORREÇÃO: Menu Abre com Ambos Botões e Não Deseleciona

## 🎯 PROBLEMAS IDENTIFICADOS

### **Problema 1: PlayerContextMenu Abre com Ambos Botões**

**Causa:** Quando você clica com botão direito, o C++ faz:
1. `SelectPlayer(HitActor)` → Dispara `OnPlayerSelected`
2. `OpenContextMenu()` → Dispara `OnPlayerContextMenuRequested`

**Resultado:** Ambos os eventos são disparados, então se o `OnPlayerSelected_Event` também criar o `WBP_PlayerContextMenu`, ele aparecerá nos dois cliques.

---

### **Problema 2: SelectedPlayerInfo Não Deseleciona**

**Causa:** O `OnPlayerDeselected_Event` não está removendo o widget corretamente ou não está sendo chamado.

---

## ✅ SOLUÇÃO 1: Corrigir OnPlayerSelected_Event

### **CRÍTICO: OnPlayerSelected_Event NÃO DEVE criar WBP_PlayerContextMenu**

**Verificar no `OnPlayerSelected_Event`:**
- [ ] **NÃO há** `Create Widget` para `WBP_PlayerContextMenu`
- [ ] **NÃO há** lógica que abre o menu de contexto
- [ ] **APENAS** cria `WBP_SelectedPlayerInfo`

**Se houver criação de `WBP_PlayerContextMenu` no `OnPlayerSelected_Event`, REMOVER!**

---

## ✅ SOLUÇÃO 2: Corrigir OnPlayerDeselected_Event

### **Estrutura Correta:**

```
[OnPlayerDeselected_Event]
    (sem inputs)
    ↓
[Print String] "🔵 [OnPlayerDeselected] Fechando widgets" (DEBUG)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Remove from Parent]
    │       Target: SelectedPlayerInfoWidgetREF
    │   ↓
    │   [Set SelectedPlayerInfoWidgetREF] = null  ← CRÍTICO! LIMPAR REFERÊNCIA
    │
    └─ FALSE: (não fazer nada)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Remove from Parent]
    │       Target: PlayerContextMenuWidgetREF
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = null  ← CRÍTICO! LIMPAR REFERÊNCIA
    │
    └─ FALSE: (não fazer nada)
```

---

## ✅ SOLUÇÃO 3: Verificar se OnPlayerDeselected está sendo chamado

### **Adicionar Logs Temporários:**

**No `OnPlayerDeselected_Event`, adicione no início:**
```
[Print String]
    In String: "🔵 [OnPlayerDeselected_Event] DISPARADO!"
```

**Se este log NÃO aparecer quando você clica em espaço vazio:**
- O `OnPlayerDeselected` não está sendo disparado
- Verificar se o delegate está conectado corretamente no `Event Construct`

---

## ✅ SOLUÇÃO 4: Fechar Menu de Contexto ao Clicar em Espaço Vazio

### **Problema Adicional:**

O `WBP_PlayerContextMenu` também deve fechar quando você clica em espaço vazio. O `OnPlayerDeselected_Event` já deve fazer isso (ver Solução 2), mas verifique se está funcionando.

---

## 🔧 CORREÇÃO RÁPIDA: Separar Lógica de Clique Esquerdo e Direito

### **Opção A: Modificar C++ (Não Recomendado)**

Modificar o C++ para não chamar `SelectPlayer` no clique direito, mas isso quebraria a lógica.

### **Opção B: Verificar no Blueprint (Recomendado)**

**No `OnPlayerSelected_Event`, adicionar verificação:**

```
[OnPlayerSelected_Event]
    PlayerInfo (input)
    ↓
[Print String] "🔵 [OnPlayerSelected] PlayerID: " + ToString(PlayerInfo.PlayerID) (DEBUG)
    ↓
[Is Valid?] (PlayerContextMenuWidgetREF)
    ├─ TRUE: (menu de contexto já está aberto, não criar SelectedPlayerInfo)
    │   ↓
    │   [Print String] "⚠️ Menu de contexto já aberto, ignorando OnPlayerSelected"
    │   ↓
    │   (RETORNAR - não fazer nada)
    │
    └─ FALSE: (criar SelectedPlayerInfo normalmente)
        ↓
        [Create Widget] → WBP_SelectedPlayerInfo
        ...
```

**OU, mais simples:**

**No `OnPlayerContextMenuRequested_Event`, fechar SelectedPlayerInfo:**

```
[OnPlayerContextMenuRequested_Event]
    PlayerInfo (input)
    ScreenPosition (input)
    ↓
[Is Valid?] (SelectedPlayerInfoWidgetREF)
    ├─ TRUE:
    │   ↓
    │   [Set Visibility]
    │       Target: SelectedPlayerInfoWidgetREF
    │       Visibility: Collapsed  ← ESCONDER (não remover)
    │
    └─ FALSE: (não fazer nada)
    ↓
[Create Widget] → WBP_PlayerContextMenu
    ...
```

---

## 📋 CHECKLIST DE CORREÇÃO

- [ ] Verificar se `OnPlayerSelected_Event` **NÃO** cria `WBP_PlayerContextMenu`
- [ ] Verificar se `OnPlayerDeselected_Event` remove ambos os widgets
- [ ] Verificar se `OnPlayerDeselected_Event` limpa as referências (set para null)
- [ ] Adicionar logs de debug para verificar se eventos estão sendo disparados
- [ ] Testar clique esquerdo → deve abrir apenas `WBP_SelectedPlayerInfo`
- [ ] Testar clique direito → deve abrir apenas `WBP_PlayerContextMenu`
- [ ] Testar clique em espaço vazio → ambos devem fechar

---

## 🧪 TESTE PASSO A PASSO

1. **Compilar** o projeto
2. **Abrir** o jogo com 2 clients
3. **Clicar com botão esquerdo** em um remote actor
   - ✅ Deve aparecer **APENAS** `WBP_SelectedPlayerInfo`
   - ✅ Deve aparecer log: `🔵 [OnPlayerSelected] ...`
   - ❌ **NÃO** deve aparecer `WBP_PlayerContextMenu`
4. **Clicar com botão direito** no mesmo remote actor
   - ✅ Deve aparecer **APENAS** `WBP_PlayerContextMenu`
   - ✅ Deve aparecer log: `🔵 [OnPlayerContextMenuRequested] ...`
   - ❌ **NÃO** deve criar novo `WBP_SelectedPlayerInfo` (pode manter o existente ou esconder)
5. **Clicar em espaço vazio**
   - ✅ Ambos os widgets devem desaparecer
   - ✅ Deve aparecer log: `🔵 [OnPlayerDeselected] ...`
   - ✅ Referências devem ser limpas (null)

---

**FIM DO GUIA**
