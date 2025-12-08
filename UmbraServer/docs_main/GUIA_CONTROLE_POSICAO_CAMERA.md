# 🎯 **GUIA: Controle de Posição da Câmera**

## 📋 **OBJETIVO**

Ter controle total sobre a posição final da câmera ao selecionar uma classe, permitindo ajustar a posição exata onde a câmera deve ficar.

---

## ✅ **FUNÇÕES DISPONÍVEIS**

### **1. `MoveCameraToLocation` - Posição Absoluta**

Move a câmera para uma posição específica (X, Y, Z) no mundo.

**Parâmetros:**
- `CameraActor` - O Actor da câmera
- `TargetLocation` - Posição final desejada (Vector: X, Y, Z)
- `Duration` - Duração da animação em segundos (padrão: 1.0)

**Uso no Blueprint:**
```
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Call Function: Move Camera To Location]
    • CameraActor: (CameraActor encontrado)
    • Target Location: (Make Vector)
        • X: 500.0
        • Y: -300.0
        • Z: 150.0
    • Duration: 1.0
```

---

### **2. `MoveCameraToSelectionWithOffsets` - Offsets Relativos ao Placeholder**

Move a câmera para uma posição relativa ao placeholder selecionado usando offsets.

**Parâmetros:**
- `TargetPlaceholder` - O placeholder do personagem selecionado
- `CameraActor` - O Actor da câmera
- `Duration` - Duração da animação em segundos (padrão: 1.0)
- `OffsetX` - Distância à direita (+) ou esquerda (-) do placeholder
- `OffsetY` - Distância à frente (+) ou atrás (-) do placeholder
- `OffsetZ` - Altura relativa ao placeholder (acima +, abaixo -)

**Uso no Blueprint:**
```
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Call Function: Move Camera To Selection With Offsets]
    • Target Placeholder: (Placeholder selecionado)
    • Camera Actor: (CameraActor encontrado)
    • Duration: 1.0
    • Offset X: 200.0  (à direita do placeholder)
    • Offset Y: 0.0    (mesma posição Y do placeholder)
    • Offset Z: 100.0   (100 unidades acima do placeholder)
```

---

## 🔧 **COMO AJUSTAR A POSIÇÃO DA CÂMERA**

### **OPÇÃO 1: Usar Offsets (Recomendado para começar)**

1. **No Blueprint `WBP_CreateCharacter`**, encontre a função `SelectClassByID`
2. **Substitua** a chamada `SelectClassAndMoveCamera` por:
   - `Get Actor of Class` → `BP_CharacterCreationManager`
   - `Find Placeholder By Class ID` (ou obtenha o placeholder de outra forma)
   - `Find Camera Actor` (no Manager)
   - `Move Camera To Selection With Offsets`
3. **Ajuste os valores de Offset:**
   - **OffsetX**: Se a câmera está passando pelo personagem, aumente este valor (ex: 300.0, 400.0)
   - **OffsetY**: Se quiser a câmera mais à frente ou atrás do personagem
   - **OffsetZ**: Se quiser a câmera mais alta ou mais baixa

**Exemplo de valores para testar:**
- `OffsetX: 300.0` (mais à direita, não passa pelo personagem)
- `OffsetY: -50.0` (ligeiramente atrás do personagem)
- `OffsetZ: 120.0` (um pouco mais alto)

---

### **OPÇÃO 2: Usar Posição Absoluta (Para controle total)**

1. **No Editor do Unreal**, posicione a câmera manualmente onde você quer que ela fique
2. **Anote a posição** (X, Y, Z) da câmera
3. **No Blueprint**, use `MoveCameraToLocation` com essa posição exata

**Exemplo:**
```
[Move Camera To Location]
    • Camera Actor: (CameraActor)
    • Target Location: (Make Vector)
        • X: 669.562
        • Y: -305.272
        • Z: 91.012
    • Duration: 1.0
```

---

## 📝 **EXEMPLO COMPLETO: Usar SelectClassAndMoveCamera com Controles**

A função `SelectClassAndMoveCamera` agora tem parâmetros para controlar a posição da câmera!

No `WBP_CreateCharacter`, função `SelectClassByID`:

```
[SelectClassByID]
    • ClassID (input)
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Call Function: Select Class]
    • ClassID: ClassID (input)
    ↓
[Get Actor of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Call Function: Select Class And Move Camera]
    • ClassID: ClassID (input)
    • Duration: 1.0        ← Duração da animação
    • Offset X: 300.0      ← AJUSTE ESTE VALOR (distância à direita/esquerda)
    • Offset Y: 0.0        ← AJUSTE ESTE VALOR (distância à frente/atrás)
    • Offset Z: 120.0      ← AJUSTE ESTE VALOR (altura relativa)
```

**Pronto!** A função já encontra o placeholder e a câmera automaticamente. Você só precisa ajustar os valores de Offset.

---

## 🎮 **DICAS**

1. **Para encontrar a posição ideal:**
   - Execute o jogo em PIE (Play In Editor)
   - Posicione a câmera manualmente onde você quer
   - Anote a posição (X, Y, Z) no log ou na viewport
   - Use essa posição em `MoveCameraToLocation`

2. **Para ajustar offsets:**
   - Comece com valores pequenos e vá aumentando
   - `OffsetX` positivo = câmera à direita do personagem
   - `OffsetY` positivo = câmera à frente do personagem
   - `OffsetZ` positivo = câmera acima do personagem

3. **Logs úteis:**
   - Os logs mostram a posição inicial e final da câmera
   - Use esses valores para ajustar os offsets

---

## ✅ **RESUMO**

- **`SelectClassAndMoveCamera`** (RECOMENDADO): Função completa que encontra o placeholder, despawna outros, aplica highlight e move a câmera. Agora com parâmetros para controlar Duration e Offsets (X, Y, Z).
- **`MoveCameraToLocation`**: Use quando souber a posição exata (X, Y, Z) onde a câmera deve ficar
- **`MoveCameraToSelectionWithOffsets`**: Use quando quiser posicionar a câmera relativa ao placeholder (mais fácil de ajustar)

**A função `SelectClassAndMoveCamera` agora aceita parâmetros de controle de câmera diretamente! Basta passar os valores de Duration, OffsetX, OffsetY e OffsetZ no Blueprint.**

