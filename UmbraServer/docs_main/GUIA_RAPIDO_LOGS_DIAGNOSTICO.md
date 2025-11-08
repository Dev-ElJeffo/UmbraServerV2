# ⚡ **GUIA RÁPIDO: Logs de Diagnóstico Essenciais**

## 🎯 **OBJETIVO:**

Adicionar logs mínimos mas essenciais para identificar **exatamente onde** a execução está parando.

---

## ✅ **LOGS ESSENCIAIS (5 MINUTOS):**

### **1. Log no Início da Função**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**APÓS `SavePositionTimer` (Function Entry):**
```
Print String: "🔵 [SavePositionTimer] INÍCIO"
```

**OBJETIVO:** Verificar se a função está sendo chamada pelo timer.

---

### **2. Log Após Cast (Success e Failed)**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**APÓS `Cast To UmbraGameInstance`:**
- **Path `Success` (then):**
  ```
  Print String: "✅ [SavePositionTimer] Cast OK"
  ```
- **Path `Cast Failed`:**
  ```
  Print String: "❌ [SavePositionTimer] Cast FALHOU"
  ```

**OBJETIVO:** Verificar se o cast está funcionando.

---

### **3. Log no Branch Is Valid**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**APÓS `Is Valid (My Game Instance)`:**
- **Path `True` (then):**
  ```
  Print String: "✅ [SavePositionTimer] MyGameInstance VÁLIDO"
  ```
- **Path `False` (else):**
  ```
  Print String: "❌ [SavePositionTimer] MyGameInstance INVÁLIDO"
  ```

**OBJETIVO:** Verificar se `My Game Instance` é válido.

---

### **4. Logs nas Validações (Simplificados)**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**APÓS `Get Active Player ID`:**
```
Format Text: "🔵 [SavePositionTimer] PlayerID: {0}"
  - {0}: ReturnValue do Get Active Player ID
Print String
```

**APÓS `Boolean AND`:**
```
Format Text: "🔵 [SavePositionTimer] Validações OK? {0}"
  - {0}: ReturnValue do Boolean AND (usar To String (Boolean))
Print String
```

**OBJETIVO:** Verificar valores das validações.

---

### **5. Log no Segundo Branch**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**APÓS o segundo `Branch` (Boolean AND):**
- **Path `True` (then):**
  ```
  Print String: "✅ [SavePositionTimer] CHAMANDO SavePlayerPosition..."
  ```
- **Path `False` (else):**
  ```
  Print String: "❌ [SavePositionTimer] Validações FALHARAM"
  ```

**OBJETIVO:** Verificar se o segundo Branch está permitindo a execução.

---

### **6. Log Antes e Depois de SavePlayerPosition**

**LOCALIZAÇÃO:** `BP_Player:SavePositionTimer`

**ANTES de `Save Player Position`:**
```
Print String: "🔵 [SavePositionTimer] EXECUTANDO SavePlayerPosition AGORA"
```

**DEPOIS de `Save Player Position` (no pin `then`):**
```
Print String: "✅ [SavePositionTimer] SavePlayerPosition EXECUTADO"
```

**OBJETIVO:** Confirmar se `SavePlayerPosition` está sendo chamado.

---

## 📊 **ESTRUTURA SIMPLIFICADA COM LOGS:**

```
SavePositionTimer (Event)
  ↓
Print String: "🔵 [SavePositionTimer] INÍCIO"
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ├─ Success:
  │    ↓
  │   Print String: "✅ [SavePositionTimer] Cast OK"
  │    ↓
  │   Set My Game Instance
  │    ↓
  │   Is Valid (My Game Instance)?
  │    ├─ True:
  │    │    ↓
  │    │   Print String: "✅ [SavePositionTimer] MyGameInstance VÁLIDO"
  │    │    ↓
  │    │   Get Active Player ID
  │    │    ↓
  │    │   Format Text: "🔵 [SavePositionTimer] PlayerID: {0}"
  │    │    ↓
  │    │   Print String
  │    │    ↓
  │    │   Greater (Integer): PlayerID > 0?
  │    │    ↓
  │    │   Get Actor Location
  │    │    ↓
  │    │   Not Equal (Vector): Location != (0,0,0)?
  │    │    ↓
  │    │   Boolean AND: (PlayerID > 0) AND (Location != 0,0,0)
  │    │    ↓
  │    │   Format Text: "🔵 [SavePositionTimer] Validações OK? {0}"
  │    │    ↓
  │    │   Print String
  │    │    ↓
  │    │   Branch
  │    │    ├─ True:
  │    │    │    ↓
  │    │    │   Print String: "✅ [SavePositionTimer] CHAMANDO SavePlayerPosition..."
  │    │    │    ↓
  │    │    │   Format Text: "SavePositionTimer - PlayerID: {0}, Position: X={1}, Y={2}, Z={3}"
  │    │    │    ↓
  │    │    │   Print String
  │    │    │    ↓
  │    │    │   Print String: "🔵 [SavePositionTimer] EXECUTANDO SavePlayerPosition AGORA"
  │    │    │    ↓
  │    │    │   Save Player Position
  │    │    │    ↓
  │    │    │   Print String: "✅ [SavePositionTimer] SavePlayerPosition EXECUTADO"
  │    │    │
  │    │    └─ False:
  │    │         ↓
  │    │        Print String: "❌ [SavePositionTimer] Validações FALHARAM"
  │    │
  │    └─ False:
  │         ↓
  │        Print String: "❌ [SavePositionTimer] MyGameInstance INVÁLIDO"
  │
  └─ Cast Failed:
       ↓
      Print String: "❌ [SavePositionTimer] Cast FALHOU"
```

---

## 🧪 **TESTE RÁPIDO:**

1. **Adicionar os 6 logs essenciais** (5 minutos)
2. **Compilar o Blueprint**
3. **Conectar um client**
4. **Mover o personagem** para uma posição conhecida
5. **Aguardar 6 segundos**
6. **VERIFICAR LOGS:**
   - **Se aparece "INÍCIO"** → Timer está funcionando ✅
   - **Se aparece "Cast OK"** → Cast está funcionando ✅
   - **Se aparece "MyGameInstance VÁLIDO"** → Variável está OK ✅
   - **Se aparece "PlayerID: {ID}"** → PlayerID está sendo obtido ✅
   - **Se aparece "Validações OK? true"** → Validações passaram ✅
   - **Se aparece "CHAMANDO SavePlayerPosition..."** → Está chegando no ponto certo ✅
   - **Se aparece "EXECUTANDO SavePlayerPosition AGORA"** → Está prestes a chamar ✅
   - **Se aparece "SavePlayerPosition EXECUTADO"** → Foi chamado com sucesso ✅

7. **IDENTIFICAR ONDE PARA:**
   - **Último log que aparece** = ponto onde está parando
   - **Próximo log que deveria aparecer** = onde está o problema

---

## 🔧 **CORREÇÕES BASEADAS NO ÚLTIMO LOG:**

### **SE NÃO APARECE "INÍCIO":**
- **CAUSA:** Timer não está executando
- **SOLUÇÃO:** Verificar se o timer está sendo iniciado no `BeginPlay`

### **SE PARA EM "Cast FALHOU":**
- **CAUSA:** `Get Game Instance` não retorna `UmbraGameInstance`
- **SOLUÇÃO:** Verificar configuração do Game Instance no projeto

### **SE PARA EM "MyGameInstance INVÁLIDO":**
- **CAUSA:** Variável não está sendo setada
- **SOLUÇÃO:** Verificar se `Set My Game Instance` está sendo executado

### **SE PARA EM "PlayerID: 0":**
- **CAUSA:** `Get Active Player ID` retorna 0
- **SOLUÇÃO:** Verificar se o personagem foi selecionado

### **SE PARA EM "Validações OK? false":**
- **CAUSA:** PlayerID <= 0 ou Location == (0,0,0)
- **SOLUÇÃO:** Verificar valores específicos nos logs anteriores

### **SE PARA EM "Validações FALHARAM":**
- **CAUSA:** Boolean AND retornou false
- **SOLUÇÃO:** Verificar PlayerID e Location nos logs anteriores

### **SE PARA EM "CHAMANDO SavePlayerPosition..." MAS NÃO APARECE "EXECUTANDO":**
- **CAUSA:** Conexão entre logs está quebrada
- **SOLUÇÃO:** Verificar conexões no Blueprint

### **SE APARECE "EXECUTANDO SavePlayerPosition AGORA" MAS NÃO APARECE "EXECUTADO":**
- **CAUSA:** `SavePlayerPosition` está travando ou dando erro
- **SOLUÇÃO:** Verificar logs do C++ e PHP API

---

## 📋 **CHECKLIST RÁPIDO:**

- [ ] Log no início: "🔵 [SavePositionTimer] INÍCIO"
- [ ] Log após Cast (Success e Failed)
- [ ] Log no Branch Is Valid (True e False)
- [ ] Log após Get Active Player ID: "🔵 [SavePositionTimer] PlayerID: {0}"
- [ ] Log após Boolean AND: "🔵 [SavePositionTimer] Validações OK? {0}"
- [ ] Log no segundo Branch (True e False)
- [ ] Log antes de SavePlayerPosition: "🔵 [SavePositionTimer] EXECUTANDO SavePlayerPosition AGORA"
- [ ] Log depois de SavePlayerPosition: "✅ [SavePositionTimer] SavePlayerPosition EXECUTADO"
- [ ] Compilar e testar
- [ ] Identificar último log que aparece

---

**Status:** ⚡ **ADICIONAR LOGS ESSENCIAIS AGORA (5 MINUTOS)**

