# 🔧 **GUIA: Como Reutilizar Referência do Pawn no Blueprint**

## 🎯 **PROBLEMA:**

Após fazer `Set Variable: NetMovementClientRef`, você precisa chamar `CreateDisconnectWidget` no Character, mas precisa reutilizar a **mesma referência do Pawn** que você já obteve antes do Cast.

---

## ✅ **SOLUÇÃO: Armazenar Referência do Cast**

### **MÉTODO 1: Usar Pin de Saída do Cast (RECOMENDADO)**

**O Cast já retorna a referência do Character no pin "As BP Third Person Character". Use essa referência diretamente:**

#### **Fluxo Completo:**

```
[Event BeginPlay]
  ↓
[... código existente ...]
  ↓
[Delay: 0.5]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Pawn]
  ↓
[Cast to BP_ThirdPersonCharacter]
  ├─ Object: [Get Pawn]
  └─ ReturnValue: [As BP Third Person Character] ← GUARDE ESTA REFERÊNCIA!
      ↓
[Set Variable: NetMovementClientRef] (do Character)
  ├─ Target: [As BP Third Person Character] ← USE A MESMA REFERÊNCIA AQUI
  └─ Value: [Self] (BP_NetMovementClient)
      ↓
[Print String: "✅ [BP_NetMovementClient] Referência definida no Character"]
  ↓
[Call Function: CreateDisconnectWidget] (do Character)
  └─ Target: [As BP Third Person Character] ← REUTILIZE A MESMA REFERÊNCIA AQUI!
```

---

## 📋 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Estrutura Atual (Antes da Correção)**

**Você provavelmente tem algo assim:**

```
[Delay: 0.5]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Get Pawn]
  ↓
[Cast to BP_ThirdPersonCharacter]
  ├─ Object: [Get Pawn]
  └─ ReturnValue: [As BP Third Person Character]
      ↓
[Set Variable: NetMovementClientRef] (do Character)
  ├─ Target: [As BP Third Person Character]
  └─ Value: [Self]
      ↓
[Print String: "✅ Referência definida"]
```

**Problema:** Após o `Set Variable`, você não tem mais acesso à referência do Character.

---

### **PASSO 2: Solução - Conectar Pin de Saída do Cast em Múltiplos Lugares**

**A chave é usar o pin "As BP Third Person Character" do Cast em MÚLTIPLOS lugares:**

#### **2.1: Conectar ao Set Variable (já está correto)**

```
[Cast to BP_ThirdPersonCharacter]
  └─ [As BP Third Person Character] → [Target do Set Variable]
```

#### **2.2: Conectar ao Call Function (NOVO)**

**Após o `Set Variable`, adicione:**

```
[Set Variable: NetMovementClientRef]
  └─ then (execução)
      ↓
[Call Function: CreateDisconnectWidget]
  └─ Target: [As BP Third Person Character] ← CONECTE AQUI!
```

**⚠️ IMPORTANTE:** Você precisa conectar o **mesmo pin "As BP Third Person Character"** do Cast ao Target do `Call Function`.

---

### **PASSO 3: Como Conectar Corretamente no Blueprint**

#### **Opção A: Usar Knot (Nó de Roteamento) - RECOMENDADO**

**Quando você tem um pin que precisa ser usado em múltiplos lugares, use um Knot:**

1. **Após o Cast, arraste o pin "As BP Third Person Character"**
2. **Solte no espaço vazio → Selecione "Promote to Variable" ou use um "Knot"**
3. **Ou melhor: arraste o pin diretamente para onde precisa**

**Passo a passo visual:**

```
[Cast to BP_ThirdPersonCharacter]
  └─ [As BP Third Person Character]
      ├─ → [Target do Set Variable: NetMovementClientRef]
      └─ → [Knot] (opcional, para organizar)
          └─ → [Target do Call Function: CreateDisconnectWidget]
```

#### **Opção B: Conectar Diretamente (Mais Simples)**

**No Blueprint, você pode conectar o mesmo pin a múltiplos lugares:**

1. **Do pin "As BP Third Person Character" do Cast:**
   - **Arraste uma conexão** para o `Target` do `Set Variable`
   - **Arraste OUTRA conexão** (do mesmo pin) para o `Target` do `Call Function`

**Visual:**

```
[Cast to BP_ThirdPersonCharacter]
  └─ [As BP Third Person Character]
      ├─ → [Set Variable: NetMovementClientRef] → Target
      └─ → [Call Function: CreateDisconnectWidget] → Target
```

---

## 📋 **IMPLEMENTAÇÃO COMPLETA NO BP_NetMovementClient:**

### **Estrutura Final do BeginPlay:**

```
[Event BeginPlay]
  ↓
[... código existente de conexão WebSocket ...]
  ↓
[Delay: 0.5]
  ↓
[Print String: "🔵 [BP_NetMovementClient] Tentando obter Character..."]
  ↓
[Get Player Controller] (Index: 0)
  ↓
[Is Valid: Player Controller?]
  ├─ True:
  │   ├─ [Print String: "✅ [BP_NetMovementClient] Player Controller obtido"]
  │   ├─ [Get Pawn]
  │   ├─ [Is Valid: Pawn?]
  │   │   ├─ True:
  │   │   │   ├─ [Print String: "✅ [BP_NetMovementClient] Pawn obtido"]
  │   │   │   ├─ [Cast to BP_ThirdPersonCharacter]
  │   │   │   │   ├─ Object: [Get Pawn]
  │   │   │   │   └─ ReturnValue: [As BP Third Person Character]
  │   │   │   │       ├─ → [Set Variable: NetMovementClientRef] → Target
  │   │   │   │       └─ → [Call Function: CreateDisconnectWidget] → Target
  │   │   │   │   ├─ Success (then):
  │   │   │   │   │   ├─ [Set Variable: NetMovementClientRef] (do Character)
  │   │   │   │   │   │   ├─ Target: [As BP Third Person Character]
  │   │   │   │   │   │   └─ Value: [Self] (BP_NetMovementClient)
  │   │   │   │   │   ├─ [Print String: "✅ [BP_NetMovementClient] Referência definida no Character"]
  │   │   │   │   │   ├─ [Call Function: CreateDisconnectWidget] (do Character)
  │   │   │   │   │   │   └─ Target: [As BP Third Person Character] ← MESMA REFERÊNCIA!
  │   │   │   │   │   └─ [Print String: "✅ [BP_NetMovementClient] Função CreateDisconnectWidget chamada"]
  │   │   │   │   └─ Cast Failed:
  │   │   │   │       └─ [Print String: "❌ [BP_NetMovementClient] Cast FALHOU"]
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [BP_NetMovementClient] Pawn é NULL"]
  └─ False:
      └─ [Print String: "❌ [BP_NetMovementClient] Player Controller é NULL"]
```

---

## 🎨 **COMO FAZER NO EDITOR DE BLUEPRINT:**

### **Passo 1: Localizar o Cast**

1. **No Event Graph do `BP_NetMovementClient`**
2. **Localize o nó `Cast to BP_ThirdPersonCharacter`**
3. **Observe o pin de saída "As BP Third Person Character"**

### **Passo 2: Conectar ao Set Variable (já deve estar conectado)**

1. **Do pin "As BP Third Person Character" do Cast**
2. **Arraste até o pin "Target" (ou "self") do `Set Variable: NetMovementClientRef`**
3. **Conecte**

### **Passo 3: Adicionar Call Function**

1. **Após o `Set Variable`, adicione um nó `Call Function`**
2. **Digite "CreateDisconnectWidget"**
3. **Selecione a função do `BP_ThirdPersonCharacter`**

### **Passo 4: Conectar Target do Call Function**

**AQUI ESTÁ O SEGREDO:**

1. **Do pin "As BP Third Person Character" do Cast (o MESMO pin que você usou no Set Variable)**
2. **Arraste OUTRA conexão** (pode ter múltiplas conexões do mesmo pin)
3. **Conecte ao pin "Target" (ou "self") do `Call Function: CreateDisconnectWidget`**

**Visual no Editor:**

```
[Cast to BP_ThirdPersonCharacter]
  └─ [As BP Third Person Character] ──┐
                                        ├─→ [Set Variable] → Target
                                        └─→ [Call Function] → Target
```

---

## 🔍 **VERIFICAÇÃO:**

### **Checklist:**

- [ ] O pin "As BP Third Person Character" do Cast está conectado ao `Target` do `Set Variable`
- [ ] O **MESMO pin** "As BP Third Person Character" está conectado ao `Target` do `Call Function`
- [ ] Não há `Get Pawn` ou `Get Player Controller` repetidos após o Cast
- [ ] A execução flui: `Set Variable` → `then` → `Call Function`

---

## ⚠️ **ERROS COMUNS:**

### **Erro 1: Obter Pawn Novamente**

**❌ ERRADO:**
```
[Set Variable: NetMovementClientRef]
  ↓
[Get Player Controller] ← NÃO PRECISA!
  ↓
[Get Pawn] ← NÃO PRECISA!
  ↓
[Cast to BP_ThirdPersonCharacter] ← NÃO PRECISA!
  ↓
[Call Function: CreateDisconnectWidget]
```

**✅ CORRETO:**
```
[Cast to BP_ThirdPersonCharacter]
  └─ [As BP Third Person Character]
      ├─ → [Set Variable] → Target
      └─ → [Call Function] → Target
```

### **Erro 2: Não Conectar Target do Call Function**

**❌ ERRADO:**
```
[Call Function: CreateDisconnectWidget]
  └─ Target: [DESCONECTADO] ← ERRO!
```

**✅ CORRETO:**
```
[Call Function: CreateDisconnectWidget]
  └─ Target: [As BP Third Person Character] ← CONECTADO!
```

---

## 🧪 **TESTE:**

1. **Compile o Blueprint**
2. **Execute o jogo**
3. **Verifique os logs:**

```
✅ [BP_NetMovementClient] Player Controller obtido
✅ [BP_NetMovementClient] Pawn obtido
✅ [BP_NetMovementClient] Referência definida no Character
✅ [BP_NetMovementClient] Função CreateDisconnectWidget chamada
✅ [Character] NetMovementClientRef válido, criando widget...
✅ [Character] Widget criado e adicionado
```

---

## 📝 **RESUMO:**

**A chave é:**
1. **O Cast retorna a referência do Character no pin "As BP Third Person Character"**
2. **Você pode conectar esse pin a MÚLTIPLOS lugares**
3. **Use o MESMO pin para:**
   - `Target` do `Set Variable`
   - `Target` do `Call Function`
4. **NÃO obtenha o Pawn novamente - reutilize a referência do Cast**

---

**✅ Guia completo para reutilizar referência do Pawn!**

