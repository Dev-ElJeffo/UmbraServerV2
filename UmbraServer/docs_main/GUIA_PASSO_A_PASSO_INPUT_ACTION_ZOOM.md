# 📚 **GUIA PASSO A PASSO: Criar Input Action para Zoom**

## 🎯 **SITUAÇÃO ATUAL:**

**Seu projeto já tem Input Actions configurados:**
- ✅ `IA_Move` (em `Content/Input/Actions/`)
- ✅ `IA_MouseLook` (em `Content/Input/Actions/`)
- ✅ `IA_Look` (em `Content/Input/Actions/`)
- ✅ `IA_Jump` (em `Content/Input/Actions/`)
- ✅ `IMC_Default` (em `Content/Input/`)
- ✅ `IMC_MouseLook` (em `Content/Input/`)

**Vamos seguir o mesmo padrão!**

---

## 📋 **PASSO 1: CRIAR INPUT ACTION**

### **1.1: Abrir Pasta de Actions**

1. **Content Browser** (aba inferior do Unreal Editor)
2. **Navegue** até: **`Content/Input/Actions/`**
   - Você deve ver: `IA_Move`, `IA_MouseLook`, `IA_Look`, `IA_Jump`
3. **Clique** na pasta `Actions` para abrir

---

### **1.2: Criar Novo Input Action**

1. **Botão direito** dentro da pasta `Actions`
2. **Menu aparece** → Procure por **`Input`**
3. **Submenu** → Clique em **`Input Action`**
4. **Nome:** Digite `IA_MouseWheelZoom`
   - **Siga o padrão:** `IA_` + nome descritivo
5. **Pressione Enter** para confirmar

---

### **1.3: Configurar Tipo do Input Action**

**Após criar, o arquivo será aberto automaticamente (ou duplo clique nele):**

1. **No painel direito (Details Panel):**
2. **Procure por:** **"Value Type"** ou **"Type"**
3. **Clique** no dropdown
4. **Selecione:** **`Axis1D (float)`**
   - **NÃO selecione:** `Digital (bool)` ou `Axis2D (Vector2D)`
   - **Selecione:** `Axis1D (float)` ← **CRÍTICO!**

**Por quê `Axis1D (float)`?**
- Scroll do mouse retorna valores contínuos:
  - Scroll Up = valor positivo (ex: `1.0`)
  - Scroll Down = valor negativo (ex: `-1.0`)
  - `Axis1D` captura esses valores como float

5. **Salve** (Ctrl+S ou botão Save no topo)

---

### **1.4: Verificar Input Action Criado**

**Você deve ver:**
- **Ícone:** Um ícone de teclado/input (diferente de outros assets)
- **Nome:** `IA_MouseWheelZoom`
- **Na pasta:** `Content/Input/Actions/IA_MouseWheelZoom.uasset`

**Se não aparecer:**
- Verifique se salvou (Ctrl+S)
- Recarregue o Content Browser (F5)
- Verifique se está na pasta correta

---

## 📋 **PASSO 2: ADICIONAR AO INPUT MAPPING CONTEXT**

### **2.1: Abrir Input Mapping Context**

1. **Content Browser** → Pasta `Content/Input/`
2. **Encontre:** `IMC_Default` (ou `IMC_MouseLook` se preferir)
3. **Duplo clique** para abrir

---

### **2.2: Entender a Interface**

**Ao abrir `IMC_Default`, você verá:**
- **Painel central:** Lista de mapeamentos (Input Actions mapeados para teclas)
- **Cada linha** representa um mapeamento:
  - **Input Action:** Qual ação (ex: `IA_Move`)
  - **Key:** Qual tecla (ex: `W`, `Mouse X`)
  - **Scale:** Multiplicador (ex: `1.0` ou `-1.0`)

---

### **2.3: Adicionar Novo Mapeamento**

1. **No painel central, clique no botão `+`** (adicionar mapeamento)
   - Pode estar no topo da lista ou no final
2. **Uma nova linha será adicionada**
3. **Configure a nova linha:**

   **a) Input Action:**
   - **Clique** no campo (deve estar vazio ou `None`)
   - **Menu aparece** → Procure por `IA_MouseWheelZoom`
   - **Selecione:** `IA_MouseWheelZoom`

   **b) Key:**
   - **Clique** no campo (deve estar vazio ou `None`)
   - **Digite:** `Mouse` (para filtrar)
   - **Procure** na lista por:
     - **`Mouse Wheel Axis`** ← **RECOMENDADO** (captura scroll up e down)
     - OU **`Mouse Scroll Up`** e **`Mouse Scroll Down`** (separadamente)
   - **Selecione:** `Mouse Wheel Axis`

   **c) Scale:**
   - **Digite:** `1.0`
   - **Ou:** `-1.0` se quiser inverter (scroll up = zoom out)

4. **Salve** (Ctrl+S)

---

### **2.4: Verificar Mapeamento Adicionado**

**Você deve ver na lista:**
- **Input Action:** `IA_MouseWheelZoom`
- **Key:** `Mouse Wheel Axis`
- **Scale:** `1.0` (ou `-1.0`)

**Se não aparecer:**
- Verifique se salvou (Ctrl+S)
- Verifique se selecionou o Input Action correto
- Verifique se selecionou a Key correta

---

## 📋 **PASSO 3: VERIFICAR PLAYER CONTROLLER**

### **3.1: Encontrar Player Controller**

1. **Content Browser** → Procure por:
   - `BP_PlayerController`
   - `BP_UmbraEternumUEPlayerController`
   - Ou qualquer Player Controller Blueprint

**Se não encontrar:**
- Verifique no **World Settings** qual Player Controller está sendo usado
- Ou verifique no **Game Mode** qual Player Controller está configurado

---

### **3.2: Verificar se Input Mapping Context Está Configurado**

1. **Duplo clique** no Player Controller Blueprint para abrir
2. **Details Panel** (painel direito) → Procure por **"Input"** ou **"Input Mappings"**
3. **Verifique** se há uma variável como:
   - `Default Mapping Contexts` (Array)
   - `Input Mapping Contexts`
   - Ou similar

4. **Verifique** se `IMC_Default` está na lista:
   - Se **estiver** → Está configurado! ✅
   - Se **NÃO estiver** → Adicione (veja abaixo)

---

### **3.3: Adicionar Input Mapping Context (Se Não Estiver)**

**Se `IMC_Default` não estiver na lista:**

1. **Details Panel** → Encontre `Default Mapping Contexts` (ou similar)
2. **Clique no `+`** (adicionar elemento ao array)
3. **Clique** no campo (deve estar `None`)
4. **Selecione:** `IMC_Default`
5. **Compile** o Blueprint (botão verde no topo)
6. **Salve** (Ctrl+S)

---

## 📋 **PASSO 4: ATRIBUIR INPUT ACTION NO CHARACTER**

### **4.1: Abrir Character Blueprint**

1. **Content Browser** → Encontre `BP_ThirdPersonCharacter` (ou seu Character Blueprint)
2. **Duplo clique** para abrir

---

### **4.2: Atribuir Input Action**

1. **Details Panel** (painel direito)
2. **Procure** pela seção **"Input"** (role a lista se necessário)
3. **Encontre:** `Mouse Wheel Zoom Action`
   - Deve aparecer como uma variável editável
   - Pode estar vazia (`None`) ou já ter algum valor
4. **Clique** no campo
5. **Menu aparece** → Procure por `IA_MouseWheelZoom`
6. **Selecione:** `IA_MouseWheelZoom`
7. **Compile** o Blueprint (botão verde no topo)
8. **Salve** (Ctrl+S)

---

### **4.3: Verificar Atribuição**

**Após atribuir, você deve ver:**
- **`Mouse Wheel Zoom Action`:** `IA_MouseWheelZoom` (não mais `None`)

**Se não aparecer:**
- Verifique se compilou o Blueprint
- Verifique se o Input Action existe
- Verifique se está na pasta correta

---

## 📋 **PASSO 5: CONFIGURAR VARIÁVEIS DA CÂMERA (OPCIONAL)**

### **5.1: Ajustar Valores no Character Blueprint**

**No `BP_ThirdPersonCharacter`, no Details Panel:**

1. **Procure** pela seção **"Camera"** (role a lista se necessário)
2. **Encontre e configure:**
   - **Camera Follows Character:** `true` (padrão) - Ativa rotação automática
   - **Camera Rotation Speed:** `360.0` (padrão) - Velocidade de rotação (graus/segundo)
   - **Min Camera Distance:** `200.0` (padrão) - Distância mínima
   - **Max Camera Distance:** `1000.0` (padrão) - Distância máxima
   - **Zoom Speed:** `50.0` (padrão) - Velocidade do zoom

3. **Ajuste** conforme necessário:
   - Zoom muito rápido? → Diminua `Zoom Speed` (ex: `25.0`)
   - Rotação muito rápida? → Diminua `Camera Rotation Speed` (ex: `180.0`)

4. **Compile** o Blueprint
5. **Salve** (Ctrl+S)

---

## 🧪 **PASSO 6: TESTAR**

### **6.1: Compilar Projeto**

1. **Feche** o Unreal Editor (se estiver aberto)
2. **Abra** o Visual Studio com `UmbraEternumUE.sln`
3. **Compile** (Build → Build Solution ou F7)
4. **Aguarde** a compilação terminar

---

### **6.2: Executar e Testar**

1. **Abra** o Unreal Editor
2. **Execute** o jogo (Play)
3. **Teste:**
   - **Scroll do mouse para cima** → Câmera deve se aproximar (zoom in)
   - **Scroll do mouse para baixo** → Câmera deve se afastar (zoom out)
   - **Rotacionar personagem** → Câmera deve seguir automaticamente

---

## 🔧 **TROUBLESHOOTING:**

### **PROBLEMA 1: Scroll Não Funciona**

**Verifique:**
1. ✅ Input Action `IA_MouseWheelZoom` existe?
2. ✅ Input Action é do tipo `Axis1D (float)`?
3. ✅ Input Mapping Context tem mapeamento para `Mouse Wheel Axis`?
4. ✅ Player Controller adiciona `IMC_Default`?
5. ✅ Character Blueprint tem `Mouse Wheel Zoom Action` atribuído?

**Solução:**
- Adicione logs na função `Zoom()` para verificar se está sendo chamada
- Verifique se o Input Mapping Context está ativo

---

### **PROBLEMA 2: Zoom Invertido**

**Solução:**
- No Input Mapping Context, mude o **Scale** de `1.0` para `-1.0`

---

### **PROBLEMA 3: Zoom Muito Rápido/Lento**

**Solução:**
- No Character Blueprint, ajuste `Zoom Speed`:
  - Muito rápido → Diminua (ex: `25.0`)
  - Muito lento → Aumente (ex: `100.0`)

---

### **PROBLEMA 4: Rotação Automática Não Funciona**

**Verifique:**
1. ✅ `bCameraFollowsCharacter` está `true`?
2. ✅ Character Blueprint tem `PrimaryActorTick` habilitado?

**Solução:**
- No Character Blueprint, verifique se `PrimaryActorTick` está habilitado
- Ajuste `CameraRotationSpeed` se necessário

---

## 📝 **RESUMO VISUAL DO FLUXO:**

```
1. Content Browser → Input/Actions/
   ↓
2. Botão direito → Input → Input Action
   ↓
3. Nome: IA_MouseWheelZoom
   ↓
4. Value Type: Axis1D (float)
   ↓
5. Salvar (Ctrl+S)
   ↓
6. Abrir IMC_Default
   ↓
7. Adicionar mapeamento:
   ├─ Input Action: IA_MouseWheelZoom
   ├─ Key: Mouse Wheel Axis
   └─ Scale: 1.0
   ↓
8. Salvar (Ctrl+S)
   ↓
9. Abrir BP_ThirdPersonCharacter
   ↓
10. Details Panel → Input → Mouse Wheel Zoom Action
    ↓
11. Atribuir: IA_MouseWheelZoom
    ↓
12. Compilar Blueprint
    ↓
13. Salvar (Ctrl+S)
    ↓
14. Compilar projeto C++
    ↓
15. Testar no jogo
```

---

## ✅ **RESULTADO ESPERADO:**

**Após seguir todos os passos:**
- ✅ Scroll do mouse aumenta/diminui zoom da câmera
- ✅ Câmera gira automaticamente junto com o personagem
- ✅ Sistema Enhanced Input configurado corretamente
- ✅ Seguindo o mesmo padrão dos outros Input Actions do projeto

**Com isso, tudo deve funcionar perfeitamente!**

