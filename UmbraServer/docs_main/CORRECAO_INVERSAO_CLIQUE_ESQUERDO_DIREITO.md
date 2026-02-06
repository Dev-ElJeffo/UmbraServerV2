# 🔴 CORREÇÃO: Inversão Clique Esquerdo/Direito

## 🎯 PROBLEMA IDENTIFICADO

**O usuário quer:**
- **Clique DIREITO** = Selecionar player (abrir `WBP_SelectedPlayerInfo`)
- **Clique ESQUERDO** = Menu de contexto (abrir `WBP_PlayerContextMenu`)

**Mas atualmente no Blueprint:**
- **Clique ESQUERDO** (`IA_SelectPlayer`) → Chama `HandlePlayerSelectionClick(bRightClick = false)` → Seleciona player
- **Clique DIREITO** (`IA_ContextMenu`) → Chama `HandlePlayerSelectionClick(bRightClick = true)` → Abre menu

**Resultado:** Está invertido!

---

## ✅ SOLUÇÃO: Trocar Valores de bRightClick no Blueprint

### **Modificar no BP_ThirdPersonCharacter**

**Localização:** `BP_ThirdPersonCharacter` → **Event Graph**

**Encontre os nós:**

1. **`IA_SelectPlayer` (Left Mouse Button)**
   - Conectado a `HandlePlayerSelectionClick` com `bRightClick = false`
   - **TROCAR para:** `bRightClick = true` ← TROCAR!

2. **`IA_ContextMenu` (Right Mouse Button)**
   - Conectado a `HandlePlayerSelectionClick` com `bRightClick = true`
   - **TROCAR para:** `bRightClick = false` ← TROCAR!

---

## 🔧 IMPLEMENTAÇÃO NO BLUEPRINT

### **PASSO 1: Modificar IA_SelectPlayer**

**No `BP_ThirdPersonCharacter` → Event Graph:**

1. **Encontre o nó `HandlePlayerSelectionClick` conectado ao `IA_SelectPlayer`**
2. **No pin `bRightClick`, mude de `false` para `true`**

**ANTES:**
```
[IA_SelectPlayer (Left Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: false  ← TROCAR!
```

**DEPOIS:**
```
[IA_SelectPlayer (Left Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: true  ← TROCAR!
```

---

### **PASSO 2: Modificar IA_ContextMenu**

**No `BP_ThirdPersonCharacter` → Event Graph:**

1. **Encontre o nó `HandlePlayerSelectionClick` conectado ao `IA_ContextMenu`**
2. **No pin `bRightClick`, mude de `true` para `false`**

**ANTES:**
```
[IA_ContextMenu (Right Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: true  ← TROCAR!
```

**DEPOIS:**
```
[IA_ContextMenu (Right Mouse)]
    ↓
[HandlePlayerSelectionClick]
    bRightClick: false  ← TROCAR!
```

---

## 📋 VERIFICAÇÃO FINAL

Após a correção:
- **Clique ESQUERDO** (`IA_SelectPlayer`) → `HandlePlayerSelectionClick(true)` → `TrySelectPlayerUnderCursor(true)` → `SelectPlayer(true)` → `OnPlayerSelected` → `WBP_SelectedPlayerInfo` ❌ **ERRADO!**

**Espera, isso ainda está errado!**

**O correto seria:**
- **Clique DIREITO** (`IA_ContextMenu`) → `HandlePlayerSelectionClick(false)` → `TrySelectPlayerUnderCursor(false)` → `SelectPlayer(false)` + `OpenContextMenu` → `OnPlayerContextMenuRequested` → `WBP_PlayerContextMenu` ❌ **ERRADO!**

**Preciso verificar melhor a lógica...**

---

## 🔄 LÓGICA CORRETA

**O usuário quer:**
- **Clique DIREITO** = Selecionar (abrir `WBP_SelectedPlayerInfo`)
- **Clique ESQUERDO** = Menu (abrir `WBP_PlayerContextMenu`)

**Então:**
- `IA_SelectPlayer` deve estar mapeado para **Right Mouse Button** (não Left)
- `IA_ContextMenu` deve estar mapeado para **Left Mouse Button** (não Right)

**OU, se os Input Actions estão corretos:**
- `IA_SelectPlayer` (Left Mouse) → deve chamar com `bRightClick = false` → mas isso seleciona, então está errado
- `IA_ContextMenu` (Right Mouse) → deve chamar com `bRightClick = true` → mas isso abre menu, então está errado

**A solução é TROCAR os valores de `bRightClick` no Blueprint!**

---

**FIM DO GUIA**
