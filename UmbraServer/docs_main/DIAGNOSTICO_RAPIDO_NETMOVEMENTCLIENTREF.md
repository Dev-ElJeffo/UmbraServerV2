# ⚡ **DIAGNÓSTICO RÁPIDO: NetMovementClientRef Inválido**

## 🎯 **PROBLEMA:**

Ao pressionar F9, `NetMovementClientRef` está inválido.

---

## ✅ **VERIFICAÇÕES RÁPIDAS (5 MINUTOS):**

### **1. Verificar se BP_NetMovementClient Está no Level**

1. **Abra o level** (ex: `Lvl_TestAuth`)
2. **No World Outliner**, procure por `BP_NetMovementClient`
3. **Se NÃO estiver:**
   - **Content Browser** → Encontre `BP_NetMovementClient`
   - **Arraste** para o level
   - **Salve** o level (Ctrl+S)

---

### **2. Verificar se a Variável Existe no Character**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Painel My Blueprint** → **Variables**
3. **Procure por `NetMovementClientRef`**
4. **Se NÃO existir:**
   - **+ Variable**
   - **Nome:** `NetMovementClientRef`
   - **Tipo:** `BP Net Movement Client` (Object Reference)
   - **Instance Editable:** `True`
   - **Compile**

---

### **3. Adicionar Logs de Diagnóstico no BP_NetMovementClient**

**No `BP_NetMovementClient.BeginPlay`, adicione:**

```
[Event BeginPlay]
  ↓
[Print String: "🔵 [BP_NetMovementClient] BeginPlay INICIADO"]
  ↓
[Delay: 1.0]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Pawn]
  ↓
[Cast to BP_ThirdPersonCharacter]
  ├─ Success:
  │   ├─ [Print String: "✅ Cast BEM-SUCEDIDO"]
  │   ├─ [Set Variable: NetMovementClientRef]
  │   │   ├─ Target: [As BP Third Person Character]
  │   │   └─ Value: [Self]
  │   └─ [Print String: "✅ Referência DEFINIDA"]
  └─ Failed:
      └─ [Print String: "❌ Cast FALHOU"]
```

**Execute e verifique os logs. Se você NÃO ver "Referência DEFINIDA", o problema está no `BP_NetMovementClient`.**

---

### **4. Verificar se o Cast Está Funcionando**

**Se o Cast estiver falhando, adicione este log:**

```
[Get Pawn]
  ↓
[Get Class Name] (do Pawn)
  ↓
[Print String: "🔍 Tipo do Pawn: [Get Class Name]"]
```

**Isso vai mostrar o tipo real do Pawn. Se não for `BP_ThirdPersonCharacter`, o Cast vai falhar.**

---

## 🔧 **SOLUÇÃO RÁPIDA: Usar GetAllActorsOfClass**

**Se o Cast não funcionar, use esta solução alternativa:**

**No `BP_NetMovementClient.BeginPlay`:**

```
[Event BeginPlay]
  ↓
[Delay: 1.0]
  ↓
[Get All Actors of Class: BP_ThirdPersonCharacter]
  ↓
[Get Array Length]
  ↓
[Branch: Length > 0?]
  ├─ True:
  │   ├─ [Get Array Item] (Index: 0)
  │   ├─ [Set Variable: NetMovementClientRef]
  │   │   ├─ Target: [Get Array Item]
  │   │   └─ Value: [Self]
  │   └─ [Print String: "✅ Referência definida via GetAllActorsOfClass"]
  └─ False:
      └─ [Print String: "❌ Nenhum Character encontrado"]
```

---

## 🧪 **TESTE RÁPIDO:**

**Execute o jogo e verifique os logs na ordem:**

1. **`🔵 [BP_NetMovementClient] BeginPlay INICIADO`** - Confirma que o BeginPlay está executando
2. **`✅ Cast BEM-SUCEDIDO`** - Confirma que o Cast funcionou
3. **`✅ Referência DEFINIDA`** - Confirma que o Set Variable foi executado

**Se algum desses logs NÃO aparecer, o problema está nesse ponto específico.**

---

## ⚠️ **PROBLEMA MAIS COMUM:**

**O `BP_NetMovementClient` não está no level, ou o Cast está falhando.**

**Solução:**
1. Adicione o `BP_NetMovementClient` ao level
2. Ou use `GetAllActorsOfClass` em vez de Cast

---

**✅ Após essas verificações, você vai descobrir o problema!**

