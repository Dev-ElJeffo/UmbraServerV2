# 🎯 **GUIA: Usar Função C++ MoveCameraToSelection**

## 📋 **OBJETIVO**

Usar a função C++ `MoveCameraToSelection` do `AUmbraCharacterCreationManager` para mover a câmera suavemente em 1 segundo.

---

## ✅ **PASSO A PASSO**

### **PASSO 1: Compilar o C++**

1. **Feche o Unreal Engine** (se estiver aberto)
2. **Compile o projeto:**
   - Abra o arquivo `.uproject` com **Generate Visual Studio Project Files**
   - Ou compile diretamente no Visual Studio
3. **Aguarde a compilação terminar**
4. **Abra o Unreal Engine novamente**

### **PASSO 2: Criar Blueprint BP_CharacterCreationManager**

1. **No Content Browser**, clique com botão direito
2. **Blueprint Class** → **All Classes** → Procure por `Umbra Character Creation Manager`
3. **Selecione** `Umbra Character Creation Manager` como classe pai
4. **Nomeie:** `BP_CharacterCreationManager`
5. **Abra o Blueprint**

### **PASSO 3: Adicionar Variáveis**

No painel **My Blueprint** → **Variables**:

1. **`CameraActor`** (Actor Reference)
   - **Category:** "Camera"
   - **Instance Editable:** ✅ true
   - **Expose on Spawn:** false

2. **`SelectedPlaceholder`** (BP_Class_Placeholder Reference)
   - **Category:** "Selection"
   - **Instance Editable:** false

### **PASSO 4: Implementar MoveCameraToSelection no Blueprint**

No **Event Graph**, quando você quiser mover a câmera (ex: no `OnClassSelected_Handler`):

```
[OnClassSelected_Handler]
    • ClassID (input)
    ↓
[Find Placeholder By Class ID]
    • ClassID: ClassID
    • Found Placeholder: (variável local)
    ↓
[Branch]
    • Condition: (Found Placeholder é válido?)
    ↓ (True)
    [Get] CameraActor
        ↓
    [Call Function: Move Camera To Selection]
        • Target: (Self - BP_CharacterCreationManager)
        • Target Placeholder: Found Placeholder
        • Camera Actor: CameraActor
        • Duration: 1.0 (padrão, pode deixar vazio)
        • Offset X: 200.0 (padrão, pode deixar vazio)
        • Offset Z: 100.0 (padrão, pode deixar vazio)
```

---

## 🎯 **FUNÇÕES DISPONÍVEIS**

### **1. MoveCameraToSelection**

**Parâmetros:**
- `TargetPlaceholder` (Actor Reference) - O placeholder selecionado
- `CameraActor` (Actor Reference) - A câmera a ser movida
- `Duration` (Float) - Duração em segundos (padrão: 1.0)
- `OffsetX` (Float) - Offset X da posição final (padrão: 200.0)
- `OffsetZ` (Float) - Offset Z da posição final (padrão: 100.0)

**O que faz:**
- Move a câmera suavemente da posição atual para a posição do placeholder + offset
- Usa interpolação com curva suave (Ease In Out)
- Duração padrão: 1 segundo

### **2. StopCameraMovement**

**Parâmetros:** Nenhum

**O que faz:**
- Para o movimento da câmera imediatamente
- Garante que a câmera fique na posição final exata

### **3. IsCameraMoving**

**Parâmetros:** Nenhum

**Retorno:** Boolean

**O que faz:**
- Retorna `true` se a câmera está se movendo
- Retorna `false` se a câmera está parada

---

## 📝 **EXEMPLO COMPLETO: OnClassSelected_Handler**

```
[OnClassSelected_Handler]
    • ClassID (input)
    ↓
[Find Placeholder By Class ID]
    • ClassID: ClassID
    • Found Placeholder: (variável local)
    ↓
[Branch]
    • Condition: (Is Valid? Found Placeholder)
    ↓ (True)
    [Get] CameraActor
        ↓
    [Call Function: Move Camera To Selection]
        • Target: (Self)
        • Target Placeholder: Found Placeholder
        • Camera Actor: CameraActor
        • Duration: 1.0
        • Offset X: 200.0
        • Offset Z: 100.0
    ↓
    [Despawn Other Placeholders]
        • Keep: Found Placeholder
    ↓
    [Show Class Info Widget]
        • ClassID: ClassID
```

---

## ⚙️ **AJUSTES**

### **Duração do Movimento:**
- `0.5` = Movimento rápido (0.5 segundos)
- `1.0` = Movimento médio (1 segundo) ← **PADRÃO**
- `2.0` = Movimento lento (2 segundos)

### **Offset da Câmera:**
- `OffsetX: 200.0` = Câmera fica 200 unidades à direita do placeholder
- `OffsetZ: 100.0` = Câmera fica 100 unidades acima do placeholder

**Ajuste conforme necessário para sua cena!**

---

## ✅ **RESUMO**

1. **Compile o C++**
2. **Crie BP_CharacterCreationManager** (herdando de `Umbra Character Creation Manager`)
3. **Adicione variável `CameraActor`**
4. **Chame `Move Camera To Selection`** quando selecionar uma classe
5. **Pronto!** A câmera se move suavemente em 1 segundo

**Sem Tick, sem Timer manual, sem Timeline. Tudo feito em C++ com Timer interno.**

---

**Fim do Guia**

