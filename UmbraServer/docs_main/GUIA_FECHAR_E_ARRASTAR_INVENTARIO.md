# 🎮 **GUIA: Botão de Fechar e Arrastar Inventário**

## 📋 **O QUE FAREMOS:**

1. ✅ Adicionar botão de fechar no inventário
2. ✅ Tornar o inventário arrastável pela tela
3. ✅ Mudar o Input Mode para permitir movimento do jogador

---

## 🔴 **PARTE 1: ADICIONAR BOTÃO DE FECHAR**

### **1.1 No Designer do WBP_Inventory:**

1. Abra o `WBP_Inventory` no **Designer**

2. **Adicione um Button:**
   - Na **Palette** (esquerda), procure: `Button`
   - Arraste para o **Canvas Panel** (ou dentro do `Border_BG`)
   - Posicione no **canto superior direito** do inventário

3. **Configure o Button:**
   - Renomeie para: `Btn_Close`
   - No painel **Details**:
     - **Position X**: `870` (ajuste para seu tamanho)
     - **Position Y**: `10`
     - **Size X**: `30`
     - **Size Y**: `30`
     - **Anchors**: Canto superior direito (⚓ top-right)

4. **Adicione um Text ao Button:**
   - Dentro da hierarquia do `Btn_Close`, adicione um **Text Block**
   - Renomeie para: `Text_CloseIcon`
   - Configure o texto: **"X"**
   - Font Size: **20**
   - Justification: **Center**
   - Color: **Branco** ou **Vermelho**

5. **Estilize o Button (opcional):**
   - No `Btn_Close`, no painel **Details**
   - Seção **Style** → **Normal**
     - Tint: `(R=0.8, G=0.2, B=0.2, A=1)` (vermelho escuro)
   - Seção **Style** → **Hovered**
     - Tint: `(R=1, G=0.3, B=0.3, A=1)` (vermelho claro)
   - Seção **Style** → **Pressed**
     - Tint: `(R=0.5, G=0.1, B=0.1, A=1)` (vermelho muito escuro)

---

### **1.2 No Event Graph do WBP_Inventory:**

1. Volte para o **Event Graph**

2. **Adicione o evento OnClicked:**
   - Na hierarquia (esquerda), selecione `Btn_Close`
   - No painel **Details** (direita), role até **Events**
   - Clique no botão **"+"** ao lado de **On Clicked**

3. **Implemente a lógica de fechar:**

```blueprint
[Event On Clicked (Btn_Close)]
    │
    ▼
[Remove from Parent]
    │ Target: self (WBP_Inventory)
    │
    ▼
[Get Player Controller] (Index: 0)
    │
    ▼
[Set Input Mode Game Only]
    │ Target: (Player Controller)
    │ Player Controller: (do Get acima)
    │
    ▼
[Set Show Mouse Cursor]
    │ Target: (Player Controller)
    │ Show Mouse Cursor: FALSE
```

---

### **1.3 Código Blueprint (Event On Clicked):**

Se você preferir copiar/colar, aqui está a estrutura:

**Nós necessários:**
1. `Event On Clicked (Btn_Close)` - automático
2. `Remove from Parent` - remove o inventário da tela
3. `Get Player Controller` (Index 0) - pega o controller
4. `Set Input Mode Game Only` - volta o foco para o jogo
5. `Set Show Mouse Cursor` (FALSE) - esconde o cursor

---

## 🖱️ **PARTE 2: TORNAR O INVENTÁRIO ARRASTÁVEL**

### **2.1 Adicionar uma "Barra de Título" (Handle):**

1. No **Designer** do `WBP_Inventory`

2. **Adicione um novo Border (para a barra):**
   - Arraste um `Border` para dentro do `Canvas Panel`
   - Renomeie para: `Border_TitleBar`
   - Posicione no **topo** do inventário
   - Configure:
     - **Position X**: `0`
     - **Position Y**: `0`
     - **Size X**: `900` (largura total)
     - **Size Y**: `40` (altura da barra)
     - **Anchors**: Top Stretch (estica horizontalmente)

3. **Estilize o Border_TitleBar:**
   - Background Color: `(R=0.1, G=0.1, B=0.1, A=0.8)` (preto semi-transparente)
   - Padding: `10, 5, 10, 5`

4. **Adicione um Text ao Title Bar:**
   - Dentro do `Border_TitleBar`, adicione um **Text Block**
   - Renomeie para: `Text_Title`
   - Texto: **"Inventory"** ou **"Inventário"**
   - Font Size: **18**
   - Color: Amarelo ou Branco
   - Justification: **Center**

5. **Mova o botão de fechar para a Title Bar:**
   - Arraste o `Btn_Close` para dentro do `Border_TitleBar`
   - Reposicione no canto direito

---

### **2.2 Implementar Drag (Override OnMouseButtonDown):**

1. No **Event Graph** do `WBP_Inventory`

2. **Override OnMouseButtonDown:**
   - No painel **"Functions"** (esquerda, embaixo)
   - Clique no botão dropdown **"Override"**
   - Procure na lista: **"On Mouse Button Down"**
   - Clique nele para adicionar

3. **Implemente a lógica de drag:**

```blueprint
[On Mouse Button Down] (Override)
    │ My Geometry
    │ Mouse Event (PointerEvent)
    │
    ▼
[Detected Drag If Pressed]
    │ Pointer Event: (Mouse Event do override)
    │ Drag Key: Left Mouse Button
    │
    └─> Return Value (Event Reply)
        │
        └─> [Return Node]
            │ Return Value: (Event Reply do Detected Drag)
```

**Passo a passo detalhado:**

1. Do pin **MouseEvent** do override `On Mouse Button Down`:
   - Arraste e solte no espaço vazio
   - Procure: **`Detected Drag If Pressed`**
   
2. No nó `Detected Drag If Pressed`:
   - Pin **Pointer Event**: já conectado automaticamente
   - Pin **Drag Key**: selecione `Left Mouse Button` no dropdown
   
3. Do pin **Return Value** do `Detected Drag If Pressed`:
   - Conecte ao pin **Return Value** do `Return Node`

---

### **2.3 Implementar OnDragDetected:**

1. **Override OnDragDetected:**
   - No painel **"Functions"** (esquerda, embaixo)
   - Clique no botão dropdown **"Override"**
   - Procure na lista: **"On Drag Detected"**
   - Clique nele para adicionar

2. **Implemente a lógica:**

```blueprint
[On Drag Detected] (Override)
    │ My Geometry
    │ Pointer Event
    │
    ▼
[Create Drag Drop Operation]
    │ Class: DragDropOperation (deixe default)
    │ Pivot: Mouse Down (0.5, 0.5)
    │ Offset: (0, 0)
    │ Default Drag Visual: self (WBP_Inventory)
    │
    └─> Return Value (Drag Drop Operation)
        │
        └─> [Return Node]
            │ Return Value: (Drag Drop Operation)
```

**Passo a passo detalhado:**

1. Clique com botão direito no espaço vazio
2. Procure: **`Create Drag Drop Operation`**
3. Adicione o nó

4. Configure os pins:
   - **Pivot**: Clique e selecione `Mouse Down` (ou deixe em 0.5, 0.5)
   - **Default Drag Visual**: Arraste do `self` (pin de referência) ou digite "self"

5. Conecte:
   - Pin **Return Value** do `Create Drag Drop Operation` → `Return Node` pin **Return Value**

---

### **2.4 Implementar OnDrop:**

1. **Override OnDrop:**
   - No painel **"Functions"** (esquerda, embaixo)
   - Clique no botão dropdown **"Override"**
   - Procure na lista: **"On Drop"**
   - Clique nele para adicionar

2. **Implemente a lógica:**

```blueprint
[On Drop] (Override)
    │ My Geometry
    │ Pointer Event
    │ Operation
    │
    ▼
[Get Screen Space Position]
    │ Target: Pointer Event
    │
    ▼
[Get Desired Size] ←─────────────────┐
    │ Target: self                    │
    │                                 │
    ▼                                 │
[Vector2D / Vector2D]                 │
    │ A: (Get Screen Space Position)  │
    │ B: (Get Desired Size)           │
    │ Output: (Divided Vector)        │
    │                                 │
    ▼                                 │
[Set Position in Viewport]            │
    │ Target: self (WBP_Inventory)    │
    │ Position: (Divided Vector)      │
    │ Remove DPI Scale: FALSE ◄────────── IMPORTANTE!
    │
    │ (then) ──────────────────────────────────┐
    │                                         │
    │                                         ▼
    │                                    [Return Node]
    │                                       │ execute
    │                                       │ Return Value (pode deixar vazio!)
    │
    │ OPCIONAL:
    │ [Handled] ou [Unhandled]
    │     │ Return Value
    │     └──────> (conecte ao Return Value se quiser)
```

**⚠️ CORREÇÃO IMPORTANTE: Dividir pelo tamanho do widget**

**Passo a passo detalhado:**

1. Do pin **Pointer Event** do `On Drop`:
   - **ARRASTE DO PIN** `Pointer Event` e solte no espaço vazio
   - Procure por um destes nomes:
     - **`Get Screen Space Position`** (mais comum)
     - **`Get Last Screen Space Position`**
     - **`Get Mouse Position on Platform`**
     - **`Get Screen Space Pointer Position`**
   - Se NENHUM aparecer, **desmarque "Context Sensitive"** e procure novamente
   - Isso retorna a posição do mouse em pixels (Vector2D)

2. Adicione **`Get Desired Size`**:
   - Target: arraste `self` do canto superior esquerdo
   - Isso retorna o tamanho do inventário (width, height)

3. Adicione **`Vector2D / Vector2D`** (divisão):
   - Pin **A**: conecte ao output do `Get Screen Space Position`
   - Pin **B**: conecte ao output do `Get Desired Size`
   - Isso normaliza a posição

4. Adicione **`Set Position in Viewport`**:
   - Pin **Target**: `self` (o próprio WBP_Inventory)
   - Pin **Position**: conecte ao output da divisão
   - Pin **Remove DPI Scale**: marque como **FALSE** (não TRUE!)

5. Conecte a execução:
   - Pin **then** do `Set Position in Viewport` → Pin **execute** do `Return Node`

6. **IMPORTANTE - 3 OPÇÕES para o Return Value:**

   **OPÇÃO 1 - Deixar vazio (MAIS SIMPLES):**
   - Não conecte nada ao `Return Value` do `Return Node`
   - O Unreal automaticamente retorna `Unhandled` (funciona perfeitamente)

   **OPÇÃO 2 - Usar Unhandled:**
   - Clique com botão direito no espaço vazio
   - Procure: `Unhandled`
   - Conecte o **Return Value** do `Unhandled` ao `Return Value` do `Return Node`

   **OPÇÃO 3 - Usar Handled (se funcionar):**
   - Clique com botão direito no espaço vazio
   - Procure: `Handled`
   - Conecte o **Return Value** do `Handled` ao `Return Value` do `Return Node`
   - ⚠️ Se não conectar, use a **OPÇÃO 1** ou **OPÇÃO 2**

---

### **✅ SOLUÇÃO MAIS PRECISA (RECOMENDADA):**

Para o inventário ir **EXATAMENTE** onde você soltar o mouse:

**1. Modifique o `OnMouseButtonDown` para salvar o offset:**

```blueprint
[On Mouse Button Down] (Override)
    │ My Geometry
    │ Mouse Event
    │
    ├─> [Get Position in Viewport] ←─────────┐
    │      │ Target: self                     │
    │      │ Output: Current Position         │
    │      │                                  │
    ├─> [Get Screen Space Position]           │
    │      │ Target: Mouse Event              │
    │      │ Output: Mouse Position           │
    │      │                                  │
    │   [Subtract (Vector2D - Vector2D)]      │
    │      │ A: Mouse Position                │
    │      │ B: Current Position              │
    │      │ Output: Offset                   │
    │      │                                  │
    │   [Set DragOffset]                      │
    │      │ Drag Offset: (Offset)            │
    │                                         │
    ├─> [Detected Drag If Pressed]            │
    │      │ Pointer Event: Mouse Event       │
    │      │                                  │
    │      └─> Return Value → [Return Node]   │
```

**2. Modifique o `OnDrop` para usar o offset salvo:**

```blueprint
[On Drop] (Override)
    │ Pointer Event
    │
    ├─> [Get Screen Space Position]
    │      │ Target: Pointer Event
    │      │ Output: Mouse Position
    │      │
    │   [Get DragOffset]
    │      │ Output: Saved Offset
    │      │
    │   [Subtract (Vector2D - Vector2D)]
    │      │ A: Mouse Position
    │      │ B: Saved Offset
    │      │ Output: Final Position
    │      │
    │   [Set Position in Viewport]
    │      │ Target: self
    │      │ Position: Final Position
    │      │ Remove DPI Scale: FALSE
    │      │ (then) ────────────────────────┐
    │                                       │
    │                                       ▼
    │                                  [Return Node]
    │                                    │ execute
    │                                    │ Return Value (DEIXE VAZIO!)
    │
    │   OPCIONAL: [Handled] ou [Unhandled]
    │      │ Return Value
    │      └──────> (conecte ao Return Value se quiser)
```

**Passo a passo simplificado:**

**No OnMouseButtonDown, ADICIONE antes do Detected Drag:**
1. `Get Position in Viewport` (Target: self) - posição atual do widget
2. `Get Screen Space Position` (Target: Mouse Event) - posição do mouse
3. `Subtract` (Mouse - Widget Position) = **Offset**
4. `Set DragOffset` - salva o offset

**No OnDrop, USE o offset:**
1. `Get Screen Space Position` (do Pointer Event) - posição onde soltou
   - **ALTERNATIVA:** `Get Mouse Position` (do PlayerController)
2. `Get DragOffset` - recupera o offset salvo
3. `Subtract` (Mouse Position - Offset) = **Posição Final**
4. `Set Position in Viewport` com a Posição Final

**⚠️ ALTERNATIVA UNIVERSAL (se Get Screen Space Position não aparecer):**

Use esta sequência (funciona em TODAS as versões do Unreal):

```blueprint
[Get Player Controller] (Index: 0)
    │
    ▼
[Get Mouse Position in Viewport]
    │ Location X (float)
    │ Location Y (float)
    │
    ▼
[Make Vector2D]
    │ X: Location X
    │ Y: Location Y
    │ Output: Vector2D (posição do mouse)
```

**Passo a passo:**
1. Adicione `Get Player Controller` (Player Index: 0)
2. Do output, adicione `Get Mouse Position in Viewport`
3. Adicione `Make Vector2D` e conecte Location X e Location Y
4. Use o output do Make Vector2D como a posição do mouse

📄 **Veja:** `ALTERNATIVA_GET_MOUSE_POSITION.txt` para guia visual completo

---

## 🎮 **PARTE 3: MUDAR O INPUT MODE**

### **3.1 No WBP_Inventory, Event Construct:**

Atualize o `Event Construct` para **NÃO** usar "UI Only":

```blueprint
[Event Construct]
    │
    ▼
[Get Player Controller] (Index: 0)
    │
    ▼
[Set Input Mode Game And UI] ← MUDE PARA ESTE!
    │ Player Controller: (do Get acima)
    │ In Widget to Focus: self (WBP_Inventory)
    │
    ▼
[Set Show Mouse Cursor]
    │ Target: (Player Controller)
    │ Show Mouse Cursor: TRUE
```

**Diferença:**
- ❌ `Set Input Mode UI Only` - BLOQUEIA movimento do jogador
- ✅ `Set Input Mode Game And UI` - PERMITE movimento + UI

---

### **3.2 Configurar Input no PlayerController:**

Se o jogador ainda não conseguir se mover:

1. Vá para o seu **Player Controller** (ou Character Blueprint)
2. No `BeginPlay`, adicione:

```blueprint
[Event Begin Play]
    │
    ▼
[Enable Input]
    │ Target: self
    │ Player Controller: Get Player Controller (0)
```

---

## 🧪 **TESTE COMPLETO:**

### **Checklist de testes:**

1. [ ] Abrir o inventário (tecla 'I')
2. [ ] **Mover o jogador** (WASD) enquanto o inventário está aberto
3. [ ] **Clicar e arrastar** a barra de título para mover o inventário
4. [ ] **Clicar no botão X** para fechar
5. [ ] Após fechar, **cursor desaparece** e **jogador volta a se mover normalmente**

---

## 🎨 **MELHORIAS VISUAIS (OPCIONAL):**

### **Adicionar animação ao fechar:**

1. No **Animations** do `WBP_Inventory`, crie: `Anim_FadeOut`
2. Duração: `0.3s`
3. Track: `Render Opacity` (de 1.0 para 0.0)

4. No `Event On Clicked (Btn_Close)`:
```blueprint
[Event On Clicked (Btn_Close)]
    │
    ▼
[Play Animation]
    │ Target: self
    │ Animation: Anim_FadeOut
    │ Start at Time: 0
    │ Num Loops to Play: 1
    │ Play Mode: Forward
    │
    ▼
[Delay] (0.3 segundos)
    │
    ▼
[Remove from Parent]
```

---

## 📋 **RESUMO FINAL:**

| Feature | Status |
|---------|--------|
| Botão de fechar | ✅ |
| Input Mode "Game And UI" | ✅ |
| Inventário arrastável | ✅ |
| Cursor visível com inventário aberto | ✅ |
| Jogador pode se mover com inventário aberto | ✅ |

---

---

## ⚠️ **TROUBLESHOOTING:**

### **Problema: "Get Screen Space Position" não aparece**
**Solução:** 
- ✅ Arraste DO PIN "Pointer Event" e procure por:
  - `Get Screen Space Position`
  - `Get Last Screen Space Position`
  - `Get Cursor Position`
- ✅ Desmarque "Context Sensitive" se necessário
- ✅ **ALTERNATIVA UNIVERSAL:** Use esta sequência:
  1. `Get Player Controller` (Index: 0)
  2. `Get Mouse Position in Viewport`
  3. `Make Vector2D` (conecte Location X e Y)
- 📄 Veja: `ALTERNATIVA_GET_MOUSE_POSITION.txt`

### **Problema: "Detected Drag If Pressed" não aparece**
**Solução:** 
- Certifique-se de arrastar DO pin **MouseEvent** do override
- Se não aparecer, use a alternativa com `Unhandled` + `Detect Drag`

### **Problema: "Handled" não conecta no Return Value**
**Solução:** 
- ✅ **DEIXE O RETURN VALUE VAZIO!** (funciona perfeitamente)
- O Unreal automaticamente retorna `Unhandled` quando vazio
- Se quiser conectar algo, use `Unhandled` ao invés de `Handled`
- Handled/Unhandled são opcionais para este caso

### **Problema: Inventário não move ao arrastar**
**Solução:**
1. Verifique se `On Drag Detected` está criando o DragDropOperation
2. Verifique se `On Drop` está implementado
3. Verifique se `Set Position in Viewport` tem `self` como Target

### **Problema: Inventário vai para posição diferente de onde soltei**
**Solução:**
1. **MELHOR SOLUÇÃO:** Use o sistema de offset (salvar onde clicou no `OnMouseButtonDown` e subtrair no `OnDrop`)
2. Verifique se `DragOffset` está sendo salvo corretamente
3. Verifique se está usando `Subtract` no `OnDrop` (Mouse Position - DragOffset)
4. Certifique-se de que `Remove DPI Scale` está **FALSE** no `Set Position in Viewport`

### **Problema: Não consigo clicar no botão de fechar**
**Solução:**
- Certifique-se de que o `Btn_Close` está ACIMA da `Border_TitleBar` na hierarquia
- Ou coloque o botão FORA da Title Bar (no Canvas Panel diretamente)

### **Problema: Jogador não consegue se mover com inventário aberto**
**Solução:**
- Use `Set Input Mode Game And UI` ao invés de `UI Only`
- Verifique se `Enable Input` está sendo chamado no PlayerController

### **Problema: Cursor não aparece/desaparece corretamente**
**Solução:**
- Ao abrir: `Set Show Mouse Cursor` (TRUE)
- Ao fechar: `Set Show Mouse Cursor` (FALSE)

---

## 📋 **CHECKLIST FINAL:**

- [ ] Botão de fechar funciona
- [ ] Cursor esconde ao fechar
- [ ] Input volta para "Game Only" ao fechar
- [ ] Inventário pode ser arrastado pela tela
- [ ] Jogador pode se mover com inventário aberto
- [ ] Cursor está visível com inventário aberto

---

**Próximo passo: Implementar Drag & Drop de itens entre slots! 🎯**

