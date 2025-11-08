# 🔧 **CORREÇÃO: BeginPlay e Timer SavePosition**

## ❌ **ERRO COMUM:**

**NÃO deve chamar `SavePlayerPosition` diretamente no `BeginPlay`!**

O `BeginPlay` deve apenas:
1. Inicializar `MyGameInstance`
2. **Iniciar o timer** que vai chamar `SavePositionTimer` periodicamente

---

## ✅ **FLUXO CORRETO:**

### **1. BeginPlay (Inicialização)**

**LOCALIZAÇÃO:** `BP_Player:EventGraph - BeginPlay`

**O QUE DEVE FAZER:**
```
BeginPlay
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ↓ (Cast Success)
Set Variable: MyGameInstance
  ↓
Set Timer by Function Name
  - Function Name: "SavePositionTimer"
  - Time: 5.0 (segundos)
  - Looping: true
  - Initial Start Delay: 0.0
```

**O QUE NÃO DEVE FAZER:**
- ❌ Chamar `SavePlayerPosition` diretamente
- ❌ Chamar `SavePositionTimer` diretamente
- ❌ Fazer validações ou lógica de salvamento

---

### **2. SavePositionTimer (Função do Timer)**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer` (Custom Function)

**O QUE DEVE FAZER:**
```
SavePositionTimer (chamada pelo timer a cada 5 segundos)
  ↓
Validações (MyGameInstance válido, PlayerID > 0, Location != 0,0,0)
  ↓
Save Player Position (chamada apenas se validações passarem)
```

**QUANDO É CHAMADA:**
- Automaticamente pelo timer a cada 5 segundos
- **NÃO** deve ser chamada manualmente no `BeginPlay`

---

## 🔍 **VERIFICAÇÃO: Timer Está Sendo Iniciado?**

### **PROBLEMA COMUM:**

O timer pode não estar sendo iniciado corretamente no `BeginPlay`.

### **COMO VERIFICAR:**

**NO `BP_Player:EventGraph - BeginPlay`:**

1. **Verificar se existe `Set Timer by Function Name`:**
   - Function Name: `"SavePositionTimer"`
   - Time: `5.0` (ou outro valor)
   - Looping: `true`
   - Initial Start Delay: `0.0` (ou `5.0` para começar após 5 segundos)

2. **Verificar se está conectado ao fluxo:**
   - Deve estar conectado após `Set Variable: MyGameInstance`
   - Deve estar no path `Success` do `Cast To UmbraGameInstance`

3. **Adicionar log para verificar:**
   ```
   Print String: "🔵 [BeginPlay] Timer SavePositionTimer iniciado"
   ```
   - Após `Set Timer by Function Name`

---

## ✅ **ESTRUTURA CORRETA DO BeginPlay:**

```
BeginPlay (Event)
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ├─ Success:
  │    ↓
  │   Set Variable: MyGameInstance
  │    ↓
  │   Print String: "🔵 [BeginPlay] MyGameInstance setado"
  │    ↓
  │   Set Timer by Function Name
  │     - Function Name: "SavePositionTimer"
  │     - Time: 5.0
  │     - Looping: true
  │     - Initial Start Delay: 0.0
  │    ↓
  │   Print String: "🔵 [BeginPlay] Timer SavePositionTimer iniciado"
  │
  └─ Cast Failed:
       ↓
      Print String: "❌ [BeginPlay] Falha ao fazer cast para UmbraGameInstance"
```

---

## 🔧 **CORREÇÃO PASSO A PASSO:**

### **PASSO 1: Verificar BeginPlay**

1. **Abrir `BP_Player:EventGraph - BeginPlay`**
2. **Verificar se existe `Set Timer by Function Name`:**
   - Se **NÃO existir:** Adicionar
   - Se **existir:** Verificar configuração

### **PASSO 2: Remover Chamada Direta (Se Existir)**

1. **Verificar se há chamada direta a `SavePlayerPosition` no `BeginPlay`:**
   - Se **existir:** Remover completamente
   - Se **existir chamada a `SavePositionTimer`:** Remover também

2. **O `BeginPlay` deve apenas:**
   - Inicializar `MyGameInstance`
   - Iniciar o timer

### **PASSO 3: Configurar Timer Corretamente**

1. **Adicionar `Set Timer by Function Name`** (se não existir)
2. **Configurar:**
   - Function Name: `"SavePositionTimer"` (exatamente como o nome da função)
   - Time: `5.0` (segundos)
   - Looping: `true` (para repetir a cada 5 segundos)
   - Initial Start Delay: `0.0` (ou `5.0` para começar após 5 segundos)

3. **Conectar ao fluxo:**
   - Após `Set Variable: MyGameInstance`
   - No path `Success` do `Cast To UmbraGameInstance`

### **PASSO 4: Adicionar Logs de Verificação**

1. **Após `Set Variable: MyGameInstance`:**
   ```
   Print String: "🔵 [BeginPlay] MyGameInstance setado"
   ```

2. **Após `Set Timer by Function Name`:**
   ```
   Print String: "🔵 [BeginPlay] Timer SavePositionTimer iniciado"
   ```

---

## 🧪 **TESTE:**

### **TESTE 1: Verificar se Timer Está Sendo Iniciado**

1. **Compilar o Blueprint**
2. **Conectar um client**
3. **VERIFICAR LOGS:**
   - Deve aparecer: "🔵 [BeginPlay] MyGameInstance setado"
   - Deve aparecer: "🔵 [BeginPlay] Timer SavePositionTimer iniciado"

**SE NÃO APARECER:**
- Timer não está sendo iniciado
- Verificar conexões no `BeginPlay`

---

### **TESTE 2: Verificar se SavePositionTimer Está Sendo Chamado**

1. **Aguardar 5 segundos após conectar**
2. **VERIFICAR LOGS:**
   - Deve aparecer: "🔵 [SavePositionTimer] INÍCIO" (a cada 5 segundos)

**SE NÃO APARECER:**
- Timer não está executando
- Verificar se o nome da função está correto no `Set Timer by Function Name`
- Verificar se `Looping` está como `true`

---

### **TESTE 3: Verificar se SavePlayerPosition Está Sendo Chamado**

1. **Mover o personagem** para uma posição conhecida
2. **Aguardar 6 segundos** (timer de 5s + margem)
3. **VERIFICAR LOGS:**
   - Deve aparecer: "🔵 [SavePositionTimer] INÍCIO"
   - Deve aparecer: "✅ [SavePositionTimer] Cast OK"
   - Deve aparecer: "✅ [SavePositionTimer] MyGameInstance VÁLIDO"
   - Deve aparecer: "🔵 [SavePositionTimer] PlayerID: {ID}"
   - Deve aparecer: "🔵 [SavePositionTimer] Validações OK? true"
   - Deve aparecer: "✅ [SavePositionTimer] CHAMANDO SavePlayerPosition..."
   - Deve aparecer: "🔵 [SavePositionTimer] EXECUTANDO SavePlayerPosition AGORA"
   - Deve aparecer: "✅ [SavePositionTimer] SavePlayerPosition EXECUTADO"

---

## 📋 **CHECKLIST DE CORREÇÃO:**

### **BeginPlay:**

- [ ] Remover qualquer chamada direta a `SavePlayerPosition`
- [ ] Remover qualquer chamada direta a `SavePositionTimer`
- [ ] Verificar se `Set Timer by Function Name` existe
- [ ] Verificar se Function Name está como `"SavePositionTimer"` (exatamente)
- [ ] Verificar se Time está como `5.0`
- [ ] Verificar se Looping está como `true`
- [ ] Verificar se está conectado após `Set Variable: MyGameInstance`
- [ ] Adicionar log: "🔵 [BeginPlay] Timer SavePositionTimer iniciado"

### **SavePositionTimer (Função):**

- [ ] Verificar se a função existe
- [ ] Verificar se o nome está exatamente como `SavePositionTimer`
- [ ] Verificar se tem todas as validações
- [ ] Verificar se chama `SavePlayerPosition` no path correto
- [ ] Adicionar logs de diagnóstico (ver `GUIA_RAPIDO_LOGS_DIAGNOSTICO.md`)

---

## ⚠️ **PROBLEMAS COMUNS:**

### **PROBLEMA 1: Nome da Função Incorreto**

**ERRO:**
- `Set Timer by Function Name` → Function Name: `"SavePosition"` ❌
- Ou: `"Save Position Timer"` ❌
- Ou: `"SavePositionTimerFunction"` ❌

**CORRETO:**
- `Set Timer by Function Name` → Function Name: `"SavePositionTimer"` ✅
- **DEVE SER EXATAMENTE** o nome da função Custom Function

---

### **PROBLEMA 2: Timer Não Está Looping**

**ERRO:**
- `Set Timer by Function Name` → Looping: `false` ❌

**CORRETO:**
- `Set Timer by Function Name` → Looping: `true` ✅

---

### **PROBLEMA 3: Timer Não Está Conectado**

**ERRO:**
- `Set Timer by Function Name` não está conectado ao fluxo ❌
- Está no path `Cast Failed` ❌

**CORRETO:**
- `Set Timer by Function Name` está conectado após `Set Variable: MyGameInstance` ✅
- Está no path `Success` do `Cast To UmbraGameInstance` ✅

---

### **PROBLEMA 4: Função Não Existe**

**ERRO:**
- A função `SavePositionTimer` não existe como Custom Function ❌
- O nome está diferente ❌

**CORRETO:**
- A função `SavePositionTimer` existe como Custom Function ✅
- O nome está exatamente como no `Set Timer by Function Name` ✅

---

## 📊 **RESUMO:**

1. **`BeginPlay`** → Apenas inicializa `MyGameInstance` e inicia o timer
2. **`SavePositionTimer`** → Função chamada pelo timer a cada 5 segundos
3. **`SavePlayerPosition`** → Chamada dentro de `SavePositionTimer` após validações

**NÃO chamar `SavePlayerPosition` diretamente no `BeginPlay`!**

---

**Status:** 🔧 **CORRIGIR BeginPlay E VERIFICAR TIMER**

