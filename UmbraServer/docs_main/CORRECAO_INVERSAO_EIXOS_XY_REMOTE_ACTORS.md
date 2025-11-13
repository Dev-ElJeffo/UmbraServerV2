# 🔧 **CORREÇÃO: Inversão de Eixos X e Y nos Remote Actors**

## 🎯 **PROBLEMA:**

**Os eixos X e Y dos remote actors estão invertidos em relação ao client local.**

**Quando um personagem se move em linha reta, ele aparece se movendo perpendicularmente (para a direita) no outro client.**

---

## 🔍 **CAUSA:**

**O problema está na aplicação da posição aos remote actors. O Unreal Engine usa:**
- **X** = Leste/Oeste (Right/Left)
- **Y** = Norte/Sul (Forward/Backward)
- **Z** = Cima/Baixo (Up/Down)

**Mas pode haver inversão entre:**
- Como a posição é **enviada** (do local character)
- Como a posição é **recebida e aplicada** (nos remote actors)

---

## ✅ **SOLUÇÃO: Trocar X e Y ao Aplicar Posição**

### **PASSO 1: Localizar Onde a Posição é Aplicada**

**No `BP_NetMovementClient`, no `Event Tick`:**

1. **Encontre** o node `Set Actor Location` que aplica a posição aos remote actors
2. **Encontre** o `InterpolatedLocation` (Vector) que é usado como input

### **PASSO 2: Trocar X e Y**

**ANTES (incorreto):**
```
[Lerp (Vector): StateA_Location → StateB_Location] → InterpolatedLocation
  ↓
[Set Actor Location (Target: RemoteActorRef, Location: InterpolatedLocation)]
```

**DEPOIS (correto):**
```
[Lerp (Vector): StateA_Location → StateB_Location] → InterpolatedLocation
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector (X: Y, Y: X, Z: Z)] → SwappedLocation
  ↓
[Set Actor Location (Target: RemoteActorRef, Location: SwappedLocation)]
```

**OU, se precisar trocar apenas no spawn inicial:**

**No `OnWSBinaryMessage`, quando spawna um novo remote actor:**
```
[ParseStateUpdateFrame] → OutLocation (Vector)
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector (X: Y, Y: X, Z: Z)] → SwappedLocation
  ↓
[Make Transform (Location: SwappedLocation, Rotation: OutYawDegrees, Scale: 1,1,1)]
  ↓
[Spawn Actor from Class (Transform: resultado)]
```

---

## 📋 **IMPLEMENTAÇÃO DETALHADA:**

### **No Blueprint `BP_NetMovementClient`, no `Event Tick`:**

1. **Encontre** o node `Lerp (Vector)` que interpola a posição (`InterpolatedLocation`)
2. **Antes** do `Set Actor Location`, adicione:
   - **`Break Vector`** → Conecte `InterpolatedLocation` ao input
   - **`Make Vector`** → Conecte:
     - **X:** `Y` (do Break Vector)
     - **Y:** `X` (do Break Vector)
     - **Z:** `Z` (do Break Vector - mantém igual)
3. **Conecte** o resultado do `Make Vector` ao input `New Location` do `Set Actor Location`

**Fluxo completo:**
```
[Lerp (Vector): StateA_Location → StateB_Location] → InterpolatedLocation
  ↓
[Break Vector] → X, Y, Z
  ↓
[Make Vector (X: Y, Y: X, Z: Z)] → SwappedLocation
  ↓
[Set Actor Location (Target: RemoteActorRef, Location: SwappedLocation)]
```

---

## 🧪 **TESTE:**

1. **Compile** o Blueprint
2. **Execute** o jogo
3. **Mova** um personagem em linha reta (para frente/trás)
4. **Verifique** se o remote actor se move na mesma direção

**Se ainda estiver incorreto:**
- **Verifique** se precisa trocar também no spawn inicial
- **Verifique** se há inversão também no Yaw (rotação)

---

## 🎯 **ALTERNATIVA: Corrigir no Parse**

**Se preferir, pode corrigir diretamente no código C++ ao parsear:**

**No `WSBinaryBPFL.cpp`, função `ParseStateUpdateFrame`:**

**ANTES:**
```cpp
OutLocation = FVector(X, Y, Z);
```

**DEPOIS:**
```cpp
OutLocation = FVector(Y, X, Z);  // Trocar X e Y
```

**Mas isso afetaria TODOS os usos do ParseStateUpdateFrame, então é melhor fazer no Blueprint!**

---

## 📝 **NOTA:**

**Se trocar X e Y não resolver completamente, pode ser necessário:**
- **Trocar também no spawn inicial** (quando cria o remote actor)
- **Ajustar o Yaw** (rotação) também
- **Verificar se há inversão no servidor** (como os dados são enviados)

---

## ✅ **RESULTADO ESPERADO:**

**Após trocar X e Y:**
- ✅ Remote actors se movem na mesma direção que o local character
- ✅ Personagens se movendo em linha reta aparecem corretos
- ✅ Posição sincronizada entre clients

**Com isso, o problema de inversão de eixos deve ser resolvido!**

