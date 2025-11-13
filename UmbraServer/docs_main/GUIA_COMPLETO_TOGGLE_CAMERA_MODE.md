# 🎯 **GUIA COMPLETO: Implementação do Toggle Camera Mode**

## 📋 **RESUMO**

Este guia explica como implementar o sistema de alternância entre dois modos de câmera:
- **Default Mode**: Sem rotação automática (padrão)
- **Combat Mode**: Com rotação automática suave seguindo as costas do personagem

---

## ✅ **PASSO 1: Criar o Input Action**

### **1.1. Criar o Asset**

1. No **Content Browser**, navegue até: `/Game/Input/Actions/`
2. Clique com botão direito → **Input** → **Input Action**
3. Nomeie como: `IA_ToggleCameraMode`
4. Pressione **Enter** para confirmar

### **1.2. Configurar o Input Action**

1. Selecione o `IA_ToggleCameraMode` criado
2. No **Details Panel**, configure:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: ✅ (marcado)
   - **Triggers**: 
     - ✅ **Pressed** (quando a tecla é pressionada)

---

## ✅ **PASSO 2: Configurar no Blueprint do Personagem**

### **2.1. Abrir o Blueprint**

1. Abra o Blueprint do personagem (ex: `BP_ThirdPersonCharacter`)
2. Vá para a aba **Viewport** ou **Event Graph**

### **2.2. Definir o ToggleCameraModeAction**

1. No **Details Panel** (lado esquerdo), encontre a seção **Input**
2. Localize a propriedade **Toggle Camera Mode Action**
3. Clique no dropdown e selecione `IA_ToggleCameraMode`
   - Se não aparecer, clique no ícone de **eyedropper** e selecione o asset no Content Browser

### **2.3. Verificar Variáveis**

No **Details Panel**, verifique se existem:
- ✅ **Combat Mode** (bool) - deve estar em `false` por padrão
- ✅ **Camera Follows Character** (bool) - deve estar em `true`

---

## ✅ **PASSO 3: Mapear a Tecla no Input Mapping Context**

### **3.1. Abrir o Input Mapping Context**

1. No **Content Browser**, encontre o `IMC_Default` (ou seu Input Mapping Context principal)
2. Abra o arquivo

### **3.2. Adicionar o Mapeamento**

1. No **Details Panel**, encontre a seção **Mappings**
2. Clique no botão **+ Add** para adicionar um novo mapeamento
3. Configure:
   - **Action**: Selecione `IA_ToggleCameraMode`
   - **Key**: Selecione a tecla desejada (ex: `V`, `C`, `Tab`, etc.)
   - **Modifiers**: Deixe vazio (ou adicione `Shift`, `Ctrl`, `Alt` se quiser combinação)

### **3.3. Salvar**

1. Pressione **Ctrl + S** para salvar
2. Ou clique em **File** → **Save**

---

## ✅ **PASSO 4: Testar a Implementação**

### **4.1. Compilar o C++**

1. Feche o Unreal Editor
2. Compile o projeto C++ (botão direito no `.uproject` → **Generate Visual Studio Project Files** → Abrir `.sln` → Build)
3. Abra o Unreal Editor novamente

### **4.2. Testar no Editor**

1. Pressione **Play** (ou **Alt + P**)
2. No jogo, pressione a tecla configurada (ex: `V`)
3. Verifique no **Output Log**:
   - `🎯 Modo de Câmera: COMBAT (rotação automática ativada)` - quando ativar
   - `📷 Modo de Câmera: DEFAULT (sem rotação automática)` - quando desativar

### **4.3. Verificar o Comportamento**

**Default Mode (padrão):**
- ✅ Câmera NÃO rotaciona automaticamente
- ✅ Apenas rotação manual pelo mouse funciona

**Combat Mode (após pressionar a tecla):**
- ✅ Câmera rotaciona suavemente para seguir as costas do personagem
- ✅ Rotação automática funciona apenas quando o personagem está se movendo
- ✅ Rotação automática desativa quando você move o mouse manualmente

---

## 🔧 **PASSO 5: Ajustes Opcionais**

### **5.1. Alterar a Tecla**

1. Abra o `IMC_Default`
2. Encontre o mapeamento de `IA_ToggleCameraMode`
3. Altere o **Key** para a tecla desejada
4. Salve

### **5.2. Alterar a Velocidade de Rotação**

No Blueprint do personagem, no **Details Panel**:
- **Camera Rotation Speed**: Velocidade base (padrão: `180.0`)
  - No código, está sendo multiplicado por `0.775` (redução de 22.5%)
  - Velocidade efetiva: `180.0 * 0.775 = 139.5` graus/segundo

### **5.3. Alterar o Modo Padrão**

No Blueprint do personagem, no **Details Panel**:
- **Combat Mode**: 
  - `false` = Default Mode (sem rotação automática) - **PADRÃO**
  - `true` = Combat Mode (com rotação automática)

---

## 📝 **RESUMO DAS CONFIGURAÇÕES**

### **Input Action:**
- Nome: `IA_ToggleCameraMode`
- Tipo: `Digital (bool)`
- Trigger: `Pressed`

### **Blueprint do Personagem:**
- `Toggle Camera Mode Action` → `IA_ToggleCameraMode`
- `Combat Mode` → `false` (padrão)
- `Camera Follows Character` → `true`

### **Input Mapping Context:**
- `IA_ToggleCameraMode` → Tecla escolhida (ex: `V`)

---

## ⚠️ **TROUBLESHOOTING**

### **Problema: A tecla não funciona**

**Soluções:**
1. Verifique se o `IA_ToggleCameraMode` está configurado no Blueprint do personagem
2. Verifique se o mapeamento está correto no `IMC_Default`
3. Verifique se o Input Mapping Context está sendo carregado (geralmente no `BeginPlay` do personagem)
4. Compile o C++ novamente se fez alterações

### **Problema: A rotação automática não funciona no Combat Mode**

**Soluções:**
1. Verifique se `bCameraFollowsCharacter` está em `true`
2. Verifique se `bCombatMode` está em `true` (após pressionar a tecla)
3. Verifique no Output Log se a mensagem de modo aparece
4. Certifique-se de que o personagem está se movendo (rotação só funciona quando `Speed > 10.0f`)

### **Problema: A rotação está muito rápida/lenta**

**Soluções:**
1. Ajuste `Camera Rotation Speed` no Blueprint do personagem
2. O código multiplica por `0.775` (redução de 22.5%)
3. Para alterar a redução, modifique o código C++ em `UmbraEternumUECharacter.cpp`:
   ```cpp
   float RotationSpeed = CameraRotationSpeed * 0.775f; // Altere 0.775f para o valor desejado
   ```

---

## ✅ **CHECKLIST FINAL**

- [ ] Input Action `IA_ToggleCameraMode` criado e configurado
- [ ] Blueprint do personagem tem `Toggle Camera Mode Action` definido
- [ ] Input Mapping Context tem o mapeamento da tecla
- [ ] Código C++ compilado sem erros
- [ ] Testado no Editor e funcionando
- [ ] Logs aparecem no Output Log ao alternar modos
- [ ] Rotação automática funciona no Combat Mode
- [ ] Rotação automática desativa no Default Mode

---

## 🎮 **COMO USAR**

1. **Iniciar o jogo**: Modo Default (sem rotação automática)
2. **Pressionar a tecla** (ex: `V`): Alterna para Combat Mode (com rotação automática)
3. **Pressionar novamente**: Volta para Default Mode
4. **Mover o mouse manualmente**: Desativa temporariamente a rotação automática (por 0.5 segundos)

---

**Pronto! O sistema de alternância de modos de câmera está implementado! 🎉**

