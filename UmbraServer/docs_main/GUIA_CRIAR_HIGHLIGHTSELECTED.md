# 🎯 **GUIA: Criar Função HighlightSelected no BP_Class_Placeholder**

## 📋 **OBJETIVO**

Criar a função `HighlightSelected` que fornece feedback visual quando um placeholder é selecionado (mudança de cor, brilho, outline, etc.).

---

## ✅ **PASSO A PASSO COMPLETO**

### **PASSO 1: Criar a Função**

1. **Abra o Blueprint `BP_Class_Placeholder`**
2. No painel **My Blueprint** → **Functions**, clique em **+ Function**
3. Nomeie como: `HighlightSelected`
4. A função será criada e aberta automaticamente

### **PASSO 2: Configurar a Função**

A função não precisa de inputs ou outputs, apenas executa a lógica visual.

**No painel Details do nó Function Entry:**
- **Inputs:** Nenhum necessário
- **Outputs:** Nenhum necessário

---

## 🎨 **OPÇÕES DE IMPLEMENTAÇÃO**

### **OPÇÃO 1: Mudar Material/Cor (Recomendado - Mais Simples)**

Esta opção muda a cor do material do mesh quando selecionado.

#### **Passo 1: Criar Material Instance Dinâmico (Opcional)**

Se você quiser mudar a cor dinamicamente:

1. No **Content Browser**, crie um **Material Instance Dynamic**
2. Ou use um **Material Parameter Collection**

#### **Passo 2: Implementar HighlightSelected**

```
[HighlightSelected]
    ↓
[Get] Mesh_Character
    ↓
[Create Dynamic Material Instance]
    • Parent: (material base do mesh)
    • Target: Mesh_Character
    ↓
[Set Vector Parameter Value]
    • Target: (material instance criado)
    • Parameter Name: "HighlightColor" (ou "BaseColor")
    • Value: (cor de destaque - ex: amarelo, azul brilhante)
        • R: 1.0
        • G: 0.8
        • B: 0.0
        • A: 1.0
```

**Passo a Passo Detalhado:**

1. **Adicionar Get Mesh_Character:**
   - Arraste do pino **exec** do nó `HighlightSelected`
   - Digite: `Get Mesh_Character` ou arraste a variável do painel **My Blueprint**
   - Selecione: **Get Mesh_Character**

2. **Adicionar Create Dynamic Material Instance:**
   - Arraste do pino de saída de `Get Mesh_Character`
   - Digite: `Create Dynamic Material Instance`
   - Selecione: **Create Dynamic Material Instance**
   - Conecte:
     - **Parent:** Arraste `Get Material` do `Mesh_Character` (ou use um material específico)
     - **Target:** Conecte o pino de saída de `Get Mesh_Character`

3. **Adicionar Set Vector Parameter Value:**
   - Arraste do pino **exec** do `Create Dynamic Material Instance`
   - Digite: `Set Vector Parameter Value`
   - Selecione: **Set Vector Parameter Value**
   - Conecte:
     - **Target:** Conecte o **Return Value** (Material Instance Dynamic) do `Create Dynamic Material Instance`
     - **Parameter Name:** Digite `"HighlightColor"` ou `"BaseColor"` (depende do seu material)
     - **Value:** Crie um **Make Vector** com valores RGB:
       - **R:** 1.0 (vermelho)
       - **G:** 0.8 (verde)
       - **B:** 0.0 (azul)
       - **A:** 1.0 (alpha)

---

### **OPÇÃO 2: Adicionar Outline/Glow (Mais Visual)**

Esta opção adiciona um efeito de outline ou glow ao redor do personagem.

#### **Passo 1: Adicionar Post Process Component**

1. No **Viewport** do Blueprint `BP_Class_Placeholder`
2. No painel **Components**, clique em **Add Component**
3. Selecione: **Post Process Component**
4. Nomeie como: `PostProcess_Highlight`

#### **Passo 2: Implementar HighlightSelected**

```
[HighlightSelected]
    ↓
[Get] PostProcess_Highlight
    ↓
[Set Visibility]
    • Target: PostProcess_Highlight
    • New Visibility: Visible
    ↓
[Set Scalar Parameter Value]
    • Target: PostProcess_Highlight
    • Parameter Name: "OutlineThickness"
    • Value: 0.5 (ajuste conforme necessário)
    ↓
[Set Vector Parameter Value]
    • Target: PostProcess_Highlight
    • Parameter Name: "OutlineColor"
    • Value: (cor do outline - ex: amarelo brilhante)
```

---

### **OPÇÃO 3: Mudar Scale (Mais Simples - Sem Materiais)**

Esta opção aumenta ligeiramente o tamanho do personagem quando selecionado.

```
[HighlightSelected]
    ↓
[Get] Mesh_Character
    ↓
[Get Actor Scale]
    • Target: (Self)
    ↓
[Make Vector]
    • X: 1.1 (10% maior)
    • Y: 1.1
    • Z: 1.1
    ↓
[Set Actor Scale]
    • Target: (Self)
    • New Scale: (resultado do Make Vector)
```

**Passo a Passo:**

1. **Adicionar Get Actor Scale:**
   - Arraste do pino **exec** do `HighlightSelected`
   - Digite: `Get Actor Scale`
   - Selecione: **Get Actor Scale**
   - **Target:** Arraste `Self` ou deixe vazio

2. **Adicionar Make Vector:**
   - Arraste do pino **exec** do `Get Actor Scale`
   - Digite: `Make Vector`
   - Selecione: **Make Vector**
   - Configure:
     - **X:** 1.1
     - **Y:** 1.1
     - **Z:** 1.1

3. **Adicionar Set Actor Scale:**
   - Arraste do pino **exec** do `Make Vector`
   - Digite: `Set Actor Scale`
   - Selecione: **Set Actor Scale**
   - Conecte:
     - **Target:** Arraste `Self` ou deixe vazio
     - **New Scale:** Conecte o **Return Value** do `Make Vector`

---

### **OPÇÃO 4: Adicionar Point Light (Efeito de Brilho)**

Esta opção adiciona uma luz pontual ao redor do personagem selecionado.

#### **Passo 1: Adicionar Point Light Component**

1. No **Viewport** do Blueprint `BP_Class_Placeholder`
2. No painel **Components**, clique em **Add Component**
3. Selecione: **Point Light**
4. Nomeie como: `Light_Highlight`
5. No **Details**, configure:
   - **Intensity:** 1000 (ajuste conforme necessário)
   - **Light Color:** Amarelo ou azul brilhante
   - **Attenuation Radius:** 500
   - **Visibility:** Hidden (inicialmente oculto)

#### **Passo 2: Implementar HighlightSelected**

```
[HighlightSelected]
    ↓
[Get] Light_Highlight
    ↓
[Set Visibility]
    • Target: Light_Highlight
    • New Visibility: Visible
    ↓
[Set Intensity]
    • Target: Light_Highlight
    • New Intensity: 2000 (ajuste conforme necessário)
    ↓
[Set Light Color]
    • Target: Light_Highlight
    • New Light Color: (cor amarela/azul brilhante)
        • R: 1.0
        • G: 0.9
        • B: 0.3
```

---

### **OPÇÃO 5: Combinar Múltiplos Efeitos (Mais Impactante)**

Combine várias opções para um efeito mais visual:

```
[HighlightSelected]
    ↓
[Get] Mesh_Character
    ↓
[Create Dynamic Material Instance]
    • Parent: (material base)
    • Target: Mesh_Character
    ↓
[Set Vector Parameter Value]
    • Target: (material instance)
    • Parameter Name: "BaseColor"
    • Value: (cor de destaque)
    ↓
[Get Actor Scale]
    ↓
[Make Vector]
    • X: 1.05
    • Y: 1.05
    • Z: 1.05
    ↓
[Set Actor Scale]
    • New Scale: (resultado)
    ↓
[Get] Light_Highlight
    ↓
[Set Visibility]
    • Target: Light_Highlight
    • New Visibility: Visible
```

---

## 🔄 **OPÇÃO 6: Usar Timeline para Animação (Mais Suave)**

Esta opção anima o highlight com uma Timeline para um efeito mais suave.

#### **Passo 1: Criar Timeline**

1. No painel **My Blueprint** → **Timelines**
2. Clique em **+ Timeline**
3. Nomeie como: `HighlightTimeline`
4. Configure:
   - **Length:** 0.5 (segundos)
   - **Use Last Keyframe:** true
   - **Looping:** false

#### **Passo 2: Adicionar Float Track**

1. Na Timeline `HighlightTimeline`, adicione um **Float Track**
2. Configure a curva:
   - **Keyframe 0:** Value = 0.0
   - **Keyframe 1:** Value = 1.0
   - **Curva:** Ease In Out (suave)

#### **Passo 3: Implementar HighlightSelected**

```
[HighlightSelected]
    ↓
[Play] HighlightTimeline
    ↓
[Update] HighlightTimeline
    • Alpha: (output da Timeline)
    ↓
[Lerp] (Float)
    • A: 1.0 (scale normal)
    • B: 1.1 (scale destacado)
    • Alpha: Alpha (da Timeline)
    ↓
[Make Vector]
    • X: (resultado do Lerp)
    • Y: (resultado do Lerp)
    • Z: (resultado do Lerp)
    ↓
[Set Actor Scale]
    • New Scale: (resultado do Make Vector)
```

---

## 🎯 **IMPLEMENTAÇÃO RECOMENDADA (Mais Simples)**

Para começar rapidamente, use a **OPÇÃO 3 (Mudar Scale)**:

```
[HighlightSelected]
    ↓
[Set Actor Scale]
    • Target: (Self)
    • New Scale: (1.1, 1.1, 1.1)
```

**Passo a Passo Rápido:**

1. No Event Graph da função `HighlightSelected`
2. Arraste do pino **exec** do nó `HighlightSelected`
3. Digite: `Set Actor Scale`
4. Selecione: **Set Actor Scale**
5. **Target:** Deixe vazio (ou arraste `Self`)
6. **New Scale:** Clique direito → **Make Vector**
   - **X:** 1.1
   - **Y:** 1.1
   - **Z:** 1.1
7. Conecte o **Return Value** do `Make Vector` ao pino **New Scale**

---

## 🔄 **FUNÇÃO COMPLEMENTAR: RemoveHighlight**

Crie também uma função para remover o highlight:

```
[RemoveHighlight]
    ↓
[Set Actor Scale]
    • Target: (Self)
    • New Scale: (1.0, 1.0, 1.0)
```

E atualize a função `SelectClass`:

```
[SelectClass]
    • ClassID (input)
    ↓
[Call Function: RemoveHighlight] (remove highlight de todos)
    • Target: (Self)
    ↓
[SET] bIsSelected = true
    ↓
[Call Function: HighlightSelected]
    • Target: (Self)
    ↓
[Broadcast] OnClassSelected
    • ClassID: ClassID
```

---

## ✅ **CHECKLIST**

- [ ] Função `HighlightSelected` criada
- [ ] Efeito visual escolhido (Scale, Material, Light, etc.)
- [ ] Lógica implementada no Event Graph
- [ ] Função compila sem erros
- [ ] Efeito funciona quando a função é chamada
- [ ] (Opcional) Função `RemoveHighlight` criada

---

## 📝 **NOTAS**

- **Scale:** Mais simples, funciona imediatamente
- **Material:** Requer material com parâmetros configurados
- **Light:** Efeito mais dramático, mas pode afetar performance
- **Timeline:** Efeito mais suave e profissional
- **Combinado:** Melhor resultado visual, mas mais complexo

---

**Fim do Guia**

