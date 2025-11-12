# 🚀 **GUIA COMPLETO: Integração NetMovementClient C++**

## ✅ **O QUE JÁ ESTÁ IMPLEMENTADO EM C++ (NÃO PRECISA FAZER NO BLUEPRINT):**

### **Automaticamente no `BeginPlay`:**
- ✅ Obter `GameInstance` e verificar se há personagem ativo
- ✅ Verificar se está no Client (não Server)
- ✅ Setar `MyPlayerId` automaticamente
- ✅ Criar e conectar WebSocket
- ✅ Conectar todos os delegates (`OnConnected`, `OnClosed`, `OnRawMessage`, `OnError`)

### **Automaticamente no `EndPlay`:**
- ✅ Remover remote actor do próprio client
- ✅ Fechar WebSocket explicitamente
- ✅ Limpar todos os remote actors
- ✅ Desconectar todos os delegates

### **Funções C++ Disponíveis (podem ser chamadas do Blueprint):**
- ✅ `CreateAndConnectWebSocket(Url)` - Criar e conectar WebSocket
- ✅ `CloseWebSocket()` - Fechar WebSocket
- ✅ `RemoveRemoteActor(PlayerId)` - Remover um remote actor específico
- ✅ `CleanupRemoteActors()` - Limpar todos os remote actors
- ✅ `SetMyPlayerId(PlayerId)` - Definir Player ID

### **Variáveis Disponíveis (podem ser acessadas do Blueprint):**
- ✅ `WebSocketRef` - Referência do WebSocket (já gerenciada automaticamente)
- ✅ `MyPlayerId` - ID do player (já setado automaticamente)
- ✅ `MyGameInstance` - Referência do GameInstance (já setado automaticamente)
- ✅ `RemoteActors` - Array de remote actors
- ✅ `RemoteActorIds` - Array de IDs dos remote actors

---

## 📋 **PASSO 1: Abrir o Blueprint Criado**

1. **Content Browser** → Encontre `BP_NetMovementClient` (o Blueprint que você criou)
2. **Duplo clique** para abrir no Blueprint Editor

---

## 📋 **PASSO 2: Implementar Eventos Blueprint**

### **2.1: Evento `OnWSConnected`**

**Este evento é chamado automaticamente pelo C++ quando o WebSocket conecta.**

**No `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → Procure por **`OnWSConnected`** (deve aparecer como "Event")
2. **Se não aparecer**, adicione como **Custom Event** com nome `OnWSConnected`
3. **Implemente a lógica que você tinha no Blueprint antigo:**

```
[OnWSConnected] (Event)
  ↓
[Print String: "WebSocket Connected!"]
  ↓
[Get Variable: MyGameInstance]
  ↓
[Get Active Character] (do MyGameInstance)
  ↓
[Branch: Is Valid?]
  ├─ True:
  │    ↓
  │  [Get Position] (do Active Character)
  │    ↓
  │  [Set Actor Location] (aplicar posição salva ao local pawn)
  │    ↓
  │  [Print String: "Posição aplicada: X={0}, Y={1}, Z={2}"]
  │    ↓
  │  [Set Timer by Function Name]
  │    ├─ Function Name: "SendMoveUpdate"
  │    ├─ Time: 0.05
  │    └─ Looping: True
  └─ False:
       [Print String: "Nenhum personagem ativo!"]
```

**NOTA:** A lógica de aplicar posição e iniciar timer deve vir do seu Blueprint antigo.

---

### **2.2: Evento `OnWSClosed`**

**Este evento é chamado automaticamente pelo C++ quando o WebSocket fecha.**

**No `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → Procure por **`OnWSClosed`** (deve aparecer como "Event")
2. **Se não aparecer**, adicione como **Custom Event** com nome `OnWSClosed`
3. **Implemente apenas lógica adicional (se necessário):**

```
[OnWSClosed] (Event)
  ↓
[Print String: "WebSocket Closed!"]
  ↓
[Qualquer lógica adicional de cleanup que você precisar]
```

**NOTA:** O C++ já remove remote actors automaticamente! Você só precisa adicionar lógica adicional aqui se necessário.

---

### **2.3: Evento `OnWSBinaryMessage`**

**Este evento é chamado automaticamente pelo C++ quando recebe mensagem binária.**

**No `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → Procure por **`OnWSBinaryMessage`** (deve aparecer como "Event")
2. **Se não aparecer**, adicione como **Custom Event** com nome `OnWSBinaryMessage` e parâmetro `Data` (Array of Bytes)
3. **Implemente a lógica de processamento:**

```
[OnWSBinaryMessage] (Event)
  ├─ Input: Data (Array of Bytes)
  ↓
[Print String: "Received binary message, size: {0}"]
  ↓
[Branch: Data.Num() == 5?]
  ├─ True:
  │    ↓
  │  [Branch: Data[0] == 3?] (PlayerDisconnected)
  │    ├─ True:
  │    │    ↓
  │    │  [ParsePlayerDisconnected] (do WSBinaryBPFL)
  │    │    ├─ Input: Data
  │    │    └─ Output: PlayerId
  │    │    ↓
  │    │  [Call Function: RemoveRemoteActor]
  │    │    ├─ Input: PlayerId
  │    └─ False: (não é PlayerDisconnected)
  └─ False:
       ↓
     [ProcessBinaryBuffer] (do WSBinaryBPFL)
       ├─ Input: Data
```

**NOTA:** Use a mesma lógica que você tinha no Blueprint antigo para processar mensagens binárias.

---

### **2.4: Evento `OnWSError`**

**Este evento é chamado automaticamente pelo C++ quando há erro de conexão.**

**No `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → Procure por **`OnWSError`** (deve aparecer como "Event")
2. **Se não aparecer**, adicione como **Custom Event** com nome `OnWSError` e parâmetro `Error` (String)
3. **Implemente tratamento de erro:**

```
[OnWSError] (Event)
  ├─ Input: Error (String)
  ↓
[Print String: "WebSocket Error: {0}"]
  ↓
[Qualquer lógica de tratamento de erro]
```

---

## 📋 **PASSO 3: Migrar Funções do Blueprint Antigo**

### **3.1: Função `SendMoveUpdate`**

**Se você tinha uma função `SendMoveUpdate` no Blueprint antigo, migre ela:**

1. **No Blueprint antigo**, encontre a função `SendMoveUpdate`
2. **Copie toda a lógica** (botão direito → Copy)
3. **No novo `BP_NetMovementClient`**, crie a mesma função:
   - **My Blueprint** → **Functions** → **+ Function**
   - **Nome:** `SendMoveUpdate`
4. **Cole a lógica** (botão direito → Paste)

**A função deve usar:**
- `Get Variable: WebSocketRef` → `SendBytes(Data)`
- Obter posição/rotação do local pawn
- Criar array de bytes com os dados
- Enviar via WebSocket

---

### **3.2: Função `ProcessNextFrame` (se existir)**

**Se você tinha uma função `ProcessNextFrame`, migre ela também:**

1. **No Blueprint antigo**, encontre a função `ProcessNextFrame`
2. **Copie toda a lógica**
3. **No novo `BP_NetMovementClient`**, crie a mesma função
4. **Cole a lógica**

---

### **3.3: Função `SavePlayerPosition` (se existir)**

**Se você tinha uma função `SavePlayerPosition`, migre ela:**

1. **No Blueprint antigo**, encontre a função `SavePlayerPosition`
2. **Copie toda a lógica**
3. **No novo `BP_NetMovementClient`**, crie a mesma função
4. **Cole a lógica**

**NOTA:** Esta função pode ser chamada no `EndPlay` se necessário, mas o C++ já faz o cleanup automaticamente.

---

## 📋 **PASSO 4: Migrar Lógica de Remote Actors**

### **4.1: Spawn de Remote Actors**

**Se você tinha lógica para spawnar remote actors, migre ela:**

**No evento `OnWSBinaryMessage`, após processar frames de movimento:**

```
[ProcessBinaryBuffer] (do WSBinaryBPFL)
  ├─ Output: PlayerId, Location, Rotation, etc.
  ↓
[Branch: PlayerId == MyPlayerId?]
  ├─ True: (é o próprio player, ignorar)
  └─ False:
       ↓
     [Array Find] (RemoteActorIds, PlayerId)
       ↓
     [Branch: Found Index >= 0?]
       ├─ True: (actor já existe)
       │    ↓
       │  [Get Array Item] (RemoteActors, Found Index)
       │    ↓
       │  [Set Actor Location]
       │  [Set Actor Rotation]
       └─ False: (criar novo actor)
            ↓
          [Spawn Actor from Class] (BP_ThirdPersonCharacter ou similar)
            ↓
          [Set Actor Location]
          [Set Actor Rotation]
            ↓
          [Array Add] (RemoteActors, Spawned Actor)
          [Array Add] (RemoteActorIds, PlayerId)
```

**NOTA:** Use os arrays `RemoteActors` e `RemoteActorIds` que já existem na classe C++!

---

## 📋 **PASSO 5: Verificar Variáveis e Conexões**

### **5.1: Verificar Variáveis Existentes**

**No `BP_NetMovementClient`, verifique se as seguintes variáveis existem:**

- ✅ `WebSocketRef` (UUmbraWSClient) - **JÁ EXISTE** (herdada do C++)
- ✅ `MyPlayerId` (Integer) - **JÁ EXISTE** (herdada do C++)
- ✅ `MyGameInstance` (UmbraGameInstance) - **JÁ EXISTE** (herdada do C++)
- ✅ `RemoteActors` (Array of Actor) - **JÁ EXISTE** (herdada do C++)
- ✅ `RemoteActorIds` (Array of Integer) - **JÁ EXISTE** (herdada do C++)

**Se alguma variável adicional for necessária, adicione:**

1. **My Blueprint** → **Variables** → **+ Variable**
2. **Configure** conforme necessário

---

### **5.2: Remover Lógica Duplicada**

**NÃO adicione no novo Blueprint:**

- ❌ Lógica de criar WebSocket no `BeginPlay` (já está no C++)
- ❌ Lógica de conectar delegates no `BeginPlay` (já está no C++)
- ❌ Lógica de fechar WebSocket no `EndPlay` (já está no C++)
- ❌ Lógica de remover remote actors no `EndPlay` (já está no C++)
- ❌ Lógica de cleanup no `OnWSClosed` (já está no C++)

---

## 📋 **PASSO 6: Testar a Integração**

### **6.1: Compilar o Blueprint**

1. **Compile** o Blueprint (botão **Compile** no topo)
2. **Verifique** se há erros
3. **Corrija** qualquer erro que aparecer

### **6.2: Executar o Jogo**

1. **Salve** o Blueprint (Ctrl+S)
2. **Execute** o jogo (PIE)
3. **Verifique os logs:**

**Ao conectar:**
```
[NetMovementClient] MyPlayerId setado: 1
[NetMovementClient] Criando WebSocket: ws://127.0.0.1:8082
[NetMovementClient] Conectando delegates do WebSocket...
[NetMovementClient] Delegates conectados com sucesso!
[NetMovementClient] WebSocket Connected!
WebSocket Connected! (do Blueprint)
```

**Ao fechar:**
```
[NetMovementClient] EndPlay chamado! Reason: X, MyPlayerId: 1
[NetMovementClient] Removendo remote actor do próprio client (ID: 1)
[NetMovementClient] Fechando WebSocket explicitamente...
[NetMovementClient] WebSocket Closed!
[NetMovementClient] CleanupRemoteActors chamado!
WebSocket Closed! (do Blueprint)
```

---

## 📋 **PASSO 7: Atualizar Referências no Level**

### **7.1: Substituir Blueprint Antigo**

1. **Abra o Level** onde `BP_NetMovementClient` antigo está
2. **World Outliner** → Encontre o `BP_NetMovementClient` antigo
3. **Delete** o antigo
4. **Content Browser** → Arraste o **novo `BP_NetMovementClient`** para o level
5. **Salve** o level (Ctrl+S)

### **7.2: Verificar Referências no Character**

**Se `BP_ThirdPersonCharacter` tinha referência ao `BP_NetMovementClient` antigo:**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Verifique** se há referências ao `BP_NetMovementClient` antigo
3. **Atualize** para usar o novo `BP_NetMovementClient`

---

## ✅ **CHECKLIST DE INTEGRAÇÃO:**

- [ ] Blueprint `BP_NetMovementClient` criado baseado na classe C++
- [ ] Evento `OnWSConnected` implementado
- [ ] Evento `OnWSClosed` implementado
- [ ] Evento `OnWSBinaryMessage` implementado
- [ ] Evento `OnWSError` implementado
- [ ] Função `SendMoveUpdate` migrada (se existir)
- [ ] Função `ProcessNextFrame` migrada (se existir)
- [ ] Função `SavePlayerPosition` migrada (se existir)
- [ ] Lógica de spawn de remote actors migrada
- [ ] Variáveis verificadas (todas existem)
- [ ] Lógica duplicada removida
- [ ] Blueprint compilado sem erros
- [ ] Level atualizado com novo Blueprint
- [ ] Testado e funcionando

---

## 🎯 **RESUMO:**

**O que fazer:**
1. ✅ Implementar os 4 eventos Blueprint (`OnWSConnected`, `OnWSClosed`, `OnWSBinaryMessage`, `OnWSError`)
2. ✅ Migrar funções do Blueprint antigo (`SendMoveUpdate`, etc.)
3. ✅ Migrar lógica de spawn de remote actors
4. ✅ Atualizar referências no level

**O que NÃO fazer:**
- ❌ Criar WebSocket manualmente (já está no C++)
- ❌ Conectar delegates manualmente (já está no C++)
- ❌ Fechar WebSocket manualmente (já está no C++)
- ❌ Remover remote actors manualmente no `EndPlay` (já está no C++)

**Com essa integração, tudo deve funcionar corretamente!**

