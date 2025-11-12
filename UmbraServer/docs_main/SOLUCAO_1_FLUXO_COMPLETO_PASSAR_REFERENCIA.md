# 🔧 **SOLUÇÃO 1: Passar Referência Diretamente - Fluxo Completo Blueprint**

## 🎯 **OBJETIVO:**

Passar a referência do `BP_NetMovementClient` diretamente do Character para o Widget, evitando usar `GetAllActorsOfClass`.

---

## 📋 **PASSO 1: BP_ThirdPersonCharacter - Adicionar Variável**

### **1.1: Criar Variável**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Painel My Blueprint** (lado esquerdo) → **Variables** → **+ (Add Variable)**
3. **Configure:**
   - **Variable Name:** `NetMovementClientRef`
   - **Variable Type:** `BP Net Movement Client` (Object Reference)
   - **Instance Editable:** ✓ **TRUE**
   - **Tooltip:** "Referência ao BP_NetMovementClient local"
4. **Compile**

---

## 📋 **PASSO 2: BP_NetMovementClient - Definir Referência no Character**

### **2.1: No Event Graph do `BP_NetMovementClient`**

**Localize o `BeginPlay`** (ou adicione se não existir):

```
[Event BeginPlay]
  ↓
[... código existente de conexão WebSocket ...]
  ↓
[Delay: 0.5] (aguardar inicialização do Character)
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Pawn]
  ↓
[Cast to BP_ThirdPersonCharacter]
  ├─ Success (then):
  │   ├─ [Set Variable: NetMovementClientRef] (do Character)
  │   │   └─ Value: [Self] (BP_NetMovementClient)
  │   └─ [Print String: "✅ [BP_NetMovementClient] Referência definida no Character"]
  └─ Cast Failed:
      └─ [Print String: "⚠️ [BP_NetMovementClient] Falha ao definir referência no Character"]
```

### **2.2: Detalhamento dos Nós**

**Nó 1: Event BeginPlay**
- **Localização:** Event Graph
- **Ação:** Clique direito → **Event BeginPlay**

**Nó 2: Delay**
- **Localização:** Após código existente
- **Ação:** Clique direito → **Delay**
- **Duration:** `0.5`

**Nó 3: Get Player Controller**
- **Localização:** Após Delay
- **Ação:** Clique direito → **Get Player Controller**
- **Player Index:** `0`

**Nó 4: Get Pawn**
- **Localização:** Após Get Player Controller
- **Ação:** Arraste o pin de saída do **Get Player Controller** → Solte → **Get Pawn**

**Nó 5: Cast to BP_ThirdPersonCharacter**
- **Localização:** Após Get Pawn
- **Ação:** Arraste o pin de saída do **Get Pawn** → Solte → **Cast to BP_ThirdPersonCharacter**
- **Target Type:** `BP_ThirdPersonCharacter`

**Nó 6: Set Variable: NetMovementClientRef**
- **Localização:** No caminho "Success" do Cast
- **Ação:** 
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint** do `BP_ThirdPersonCharacter`
  - Solte no Event Graph → Selecione **"Set"**
- **Target:** Conecte ao pin de saída do **Cast** (geralmente "As BP Third Person Character")
- **Value:** Conecte ao pin **[Self]** do `BP_NetMovementClient`

**Nó 7: Print String (Success)**
- **Localização:** Após Set Variable
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [BP_NetMovementClient] Referência definida no Character"`

**Nó 8: Print String (Cast Failed)**
- **Localização:** No caminho "Cast Failed" do Cast
- **Ação:** Clique direito → **Print String**
- **In String:** `"⚠️ [BP_NetMovementClient] Falha ao definir referência no Character"`

---

## 📋 **PASSO 3: WBP_TestDisconnect - Adicionar Função SetNetMovementClient**

### **3.1: Criar Função**

1. **Abra `WBP_TestDisconnect`**
2. **Painel My Blueprint** → **Functions** → **+ (Add Function)**
3. **Nome:** `SetNetMovementClient`
4. **Compile**

### **3.2: Adicionar Input à Função**

1. **Na função `SetNetMovementClient`, clique em "Inputs"**
2. **+ (Add Input)**
3. **Configure:**
   - **Input Name:** `NetMovementClient`
   - **Input Type:** `BP Net Movement Client` (Object Reference)
4. **Compile**

### **3.3: Event Graph da Função**

**No Event Graph da função `SetNetMovementClient`:**

```
[Function Entry: SetNetMovementClient] → NetMovementClient (Input)
  ↓
[Set Variable: NetMovementClientRef] (do Widget)
  └─ Value: NetMovementClient (Input)
  ↓
[Print String: "✅ [Widget] NetMovementClientRef recebido"]
```

**Detalhamento:**

**Nó 1: Function Entry**
- **Localização:** Event Graph da função `SetNetMovementClient`
- **Já existe automaticamente** quando você cria a função
- **Input Pin:** `NetMovementClient` (já conectado)

**Nó 2: Set Variable: NetMovementClientRef**
- **Localização:** Após Function Entry
- **Ação:**
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint** do Widget
  - Solte no Event Graph → Selecione **"Set"**
- **Value:** Conecte ao pin `NetMovementClient` (Input da função)

**Nó 3: Print String**
- **Localização:** Após Set Variable
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [Widget] NetMovementClientRef recebido"`

---

## 📋 **PASSO 4: BP_ThirdPersonCharacter - Passar Referência ao Widget**

### **4.1: Localizar onde o Widget é Criado**

**No `BP_ThirdPersonCharacter`, Event Graph, `BeginPlay`:**

**Localize onde você cria o widget `WBP_TestDisconnect`** (ou adicione se não existir).

### **4.2: Substituir Lógica Atual**

**Substitua toda a lógica de `GetAllActorsOfClass` por:**

```
[Event BeginPlay]
  ↓
[... código existente ...]
  ↓
[Delay: 2.0] (aguardar inicialização completa, incluindo BP_NetMovementClient)
  ↓
[Get Variable: NetMovementClientRef] (do Character)
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Print String: "✅ [Character] NetMovementClientRef válido, criando widget..."]
  │   ├─ [Create Widget: WBP_TestDisconnect]
  │   ├─ [Call Function: SetNetMovementClient] (do Widget)
  │   │   ├─ NetMovementClient: [Get Variable: NetMovementClientRef] (do Character)
  │   │   └─ then
  │   │       ↓
  │   ├─ [Add to Viewport] (do Widget)
  │   └─ [Print String: "✅ [Character] Widget criado e referência passada"]
  └─ False:
      └─ [Print String: "⚠️ [Character] NetMovementClientRef inválido, widget não criado"]
```

### **4.3: Detalhamento dos Nós**

**Nó 1: Event BeginPlay**
- **Já existe** no `BP_ThirdPersonCharacter`

**Nó 2: Delay**
- **Localização:** Após código existente
- **Ação:** Clique direito → **Delay**
- **Duration:** `2.0` (aguardar inicialização completa)

**Nó 3: Get Variable: NetMovementClientRef**
- **Localização:** Após Delay
- **Ação:**
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint**
  - Solte no Event Graph → Selecione **"Get"**

**Nó 4: Is Valid**
- **Localização:** Após Get Variable
- **Ação:** Clique direito → **Is Valid**
- **Object:** Conecte ao pin de saída do **Get Variable: NetMovementClientRef**

**Nó 5: Branch**
- **Localização:** Após Is Valid
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Is Valid**

**Nó 6: Print String (True)**
- **Localização:** No caminho "True" do Branch
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [Character] NetMovementClientRef válido, criando widget..."`

**Nó 7: Create Widget**
- **Localização:** Após Print String (True)
- **Ação:** Clique direito → **Create Widget**
- **Class:** `WBP_TestDisconnect`
- **Owning Player:** Deixe vazio (ou conecte ao Player Controller se necessário)

**Nó 8: Call Function: SetNetMovementClient**
- **Localização:** Após Create Widget
- **Ação:**
  - Arraste o pin `ReturnValue` do **Create Widget**
  - Solte no Event Graph → Procure por **"SetNetMovementClient"**
  - Selecione a função
- **Target:** Conecte ao `ReturnValue` do **Create Widget**
- **NetMovementClient:** Conecte ao pin de saída do **Get Variable: NetMovementClientRef** (do Character)

**Nó 9: Add to Viewport**
- **Localização:** Após Call Function
- **Ação:**
  - Arraste o pin `ReturnValue` do **Create Widget**
  - Solte no Event Graph → **Add to Viewport**
- **Target:** Conecte ao `ReturnValue` do **Create Widget**

**Nó 10: Print String (Sucesso)**
- **Localização:** Após Add to Viewport
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [Character] Widget criado e referência passada"`

**Nó 11: Print String (False)**
- **Localização:** No caminho "False" do Branch
- **Ação:** Clique direito → **Print String**
- **In String:** `"⚠️ [Character] NetMovementClientRef inválido, widget não criado"`

---

## 📋 **PASSO 5: WBP_TestDisconnect - Corrigir OnClicked**

### **5.1: Remover Lógica Antiga**

**No `WBP_TestDisconnect`, Event Graph, `OnClicked` do botão:**

**Remova toda a lógica de:**
- `GetAllActorsOfClass`
- `Array Length`
- `Greater`
- `Get Array Item`

### **5.2: Nova Lógica do OnClicked**

```
[BTN_Disconnect] → OnClicked
  ↓
[Print String: "🔴 [OnClicked] Botão desconectar pressionado"]
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Print String: "✅ [OnClicked] NetMovementClientRef é válido"]
  │   ├─ [Get Variable: NetMovementClientRef] (do Widget)
  │   │   └─ (Output: BP_NetMovementClient)
  │   │       ↓
  │   ├─ [Get Variable: WebSocketRef] (do NetMovementClientRef)
  │   │   └─ (Output: UmbraWSClient)
  │   │       ↓
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Print String: "✅ [OnClicked] WebSocketRef é válido, fechando..."]
  │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   └─ [Print String: "✅ [OnClicked] Close() chamado no WebSocketRef"]
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [OnClicked] WebSocketRef é inválido!"]
  └─ False:
      └─ [Print String: "❌ [OnClicked] NetMovementClientRef é inválido!"]
```

### **5.3: Detalhamento dos Nós**

**Nó 1: Component Bound Event (OnClicked)**
- **Já existe** quando você conecta o botão ao Event Graph
- **Component:** `BTN_Disconnect`
- **Event:** `OnClicked`

**Nó 2: Print String (Início)**
- **Localização:** Após OnClicked
- **Ação:** Clique direito → **Print String**
- **In String:** `"🔴 [OnClicked] Botão desconectar pressionado"`

**Nó 3: Is Valid**
- **Localização:** Após Print String
- **Ação:** Clique direito → **Is Valid**
- **Object:** 
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint**
  - Solte no Event Graph → Selecione **"Get"**
  - Conecte o pin de saída ao **Object** do **Is Valid**

**Nó 4: Branch**
- **Localização:** Após Is Valid
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Is Valid**

**Nó 5: Print String (True - NetMovementClientRef válido)**
- **Localização:** No caminho "True" do Branch
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [OnClicked] NetMovementClientRef é válido"`

**Nó 6: Get Variable: NetMovementClientRef**
- **Localização:** Após Print String (True)
- **Ação:**
  - Arraste a variável `NetMovementClientRef` do painel **My Blueprint**
  - Solte no Event Graph → Selecione **"Get"**

**Nó 7: Get Variable: WebSocketRef**
- **Localização:** Após Get Variable: NetMovementClientRef
- **Ação:**
  - **Arraste o pin de saída** do **Get Variable: NetMovementClientRef**
  - **Solte no Event Graph** → No menu, digite: `websocket` ou `websocketref`
  - **Selecione:** **"Get WebSocketRef"** ou **"WebSocketRef"**
- **Target:** Conecte ao pin de saída do **Get Variable: NetMovementClientRef**

**Nó 8: Is Valid (WebSocketRef)**
- **Localização:** Após Get Variable: WebSocketRef
- **Ação:** Clique direito → **Is Valid**
- **Object:** Conecte ao pin de saída do **Get Variable: WebSocketRef**

**Nó 9: Branch (WebSocketRef)**
- **Localização:** Após Is Valid (WebSocketRef)
- **Ação:** Clique direito → **Branch**
- **Condition:** Conecte ao pin `ReturnValue` do **Is Valid (WebSocketRef)**

**Nó 10: Print String (True - WebSocketRef válido)**
- **Localização:** No caminho "True" do Branch (WebSocketRef)
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [OnClicked] WebSocketRef é válido, fechando..."`

**Nó 11: Call Function: Close**
- **Localização:** Após Print String (True - WebSocketRef)
- **Ação:**
  - Arraste o pin de saída do **Get Variable: WebSocketRef**
  - Solte no Event Graph → Procure por **"Close"**
  - Selecione a função **"Close"** (deve estar em `UmbraWSClient`)
- **Target:** Conecte ao pin de saída do **Get Variable: WebSocketRef**

**Nó 12: Print String (Close chamado)**
- **Localização:** Após Call Function: Close
- **Ação:** Clique direito → **Print String**
- **In String:** `"✅ [OnClicked] Close() chamado no WebSocketRef"`

**Nó 13: Print String (False - WebSocketRef inválido)**
- **Localização:** No caminho "False" do Branch (WebSocketRef)
- **Ação:** Clique direito → **Print String**
- **In String:** `"❌ [OnClicked] WebSocketRef é inválido!"`

**Nó 14: Print String (False - NetMovementClientRef inválido)**
- **Localização:** No caminho "False" do Branch (NetMovementClientRef)
- **Ação:** Clique direito → **Print String**
- **In String:** `"❌ [OnClicked] NetMovementClientRef é inválido!"`

---

## 📋 **PASSO 6: Event Construct do Widget (Opcional - Limpar Lógica Antiga)**

### **6.1: Remover Lógica Antiga**

**No `WBP_TestDisconnect`, Event Graph, `Event Construct`:**

**Remova toda a lógica de:**
- `GetAllActorsOfClass`
- `Array Length`
- `Greater`
- `Get Array Item`
- `Set Variable: NetMovementClientRef` (do Event Construct)

**OU deixe vazio**, já que a referência será passada através da função `SetNetMovementClient`.

---

## ✅ **CHECKLIST FINAL:**

### **BP_ThirdPersonCharacter:**
- [ ] Variável `NetMovementClientRef` criada (tipo: `BP Net Movement Client`, Instance Editable: True)
- [ ] No `BeginPlay`, após Delay, obtém `NetMovementClientRef`
- [ ] Verifica se `NetMovementClientRef` é válido
- [ ] Cria widget `WBP_TestDisconnect`
- [ ] Chama `SetNetMovementClient` passando `NetMovementClientRef`
- [ ] Adiciona widget ao Viewport

### **BP_NetMovementClient:**
- [ ] No `BeginPlay`, após Delay, obtém Player Controller e Pawn
- [ ] Faz Cast para `BP_ThirdPersonCharacter`
- [ ] Define `NetMovementClientRef` no Character com `[Self]`

### **WBP_TestDisconnect:**
- [ ] Função `SetNetMovementClient` criada com input `NetMovementClient`
- [ ] Função `SetNetMovementClient` define `NetMovementClientRef` do Widget
- [ ] `OnClicked` do botão usa `NetMovementClientRef` do Widget (não busca com `GetAllActorsOfClass`)
- [ ] `OnClicked` acessa `WebSocketRef` através do `NetMovementClientRef`
- [ ] `OnClicked` chama `Close()` no `WebSocketRef` correto

---

## 🧪 **TESTE:**

1. **Compile todos os Blueprints**
2. **Execute o jogo**
3. **Verifique logs:**
   - `"✅ [BP_NetMovementClient] Referência definida no Character"`
   - `"✅ [Character] NetMovementClientRef válido, criando widget..."`
   - `"✅ [Widget] NetMovementClientRef recebido"`
   - `"✅ [Character] Widget criado e referência passada"`
4. **Clique no botão "🔴 DESCONECTAR"**
5. **Verifique logs:**
   - `"🔴 [OnClicked] Botão desconectar pressionado"`
   - `"✅ [OnClicked] NetMovementClientRef é válido"`
   - `"✅ [OnClicked] WebSocketRef é válido, fechando..."`
   - `"✅ [OnClicked] Close() chamado no WebSocketRef"`
   - `"[OnWSClosed] WebSocket fechado!"` (no BP_NetMovementClient)

---

**✅ Fluxo completo da Solução 1!**

