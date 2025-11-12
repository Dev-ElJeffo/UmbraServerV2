# 📚 **GUIA DETALHADO: ForEach Loop com Break - Implementação Completa**

## 🎯 **OBJETIVO:**

Implementar a lógica para encontrar o `BP_NetMovementClient` correto (usando `MyPlayerId`) e fechar apenas o WebSocket do próprio client.

---

## 📋 **PARTE 1: ENTENDENDO O FOREACHLOOPWITHBREAK**

### **O QUE É O FOREACHLOOPWITHBREAK?**

O **ForEachLoopWithBreak** é um **macro** (nó especial) que itera sobre cada elemento de um **Array** (lista), executando uma ação para cada elemento, e permite **interromper** o loop quando necessário.

**Estrutura do ForEachLoopWithBreak:**
```
┌─────────────────────────────────┐
│   ForEachLoopWithBreak          │
├─────────────────────────────────┤
│ Array (entrada)                 │ ← Conecta o Array aqui
│                                 │
│ Break (entrada)                 │ ← Conecta quando quer parar o loop
│                                 │
│ Array Element (saída)          │ ← Cada elemento do Array
│ Array Index (saída)             │ ← Índice atual (0, 1, 2, ...)
│ Loop Body (saída)               │ ← Executa para cada elemento
│ Completed (saída)               │ ← Executa quando termina (sem Break)
└─────────────────────────────────┘
```

**IMPORTANTE:**
- **Não existe** um nó "Break" separado
- O `Break` é uma **entrada** do próprio `ForEachLoopWithBreak`
- Quando você conecta uma execução ao pino `Break`, o loop **interrompe imediatamente**

**Quando usar o Break:**
- Quando encontrar o elemento que procura
- Quando uma condição for satisfeita
- Quando não precisa continuar iterando

---

## 🔧 **PARTE 3: IMPLEMENTAÇÃO PASSO A PASSO**

### **PASSO 1: Criar os Nós Básicos**

#### **1.1. Criar o Evento de Input**

1. **No Event Graph do `BP_ThirdPersonCharacter`:**
   - **Botão direito** → **"Input"** → **"Action DisconnectWebsocket"**
   - **OU** se já existe, localize-o no gráfico

#### **1.2. Criar Print String Inicial**

1. **Botão direito** → **"Print String"**
2. **Conectar:**
   - **Execução:** Do `Action DisconnectWebsocket Pressed` → Para o `Print String`
3. **Configurar:**
   - **In String:** `"🔴 [F9] Desconectando WebSocket..."`

#### **1.3. Obter Game Instance**

1. **Botão direito** → **"Get Game Instance"**
2. **Conectar:**
   - **Execução:** Do `Print String` → Para o `Get Game Instance`

#### **1.4. Cast to Umbra Game Instance**

1. **Botão direito** → **"Cast to Umbra Game Instance"**
2. **Conectar:**
   - **Object:** Do `Get Game Instance Return Value` → Para o `Object` do Cast
   - **Execução:** Do `Get Game Instance` → Para o `Cast`
3. **Configurar:**
   - **Target Type:** `Umbra Game Instance` (se não aparecer, digite "Umbra" na busca)

#### **1.5. Get Active Player ID**

1. **Botão direito** → **"Get Active Player ID"**
2. **Conectar:**
   - **Target:** Do `Cast to Umbra Game Instance` (pino "As Umbra Game Instance") → Para o `Target` do `Get Active Player ID`
   - **Execução:** Do `Cast to Umbra Game Instance` (pino "Cast Success") → Para o `Get Active Player ID`
3. **IMPORTANTE:** O **Return Value** deste nó será usado depois para comparar com `MyPlayerId`

---

### **PASSO 2: Obter Todos os BP_NetMovementClient**

#### **2.1. Get All Actors of Class**

1. **Botão direito** → **"Get All Actors of Class"**
2. **Conectar:**
   - **Execução:** Do `Get Active Player ID` → Para o `Get All Actors of Class`
3. **Configurar:**
   - **Actor Class:** `BP_NetMovementClient` (se não aparecer, digite "BP_NetMovementClient" na busca)
   - **World Context Object:** Deixe vazio (usa o contexto atual)

#### **2.2. Get Array Length**

1. **Botão direito** → **"Array Length"** (ou digite "Array Length" na busca)
2. **Conectar:**
   - **Array:** Do `Get All Actors of Class` (pino `OutActors`) → Para o `Array` do `Array Length`
   - **Execução:** Do `Get All Actors of Class` → Para o `Array Length`

#### **2.3. Branch (Verificar se Array tem elementos)**

1. **Botão direito** → **"Branch"**
2. **Conectar:**
   - **Execução:** Do `Array Length` → Para o `Branch`
   - **Condition:** Do `Array Length` (pino `Length`) → Para o `Condition` do `Branch`
3. **Configurar:**
   - **Condition:** Conectar a um **"Greater"** (Int > Int):
     - **A:** `Array Length` → `Length`
     - **B:** `0` (constante)
   - **OU** usar diretamente: `Length > 0`

---

### **PASSO 3: Criar o ForEachLoopWithBreak**

#### **3.1. Criar o Nó ForEachLoopWithBreak**

1. **Botão direito** → **"Flow Control"** → **"ForEachLoopWithBreak"**
   - **OU** digite "ForEachLoopWithBreak" na busca
   - **OU** digite "ForEach" e procure por "ForEachLoopWithBreak"
2. **Localização:** Coloque o nó após o `Branch` (pino `True`)

#### **3.2. Conectar o Array ao ForEachLoopWithBreak**

**IMPORTANTE:** O `ForEachLoopWithBreak` precisa do **Array** como entrada.

1. **Conectar:**
   - **Array (entrada):** Do `Get All Actors of Class` (pino `OutActors`) → Para o `Array` do `ForEachLoopWithBreak`
   - **Execução:** Do `Branch` (pino `True`) → Para o `ForEachLoopWithBreak` (pino `Exec`, geralmente no topo)

**VISUALIZAÇÃO:**
```
[Get All Actors of Class]
  └─ OutActors ──────────┐
                         │
[ForEachLoopWithBreak]   │
  └─ Array (entrada) ←───┘
```

#### **3.3. Entender os Pinos do ForEachLoopWithBreak**

**ENTRADAS (Inputs):**
- **Array:** O Array que será iterado (conecta `OutActors` aqui)
- **Break:** Quando conectado e executado, **interrompe o loop imediatamente**

**SAÍDAS (Outputs):**
- **Array Element:** O elemento atual do Array (cada `BP_NetMovementClient` na iteração)
- **Array Index:** O índice atual (0, 1, 2, 3, ...)
- **Loop Body:** Executa para cada elemento (conecta a lógica aqui)
- **Completed:** Executa quando o loop termina **SEM** ser interrompido pelo Break

---

### **PASSO 4: Implementar a Lógica Dentro do Loop**

#### **4.1. Obter o MyPlayerId do Elemento Atual**

1. **Botão direito** → **"Get Variable"** (ou arraste do painel "My Blueprint")
2. **Configurar:**
   - **Variable:** `MyPlayerId` (variável do `BP_NetMovementClient`)
   - **Target:** Do `ForEach Loop` (pino `Array Element`) → Para o `Target` do `Get Variable`
3. **IMPORTANTE:** O `Target` deve ser o `Array Element` do `ForEach Loop`, não `Self`!

**VISUALIZAÇÃO:**
```
[ForEach Loop]
  └─ Array Element ──────────┐
                            │
[Get Variable: MyPlayerId]  │
  └─ Target (entrada) ←─────┘
```

#### **4.2. Comparar MyPlayerId com ActivePlayerID**

1. **Botão direito** → **"Equal (Int Int)"** (ou digite "Equal" na busca)
2. **Conectar:**
   - **A:** Do `Get Variable: MyPlayerId` (pino `MyPlayerId`) → Para o `A` do `Equal`
   - **B:** Do `Get Active Player ID` (pino `Return Value`) → Para o `B` do `Equal`
   - **Execução:** Do `ForEach Loop` (pino `Loop Body`) → Para o `Equal` (ou para um nó intermediário)

**VISUALIZAÇÃO:**
```
[Get Variable: MyPlayerId]
  └─ MyPlayerId ──────────┐
                          │
[Equal (Int Int)]         │
  └─ A (entrada) ←─────────┘
  └─ B (entrada) ←─────────┐
                          │
[Get Active Player ID]    │
  └─ Return Value ──────────┘
```

#### **4.3. Branch (Verificar se é o Client Correto)**

1. **Botão direito** → **"Branch"**
2. **Conectar:**
   - **Execução:** Do `ForEach Loop` (pino `Loop Body`) → Para o `Branch`
   - **Condition:** Do `Equal` (pino `Return Value`) → Para o `Condition` do `Branch`

**VISUALIZAÇÃO:**
```
[ForEach Loop]
  └─ Loop Body ──────────┐
                        │
[Branch]                │
  └─ execute ←───────────┘
  └─ Condition ←─────────┐
                         │
[Equal]                  │
  └─ Return Value ────────┘
```

---

### **PASSO 5: Conectar o Break**

#### **5.1. Entender o Break**

**IMPORTANTE:** 
- **Não existe** um nó "Break" separado
- O `Break` é uma **entrada** do próprio `ForEachLoopWithBreak`
- Você conecta uma **execução** ao pino `Break` do `ForEachLoopWithBreak`

#### **5.2. Conectar o Break ao ForEachLoopWithBreak**

**IMPORTANTE:** O `Break` **interrompe** o `ForEachLoopWithBreak` quando executado.

**Onde conectar:**
- **Após encontrar o elemento correto e fechar o WebSocket com sucesso**

1. **Localize o pino `Break`** no `ForEachLoopWithBreak` (é uma **entrada**)
2. **Conecte:** Do `Print String` (após fechar WebSocket) → Para o `Break` do `ForEachLoopWithBreak`

**VISUALIZAÇÃO:**
```
[Branch: MyPlayerId == ActivePlayerID?]
  └─ True:
      └─ [Close WebSocket]
          └─ [Print: "WebSocket fechado"]
              └─ [Break] ← Conectar aqui!
                  └─ (pino Break do ForEachLoopWithBreak)
```

**O QUE ACONTECE:**
- Quando o `Break` é executado, o `ForEachLoopWithBreak` **para imediatamente**
- O `Loop Body` não será executado para os elementos restantes
- O `Completed` do `ForEachLoopWithBreak` **NÃO** será executado (porque foi interrompido)

---

### **PASSO 6: Fechar o WebSocket**

#### **6.1. Obter o WebSocketRef**

1. **Botão direito** → **"Get Variable"**
2. **Configurar:**
   - **Variable:** `WebSocketRef` (variável do `BP_NetMovementClient`)
   - **Target:** Do `ForEach Loop` (pino `Array Element`) → Para o `Target` do `Get Variable`
3. **IMPORTANTE:** Use o **mesmo `Array Element`** do `ForEach Loop`!

#### **6.2. Verificar se WebSocketRef é Válido**

1. **Botão direito** → **"Is Valid"**
2. **Conectar:**
   - **Object:** Do `Get Variable: WebSocketRef` (pino `WebSocketRef`) → Para o `Object` do `Is Valid`
   - **Execução:** Do `Branch` (pino `True`) → Para o `Is Valid`

#### **6.3. Branch (Verificar Validade)**

1. **Botão direito** → **"Branch"**
2. **Conectar:**
   - **Execução:** Do `Is Valid` → Para o `Branch`
   - **Condition:** Do `Is Valid` (pino `Return Value`) → Para o `Condition` do `Branch`

#### **6.4. Chamar Close no WebSocket**

1. **Botão direito** → **"Call Function"** → **"Close"**
   - **OU** digite "Close" na busca e selecione `UmbraWSClient::Close`
2. **Conectar:**
   - **Target:** Do `Get Variable: WebSocketRef` (pino `WebSocketRef`) → Para o `Target` do `Close`
   - **Execução:** Do `Branch` (pino `True`) → Para o `Close`

#### **6.5. Print String de Sucesso**

1. **Botão direito** → **"Print String"**
2. **Conectar:**
   - **Execução:** Do `Close` → Para o `Print String`
3. **Configurar:**
   - **In String:** `"✅ [F9] WebSocket fechado com sucesso"`

---

### **PASSO 7: Tratar Casos de Erro**

#### **7.1. Se WebSocketRef for Inválido**

1. **Botão direito** → **"Print String"**
2. **Conectar:**
   - **Execução:** Do `Branch` (pino `False`, quando `Is Valid` retorna false) → Para o `Print String`
3. **Configurar:**
   - **In String:** `"⚠️ [F9] WebSocketRef inválido"`

#### **7.2. Se Não Encontrar o Client Correto**

1. **Botão direito** → **"Print String"**
2. **Conectar:**
   - **Execução:** Do `ForEach Loop` (pino `Completed`) → Para o `Print String`
3. **Configurar:**
   - **In String:** `"❌ [F9] BP_NetMovementClient com MyPlayerId correspondente não encontrado"`

**IMPORTANTE:** O `Completed` só executa se o loop **não foi interrompido** pelo `Break`. Se o `Break` foi executado, o `Completed` **NÃO** será executado.

#### **7.3. Se Array Estiver Vazio**

1. **Botão direito** → **"Print String"**
2. **Conectar:**
   - **Execução:** Do `Branch` (pino `False`, quando `Array Length` é 0) → Para o `Print String`
3. **Configurar:**
   - **In String:** `"❌ [F9] BP_NetMovementClient não encontrado no level"`

---

## 📊 **FLUXO COMPLETO VISUALIZADO**

```
[Action DisconnectWebsocket Pressed]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ├─ Cast Success:
  │   ├─ [Get Active Player ID] → Armazena ActivePlayerID
  │   ├─ [Get All Actors of Class: BP_NetMovementClient] → Array de BP_NetMovementClient
  │   ├─ [Array Length] → Quantidade de elementos
  │   ├─ [Branch: Length > 0?]
  │   │   ├─ True:
  │   │   │   ├─ [ForEachLoopWithBreak]
  │   │   │   │   ├─ Array: [OutActors do GetAllActorsOfClass]
  │   │   │   │   ├─ Loop Body:
  │   │   │   │   │   ├─ [Get Variable: MyPlayerId] (Target: Array Element)
  │   │   │   │   │   ├─ [Equal] (MyPlayerId == ActivePlayerID?)
  │   │   │   │   │   ├─ [Branch: Equal?]
  │   │   │   │   │   │   ├─ True: (ENCONTROU O CORRETO!)
  │   │   │   │   │   │   │   ├─ [Get Variable: WebSocketRef] (Target: Array Element)
  │   │   │   │   │   │   │   ├─ [Is Valid: WebSocketRef?]
  │   │   │   │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   │   │   │   ├─ [Print: "WebSocketRef válido, fechando..."]
  │   │   │   │   │   │   │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   │   │   │   │   │   │   ├─ [Print String: "✅ [F9] WebSocket fechado"]
  │   │   │   │   │   │   │   │   │   └─ → [Break] ← ✅ CONECTAR AQUI!
  │   │   │   │   │   │   │   │   └─ False:
  │   │   │   │   │   │   │   │       ├─ [Print String: "⚠️ [F9] WebSocketRef inválido"]
  │   │   │   │   │   │   │   │       └─ → [Break] ← ✅ CONECTAR AQUI TAMBÉM!
  │   │   │   │   │   │   └─ False: (continuar loop, próximo elemento)
  │   │   │   │   └─ Completed: (só executa se não foi interrompido)
  │   │   │   │       └─ [Print String: "❌ [F9] Não encontrado"]
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [F9] Array vazio"]
  └─ Cast Failed:
      └─ [Print String: "❌ [F9] Falha ao obter Game Instance"]
```

**IMPORTANTE:** 
- O `Break` é uma **entrada** do `ForEachLoopWithBreak`
- Conecte o `Break` **após** fechar o WebSocket com sucesso (ou após verificar que é inválido, mas já encontrou o correto)
- **NÃO** conecte o `Break` ao caminho `False` do Branch!

---

## 🔍 **DETALHES IMPORTANTES SOBRE O FOREACHLOOPWITHBREAK**

### **1. Array Element**

- **O que é:** O elemento atual do Array na iteração atual
- **Tipo:** Depende do tipo do Array (no nosso caso, `BP_NetMovementClient`)
- **Como usar:** Conecte ao `Target` de `Get Variable` para acessar variáveis do elemento atual

### **2. Array Index**

- **O que é:** O índice atual (0, 1, 2, 3, ...)
- **Tipo:** `Integer`
- **Quando usar:** Se precisar saber qual posição do Array está sendo processada

### **3. Loop Body**

- **O que é:** A execução que acontece para cada elemento
- **Tipo:** `Exec` (execução)
- **Como usar:** Conecte a lógica que será executada para cada elemento

### **4. Break (Entrada)**

- **O que é:** Uma **entrada** do `ForEachLoopWithBreak` que, quando executada, interrompe o loop
- **Tipo:** `Exec` (execução)
- **Quando usar:** Quando encontrar o elemento que procura ou quando uma condição for satisfeita
- **Como usar:** Conecte uma execução ao pino `Break` do `ForEachLoopWithBreak`
- **O que acontece:** Quando executado, o loop **para imediatamente** e o `Completed` **NÃO** será executado

### **5. Completed**

- **O que é:** A execução que acontece quando o loop termina
- **Tipo:** `Exec` (execução)
- **Quando executa:** Apenas se o loop **não foi interrompido** pelo `Break`
- **Como usar:** Conecte a lógica de "não encontrado" ou limpeza

---

## 🔍 **DETALHES IMPORTANTES SOBRE O BREAK**

### **1. O Break é uma Entrada do ForEachLoopWithBreak**

- **Não existe** um nó "Break" separado
- O `Break` é uma **entrada** do próprio `ForEachLoopWithBreak`
- Você conecta uma **execução** ao pino `Break` do `ForEachLoopWithBreak`

### **2. O Que Acontece Quando Break é Executado**

- O `ForEachLoopWithBreak` **para imediatamente**
- O `Loop Body` **não será executado** para os elementos restantes
- O `Completed` **NÃO será executado** (porque foi interrompido)

### **3. Onde Conectar o Break**

- **Conecte ao caminho `True`** do `Branch` quando encontrar o elemento correto
- **Conecte após fechar o WebSocket** com sucesso
- **NÃO** conecte ao caminho `False` do `Branch` (isso faria o loop parar quando NÃO encontra o correto!)
- **NÃO** conecte ao `Completed` (eles são mutuamente exclusivos)

---

## ⚠️ **ERROS COMUNS E COMO EVITAR**

### **ERRO 1: Conectar Array Element ao Self**

**ERRADO:**
```
[Get Variable: MyPlayerId]
  └─ Target: Self ← ERRADO!
```

**CORRETO:**
```
[Get Variable: MyPlayerId]
  └─ Target: Array Element ← CORRETO!
```

### **ERRO 2: Não Conectar o Break**

**PROBLEMA:** Se não conectar o `Break`, o loop continuará iterando mesmo após encontrar o elemento correto.

**SOLUÇÃO:** Sempre conecte o `Break` quando encontrar o elemento correto.

### **ERRO 3: Conectar Break ao Completed**

**ERRADO:**
```
[Print após fechar] → [ForEachLoopWithBreak Completed] ← ERRADO!
```

**CORRETO:**
```
[Print após fechar] → [ForEachLoopWithBreak Break] ← CORRETO!
```

### **ERRO 4: Conectar Break ao Caminho False**

**ERRADO:**
```
[Branch: MyPlayerId == ActivePlayerID?]
  ├─ True: (fechar WebSocket)
  └─ False: → [Break] ← ❌ ERRADO! Está no caminho errado!
```

**PROBLEMA:** Quando encontra o elemento correto (`True`), o Break não é executado, então o loop continua.

**CORRETO:**
```
[Branch: MyPlayerId == ActivePlayerID?]
  ├─ True: 
  │   └─ (fechar WebSocket)
  │       └─ → [Break] ← ✅ CORRETO! Está no caminho certo!
  └─ False: (nada - loop continua)
```

### **ERRO 5: Usar Array Index em vez de Array Element**

**ERRADO:**
```
[Get Variable: MyPlayerId]
  └─ Target: Array Index ← ERRADO! (Array Index é um Integer, não um objeto)
```

**CORRETO:**
```
[Get Variable: MyPlayerId]
  └─ Target: Array Element ← CORRETO! (Array Element é o objeto atual)
```

---

## 🧪 **TESTE FINAL**

1. **Compile** o Blueprint
2. **Execute** o jogo com **2 clients**
3. **No Client 1**, pressione F9
4. **Verifique os logs:**
   ```
   🔴 [F9] Desconectando WebSocket...
   ✅ [F9] WebSocket fechado com sucesso
   ```
5. **Verifique:**
   - ✅ Apenas o Client 1 desconecta
   - ✅ O Client 2 continua conectado
   - ✅ O servidor detecta a desconexão
   - ✅ O Client 2 remove o remote actor do Client 1

---

**✅ Com este guia detalhado, você deve conseguir implementar o ForEach Loop com Break corretamente!**

