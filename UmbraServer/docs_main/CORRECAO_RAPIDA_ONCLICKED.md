# ⚡ **CORREÇÃO RÁPIDA: OnClicked Não Funciona**

## 🎯 **5 VERIFICAÇÕES OBRIGATÓRIAS:**

### **1. Collision_Box - Generate Hit Events** ⚠️ **MAIS PROVÁVEL**

**No `BP_Class_Placeholder`:**

1. Selecione o componente **`Collision_Box`** (Box Component)
2. No painel **Details**, procure por:
   - **"Events"** ou **"Interaction"** ou **"Collision"**
3. ✅ **MARQUE:** `Generate Hit Events` ou `Enable Click Events`
4. ✅ **MARQUE:** `Generate Overlap Events` (opcional)

**SE NÃO ENCONTRAR:**
- Procure por **"Collision Responses"**
- Em **"Collision Responses"**, procure por **"Events"**
- Ou procure por **"Simulate Generate Hit Events"**

---

### **2. Input Mode - Game And UI** ⚠️ **SEGUNDO MAIS PROVÁVEL**

**No `Lvl_Character_Creation` (Level Blueprint):**

1. Abra o **Event Graph**
2. No `Event BeginPlay`, verifique se tem:

```
[Event BeginPlay]
    ↓
[Get Player Controller]
    • Player Index: 0
    ↓
[Set Input Mode Game And UI] ← DEVE SER ESTE!
    • Target: (Player Controller)
    • Lock Mouse to Viewport: Do Not Lock
    ↓
[Set Show Mouse Cursor]
    • Target: (Player Controller)
    • Show Mouse Cursor: true
```

**SE ESTIVER COMO `Game Only`:**
- ❌ **REMOVA** `Set Input Mode Game Only`
- ✅ **ADICIONE** `Set Input Mode Game And UI`
- ✅ **ADICIONE** `Set Show Mouse Cursor: true`

---

### **3. Widget Não Está Bloqueando** ⚠️ **TERCEIRO MAIS PROVÁVEL**

**No `WBP_CreateCharacter`:**

1. Abra o widget
2. Se houver um **Canvas Panel** ou **Panel** cobrindo a tela toda:
   - Selecione esse painel
   - No **Details**, procure por:
     - **"Is Hit Testable"** ou **"Hit Test Invisible"**
   - ✅ **DESMARQUE** `Is Hit Testable` (ou marque `Hit Test Invisible: true`)

**Isso permite que cliques passem através do widget para os atores 3D.**

---

### **4. Mesh Não Está Bloqueando**

**No `BP_Class_Placeholder`:**

1. Se houver um **Skeletal Mesh** ou **Static Mesh** (o personagem):
   - Selecione o mesh
   - No **Details**, procure por **"Collision"**:
     - ✅ **Collision Enabled:** `No Collision` ou `Query Only`
     - ✅ **Collision Responses → Visibility:** `Ignore`

**OU:**

1. Selecione o `Collision_Box`
2. Aumente o **Scale** (ex: 1.5, 1.5, 1.5) para ficar maior que o mesh

---

### **5. Nenhum Outro Lugar Está Mudando Input Mode**

**Procure e REMOVA:**

1. **WBP_CreateCharacter:**
   - ❌ Qualquer `Set Input Mode Game Only`
   - ❌ Qualquer `Set Show Mouse Cursor: false`

2. **BP_CharacterCreationManager:**
   - ❌ Qualquer `Set Input Mode Game Only`
   - ❌ Qualquer `Set Show Mouse Cursor: false`

3. **BP_ThirdPersonGameMode:**
   - ❌ Qualquer `Set Input Mode Game Only`
   - ❌ Qualquer `Set Show Mouse Cursor: false`

**Configure Input Mode APENAS NO LEVEL BLUEPRINT no `Event BeginPlay`.**

---

## ✅ **ORDEM DE PRIORIDADE:**

1. **PRIMEIRO:** Verificar `Generate Hit Events` no `Collision_Box` ⚠️ **MAIS PROVÁVEL**
2. **SEGUNDO:** Verificar Input Mode no Level Blueprint
3. **TERCEIRO:** Verificar se Widget está bloqueando
4. **QUARTO:** Verificar se Mesh está bloqueando
5. **QUINTO:** Remover outros `Set Input Mode`

---

## 🧪 **TESTE APÓS CADA CORREÇÃO:**

1. Compile o Blueprint
2. Execute o jogo
3. Clique no personagem
4. Verifique o **Output Log**:
   - ✅ Deve aparecer "CLIQUE DETECTADO!"
   - ✅ Cursor deve continuar visível

**Se aparecer o print, o problema está resolvido!**

---

## 🐛 **SE AINDA NÃO FUNCIONAR:**

Use a função C++ `GetClickedActor` (veja `GUIA_USAR_GETCLICKEDACTOR.md`).

---

**Fim do Guia**

