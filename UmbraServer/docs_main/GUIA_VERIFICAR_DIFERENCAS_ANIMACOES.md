# 🔧 **GUIA PRÁTICO: Verificar Diferenças Entre Versão SEM e COM Animações**

## 📋 **OBJETIVO:**

Identificar exatamente o que está diferente na versão COM animações que está causando a sobreposição de atores.

---

## ✅ **VERIFICAÇÃO PASSO A PASSO:**

### **PASSO 1: Verificar Estrutura do ProcessNextFrame**

**No Blueprint Editor:**

1. **Abra `BP_NetMovementClient` → `ProcessNextFrame`**

2. **Localize o nó `ParseStateUpdateFrameWithAnimation`**
   - Deve estar **ANTES** de `ParseStateUpdateFrame`
   - Se não estiver → **PROBLEMA:** Mova-o para antes

3. **Localize o `Branch` após `ParseStateUpdateFrameWithAnimation`**
   - **Condition:** Deve estar conectado ao `ReturnValue` do `ParseStateUpdateFrameWithAnimation`
   - **True:** Deve conectar ao caminho COM animação
   - **False:** Deve conectar ao `ParseStateUpdateFrame` (fallback)

---

### **PASSO 2: Verificar Conexões do Branch (CRÍTICO)**

**⚠️ ESTE É O PONTO MAIS CRÍTICO:**

#### **2.1. Verificar Pin `True` (Frame com Animação):**

**O pin `True` do `Branch` deve conectar a:**

1. **Filtro:** `OutPlayerId != Active Player ID`
2. **Array_Find:** `RemoteActorIds`, `OutPlayerId`
3. **Branch:** `FoundIndex >= 0?`
4. **Lógica de spawn/atualização**

**❌ PROBLEMA SE:**
- O pin `True` conectar diretamente ao mesmo ponto que o pin `False`
- Ou conectar a um ponto que também recebe conexão do pin `False`

**✅ CORRETO SE:**
- O pin `True` conecta a um caminho **SEPARADO** que processa COM animação
- Este caminho **NÃO** recebe conexão do pin `False`

---

#### **2.2. Verificar Pin `False` (Fallback):**

**O pin `False` do `Branch` deve conectar a:**

1. **`ParseStateUpdateFrame`** (fallback para frame antigo)
2. **NÃO deve conectar diretamente à lógica de spawn/atualização**

**❌ PROBLEMA SE:**
- O pin `False` conectar diretamente à lógica de spawn/atualização
- Ou conectar ao mesmo ponto que o pin `True`

**✅ CORRETO SE:**
- O pin `False` conecta **APENAS** ao `ParseStateUpdateFrame`
- O `ParseStateUpdateFrame` tem seu próprio `Branch` que conecta à lógica de spawn/atualização

---

### **PASSO 3: Verificar Ordem de Execução no Caminho True**

**No caminho `True` (com animação), após obter `RemoteActorRef`:**

**A ordem DEVE ser:**

```
Set Actor Location
  ↓
Set Actor Rotation
  ↓
Cast to Character
  ↓
Get Character Movement
  ↓
Set Velocity
```

**❌ PROBLEMA SE:**
- `Set Velocity` estiver ANTES de `Set Actor Location`
- `Set Actor Rotation` estiver ANTES de `Set Actor Location`
- Qualquer outra ordem diferente da acima

**✅ CORRETO SE:**
- A ordem está exatamente como mostrado acima

---

### **PASSO 4: Verificar Validação do Cast to Character**

**Após `Cast to Character`:**

**Deve haver um `Branch` verificando se o cast foi bem-sucedido:**

```
Cast to Character
  ↓
Branch (Is Valid?)
  ├─ True: Continuar com Get Character Movement → Set Velocity
  └─ False: Ignorar (não aplicar velocidade, mas continuar com Location/Rotation)
```

**❌ PROBLEMA SE:**
- Não há validação após `Cast to Character`
- O cast pode falhar silenciosamente
- A lógica continua mesmo se o cast falhar

**✅ CORRETO SE:**
- Há um `Branch` verificando se o cast foi bem-sucedido
- Se falhar, continua sem aplicar velocidade (compatibilidade)

---

### **PASSO 5: Verificar se Há Processamento Duplicado**

**Verifique se ambos os caminhos (True e False) estão processando o mesmo frame:**

**❌ PROBLEMA SE:**
- O mesmo `OutPlayerId` está sendo processado duas vezes:
  - Uma vez no caminho `True` (com animação)
  - Outra vez no caminho `False` (sem animação)
- Isso causaria spawn/atualização duplicada

**✅ CORRETO SE:**
- Apenas UM caminho processa cada frame
- Se `ParseStateUpdateFrameWithAnimation` retornar `True`, apenas o caminho `True` executa
- Se retornar `False`, apenas o caminho `False` executa

---

## 🎯 **CHECKLIST COMPLETO:**

- [ ] `ParseStateUpdateFrameWithAnimation` está ANTES de `ParseStateUpdateFrame`
- [ ] `Branch` após `ParseStateUpdateFrameWithAnimation` tem `Condition` conectado ao `ReturnValue`
- [ ] Pin `True` do `Branch` conecta a um caminho SEPARADO (com animação)
- [ ] Pin `False` do `Branch` conecta APENAS ao `ParseStateUpdateFrame` (fallback)
- [ ] Pin `True` NÃO conecta ao mesmo ponto que o pin `False`
- [ ] Ordem de execução no caminho `True`: Location → Rotation → Velocity
- [ ] Há validação após `Cast to Character` (Branch verificando sucesso)
- [ ] Apenas UM caminho processa cada frame (não há duplicação)

---

## 🔧 **CORREÇÕES COMUNS:**

### **Correção 1: Pin False Conectado Incorretamente**

**Se o pin `False` estiver conectado diretamente à lógica de spawn/atualização:**

1. **Desconecte** o pin `False` da lógica de spawn/atualização
2. **Conecte** o pin `False` ao `ParseStateUpdateFrame`
3. **Conecte** o `ReturnValue` do `ParseStateUpdateFrame` a um novo `Branch`
4. **Conecte** o pin `True` deste novo `Branch` à lógica de spawn/atualização (sem animação)

### **Correção 2: Ordem de Execução Incorreta**

**Se `Set Velocity` estiver antes de `Set Actor Location`:**

1. **Reordene** os nós para: Location → Rotation → Velocity
2. **Conecte** `Set Actor Location` → `then` → `Set Actor Rotation` → `then` → `Cast to Character` → `then` → `Set Velocity`

### **Correção 3: Falta de Validação do Cast**

**Se não houver validação após `Cast to Character`:**

1. **Adicione** um `Branch` após `Cast to Character`
2. **Conecte** `Is Valid` (do `Cast`) ao `Condition` do `Branch`
3. **Conecte** o pin `True` ao `Get Character Movement` → `Set Velocity`
4. **Conecte** o pin `False` ao próximo passo (ou deixe desconectado se não houver mais nada)

---

## 📊 **RESULTADO ESPERADO:**

Após verificar e corrigir:

- ✅ Apenas UM caminho processa cada frame
- ✅ A ordem de execução está correta
- ✅ Há validação adequada após `Cast to Character`
- ✅ Não há processamento duplicado
- ✅ A lógica de spawn/atualização é idêntica em ambos os caminhos (exceto pela aplicação de animação)

---

**Verifique cada item do checklist e corrija os problemas encontrados!**

