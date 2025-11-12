# 🔍 **EXPLICAÇÃO: Por Que Blueprint Antigo Funciona Mas Actors Despawnam?**

## 🎯 **SITUAÇÃO ATUAL:**

**Quando você:**
- ✅ Novo `BP_NetMovementClient` (C++) está no level
- ✅ Referência no Character aponta para o **antigo** Blueprint
- ✅ **Actors despawnam corretamente** (lógica da nova classe C++)

**Isso só pode significar uma coisa:**

---

## 💡 **EXPLICAÇÃO:**

### **CENÁRIO MAIS PROVÁVEL: Há Duas Instâncias no Level**

**O que está acontecendo:**

1. **Instância do NOVO `BP_NetMovementClient` (C++):**
   - ✅ Está no level
   - ✅ Executa `BeginPlay` do C++ (mas não conecta porque personagem não está pronto)
   - ✅ Executa `EndPlay` do C++ quando o level fecha → **Remove actors corretamente**
   - ❌ **NÃO é usado** pela referência do Character

2. **Instância do ANTIGO `BP_NetMovementClient`:**
   - ✅ Está no level (ou sendo spawnado)
   - ✅ Conecta WebSocket corretamente (tem lógica própria no Blueprint)
   - ✅ **É usado** pela referência do Character
   - ❌ **NÃO remove** actors no `EndPlay` (não tem lógica C++)

**Resultado:**
- ✅ WebSocket conecta (antigo Blueprint faz isso)
- ✅ Actors despawnam (novo Blueprint faz isso no `EndPlay`)
- ❌ Mas há **duplicação** e **lógica misturada**

---

## 🔍 **COMO VERIFICAR:**

### **PASSO 1: Verificar World Outliner**

1. **Abra o Level** (ex: `Lvl_Tutorial`)
2. **World Outliner** → Procure por `BP_NetMovementClient`
3. **Verifique:**
   - **Quantas instâncias** existem?
   - **Qual é o tipo** de cada uma? (Parent Class)

**Se houver 2 ou mais instâncias, esse é o problema!**

---

### **PASSO 2: Verificar Parent Class de Cada Instância**

**No World Outliner:**

1. **Selecione** cada instância de `BP_NetMovementClient`
2. **Details Panel** → Procure por **"Blueprint"** ou **"Class"**
3. **Verifique:**
   - Qual instância tem Parent Class: `NetMovementClient` (C++)?
   - Qual instância tem Parent Class: `Actor` (antigo)?

---

### **PASSO 3: Verificar Referência no Character**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Procure** por `NetMovementClientRef`
3. **Verifique:**
   - Qual Blueprint está sendo referenciado?
   - É o novo ou o antigo?

---

## ✅ **SOLUÇÃO: Usar Apenas o Novo Blueprint**

### **PASSO 1: Limpar Level**

1. **World Outliner** → **Delete TODAS** as instâncias de `BP_NetMovementClient`
2. **Salve** o level (Ctrl+S)

### **PASSO 2: Adicionar Apenas o Novo**

1. **Content Browser** → Encontre o **novo** `BP_NetMovementClient` (baseado em C++)
2. **Verifique** a Parent Class:
   - **Duplo clique** no Blueprint
   - **View Options** → **Show Parent Class**
   - Deve mostrar: `NetMovementClient` (C++)
3. **Arraste** para o level
4. **Salve** o level (Ctrl+S)

### **PASSO 3: Atualizar Referência no Character**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Procure** por `NetMovementClientRef`
3. **Atualize** para apontar para o **novo** Blueprint
4. **Compile** e **Salve**

### **PASSO 4: Implementar ConnectWebSocketManual no Novo Blueprint**

**No novo `BP_NetMovementClient`:**

1. **Event Graph** → **Event BeginPlay**
2. **Conecte** ao `Parent: BeginPlay` (chama o C++ primeiro)
3. **Adicione retry:**

```
[Event BeginPlay] (do Blueprint)
  ↓
[Parent: BeginPlay] ← CHAMA O C++ PRIMEIRO
  ↓
[Delay: 1.0] ← Aguardar personagem estar pronto
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual] ← FUNÇÃO C++
  │    ↓
  │  [Print String: "[BP_NEW] ✅ WebSocket conectado!"]
  └─ False:
       ↓
     [Delay: 2.0] ← Aguardar mais
     ↓
     [Call Function: ConnectWebSocketManual] ← Tentar novamente
```

---

## 🧪 **TESTE:**

1. **Compile** o projeto
2. **Execute** o jogo
3. **Verifique os logs:**

**Deve aparecer apenas logs do novo Blueprint:**
```
[NetMovementClient] ========== BeginPlay INICIADO! ==========
[NetMovementClient] ✅ GameInstance encontrado!
[NetMovementClient] HasActiveCharacter: 0
[BP_NEW] ConnectWebSocketManual chamado!
[NetMovementClient] ConnectWebSocketManual chamado!
[NetMovementClient] ✅ MyPlayerId setado: 1
[NetMovementClient] ✅ Criando e conectando WebSocket manualmente...
[NetMovementClient] WebSocket Connected!
```

**Se aparecerem logs do Blueprint antigo, há múltiplas instâncias!**

---

## 🎯 **RESUMO:**

**Por que actors despawnam quando usa o Blueprint antigo:**
- ✅ Há **duas instâncias** no level (novo + antigo)
- ✅ O **novo** executa `EndPlay` do C++ → Remove actors
- ✅ O **antigo** conecta WebSocket → Usado pelo Character
- ❌ Mas há **duplicação** de lógica

**Solução:**
1. ✅ Delete **TODAS** as instâncias
2. ✅ Use **apenas** o novo Blueprint (C++)
3. ✅ Implemente `ConnectWebSocketManual` no novo Blueprint
4. ✅ Atualize referência no Character

**Com isso, tudo funcionará corretamente com apenas uma instância!**

