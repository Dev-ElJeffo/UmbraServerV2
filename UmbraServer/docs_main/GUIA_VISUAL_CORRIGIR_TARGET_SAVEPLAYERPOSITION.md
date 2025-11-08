# 🎯 **GUIA VISUAL: Corrigir Target do SavePlayerPosition**

## 🔍 **PROBLEMA IDENTIFICADO NO XML:**

No XML fornecido, o `SavePlayerPosition` (`K2Node_CallFunction_2`) tem:
```
ErrorType=1
Target: K2Node_VariableSet_0 (Output_Get)
```

**PROBLEMA:** O `Output_Get` do `Set Variable` pode não funcionar corretamente, causando `ErrorType=1`.

---

## ✅ **SOLUÇÃO: Usar Variable Get Diretamente**

### **ESTRUTURA ATUAL (COM ERRO):**

```
K2Node_DynamicCast_0 (Cast To UmbraGameInstance)
  ↓
K2Node_VariableSet_0 (Set Variable: MyGameInstance)
  ↓
[Output_Get] ──────────┐
                        │
                        ↓
K2Node_CallFunction_2 (SavePlayerPosition)
  - Target: Output_Get (ERRO ❌)
```

### **ESTRUTURA CORRIGIDA:**

```
K2Node_DynamicCast_0 (Cast To UmbraGameInstance)
  ↓
K2Node_VariableSet_0 (Set Variable: MyGameInstance)
  ↓
K2Node_VariableGet_2 (Get Variable: MyGameInstance)
  ↓
K2Node_CallFunction_2 (SavePlayerPosition)
  - Target: MyGameInstance (DIRETO) ✅
```

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Localizar os Nós**

1. **Abrir `BP_Player:SavePositionTimer`**
2. **Localizar `K2Node_CallFunction_2` (SavePlayerPosition)**
   - NodeGuid: `7B9120E2476F5E7CAC625FB72173E536`
   - NodePosX: `4768`
   - NodePosY: `-48`
   - **Tem `ErrorType=1`** ❌

3. **Localizar `K2Node_VariableGet_2` (MyGameInstance)**
   - NodeGuid: `149E936E440855915BBFE681950AAFEA`
   - NodePosX: `4272`
   - NodePosY: `144`
   - **Este é o nó que devemos usar!** ✅

4. **Localizar `K2Node_VariableSet_0` (Set Variable: MyGameInstance)**
   - NodeGuid: `D75BF55B45C096B4E5EA87990D40DCC7`
   - NodePosX: `544`
   - NodePosY: `-16`
   - **Tem `Output_Get` conectado ao SavePlayerPosition** ❌

---

### **PASSO 2: Desconectar Output_Get**

1. **Selecionar `K2Node_CallFunction_2` (SavePlayerPosition)**
2. **Localizar o pin `Target` (self):**
   - PinId: `D10D8BD8443FE200C7E5759131083E43`
   - **Atualmente conectado a:** `K2Node_VariableSet_0` → `Output_Get` (PinId: `75001CE843920A57CC5C45BA5E92146D`)

3. **Desconectar:**
   - Clicar no pin `Target` do `SavePlayerPosition`
   - Arrastar para desconectar do `Output_Get` do `Set Variable`

---

### **PASSO 3: Conectar Variable Get**

1. **Selecionar `K2Node_VariableGet_2` (MyGameInstance)**
2. **Localizar o pin de saída (`MyGameInstance`):**
   - PinId: `BFE4447A446ED0187EF5C6830A591ADE`

3. **Conectar:**
   - Arrastar do pin `MyGameInstance` (saída) do `K2Node_VariableGet_2`
   - Conectar ao pin `Target` (self) do `K2Node_CallFunction_2` (SavePlayerPosition)

---

### **PASSO 4: Verificar Conexão**

**APÓS CONECTAR, VERIFICAR:**

1. **O pin `Target` do `SavePlayerPosition` deve estar conectado a:**
   - `K2Node_VariableGet_2` (MyGameInstance) ✅
   - **NÃO** ao `Output_Get` do `Set Variable` ❌

2. **O erro `ErrorType=1` deve desaparecer** ✅

3. **Compilar o Blueprint:**
   - Não deve haver erros de compilação
   - O `SavePlayerPosition` deve estar sem erros

---

## 📊 **ESTRUTURA FINAL CORRIGIDA:**

```
SavePositionTimer (Event)
  ↓
Get Game Instance
  ↓
Cast To UmbraGameInstance
  ↓ (Cast Success)
Set Variable: MyGameInstance
  ↓
Is Valid (MyGameInstance)?
  ↓ (True)
Get Active Player ID
  ↓
Greater (Integer): PlayerID > 0?
  ↓
Get Actor Location
  ↓
Not Equal (Vector): Location != (0,0,0)?
  ↓
Boolean AND: (PlayerID > 0) AND (Location != 0,0,0)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Print String: "SavePositionTimer - PlayerID: {0}, Position: X={1}, Y={2}, Z={3}"
  │    ↓
  │   Save Player Position
  │     - Target: MyGameInstance (de K2Node_VariableGet_2) ✅
  │     - PlayerID: Get Active Player ID
  │     - Position: Get Actor Location
  │     - CurrentZone: Current Zone
  │    ↓
  │   Print String: "SavePositionTimer - SavePlayerPosition chamado com sucesso"
  │
  └─ False:
       (nada)
```

---

## ⚠️ **IMPORTANTE:**

### **POR QUE NÃO USAR Output_Get?**

O `Output_Get` do `Set Variable` é uma saída de conveniência, mas:
- Pode não funcionar corretamente em todos os contextos
- Pode causar erros de compilação (`ErrorType=1`)
- Não é a forma recomendada de acessar variáveis

**A FORMA CORRETA é usar `Variable Get` diretamente.**

---

## 🧪 **TESTE APÓS CORREÇÃO:**

1. **Compilar o Blueprint**
2. **Verificar:** O erro `ErrorType=1` deve desaparecer
3. **Conectar um client**
4. **Mover o personagem** para uma posição conhecida
5. **Aguardar 6 segundos** (timer de 5s + margem)
6. **VERIFICAR NO BANCO:**
   ```sql
   SELECT pos_x, pos_y, pos_z FROM players WHERE id = [PlayerID];
   ```
7. **VERIFICAR:** Valores devem estar atualizados (não mais 0,0,0)

---

**Status:** 🚨 **PRIORIDADE MÁXIMA - IMPLEMENTAR AGORA**

