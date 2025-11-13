# 🔄 **ALTERNATIVA: Usar Action Mapping (Sistema Antigo)**

## 🎯 **SE VOCÊ PREFERE USAR ACTION MAPPING:**

**Se você não quer usar Input Actions, pode usar o sistema antigo de Action Mapping diretamente no código C++.**

---

## ✅ **SOLUÇÃO: Adicionar Action Mapping no C++**

### **MODIFICAÇÃO 1: Adicionar Função de Setup no C++**

**No `UmbraEternumUECharacter.cpp`, adicione após `SetupPlayerInputComponent`:**

```cpp
void AUmbraEternumUECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // ... código existente ...
    
    // Adicionar Action Mapping para zoom (sistema antigo)
    if (UInputComponent* InputComp = GetInputComponent())
    {
        // Mouse Wheel Zoom
        InputComp->BindAxis("MouseWheelZoom", this, &AUmbraEternumUECharacter::ZoomAxis);
    }
}
```

### **MODIFICAÇÃO 2: Adicionar Função ZoomAxis no Header**

**No `UmbraEternumUECharacter.h`:**

```cpp
protected:
    /** Called for mouse wheel zoom input (Axis Mapping) */
    void ZoomAxis(float Value);
```

### **MODIFICAÇÃO 3: Implementar Função ZoomAxis**

**No `UmbraEternumUECharacter.cpp`:**

```cpp
void AUmbraEternumUECharacter::ZoomAxis(float Value)
{
    if (CameraBoom)
    {
        // Calculate new target arm length
        float NewTargetArmLength = CameraBoom->TargetArmLength - (Value * ZoomSpeed);
        
        // Clamp to min/max distance
        NewTargetArmLength = FMath::Clamp(NewTargetArmLength, MinCameraDistance, MaxCameraDistance);
        
        // Apply new distance
        CameraBoom->TargetArmLength = NewTargetArmLength;
    }
}
```

---

## 📋 **CONFIGURAR NO EDITOR (Project Settings):**

### **PASSO 1: Abrir Project Settings**

1. **Edit** → **Project Settings**
2. **Categoria:** **Input** (lado esquerdo)
3. **Seção:** **Axis Mappings**

### **PASSO 2: Adicionar Axis Mapping**

1. **Clique no `+`** ao lado de **Axis Mappings**
2. **Nome:** `MouseWheelZoom`
3. **Key:** Selecione **`Mouse Wheel Axis`**
4. **Scale:** `1.0` (ou `-1.0` se quiser inverter)
5. **Salve** (não precisa compilar)

---

## ✅ **VANTAGENS DO ACTION MAPPING:**

- ✅ Mais simples (não precisa criar Input Actions)
- ✅ Configuração direta no Project Settings
- ✅ Funciona imediatamente após configurar

---

## ⚠️ **DESVANTAGENS:**

- ❌ Sistema antigo (pode ser descontinuado no futuro)
- ❌ Menos flexível que Enhanced Input
- ❌ Não suporta múltiplos mapeamentos facilmente

---

## 🎯 **RECOMENDAÇÃO:**

**Use Action Mapping se:**
- Você já está familiarizado com o sistema antigo
- Quer uma solução rápida
- Não precisa de flexibilidade extra

**Use Input Actions se:**
- Quer usar o sistema moderno do UE5
- Precisa de múltiplos mapeamentos
- Quer mais controle e flexibilidade

---

## ✅ **RESULTADO:**

**Ambos os métodos funcionam! Escolha o que preferir.**

