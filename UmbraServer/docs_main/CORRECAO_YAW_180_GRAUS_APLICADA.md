> **OBSOLETO (2026-08):** não reaplicar `Yaw + 180` no parse. Fix correto: remover `360 - Yaw` no envio — [`GUIA_MOVIMENTO_REMOTE_YAW.md`](GUIA_MOVIMENTO_REMOTE_YAW.md).

# ✅ **CORREÇÃO: Yaw com Offset de 180 Graus Aplicada**

## 🎯 **PROBLEMA:**

**Quando um client vê os dois actors virados para a mesma direção, o outro client vê o remote actor virado para a direção contrária (180 graus).**

**Isso indica que o Yaw está sendo invertido ao aplicar aos remote actors.**

---

## ✅ **SOLUÇÃO APLICADA:**

**Adicionado offset de 180 graus ao Yaw ao parsear os frames binários.**

### **Modificações em `WSBinaryBPFL.cpp`:**

**1. Função `ParseStateUpdateFrame` (frame antigo - 25 bytes):**

**ANTES:**
```cpp
OutYawDegrees = Yaw;
```

**DEPOIS:**
```cpp
// CORREÇÃO: Adicionar 180 graus ao Yaw para corrigir rotação invertida
OutYawDegrees = Yaw + 180.0f;
// Normalizar Yaw para o range 0-360
while (OutYawDegrees >= 360.0f) OutYawDegrees -= 360.0f;
while (OutYawDegrees < 0.0f) OutYawDegrees += 360.0f;
```

**2. Função `ParseStateUpdateFrameWithAnimation` (frame novo - 34 bytes):**

**ANTES:**
```cpp
OutYawDegrees = Yaw;
```

**DEPOIS:**
```cpp
// CORREÇÃO: Adicionar 180 graus ao Yaw para corrigir rotação invertida
OutYawDegrees = Yaw + 180.0f;
// Normalizar Yaw para o range 0-360
while (OutYawDegrees >= 360.0f) OutYawDegrees -= 360.0f;
while (OutYawDegrees < 0.0f) OutYawDegrees += 360.0f;
```

---

## 🎯 **POR QUE ISSO FUNCIONA:**

**Ao adicionar 180 graus ao Yaw, corrigimos a inversão de rotação ANTES de aplicar aos remote actors.**

**A normalização garante que o Yaw fique no range 0-360 graus, evitando valores negativos ou acima de 360.**

**Exemplos:**
- Se Yaw recebido = 0° → OutYawDegrees = 180° (virado para o lado oposto)
- Se Yaw recebido = 90° → OutYawDegrees = 270° (virado para o lado oposto)
- Se Yaw recebido = 180° → OutYawDegrees = 0° (normalizado)
- Se Yaw recebido = 270° → OutYawDegrees = 90° (normalizado)

---

## 🧪 **TESTE:**

1. **Compile** o projeto (C++ foi modificado)
2. **Execute** o jogo
3. **Mova** um personagem e verifique a rotação
4. **Verifique** se os remote actors estão virados na mesma direção que o local character

**Se ainda estiver incorreto:**
- Pode ser necessário **subtrair** 180 graus em vez de adicionar
- Ou pode ser necessário um offset diferente (90 graus, 270 graus)

---

## 📝 **NOTA:**

**Se o problema persistir, pode ser necessário:**
- Verificar como o Yaw é obtido do character local
- Verificar se há diferença entre rotação do Actor e direção de movimento
- Calcular o Yaw a partir da velocidade em vez de usar `GetActorRotation().Yaw`

---

## ✅ **RESULTADO ESPERADO:**

**Após compilar e testar:**
- ✅ Remote actors virados na mesma direção que o local character
- ✅ Rotação sincronizada corretamente entre clients
- ✅ Inversão de 180 graus corrigida

**Com isso, o problema de rotação invertida deve ser resolvido!**

