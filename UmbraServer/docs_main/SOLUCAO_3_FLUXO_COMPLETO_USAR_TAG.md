# 🔧 **SOLUÇÃO 3: Usar Tag para Identificar Cliente Local - Fluxo Completo**

## 🎯 **OBJETIVO:**

Usar uma Tag única no `BP_NetMovementClient` para identificá-lo como cliente local.

---

## 📋 **PASSO 1: BP_NetMovementClient - Definir Tag no BeginPlay**

### **1.1: No Event Graph do `BP_NetMovementClient`**

**No `BeginPlay`, após a conexão WebSocket:**

```
[Event BeginPlay]
  ↓
[... código existente de conexão WebSocket ...]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Player ID]
  ↓
[Format Text]
  ├─ Format: "LocalClient_{0}"
  ├─ {0}: Player ID
  └─ Result: (String)
      ↓
[Set Actor Tag]
  ├─ Tag: Result (do Format Text)
  └─ then
      ↓
[Print String: "✅ [BP_NetMovementClient] Tag definida: {Tag}"]
```

### **1.2: Detalhamento dos Nós**

**Nó 1: Event BeginPlay**
- **Já existe** no `BP_NetMovementClient`

**Nó 2: Get Player Controller**
- **Localização:** Após código existente
- **Ação:** Clique direito → **Get Player Controller**
- **Player Index:** `0`

**Nó 3: Get Player ID**
- **Localização:** Após Get Player Controller
- **Ação:**
  - Arraste o pin de saída do **Get Player Controller**
  - Solte no Event Graph → **Get Player ID**
- **Target:** Conecte ao pin de saída do **Get Player Controller**

**Nó 4: Format Text**
- **Localização:** Após Get Player ID
- **Ação:** Clique direito → **Format Text**
- **Format:** `"LocalClient_{0}"`
- **{0}:** Conecte ao pin `ReturnValue` do **Get Player ID**

**Nó 5: Set Actor Tag**
- **Localização:** Após Format Text
- **Ação:** Clique direito → **Set Actor Tag**
- **Tag:** Conecte ao pin `Result` do **Format Text**
- **Target:** Conecte ao pin **[Self]** do `BP_NetMovementClient`

**Nó 6: Print String**
- **Localização:** Após Set Actor Tag
- **Ação:** Clique direito → **Print String**
- **In String:** Use **Format Text:**
  - **Format:** `"✅ [BP_NetMovementClient] Tag definida: {0}"`
  - **{0}:** Conecte ao pin `Result` do **Format Text**

---

## 📋 **PASSO 2: WBP_TestDisconnect - Event Construct (Buscar por Tag)**

### **2.1: Nova Lógica do Event Construct**

```
[Event Construct]
  ↓
[Print String: "🔵 [Event Construct] Iniciando busca por BP_NetMovementClient local..."]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Player ID]
  ↓
[Format Text]
  ├─ Format: "LocalClient_{0}"
  ├─ {0}: Player ID
  └─ Result: (String) ← Tag esperada
      ↓
[Get All Actors of Class: BP_NetMovementClient]
  ↓
[Array Length]
  ↓
[Greater: Length > 0?]
  ├─ True:
  │   ├─ [Print String: "🔵 [Event Construct] Encontrados {Length} BP_NetMovementClient(s)"]
  │   ├─ [ForEachLoop]
  │   │   ├─ Array: OutActors (do GetAllActorsOfClass)
  │   │   ├─ Loop Body:
  │   │   │   ├─ [Get Actor Tag] (do Array Element)
  │   │   │   ├─ [Equal: Tag == Tag Esperada?]
  │   │   │   │   ├─ True:
  │   │   │   │   │   ├─ [Set Variable: NetMovementClientRef] ← Este é o local!
  │   │   │   │   │   ├─ [Print String: "✅ [Event Construct] BP_NetMovementClient local encontrado por Tag!"]
  │   │   │   │   │   └─ [Break] ← Parar loop
  │   │   │   │   └─ False: (continuar loop)
  │   │   └─ Completed:
  │   │       └─ [Print String: "⚠️ [Event Construct] Loop completado"]
  │   └─ False:
  │       └─ [Print String: "❌ [Event Construct] Nenhum BP_NetMovementClient encontrado"]
```

### **2.2: Detalhamento dos Nós**

**Nó 1: Event Construct**
- **Localização:** Event Graph
- **Ação:** Clique direito → **Event Construct**

**Nó 2: Print String (Início)**
- **Localização:** Após Event Construct
- **Ação:** Clique direito → **Print String**
- **In String:** `"🔵 [Event Construct] Iniciando busca por BP_NetMovementClient local..."`

**Nó 3: Get Player Controller**
- **Localização:** Após Print String
- **Ação:** Clique direito → **Get Player Controller**
- **Player Index:** `0`

**Nó 4: Get Player ID**
- **Localização:** Após Get Player Controller
- **Ação:**
  - Arraste o pin de saída do **Get Player Controller**
  - Solte no Event Graph → **Get Player ID**
- **Target:** Conecte ao pin de saída do **Get Player Controller**

**Nó 5: Format Text**
- **Localização:** Após Get Player ID
- **Ação:** Clique direito → **Format Text**
- **Format:** `"LocalClient_{0}"`
- **{0}:** Conecte ao pin `ReturnValue` do **Get Player ID**

**Nó 6: Get All Actors of Class**
- **Localização:** Após Format Text
- **Ação:** Clique direito → **Get All Actors of Class**
- **Actor Class:** `BP_NetMovementClient`

**Nó 7: Array Length**
- **Localização:** Após Get All Actors of Class
- **Ação:**
  - Arraste o pin `OutActors` do **Get All Actors of Class**
  - Solte no Event Graph → **Array Length**

**Nó 8: Greater**
- **Localização:** Após Array Length
- **Ação:** Clique direito → **Greater**
- **A:** Conecte ao pin `ReturnValue` do **Array Length**
- **B:** `0`

**Nó 9: Branch**
- **Localização:** Após Greater
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Greater**

**Nó 10: Print String (True)**
- **Localização:** No caminho "True" do Branch
- **Ação:** Clique direito → **Print String**
- **In String:** Use **Format Text:**
  - **Format:** `"🔵 [Event Construct] Encontrados {0} BP_NetMovementClient(s)"`
  - **{0}:** Conecte ao pin `ReturnValue` do **Array Length**

**Nó 11: ForEachLoop**
- **Localização:** Após Print String (True)
- **Ação:** Clique direito → **ForEachLoop**
- **Array:** Conecte ao pin `OutActors` do **Get All Actors of Class**

**Nó 12: Get Actor Tag**
- **Localização:** No **Loop Body** do ForEachLoop
- **Ação:**
  - Arraste o pin `Array Element` do **ForEachLoop**
  - Solte no Event Graph → **Get Actor Tag**
- **Target:** Conecte ao pin `Array Element` do **ForEachLoop**

**Nó 13: Equal (String)**
- **Localização:** Após Get Actor Tag
- **Ação:** Clique direito → **Equal (String)**
- **A:** Conecte ao pin `ReturnValue` do **Get Actor Tag**
- **B:** Conecte ao pin `Result` do **Format Text** (Tag esperada)

**Nó 14: Branch (Tag igual)**
- **Localização:** Após Equal
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Equal**

**Nó 15: Set Variable: NetMovementClientRef**
- **Localização:** No caminho "True" do Branch (Tag igual)
- **Ação:**
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint**
  - Solte no Event Graph → Selecione **"Set"**
- **Value:** Conecte ao pin `Array Element` do **ForEachLoop**

**Nó 16: Print String (Encontrado)**
- **Localização:** Após Set Variable
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [Event Construct] BP_NetMovementClient local encontrado por Tag!"`

**Nó 17: Break**
- **Localização:** Após Print String (Encontrado)
- **Ação:** Clique direito → **Break**

**Nó 18: Print String (False)**
- **Localização:** No caminho "False" do Branch (Length > 0)
- **Ação:** Clique direito → **Print String**
- **In String:** `"❌ [Event Construct] Nenhum BP_NetMovementClient encontrado"`

**Nó 19: Print String (Loop Completed)**
- **Localização:** No pin "Completed" do ForEachLoop
- **Ação:** Clique direito → **Print String**
- **In String:** `"⚠️ [Event Construct] Loop completado"`

---

## 📋 **PASSO 3: WBP_TestDisconnect - OnClicked (Mesma Lógica da Solução 1)**

**O `OnClicked` é idêntico à Solução 1:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Print String: "🔴 [OnClicked] Botão desconectar pressionado"]
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Get Variable: NetMovementClientRef]
  │   ├─ [Get Variable: WebSocketRef] (do NetMovementClientRef)
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   └─ [Print String: "✅ Close() chamado"]
  │   │   └─ False: [Print String: "❌ WebSocketRef inválido"]
  └─ False: [Print String: "❌ NetMovementClientRef inválido"]
```

**Veja detalhamento completo na Solução 1, Passo 5.**

---

## ✅ **CHECKLIST:**

### **BP_NetMovementClient:**
- [ ] No `BeginPlay`, obtém Player Controller (Index: 0)
- [ ] Obtém Player ID do Player Controller
- [ ] Usa Format Text para criar Tag: "LocalClient_{PlayerID}"
- [ ] Define Tag no actor usando Set Actor Tag
- [ ] Adiciona log confirmando Tag definida

### **WBP_TestDisconnect:**
- [ ] Event Construct obtém Player Controller (Index: 0)
- [ ] Event Construct obtém Player ID
- [ ] Event Construct cria Tag esperada: "LocalClient_{PlayerID}"
- [ ] Event Construct busca `BP_NetMovementClient` usando `GetAllActorsOfClass`
- [ ] Event Construct usa `ForEachLoop` para iterar sobre os actors
- [ ] Event Construct verifica Tag de cada actor usando `Get Actor Tag`
- [ ] Event Construct compara Tag com Tag esperada
- [ ] Event Construct define `NetMovementClientRef` quando encontra match
- [ ] Event Construct usa `Break` para parar loop após encontrar
- [ ] OnClicked usa `NetMovementClientRef` do Widget
- [ ] OnClicked acessa `WebSocketRef` através do `NetMovementClientRef`
- [ ] OnClicked chama `Close()` no `WebSocketRef` correto

---

## 🧪 **TESTE:**

1. **Compile `BP_NetMovementClient` e `WBP_TestDisconnect`**
2. **Execute o jogo com 2 clientes PIE**
3. **Verifique logs no `BP_NetMovementClient`:**
   - `"✅ [BP_NetMovementClient] Tag definida: LocalClient_X"`
4. **Verifique logs no Widget Event Construct:**
   - `"🔵 [Event Construct] Iniciando busca por BP_NetMovementClient local..."`
   - `"🔵 [Event Construct] Encontrados X BP_NetMovementClient(s)"`
   - `"✅ [Event Construct] BP_NetMovementClient local encontrado por Tag!"`
5. **Clique no botão "🔴 DESCONECTAR"**
6. **Verifique logs:**
   - `"🔴 [OnClicked] Botão desconectar pressionado"`
   - `"✅ [OnClicked] NetMovementClientRef é válido"`
   - `"✅ [OnClicked] WebSocketRef é válido, fechando..."`
   - `"[OnWSClosed] WebSocket fechado!"` (no BP_NetMovementClient do cliente local)

---

**✅ Fluxo completo da Solução 3!**

