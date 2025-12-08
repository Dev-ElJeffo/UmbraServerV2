# 🔧 **CORREÇÃO URGENTE: PreviewClass é nullptr**

## ❌ **PROBLEMA**

O log mostra:
```
[UmbraCharacterSelectionManager] ⚠️ PreviewClass é nullptr - previews não serão spawnados
```

**Isso significa que você NÃO está passando o `PreviewClass` no `Initialize`!**

---

## ✅ **SOLUÇÃO IMEDIATA**

### **No Level Blueprint, quando chamar `Initialize`:**

**Você DEVE preencher TODOS os parâmetros:**

```
[Call Function: Initialize]
    • Target: Manager
    • Player Controller: (Player Controller)
    • Widget: (Widget)
    • Preview Class: BP_CharacterPreview  ← ESTE É O PROBLEMA!
    • Spacing: 300.0
```

---

## 📝 **COMO CORRIGIR NO BLUEPRINT**

1. **Localize o nó `Initialize` no Level Blueprint**
2. **Clique no nó `Initialize`**
3. **Na aba "Details" (painel direito), procure por "Preview Class"**
4. **No campo "Preview Class", selecione `BP_CharacterPreview`**
5. **No campo "Spacing", digite: `300.0`**
6. **Compile o Blueprint**

---

## ⚠️ **IMPORTANTE**

Se você não ver o campo "Preview Class" no nó `Initialize`:
- **Compile o C++ primeiro!**
- **Feche e reabra o Unreal Editor**
- O parâmetro só aparece depois que o C++ foi compilado

---

## ✅ **VERIFICAÇÃO**

Após corrigir, os logs devem mostrar:
```
[UmbraCharacterSelectionManager] ✅ Initialize chamado - PC e Widget setados
[UmbraCharacterSelectionManager] 🔍 Spawnando 2 previews...
[UmbraCharacterSelectionManager] ✅ Preview spawnado: ElJeffo (ID: 1) em ...
[UmbraCharacterSelectionManager] ✅ Preview spawnado: Monkus (ID: 26) em ...
[UmbraCharacterSelectionManager] ✅✅✅ Total de previews spawnados: 2
[UmbraCharacterSelectionManager] ✅✅✅ Initialize completo - 2 previews spawnados
```

**Se você NÃO ver essas linhas, o `PreviewClass` ainda está nullptr!**

---

**ESSA É A CORREÇÃO. SÓ ISSO.**

