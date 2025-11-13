# 🎥 **GUIA: Zoom da Câmera e Rotação Automática**

## 🎯 **OBJETIVOS:**

1. ✅ **Zoom com scroll do mouse** (aumentar/diminuir distância da câmera)
2. ✅ **Câmera girar junto com o personagem** automaticamente (não só ao segurar botão direito)

---

## ✅ **MODIFICAÇÕES APLICADAS NO C++:**

### **1. Adicionadas Variáveis no Header (`UmbraEternumUECharacter.h`):**

- ✅ `MouseWheelZoomAction` - Input Action para scroll do mouse
- ✅ `bCameraFollowsCharacter` - Flag para ativar/desativar rotação automática
- ✅ `CameraRotationSpeed` - Velocidade de rotação da câmera (graus por segundo)
- ✅ `MinCameraDistance` - Distância mínima da câmera
- ✅ `MaxCameraDistance` - Distância máxima da câmera
- ✅ `ZoomSpeed` - Velocidade do zoom (unidades por scroll)

### **2. Adicionadas Funções:**

- ✅ `Zoom()` - Função chamada quando scroll do mouse é usado
- ✅ `Tick()` - Função chamada a cada frame para fazer câmera seguir rotação do personagem

---

## 📋 **PRÓXIMOS PASSOS NO BLUEPRINT:**

### **📚 GUIA COMPLETO:**

**Para um guia passo a passo detalhado, consulte:**
- **`GUIA_PASSO_A_PASSO_INPUT_ACTION_ZOOM.md`** ← **GUIA MAIS DETALHADO!**
- **`GUIA_COMPLETO_INPUT_ACTIONS_UE5.md`** ← Explicação completa do sistema
- **`ALTERNATIVA_ACTION_MAPPING_ZOOM.md`** ← Se preferir usar Action Mapping

---

### **RESUMO RÁPIDO:**

### **PASSO 1: Criar Input Action para Scroll do Mouse**

1. **Content Browser** → **`Content/Input/Actions/`**
2. **Botão direito** → **Input** → **Input Action**
3. **Nome:** `IA_MouseWheelZoom`
4. **Value Type:** `Axis1D (float)` ← **IMPORTANTE!**
5. **Salve** o Input Action

### **PASSO 2: Configurar Input Mapping Context**

1. **Abra** `IMC_Default` (em `Content/Input/`)
2. **Adicione** mapeamento:
   - **Input Action:** `IA_MouseWheelZoom`
   - **Key:** `Mouse Wheel Axis`
   - **Scale:** `1.0` (ou `-1.0` se quiser inverter)
3. **Salve**

### **PASSO 3: Atribuir Input Action no Character Blueprint**

1. **Abra** `BP_ThirdPersonCharacter`
2. **Details Panel** → Seção **"Input"**
3. **Encontre:** `Mouse Wheel Zoom Action`
4. **Atribua:** `IA_MouseWheelZoom`
5. **Compile** o Blueprint

### **PASSO 4: Configurar Variáveis da Câmera (Opcional)**

**No `BP_ThirdPersonCharacter`:**

1. **Details Panel** → Procure por **"Camera"**
2. **Configure:**
   - **Camera Follows Character:** `true` (padrão)
   - **Camera Rotation Speed:** `360.0` (graus por segundo)
   - **Min Camera Distance:** `200.0`
   - **Max Camera Distance:** `1000.0`
   - **Zoom Speed:** `50.0` (unidades por scroll)

---

## 🎯 **COMO FUNCIONA:**

### **Zoom com Scroll:**

**Quando você usa o scroll do mouse:**
1. `IA_MouseWheelZoom` é acionado
2. `Zoom()` é chamado com o valor do scroll (positivo = zoom in, negativo = zoom out)
3. `TargetArmLength` do `CameraBoom` é ajustado
4. Câmera se aproxima ou afasta do personagem

### **Rotação Automática:**

**A cada frame (`Tick`):**
1. Obtém a rotação atual do personagem
2. Obtém a rotação atual do Controller (câmera)
3. Calcula a diferença de Yaw
4. Rotaciona a câmera suavemente em direção à rotação do personagem
5. Velocidade controlada por `CameraRotationSpeed`

---

## 🧪 **TESTE:**

1. **Compile** o projeto (C++ foi modificado)
2. **Crie** o Input Action `IA_MouseWheelZoom`
3. **Configure** no Input Mapping Context
4. **Atribua** no Character Blueprint
5. **Execute** o jogo
6. **Teste:**
   - **Scroll do mouse** → Câmera deve zoom in/out
   - **Rotacionar personagem** → Câmera deve seguir automaticamente

---

## ⚙️ **AJUSTES:**

**Se a rotação automática estiver muito rápida:**
- Diminua `CameraRotationSpeed` (ex: `180.0` ou `90.0`)

**Se a rotação automática estiver muito lenta:**
- Aumente `CameraRotationSpeed` (ex: `540.0` ou `720.0`)

**Se o zoom estiver muito rápido:**
- Diminua `ZoomSpeed` (ex: `25.0` ou `10.0`)

**Se o zoom estiver muito lento:**
- Aumente `ZoomSpeed` (ex: `100.0` ou `150.0`)

**Para desativar rotação automática:**
- Configure `bCameraFollowsCharacter = false` no Blueprint

---

## ✅ **RESULTADO ESPERADO:**

**Após implementar:**
- ✅ Scroll do mouse aumenta/diminui zoom da câmera
- ✅ Câmera gira automaticamente junto com o personagem
- ✅ Rotação suave e controlável
- ✅ Zoom limitado entre min/max distance

**Com isso, a câmera deve funcionar como você deseja!**

