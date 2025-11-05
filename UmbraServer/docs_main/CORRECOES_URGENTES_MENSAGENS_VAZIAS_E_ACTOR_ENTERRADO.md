# 🔧 **CORREÇÕES URGENTES: Mensagens Vazias e Actor Enterrado**

## 📋 **PROBLEMAS CRÍTICOS IDENTIFICADOS:**

1. **⚠️ Mensagens Vazias (`size:0expected=25`):**
   - Aparece em TODOS os clients
   - Mensagens de 0 bytes estão sendo recebidas
   - Podem causar processamento de frames inválidos

2. **⚠️ Actor Enterrado no Chão:**
   - Client 3 mostra actor parcialmente enterrado
   - Localização Z provavelmente incorreta ou `(0,0,0)`

3. **⚠️ Múltiplos Spawns Persistem:**
   - Clients 1 e 2 veem um actor extra
   - Client 3 vê os outros corretamente + actor enterrado

---

## 🔧 **CORREÇÃO 1: Validar Mensagens Vazias no C++**

### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`

### **Localização:** Função `ProcessBinaryBuffer` (linha ~121)

### **Adicionar NO INÍCIO da função:**

```cpp
bool UWSBinaryBPFL::ProcessBinaryBuffer(TArray<uint8>& Buffer, const TArray<uint8>& NewData, FBinaryFrame& OutFrame, int32 ExpectedPlayerID)
{
    // NOVA VALIDAÇÃO: Rejeitar mensagens vazias
    if (NewData.Num() == 0)
    {
        // Se não há novos dados E o buffer também está vazio, retornar false imediatamente
        if (Buffer.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ProcessBinaryBuffer] Mensagem vazia recebida (NewData=0, Buffer=0) - ignorando"));
            return false;
        }
        // Se há dados no buffer mas não há novos dados, continuar processando o buffer
    }
    
    // Adicionar novos dados ao buffer
    Buffer.Append(NewData);
    
    // Constante: tamanho de um StateUpdateFrame = 25 bytes
    constexpr int32 FrameSize = 25;
    // ... resto do código continua igual
```

---

## 🔧 **CORREÇÃO 2: Validar Tamanho do Frame no Blueprint**

### **Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

### **Adicionar APÓS `ProcessBinaryBuffer` e ANTES de `ParseStateUpdateFrame`:**

#### **PASSO 1: Adicionar `Get Array Length`:**

1. **Após o nó `ProcessBinaryBuffer`:**
   - Clique direito → `Get Array Length`
   - Conecte `OutFrame.Data` ao pin `Array` do `Get Array Length`

#### **PASSO 2: Comparar com 25:**

1. **Adicione `Equal (Integer)`:**
   - Clique direito → `Equal (Integer)`
   - Conecte `Get Array Length` → `Length` ao pin `A` do `Equal`
   - Conecte `Make Integer (25)` ao pin `B` do `Equal`

#### **PASSO 3: Adicionar `Branch`:**

1. **Adicione `Branch`:**
   - Clique direito → `Branch`
   - Conecte `Equal` → `Return Value` ao pin `Condition` do `Branch`

#### **PASSO 4: Conectar Caminhos:**

1. **Pin `True` (tamanho == 25):**
   - Conecte a `ParseStateUpdateFrame` (fluxo normal)

2. **Pin `False` (tamanho != 25):**
   - Conecte a `Print String`:
     - Texto: `"Frame inválido - tamanho incorreto: {0} bytes"`
     - Variável {0}: `Get Array Length` → `Length`
   - **NÃO conecte nada após o `Print String`** (para o processamento)

### **Estrutura Visual:**

```
ProcessBinaryBuffer
  ↓ ReturnValue == true
Get Array Length (OutFrame.Data)
  ├─ Array ← OutFrame.Data
  └─ Length → Integer
  ↓ Length
Equal (Integer)
  ├─ A ← Get Array Length → Length
  └─ B ← Make Integer (25)
  ↓ Return Value
Branch
  ├─ Condition ← Equal → Return Value
  ├─ True → ParseStateUpdateFrame (continuar)
  └─ False → Print String ("Frame inválido - tamanho incorreto") → [PARAR]
```

---

## 🔧 **CORREÇÃO 3: Validação Rigorosa de OutLocation.Z**

### **Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

### **Adicionar APÓS `ParseStateUpdateFrame` e ANTES de `Array_Find`:**

#### **PASSO 1: Verificar OutLocation != (0,0,0):**

1. **Já deve estar implementado**, mas verifique se está funcionando

#### **PASSO 2: Verificar OutLocation.Z em Range Válido:**

1. **Adicione `Break Vector`:**
   - Clique direito → `Break Vector`
   - Conecte `ParseStateUpdateFrame` → `Out Location` ao pin `Vector` do `Break Vector`

2. **Adicione `Greater or Equal`:**
   - Clique direito → `Greater or Equal (Float)`
   - Conecte `Break Vector` → `Z` ao pin `A` do `Greater or Equal`
   - Conecte `Make Float (-1000)` ao pin `B` do `Greater or Equal`

3. **Adicione `Less or Equal`:**
   - Clique direito → `Less or Equal (Float)`
   - Conecte `Break Vector` → `Z` ao pin `A` do `Less or Equal`
   - Conecte `Make Float (10000)` ao pin `B` do `Less or Equal`

4. **Adicione `Boolean AND`:**
   - Clique direito → `Boolean AND`
   - Conecte `Greater or Equal` → `Return Value` ao pin `A` do `Boolean AND`
   - Conecte `Less or Equal` → `Return Value` ao pin `B` do `Boolean AND`

5. **Adicione `Branch`:**
   - Clique direito → `Branch`
   - Conecte `Boolean AND` → `Return Value` ao pin `Condition` do `Branch`

6. **Conecte Caminhos:**
   - Pin `True` (Z válido): Continue com `Array_Find` (fluxo normal)
   - Pin `False` (Z inválido): `Print String` → `"OutLocation.Z inválido: {0}"` → [PARAR]

### **Estrutura Visual:**

```
ParseStateUpdateFrame
  ↓ ReturnValue == true
Break Vector (OutLocation)
  ├─ Vector ← ParseStateUpdateFrame → Out Location
  └─ Z → Float
  ↓ Z
Greater or Equal (Float)
  ├─ A ← Break Vector → Z
  └─ B ← Make Float (-1000)
  ↓ Return Value
Boolean AND
  ├─ A ← Greater or Equal → Return Value
  └─ B ← Less or Equal → Return Value
  ↓ Return Value
Branch
  ├─ Condition ← Boolean AND → Return Value
  ├─ True → Array_Find (continuar)
  └─ False → Print String ("Z inválido") → [PARAR]
```

---

## 🔧 **CORREÇÃO 4: Adicionar Logs Detalhados**

### **Localização:** `BP_NetMovementClient` → `ProcessNextFrame`

### **Log 1: Após ProcessBinaryBuffer:**

```
Format Text: "[ProcessNextFrame] ProcessBinaryBuffer - ReturnValue: {0}, OutFrame tamanho: {1}"
- {0}: ReturnValue (Boolean) → To String (Boolean)
- {1}: Get Array Length (OutFrame.Data) → Length
→ Print String
```

### **Log 2: Após ParseStateUpdateFrame:**

```
Format Text: "[ProcessNextFrame] Parse OK: {0}, PlayerID: {1}, Location: ({2}, {3}, {4})"
- {0}: ReturnValue (Boolean) → To String (Boolean)
- {1}: OutPlayerId
- {2}: OutLocation.X
- {3}: OutLocation.Y
- {4}: OutLocation.Z
→ Print String
```

### **Log 3: Antes de SpawnActorFromClass:**

```
Format Text: "[ProcessNextFrame] ANTES SpawnActorFromClass - PlayerID: {0}, Location: ({1}, {2}, {3})"
- {0}: OutPlayerId
- {1}: OutLocation.X
- {2}: OutLocation.Y
- {3}: OutLocation.Z
→ Print String
```

### **Log 4: Após Array_Find_Check:**

```
Format Text: "[ProcessNextFrame] Array_Find_Check - PlayerID: {0}, FoundIndex: {1}"
- {0}: OutPlayerId
- {1}: FoundIndexCheck
→ Print String
```

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

### **C++ (`WSBinaryBPFL.cpp`):**
- [ ] Adicionar validação para `NewData.Num() == 0 && Buffer.Num() == 0`
- [ ] Adicionar log quando mensagem vazia for recebida

### **Blueprint (`ProcessNextFrame`):**
- [ ] Adicionar `Get Array Length` após `ProcessBinaryBuffer`
- [ ] Adicionar `Equal (Integer)` comparando com 25
- [ ] Adicionar `Branch` para verificar tamanho
- [ ] Adicionar validação de `OutLocation.Z` (range -1000 a 10000)
- [ ] Adicionar logs detalhados em pontos críticos
- [ ] Verificar se validação `OutLocation != (0,0,0)` está funcionando

### **Testes:**
- [ ] Compilar código C++
- [ ] Compilar Blueprint
- [ ] Testar com 3 clients simultâneos
- [ ] Verificar logs para confirmar que mensagens vazias não são processadas
- [ ] Verificar se actor enterrado não aparece mais
- [ ] Verificar se múltiplos spawns foram resolvidos

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Implementar correção C++ para mensagens vazias**
2. **Implementar validação de tamanho do frame no Blueprint**
3. **Implementar validação rigorosa de `OutLocation.Z`**
4. **Adicionar logs detalhados**
5. **Testar e analisar resultados**

---

## 📝 **NOTAS:**

- O log `size:0expected=25` é crítico e deve ser investigado primeiro
- Mensagens vazias podem estar causando frames inválidos sendo processados
- O actor enterrado sugere que `OutLocation.Z` está incorreto ou `(0,0,0)` está sendo usado
- Validações rigorosas devem prevenir spawns incorretos

---

**Fim do Documento**

