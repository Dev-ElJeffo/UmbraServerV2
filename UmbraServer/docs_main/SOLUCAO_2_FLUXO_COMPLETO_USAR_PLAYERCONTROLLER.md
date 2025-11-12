# 🔧 **SOLUÇÃO 2: Usar PlayerController para Identificar Cliente Local - Fluxo Completo**

## 🎯 **OBJETIVO:**

Identificar o `BP_NetMovementClient` local usando `PlayerController` e verificando qual pertence ao player local.

---

## 📋 **PASSO 1: WBP_TestDisconnect - Event Construct (Nova Lógica)**

### **1.1: Remover Lógica Antiga**

**No `WBP_TestDisconnect`, Event Graph, `Event Construct`:**

**Remova toda a lógica antiga de `GetAllActorsOfClass`.**

### **1.2: Nova Lógica do Event Construct**

```
[Event Construct]
  ↓
[Print String: "🔵 [Event Construct] Iniciando busca por BP_NetMovementClient local..."]
  ↓
[Get Player Controller] (Index: 0)
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
  │   │   │   ├─ [Get Owner] (do Array Element - BP_NetMovementClient)
  │   │   │   ├─ [Is Valid: Owner?]
  │   │   │   │   ├─ True:
  │   │   │   │   │   ├─ [Get Player Controller] (do Owner)
  │   │   │   │   │   ├─ [Get Player Controller] (Index: 0) ← Player local
  │   │   │   │   │   ├─ [Equal: Owner PlayerController == Local PlayerController?]
  │   │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   │   ├─ [Set Variable: NetMovementClientRef] ← Este é o local!
  │   │   │   │   │   │   │   ├─ [Print String: "✅ [Event Construct] BP_NetMovementClient local encontrado!"]
  │   │   │   │   │   │   │   └─ [Break] ← Parar loop
  │   │   │   │   │   │   └─ False: (continuar loop)
  │   │   │   │   └─ False: (continuar loop)
  │   │   └─ Completed:
  │   │       └─ [Print String: "⚠️ [Event Construct] Loop completado"]
  │   └─ False:
  │       └─ [Print String: "❌ [Event Construct] Nenhum BP_NetMovementClient encontrado"]
```

### **1.3: Detalhamento dos Nós**

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
- **Player Index:** `0` (player local)

**Nó 4: Get All Actors of Class**
- **Localização:** Após Get Player Controller
- **Ação:** Clique direito → **Get All Actors of Class**
- **Actor Class:** `BP_NetMovementClient`
- **World Context Object:** Deixe vazio (usa contexto do widget)

**Nó 5: Array Length**
- **Localização:** Após Get All Actors of Class
- **Ação:**
  - Arraste o pin `OutActors` do **Get All Actors of Class**
  - Solte no Event Graph → **Array Length**

**Nó 6: Greater**
- **Localização:** Após Array Length
- **Ação:** Clique direito → **Greater** (ou digite `>`)
- **A:** Conecte ao pin `ReturnValue` do **Array Length**
- **B:** `0`

**Nó 7: Branch**
- **Localização:** Após Greater
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Greater**

**Nó 8: Print String (True - Array tem elementos)**
- **Localização:** No caminho "True" do Branch
- **Ação:** Clique direito → **Print String**
- **In String:** Use **Format Text:**
  - **Format:** `"🔵 [Event Construct] Encontrados {0} BP_NetMovementClient(s)"`
  - **{0}:** Conecte ao pin `ReturnValue` do **Array Length**

**Nó 9: ForEachLoop**
- **Localização:** Após Print String (True)
- **Ação:** Clique direito → **ForEachLoop**
- **Array:** Conecte ao pin `OutActors` do **Get All Actors of Class**

**Nó 10: Get Owner**
- **Localização:** No **Loop Body** do ForEachLoop
- **Ação:**
  - Arraste o pin `Array Element` do **ForEachLoop**
  - Solte no Event Graph → **Get Owner**
- **Target:** Conecte ao pin `Array Element` do **ForEachLoop**

**Nó 11: Is Valid (Owner)**
- **Localização:** Após Get Owner
- **Ação:** Clique direito → **Is Valid**
- **Object:** Conecte ao pin de saída do **Get Owner**

**Nó 12: Branch (Owner válido)**
- **Localização:** Após Is Valid (Owner)
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Is Valid (Owner)**

**Nó 13: Get Player Controller (do Owner)**
- **Localização:** No caminho "True" do Branch (Owner válido)
- **Ação:**
  - Arraste o pin de saída do **Get Owner**
  - Solte no Event Graph → **Get Player Controller**
- **Target:** Conecte ao pin de saída do **Get Owner**

**Nó 14: Get Player Controller (Local - Index 0)**
- **Localização:** Após Get Player Controller (do Owner)
- **Ação:** Clique direito → **Get Player Controller**
- **Player Index:** `0` (player local)
- **NOTA:** Este é o mesmo nó do início, você pode reutilizar o pin de saída do **Nó 3**

**Nó 15: Equal (Object)**
- **Localização:** Após ambos Get Player Controller
- **Ação:** Clique direito → **Equal (Object)**
- **A:** Conecte ao pin de saída do **Get Player Controller (do Owner)**
- **B:** Conecte ao pin de saída do **Get Player Controller (Index: 0)** (ou reutilize o pin do Nó 3)

**Nó 16: Branch (PlayerController igual)**
- **Localização:** Após Equal
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Equal**

**Nó 17: Set Variable: NetMovementClientRef**
- **Localização:** No caminho "True" do Branch (PlayerController igual)
- **Ação:**
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint**
  - Solte no Event Graph → Selecione **"Set"**
- **Value:** Conecte ao pin `Array Element` do **ForEachLoop**

**Nó 18: Print String (Encontrado)**
- **Localização:** Após Set Variable
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [Event Construct] BP_NetMovementClient local encontrado!"`

**Nó 19: Break**
- **Localização:** Após Print String (Encontrado)
- **Ação:** Clique direito → **Break**
- **NOTA:** Isso para o loop após encontrar o cliente local

**Nó 20: Print String (False - Nenhum encontrado)**
- **Localização:** No caminho "False" do Branch (Length > 0)
- **Ação:** Clique direito → **Print String**
- **In String:** `"❌ [Event Construct] Nenhum BP_NetMovementClient encontrado"`

**Nó 21: Print String (Loop Completed)**
- **Localização:** No pin "Completed" do ForEachLoop
- **Ação:** Clique direito → **Print String**
- **In String:** `"⚠️ [Event Construct] Loop completado"`

---

## 📋 **PASSO 2: WBP_TestDisconnect - OnClicked (Mesma Lógica da Solução 1)**

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

### **WBP_TestDisconnect:**
- [ ] Event Construct busca `BP_NetMovementClient` usando `GetAllActorsOfClass`
- [ ] Event Construct usa `ForEachLoop` para iterar sobre os actors
- [ ] Event Construct verifica `Get Owner` de cada `BP_NetMovementClient`
- [ ] Event Construct compara `Get Player Controller` do Owner com Player Controller local (Index: 0)
- [ ] Event Construct define `NetMovementClientRef` quando encontra o match
- [ ] Event Construct usa `Break` para parar o loop após encontrar
- [ ] OnClicked usa `NetMovementClientRef` do Widget
- [ ] OnClicked acessa `WebSocketRef` através do `NetMovementClientRef`
- [ ] OnClicked chama `Close()` no `WebSocketRef` correto

---

## 🧪 **TESTE:**

1. **Compile o Widget**
2. **Execute o jogo com 2 clientes PIE**
3. **Verifique logs no Event Construct:**
   - `"🔵 [Event Construct] Iniciando busca por BP_NetMovementClient local..."`
   - `"🔵 [Event Construct] Encontrados X BP_NetMovementClient(s)"`
   - `"✅ [Event Construct] BP_NetMovementClient local encontrado!"`
4. **Clique no botão "🔴 DESCONECTAR"**
5. **Verifique logs:**
   - `"🔴 [OnClicked] Botão desconectar pressionado"`
   - `"✅ [OnClicked] NetMovementClientRef é válido"`
   - `"✅ [OnClicked] WebSocketRef é válido, fechando..."`
   - `"[OnWSClosed] WebSocket fechado!"` (no BP_NetMovementClient do cliente local)

---

**✅ Fluxo completo da Solução 2!**

