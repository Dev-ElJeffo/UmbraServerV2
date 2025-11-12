# 🚀 **GUIA: Usar Classe C++ NetMovementClient**

## ✅ **SOLUÇÃO CRIADA:**

Criei uma classe C++ `ANetMovementClient` que gerencia **TUDO** automaticamente:

- ✅ Cria e conecta WebSocket no `BeginPlay`
- ✅ Fecha WebSocket explicitamente no `EndPlay`
- ✅ Remove remote actors corretamente
- ✅ Conecta delegates automaticamente
- ✅ Chama eventos Blueprint quando necessário

**Tudo funciona de forma confiável em C++!**

---

## 📋 **PASSO 1: Compilar o Projeto**

1. **Feche o Unreal Editor** (se estiver aberto)
2. **Abra o Visual Studio** com o projeto `UmbraEternumUE.sln`
3. **Compile** o projeto (Build → Build Solution ou F7)
4. **Aguarde** a compilação terminar

---

## 📋 **PASSO 2: Criar Blueprint Baseado na Classe C++**

1. **Abra o Unreal Editor**
2. **Content Browser** → **Botão direito** → **Blueprint Class**
3. **Selecione `NetMovementClient`** (a classe C++ que acabamos de criar)
4. **Nome:** `BP_NetMovementClient` (ou mantenha o nome atual se já existir)
5. **Salve** o Blueprint

---

## 📋 **PASSO 3: Migrar Lógica do Blueprint Antigo**

**No novo `BP_NetMovementClient`:**

### **O que JÁ está implementado em C++ (NÃO precisa fazer no Blueprint):**

- ✅ `BeginPlay` - Cria WebSocket automaticamente
- ✅ `EndPlay` - Fecha WebSocket e remove remote actors
- ✅ Conectar delegates (`OnConnected`, `OnClosed`, `OnRawMessage`, `OnError`)
- ✅ `RemoveRemoteActor` - Função já implementada
- ✅ `CleanupRemoteActors` - Função já implementada

### **O que você PRECISA fazer no Blueprint:**

**Apenas implementar os eventos Blueprint:**

1. **`OnWSConnected`** (BlueprintImplementableEvent)
   - Aplicar posição salva
   - Iniciar timer de `SendMoveUpdate`
   - Qualquer outra lógica que você tinha

2. **`OnWSClosed`** (BlueprintImplementableEvent)
   - Qualquer lógica adicional de cleanup (se necessário)
   - **NOTA:** O C++ já remove remote actors automaticamente!

3. **`OnWSBinaryMessage`** (BlueprintImplementableEvent)
   - Processar mensagens binárias
   - Chamar `ProcessBinaryBuffer` ou similar

4. **`OnWSError`** (BlueprintImplementableEvent)
   - Tratar erros de conexão

---

## 📋 **PASSO 4: Atualizar Variáveis e Funções**

**No `BP_NetMovementClient`:**

### **Variáveis que JÁ existem (herdadas do C++):**

- ✅ `WebSocketRef` - Já existe e é gerenciado automaticamente
- ✅ `MyPlayerId` - Já existe e é setado automaticamente
- ✅ `MyGameInstance` - Já existe e é setado automaticamente
- ✅ `RemoteActors` - Array de remote actors
- ✅ `RemoteActorIds` - Array de IDs dos remote actors

### **Funções que JÁ existem (herdadas do C++):**

- ✅ `CreateAndConnectWebSocket(Url)` - Cria e conecta WebSocket
- ✅ `CloseWebSocket()` - Fecha WebSocket explicitamente
- ✅ `RemoveRemoteActor(PlayerId)` - Remove um remote actor específico
- ✅ `CleanupRemoteActors()` - Remove todos os remote actors
- ✅ `SetMyPlayerId(PlayerId)` - Define o Player ID

**Você pode chamar essas funções do Blueprint se necessário!**

---

## 📋 **PASSO 5: Verificar Métodos do UmbraGameInstance**

**A classe C++ usa os seguintes métodos do `UmbraGameInstance`:**

- ✅ `GetZoneServerWebSocketURL()` - Já existe! Retorna a URL do WebSocket
- ✅ `GetActivePlayerID()` - Já existe! Retorna o ID do player ativo
- ✅ `HasActiveCharacter()` - Já existe! Verifica se há personagem ativo

**Todos os métodos necessários já existem! Não precisa adicionar nada.**

---

## 🧪 **TESTE:**

1. **Compile** o projeto
2. **Abra** o Unreal Editor
3. **Execute** o jogo (PIE)
4. **Verifique os logs:**
   ```
   [NetMovementClient] MyPlayerId setado: 1
   [NetMovementClient] Criando WebSocket: ws://127.0.0.1:8082
   [NetMovementClient] Conectando delegates do WebSocket...
   [NetMovementClient] Delegates conectados com sucesso!
   [NetMovementClient] WebSocket Connected!
   ```

5. **Feche o PIE** e verifique os logs:
   ```
   [NetMovementClient] EndPlay chamado! Reason: X, MyPlayerId: 1
   [NetMovementClient] Removendo remote actor do próprio client (ID: 1)
   [NetMovementClient] Fechando WebSocket explicitamente...
   [NetMovementClient] WebSocket Closed!
   [NetMovementClient] CleanupRemoteActors chamado!
   ```

---

## ✅ **VANTAGENS DA SOLUÇÃO C++:**

1. ✅ **Confiável** - C++ é mais estável que Blueprint
2. ✅ **Automático** - Tudo funciona sem configuração manual
3. ✅ **Logs detalhados** - Cada etapa é logada
4. ✅ **Delegate binding correto** - Delegates são conectados/desconectados corretamente
5. ✅ **Cleanup garantido** - `EndPlay` sempre executa o cleanup
6. ✅ **WebSocket sempre fecha** - `Close()` é chamado explicitamente

---

## 🎯 **PRÓXIMOS PASSOS:**

1. ✅ Compilar o projeto
2. ✅ Criar Blueprint baseado na classe C++
3. ✅ Implementar apenas os eventos Blueprint necessários
4. ✅ Testar e verificar logs
5. ✅ Remover lógica antiga do Blueprint antigo (se houver)

**Com essa solução, tudo deve funcionar corretamente!**

