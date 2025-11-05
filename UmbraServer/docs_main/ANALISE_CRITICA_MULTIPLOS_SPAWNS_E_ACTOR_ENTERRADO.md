# 🔍 **ANÁLISE CRÍTICA: Múltiplos Spawns e Actor Enterrado no Chão**

## 📋 **OBSERVAÇÕES DA IMAGEM:**

### **Problemas Identificados:**

1. **✅ Limpeza Funcionando:**
   - Primeiro client loga sem outros actors
   - A função de cleanup está funcionando corretamente

2. **❌ Múltiplos Spawns Persistem:**
   - Client 1 e 2: Veem um actor extra além do esperado
   - Client 3: Vê um actor enterrado no chão (parcialmente visível)
   - Client 3 também vê os outros dois clients corretamente

3. **⚠️ LOG CRÍTICO: `Received binary message, size:0expected=25`:**
   - Aparece em **TODOS os clients**
   - Indica que mensagens **vazias (0 bytes)** estão sendo recebidas
   - O sistema espera 25 bytes, mas recebe 0 bytes
   - Isso pode causar processamento de frames inválidos

4. **⚠️ Performance Extremamente Ruim:**
   - FPS: 4 (muito baixo)
   - GPU: 99% (sobrecarga)
   - CPU: 34%
   - Warning: "expect extremely poor performance"

---

## 🚨 **PROBLEMAS CRÍTICOS IDENTIFICADOS:**

### **PROBLEMA 1: Mensagens Vazias (`size:0expected=25`)**

**Causa Raiz:**
- O WebSocket está recebendo mensagens vazias (0 bytes)
- O sistema espera 25 bytes (tamanho de um frame)
- Mensagens vazias podem estar sendo processadas como frames válidos

**Impacto:**
- Frames inválidos podem causar spawns com localização `(0,0,0)` ou inválida
- Actor enterrado pode ser resultado de `Z=-92.0` (valor default ou inválido)

**Solução Necessária:**
1. **Adicionar validação em `ProcessBinaryBuffer` para rejeitar mensagens vazias**
2. **Adicionar validação em `ProcessNextFrame` para verificar se `OutFrame` não está vazio**

---

### **PROBLEMA 2: Actor Enterrado no Chão**

**Causa Raiz:**
- O actor está sendo spawnado com localização incorreta
- Possivelmente `Z=-92.0` (valor negativo que enterra o actor)
- Ou `OutLocation` está sendo recebido como `(0,0,0)` antes da validação

**Solução Necessária:**
1. **Adicionar validação rigorosa de `OutLocation` ANTES de `SpawnActorFromClass`:**
   - Verificar se `OutLocation.Z` está em um range válido (ex: entre -1000 e 10000)
   - Verificar se `OutLocation` não é `(0,0,0)`
   - Verificar se `OutLocation` não contém valores NaN ou infinitos

---

### **PROBLEMA 3: Múltiplos Spawns Ainda Ocorrendo**

**Causa Raiz:**
- A verificação dupla pode não estar implementada ainda
- Ou há uma condição de corrida mais complexa
- Mensagens vazias podem estar causando múltiplos processamentos

**Solução Necessária:**
1. **Implementar verificação dupla conforme guia detalhado**
2. **Adicionar logs detalhados para diagnosticar:**
   - Log antes de cada `Array_Find`
   - Log antes de cada `SpawnActorFromClass`
   - Log antes de cada `Array_Add`
   - Log com `OutLocation` antes de spawnar

---

## 🔧 **CORREÇÕES NECESSÁRIAS:**

### **CORREÇÃO 1: Validar Mensagens Vazias em `ProcessBinaryBuffer` (C++)**

**Localização:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`

**Adicionar no início de `ProcessBinaryBuffer`:**

```cpp
bool UWSBinaryBPFL::ProcessBinaryBuffer(TArray<uint8>& Buffer, const TArray<uint8>& NewData, FBinaryFrame& OutFrame, int32 ExpectedPlayerID)
{
    // Adicionar novos dados ao buffer
    Buffer.Append(NewData);
    
    // NOVA VALIDAÇÃO: Rejeitar mensagens vazias
    if (NewData.Num() == 0 && Buffer.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Mensagem vazia recebida - ignorando"));
        return false;
    }
    
    // Constante: tamanho de um StateUpdateFrame = 25 bytes
    constexpr int32 FrameSize = 25;
    // ... resto do código
```

---

### **CORREÇÃO 2: Validar `OutFrame` Antes de Processar (Blueprint)**

**Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

**Adicionar APÓS `ProcessBinaryBuffer` e ANTES de `ParseStateUpdateFrame`:**

1. **Adicionar verificação de tamanho do `OutFrame`:**
   - Após `ProcessBinaryBuffer`, adicionar `Get Array Length` do `OutFrame.Data`
   - Comparar com `25` usando `Equal (Integer)`
   - Se não for igual a 25, **NÃO processar** e retornar

**Estrutura:**
```
ProcessBinaryBuffer
  ↓ ReturnValue == true
Get Array Length (OutFrame.Data)
  ↓ Length
Equal (Integer): Length == 25?
  ↓
Branch: Length == 25?
  ├─ True: CONTINUAR (ParseStateUpdateFrame)
  └─ False: PARAR (Print String: "Frame inválido - tamanho incorreto")
```

---

### **CORREÇÃO 3: Validação Rigorosa de `OutLocation` (Blueprint)**

**Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

**Adicionar APÓS `ParseStateUpdateFrame` e ANTES de `SpawnActorFromClass`:**

1. **Verificar se `OutLocation` não é `(0,0,0)`:**
   - Já deve estar implementado, mas verificar se está funcionando

2. **Verificar se `OutLocation.Z` está em range válido:**
   - Adicionar `Break Vector` para obter `OutLocation.Z`
   - Adicionar `Greater or Equal`: `OutLocation.Z >= -1000`
   - Adicionar `Less or Equal`: `OutLocation.Z <= 10000`
   - Combinar com `Boolean AND`
   - Se não passar, **NÃO spawnar**

**Estrutura:**
```
ParseStateUpdateFrame
  ↓ ReturnValue == true
Break Vector (OutLocation)
  ↓ Z
Greater or Equal: Z >= -1000?
  ↓ ReturnValue
Boolean AND
  ↓
Less or Equal: Z <= 10000?
  ↓ ReturnValue
Boolean AND
  ↓
Branch: Z válido?
  ├─ True: CONTINUAR (Array_Find, etc.)
  └─ False: PARAR (Print String: "OutLocation.Z inválido: {Z}")
```

---

### **CORREÇÃO 4: Adicionar Logs Detalhados para Diagnóstico**

**Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

**Adicionar logs em pontos críticos:**

1. **Após `ProcessBinaryBuffer`:**
   ```
   Format Text: "[ProcessNextFrame] ProcessBinaryBuffer retornou: {0}, OutFrame tamanho: {1}"
   - {0}: ReturnValue (Boolean convertido para String)
   - {1}: Get Array Length (OutFrame.Data)
   ```

2. **Após `ParseStateUpdateFrame`:**
   ```
   Format Text: "[ProcessNextFrame] Parse OK: {0}, OutPlayerId: {1}, OutLocation: ({2}, {3}, {4})"
   - {0}: ReturnValue (Boolean)
   - {1}: OutPlayerId
   - {2}: OutLocation.X
   - {3}: OutLocation.Y
   - {4}: OutLocation.Z
   ```

3. **Antes de `SpawnActorFromClass`:**
   ```
   Format Text: "[ProcessNextFrame] ANTES SpawnActorFromClass - OutPlayerId: {0}, OutLocation: ({1}, {2}, {3})"
   - {0}: OutPlayerId
   - {1}: OutLocation.X
   - {2}: OutLocation.Y
   - {3}: OutLocation.Z
   ```

4. **Após `Array_Find` (verificação dupla):**
   ```
   Format Text: "[ProcessNextFrame] Array_Find_Check - OutPlayerId: {0}, FoundIndexCheck: {1}"
   - {0}: OutPlayerId
   - {1}: FoundIndexCheck
   ```

---

## ✅ **CHECKLIST DE CORREÇÕES:**

### **C++ (`WSBinaryBPFL.cpp`):**
- [ ] Adicionar validação para rejeitar mensagens vazias (`NewData.Num() == 0 && Buffer.Num() == 0`)
- [ ] Adicionar log quando mensagem vazia for recebida

### **Blueprint (`ProcessNextFrame`):**
- [ ] Adicionar verificação de tamanho do `OutFrame` após `ProcessBinaryBuffer`
- [ ] Adicionar validação de `OutLocation.Z` (range válido)
- [ ] Adicionar logs detalhados em pontos críticos
- [ ] Verificar se validação `OutLocation != (0,0,0)` está funcionando

### **Verificações Gerais:**
- [ ] Verificar se verificação dupla (`Array_Find_Check`) está implementada
- [ ] Verificar se verificação antes de `Array_Add` está implementada
- [ ] Compilar e testar com 3 clients simultâneos
- [ ] Verificar logs para confirmar que mensagens vazias não estão sendo processadas

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Implementar correção C++ para mensagens vazias**
2. **Implementar validação de tamanho do `OutFrame` no Blueprint**
3. **Implementar validação rigorosa de `OutLocation.Z`**
4. **Adicionar logs detalhados para diagnóstico**
5. **Testar com 3 clients simultâneos**
6. **Analisar logs para identificar padrões**

---

## 📝 **NOTAS:**

- O log `size:0expected=25` é crítico e deve ser investigado primeiro
- O actor enterrado sugere que `OutLocation.Z` está incorreto ou `(0,0,0)` está sendo usado
- Múltiplos spawns podem ser causados por mensagens vazias sendo processadas como frames válidos
- A performance ruim pode ser resultado de múltiplos spawns e processamento excessivo

---

**Fim do Documento**

