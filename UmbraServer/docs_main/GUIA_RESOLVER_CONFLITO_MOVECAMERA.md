# 🔧 **GUIA: Resolver Conflito MoveCameraToSelection**

## ❌ **PROBLEMA**

Após reparentar o Blueprint para `Umbra Character Creation Manager`, você tem:
- ✅ Função C++ `MoveCameraToSelection` (com 5 parâmetros)
- ❌ Função Blueprint `MoveCameraToSelection` (com 1 parâmetro) ← **CONFLITO!**

**Erro:** "The function name in node Q MoveCameraToSelection is already used"

---

## ✅ **SOLUÇÃO: Deletar Função Blueprint**

### **PASSO 1: Deletar a Função Blueprint**

1. **No painel "My Blueprint"** (lado esquerdo)
2. **Encontre a seção "FUNÇÕES"**
3. **Procure por:** `MoveCameraToSelection`
4. **Clique com botão direito** na função
5. **Selecione:** "Delete" ou "Delete Function"

### **PASSO 2: Compilar**

1. Clique no botão **"Compile"** (canto superior direito)
2. Os erros devem desaparecer!

---

## ✅ **AGORA USE A FUNÇÃO C++**

A função C++ já está disponível! No Event Graph:

1. **Arraste e solte** ou **clique direito** → **Call Function**
2. **Procure por:** `Move Camera To Selection`
3. **Você verá os parâmetros:**
   - `Target Placeholder` (Actor Reference)
   - `Camera Actor` (Actor Reference)
   - `Duration` (Float) - opcional, padrão: 1.0
   - `Offset X` (Float) - opcional, padrão: 200.0
   - `Offset Z` (Float) - opcional, padrão: 100.0

---

## 📝 **EXEMPLO: Atualizar OnClassSelected_Handler**

**ANTES (função Blueprint antiga):**
```
[MoveCameraToSelection]
    • TargetPlaceholder: ...
```

**DEPOIS (função C++):**
```
[Call Function: Move Camera To Selection]
    • Target: (Self)
    • Target Placeholder: Found Placeholder
    • Camera Actor: CameraActor
    • Duration: 1.0
    • Offset X: 200.0
    • Offset Z: 100.0
```

---

## ✅ **RESUMO**

1. **Deletar** a função `MoveCameraToSelection` do Blueprint
2. **Compilar**
3. **Usar** a função C++ `Move Camera To Selection` diretamente

**Pronto! Sem conflitos!**

---

**Fim do Guia**

