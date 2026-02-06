# 🔴 CORREÇÃO: Inversão Botões Esquerdo/Direito

## 🎯 PROBLEMA

Os botões esquerdo e direito estão invertidos. O comportamento atual não corresponde ao desejado.

**Comportamento desejado:**
- **Clique ESQUERDO** = Selecionar player (abrir `WBP_SelectedPlayerInfo`)
- **Clique DIREITO** = Abrir menu de contexto (abrir `WBP_PlayerContextMenu`) **E** selecionar player (abrir `WBP_SelectedPlayerInfo` também)

**Comportamento atual (invertido):**
- **Clique DIREITO** = Selecionar player
- **Clique ESQUERDO** = Menu de contexto

---

## ✅ SOLUÇÃO: Trocar Valores de bRightClick

### **Localização:** `BP_ThirdPersonCharacter` → **Event Graph**

---

## 🔧 PASSO 1: Encontrar os Input Actions

**No `BP_ThirdPersonCharacter`, encontre:**

1. **`IA_SelectPlayer`** (geralmente mapeado para **Left Mouse Button**)
2. **`IA_ContextMenu`** (geralmente mapeado para **Right Mouse Button**)

---

## 🔧 PASSO 2: Verificar Valores Atuais

**Para cada Input Action, verifique qual valor de `bRightClick` está sendo passado para `HandlePlayerSelectionClick`:**

### **Lógica do C++:**
- `bRightClick = true` → Seleciona player (dispara `OnPlayerSelected`)
- `bRightClick = false` → Abre menu de contexto (dispara `OnPlayerContextMenuRequested`)

### **Comportamento desejado:**
- **Clique ESQUERDO** → Seleciona player → `bRightClick = true` ✅
- **Clique DIREITO** → Abre menu de contexto → `bRightClick = false` ✅
  - **NOTA:** O clique direito também deve selecionar o player, então o C++ já faz isso automaticamente (chama `SelectPlayer` antes de `OpenContextMenu`)

---

## 🔧 PASSO 3: Corrigir Valores

### **Se `IA_SelectPlayer` (Left Mouse) está chamando com `bRightClick = false`:**

**TROCAR para `bRightClick = true`**

**ANTES:**
```
[IA_SelectPlayer (Left Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: false  ← ERRADO!
```

**DEPOIS:**
```
[IA_SelectPlayer (Left Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: true  ← CORRETO! (seleciona player)
```

---

### **Se `IA_ContextMenu` (Right Mouse) está chamando com `bRightClick = true`:**

**TROCAR para `bRightClick = false`**

**ANTES:**
```
[IA_ContextMenu (Right Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: true  ← ERRADO!
```

**DEPOIS:**
```
[IA_ContextMenu (Right Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: false  ← CORRETO! (abre menu de contexto)
```

---

## 📋 RESUMO DA CORREÇÃO

**No `BP_ThirdPersonCharacter`:**

1. **`IA_SelectPlayer` (Left Mouse Button)**
   - Deve chamar `HandlePlayerSelectionClick` com `bRightClick = true`
   - **Resultado:** Seleciona player (`WBP_SelectedPlayerInfo`)

2. **`IA_ContextMenu` (Right Mouse Button)**
   - Deve chamar `HandlePlayerSelectionClick` com `bRightClick = false`
   - **Resultado:** Abre menu de contexto (`WBP_PlayerContextMenu`) **E** seleciona player (`WBP_SelectedPlayerInfo`) - o C++ faz ambos automaticamente

---

## 🧪 TESTE APÓS CORREÇÃO

1. **Clicar com botão ESQUERDO** em um remote actor
   - ✅ Deve aparecer `WBP_SelectedPlayerInfo`
   - ❌ **NÃO** deve aparecer `WBP_PlayerContextMenu`

2. **Clicar com botão DIREITO** em um remote actor
   - ✅ Deve aparecer `WBP_PlayerContextMenu`
   - ✅ Deve aparecer `WBP_SelectedPlayerInfo` também (o C++ seleciona o player antes de abrir o menu)

---

## ⚠️ NOTA IMPORTANTE

**✅ CORREÇÃO IMPLEMENTADA NO C++:**

O C++ foi modificado para que quando `bRightClick = false` (clique direito para menu), também dispare `OnPlayerSelected`:

```cpp
// Selecionar o player (COM broadcast para criar WBP_SelectedPlayerInfo) e abrir menu
// O usuário quer que clique direito abra AMBOS os widgets
if (SelectPlayer(HitActor, true))  // Mudado para true para disparar OnPlayerSelected também
{
    OpenContextMenu();
    return true;
}
```

**Resultado:**
- Clique direito (`bRightClick = false`) agora:
  1. Dispara `OnPlayerSelected` → Cria `WBP_SelectedPlayerInfo`
  2. Dispara `OnPlayerContextMenuRequested` → Cria `WBP_PlayerContextMenu`

**Ambos os widgets serão criados automaticamente!**

---

**FIM DO GUIA**
