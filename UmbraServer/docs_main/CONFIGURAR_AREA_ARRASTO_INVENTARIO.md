# 🎯 CONFIGURAR ÁREA DE ARRASTO DO INVENTÁRIO

**PROBLEMA RESOLVIDO:** Agora o inventário só arrasta quando você clica na **área de arrasto** (ex: barra de título).

---

## ✅ **SOLUÇÃO IMPLEMENTADA:**

O C++ agora verifica se o clique foi na `DragAreaWidget`. Se não foi, **NÃO arrasta o inventário**.

---

## 🔧 **CONFIGURAÇÃO NO BLUEPRINT:**

### **1. NO WBP_Inventory (Designer):**

#### **A. Crie ou identifique a barra de título:**

- Se já tem `Border_TitleBar` → Use ela
- Se não tem → Crie um **Border** no topo do inventário
- Nome: `Border_TitleBar` (ou qualquer nome)

#### **B. Configure a variável:**

1. **My Blueprint** → **Variables**
2. Procure por **`Drag Area Widget`** (ou crie se não existir)
3. **Tipo:** `Widget` (UWidget)
4. **Is Variable:** ✅ Marcado
5. **Default Value:** Arraste `Border_TitleBar` do Designer para este campo

---

### **2. NO WBP_Inventory (Event Graph):**

#### **A. No Event Construct (ou onde inicializa):**

```
Event Construct
  ↓
Set Drag Area Widget
  ├─ Target: self
  └─ New Drag Area Widget: Border_TitleBar (GET)
```

---

## 📋 **RESULTADO:**

✅ **Clicar na barra de título** → Arrasta o inventário  
✅ **Clicar em slot/item** → Arrasta o item (não o inventário)  
✅ **Clicar em área vazia** → Não arrasta nada

---

## 🎮 **TESTE:**

1. Compile o C++
2. Configure `Drag Area Widget = Border_TitleBar` no WBP_Inventory
3. Teste:
   - Clicar na barra de título → Inventário arrasta ✅
   - Clicar em item → Item arrasta (não inventário) ✅

---

**CONFIGURE A VARIÁVEL `Drag Area Widget` NO BLUEPRINT!** 🚀

