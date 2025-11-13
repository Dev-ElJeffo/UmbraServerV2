# 📚 **GUIA DETALHADO: Criar Input Action para Scroll do Mouse**

## 🎯 **OBJETIVO:**

**Criar um Input Action para capturar o scroll do mouse e usar no zoom da câmera.**

---

## 📋 **PASSO A PASSO COMPLETO:**

### **PASSO 1: Criar Input Action**

1. **Content Browser** → Navegue até a pasta **`Input`** (ou onde estão seus Input Actions)
2. **Botão direito** na pasta → **Input** → **Input Action**
3. **Nome:** `IA_MouseWheelZoom`
4. **Value Type:** Selecione **`Axis1D (float)`** ← **CRÍTICO!**
   - **NÃO use** `Digital (bool)` ou `Axis2D (Vector2D)`
   - **Use** `Axis1D (float)` para capturar valores contínuos do scroll
5. **Salve** (Ctrl+S)

---

### **PASSO 2: Configurar Input Mapping Context**

1. **Content Browser** → Encontre seu **Input Mapping Context** (ex: `IMC_Default` ou similar)
2. **Duplo clique** para abrir
3. **No painel de mapeamentos:**
   - **Clique no `+`** para adicionar um novo mapeamento
   - **Input Action:** Selecione `IA_MouseWheelZoom`
   - **Key:** Clique no campo e procure por **`Mouse Wheel Axis`** ou **`Mouse Scroll Up`** e **`Mouse Scroll Down`**
   
   **OPÇÃO A: Usar Mouse Wheel Axis (Recomendado)**
   - **Key:** `Mouse Wheel Axis`
   - **Scale:** `1.0` (ou `-1.0` se quiser inverter)
   - Isso captura scroll up e down em um único mapeamento
   
   **OPÇÃO B: Usar Mouse Scroll Up/Down Separadamente**
   - **Mapeamento 1:**
     - **Input Action:** `IA_MouseWheelZoom`
     - **Key:** `Mouse Scroll Up`
     - **Scale:** `1.0`
   - **Mapeamento 2:**
     - **Input Action:** `IA_MouseWheelZoom`
     - **Key:** `Mouse Scroll Down`
     - **Scale:** `-1.0`

4. **Salve** (Ctrl+S)

---

### **PASSO 3: Atribuir Input Action no Character Blueprint**

1. **Content Browser** → Encontre `BP_ThirdPersonCharacter` (ou seu Character Blueprint)
2. **Duplo clique** para abrir
3. **Details Panel** (painel direito) → Procure por **"Input"**
4. **Encontre:** `Mouse Wheel Zoom Action`
5. **Clique** no campo (deve estar vazio ou `None`)
6. **Selecione:** `IA_MouseWheelZoom` (o Input Action que você criou)
7. **Compile** o Blueprint (botão verde no topo)
8. **Salve** (Ctrl+S)

---

## 🎯 **VERIFICAÇÃO:**

**Após configurar, verifique:**

1. ✅ `IA_MouseWheelZoom` existe e é do tipo `Axis1D (float)`
2. ✅ `IMC_Default` (ou seu Input Mapping Context) tem mapeamento para `Mouse Wheel Axis`
3. ✅ `BP_ThirdPersonCharacter` tem `Mouse Wheel Zoom Action` atribuído

---

## 🧪 **TESTE:**

1. **Compile** o projeto (C++ foi modificado)
2. **Execute** o jogo
3. **Use o scroll do mouse:**
   - **Scroll Up** → Câmera deve se aproximar (zoom in)
   - **Scroll Down** → Câmera deve se afastar (zoom out)

**Se não funcionar:**
- Verifique se o Input Action está atribuído no Character Blueprint
- Verifique se o Input Mapping Context está sendo usado pelo Player Controller
- Verifique se o `Value Type` do Input Action é `Axis1D (float)`

---

## ⚙️ **AJUSTES:**

**Se o zoom estiver invertido:**
- No Input Mapping Context, mude o **Scale** de `1.0` para `-1.0` (ou vice-versa)

**Se o zoom estiver muito rápido:**
- No Character Blueprint, diminua `Zoom Speed` (ex: `25.0` ou `10.0`)

**Se o zoom estiver muito lento:**
- No Character Blueprint, aumente `Zoom Speed` (ex: `100.0` ou `150.0`)

---

## ✅ **RESULTADO ESPERADO:**

**Após configurar:**
- ✅ Scroll do mouse aumenta/diminui zoom da câmera
- ✅ Zoom limitado entre min/max distance
- ✅ Câmera gira automaticamente junto com o personagem

**Com isso, a câmera deve funcionar como você deseja!**

