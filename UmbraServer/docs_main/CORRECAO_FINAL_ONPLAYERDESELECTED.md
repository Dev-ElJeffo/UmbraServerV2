# 🔴 CORREÇÃO FINAL: OnPlayerDeselected_Event

## 🎯 PROBLEMA IDENTIFICADO

No `OnPlayerDeselected_Event`:
- `K2Node_VariableSet_5` está setando `SelectedPlayerInfoWidgetREF` para o **valor atual** (via `K2Node_VariableGet_1`)
- **DEVERIA** setar para `null`

Isso causa o erro "None acessado" porque está tentando acessar uma referência que pode estar `None`.

## ✅ SOLUÇÃO

### **No `OnPlayerDeselected_Event`:**

**REMOVA a conexão de `K2Node_VariableGet_1` para `K2Node_VariableSet_5`:**

1. Localize `K2Node_VariableSet_5` (Set SelectedPlayerInfoWidgetREF)
2. **DESCONECTE** o pino `SelectedPlayerInfoWidgetREF` de `K2Node_VariableGet_1`
3. **DEIXE O PINO VAZIO** (ou conecte a um `Make Literal` com valor `null`)

**Estrutura CORRETA:**

```
[OnPlayerDeselected_Event]
    ↓
[Set SelectedPlayerInfoWidgetREF] = null  ← SEM conectar ao VariableGet!
    ↓
[Remove from Parent] → SelectedPlayerInfoWidgetREF (do Output_Get do Set anterior)
    ↓
[Set Visibility] → SelectedPlayerInfoWidgetREF, Collapsed
    ↓
[Is Valid?] → PlayerContextMenuWidgetREF
    ├─ TRUE:
    │   ↓
    │   [Set PlayerContextMenuWidgetREF] = (valor atual)
    │   ↓
    │   [Remove from Parent] → PlayerContextMenuWidgetREF
    │   ↓
    │   [Set Visibility] → PlayerContextMenuWidgetREF, Collapsed
    │
    └─ FALSE: (não fazer nada)
```

**⚠️ CRÍTICO:**
- `K2Node_VariableSet_5` deve setar `SelectedPlayerInfoWidgetREF` para `null` (não para o valor atual)
- O `Remove from Parent` deve usar o `Output_Get` do `K2Node_VariableSet_5` ANTES de setar para `null`, ou usar `K2Node_VariableGet_1` ANTES do `Set`

**MAS** a melhor solução é **remover o `Is Valid?` completamente** e usar a estratégia "sempre remover":

```
[OnPlayerDeselected_Event]
    ↓
[Remove from Parent] → SelectedPlayerInfoWidgetREF (do VariableGet)
    ↓
[Set SelectedPlayerInfoWidgetREF] = null
    ↓
[Remove from Parent] → PlayerContextMenuWidgetREF (do VariableGet)
    ↓
[Set PlayerContextMenuWidgetREF] = null
```

---

## 🔧 CORREÇÃO NO BLUEPRINT

1. Abra `WBP_PlayerHUD` → **Event Graph**
2. Localize `OnPlayerDeselected_Event`
3. Localize `K2Node_VariableSet_5` (Set SelectedPlayerInfoWidgetREF)
4. **DESCONECTE** o pino `SelectedPlayerInfoWidgetREF` de `K2Node_VariableGet_1`
5. **DEIXE O PINO VAZIO** (será `null` automaticamente)
6. **OU** remova o `Is Valid?` e use a estratégia "sempre remover"

---

**Essa é a correção para o erro "None acessado".**
