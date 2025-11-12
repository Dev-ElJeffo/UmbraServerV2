# 🚀 **GUIA: Tecla para Desconectar Client (C++)**

## ✅ **FUNÇÃO C++ CRIADA:**

Adicionei uma função estática `DisconnectLocalClient` que:
- ✅ Encontra automaticamente o `NetMovementClient` do player local
- ✅ Fecha o WebSocket do cliente específico
- ✅ Pode ser chamada de qualquer lugar (Character, Controller, etc.)

---

## 📋 **PASSO 1: Compilar o Projeto**

1. **Feche o Unreal Editor** (se estiver aberto)
2. **Compile** o projeto no Visual Studio
3. **Aguarde** a compilação terminar

---

## 📋 **PASSO 2: Mapear Tecla no Character Blueprint**

**No `BP_ThirdPersonCharacter` (ou seu Character principal):**

### **2.1: Adicionar Input Action**

1. **Event Graph** → **Botão direito** → Procure por **`InputAction`** ou **`Input Key Event`**
2. **Selecione:** `InputAction` (recomendado) ou `Input Key Event` (F9, F10, etc.)
3. **Configure:**
   - **Input Action:** Crie um novo ou use existente (ex: "DisconnectWebSocket")
   - **OU Key:** Escolha uma tecla (ex: F9, F10, etc.)

### **2.2: Chamar Função C++**

**Após o evento de input:**

```
[InputAction: DisconnectWebSocket] (ou [F9 Key Pressed])
  ↓
[Print String: "[F9] Desconectando WebSocket..."]
  ↓
[Call Function: DisconnectLocalClient] ← FUNÇÃO C++ ESTÁTICA
  ├─ Target: (deixe vazio ou Self)
  ├─ World Context Object: Get World Context Object (ou Self)
  ↓
[Print String: "[F9] DisconnectLocalClient chamado!"]
```

**Como fazer:**
1. **Botão direito** no Event Graph
2. **Procure por:** `DisconnectLocalClient`
3. **Selecione:** `Disconnect Local Client` (deve aparecer como função estática)
4. **Conecte:**
   - **World Context Object:** `Get World Context Object` ou `Self`
   - **Execute:** Do evento de input → Para a função

---

## 📋 **PASSO 3: Configurar Input Action (se usar InputAction)**

**No Unreal Editor:**

1. **Edit** → **Project Settings** → **Input**
2. **Action Mappings** ou **Axis Mappings**
3. **+ Add** → Crie um novo:
   - **Action Name:** `DisconnectWebSocket`
   - **Key:** Escolha uma tecla (ex: F9)
4. **Salve** (Ctrl+S)

---

## 🧪 **TESTE:**

1. **Compile** o projeto
2. **Execute** o jogo (PIE)
3. **Pressione F9** (ou a tecla configurada)
4. **Verifique os logs:**

```
[F9] Desconectando WebSocket...
[NetMovementClient] DisconnectLocalClient: Procurando NetMovementClient para PlayerID: 1
[NetMovementClient] DisconnectLocalClient: Encontrado! Fechando WebSocket...
[NetMovementClient] Fechando WebSocket...
[NetMovementClient] WebSocket Closed!
[NetMovementClient] Removendo remote actor do próprio client (ID: 1)
[NetMovementClient] CleanupRemoteActors chamado!
[F9] DisconnectLocalClient chamado!
```

---

## 🔍 **VERIFICAR: Por Que Logs da BP Antiga Ainda Aparecem?**

### **PROBLEMA: Múltiplas Instâncias ou Blueprint Antigo Ainda no Level**

**Possíveis causas:**
1. **Blueprint antigo ainda está no level** (mesmo que você tenha deletado)
2. **Múltiplas instâncias** do `BP_NetMovementClient` no level
3. **Blueprint antigo está sendo spawnado dinamicamente**
4. **Cache do Unreal** ainda tem referências antigas

---

## ✅ **SOLUÇÃO: Limpar e Verificar**

### **PASSO 1: Verificar World Outliner**

1. **Abra o Level** (ex: `Lvl_Tutorial`)
2. **World Outliner** → Procure por `BP_NetMovementClient`
3. **Verifique:**
   - Quantas instâncias existem?
   - Qual é o tipo de cada uma? (deve ser o novo baseado em C++)
   - Há alguma instância do Blueprint antigo?

### **PASSO 2: Deletar Todas as Instâncias e Recriar**

1. **World Outliner** → Selecione **TODAS** as instâncias de `BP_NetMovementClient`
2. **Delete** (Delete ou Del)
3. **Content Browser** → Encontre o **novo `BP_NetMovementClient`** (baseado em C++)
4. **Arraste** para o level
5. **Salve** o level (Ctrl+S)

### **PASSO 3: Verificar Blueprint no Content Browser**

1. **Content Browser** → Procure por `BP_NetMovementClient`
2. **Verifique:**
   - Há **múltiplos** `BP_NetMovementClient`?
   - Qual é o **Parent Class** de cada um?
   - O novo deve ter **Parent Class:** `NetMovementClient` (C++)

**Para verificar Parent Class:**
- **Duplo clique** no Blueprint
- **View Options** → **Show Parent Class**
- Deve mostrar: `NetMovementClient` (não `Actor` ou outro)

### **PASSO 4: Limpar Cache do Unreal**

1. **Feche o Unreal Editor**
2. **Delete** a pasta `Intermediate/` do projeto
3. **Delete** a pasta `Saved/` do projeto (opcional, mas recomendado)
4. **Reabra** o Unreal Editor
5. **Recompile** o projeto

### **PASSO 5: Verificar Logs com Prefixo**

**Adicione um prefixo único nos logs do novo Blueprint:**

**No novo `BP_NetMovementClient`, no evento `OnWSConnected`:**

```
[OnWSConnected] (Event)
  ↓
[Print String: "[BP_NEW] OnWSConnected - WebSocket Connected!"] ← PREFIXO ÚNICO
```

**No evento `OnWSClosed`:**

```
[OnWSClosed] (Event)
  ↓
[Print String: "[BP_NEW] OnWSClosed - WebSocket Closed!"] ← PREFIXO ÚNICO
```

**Isso permitirá identificar qual Blueprint está gerando os logs!**

---

## 🎯 **RESUMO:**

**Para desconectar via tecla:**
1. ✅ Compile o projeto (função C++ já está criada)
2. ✅ Adicione evento de input no Character Blueprint
3. ✅ Chame `DisconnectLocalClient` (função estática C++)
4. ✅ Configure tecla no Project Settings (se usar InputAction)

**Para verificar logs da BP antiga:**
1. ✅ Verifique World Outliner (múltiplas instâncias?)
2. ✅ Verifique Content Browser (múltiplos Blueprints?)
3. ✅ Delete todas as instâncias e recrie
4. ✅ Limpe cache do Unreal (Intermediate/, Saved/)
5. ✅ Adicione prefixo único nos logs para identificar

**Com essas verificações, você identificará qual Blueprint está gerando os logs!**

