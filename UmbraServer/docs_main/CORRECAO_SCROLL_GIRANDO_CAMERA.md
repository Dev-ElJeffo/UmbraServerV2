# 🔧 **CORREÇÃO: Scroll do Mouse Girando Câmera ao Invés de Zoom**

## 🎯 **PROBLEMA:**

**O scroll do mouse está girando a câmera ao invés de mudar a distância (zoom).**

**Causa provável:**
- O `IA_MouseLook` está mapeado para `Mouse Wheel Axis` no `IMC_Default`
- Isso faz com que o scroll do mouse chame `Look()` ao invés de `Zoom()`

---

## ✅ **SOLUÇÃO:**

### **PASSO 1: Verificar Mapeamento no IMC_Default**

1. **Content Browser** → `Content/Input/`
2. **Duplo clique** em `IMC_Default`
3. **Verifique** se há um mapeamento:
   - **Input Action:** `IA_MouseLook` (ou `IA_MouseLookAction`)
   - **Key:** `Mouse Wheel Axis` ← **PROBLEMA!**

**Se encontrar este mapeamento:**
- **REMOVA** ou **ALTERE** a Key para `Mouse X` e `Mouse Y` (não `Mouse Wheel Axis`)

---

### **PASSO 2: Verificar Mapeamento do IA_MouseWheelZoom**

**No mesmo `IMC_Default`, verifique se existe:**

- **Input Action:** `IA_MouseWheelZoom`
- **Key:** `Mouse Wheel Axis` ← **DEVE SER ESTE!**
- **Scale:** `1.0` (ou `-1.0`)

**Se NÃO existir:**
- **Adicione** este mapeamento (veja guia anterior)

---

### **PASSO 3: Verificar Logs**

**Após compilar e executar, verifique os logs:**

**Se ver:**
```
Look chamado com Y: [valor] - Isso pode ser scroll do mouse mapeado incorretamente!
```
**Significa que:**
- O `IA_MouseLook` está capturando o scroll do mouse
- Você precisa remover ou alterar o mapeamento

**Se ver:**
```
Zoom chamado! ZoomDelta: [valor]
Zoom aplicado! Nova distância: [valor]
```
**Significa que:**
- O zoom está funcionando corretamente! ✅

---

## 📋 **CONFIGURAÇÃO CORRETA:**

### **IMC_Default deve ter:**

**Mapeamento 1: IA_MouseLook**
- **Input Action:** `IA_MouseLook`
- **Key:** `Mouse X` (para rotação horizontal)
- **Scale:** `1.0`

**Mapeamento 2: IA_MouseLook**
- **Input Action:** `IA_MouseLook`
- **Key:** `Mouse Y` (para rotação vertical)
- **Scale:** `-1.0` (ou `1.0` dependendo da preferência)

**Mapeamento 3: IA_MouseWheelZoom**
- **Input Action:** `IA_MouseWheelZoom`
- **Key:** `Mouse Wheel Axis` ← **ÚNICO que deve usar Mouse Wheel Axis!**
- **Scale:** `1.0` (ou `-1.0` se quiser inverter)

---

## ⚠️ **IMPORTANTE:**

**NUNCA mapeie `IA_MouseLook` para `Mouse Wheel Axis`!**

**O `Mouse Wheel Axis` deve ser usado APENAS para:**
- `IA_MouseWheelZoom` (zoom da câmera)

**O `IA_MouseLook` deve usar:**
- `Mouse X` (rotação horizontal)
- `Mouse Y` (rotação vertical)
- **E APENAS quando o botão direito estiver pressionado** (se configurado assim)

---

## 🔧 **TROUBLESHOOTING:**

### **PROBLEMA: Scroll ainda gira a câmera**

**Solução:**
1. Verifique se removeu o mapeamento `IA_MouseLook` → `Mouse Wheel Axis`
2. Verifique se o `IA_MouseWheelZoom` está mapeado para `Mouse Wheel Axis`
3. Verifique os logs para confirmar qual função está sendo chamada
4. Recompile o projeto

---

### **PROBLEMA: Zoom não funciona**

**Solução:**
1. Verifique se o `IA_MouseWheelZoom` existe
2. Verifique se está mapeado para `Mouse Wheel Axis` no `IMC_Default`
3. Verifique se o Input Action é do tipo `Axis1D (float)`
4. Verifique os logs para ver se `Zoom()` está sendo chamado

---

## ✅ **RESULTADO ESPERADO:**

**Após corrigir:**
- ✅ Scroll do mouse → Zoom in/out (muda distância da câmera)
- ✅ Botão direito + movimento do mouse → Gira câmera
- ✅ Câmera gira automaticamente junto com o personagem

**Com isso, tudo deve funcionar corretamente!**

