# 🔍 **VERIFICAÇÃO: Referências Hardcoded ao BP_Player**

## 🎯 **PROBLEMA:**

**Apenas trocou o Default Pawn de `BP_Player` para `BP_ThirdPersonCharacter` e remote actors pararam de spawnar.**

---

## 🔍 **VERIFICAÇÃO CRÍTICA:**

### **BP_NetMovementClient Tem Referência Hardcoded ao BP_Player?**

**LOCALIZAÇÃO:** `BP_NetMovementClient` (todo o Blueprint)

**VERIFICAÇÃO:**
1. Abrir `BP_NetMovementClient`
2. **Procurar por:**
   - Casts para `BP_Player`
   - Referências a `BP_Player` em variáveis
   - Spawns de `BP_Player`
   - Qualquer referência hardcoded ao `BP_Player`

**SE ENCONTRAR:**
- Trocar para usar `BP_ThirdPersonCharacter` ou usar uma referência genérica (Character, Pawn)

---

### **VERIFICAÇÃO ESPECÍFICA: SpawnActorFromClass**

**LOCALIZAÇÃO:** `BP_NetMovementClient:ProcessNextFrame`

**VERIFICAÇÃO:**
1. Localizar `SpawnActorFromClass`
2. **VERIFICAR:**
   - Qual classe está sendo usada para spawnar remote actors?
   - Está usando `BP_Player` hardcoded?
   - **SE ESTIVER:** Trocar para `BP_ThirdPersonCharacter` ou usar a mesma classe do local player

---

## 🔧 **CORREÇÃO:**

### **SE SpawnActorFromClass ESTÁ USANDO BP_Player:**

**TROCAR PARA:**
- Usar `BP_ThirdPersonCharacter` (se quiser que remote actors sejam iguais ao local)
- OU usar a mesma classe do local player (obter dinamicamente)

---

**Status:** 🔍 **VERIFICAR SE SPAWNACTORFROMCLASS ESTÁ USANDO BP_PLAYER HARDCODED**

