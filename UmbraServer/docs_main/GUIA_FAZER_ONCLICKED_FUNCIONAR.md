# ✅ **SOLUÇÃO: Fazer OnClicked Funcionar**

## ❌ **PROBLEMA**

O `OnClicked` do `Collision_Box` não está disparando.

---

## ✅ **SOLUÇÃO: 3 Passos Simples**

### **PASSO 1: Habilitar Click Events no Collision_Box**

1. No `BP_Class_Placeholder`, selecione o **`Collision_Box`** (Box Component)
2. No painel **Details**, procure por **"Events"** ou **"Interaction"**
3. ✅ **Marque:** `Enable Click Events` (ou `Generate Hit Events`)
4. ✅ **Marque:** `Enable Mouse Over Events` (opcional, mas ajuda)

### **PASSO 2: Configurar Collision Corretamente**

No mesmo `Collision_Box`, no painel **Details**:

1. **Collision Enabled:** `Query Only` ou `Query and Physics`
2. **Object Type:** `WorldDynamic` ou `WorldStatic`
3. **Collision Responses:**
   - **Visibility:** `Block` ou `Overlap`
   - **Camera:** `Ignore` (opcional)

### **PASSO 3: Verificar Input Mode**

No `BP_Class_Placeholder`, no **Event BeginPlay**:

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Set Input Mode Game Only]
    • Target: (Player Controller)
```

**OU** se você tem UI aberto:

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Set Input Mode Game and UI]
    • Target: (Player Controller)
    • Lock Mouse to Viewport: false
    • Hide Cursor During Capture: false
```

---

## ⚠️ **VERIFICAÇÕES IMPORTANTES**

1. **O `Collision_Box` está visível?**
   - Se estiver invisível, o clique não funciona
   - Solução: Deixe visível ou aumente o tamanho

2. **Há outro componente na frente?**
   - Se houver um Skeletal Mesh ou Static Mesh na frente, ele bloqueia o clique
   - Solução: Aumente o tamanho do `Collision_Box` ou mova-o para frente do mesh

3. **O `Collision_Box` está como Root Component?**
   - Se não estiver, pode não funcionar
   - Solução: Certifique-se de que está configurado corretamente na hierarquia

---

## 🎯 **RESUMO**

**3 coisas para fazer:**
1. ✅ Marcar `Enable Click Events` no `Collision_Box`
2. ✅ Configurar Collision como `Query Only` ou `Query and Physics`
3. ✅ `Set Input Mode Game Only` no `Event BeginPlay`

**Pronto! O `OnClicked` deve funcionar.**

---

**Fim do Guia**

